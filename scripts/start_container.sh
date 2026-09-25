#!/usr/bin/env bash

export XDG_RUNTIME_DIR=/run/user/$(id -u)
export VNC_PORT=5900
export NOVNC_PORT=6080
export DISPLAY=:99

export MAIHOME=/maiserver
export DEVICE_ID_FILE=$HOME/.device_id

if [ ! -d "$XDG_RUNTIME_DIR" ]; then
    mkdir -p "$XDG_RUNTIME_DIR"
    chown $(id -u):$(id -g) "$XDG_RUNTIME_DIR"
    chmod 700 "$XDG_RUNTIME_DIR"
fi

if [[ ! -f "$DEVICE_ID_FILE" ]]; then
    mkdir -p "$(dirname "$DEVICE_ID_FILE")"
    openssl rand 32 | base64 -w 0 > "$DEVICE_ID_FILE"
fi

export MAISERVER_SLIM=1
export MAISERVER_CLOUD_PROXY_DEVICE_ID="$(cat "$DEVICE_ID_FILE")"

Xvfb $DISPLAY -screen 0 1280x800x24 &

x11vnc -display $DISPLAY -forever -shared -nopw -listen localhost -rfbport $VNC_PORT &

websockify --web /usr/share/novnc $NOVNC_PORT 0.0.0.0:$VNC_PORT &

LD_PRELOAD=$MAIHOME/libmaiserver.so wechat
