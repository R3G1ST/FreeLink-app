#!/bin/bash
set -e

rm -rf $DEST
mkdir -p $DEST

#### copy binary ####
cp $GITHUB_WORKSPACE/build/FreeLink $DEST

#### copy FreeLink.png ####
cp $GITHUB_WORKSPACE/res/public/FreeLink.png $DEST

#### copy Core ####
cd download-artifact
cd *${DEST_SUFFIX%-system-qt}
tar xvzf artifacts.tgz -C ../../
cd ../..
cp deployment/${DEST_SUFFIX%-system-qt}/FreeLinkCore $DEST
rm -rf deployment/${DEST_SUFFIX%-system-qt}

# handle debug info
objcopy --only-keep-debug $DEST/FreeLink $DEST/FreeLink.debug
strip --strip-debug --strip-unneeded $DEST/FreeLink
objcopy --add-gnu-debuglink=$DEST/FreeLink.debug $DEST/FreeLink
