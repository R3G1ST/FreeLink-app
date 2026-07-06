#!/bin/bash
set -e

rm -rf $DEST
mkdir -p $DEST

cd download-artifact
cd *$DEST_SUFFIX
tar xvzf artifacts.tgz -C ../../
cd ../..

#### copy exe ####
cp $GITHUB_WORKSPACE/build/FreeLink.exe $DEST
cp $GITHUB_WORKSPACE/build/FreeLink.pdb $DEST || true
# Copy updater LAST to avoid overwrite by artifacts.tgz
cp $GITHUB_WORKSPACE/updater/updater.exe $DEST
ls -la $DEST/updater.exe
