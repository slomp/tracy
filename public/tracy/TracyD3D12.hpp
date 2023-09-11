#ifndef __TRACYD3D12_HPP__
#define __TRACYD3D12_HPP__

#ifndef TRACY_ENABLE

#define TracyD3D12Context(device, queue) nullptr
#define TracyD3D12Destroy(ctx)
#define TracyD3D12ContextName(ctx, name, size)

#define TracyD3D12Zone(ctx, cmdList, name)
#define TracyD3D12ZoneC(ctx, cmdList, name, color)
#define TracyD3D12NamedZone(ctx, varname, cmdList, name, active)
#define TracyD3D12NamedZoneC(ctx, varname, cmdList, name, color, active)
#define TracyD3D12ZoneTransient(ctx, varname, cmdList, name, active)

#define TracyD3D12ZoneS(ctx, cmdList, name, depth)
#define TracyD3D12ZoneCS(ctx, cmdList, name, color, depth)
#define TracyD3D12NamedZoneS(ctx, varname, cmdList, name, depth, active)
#define TracyD3D12NamedZoneCS(ctx, varname, cmdList, name, color, depth, active)
#define TracyD3D12ZoneTransientS(ctx, varname, cmdList, name, depth, active)

#define TracyD3D12Collect(ctx)

namespace tracy
{
    class D3D12ZoneScope {};
}

using TracyD3D12Ctx = void*;

#else

#include "Tracy.hpp"
#include "../client/TracyProfiler.hpp"
#include "../client/TracyCallstack.hpp"

#include <cstdlib>
#include <cassert>
#include <d3d12.h>
#include <dxgi.h>

#define TracyD3D12Panic(msg) assert(false && "TracyD3D12: " msg); TracyMessageLC("TracyD3D12: " msg, tracy::Color::Red4);

namespace tracy
{

    // Command queue context.
    class D3D12QueueCtx
    {
        friend class D3D12ZoneScope;

        ID3D12Device* m_device = nullptr;
        ID3D12CommandQueue* m_queue = nullptr;
        uint8_t m_contextId = 255;  // TODO: apparently, 255 means "invalid id"; is this documented somewhere?
        ID3D12QueryHeap* m_queryHeap = nullptr;
        ID3D12Resource* m_readbackBuffer = nullptr;

        static_assert(std::atomic<UINT64>::is_always_lock_free);
        std::atomic<UINT64> m_queryCounter = 0;
        uint32_t m_queryLimit = 0;

        ID3D12Fence* m_fenceCheckpoint = nullptr;
        std::atomic<UINT64> m_previousCheckpoint = 0;
        UINT64 m_nextCheckpoint = 0;

        UINT64 m_prevCalibrationTicksCPU = 0;

        tracy_force_inline uint32_t RingIndex(UINT64 index)
        {
            index %= m_queryLimit;
            return static_cast<uint32_t>(index);
        }

        tracy_force_inline uint32_t RingCount(UINT64 begin, UINT64 end)
        {
            // wrap-around safe: all unsigned
            UINT64 count = end - begin;
            return static_cast<uint32_t>(count);
        }

        tracy_force_inline void SubmitQueueItem(tracy::QueueItem* item)
        {
#ifdef TRACY_ON_DEMAND
            GetProfiler().DeferItem(*item);
#endif
            Profiler::QueueSerialFinish();
        }

        void RecalibrateClocks()
        {
            UINT64 cpuTimestamp;
            UINT64 gpuTimestamp;
            if (FAILED(m_queue->GetClockCalibration(&gpuTimestamp, &cpuTimestamp)))
            {
                TracyD3D12Panic("failed to obtain queue clock calibration counters.");
                return;
            }

            int64_t cpuDeltaTicks = cpuTimestamp - m_prevCalibrationTicksCPU;
            if (cpuDeltaTicks > 0)
            {
                static const int64_t nanosecodsPerTick = int64_t(1000000000) / GetFrequencyQpc();
                int64_t cpuDeltaNS = cpuDeltaTicks * nanosecodsPerTick;
                // Save the device cpu timestamp, not the Tracy profiler timestamp:
                m_prevCalibrationTicksCPU = cpuTimestamp;

                cpuTimestamp = Profiler::GetTime();

                auto* item = Profiler::QueueSerial();
                MemWrite(&item->hdr.type, QueueType::GpuCalibration);
                MemWrite(&item->gpuCalibration.gpuTime, gpuTimestamp);
                MemWrite(&item->gpuCalibration.cpuTime, cpuTimestamp);
                MemWrite(&item->gpuCalibration.cpuDelta, cpuDeltaNS);
                MemWrite(&item->gpuCalibration.context, GetId());
                SubmitQueueItem(item);
            }
        }

