#!/usr/bin/env bash

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  export CC=gcc-5
  export CXX=g++-5
  export DISPLAY=:99.0
  sh -e /etc/init.d/xvfb start +extension RANDR;
  sleep 3

  sudo apt-key adv --keyserver pgp.mit.edu --recv D101F7899D41F3C3
  echo "deb http://dl.yarnpkg.com/debian/ stable main" | sudo tee /etc/apt/sources.list.d/yarn.list
  sudo apt-get update && sudo apt-get install yarn

  curl -O https://dripcap.org/storage/libpcap-1.7.4.tar.gz
  tar xzf libpcap-1.7.4.tar.gz
  (cd libpcap-1.7.4 && ./configure -q --enable-shared=no && make -j2 && sudo make install)
fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  curl -o- -L https://yarnpkg.com/install.sh | bash
  export PATH="$HOME/.yarn/bin:$PATH"
fi

yarn global add node-gyp mocha
yarn
npm install .

for i in {1..10}; do npm test; test $? -ne 0 && exit 1; done
exit 0
