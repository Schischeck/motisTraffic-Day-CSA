# Minimal Build Instructions on Ubuntu 18.04

This command sequence produces a minimal build of the main MOTIS binary with no modules enabled on a clean Ubuntu 18.04 system:

```bash
#!/bin/bash

# Install dependencies from package manager
apt update && apt upgrade -y && apt install -y g++ cmake git zlib1g-dev ninja-build

# Build own boost
cd ~
wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2
tar xf boost_1_67_0.tar.bz2
rm boost_1_67_0.tar.bz2
cd boost_1_67_0
./bootstrap.sh
./b2 -j6 link=static threading=multi variant=release  \
        --with-system \
        --with-filesystem \
        --with-iostreams \
        --with-program_options \
        --with-thread \
        --with-date_time \
        --with-regex \
        --with-serialization \
        -s NO_BZIP2=1

# Clone and build MOTIS (requires SSH-key!)
cd ~
git clone --recursive -y git@git.motis-project.de:motis/motis.git
cd motis && mkdir build && cd build
CXXFLAGS="-march=native" BOOST_ROOT=~/boost_1_67_0 cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DMODULES="" ..
ninja
````