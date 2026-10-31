# Maiserver

This is a WeChat (for Linux) plugin that converts the functions of the `舞萌|中二` service account into a RESTful API, supports low-power devices.

> Compatible WeChat Version: **4.1.13.26 (x86_64, arm64)**

> [!TIP]
> Maiserver is still in the very early stages; it may crash or encounter other issues. Feel free to file an issue.

- [What problem does it solve?](#what-problem-does-it-solve)
- [Pros and Cons](#pros-and-cons)
- [API Endpoints](#api-endpoints)
  - [Basic](#basic)
  - [Streaming](#streaming)
- [Installation](#installation)
- [Installation (manually)](#installation-manually)
  - [Compile Guidelines](#compile-guidelines)
  - [About the Cloud Proxy Device ID](#about-the-cloud-proxy-device-id)
  - [About Slim Mode](#about-slim-mode)
  - [Bwrap](#bwrap)
- [Available Environment Variables](#available-environment-variables)
- [License](#license)

## What problem does it solve?

If you think offering game features on a WeChat service account isn't a problem, then you've come to the wrong place.

- WeChat is actually very difficult to use, but I have no choice but to use it.
- The Wahlap server is sometimes unstable, but WeChat hasn't handled this very well.
- NET has been embedded in the WeChat browser, resulting in a combination of two clunky entities.
- If you need to quickly extract `MAID` or `AIMETOKEN`, this plugin can help you (no need to capture packets).

## Pros and Cons

- Supports unattended operation and provides RESTful + Streaming APIs.
- It does not rely on WMPF[^1], which will save a significant amount of memory.
- The Linux version of WeChat has difficulty detecting injections, so your account is *relatively* secure.
- Access the WeChat GUI via [noVNC](https://novnc.com) (Integrated inside the Docker image).

But,
- It cannot bypass WeChat's restriction that allows only one PC login at a time.
- Its goal is to inject into WeChat, not to reverse-engineer the protocol, so it still requires running the WeChat application.

> [!WARNING]
> I will not be held responsible for any consequences, including permanent account suspension. Please use at your own risk.

## API Endpoints

### Basic

The basic APIs are RESTful, and all API calls are blocking—they wait until a result is returned (or a timeout occurs). They are relatively simple, but may not be suitable in certain scenarios.

- `/api/v1/auth/qrcode` - Generate a Player QR Code

After calling this API, it will wait for 15 seconds. If a response is received from the WeChat server before the timeout, that response will be returned; otherwise, a timeout will be returned. If this API is called multiple times within 15 seconds, all requests will be merged, and the same result will ultimately be returned.

This API shares a timeout timer with the corresponding Streaming API, which means the actual wait time may be less than 15 seconds.

> [!NOTE]
> **Why is the timeout 15 seconds?**  
> According to the [WeChat Service Account development documentation](https://developers.weixin.qq.com/doc/service/guide/product/message/Passive_user_reply_message.html): *If the WeChat server does not receive a response within 5 seconds, it will disconnect and resend the request, retrying a total of 3 times.*

> [!WARNING]
> This API cannot guarantee that the received QR code remains usable; we recommend using the Streaming API.

```json
{
    "code": 0,
    "message": "Success.",
    "data": {
        "url": "https://wq.wahlap.net/qrcode/req/MAID...",
        "description": "把下方二维码对准机台扫描处，可用机台有【舞萌DX】和【中二节奏】\n有效期限 : 9/11 16:17",
        "maid": "...",
        "expires_in": 1789114620
    }
}
```

- `/api/v1/auth/aimetoken/maimai-dx` - Get the authorized URL used to log in to the maimai DX NET.
```json
{
    "code": 0,
    "message": "Success.",
    "data": {
        "url": "https://maimai.wahlap.com/maimai-mobile/?t=..."
    }
}
```

- `/api/v1/auth/aimetoken/chunithm` - Get the authorized URL used to log in to the CHUNITHM NET.
```json
{
    "code": 0,
    "message": "Success.",
    "data": {
        "url": "https://chunithm.wahlap.com/mobile/?t=..."
    }
}
```

> [!NOTE]
> **Important Notes Regarding Authorized URLs**
> 1. The client must support HTTP/2.
> 2. The browser's `User-Agent` must be set; otherwise, you will be blocked by TencentCloud EdgeOne for 30 minutes. 😅
> 3. Once a new session begins (by accessing an authorized URL and receiving a successful response), the old session will immediately expire.

 - User agent for WeChat 4.1.13.26 (WeChatAppEx 25806) on Linux

```
Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/144.0.0.0 Safari/537.36 NetType/WIFI MicroMessenger/7.0.20.1781(0x6700143B) WindowsWechat(0x63090a13) UnifiedPCLinuxWechat(0xf2741d1a) XWEB/25806 Flue
```

### Streaming

 - `/api/v1/stream`

The Streaming API complements the Basic API. It has three basic message types. `S` stands for Server, and `C` stands for Client.

 1. Command Message (C->S) 

To request the server to perform certain operations; currently, the path is identical to that in the Basic API.

```json
{
    "path": "/api/v1/...",
    "echo": 1919810 # non-zero, optional
}
```

 2. Reply Message (S->C)

The server responds to specific client messages; if the client message contains an “echo,” it will be included in the response. Please note that there is no guarantee that all client messages will receive a reply.

```json
{
    "type": "reply",
    "echo": 1919810, # optional
    "code": 0,
    "message": "...",
    "data": { ... }  # optional if code != 0
}
```

 3. Broadcast Message (S->C)

The server broadcasts messages to all connected clients.

```json
{
    "type": "broadcast",
    "sub_type": "...",
    "data": { ... }
}
```

Currently Existing Subtypes

 - `qrcode-received` The WeChat server has pushed a new player QR code.

#### Examples

```
connected to ws://127.0.0.1:8080/api/v1/stream
C->S| hello
S->C| { "type": "reply", "code": 3, "message": "Illegal request." }
C->S| { "path": "/api/v1/auth/qrcode" }
S->C| { "type": "broadcast", "sub_type": "qrcode_received", "data": { "url": "https://wq.wahlap.net/qrcode/req/MAID...", "description": "把下方二维码对准机台扫描处，可用机台有【舞萌DX】和【中二节奏】\n有效期限 : 10/31 02:35", "maid": "...", "expires_in": 1793385300 } }
C->S| { "path": "/api/v1/auth/aimetoken/maimai-dx", "echo": 114514 }
S->C| { "type": "reply", "echo": 114514, "code": 0, "message": "Success.", "data": { "url": "https://maimai.wahlap.com/maimai-mobile/?t=..." } }
>>> user tapped 'Player QR Code' twice in WeChat on their phone <<<
S->C| { "type": "broadcast", "sub_type": "qrcode_received", "data": { "url": "https://wq.wahlap.net/qrcode/req/MAID...", "description": "把下方二维码对准机台扫描处，可用机台有【舞萌DX】和【中二节奏】\n有效期限 : 11/31 03:35", "maid": "...", "expires_in": 1796067300 } }
S->C| { "type": "broadcast", "sub_type": "qrcode_received", "data": { "url": "https://wq.wahlap.net/qrcode/req/MAID...", "description": "把下方二维码对准机台扫描处，可用机台有【舞萌DX】和【中二节奏】\n有效期限 : 12/31 04:35", "maid": "...", "expires_in": 1798662900 } }
```

## Installation

The simplest and most recommended installation method is to use Docker Compose, which will automatically set up the environment for you and handle the details.

> Please follow the guidelines for your distribution to set up docker-compose.

Clone this repository:
```bash
git clone https://github.com/Redbeanw44602/maiserver.git && cd maiserver
```

Copy the `.env` file:
```bash
cp .env.example .env
```

By default, Maiserver listens on port `8080`, while noVNC (WeChat GUI) listens on port `6080`. You can change these settings as needed:

```
MAISERVER_PORT=8080
NOVNC_PORT=6080
```

> [!WARNING]
> Maiserver and noVNC listen on `127.0.0.1` by default because they are configured or designed not to support authentication. If you want to make these services available on the public internet, please use Nginx as a reverse proxy and configure authentication measures appropriately.

> [!CAUTION]
> Please do **NOT** attempt to expose ports 6080 or 8080 DIRECTLY to the public internet, as this will result in your account credentials being stolen!

<details>
<summary>Sample: Using Nginx as a reverse proxy and exposing the service to the public internet via Cloudflared</summary>

### Nginx

> Please follow the guidelines for your distribution to set up nginx.

The following example sets up a reverse proxy for Maiserver and noVNC on port `8081`. For Maiserver (`/`), it will verify whether the requester has the expected `X-Auth-Token` header, while for noVNC (`/novnc`), it will display a pop-up window prompting the user to enter a username and password.  

First, set the username and password for logging into noVNC:
```bash
# Please replace <USERNAME> and <PASSWORD>!
echo "<USERNAME>:$(openssl passwd -apr1 <PASSWORD>)" > /etc/nginx/.htpasswd
```

The following example does not have HTTPS configured and listens on `127.0.0.1`, as it will be used by Cloudflared next.
```nginx
# Please replace <TOKEN>!
# If you have modified the port mapping, please update it accordingly.
map $http_upgrade $connection_upgrade {
    default upgrade;
    ''      close;
}

server {
    listen 127.0.0.1:8081;

    absolute_redirect off;

    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection $connection_upgrade;
    proxy_set_header Host $host;
    proxy_set_header X-Real-IP $remote_addr;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_set_header X-Forwarded-Proto $scheme;

    rewrite ^/novnc/?$ /novnc/vnc.html permanent;

    location / {
        if ($http_x_auth_token != "<TOKEN>") {
            return 401;
        }
        proxy_pass http://127.0.0.1:8080;
    }

    location /novnc/ {
        auth_basic "noVNC";
        auth_basic_user_file /etc/nginx/.htpasswd; 
        proxy_pass http://127.0.0.1:6080/;
    }
}
```

Save the configuration to `/etc/nginx/conf.d/maiserver.conf`, then restart the nginx service.

### Cloudflared

> Please follow the guidelines for your distribution and [cloudflare docs](https://developers.cloudflare.com/tunnel/get-started/) to set up cloudflared.

Navigate to <ins>Protect & Connect</ins> > <ins>Networking</ins> > <ins>Tunnels</ins>, then <ins>Create Tunnel</ins>

<img width="1905" height="212" alt="image" src="https://github.com/user-attachments/assets/3eeb7b76-1913-4fe7-a55f-5a6c75e06a6b" />

Once the tunnel has been created + connected, navigate to <ins>Routes</ins> and then click <ins>Add route</ins>, select <ins>Published application</ins>

<img width="1316" height="887" alt="image" src="https://github.com/user-attachments/assets/d9dc6680-1ca4-4ddf-91c4-ed913cac22dc" />

Select a <ins>Domain</ins>, then enter a <ins>Subdomain</ins>, set `http://127.0.0.1:8081` as the <ins>Service URL</ins>.

<img width="1316" height="1196" alt="image" src="https://github.com/user-attachments/assets/f740c845-963b-435a-bdbb-89bf6765872c" />

Click <ins>Add route</ins>, and your service is now deployed to the public internet!

---

</details>

Start the container and automatically update it on every startup:
```bash
docker compose up --pull always
```

If you do not want automatic updates:
```bash
docker compose up
```

If everything is working properly, you should now be able to access the WeChat at: http://127.0.0.1:6080/vnc.html

Please log in to WeChat so you can use the API.

Stop the container:

> [!NOTE]
> WeChat data is persistently stored in the `maiserver_data` Docker volume.

```bash
docker compose down
```

## Installation (manually)

Please compile it yourself, or download [precompiled binaries](https://github.com/Redbeanw44602/maiserver/releases).

Simply set the `MAISERVER_CLOUD_PROXY_DEVICE_ID` environment variable and find a way to inject `libmaiserver.so` into `wechat`; `LD_PRELOAD` is supported.

The simplest startup command might look like this: please do not copy it directly! Refer to the text below to obtain the device ID.
```bash
export MAISERVER_CLOUD_PROXY_DEVICE_ID='E2NnKfY4zFAGg3gYpqcIPdFzlMigHitX8MIyXuyVXBI='
export LD_PRELOAD='/path/to/libmaiserver.so'
wechat
```

If Maiserver runs successfully, it will print something like the following to stdout:

```
Hello maiserver!
The device ID is set to: 13636729f638cc5006837818a6a7083dd17394c8a01e2b57f0c2325eec955c12
Starting to listen on 0.0.0.0:8080 ...
```

### Compile Guidelines

Maiserver uses the Meson build system and Conan to manage dependencies. To compile it yourself, you will need:
 - Conan 2
 - Compilers that support C++23 (`clang` is recommended)
 - Standard library that supports C++23 (`libstdc++` is recommended)

> **Technical Details**  
> WeChat uses at least two different configurations of the `libc++`, but Maiserver *should* handle this well. Therefore, the build of Maiserver is not affected by the standard library used by WeChat (See [mem/std_string.h](src/mem/std_string.h)).

First, select a [compilation profile](.conan2/profiles); for cross-compilation, you’ll need to select two.

Install Dependencies:
```bash
# conan install . -pr:a build-clang-libstdc++-debug -b missing -of=build/conan # native compiling
# conan install . -pr:b build-gcc-debug -pr:h host-gcc-debug-aarch64 -b missing -of=build/conan # cross compiling to aarch64
source build/conan/conanbuild.sh
```

Configure the project:
```bash
# meson setup build --native-file build/conan/conan_meson_native.ini # native compiling
# meson setup build --cross-file build/conan/conan_meson_cross.ini # cross compiling to aarch64
```

Compile the project:
```bash
meson compile -C build
```

### About the Cloud Proxy Device ID

This is a 32-byte ID **randomly generated** by WMPF to retrieve session information from WeChat server. If you still need to use WeChat, I don’t recommend generating one yourself; you can simply use the same ID as WMPF:

> [!TIP]
> Depending on the distribution and/or whether you are using bwrap, the data directory may vary; please refer to the notes below.

```bash
$ grep -r 'kTdiKeyDeviceId' ~/.xwechat
/home/neko/.xwechat/radium/ilink/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/wechat/cloud_account.txt:kTdiKeyDeviceId=E2NnKfY4zFAGg3gYpqcIPdFzlMigHitX8MIyXuyVXBI=
```

The content following `kTdiKeyDeviceId=` is what you need; please refer to the instructions above for setting the environment variable.

The device ID is generated once when you log in to WMPF. If you have disabled WMPF, you can generate it yourself:

```bash
$ openssl rand 32 | base64 -w 0
E2NnKfY4zFAGg3gYpqcIPdFzlMigHitX8MIyXuyVXBI=
```

### About Slim Mode

When slim mode is enabled, Maiserver will save as much memory as possible at the expense of WeChat functionality, making it suitable for running on low-power devices.

Maiserver has fully reverse-engineered the OAuth request process, so it can run by relying solely on the WeChat main process. Currently, slim mode blocks some child processes:

- WeChatAppEx (WMPF)
- wxocr
- wxutility
- wxplayer
- crashpad

```bash
export MAISERVER_SLIM=1
```

Memory usage for reference:

<img width="1716" height="814" alt="image" src="https://github.com/user-attachments/assets/ebf55971-0404-4cb6-8640-ce098b899942" />

### Bwrap

If you're running WeChat in bwrap. For example, I'm using [aur/wechat-universal-bwrap](https://aur.archlinux.org/packages/wechat-universal-bwrap), some things might be different:

- The data path depends on the mount point. Please check your startup script:
```bash
--bind "${WECHAT_HOME_DIR}" "${HOME}"
```
- Pass the environment variables to bwrap:
```bash
--ro-bind "/path/to/maiserver/" "/tmp/maiserver"
--setenv MAISERVER_CLOUD_PROXY_DEVICE_ID "E2NnKfY4zFAGg3gYpqcIPdFzlMigHitX8MIyXuyVXBI="
--setenv MAISERVER_SLIM "1"
--setenv LD_PRELOAD "/tmp/maiserver/libmaiserver.so"
```
- Maiserver requires HTTPS (cURL)
```bash
--ro-bind /etc/ssl/certs/ca-certificates.crt{,}
```

## Available Environment Variables

Environment Variable | Default Value | Description
-|-|-
`MAISERVER_LISTEN_ADDRESS` | `0.0.0.0` | The address that the web server listen on.
`MAISERVER_LISTEN_PORT` | `8080` | The port that the web server listen on.
`MAISERVER_CLOUD_PROXY_DEVICE_ID` | - | This variable must be set manually; see above for details.
`MAISERVER_SLIM` | `0` | Options for running with low memory; see above for details.

## License

GPLv3

[^1]: WMPF (WeChat Mini Program Framework) is a Chromium-based standalone process that is launched by WeChat.