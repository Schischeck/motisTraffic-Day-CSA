#!/bin/sh

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd $SCRIPT_DIR/..

npm install
node bundler/bundler.node.js

mkdir -p dist/fonts
cp fonts/* dist/fonts