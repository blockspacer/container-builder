#!/bin/bash

set -e
set -o xtrace

VERSION=$1

export TOP_LEVEL=$(pwd)

source ${MODULESHOME}/init/bash
export PATH=$PATH:${MODULESHOME}/bin

module unload xalt
module load gcc
module load cmake/3.9.2

set -x

SW_ROOT=/sw/summitdev/container-builder/${VERSION}
mkdir -p ${SW_ROOT}

mkdir boost_install && cd boost_install

# Install boost
cd ${TOP_LEVEL}
rm -rf boost_build && mkdir boost_build && cd boost_build
curl -L https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz -O
tar xf boost_1_66_0.tar.gz
cd ${TOP_LEVEL}/boost_1_66_0
./bootstrap.sh --with-libraries=filesystem,regex,system,serialization,thread,program_options --prefix=${SW_ROOT}
./b2 install || :
rm -rf /boost_1_66_0

# Install container-builder
cd ${TOP_LEVEL}
rm -rf build && mkdir build && cd build
CC=gcc CXX=g++ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${SW_ROOT} ..
make
make install

# Generate a public modulefile
MF_ROOT=/sw/summitdev/modulefiles/container-builder
mkdir -p ${MF_ROOT}

# Grab latest queue host
QUEUE_HOST=$(curl https://code.ornl.gov/olcf/container-builder/raw/master/queue-host)

cat << EOF > ${MF_ROOT}/${VERSION}
#%Module

setenv QUEUE_HOST ${QUEUE_HOST}
setenv QUEUE_PORT 8080

module load gcc
prepend-path LD_LIBRARY_PATH ${SW_ROOT}/lib
prepend-path PATH ${SW_ROOT}/bin
EOF