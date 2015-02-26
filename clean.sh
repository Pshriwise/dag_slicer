#!/bin/bash


if [ -f ./dag_slicer/CMakeLists.txt ]; then
    mv dag_slicer/CMakeLists.txt CMakeLists.txt.bak
fi

rm -rf build 
rm -rf ./dag_slicer
rm -rf ./CMakeListsFiles
rm -rf ./CMakeFiles
rm CMakeCache.txt
rm cmake_install.cmake
rm CTestTestfile.cmake

mkdir dag_slicer 
mv CMakeLists.txt.bak ./dag_slicer/CMakeLists.txt
