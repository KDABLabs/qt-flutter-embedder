#!/bin/bash

ENGINE_VERSION=3.24.0

RELEASE_TAG=`gh release list --repo ardera/flutter-ci | grep $ENGINE_VERSION | awk '{print $2}'`

gh release download $RELEASE_TAG -p engine-x64-generic-debug_unopt.tar.xz --repo ardera/flutter-ci || exit 1
tar xvf engine-x64-generic-debug_unopt.tar.xz || exit 1
rm engine-x64-generic-debug_unopt.tar.xz
