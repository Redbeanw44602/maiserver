# Maiserver

This is a WeChat (for Linux) plugin that converts the functions of the `舞萌|中二` service account into a RESTful API, supports low-power devices.

> Compatible WeChat Version: **4.1.13.9 (x86_64, arm64)**

> [!TIP]
> Maiserver is still in the very early stages; it may crash or encounter other issues. Feel free to file an issue.

### TODOs

- [ ] Headless mode & Docker image.

## What problem does it solve?

If you think offering game features on a WeChat service account isn't a problem, then you've come to the wrong place.

- WeChat is actually very difficult to use, but I have no choice but to use it.
- The WAHLAP server is sometimes unstable, but WeChat hasn't handled this very well.
- AIME has been embedded in the WeChat browser, resulting in a combination of two clunky entities.
- In addition, if you need to quickly extract `MAID` or `AIMETOKEN`, this plugin can help you (no need to capture packets).

## Pros and Cons

- Supports unattended operation and provides RESTful endpoints.
- It does not rely on WMPF[^1], which will save a significant amount of memory.
- The Linux version of WeChat has difficulty detecting injection attacks, so your account is (relatively) secure.

But,
- It cannot bypass WeChat's restriction that allows only one PC login at a time.
- Its goal is to inject into WeChat, not to reverse-engineer the protocol, so it still requires running the full WeChat application.

> [!WARNING]
> I will not be held responsible for any consequences, including permanent account suspension. Please use at your own risk.

## API Endpoints

Currently, all APIs are non-reentrant and block until a timeout occurs.

- `/api/v1/auth/qrcode` - Generate a Player QR Code
> [!NOTE]
> If the Wahlap server does not respond, WeChat will stop waiting after 15 seconds.
```json
{
    "code": 0,
    "message": "Success.",
    "data": {
        "url": "https://wq.wahlap.net/qrcode/req/MAID ...",
        "description": "把下方二维码对准机台扫描处，可用机台有【舞萌DX】和【中二节奏】\n有效期限 : 9/11 16:17",
        "maid": "...",
        "expires_in": 1789114620
    }
}
```

- `/api/v1/auth/aimetoken` - Get the token URL used to log in to the aime website.
> [!NOTE]
> The client must support HTTP/2.  
> The server will Set-Cookie for the client and redirect it to https://maimai.wahlap.com/maimai-mobile/home/.
```json
{
    "code": 0,
    "message": "Success.",
    "data": {
        "url": "https://maimai.wahlap.com/maimai-mobile/?t=..."
    }
}
```

## Installation

Recommended to compile it yourself, or download [precompiled binaries](https://github.com/Redbeanw44602/maiserver/releases) to obtain `libmaiserver.so`.

Simply set the `MAISERVER_CLOUD_PROXY_DEVICE_ID` environment variable and find a way to inject it into `wechat`; `LD_PRELOAD` is supported.

The simplest startup command might look like this: please do not copy it directly! Refer to the text below to obtain the device ID.
```bash
export MAISERVER_CLOUD_PROXY_DEVICE_ID='E2NnKfY4zFAGg3gYpqcIPdFzlMigHitX8MIyXuyVXBI='
export LD_PRELOAD='/path/to/libmaiserver.so'
wechat
```

If Maiserver runs successfully, it will print the following to stdout:

```
Hello maiserver!
The device ID is set to: 13636729f638cc5006837818a6a7083dd17394c8a01e2b57f0c2325eec955c12
Starting to listen on 0.0.0.0:8080 ...
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

When slim mode is enabled, Maiserver will conserve as much memory as possible at the expense of WeChat functionality, making it suitable for running on low-power devices.

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
`MAISERVER_SLIM` | `0` | Save as much memory as possible; see above for details.

## License

GPLv3

[^1]: WMPF (WeChat Mini Program Framework) is a Chromium-based standalone process that is launched by WeChat.