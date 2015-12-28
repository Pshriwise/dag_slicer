
# coding: utf-8

# In[1]:

get_ipython().magic(u'matplotlib inline')
import matplotlib
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.path import Path
from matplotlib.patches import PathPatch
from IPython.html.widgets import interact,interact_manual
from IPython.html import widgets
from IPython.display import display
import mpld3
mpld3.enable_notebook()
from dagmc_slice_tool import dagmc_slicer
from matplotlib.colors import rgb2hex

class slicer_gui(dagmc_slicer):

    def slice(self, filename, axis, slice_coordinate, slice_by_group):
        self.filename = filename
        self.axis = axis
        self.coord = slice_coordinate
        self.by_group = slice_by_group
        self.create_slice()
        self.show_slice()
#        self.make_legend()

    def show_slice(self, colors=None):
        
        if 0 == len(self.slice_x_pnts):
            self.create_slice()


        self.legend = widgets.Box()
        
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

        self.color_map = {}
        if 0 != len(self.group_names):
            for gid,p in zip(self.group_ids,patches): self.color_map[gid] = p.get_facecolor()

                        
            legend_items = []
            for gid,gname in zip(self.group_ids,self.group_names): 
                lb = widgets.Text(gname,description="Group " + str(gid))
                bg_color = rgb2hex(self.color_map[gid][:-1])
                cb = widgets.Box(background_color=bg_color,height=32,width=80)
                cb.margin = 10
                lb.margin = 10
                i = widgets.HBox(children=[cb,lb])
                legend_items.append(i)
                self.legend_box.children = legend_items

        #create a new figure
        fig, ax = plt.subplots()
        self.figure = fig
        self.plt_ax = ax
        
        
        #add the patches to the plot
        for patch in patches:
            ax.add_patch(patch)

        ax.autoscale_view()
        ax.set_aspect('equal')
        return plt
        
    def setup_gui(self):

        ax_widget = widgets.RadioButtons(description="Axis", options = {'x':0,'y':1,'z':2})
        coord_widget = widgets.FloatText(description="Slice Coordinate", value = 0.0)
        group_widget = widgets.Checkbox(description="Slice by Group")
        i = widgets.interactive(self.slice, filename = "teapot_grps_zip.h5m", axis=ax_widget, slice_coordinate = coord_widget, slice_by_group = group_widget, __manual = True)
        
        params_box = widgets.Box()
        params_box.children = i.children[:-1]
        accord = widgets.Accordion()
        accord.children = (params_box,)
        accord.set_title(0,"Slice Parameters")
        self.slice_box = widgets.Box()
        self.slice_box.children = (accord,i.children[-1])
        self.legend_box = widgets.Box()
        self.gui_box = widgets.HBox()
        accord1 = widgets.Accordion()
        accord1.children = (self.legend_box,)
        accord1.set_title(0,"Legend")
        self.gui_box.children = (self.slice_box,accord1)
        
        display(self.gui_box)


