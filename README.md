dag_slicer
==========

C++/python tool for sliced views of dagmc models

Requirements
============

python-numpy http://www.numpy.org/
python-xdress https://github.com/xdress/xdress/releases
python-matplotlib http://matplotlib.org/
MOAB http://sigma.mcs.anl.gov/moab-library/

Installation
============

1) Add your moab install directory to the LD_LIBRARY_PATH environment variable.

2) Run the bash ./build.sh scirpt from its directory in the repository. 

3) Run `python setup.py install --moab-path <moab_install_path>` to install the python module. 

Use
===

1) copy slice_tool.py into directory for intended use

2) open an ipython (or python) instance

3) use as seen in video here: https://www.youtube.com/watch?v=q1pBCIVjKUU

