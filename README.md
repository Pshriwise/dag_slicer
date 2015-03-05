dag_slicer
==========

C++/python tool for sliced views of dagmc models

Requirements
============

* python-numpy http://www.numpy.org/
* python-matplotlib http://matplotlib.org/
* MOAB (w/ hdf5) http://sigma.mcs.anl.gov/moab-library/

Installation
============

1) Add your moab install directory to the LD_LIBRARY_PATH environment variable.

2) Run `python setup.py install --user --moab-path <moab_install_path>` to install the python module. 

Use
=======

Please see the example in the examples dir and this video (https://www.youtube.com/watch?v=Z3f2PFKenhw&feature=youtu.be) for examples of use. 

Note: For garaunteed performance of this tool, all facet-based models should be made watertight before use.