# syntax=docker/dockerfile:1
FROM ubuntu:26.04

ARG TARGETARCH

ARG VERSION=dev-nightly
ARG BUILD_TYPE=release
ARG WECHAT_VERSION=4.1.13.9
ARG WECHAT_SHA256_x86_64=096865e050ba0d3c1a23887227e2400bf343037b1d7d658c84c88ff26bfdc17f
ARG WECHAT_SHA256_arm64=a6d115d24dfe3ed1b7e7de16cf6cc02acef8df5668150f702ac8d8c5256405fa
ARG OS_NAME=ubuntu26

ENV DEBIAN_FRONTEND=noninteractive

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    case "$TARGETARCH" in \
      amd64) ARCH=x86_64; WECHAT_SHA256=$WECHAT_SHA256_x86_64; ;; \
      arm64) ARCH=arm64; WECHAT_SHA256=$WECHAT_SHA256_arm64; ;; \
      *) echo "Unsupported architecture: $TARGETARCH" >&2; exit 1 ;; \
    esac \
 && apt-get update \
 && apt-get install -y --no-install-recommends \
        libglib2.0-0 libfontconfig1 libdbus-1-3 libnss3 libwayland-egl1 libegl1 libwayland-cursor0 libpulse0 \
        xvfb x11vnc novnc websockify \
        wget unzip \
 && wget -O wechat.deb https://github.com/Redbeanw44602/WeChatLinux/releases/download/${WECHAT_VERSION}/WeChatLinux_${ARCH}.deb \
 && wget -O maiserver.zip https://github.com/Redbeanw44602/maiserver/releases/download/${VERSION}/maiserver-${OS_NAME}-${ARCH}-${BUILD_TYPE}.zip \
 && echo "${WECHAT_SHA256} wechat.deb" | sha256sum -c - \
 && apt-get install -y ./wechat.deb \
 && unzip maiserver.zip \
 && mv maiserver-* maiserver \
 && apt-get purge -y --autoremove wget unzip \
 && rm -rf wechat.deb maiserver.zip \
 && rm -rf /var/lib/apt/lists/* \
 && rm -rf /opt/wechat/RadiumWMPF/runtime \
 && rm -rf /opt/wechat/wxutility \
 && rm -rf /opt/wechat/wxplayer \
 && rm -rf /opt/wechat/wxocr \
 && rm -rf /opt/wechat/libwxocr.so \
 && rm -rf /opt/wechat/wxutility

COPY --chmod=755 scripts/start_container.sh /start.sh

EXPOSE 8080 6080
VOLUME ["/root"]
CMD ["/start.sh"]
