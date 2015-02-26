
import numpy as np
from matplotlib.path import Path
from matplotlib.patches import PathPatch 
import matplotlib.pyplot as plt
import numpy as np
from dag_slicer.dag_slicer import Dag_Slicer 
from matplotlib.widgets import CheckButtons, RadioButtons


class dagmc_slicer(Dag_Slicer):

    #wrapper for the super init 
    def __init__(self, filename, axis = 0, coordinate = 0, by_group = False):
        
        super(dagmc_slicer, self).__init__( filename, axis, coordinate, by_group )
        self.shown = False
        
        
    def clear_slice(self):
        #clear old arrays so there isn't junk data in the way
        self.slice_x_pnts = np.array([])
        self.slice_y_pnts = np.array([])
        self.path_coding = np.array([], dtype='int')
        self.group_names = np.array([], dtype='str')
        self.shown = False 

    def create_slice(self):
        
        #clear out old info
        self.clear_slice()

        #clear old arrays so there isn't junk data in the way
        self.slice_x_pnts = np.array([])
        self.slice_y_pnts = np.array([])
        self.path_coding = np.array([], dtype='int')
        self.group_names = np.array([], dtype='str')

        #run the super function to create the slice
        super(dagmc_slicer, self).create_slice()
            
        #now setup the plot object
        all_paths = []
        for i in range(len(self.slice_x_pnts)):
            new_list = [ np.transpose(np.vstack((self.slice_x_pnts[i],self.slice_y_pnts[i]))), self.path_coding[i]]
            all_paths.append(new_list)

        #create the patches for this plot
        patches = []
        for coord, code in all_paths:
            path = Path(coord, code)
            color = np.random.rand(3, 1)
            patches.append(PathPatch(path, color=color, ec='black', lw=1))

                  
        #create a new figure
        fig, ax = plt.subplots()
        self.figure = fig
        self.plt_ax = ax
        
        
        #add the patches to the plot
        for patch in patches:
            ax.add_patch(patch)

        if self.by_group:
            leg = ax.legend(patches, self.group_names, prop={'size':14}, loc=2, bbox_to_anchor=(1.05,1.), borderaxespad=0.)
            #create mapping of artist to legend entry
            self.legend_map = {}
            for legpatch, patch in zip(leg.get_patches(), patches):
                legpatch.set_picker(True)
                self.legend_map[legpatch] = patch

        #plot axis settings
        ax.autoscale_view()
        ax.set_aspect('equal')


    def show_slice(self):        
        if self.shown or 0 == len(self.slice_x_pnts):
            self.create_slice()

        cid = self.figure.canvas.mpl_connect('pick_event', self.onpick)

        #setup the check boxex
        cax = plt.axes([0.025, 0.5, 0.12, 0.12])
        self.check = CheckButtons( cax, ('Visible','Filled'),(True,True) )
        self.check.visible = False
        self.check.on_clicked(self.visiblefunc)
        plt.show()
        self.shown = True


    def onpick(self,event):
        self.picked = event.artist
        #Reset all legend items to black then highlight current selection
        [a.set_edgecolor('black') for a in self.legend_map.keys()]
        event.artist.set_edgecolor('orange')

        #Get the patch item through the legend map and update the checkbox settings
        origpatch = self.legend_map[event.artist]
        [l.set_visible( origpatch.get_visible() ) for l in self.check.lines[0]]
        [l.set_visible( origpatch.get_fill() ) for l in self.check.lines[1]]
        #Redraw the plot
        self.figure.canvas.draw()

    def visiblefunc(self,label):
        #Check the current visibility/fill of the patch based
        #on the state of the check boxes
        vis = self.check.lines[0][0].get_visible()
        filled = self.check.lines[1][0].get_visible()
        #Reflect the changes to the patch in the legend item
        self.picked.set_alpha( 1.0 if vis else 0.6 )        
        self.picked.set_fill(filled)
        #Make changes to the original patch
        origpatch = self.legend_map[self.picked]
        origpatch.set_visible(vis)
        origpatch.set_fill(filled)
        #Redraw the plot
        self.figure.canvas.draw()
        
