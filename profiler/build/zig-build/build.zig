// this build.zig file was written targeting zig 0.11.0

// to ask zig for a build.zig file, use zig 'init-exe' (or init-lib)

// zig build "documentation"
// https://ziglang.org/documentation/0.11.0/std/#A;std:Build

const std = @import("std");

pub fn build(b: *std.Build) void
{
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});



    const exeTracyProfiler = b.addExecutable(.{
        .name = "Tracy",
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
    });

    exeTracyProfiler.defineCMacro("NDEBUG", null);  // won't zig do this for us already?
    exeTracyProfiler.defineCMacro("IMGUI_ENABLE_FREETYPE", null);
    //exeTracyProfiler.defineCMacro("NO_PARALLEL_SORT", null);
    if (target.isWindows())
    {
        exeTracyProfiler.defineCMacro("NOMINMAX", null);
        exeTracyProfiler.defineCMacro("WIN32_LEAN_AND_MEAN", null);
        //exeTracyProfiler.defineCMacro("_CRT_SECURE_NO_WARNINGS", null);
        //exeTracyProfiler.defineCMacro("_CRT_NONSTDC_NO_DEPRECATE", null);
        //exeTracyProfiler.defineCMacro("_USE_MATH_DEFINES", null);
    }

    exeTracyProfiler.addIncludePath(.{.path = "../../../imgui"});
    if (target.isWindows())
    {
        exeTracyProfiler.addIncludePath(.{.path = "../../../vcpkg_installed/x64-windows-static/include"});
        exeTracyProfiler.addIncludePath(.{.path = "../../../vcpkg_installed/x64-windows-static/include/capstone"});
    }

    exeTracyProfiler.addCSourceFiles(
        &[_][]const u8{
        // $(ls ../../src/ini.c)
            "../../src/ini.c",
        // $(ls ../../../zstd/dictBuilder/*.c)
            "../../../zstd/dictBuilder/cover.c",
            "../../../zstd/dictBuilder/divsufsort.c",
            "../../../zstd/dictBuilder/fastcover.c",
            "../../../zstd/dictBuilder/zdict.c",
        // $(ls ../../../zstd/decompress/*.c)
            "../../../zstd/decompress/huf_decompress.c",
            "../../../zstd/decompress/zstd_ddict.c",
            "../../../zstd/decompress/zstd_decompress.c",
            "../../../zstd/decompress/zstd_decompress_block.c",
        // $(ls ../../../zstd/compress/*.c)
            "../../../zstd/compress/fse_compress.c",
            "../../../zstd/compress/hist.c",
            "../../../zstd/compress/huf_compress.c",
            "../../../zstd/compress/zstd_compress.c",
            "../../../zstd/compress/zstd_compress_literals.c",
            "../../../zstd/compress/zstd_compress_sequences.c",
            "../../../zstd/compress/zstd_compress_superblock.c",
            "../../../zstd/compress/zstd_double_fast.c",
            "../../../zstd/compress/zstd_fast.c",
            "../../../zstd/compress/zstd_lazy.c",
            "../../../zstd/compress/zstd_ldm.c",
            "../../../zstd/compress/zstd_opt.c",
            "../../../zstd/compress/zstdmt_compress.c",
        // $(ls ../../../zstd/common/*.c)
            "../../../zstd/common/debug.c",
            "../../../zstd/common/entropy_common.c",
            "../../../zstd/common/error_private.c",
            "../../../zstd/common/fse_decompress.c",
            "../../../zstd/common/pool.c",
            "../../../zstd/common/threading.c",
            "../../../zstd/common/xxhash.c",
            "../../../zstd/common/zstd_common.c",
        // $(ls ../../../zstd/decompress/huf_decompress_amd64.S)
            "../../../zstd/decompress/huf_decompress_amd64.S",
        },
        &(.{})
    );

    exeTracyProfiler.addCSourceFiles(
        &[_][]const u8{
        // $(ls ../../../nfd/nfd_win.cpp)
            "../../../nfd/nfd_win.cpp",
        // $(ls ../../src/*.cpp | grep -v BackendWayland.cpp)
            "../../src/BackendGlfw.cpp",
            //"../../src/BackendWayland.cpp",   // Linux-only
            "../../src/ConnectionHistory.cpp",
            "../../src/Filters.cpp",
            "../../src/Fonts.cpp",
            "../../src/HttpRequest.cpp",
            "../../src/ImGuiContext.cpp",
            "../../src/IsElevated.cpp",
            "../../src/main.cpp",
            "../../src/ResolvService.cpp",
            "../../src/RunQueue.cpp",
            "../../src/WindowPosition.cpp",
            "../../src/winmain.cpp",
            "../../src/winmainArchDiscovery.cpp",
        // $(ls ../../../server/*.cpp)
            "../../../server/TracyBadVersion.cpp",
            "../../../server/TracyColor.cpp",
            //"../../../server/TracyEventDebug.cpp",    // not in Tracy.sln
            "../../../server/TracyFileselector.cpp",
            "../../../server/TracyFilesystem.cpp",
            "../../../server/TracyImGui.cpp",
            "../../../server/TracyMemory.cpp",
            "../../../server/TracyMicroArchitecture.cpp",
            "../../../server/TracyMmap.cpp",
            "../../../server/TracyMouse.cpp",
            "../../../server/TracyPrint.cpp",
            "../../../server/TracyProtoHistory.cpp",
            "../../../server/TracySourceContents.cpp",
            "../../../server/TracySourceTokenizer.cpp",
            "../../../server/TracySourceView.cpp",
            "../../../server/TracyStorage.cpp",
            "../../../server/TracyTaskDispatch.cpp",
            "../../../server/TracyTexture.cpp",
            "../../../server/TracyTextureCompression.cpp",
            "../../../server/TracyThreadCompress.cpp",
            "../../../server/TracyTimelineController.cpp",
            "../../../server/TracyTimelineItem.cpp",
            "../../../server/TracyTimelineItemCpuData.cpp",
            "../../../server/TracyTimelineItemGpu.cpp",
            "../../../server/TracyTimelineItemPlot.cpp",
            "../../../server/TracyTimelineItemThread.cpp",
            "../../../server/TracyUserData.cpp",
            "../../../server/TracyUtility.cpp",
            "../../../server/TracyView.cpp",
            "../../../server/TracyView_Annotations.cpp",
            "../../../server/TracyView_Callstack.cpp",
            "../../../server/TracyView_Compare.cpp",
            "../../../server/TracyView_ConnectionState.cpp",
            "../../../server/TracyView_ContextSwitch.cpp",
            "../../../server/TracyView_CpuData.cpp",
            "../../../server/TracyView_FindZone.cpp",
            "../../../server/TracyView_FrameOverview.cpp",
            "../../../server/TracyView_FrameTimeline.cpp",
            "../../../server/TracyView_FrameTree.cpp",
            "../../../server/TracyView_GpuTimeline.cpp",
            "../../../server/TracyView_Locks.cpp",
            "../../../server/TracyView_Memory.cpp",
            "../../../server/TracyView_Messages.cpp",
            "../../../server/TracyView_Navigation.cpp",
            "../../../server/TracyView_NotificationArea.cpp",
            "../../../server/TracyView_Options.cpp",
            "../../../server/TracyView_Playback.cpp",
            "../../../server/TracyView_Plots.cpp",
            "../../../server/TracyView_Ranges.cpp",
            "../../../server/TracyView_Samples.cpp",
            "../../../server/TracyView_Statistics.cpp",
            "../../../server/TracyView_Timeline.cpp",
            "../../../server/TracyView_TraceInfo.cpp",
            "../../../server/TracyView_Utility.cpp",
            "../../../server/TracyView_ZoneInfo.cpp",
            "../../../server/TracyView_ZoneTimeline.cpp",
            "../../../server/TracyWeb.cpp",
            "../../../server/TracyWorker.cpp",
        // $(ls ../../../public/common/*.cpp)
            "../../../public/common/tracy_lz4.cpp",
            "../../../public/common/tracy_lz4hc.cpp",
            "../../../public/common/TracySocket.cpp",
            "../../../public/common/TracyStackFrames.cpp",
            "../../../public/common/TracySystem.cpp",
        // $(ls ../../src/imgui/*.cpp)
            "../../src/imgui/imgui_impl_glfw.cpp",
            "../../src/imgui/imgui_impl_opengl3.cpp",
        // $(ls ../../../imgui/*.cpp)
            "../../../imgui/imgui.cpp",
            "../../../imgui/imgui_demo.cpp",
            "../../../imgui/imgui_draw.cpp",
            "../../../imgui/imgui_tables.cpp",
            "../../../imgui/imgui_widgets.cpp",
        // $(ls ../../../imgui/misc/freetype/*.cpp)
            "../../../imgui/misc/freetype/imgui_freetype.cpp",
        },
        // flags
        &(.{
            "-std=c++17",
            "-Wno-deprecated-declarations",
            "-Wunused-command-line-argument", // argument unused during compilation: '-nostdinc++'
            })
    );

    if (target.isWindows())
    {
        exeTracyProfiler.addLibraryPath(.{.path = "../../../vcpkg_installed/x64-windows-static/lib"});

        // workaround for : error: lld-link: could not open 'libLIBCMT.a': No such file or directory
        //exeTracyProfiler.addLibraryPath(.{.path = "C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Tools/MSVC/14.29.30133/lib/x64"});

        //exeTracyProfiler.addLibraryPath(.{.path = "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/ucrt/x64"});

        exeTracyProfiler.linkSystemLibrary("Ws2_32");
        exeTracyProfiler.linkSystemLibrary("Ole32");
        exeTracyProfiler.linkSystemLibrary("Gdi32");
        exeTracyProfiler.linkSystemLibrary("Shell32");
        exeTracyProfiler.linkSystemLibrary("Advapi32");
        //exeTracyProfiler.linkSystemLibrary("dbghelp");

        //exeTracyProfiler.linkSystemLibrary("legacy_stdio_definitions");
        //exeTracyProfiler.linkSystemLibrary("libcmt");
        //exeTracyProfiler.linkSystemLibrary("ucrt");
    }

    exeTracyProfiler.linkLibC();

    exeTracyProfiler.linkSystemLibrary("brotlicommon");
    exeTracyProfiler.linkSystemLibrary("brotlidec");
    exeTracyProfiler.linkSystemLibrary("freetype");
    exeTracyProfiler.linkSystemLibrary("glfw3");
    exeTracyProfiler.linkSystemLibrary("libpng16");
    exeTracyProfiler.linkSystemLibrary("zlib");
    exeTracyProfiler.linkSystemLibrary("bz2");
    exeTracyProfiler.linkSystemLibrary("capstone");

    if (!target.isWindows() or (target.abi != std.Target.Abi.msvc))
    {
        // libc++ not needed when using '-msvc' in target... (but libc still is!)
        exeTracyProfiler.linkLibCpp();
    }

    // exeTracyProfiler.addObjectFile(.{ .path = "./src-patch/windows-icon.res" });
   
    b.installArtifact(exeTracyProfiler);

    //std.debug.print("Target is: {}\n", .{ target });
    const native = std.zig.system.NativeTargetInfo.detect(target) catch unreachable;
    std.debug.print("Target is: {s}-{s}-{s}\n", .{@tagName(native.target.cpu.arch), @tagName(native.target.os.tag), @tagName(native.target.abi)});
    std.debug.print("Install path is: {s}\n", .{ b.getInstallPath(.prefix, "") });
}
