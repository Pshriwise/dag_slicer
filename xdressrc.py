from xdress.utils import apiname

package = 'dag_slicer'     # top-level python package name
packagedir = 'dag_slicer'  # loation of the python package

stlcontainers = [
    ('vector', ('vector', 'int')),
    ('vector', ('vector', 'float64')),
    ]

functions = [apiname('slice_faceted_model_out', ('src/slicer.cpp'),  
                     incfiles='slicer.hpp')]
