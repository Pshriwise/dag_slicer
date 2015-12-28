
import numpy as np
from matplotlib.path import Path
from matplotlib.patches import PathPatch 
import matplotlib.pyplot as plt
import numpy as np
from dag_slicer.dag_slicer import Dag_Slicer 
from matplotlib.widgets import CheckButtons, RadioButtons
from IPython.html import widgets
from IPython.display import display
from matplotlib.colors import rgb2hex

class dagmc_slicer(Dag_Slicer):

    #wrapper for the super init 
    def __init__(self, filename = "", axis = 0, coordinate = 0, by_group = False):
        
        super(dagmc_slicer, self).__init__( filename, axis, coordinate, by_group )
        self.shown = False
        self.color_seed = 56
        
    def clear_slice(self):
        #clear old arrays so there isn't junk data in the way
        self.slice_x_pnts = np.array([])
        self.slice_y_pnts = np.array([])
        self.path_coding = np.array([], dtype='int')
        self.group_names = np.array([], dtype='str')
        self.group_ids = np.array([],dtype='int')
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
        if str(self.filename) is not "":
            super(dagmc_slicer, self).create_slice()
            
    def rename_group(self, id, new_name):
        super(dagmc_slicer, self).rename_group(id, new_name)

    def write_file(self, new_filename):
        if new_filename == self.filename:
            continue_result = self.continue_query("Continuing will overwrite the current file. Continue?")
            if  not continue_result:
                print "Ok. Doing nothing."
                return
        super(dagmc_slicer, self).write_file(new_filename)

    def continue_query(self, question):
        reply = str(raw_input(question + ' (y/n) :')).lower().strip()

        if reply == 'y':
            return True
        if reply == 'n':
            return False
        else:
            print "Please reply with either y or n."
            self.continue_query(question)

    def show_slice(self, colors=None):        

        if 0 == len(self.slice_x_pnts):
            self.create_slice()


        #now setup the plot object
        all_paths = []
        for i in range(len(self.slice_x_pnts)):
            new_list = [ np.transpose(np.vstack((self.slice_x_pnts[i],self.slice_y_pnts[i]))), self.path_coding[i]]
            all_paths.append(new_list)

        if colors == None:
            colors = []
            np.random.seed(self.color_seed)
            for i in range(len(all_paths)):
                colors.append(np.random.rand(3,).tolist())
        elif len(colors) != len(all_paths):
            raise ValueError("{} colors are required, {} colors have been specified".format(
                             len(colors), len(all_paths)))

        #create the patches for this plot
        patches = []
        for i, (coord, code) in enumerate(all_paths):
            path = Path(coord, code)
            patches.append(PathPatch(path, color=colors[i], ec='black', lw=1))

                  
        #create a new figure
        fig, ax = plt.subplots()
        self.figure = fig
        self.plt_ax = ax
        
        
        #add the patches to the plot
        for patch in patches:
            ax.add_patch(patch)
            
        self.color_map = {}
        self.legend_map = {}
        if 0 != len(self.group_names):
            labels = ["Group " + str(group_id) +  ": " + group_name for group_id,group_name in zip(self.group_ids,self.group_names)]
            for gid,patch in zip(self.group_ids,patches): self.color_map[gid] = patch.get_facecolor() 
            leg = ax.legend(patches, labels, prop={'size':14}, loc=2, bbox_to_anchor=(1.05,1.), borderaxespad=0.)
            #create mapping of artist to legend entry
            for legpatch, patch in zip(leg.get_patches(), patches):
                legpatch.set_picker(True)
                self.legend_map[legpatch] = patch

                #setup the check boxex
                cax = plt.axes([0.025, 0.5, 0.12, 0.12])
                self.check = CheckButtons( cax, ('Visible','Filled'),(True,True) )
                self.check.visible = False
                self.check.on_clicked(self.visiblefunc)


        #plot axis settings
        ax.autoscale_view()
        ax.set_aspect('equal')


        cid = self.figure.canvas.mpl_connect('pick_event', self.onpick)

        plt.show()
        self.shown = True

    def make_legend(self):
        
        legend_items = []
        for leg_patch,gid,gname in zip(self.legend_map.keys(),self.group_ids,self.group_names): 
            lb = widgets.Text(gname,description="Group " + str(gid))
            bg_color = rgb2hex(self.color_map[gid][:-1])
            cb = widgets.Box(background_color=bg_color,height=32,width=80)
            cb.margin = 10
            lb.margin = 10
            i = widgets.HBox(children=[cb,lb])
            legend_items.append(i)
            
        self.new_filename = widgets.Text(description='Filename')
        exp_but = widgets.Button(description="Export Model")
        exp_but.on_click(self.write_file)
          
        leg = widgets.Box(children=legend_items)
        leg.children += (exp_but,self.new_filename,)
            
        #one more outer hozo box for the figure
        box = widgets.HBox(children=[leg])
        display(box)

    def export(self):
        self.write_file(self.new_filename.value)
        
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
        
