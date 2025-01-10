#!/bin/bash

set -e

ENGINE_VERSION=`flutter --version | grep -oP '^Flutter \K[^\s]+'`

echo "Fetching engine version ${ENGINE_VERSION}..."

# Get 2nd to last field, which is the release tag
RELEASE_TAG=`gh release list --repo ardera/flutter-ci | grep $ENGINE_VERSION | awk '{print $(NF-1)}'`

echo "Release tag is ${RELEASE_TAG}"

mkdir -p engine_binaries/rel
mkdir -p engine_binaries/dbg_unopt

gh release download $RELEASE_TAG -p engine-x64-generic-debug_unopt.tar.xz --repo ardera/flutter-ci
gh release download $RELEASE_TAG -p engine-x64-generic-release.tar.xz --repo ardera/flutter-ci

tar xvf engine-x64-generic-debug_unopt.tar.xz
mv libflutter_engine.so engine_binaries/dbg_unopt

tar xvf engine-x64-generic-release.tar.xz
mv libflutter_engine.so engine_binaries/rel

rm engine-x64-generic-debug_unopt.tar.xz
rm engine-x64-generic-release.tar.xz
