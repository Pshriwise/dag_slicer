#!/usr/bin/python 

from itaps import iMesh, iBase
import argparse
from yt.utilities.lib.geometry_utils import triangle_plane_intersect
import numpy as np
from matplotlib.path import Path
from matplotlib.patches import PathPatch
import matplotlib.pyplot as plt
from dag_slicer import dag_slicer


import time

def parsing():
    """
    Sets up expected arguments when running the file as main. 
    """

    #Set help description
    description = 'This program is designed for the plotting of a watertight, DAGMC-ready .h5m file' 
    description += 'w/ triangular facets.'
    parser = argparse.ArgumentParser(description=description)
    
    #Filename argument
    parser.add_argument(
        '-f', action='store', dest='filename', required=True, help='The path to the .h5m file')

    # Indicates the axis along which to slice
    parser.add_argument('-axis', action='store', dest='axis', type=int,
                        help='Set axis along which to slice the model x=0, y=1, z=2 (default = x)')

    # Defines the coordinate of the plane along which to slice. 
    parser.add_argument('-coord', action='store', dest='coord', type=float,
                        help='Coordinate for the slice (default = 0)')
    
    # Indicates that the user wishes to plot groups as the same color
    parser.add_argument('--by-group', action='store_true', dest='by_group', 
                        help='Plot intersections by groups using the same color for each group')

    # Option for writing raw point data to file 
    parser.add_argument( '--write-pnts', action='store_true', dest='write_pnts', help = 'If set, the program will now write raw point data to file name "slicepnts.txt".')

    parser.set_defaults(axis = 0)
    parser.set_defaults(coord = 0)
    parser.set_defaults(by_group = False)
    parser.set_defaults(file_out = 'slicepnts.txt')
    parser.set_defaults(write_pnts = False)
    args = parser.parse_args()

    if not args.filename:
        raise Exception('h5m file path not specified!!. [-f] is not set')

    return args


def show_slice(filename, axis, coord, write_pnts=False, by_group=False):

    slicer = dag_slicer.Dag_Slicer(filename, axis, coord)
    slicer.create_slice(by_group)
    all_paths = []
    for i in range(len(slicer.slice_x_pnts)):
        new_list = [ np.transpose(np.vstack((slicer.slice_x_pnts[i],slicer.slice_y_pnts[i]))), slicer.path_coding[i]]
        all_paths.append(new_list)


    if __name__ == "__main__":
        print "Plotting..."

    file = open('slicepnts.txt', 'a')
    
    patches = []
    for coord, code in all_paths:
        if write_pnts: np.savetxt(file, coord, delimiter = ' ')
        path = Path(coord, code)
        color = np.random.rand(3, 1)
        patches.append(PathPatch(path, color=color, ec='black', lw=1, alpha=0.4))

        
    #create a new figure
    fig, ax = plt.subplots()

    #add the patches to the plot
    for patch in patches:
        ax.add_patch(patch)

   # if by_group:
   #     ax.legend(patches, group_names, prop={'size':10})
    #show the plot!
    ax.autoscale_view()
    ax.set_aspect('equal')
    plt.show()  

def main():

    #parse arguments and load the file
    args = parsing()

    print args.coord
    print args.axis
    #all_paths, group_names = slice_faceted_model(args.filename, args.coord, args.axis, args.by_group)

    show_slice(args.filename, args.axis, args.coord, args.write_pnts)

    
    
if __name__ == "__main__":
    start = time.clock()
    main()
    print "Took " + str((time.clock()-start)) + " seconds total."




