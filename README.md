# FreeLink

Cross-platform GUI proxy utility, empowered by [Sing-box](https://github.com/SagerNet/sing-box)

Supports Windows 11/10/8/7 / Linux / MacOS out of the box.

> Fork of [Throne](https://github.com/throneproj/Throne) (Formerly Nekoray), customized and maintained as FreeLink.

### Note on MacOS releases
Apple platforms have a very strict security policy and since FreeLink does not have a signed certificate, you will have to remove the quarantine using `xattr -d com.apple.quarantine /path/to/freelink.app`. Also to get the built-in privilege escalation to work, `Terminal` should have the `Full Disk` access.

## Supported protocols

- SOCKS
- HTTP(S)
- Shadowsocks
- Trojan
- VMess
- VLESS
- TUIC
- Hysteria
- Hysteria2
- AnyTLS
- Mieru
- NaïveProxy
- Juicity
- TrustTunnel
- ShadowTLS
- Wireguard
- AmneziaWG
- SSH
- Xray VLESS
- Custom Outbound (Both Sing-box and Xray)
- Custom Config (Both Sing-box and Xray)
- Chaining outbounds
- Extra Core

## Subscription Formats

Various formats are supported, including share links, various JSON representation of Sing-box configs, and v2rayN link format as well as limited support for Shadowsocks and Clash formats.

Deeplinks are also supported, read the [documentation](https://freelinkproj.github.io/advanced/deeplinks/) for more information.

## Credits

- [SagerNet/sing-box](https://github.com/SagerNet/sing-box)
- [XTLS/Xray-core](https://github.com/xtls/xray-core)
- [Qv2ray](https://github.com/Qv2ray/Qv2ray)
- [Qt](https://www.qt.io/)
- [simple-protobuf](https://github.com/tonda-kriz/simple-protobuf)
- [fkYAML](https://github.com/fktn-k/fkYAML)
- [quirc](https://github.com/dlbeer/quirc)
- [QHotkey](https://github.com/Skycoder42/QHotkey)
- [srombauts/sqlitecpp](https://github.com/srombauts/sqlitecpp)

## FAQ

**Why does my Anti-Virus detect FreeLink and/or its Core as malware?** <br/>
FreeLink's built-in update functionality downloads the new release, removes the old files and replaces them with the new ones. Also the `System DNS` feature will change your system's DNS settings, which is considered a dangerous action by some Anti-Virus applications.

**Is setting the `SUID` bit really needed on Linux?** <br/>
To create and manage a system TUN interface, root access is required. Without it, you will have to grant the Core some `Cap_xxx_admin` and still need to enter your password 3 to 4 times per TUN activation. You can also opt to disable the automatic privilege escalation in `Basic Settings`->`Security`, but note that features that require root access will stop working unless you manually grant the needed permissions.

**Why does my internet stop working after I force quit FreeLink?** <br/>
If FreeLink is force-quit while `System proxy` is enabled, the process ends immediately and FreeLink cannot reset the proxy. <br/>
Solution:
- Always close FreeLink normally.
- If you force quit by accident, open FreeLink again, enable `System proxy`, then disable it — this will reset the settings.

## Credits

- [SagerNet/sing-box](https://github.com/SagerNet/sing-box)
- [XTLS/Xray-core](https://github.com/xtls/xray-core)
- [Throne (Original)](https://github.com/throneproj/Throne)
- [Qt](https://www.qt.io/)
