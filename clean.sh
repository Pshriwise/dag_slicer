#!/bin/bash


if [ -f ./slicer/CMakeLists.txt ]; then
    mv slicer/CMakeLists.txt CMakeLists.txt.bak
fi

sudo rm -rf build 
rm -rf ./slicer
rm -rf ./CMakeListsFiles
rm -rf ./CMakeFiles
rm CMakeCache.txt
rm cmake_install.cmake
rm CTestTestfile.cmake

mkdir slicer 
mv CMakeLists.txt.bak ./slicer/CMakeLists.txt
