#!/bin/bash

# TODO: Figure out how to include SDL2 in this bundle

platform='unknown'
unamestr=`uname`

if [[ "$unamestr" == 'Linux' ]]; then
	platform='linux'
elif [[ "$unamestr" == 'Darwin' ]]; then
	platform='macos'
elif [[ "$unamestr" == 'FreeBSD' ]]; then
	platform='freebsd'
fi

cmake . -DCMAKE_BUILD_TYPE=Release
make

date=$(date +"%Y-%m-%d")
git=$(git rev-parse --verify HEAD | cut -c1-8)
bundlename=$date-$git-$platform

mkdir $bundlename
mkdir $bundlename/bin
mkdir $bundlename/output
cp ./bin/c-ray $bundlename/bin
cp -r ./input $bundlename/