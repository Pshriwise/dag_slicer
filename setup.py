import os
import sys
from subprocess import call, Popen, PIPE, STDOUT
import argparse
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

import numpy as np

def get_moab_paths():

    argv = sys.argv[-2:]
    
    for a in argv:
        sys.argv.remove(a)

    parser = argparse.ArgumentParser()
    
    parser.add_argument('--moab-path', dest='moab_path', required=True, type=str)
    args =  parser.parse_args(argv)
    
    #create moab include and library dirs 
    moab_inc = args.moab_path + "/include/"
    moab_lib = args.moab_path + "/lib/"

    return moab_inc,moab_lib



moab_include_path,moab_library_path = get_moab_paths()

incdirs = [os.path.join(os.getcwd(), 'src'), np.get_include(), moab_include_path]
print moab_library_path
libdirs = [moab_library_path]
libs = ['MOAB']

py_modules = ['src/dagmc_slice_tool','src/dagmc_slicer_gui','src/Dag_Slicer']

ext_modules = [
    Extension("_Dag_Slicer", ["src/dag_slicer_wrap.cxx","src/dag_slicer.cpp","src/slicer.cpp"], 
              include_dirs=incdirs, library_dirs=libdirs, libraries=libs, language="c++"),
    ]

setup(  
  name = 'dag_slicer',
  cmdclass = {'build_ext': build_ext},
  ext_modules = ext_modules,
  py_modules = py_modules,
)




        