    public:
        D3D12QueueCtx(ID3D12Device* device, ID3D12CommandQueue* queue)
            : m_device(device)
            , m_queue(queue)
        {
            // Verify we support timestamp queries on this queue.

            if (queue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_COPY)
            {
                D3D12_FEATURE_DATA_D3D12_OPTIONS3 featureData{};

                bool Success = SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &featureData, sizeof(featureData)));
                assert(Success && featureData.CopyQueueTimestampQueriesSupported && "Platform does not support profiling of copy queues.");
            }

            static constexpr uint32_t MaxQueries = 64 * 1024;  // Must be even, because queries are (begin, end) pairs
            m_queryLimit = MaxQueries;

            D3D12_QUERY_HEAP_DESC heapDesc{};
            heapDesc.Type = queue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_COPY ? D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP : D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
            heapDesc.Count = m_queryLimit;
            heapDesc.NodeMask = 0;  // #TODO: Support multiple adapters.

            while (FAILED(device->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&m_queryHeap))))
            {
                m_queryLimit /= 2;
                heapDesc.Count = m_queryLimit;
            }

            // Create a readback buffer, which will be used as a destination for the query data.

            D3D12_RESOURCE_DESC readbackBufferDesc{};
            readbackBufferDesc.Alignment = 0;
            readbackBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            readbackBufferDesc.Width = m_queryLimit * sizeof(uint64_t);
            readbackBufferDesc.Height = 1;
            readbackBufferDesc.DepthOrArraySize = 1;
            readbackBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            readbackBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;  // Buffers are always row major.
            readbackBufferDesc.MipLevels = 1;
            readbackBufferDesc.SampleDesc.Count = 1;
            readbackBufferDesc.SampleDesc.Quality = 0;
            readbackBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            D3D12_HEAP_PROPERTIES readbackHeapProps{};
            readbackHeapProps.Type = D3D12_HEAP_TYPE_READBACK;
            readbackHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            readbackHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            readbackHeapProps.CreationNodeMask = 0;
            readbackHeapProps.VisibleNodeMask = 0;  // #TODO: Support multiple adapters.

            // readback buffer will be initialized with zeroes (unless D3D12_HEAP_FLAG_CREATE_NOT_ZEROED)
            if (FAILED(device->CreateCommittedResource(&readbackHeapProps, D3D12_HEAP_FLAG_NONE, &readbackBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_readbackBuffer))))
            {
                assert(false && "Failed to create query readback buffer.");
            }

            if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fenceCheckpoint))))
            {
                assert(false && "Failed to create payload fence.");
            }

            float period = [queue]()
            {
                uint64_t timestampFrequency;
                if (FAILED(queue->GetTimestampFrequency(&timestampFrequency)))
                {
                    return 0.0f;
                }
                return static_cast<float>( 1E+09 / static_cast<double>(timestampFrequency) );
            }();

            if (period == 0.0f)
            {
                assert(false && "Failed to get timestamp frequency.");
            }

            uint64_t cpuTimestamp;
            uint64_t gpuTimestamp;
            if (FAILED(queue->GetClockCalibration(&gpuTimestamp, &cpuTimestamp)))
            {
                assert(false && "Failed to get queue clock calibration.");
            }

            // Save the device cpu timestamp, not the profiler's timestamp.
            m_prevCalibrationTicksCPU = cpuTimestamp;

            cpuTimestamp = Profiler::GetTime();

            // all checked: ready to roll
            m_contextId = GetGpuCtxCounter().fetch_add(1);

            auto* item = Profiler::QueueSerial();
            MemWrite(&item->hdr.type, QueueType::GpuNewContext);
            MemWrite(&item->gpuNewContext.cpuTime, cpuTimestamp);
            MemWrite(&item->gpuNewContext.gpuTime, gpuTimestamp);
            MemWrite(&item->gpuNewContext.thread, decltype(item->gpuNewContext.thread)(0)); // TODO: why 0?
            MemWrite(&item->gpuNewContext.period, period);
            MemWrite(&item->gpuNewContext.context, m_contextId);
            MemWrite(&item->gpuNewContext.flags, GpuContextCalibration);
            MemWrite(&item->gpuNewContext.type, GpuContextType::Direct3D12);
            SubmitQueueItem(item);
        }

        ~D3D12QueueCtx()
        {
            m_fenceCheckpoint->Release();
            m_readbackBuffer->Release();
            m_queryHeap->Release();
        }

        void Name( const char* name, uint16_t len )
        {
            auto ptr = (char*)tracy_malloc( len );
            memcpy( ptr, name, len );

            auto item = Profiler::QueueSerial();
            MemWrite( &item->hdr.type, QueueType::GpuContextName );
            MemWrite( &item->gpuContextNameFat.context, GetId() );
            MemWrite( &item->gpuContextNameFat.ptr, (uint64_t)ptr );
            MemWrite( &item->gpuContextNameFat.size, len );
            SubmitQueueItem(item);
        }

        void Collect()
        {
            ZoneScopedC(Color::Red4);

#ifdef TRACY_ON_DEMAND
            if (!GetProfiler().IsConnected())
            {
                return;
            }
#endif

            // Find out what payloads are available.
            const auto newestReadyPayload = m_payloadFence->GetCompletedValue();
            const auto payloadCount = m_payloadQueue.size() - (m_activePayload - newestReadyPayload);

            UINT64 begin = m_previousCheckpoint.load();
            UINT64 latestCheckpoint = m_fenceCheckpoint->GetCompletedValue();
            uint32_t count = RingCount(begin, latestCheckpoint);

            if (count == 0)   // no pending timestamp queries
            {
                // #WARN: there's the possibility of a subtle race condition here:
                // even though NextQueryId() generates ids atomically, the matching timestamp query
                // command associated with an id is only placed into a command list a few lines later;
                // meanwhile, if Collect() happens to be running in parallel with NextQueryId(), it may
                // end up updating the checkpoint (and fence) with a query counter value that is yet to
                // be placed into a command list and submitted for execution... (that said, this should
                // not lead to issues because Collect() attempts to detect unresolved timestamp queries
                // and will defer the collection of such queries to subsequent calls)
                UINT64 nextCheckpoint = m_queryCounter.load();
                if (nextCheckpoint != latestCheckpoint)
                {
                    // safe to call Signal at any time: D3D12 command queues perform internal locking
                    m_queue->Signal(m_fenceCheckpoint, nextCheckpoint);
                }
                return;
            }

            if ((count % 2) != 0)   // paranoid check
            {
            D3D12_RANGE mapRange{ 0, m_queryLimit * sizeof(uint64_t) };
                return;
            }

            if (count >= m_queryLimit)
            {
                assert(false && "Failed to map readback buffer.");
                return;
            }

            // technically, the timestamp buffer could be persistently mapped
            UINT64* timestamps = [&]()
            {
                void* rawbytes = nullptr;
                // #TODO: map only the [begin, end) range;
                // (may require multiple mapping due to circular buffer wrap-around)
                D3D12_RANGE mapRange { 0, m_queryLimit * sizeof(UINT64) };
                m_readbackBuffer->Map(0, &mapRange, &rawbytes);
                return static_cast<UINT64*>(rawbytes);
            }();

            if (timestamps == nullptr)
            {
                    const auto counter = (payload.m_queryIdStart + j) % m_queryLimit;
                return;
            }

            for (auto i = begin; i != latestCheckpoint; ++i)
            {
                uint32_t k = RingIndex(i);
                UINT64& timestamp = timestamps[k];
                // Some timestamps may just linger around for a bit (or forever) because:
                // 1. the order in which commands are submitted to the queue is unknown to Tracy
                // 2. with parallel command list recording, timestamps ids can be distributed in any order
                // 3. an instrumented command list may not ever be submitted to a queue
                // 4. an instrumented command list may be re-submitted multiple times
                // A timestamp value of 0 does not have special meaning as far as D3D12 is concerned;
                // we just need something to identify timestamps that have not being resolved yet, and
                // zero is as good a value as anything else (sure, the GPU may actually end up writing a
                // timestamp value of zero at some point, but the odds are astronomically negligible)
                if (timestamp == 0)
                {
                    break;
                }
                m_previousCheckpoint += 1;
                auto* item = Profiler::QueueSerial();
                MemWrite(&item->hdr.type, QueueType::GpuTime);
                MemWrite(&item->gpuTime.gpuTime, timestamp);
                MemWrite(&item->gpuTime.queryId, static_cast<uint16_t>(k));
                MemWrite(&item->gpuTime.context, m_contextId);
                Profiler::QueueSerialFinish();
                timestamp = 0;  // "reset" timestamp
            }

            m_readbackBuffer->Unmap(0, nullptr);

            RecalibrateClocks();    // to account for drift
        }

    private:
        tracy_force_inline uint32_t NextQueryId()
        {
            auto id = m_queryCounter.fetch_add(2);
            if (RingCount(m_previousCheckpoint, id) >= m_queryLimit)
            {
                // #TODO: return some sentinel value; ideally a "hidden" query index
            }
            return RingIndex(id);
        }

        tracy_force_inline uint8_t GetId() const
        {
            return m_contextId;
        }
    };

    class D3D12ZoneScope
    {
        const bool m_active;
        D3D12QueueCtx* m_ctx = nullptr;
        ID3D12GraphicsCommandList* m_cmdList = nullptr;
        uint32_t m_queryId = 0;  // Used for tracking in nested zones.

        
        tracy_force_inline void WriteQueueItem(QueueItem* item, QueueType type, uint64_t srcLocation)
        {
            MemWrite(&item->hdr.type, type);
            MemWrite(&item->gpuZoneBegin.cpuTime, Profiler::GetTime());
            MemWrite(&item->gpuZoneBegin.srcloc, srcLocation);
            MemWrite(&item->gpuZoneBegin.thread, GetThreadHandle());
            MemWrite(&item->gpuZoneBegin.queryId, static_cast<uint16_t>(m_queryId));
            MemWrite(&item->gpuZoneBegin.context, m_ctx->GetId());
            Profiler::QueueSerialFinish();
        }

        tracy_force_inline D3D12ZoneScope(D3D12QueueCtx* ctx, ID3D12GraphicsCommandList* cmdList, bool active)
#ifdef TRACY_ON_DEMAND
            : m_active(active&& GetProfiler().IsConnected())
#else
            : m_active(active)
#endif
        {
            if (!m_active) return;

            m_ctx = ctx;
            m_cmdList = cmdList;

            m_queryId = m_ctx->NextQueryId();
            m_cmdList->EndQuery(m_ctx->m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, m_queryId);
        }

    public:
        tracy_force_inline D3D12ZoneScope(D3D12QueueCtx* ctx, ID3D12GraphicsCommandList* cmdList, const SourceLocationData* srcLocation, bool active)
            : D3D12ZoneScope(ctx, cmdList, active)
        {
            if (!m_active) return;

            auto* item = Profiler::QueueSerial();
            WriteQueueItem(item, QueueType::GpuZoneBeginSerial, reinterpret_cast<uint64_t>(srcLocation));
        }

        tracy_force_inline D3D12ZoneScope(D3D12QueueCtx* ctx, ID3D12GraphicsCommandList* cmdList, const SourceLocationData* srcLocation, int depth, bool active)
            : D3D12ZoneScope(ctx, cmdList, active)
        {
            if (!m_active) return;

            auto* item = Profiler::QueueSerialCallstack(Callstack(depth));
            WriteQueueItem(item, QueueType::GpuZoneBeginCallstackSerial, reinterpret_cast<uint64_t>(srcLocation));
        }

        tracy_force_inline D3D12ZoneScope(D3D12QueueCtx* ctx, uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz, ID3D12GraphicsCommandList* cmdList, bool active)
            : D3D12ZoneScope(ctx, cmdList, active)
        {
            if (!m_active) return;

            const auto sourceLocation = Profiler::AllocSourceLocation(line, source, sourceSz, function, functionSz, name, nameSz);

            auto* item = Profiler::QueueSerial();
            WriteQueueItem(item, QueueType::GpuZoneBeginAllocSrcLocSerial, sourceLocation);
        }

        tracy_force_inline D3D12ZoneScope(D3D12QueueCtx* ctx, uint32_t line, const char* source, size_t sourceSz, const char* function, size_t functionSz, const char* name, size_t nameSz, ID3D12GraphicsCommandList* cmdList, int depth, bool active)
            : D3D12ZoneScope(ctx, cmdList, active)
        {
            if (!m_active) return;

            const auto sourceLocation = Profiler::AllocSourceLocation(line, source, sourceSz, function, functionSz, name, nameSz);

            auto* item = Profiler::QueueSerialCallstack(Callstack(depth));
            WriteQueueItem(item, QueueType::GpuZoneBeginAllocSrcLocCallstackSerial, sourceLocation);
        }

        tracy_force_inline ~D3D12ZoneScope()
        {
            if (!m_active) return;

            const auto queryId = m_queryId + 1;  // Our end query slot is immediately after the begin slot.
            m_cmdList->EndQuery(m_ctx->m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, queryId);

            auto* item = Profiler::QueueSerial();
            MemWrite(&item->hdr.type, QueueType::GpuZoneEndSerial);
            MemWrite(&item->gpuZoneEnd.cpuTime, Profiler::GetTime());
            MemWrite(&item->gpuZoneEnd.thread, GetThreadHandle());
            MemWrite(&item->gpuZoneEnd.queryId, static_cast<uint16_t>(queryId));
            MemWrite(&item->gpuZoneEnd.context, m_ctx->GetId());
            Profiler::QueueSerialFinish();

            // #TODO: consider resolving entire ranges during Collect
            // (ResolveQueryData seems to ntroduce a gap of a few microseconds between commands)
            m_cmdList->ResolveQueryData(m_ctx->m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, m_queryId, 2, m_ctx->m_readbackBuffer, m_queryId * sizeof(uint64_t));
        }
    };

    static inline D3D12QueueCtx* CreateD3D12Context(ID3D12Device* device, ID3D12CommandQueue* queue)
    {
        auto* ctx = static_cast<D3D12QueueCtx*>(tracy_malloc(sizeof(D3D12QueueCtx)));
        new (ctx) D3D12QueueCtx{ device, queue };

        return ctx;
    }

    static inline void DestroyD3D12Context(D3D12QueueCtx* ctx)
    {
        ctx->~D3D12QueueCtx();
        tracy_free(ctx);
    }

}

