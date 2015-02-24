
import numpy as np
from matplotlib.path import Path
from matplotlib.patches import PathPatch 
import matplotlib.pyplot as plt
import numpy as np
from dag_slicer.dag_slicer import Dag_Slicer 


class dagmc_slicer(Dag_Slicer):

    def create_slice(self):
        
        #clear old arrays so there isn't junk data in the way
        self.slice_x_pnts = np.array([])
        self.slice_y_pnts = np.array([])
        self.path_coding = np.array([], dtype='int')
        
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
            leg = ax.legend(patches, self.group_names, prop={'size':10}, loc=2, bbox_to_anchor=(1.05,1.), borderaxespad=0.)
            #create mapping of artist to legend entry
            self.legend_map = {}
            for legpatch, patch in zip(leg.get_patches(), patches):
                legpatch.set_picker(True)
                self.legend_map[legpatch] = patch

        #plot axis settings
        ax.autoscale_view()
        ax.set_aspect('equal')


    def show_slice(self):        
        cid = self.figure.canvas.mpl_connect('pick_event', self.onpick)
        self.pick_counter = 0 #hack until new release of matplotlib
        plt.show()


    def onpick(self,event):

        self.pick_counter += 1
        if 0 == self.pick_counter % 2: #again, part of the hack until new matplotlib release
            # on the pick event, find the orig line corresponding to the
            # legend proxy line, and toggle the visibility
            print "Picked item: ", event.artist

            legenditem = event.artist
            origpatch = self.legend_map[legenditem]
            print "This patch is : " , origpatch

            vis = not origpatch.get_visible()
            origpatch.set_visible(vis)
        
            if vis:
                legenditem.set_alpha(1.0)
            else:
                legenditem.set_alpha(0.2)

            self.figure.canvas.draw()

