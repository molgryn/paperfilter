#!/usr/bin/env bash

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  export CC=gcc-5
  export CXX=g++-5
  export DISPLAY=:99.0
  sh -e /etc/init.d/xvfb start
  sleep 3

  export ELECTRON_VERSION=`jq .devDependencies.electron package.json -r`
  echo $ELECTRON_VERSION
fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  brew update
  brew install jq

  export ELECTRON_VERSION=`jq .devDependencies.electron package.json -r`
  echo $ELECTRON_VERSION

  mkdir ~/.electron
  curl -L -o ~/.electron/electron-v${ELECTRON_VERSION}-darwin-x64.zip https://github.com/electron/electron/releases/download/v${ELECTRON_VERSION}/electron-v${ELECTRON_VERSION}-darwin-x64.zip
  curl -L -o ~/.electron/SHASUMS256.txt-${ELECTRON_VERSION} https://github.com/electron/electron/releases/download/v${ELECTRON_VERSION}/SHASUMS256.txt
  npm install --depth 0 electron@${ELECTRON_VERSION}
fi

npm install --depth 0 -g node-gyp mocha electron@${ELECTRON_VERSION}
npm install --depth 0

make clean
make
