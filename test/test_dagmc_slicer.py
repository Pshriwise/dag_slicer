
from subprocess import call,PIPE
from os.path import isfile
from dagmc_slice_tool import dagmc_slicer

def test_simple():
    #create a new slicer
    s = dagmc_slicer("../examples/teapot_grps_zip.h5m")

    s.create_slice()

    assert(len(s.slice_x_pnts) == 2)
    assert(len(s.slice_y_pnts) == 2)
    assert(len(s.group_names) == 0)

def test_w_groups():

    #create a new slicer
    s = dagmc_slicer("../examples/teapot_grps_zip.h5m")

    s.by_group = True
    s.create_slice()
 
    assert(len(s.slice_x_pnts) == 2)
    assert(len(s.slice_y_pnts) == 2)
    assert(len(s.group_names) == 2)
   
    
def test_change_file():
    #create a new slicer
    s = dagmc_slicer("../examples/teapot_grps_zip.h5m")

    s.create_slice()

    assert(len(s.slice_x_pnts) == 2)
    assert(len(s.slice_y_pnts) == 2)
    assert(len(s.group_names) == 0)

    s.filename = "./cube.h5m"

    s.clear_slice()

    s.create_slice()

    assert(len(s.slice_x_pnts) == 1)
    assert(len(s.slice_y_pnts) == 1)
    assert(len(s.group_names) == 0)
    
def test_no_groups():

    s = dagmc_slicer("./cube.h5m")

    s.by_group = True

    s.create_slice()

    assert(len(s.group_names) == 0)
    assert(len(s.slice_x_pnts) == 0)
    assert(len(s.slice_y_pnts) == 0)

def test_src():

    if isfile('test_slicer'):
        cmd = './test_slicer'
    else:
        cmd = 'cd ../src/ && make && cd ../test/ && make && ./test_slicer'
    assert(not call(cmd,shell=True, stdout=PIPE))
    
