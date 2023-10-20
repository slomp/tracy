if ! which zig; then exit; fi

#zig build -Doptimize=ReleaseFast -Dtarget=x86_64-windows-msvc $@
#exit

export CC="zig cc"

# zig cc on Windows : -fno-lto is necessary at the moment to circumvent a linker error:
#                     "undefined symbol: __declspec(dllimport) _create_locale"

# need for loops because 'error: coff does not support linking multiple objects into one'

mkdir -p zig-out/obj
mkdir -p zig-out/bin

# ls not really necessary: for loop assumes iteration over files, and takes care of wildcards

TRACY_C_SOURCES=
TRACY_C_SOURCES+=" $(ls ../../src/ini.c)"
TRACY_C_SOURCES+=" $(ls ../../../zstd/dictBuilder/*.c)"
TRACY_C_SOURCES+=" $(ls ../../../zstd/decompress/*.c)"
TRACY_C_SOURCES+=" $(ls ../../../zstd/compress/*.c)"
TRACY_C_SOURCES+=" $(ls ../../../zstd/common/*.c)"
TRACY_C_SOURCES+=" $(ls ../../../zstd/decompress/huf_decompress_amd64.S)"
for f in ${TRACY_C_SOURCES}; do
    ${CC} -c $f -o ./zig-out/obj/$(basename -s .c $f).obj -O2 -target native-windows-msvc -Wno-deprecated-declarations
done

TRACY_CPP_SOURCES=
TRACY_CPP_SOURCES+=" $(ls ../../../nfd/nfd_win.cpp)"
TRACY_CPP_SOURCES+=" $(ls ../../src/*.cpp | grep -v BackendWayland.cpp)"
TRACY_CPP_SOURCES+=" $(ls ../../../server/*.cpp)"
TRACY_CPP_SOURCES+=" $(ls ../../../public/common/*.cpp)"
TRACY_CPP_SOURCES+=" $(ls ../../src/imgui/*.cpp)"
TRACY_CPP_SOURCES+=" $(ls ../../../imgui/*.cpp)"
TRACY_CPP_SOURCES+=" $(ls ../../../imgui/misc/freetype/*.cpp)"
for f in ${TRACY_CPP_SOURCES}; do
    ${CC} -c $f -o ./zig-out/obj/$(basename -s .cpp $f).obj -O2 -target native-windows-msvc \
    -Wno-deprecated-declarations -Wno-unused-command-line-argument \
    -std=c++17 -DNDEBUG -DNOMINMAX -DWIN32_LEAN_AND_MEAN -DIMGUI_ENABLE_FREETYPE \
    -I../../../imgui -I../../../vcpkg_installed/x64-windows-static/include -I../../../vcpkg_installed/x64-windows-static/include/capstone
done

${CC} \
    $(ls ./zig-out/obj/*.obj) \
    -o ./zig-out/bin/Tracy.exe \
    -target native-windows-msvc \
    -L../../../vcpkg_installed/x64-windows-static/lib \
    -lbrotlicommon -lbrotlidec -lfreetype -lglfw3 -llibpng16 -lzlib -lbz2 -lcapstone \
    -lws2_32 -lole32 -lgdi32 -lshell32 -ladvapi32 \
    -lc \
