#
# 1) Build stage: compile custom llvm-runtimes apk with -fPIC
#
FROM alpine:3.21 AS build-llvm-runtimes

RUN apk update && apk add --no-cache \
    alpine-sdk \
    linux-headers \
    python3 \
    samurai \
    clang \
    clang-dev \
    llvm19-dev \
    llvm19-static \
    cmake \
    make \
    tar \
    wget

# Create abuild user
RUN adduser -D builder \
 && addgroup builder abuild \
 && echo "PACKAGER=\"builder <builder@local>\"" >> /etc/abuild.conf \
 && su builder -c "abuild-keygen -an" \
 && cp /home/builder/.abuild/*.pub /etc/apk/keys/

WORKDIR /home/builder

# Download only the llvm-runtimes directory from aports
RUN wget -O- 'https://gitlab.alpinelinux.org/alpine/aports/-/archive/3.21-stable/aports-3.21-stable.tar.gz?path=main/llvm-runtimes' \
    | tar -xvzf -

# Permit 'builder' user to write everything
RUN chown -R builder:abuild /home/builder

USER builder

# Go to the llvm-runtimes folder with the APKBUILD
WORKDIR /home/builder/aports-3.21-stable-main-llvm-runtimes/main/llvm-runtimes

# Add -fPIC
RUN sed -i '/^options=/a CFLAGS="-fPIC"' APKBUILD
RUN sed -i '/^options=/a CXXFLAGS="-fPIC"' APKBUILD

# Build the packages
RUN abuild checksum && abuild -r

#
# 2) Final stage: install our newly built packages
#
FROM alpine:3.21

RUN apk update && apk add --no-cache \
    clang clang-extra-tools \
    cmake make musl-dev \
    libc++-dev libc++-static \
    lld \
    wget

# Copy only the x86_64 build artifacts from the first stage
COPY --from=build-llvm-runtimes /home/builder/packages/main/x86_64 /tmp/packages

# Install them
RUN apk add --allow-untrusted /tmp/packages/*.apk

WORKDIR /app

CMD ["sh", "-c", "mkdir -p build && cd build && \
  cmake .. \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_FLAGS='-target x86_64-alpine-linux-musl -B/usr/lib/gcc/x86_64-alpine-linux-musl/14.2.0 -fPIC' \
    -DCMAKE_CXX_FLAGS='-stdlib=libc++ -target x86_64-alpine-linux-musl -B/usr/lib/gcc/x86_64-alpine-linux-musl/14.2.0 -fPIC' \
    -DCMAKE_SHARED_LINKER_FLAGS='-fuse-ld=lld -L/usr/lib/gcc/x86_64-alpine-linux-musl/14.2.0 -static -Wl,--no-undefined -Wl,/usr/lib/libc++.a -Wl,/usr/lib/libc++abi.a -Wl,--no-whole-archive' \
    -DCMAKE_BUILD_TYPE=Release && \
  make -j$(nproc)"]
