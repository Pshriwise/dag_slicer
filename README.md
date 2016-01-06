dag_slicer
==========

C++/python tool for sliced views of dagmc models

Requirements
============

Slicer Tool:

* python-numpy http://www.numpy.org/
* python-matplotlib http://matplotlib.org/
* MOAB (w/ hdf5) http://sigma.mcs.anl.gov/moab-library/

Slicer Gui:

* iPython notebooks

Installation
============

1) Add your moab install directory to the LD_LIBRARY_PATH environment variable.

2) Run `python setup.py install --user --moab-path <moab_install_path>` to install the python module. 

Use
=======

Please see the example in the examples dir and this video playlist (https://www.youtube.com/playlist?list=PLa7CZUIEd-lRsDJgIVxppaYDqgS04hwfq) for examples of use.


Note: For garaunteed performance of this tool, all facet-based models should be made watertight before use.

Recent additions:

* Group names can now be altered using the `rename_group` method. Like so:

  ```
  slicer.rename_group(<group_id>,<new_group_name>)
  ```
* A new file can be exported using the slicer:

  ```
  # a warning will be issued if the new file name matches that of the one in use
  slicer.write_file(<new_filename>)
  ```
  
* A new GUI using iPython widgets. See the example in the examples folder.

**Note: GH will not fully render the notebook with widgets so they won't be visible unless the notebook is run locally.** 
 