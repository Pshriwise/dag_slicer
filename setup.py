import os
import sys
import argparse
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

import numpy as np

if not os.path.exists('dag_slicer/xdress_extra_types.h'):
    sys.exit("please run xdress first!")


def get_moab_paths():

    argv = sys.argv[2:]
    
    for a in argv:
        sys.argv.remove(a)

    parser = argparse.ArgumentParser()
    
    parser.add_argument('--moab-path', type=str)
    parser.parse_args(argv)

    
    if 0 == len(argv):
        sys.exit("Please set the moab path using '--moab-path=<moab_install_dir>' ")

    if 2 < len(argv):
        sys.exit("Please provide only one moab installation.")

    #create moab include and library dirs 
    moab_inc = a + "/include/"
    moab_lib = a + "/lib/"

    return moab_inc, moab_lib



moab_include_path,  moab_library_path = get_moab_paths()

incdirs = [os.path.join(os.getcwd(), 'src'), np.get_include(), moab_include_path]
libdirs = [moab_library_path]
libs = ['MOAB']

py_modules = ['dagmc_slice_tool']

ext_modules = [
    Extension("dag_slicer.xdress_extra_types", ["dag_slicer/xdress_extra_types.pyx"], 
              include_dirs=incdirs, language="c++"),
    Extension("dag_slicer.dtypes", ["dag_slicer/dtypes.pyx"], 
              include_dirs=incdirs, language="c++"),
    Extension("dag_slicer.stlcontainers", ["dag_slicer/stlcontainers.pyx"], 
              include_dirs=incdirs, language="c++"),
    Extension("dag_slicer.dag_slicer", ['src/dag_slicer.cpp', 'src/slicer.cpp', "dag_slicer/dag_slicer.pyx", ],
    	      include_dirs=incdirs, library_dirs=libdirs, libraries=libs, language="c++"),
    ]

setup(  
  name = 'dag_slicer',
  cmdclass = {'build_ext': build_ext},
  ext_modules = ext_modules,
  py_modules = py_modules,
  packages = ['dag_slicer']
)




        
