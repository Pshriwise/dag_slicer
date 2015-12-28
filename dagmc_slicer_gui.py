
# coding: utf-8

# In[1]:

get_ipython().magic(u'matplotlib notebook')
import matplotlib
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.path import Path
from matplotlib.patches import PathPatch
from IPython.html.widgets import interact,interact_manual
from IPython.html import widgets
from IPython.display import display
from dagmc_slice_tool import dagmc_slicer
from matplotlib.colors import rgb2hex
from IPython.display import Javascript

class slicer_gui(dagmc_slicer):

    def slice(self, button):
        kids = self.slice_box.children[0].children[0].children
        self.filename = kids[0].value
        self.axis = kids[1].value
        self.coord = kids[2].value
        self.by_group = kids[3].value
        self.create_slice()
        self.show_slice()

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

        if 0 != len(self.group_names):
            self.legend_map = {}
                        
            legend_items = []
            for patch,gid,gname in zip(patches,self.group_ids,self.group_names):
                lb = widgets.Text(gname,description="Group " + str(gid))
                bg_color = rgb2hex(patch.get_facecolor()[:-1])
                key = widgets.Box(background_color=bg_color,height=32,width=32)
                cb1 = widgets.Button(description="Visible")
                cb1.on_click(self.visiblefunc)
                cb2 = widgets.Button(description="Fill")
                cb2.on_click(self.filledfunc)
                updeight = widgets.Button(description="Update")
                updeight.on_click(self.update_group_name)
                updeight.margin = 3
                cb1.margin = 3
                cb2.margin = 3
                key.margin = 3
                lb.margin = 3
                children = [key,lb,cb1,cb2,updeight]
                item = widgets.HBox(children=children)
                updeight.parent = item
                self.legend_map[cb1] = patch
                self.legend_map[cb2] = patch
                legend_items.append(item)


            pages = []
            number_per_page = 5
            while number_per_page < len(legend_items):
                pages.append(legend_items[0:6])
                legend_items = legend_items[6:]
            #add the last bit (if any)
            if len(legend_items) != 0: pages.append(legend_items)

            boxes = []
            for page in pages:
                b = widgets.Box(children=page)
                boxes.append(b)

            self.legend_box.children=boxes
        
        #add the patches to the plot
        self.plt_ax.clear()
        for patch in patches:
            self.plt_ax.add_patch(patch)

        self.plt_ax.autoscale_view()
        self.plt_ax.set_aspect('equal')
        self.figure.canvas.draw()
        
    def setup_gui(self):

        run_button = widgets.Button(description="Run Slice")
        run_button.on_click(self.slice)
        ax_widget = widgets.RadioButtons(description="Axis", options = {'x':0,'y':1,'z':2})
        coord_widget = widgets.FloatText(description="Slice Coordinate", value = 0.0)
        group_widget = widgets.Checkbox(description="Slice by Group")
        filename_widget = widgets.Text(description = "Filename")
        
        params_box = widgets.Box()
        params_box.children = [filename_widget,ax_widget,coord_widget,group_widget]
        self.export_box = widgets.Box()
        export_button = widgets.Button(description="Save File")
        export_button.on_click(self.export_file)
        export_name = widgets.Text(description="Filename", margin = 5)
        self.export_box.children = (export_name,export_button,)
        accord = widgets.Accordion()
        accord.children = (params_box,self.export_box,)
        accord.set_title(0,"Slice Parameters")
        accord.set_title(1,"Export")

        
        self.slice_box = widgets.Box()
        self.slice_box.children = (accord,run_button)
        
        self.legend_box = widgets.Tab()
        self.gui_box = widgets.HBox()
        accord2 = widgets.Accordion()
        accord2.children = (self.legend_box,)
        accord2.set_title(0,"Legend")
        self.gui_box.children = (self.slice_box,accord2)

        #create a new figure
        fig, ax = plt.subplots()
        fig.set_size_inches(8,8)
        self.figure = fig
        self.plt_ax = ax

        display(self.gui_box)
        display(plt)
        
    def visiblefunc(self,button):

        button_color = 'grey' if button.background_color == '' else ''

        button.background_color = button_color
        patch = self.legend_map[button]
        patch.set_visible(True if button.background_color == '' else False)

    def filledfunc(self,button):

        button_color = 'grey' if button.background_color == '' else ''

        button.background_color = button_color
        patch = self.legend_map[button]
        patch.set_fill(True if button.background_color == '' else False)

    def export_file(self,button):

        #a decision has been made, remove confirmation
        if button.description == "Yes" or button.description == "No":
            self.export_box.children = self.export_box.children[:-1]

        #if no, do nothing
        if button.description == "No":
            return
        
        new_filename = self.export_box.children[0].value

        override = button.description == "Yes"

        #if the filename is the same, confirm they want this to happen
        if (new_filename == self.filename and not override):
            #add buttons to the box to confirm
            msg = widgets.Button(description = "This will overwrite the current file. Are you sure?",border_color='white')
            y = widgets.Button(description="Yes", margin = 2)
            y.on_click(self.export_file)
            n = widgets.Button(description="No", margin = 2)
            n.on_click(self.export_file)
            resp_box = widgets.HBox(children=[y,n])
            confirm_box = widgets.Box(children=[msg,resp_box])
            self.export_box.children += (confirm_box,)
            return

        super(dagmc_slicer,self).write_file(new_filename)
        return

    def update_group_name(self,button):

        parent = button.parent
        new_group_name = parent.children[1].value
        group_id = int(parent.children[1].description.strip()[-1])

        self.rename_group(group_id,new_group_name)
