#!/bin/bash
set -e

rm -rf $DEST
mkdir -p $DEST

#### copy exe ####
cp $GITHUB_WORKSPACE/build/FreeLink.exe $DEST
cp $GITHUB_WORKSPACE/build/FreeLink.pdb $DEST || true
cp $GITHUB_WORKSPACE/updater/updater.exe $DEST || true

cd download-artifact
cd *$DEST_SUFFIX
tar xvzf artifacts.tgz -C ../../
cd ../..