#undef TracyD3D12Panic

using TracyD3D12Ctx = tracy::D3D12QueueCtx*;

#define TracyD3D12Context(device, queue) tracy::CreateD3D12Context(device, queue);
#define TracyD3D12Destroy(ctx) tracy::DestroyD3D12Context(ctx);
#define TracyD3D12ContextName(ctx, name, size) ctx->Name(name, size);

#define TracyD3D12UnnamedZone ___tracy_gpu_d3d12_zone
#define TracyD3D12SrcLocSymbol TracyConcat(__tracy_d3d12_source_location,TracyLine)
#define TracyD3D12SrcLocObject(name, color) static constexpr tracy::SourceLocationData TracyD3D12SrcLocSymbol { name, TracyFunction, TracyFile, (uint32_t)TracyLine, color };

#if defined TRACY_HAS_CALLSTACK && defined TRACY_CALLSTACK
#  define TracyD3D12Zone(ctx, cmdList, name) TracyD3D12NamedZoneS(ctx, TracyD3D12UnnamedZone, cmdList, name, TRACY_CALLSTACK, true)
#  define TracyD3D12ZoneC(ctx, cmdList, name, color) TracyD3D12NamedZoneCS(ctx, TracyD3D12UnnamedZone, cmdList, name, color, TRACY_CALLSTACK, true)
#  define TracyD3D12NamedZone(ctx, varname, cmdList, name, active) TracyD3D12SrcLocObject(name, 0); tracy::D3D12ZoneScope varname{ ctx, cmdList, &TracyD3D12SrcLocSymbol, TRACY_CALLSTACK, active };
#  define TracyD3D12NamedZoneC(ctx, varname, cmdList, name, color, active) TracyD3D12SrcLocObject(name, color); tracy::D3D12ZoneScope varname{ ctx, cmdList, &TracyD3D12SrcLocSymbol, TRACY_CALLSTACK, active };
#  define TracyD3D12ZoneTransient(ctx, varname, cmdList, name, active) TracyD3D12ZoneTransientS(ctx, varname, cmdList, name, TRACY_CALLSTACK, active)
#else
#  define TracyD3D12Zone(ctx, cmdList, name) TracyD3D12NamedZone(ctx, TracyD3D12UnnamedZone, cmdList, name, true)
#  define TracyD3D12ZoneC(ctx, cmdList, name, color) TracyD3D12NamedZoneC(ctx, TracyD3D12UnnamedZone, cmdList, name, color, true)
#  define TracyD3D12NamedZone(ctx, varname, cmdList, name, active) TracyD3D12SrcLocObject(name, 0); tracy::D3D12ZoneScope varname{ ctx, cmdList, &TracyD3D12SrcLocSymbol, active };
#  define TracyD3D12NamedZoneC(ctx, varname, cmdList, name, color, active) TracyD3D12SrcLocObject(name, color); tracy::D3D12ZoneScope varname{ ctx, cmdList, &TracyD3D12SrcLocSymbol, active };
#  define TracyD3D12ZoneTransient(ctx, varname, cmdList, name, active) tracy::D3D12ZoneScope varname{ ctx, TracyLine, TracyFile, strlen(TracyFile), TracyFunction, strlen(TracyFunction), name, strlen(name), cmdList, active };
#endif

