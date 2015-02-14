from xdress.utils import apiname

package = 'dag_slicer'     # top-level python package name
packagedir = 'dag_slicer'  # loation of the python package

#plugins = ('xdress.stlwrap', 'xdress.cythongen', 
#           'xdress.autodescribe', 'xdress.autoall')

extra_types = 'xdress_extra_types'

stlcontainers = [
    ('vector', 'str'),
    ('vector', 'int'),
    ('vector', ('vector', 'int')),
    ('vector', 'float'),
    ('vector', ('vector', 'float')),
    ]

classes = [apiname('Dag_Slicer', ('src/dag_slicer.hpp','src/dag_slicer.cpp'), incfiles = 'dag_slicer.hpp'),]


