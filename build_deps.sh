#!/bin/sh

set -e

mkdir -p deps
cd deps

wget -N https://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.14.tar.bz2
wget -N https://downloads.xiph.org/releases/opus/opus-1.5.2.tar.gz

tar xf alsa-lib-1.2.14.tar.bz2
tar xf opus-1.5.2.tar.gz

export CC=/opt/jetkvm-native-buildkit/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc

cd alsa-lib-1.2.14
./configure --host arm-rockchip830-linux-uclibcgnueabihf --enable-static=yes --enable-shared=no --with-pcm-plugins=rate,linear --disable-seq --disable-rawmidi --disable-ucm
make -j10
cd -

cd opus-1.5.2
./configure --host arm-rockchip830-linux-uclibcgnueabihf --enable-static=yes --enable-shared=no --enable-fixed-point
make -j10
cd -

cd -
