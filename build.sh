#!/bin/bash

#xdress the project
xdress

#create the cmake files
cmake .

#corrections to cython import locations
patch -p1 ./dag_slicer/dag_slicer.pyx < pyx_patch.txt 
patch -p1 ./dag_slicer/dag_slicer.pxd < pxd_patch.txt 

#build the package
make 

#package should now be ready to be installed