#ifdef TRACY_HAS_CALLSTACK
#  define TracyD3D12ZoneS(ctx, cmdList, name, depth) TracyD3D12NamedZoneS(ctx, TracyD3D12UnnamedZone, cmdList, name, depth, true)
#  define TracyD3D12ZoneCS(ctx, cmdList, name, color, depth) TracyD3D12NamedZoneCS(ctx, TracyD3D12UnnamedZone, cmdList, name, color, depth, true)
#  define TracyD3D12NamedZoneS(ctx, varname, cmdList, name, depth, active) TracyD3D12SrcLocObject(name, 0); tracy::D3D12ZoneScope varname{ ctx, cmdList, &TracyD3D12SrcLocSymbol, depth, active };
#  define TracyD3D12NamedZoneCS(ctx, varname, cmdList, name, color, depth, active) TracyD3D12SrcLocObject(name, color); tracy::D3D12ZoneScope varname{ ctx, cmdList, &TracyD3D12SrcLocSymbol, depth, active };
#  define TracyD3D12ZoneTransientS(ctx, varname, cmdList, name, depth, active) tracy::D3D12ZoneScope varname{ ctx, TracyLine, TracyFile, strlen(TracyFile), TracyFunction, strlen(TracyFunction), name, strlen(name), cmdList, depth, active };
#else
#  define TracyD3D12ZoneS(ctx, cmdList, name, depth) TracyD3D12Zone(ctx, cmdList, name)
#  define TracyD3D12ZoneCS(ctx, cmdList, name, color, depth) TracyD3D12Zone(ctx, cmdList, name, color)
#  define TracyD3D12NamedZoneS(ctx, varname, cmdList, name, depth, active) TracyD3D12NamedZone(ctx, varname, cmdList, name, active)
#  define TracyD3D12NamedZoneCS(ctx, varname, cmdList, name, color, depth, active) TracyD3D12NamedZoneC(ctx, varname, cmdList, name, color, active)
#  define TracyD3D12ZoneTransientS(ctx, varname, cmdList, name, depth, active) TracyD3D12ZoneTransient(ctx, varname, cmdList, name, active)
#endif

#define TracyD3D12Collect(ctx) ctx->Collect();

#endif

#endif
