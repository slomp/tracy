if ! which zig; then exit; fi

#zig build -Doptimize=ReleaseFast $@
#exit

export CC="zig cc"

# zig cc on MacOS : LTO is not yet supported with the Mach-O object format

mkdir -p zig-out/bin

${CC} \
    $(ls ../../src/ini.c) \
    $(ls ../../../nfd/nfd_cocoa.m) \
    $(ls ../../../zstd/dictBuilder/*.c) \
    $(ls ../../../zstd/decompress/*.c) \
    $(ls ../../../zstd/decompress/huf_decompress_amd64.S) \
    $(ls ../../../zstd/compress/*.c) \
    $(ls ../../../zstd/common/*.c) \
    $(ls ../../src/*.cpp | grep -v BackendWayland.cpp) \
    $(ls ../../../server/*.cpp) \
    $(ls ../../../public/common/*.cpp) \
    $(ls ../../src/imgui/*.cpp) \
    $(ls ../../../imgui/*.cpp) \
    $(ls ../../../imgui/misc/freetype/*.cpp) \
    -o zig-out/bin/Tracy-release \
    -O2 \
    -DNDEBUG -DIMGUI_ENABLE_FREETYPE \
    -I../../../imgui \
    -I/opt/homebrew/Cellar/glfw/3.3.8/include \
    -I/opt/homebrew/opt/freetype/include/freetype2 \
    -I/opt/homebrew/opt/libpng/include/libpng16 \
    -I/opt/homebrew/Cellar/capstone/5.0.1/include/capstone \
    -Wno-deprecated-declarations \
    -L/opt/homebrew/Cellar/glfw/3.3.8/lib \
    -L/opt/homebrew/opt/freetype/lib \
    -L/opt/homebrew/Cellar/capstone/5.0.1/lib \
    -lglfw -lfreetype -lcapstone -lpthread -ldl \
    -framework CoreFoundation -framework AppKit -framework UniformTypeIdentifiers \
    -lc++ \
