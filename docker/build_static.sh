#!/bin/sh
TARGETARCH=$(apk info --print-arch)
GCC_VERSION=$(basename $(ls -1d /usr/lib/gcc/${TARGETARCH}-alpine-linux-musl/* 2>/dev/null | head -n 1))
rm -rf build
mkdir -p build
cd build

cmake .. \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_FLAGS="-target ${TARGETARCH}-alpine-linux-musl -B/usr/lib/gcc/${TARGETARCH}-alpine-linux-musl/${GCC_VERSION} -fPIC" \
    -DCMAKE_CXX_FLAGS="-stdlib=libc++ -target ${TARGETARCH}-alpine-linux-musl -B/usr/lib/gcc/${TARGETARCH}-alpine-linux-musl/${GCC_VERSION} -fPIC" \
    -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld -L/usr/lib/gcc/${TARGETARCH}-alpine-linux-musl/${GCC_VERSION} -static -Wl,--no-undefined -Wl,/usr/lib/libc++.a -Wl,/usr/lib/libc++abi.a -Wl,--no-whole-archive" \
    -DCMAKE_BUILD_TYPE=Release \
make -j
