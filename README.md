<p align="center">
  <img src="res/public/FreeLink.png" width="128" alt="FreeLink Logo"/>
</p>

<h1 align="center">FreeLink</h1>

<p align="center">Cross-platform GUI proxy utility, empowered by <a href="https://github.com/SagerNet/sing-box">Sing-box</a></p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-blue?style=for-the-badge" alt="Platform"/>
  <img src="https://img.shields.io/badge/License-GPL--3.0-green?style=for-the-badge" alt="License"/>
</p>

> Fork of [Throne](https://github.com/throneproj/Throne) (Formerly Nekoray), customized and maintained as FreeLink.

## Features

- **Multi-protocol**: SOCKS, HTTP(S), Shadowsocks, Trojan, VMess, VLESS, TUIC, Hysteria, Hysteria2, Wireguard, AmneziaWG, SSH, and more
- **Subscription support**: Share links, Sing-box JSON, v2rayN, Clash formats
- **Deeplinks**: Quick import via custom URL scheme
- **System proxy**: Automatic proxy configuration
- **TUN mode**: Full system traffic capture
- **Route profiles**: Downloadable rulesets for advanced routing

## Installation

### Windows
Download the latest `FreeLinkSetup.exe` from [Releases](https://github.com/R3G1ST/FreeLink-app/releases).

### Linux
Download the `.deb` or `.AppImage` package from [Releases](https://github.com/R3G1ST/FreeLink-app/releases).

### macOS
```bash
# Remove quarantine attribute
xattr -d com.apple.quarantine /path/to/FreeLink.app
```

## Supported Protocols

| Protocol | Status |
|----------|--------|
| SOCKS / HTTP(S) | ✅ |
| Shadowsocks | ✅ |
| Trojan | ✅ |
| VMess / VLESS | ✅ |
| TUIC | ✅ |
| Hysteria / Hysteria2 | ✅ |
| Wireguard / AmneziaWG | ✅ |
| SSH | ✅ |
| NaïveProxy / Juicity | ✅ |
| Custom Outbound | ✅ |

## Building from Source

### Requirements
- CMake 3.20+
- Qt 6.x or Qt 5.x
- Go 1.20+
- Protobuf compiler

### Build
```bash
git clone https://github.com/R3G1ST/FreeLink-app.git
cd FreeLink-app
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Credits

- [Throne (Original)](https://github.com/throneproj/Throne)
- [SagerNet/sing-box](https://github.com/SagerNet/sing-box)
- [XTLS/Xray-core](https://github.com/xtls/xray-core)
- [Qt](https://www.qt.io/)

## License

[GPL-3.0](LICENSE)
