# syntax=docker/dockerfile:1
ARG ALPINE_VERSION=3.21

##########################
# 1) Build stage: compile custom llvm-runtimes apk with -fPIC
##########################
FROM alpine:${ALPINE_VERSION} AS build-llvm-runtimes
ARG ALPINE_VERSION
ENV ALPINE_VERSION=${ALPINE_VERSION}

RUN apk update && apk add --no-cache \
    alpine-sdk \
    linux-headers \
    python3 \
    samurai \
    clang \
    clang-dev \
    llvm-dev \
    llvm-static \
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

# Download only the llvm-runtimes directory from aports for the specified Alpine version
RUN wget -O- "https://gitlab.alpinelinux.org/alpine/aports/-/archive/${ALPINE_VERSION}-stable/aports-${ALPINE_VERSION}-stable.tar.gz?path=main/llvm-runtimes" \
    | tar -xvzf -

# Permit 'builder' user to write everything
RUN chown -R builder:abuild /home/builder

USER builder

WORKDIR /home/builder/aports-$ALPINE_VERSION-stable-main-llvm-runtimes/main/llvm-runtimes

# Add -fPIC to CFLAGS and CXXFLAGS in APKBUILD
RUN sed -i '/^options=/a CFLAGS="-fPIC"' APKBUILD && \
    sed -i '/^options=/a CXXFLAGS="-fPIC"' APKBUILD

# Build the packages
RUN abuild checksum && abuild -r

RUN mv /home/builder/packages/main/$(apk info --print-arch) /home/builder/packages/main/packages

##########################
# 2) Final stage: install our newly built packages and build our app
##########################
FROM alpine:${ALPINE_VERSION}

RUN apk update && apk add --no-cache \
    clang clang-extra-tools \
    cmake make musl-dev \
    libc++-dev libc++-static \
    lld \
    wget

WORKDIR /app

COPY --from=build-llvm-runtimes /home/builder/packages/main/packages /tmp/packages

RUN apk add --allow-untrusted /tmp/packages/*.apk

COPY ./docker/build_static.sh /usr/local/bin/build.sh

RUN chmod +x /usr/local/bin/build.sh

CMD ["/usr/local/bin/build.sh"]
