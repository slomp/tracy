#include "TracyFileselector.hpp"
#include "TracyImGui.hpp"
#include "TracyPrint.hpp"
#include "TracyTexture.hpp"
#include "TracyView.hpp"
#include "../../public/common/TracyProtocol.hpp"
#include "../../public/common/TracyQueue.hpp"
#include "../Fonts.hpp"

namespace tracy
{

static const char* GetQueueTypeName( QueueType type )
{
    switch( type )
    {
    case QueueType::ZoneText: return "ZoneText";
    case QueueType::ZoneName: return "ZoneName";
    case QueueType::Message: return "Message";
    case QueueType::MessageColor: return "MessageColor";
    case QueueType::MessageCallstack: return "MessageCallstack";
    case QueueType::MessageColorCallstack: return "MessageColorCallstack";
    case QueueType::MessageAppInfo: return "MessageAppInfo";
    case QueueType::ZoneBeginAllocSrcLoc: return "ZoneBeginAllocSrcLoc";
    case QueueType::ZoneBeginAllocSrcLocCallstack: return "ZoneBeginAllocSrcLocCallstack";
    case QueueType::CallstackSerial: return "CallstackSerial";
    case QueueType::Callstack: return "Callstack";
    case QueueType::CallstackAlloc: return "CallstackAlloc";
    case QueueType::CallstackSample: return "CallstackSample";
    case QueueType::CallstackSampleContextSwitch: return "CallstackSampleContextSwitch";
    case QueueType::FrameImage: return "FrameImage";
    case QueueType::ZoneBegin: return "ZoneBegin";
    case QueueType::ZoneBeginCallstack: return "ZoneBeginCallstack";
    case QueueType::ZoneEnd: return "ZoneEnd";
    case QueueType::LockWait: return "LockWait";
    case QueueType::LockObtain: return "LockObtain";
    case QueueType::LockRelease: return "LockRelease";
    case QueueType::LockSharedWait: return "LockSharedWait";
    case QueueType::LockSharedObtain: return "LockSharedObtain";
    case QueueType::LockSharedRelease: return "LockSharedRelease";
    case QueueType::LockName: return "LockName";
    case QueueType::MemAlloc: return "MemAlloc";
    case QueueType::MemAllocNamed: return "MemAllocNamed";
    case QueueType::MemFree: return "MemFree";
    case QueueType::MemFreeNamed: return "MemFreeNamed";
    case QueueType::MemAllocCallstack: return "MemAllocCallstack";
    case QueueType::MemAllocCallstackNamed: return "MemAllocCallstackNamed";
    case QueueType::MemFreeCallstack: return "MemFreeCallstack";
    case QueueType::MemFreeCallstackNamed: return "MemFreeCallstackNamed";
    case QueueType::MemDiscard: return "MemDiscard";
    case QueueType::MemDiscardCallstack: return "MemDiscardCallstack";
    case QueueType::GpuZoneBegin: return "GpuZoneBegin";
    case QueueType::GpuZoneBeginCallstack: return "GpuZoneBeginCallstack";
    case QueueType::GpuZoneBeginAllocSrcLoc: return "GpuZoneBeginAllocSrcLoc";
    case QueueType::GpuZoneBeginAllocSrcLocCallstack: return "GpuZoneBeginAllocSrcLocCallstack";
    case QueueType::GpuZoneEnd: return "GpuZoneEnd";
    case QueueType::GpuZoneBeginSerial: return "GpuZoneBeginSerial";
    case QueueType::GpuZoneBeginCallstackSerial: return "GpuZoneBeginCallstackSerial";
    case QueueType::GpuZoneBeginAllocSrcLocSerial: return "GpuZoneBeginAllocSrcLocSerial";
    case QueueType::GpuZoneBeginAllocSrcLocCallstackSerial: return "GpuZoneBeginAllocSrcLocCallstackSerial";
    case QueueType::GpuZoneEndSerial: return "GpuZoneEndSerial";
    case QueueType::PlotDataInt: return "PlotDataInt";
    case QueueType::PlotDataFloat: return "PlotDataFloat";
    case QueueType::PlotDataDouble: return "PlotDataDouble";
    case QueueType::ContextSwitch: return "ContextSwitch";
    case QueueType::ThreadWakeup: return "ThreadWakeup";
    case QueueType::GpuTime: return "GpuTime";
    case QueueType::GpuContextName: return "GpuContextName";
    case QueueType::GpuAnnotationName: return "GpuAnnotationName";
    case QueueType::CallstackFrameSize: return "CallstackFrameSize";
    case QueueType::SymbolInformation: return "SymbolInformation";
    case QueueType::ExternalNameMetadata: return "ExternalNameMetadata";
    case QueueType::SymbolCodeMetadata: return "SymbolCodeMetadata";
    case QueueType::SourceCodeMetadata: return "SourceCodeMetadata";
    case QueueType::FiberEnter: return "FiberEnter";
    case QueueType::FiberLeave: return "FiberLeave";
    case QueueType::Terminate: return "Terminate";
    case QueueType::KeepAlive: return "KeepAlive";
    case QueueType::ThreadContext: return "ThreadContext";
    case QueueType::GpuCalibration: return "GpuCalibration";
    case QueueType::GpuTimeSync: return "GpuTimeSync";
    case QueueType::Crash: return "Crash";
    case QueueType::CrashReport: return "CrashReport";
    case QueueType::ZoneValidation: return "ZoneValidation";
    case QueueType::ZoneColor: return "ZoneColor";
    case QueueType::ZoneValue: return "ZoneValue";
    case QueueType::FrameMarkMsg: return "FrameMarkMsg";
    case QueueType::FrameMarkMsgStart: return "FrameMarkMsgStart";
    case QueueType::FrameMarkMsgEnd: return "FrameMarkMsgEnd";
    case QueueType::FrameVsync: return "FrameVsync";
    case QueueType::SourceLocation: return "SourceLocation";
    case QueueType::LockAnnounce: return "LockAnnounce";
    case QueueType::LockTerminate: return "LockTerminate";
    case QueueType::LockMark: return "LockMark";
    case QueueType::MessageLiteral: return "MessageLiteral";
    case QueueType::MessageLiteralColor: return "MessageLiteralColor";
    case QueueType::MessageLiteralCallstack: return "MessageLiteralCallstack";
    case QueueType::MessageLiteralColorCallstack: return "MessageLiteralColorCallstack";
    case QueueType::GpuNewContext: return "GpuNewContext";
    case QueueType::CallstackFrame: return "CallstackFrame";
    case QueueType::SysTimeReport: return "SysTimeReport";
    case QueueType::SysPowerReport: return "SysPowerReport";
    case QueueType::TidToPid: return "TidToPid";
    case QueueType::HwSampleCpuCycle: return "HwSampleCpuCycle";
    case QueueType::HwSampleInstructionRetired: return "HwSampleInstructionRetired";
    case QueueType::HwSampleCacheReference: return "HwSampleCacheReference";
    case QueueType::HwSampleCacheMiss: return "HwSampleCacheMiss";
    case QueueType::HwSampleBranchRetired: return "HwSampleBranchRetired";
    case QueueType::HwSampleBranchMiss: return "HwSampleBranchMiss";
    case QueueType::PlotConfig: return "PlotConfig";
    case QueueType::ParamSetup: return "ParamSetup";
    case QueueType::AckServerQueryNoop: return "AckServerQueryNoop";
    case QueueType::AckSourceCodeNotAvailable: return "AckSourceCodeNotAvailable";
    case QueueType::AckSymbolCodeNotAvailable: return "AckSymbolCodeNotAvailable";
    case QueueType::CpuTopology: return "CpuTopology";
    case QueueType::SingleStringData: return "SingleStringData";
    case QueueType::SecondStringData: return "SecondStringData";
    case QueueType::MemNamePayload: return "MemNamePayload";
    case QueueType::ThreadGroupHint: return "ThreadGroupHint";
    case QueueType::GpuZoneAnnotation: return "GpuZoneAnnotation";
    case QueueType::StringData: return "StringData";
    case QueueType::ThreadName: return "ThreadName";
    case QueueType::PlotName: return "PlotName";
    case QueueType::SourceLocationPayload: return "SourceLocationPayload";
    case QueueType::CallstackPayload: return "CallstackPayload";
    case QueueType::CallstackAllocPayload: return "CallstackAllocPayload";
    case QueueType::FrameName: return "FrameName";
    case QueueType::FrameImageData: return "FrameImageData";
    case QueueType::ExternalName: return "ExternalName";
    case QueueType::ExternalThreadName: return "ExternalThreadName";
    case QueueType::SymbolCode: return "SymbolCode";
    case QueueType::SourceCode: return "SourceCode";
    case QueueType::FiberName: return "FiberName";
    case QueueType::NUM_TYPES: return "?";
    default: return "?";
    }
}

static const char* GetServerQueryTypeName( ServerQuery type )
{
    switch( type )
    {
    case ServerQueryTerminate:       return "Terminate";
    case ServerQueryString:         return "String";
    case ServerQueryThreadString:   return "Thread name";
    case ServerQuerySourceLocation: return "Source location";
    case ServerQueryPlotName:       return "Plot name";
    case ServerQueryFrameName:      return "Frame name";
    case ServerQueryParameter:      return "Parameter";
    case ServerQueryFiberName:      return "Fiber name";
    case ServerQueryExternalName:   return "External name";
    case ServerQueryDisconnect:     return "Disconnect";
    case ServerQueryCallstackFrame: return "Callstack frame";
    case ServerQuerySymbol:         return "Symbol";
    case ServerQuerySymbolCode:     return "Symbol code";
    case ServerQuerySourceCode:     return "Source code";
    case ServerQueryDataTransfer:   return "Data transfer";
    case ServerQueryDataTransferPart: return "Data transfer (part)";
    default: return "?";
    }
}

constexpr size_t SendQueueEnableThreshold = 1000000;
constexpr size_t SendQueueDisableThreshold = 500000;
constexpr int64_t SendQueueTimespanMs = 10000;  // 10 s

bool View::DrawConnection()
{
    const auto scale = GetScale();
    const auto ty = ImGui::GetTextLineHeight();
    const auto isConnected = m_worker.IsConnected();
    size_t sendQueue;
    std::array<size_t, ServerQueryCount> sendQueueBreakdown;

    {
        std::shared_lock<std::shared_mutex> lock( m_worker.GetMbpsDataLock() );
        TextFocused( isConnected ? "Connected to:" : "Disconnected:", m_worker.GetAddr().c_str() );
        const auto& mbpsVector = m_worker.GetMbpsData();
        const auto mbps = mbpsVector.back();
        char buf[64];
        if( mbps < 0.1f )
        {
            sprintf( buf, "%6.2f Kbps", mbps * 1000.f );
        }
        else
        {
            sprintf( buf, "%6.2f Mbps", mbps );
        }
        ImGui::AlignTextToFramePadding();
        TextColoredUnformatted( isConnected ? 0xFF2222CC : 0xFF444444, ICON_FA_CIRCLE );
        ImGui::SameLine();
        ImGui::PlotLines( buf, mbpsVector.data(), mbpsVector.size(), 0, nullptr, 0, std::numeric_limits<float>::max(), ImVec2( 150 * scale, 0 ) );
        TextDisabledUnformatted( "Ratio" );
        ImGui::SameLine();
        ImGui::Text( "%.1f%%", m_worker.GetCompRatio() * 100.f );
        ImGui::SameLine();
        TextDisabledUnformatted( "Real:" );
        ImGui::SameLine();
        ImGui::Text( "%6.2f Mbps", mbps / m_worker.GetCompRatio() );
        TextFocused( "Data transferred:", MemSizeToString( m_worker.GetDataTransferred() ) );
        sendQueue = m_worker.GetSendQueueSize();
        sendQueueBreakdown = m_worker.GetSendQueueBreakdown();
    }

    if( ImGui::CollapsingHeader( "Query backlog" ) )
    {
        const float backlogTableWidth = 320.f * scale;
        const float backlogTableHeight = 180.f * scale;
        if( ImGui::BeginChild( "##querybacklogscroll", ImVec2( backlogTableWidth, backlogTableHeight ), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_AlwaysVerticalScrollbar ) )
        {
            if( ImGui::BeginTable( "##querybacklog", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable ) )
            {
                ImGui::TableSetupColumn( "Query type", ImGuiTableColumnFlags_WidthStretch );
                ImGui::TableSetupColumn( "Backlog", ImGuiTableColumnFlags_WidthFixed, 80 * scale );
                ImGui::TableHeadersRow();
                for( int i = 0; i < ServerQueryCount; i++ )
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( GetServerQueryTypeName( (ServerQuery)i ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", RealToString( sendQueueBreakdown[i] ) );
                }
                ImGui::TableNextRow();
                ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, 0xFF404040 );
                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Total" );
                ImGui::TableNextColumn();
                ImGui::Text( "%s", RealToString( sendQueue ) );
                ImGui::EndTable();
            }
            ImGui::Dummy( ImVec2( 0, 8 ) );
        }
        ImGui::EndChild();
    }

    if( ImGui::CollapsingHeader( "Server work breakdown", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        std::array<uint64_t, (size_t)QueueType::NUM_TYPES> totalTimeNs, totalCalls;
        uint64_t totalIdleTimeNs = 0, idleCount = 0;
        uint64_t totalMainThreadHandoffTimeNs = 0, mainThreadHandoffCount = 0;
        uint64_t totalServerQuerySendTimeNs = 0, serverQuerySendCount = 0;
        m_worker.GetServerWorkStats( totalTimeNs, totalCalls, totalIdleTimeNs, idleCount, totalMainThreadHandoffTimeNs, mainThreadHandoffCount, totalServerQuerySendTimeNs, serverQuerySendCount );
        const float tableWidth = 450.f * scale;
        const float tableHeight = 200.f * scale;
        if( ImGui::BeginChild( "##serverworkscroll", ImVec2( tableWidth, tableHeight ), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_AlwaysVerticalScrollbar ) )
        {
            if( ImGui::BeginTable( "##serverwork", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable ) )
            {
                ImGui::TableSetupColumn( "Operation", ImGuiTableColumnFlags_WidthStretch );
                ImGui::TableSetupColumn( "Total time", ImGuiTableColumnFlags_WidthFixed, 100 * scale );
                ImGui::TableSetupColumn( "Calls", ImGuiTableColumnFlags_WidthFixed, 80 * scale );
                ImGui::TableSetupColumn( "Mean/call", ImGuiTableColumnFlags_WidthFixed, 80 * scale );
                ImGui::TableHeadersRow();
                for( size_t i = 0; i < (size_t)QueueType::NUM_TYPES; i++ )
                {
                    if( totalCalls[i] == 0 ) continue;
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( GetQueueTypeName( (QueueType)i ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", TimeToString( (int64_t)totalTimeNs[i] ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", RealToString( totalCalls[i] ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", TimeToString( totalCalls[i] ? (int64_t)( totalTimeNs[i] / totalCalls[i] ) : 0 ) );
                }
                if( idleCount > 0 )
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, 0xFF404040 );
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( "Idle" );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", TimeToString( (int64_t)totalIdleTimeNs ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", RealToString( idleCount ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", TimeToString( (int64_t)( totalIdleTimeNs / idleCount ) ) );
                }
                if( mainThreadHandoffCount > 0 )
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, 0xFF404040 );
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( "Main thread handoff" );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", TimeToString( (int64_t)totalMainThreadHandoffTimeNs ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", RealToString( mainThreadHandoffCount ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", TimeToString( (int64_t)( totalMainThreadHandoffTimeNs / mainThreadHandoffCount ) ) );
                }
                if( serverQuerySendCount > 0 )
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor( ImGuiTableBgTarget_RowBg0, 0xFF404040 );
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( "Server query send" );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", TimeToString( (int64_t)totalServerQuerySendTimeNs ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", RealToString( serverQuerySendCount ) );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%s", TimeToString( (int64_t)( totalServerQuerySendTimeNs / serverQuerySendCount ) ) );
                }
                ImGui::EndTable();
            }
            ImGui::Dummy( ImVec2( 0, 8 ) );
        }
        ImGui::EndChild();
        TextFocused( "Bytes waiting for recv:", MemSizeToString( m_worker.GetSocketRecvQueueBytes() ) );
    }

    if( !m_sendQueueWarning.enabled )
    {
        if( !m_sendQueueWarning.monitor )
        {
            if( sendQueue > SendQueueEnableThreshold )
            {
                m_sendQueueWarning.monitor = true;
                m_sendQueueWarning.time = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
            }
        }
        else
        {
            if( sendQueue < SendQueueDisableThreshold )
            {
                m_sendQueueWarning.monitor = false;
            }
            else
            {
                const auto t = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
                if( t - m_sendQueueWarning.time > SendQueueTimespanMs )
                {
                    m_sendQueueWarning.enabled = true;
                    m_sendQueueWarning.monitor = false;
                }
            }
        }
    }

    FrameImage lastFrameImage{};
    {
        Worker::MainThreadDataLockGuard lock = m_worker.ObtainLockForMainThread();
        ImGui::SameLine();
        TextFocused( "+", RealToString( m_worker.GetSendInFlight() ) );
        const auto sz = m_worker.GetFrameCount( *m_frames );
        if( sz > 1 )
        {
            const auto dt = m_worker.GetFrameTime( *m_frames, sz - 2 );
            const auto fps = 1000000000.f / dt;
            TextDisabledUnformatted( "FPS:" );
            ImGui::SameLine();
            ImGui::Text( "%6.1f", fps );
            ImGui::SameLine();
            TextFocused( "Frame time:", TimeToString( dt ) );
        }        
        const auto& fis = m_worker.GetFrameImages();
        // Keep a copy here since the worker may modify the frame images vector while we do not own the lock
        if( !fis.empty() ) lastFrameImage = *fis.back();
    }

    if( lastFrameImage.ptr.get() )
    {
        ImGui::Separator();
        DrawFrameImage( m_FrameTextureCacheConnection, lastFrameImage, scale * 0.5f );
    }

    ImGui::Separator();
    if( ImGui::Button( ICON_FA_FLOPPY_DISK " Save trace" ) && m_saveThreadState.load( std::memory_order_relaxed ) == SaveThreadState::Inert )
    {
        auto cb = [this]( const char* fn ) {
            const auto sz = strlen( fn );
            if( sz < 7 || memcmp( fn + sz - 6, ".tracy", 6 ) != 0 )
            {
                char tmp[1024];
                sprintf( tmp, "%s.tracy", fn );
                m_filenameStaging = tmp;
            }
            else
            {
                m_filenameStaging = fn;
            }
        };

#ifndef TRACY_NO_FILESELECTOR
        Fileselector::SaveFile( "tracy", "Tracy Profiler trace file", cb );
#else
        cb( "trace.tracy" );
#endif
    }

    ImGui::SameLine( 0, 2 * ty );
    const char* stopStr = ICON_FA_PLUG " Stop";
    Worker::MainThreadDataLockGuard lock = m_worker.ObtainLockForMainThread();
    if( !m_disconnectIssued && m_worker.IsConnected() )
    {
        if( ImGui::Button( stopStr ) )
        {
            m_worker.Disconnect();
            m_disconnectIssued = true;
        }
    }
    else
    {
        ImGui::BeginDisabled();
        ImGui::Button( stopStr );
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if( ImGui::Button( ICON_FA_TRIANGLE_EXCLAMATION " Discard" ) )
    {
        ImGui::OpenPopup( "Confirm trace discard" );
    }

    if( ImGui::BeginPopupModal( "Confirm trace discard", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        ImGui::PushFont( g_fonts.normal, FontBig );
        TextCentered( ICON_FA_TRIANGLE_EXCLAMATION );
        ImGui::PopFont();
        ImGui::TextUnformatted( "All unsaved profiling data will be lost!" );
        ImGui::TextUnformatted( "Are you sure you want to proceed?" );
        ImGui::Separator();
        if( ImGui::Button( "Yes" ) )
        {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return false;
        }
        ImGui::SameLine();
        if( ImGui::Button( "Reconnect" ) )
        {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            m_reconnectRequested = true;
            return false;
        }
        ImGui::SameLine( 0, ty * 2 );
        if( ImGui::Button( "No", ImVec2( ty * 6, 0 ) ) )
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if( m_worker.IsConnected() )
    {
        const auto& params = m_worker.GetParameters();
        if( !params.empty() )
        {
            ImGui::Separator();
            if( ImGui::TreeNode( "Trace parameters" ) )
            {
                if( ImGui::BeginTable( "##traceparams", 2, ImGuiTableFlags_Borders ) )
                {
                    ImGui::TableSetupColumn( "Name" );
                    ImGui::TableSetupColumn( "Value", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize );
                    ImGui::TableHeadersRow();
                    size_t idx = 0;
                    for( auto& p : params )
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted( m_worker.GetString( p.name ) );
                        ImGui::TableNextColumn();
                        ImGui::PushID( idx );
                        switch( p.type )
                        {
                        case ParameterType::Boolean:
                        {
                            bool val = p.val;
                            if( ImGui::Checkbox( "", &val ) )
                            {
                                m_worker.SetParameter( idx, int32_t( val ) );
                            }
                            break;
                        }
                        case ParameterType::Integer:
                        {
                            auto val = int( p.val );
                            ImGui::SetNextItemWidth( 100 * GetScale() );
                            if( ImGui::InputInt( "", &val, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue ) )
                            {
                                m_worker.SetParameter( idx, int32_t( val ) );
                            }
                            break;
                        }
                        case ParameterType::Trigger:
                            if( ImGui::Button( ICON_FA_CIRCLE_DOT ) )
                            {
                                m_worker.SetParameter( idx, p.val );
                            }
                            break;
                        }
                        ImGui::PopID();
                        idx++;
                    }
                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
        }
    }

    return true;
}

}
