#!/bin/bash

set -e
#如果没有build目录则创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi
rm -rf `pwd`/build/*

cd `pwd`/build && cmake .. && make



cd ../src
if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi 

for header in `ls *.hpp`
do 
    cp $header /usr/include/mymuduo
done


#回到主目录
cd ..   
cp lib/libmymuduo.so /usr/local/lib

ldconfig
