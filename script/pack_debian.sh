#!/bin/bash
set -e

VERSION="${1#v}"
ARCH="$2"

mkdir -p FreeLink/DEBIAN
mkdir -p FreeLink/opt
cp -r linux-$ARCH$([[ $3 == "systemqt" ]] && echo "-system-qt") FreeLink/opt
mv FreeLink/opt/linux-$ARCH$([[ $3 == "systemqt" ]] && echo "-system-qt") FreeLink/opt/FreeLink
rm FreeLink/opt/FreeLink/FreeLink.debug

# basic
cat >FreeLink/DEBIAN/control <<-EOF
Package: FreeLink
Version: $VERSION
Architecture: $ARCH
Maintainer: Mahdi Mahdi.zrei@gmail.com
Depends: desktop-file-utils$([[ $3 == "systemqt" ]] && echo ", libqt6core6, libqt6gui6, libqt6network6, libqt6widgets6, qt6-qpa-plugins, qt6-wayland, qt6-gtk-platformtheme, qt6-xdgdesktopportal-platformtheme, libxcb-cursor0, fonts-noto-color-emoji")
Description: Qt based cross-platform GUI proxy configuration manager (backend: sing-box)
EOF

cat >FreeLink/DEBIAN/postinst <<-EOF
cat >/usr/share/applications/FreeLink.desktop<<-END
[Desktop Entry]
Name=FreeLink
Comment=Qt based cross-platform GUI proxy configuration manager (backend: sing-box)
Exec=sh -c "PATH=/opt/FreeLink:\$PATH /opt/FreeLink/FreeLink -appdata"
Icon=/opt/FreeLink/FreeLink.png
Terminal=false
Type=Application
Categories=Network;Application;
END

update-desktop-database
EOF

sudo chmod 0755 FreeLink/DEBIAN/postinst

# desktop && PATH

sudo dpkg-deb --build FreeLink
