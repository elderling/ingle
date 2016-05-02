#!/bin/sh

set -e

mkdir -pv compiled
cd src/flashmenu && ./make.sh && cd ../..
cd src/sh && ./make.sh && cd ../.. && cp src/sh/sh compiled
sbcl --noinform --core bender/bender make.lisp
rm -fv g.zip compiled/charset.lst
cp -v README.md compiled
cd compiled && zip -r ../g.zip *
