import os
import sys
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

import numpy as np

if not os.path.exists('slicer/xdress_extra_types.h'):
    sys.exit("please run xdress first!")

incdirs = [os.path.join(os.getcwd(), 'src'), np.get_include()]

ext_modules = [
    Extension("slicer.xdress_extra_types", ["slicer/xdress_extra_types.pyx"], 
              include_dirs=incdirs, language="c++"),
    Extension("slicer.dtypes", ["slicer/dtypes.pyx"], 
              include_dirs=incdirs, language="c++"),
    Extension("slicer.stlcontainers", ["slicer/stlcontainers.pyx"], 
              include_dirs=incdirs, language="c++"),
    Extension("slicer.dag_slicer", ['src/dag_slicer.cpp', "slicer/dag_slicer.pyx", ],
    	      include_dirs=incdirs, language="c++"),
    ]

setup(  
  name = 'slicer',
  cmdclass = {'build_ext': build_ext},
  ext_modules = ext_modules,
  packages = ['slicer']
)
