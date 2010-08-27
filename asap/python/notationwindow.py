import os
import matplotlib
from asap.logging import asaplog, asaplog_post_dec

######################################
##    Notation box window           ##
######################################
class NotationWindowCommon:
    """
    A base class to define the functions that the backend-based
    GUI notation class must implement to print/modify/delete notes on a canvas.

    The following methods *must* be implemented in the backend-based
    parent class:
        _get_note : get text in text box
        _get_anchval : get anchor value selected
    """
    def __init__(self,master=None):
        #self.parent = master
        self.canvas = master
        self.event = None
        self.note = None
        self.anchors = ["figure","axes","data"]
        self.seltext = {}
        self.numnote = 0

    @asaplog_post_dec
    def print_text(self):
        """
        Print a note on a canvas specified with the Notation window.
        Called when 'print' button selected on the window.
        """
        anchor = self.anchors[self._get_anchval()]
        notestr = self._get_note().rstrip("\n")
        if len(notestr.strip()) == 0:
            #self._clear_textbox()
            #print "Empty string!"
            return

        myaxes = None
        calcpos = True
        xpos = None
        ypos = None
        if self.seltext:
            # You are modifying a text
            mycanvas = self.canvas
            oldanch = self.seltext['anchor']
            if oldanch != 'figure':
                myaxes = self.seltext['parent']
            calcpos = (anchor != oldanch)
            if not calcpos:
                # printing text in the same coord.
                # you don't have to recalc position
                parent = self.seltext['parent']
                transform = self.seltext['textobj'].get_transform()
                (xpos, ypos) = self.seltext['textobj'].get_position()
            elif anchor == "figure":
                # converting from "axes"/"data" -> "figure"
                (x, y) = self.seltext['textobj']._get_xy_display()
            elif oldanch == "data":
                # converting from "data" -> "axes".
                # need xdata & ydata in the axes
                (x, y) = self.seltext['textobj'].get_position()
            else:
                # converting "figure"/"axes" -> "data"
                # need to calculate xdata & ydata in the axes
                pixpos = self.seltext['textobj']._get_xy_display()
                (w,h) = mycanvas.get_width_height()
                relpos = (pixpos[0]/float(w), pixpos[1]/float(h))
                if not myaxes:
                    myaxes = self._get_axes_from_pos(relpos,mycanvas)
                    if not myaxes:
                        raise RuntimeError, "Axes resolution failed!"
                (x, y) = self._convert_pix2dat(relpos,myaxes)
            self._remove_seltext()
        elif self.event:
            mycanvas = self.event.canvas
            myaxes = self.event.inaxes
            if myaxes and (anchor != "figure"):
                x = self.event.xdata
                y = self.event.ydata
            else:
                x = self.event.x
                y = self.event.y
        else:
            raise RuntimeError, "No valid position to print data"
            return

        # now you know 
        picker = True
        # alignment of the text: ha (horizontal), va (vertical)
        ha = 'left'
        #va = 'center'
        va = 'top'
        if not calcpos:
            # you aready know parent, tansform, xpos and ypos
            pass
        elif anchor == "figure":
            # text instance will be appended to mycanvas.figure.texts
            parent = mycanvas.figure
            transform = parent.transFigure
            (w,h) = mycanvas.get_width_height()
            xpos = x/float(w)
            ypos = y/float(h)            
        elif myaxes:
            ## text instance will be appended to myaxes.texts
            parent = myaxes
            if anchor == "axes":
                transform = myaxes.transAxes
                lims = myaxes.get_xlim()
                xpos = (x-lims[0])/(lims[1]-lims[0])
                lims = myaxes.get_ylim()
                ypos = (y-lims[0])/(lims[1]-lims[0])
            else:
                # anchored on "data"
                transform = myaxes.transData
                xpos = x
                ypos = y
        parent.text(xpos,ypos,notestr,transform=transform,
                    ha=ha,va=va,picker=picker)
        mycanvas.draw()

        self.numnote += 1

        #self._clear_textbox()
        msg = "Added note: '"+notestr+"'"
        msg += " @["+str(xpos)+", "+str(ypos)+"] ("+anchor+"-coord)"
        msg += "\ntotal number of notes are "+str(self.numnote)
        asaplog.push( msg )

    def _get_axes_from_pos(self,pos,canvas):
        """helper function to get axes of a position in a plot (fig-coord)"""
        if len(pos) != 2:
            raise ValueError, "pixel position should have 2 elements"
        for axes in canvas.figure.axes:
            ##check if pos is in the axes
            #if axes.contains_point(pos): ### seems not working
            #    return axes
            try:
                axbox = axes.get_position().get_points()
            except AttributeError: ### WORKAROUND for old matplotlib
                axbox = self._oldpos2new(axes.get_position())
            if (axbox[0][0] <= pos[0] <= axbox[1][0]) and \
               (axbox[0][1] <= pos[1] <= axbox[1][1]):
                return axes
        return None

    ### WORKAROUND for old matplotlib
    def _oldpos2new(self,oldpos=None):
        return [[oldpos[0],oldpos[1]],[oldpos[0]+oldpos[2],oldpos[1]+oldpos[3]]]
        
    def _convert_pix2dat(self,pos,axes):
        """
        helper function to convert a position in figure-coord (0-1) to
        data-coordinate of the axes        
        """
        # convert a relative position from lower-left of the canvas
        # to a data in axes
        if len(pos) != 2:
            raise ValueError, "pixel position should have 2 elements"
        # left-/bottom-pixel, and pixel width & height of the axes
        bbox = axes.get_position()
        try: 
            axpos = bbox.get_points()
        except AttributeError: ### WORKAROUND for old matplotlib
            axpos = self._oldpos2new(bbox)
        # check pos value
        if (pos[0] < axpos[0][0]) or (pos[1] < axpos[0][1]) \
               or (pos[0] > axpos[1][0]) or (pos[1] > axpos[1][1]):
            raise ValueError, "The position is out of the axes"
        xlims = axes.get_xlim()
        ylims = axes.get_ylim()
        wdat = xlims[1] - xlims[0]
        hdat = ylims[1] - ylims[0]
        xdat = xlims[0] + wdat*(pos[0] - axpos[0][0])/(axpos[1][0] - axpos[0][0])
        ydat = ylims[0] + hdat*(pos[1] - axpos[0][1])/(axpos[1][1] - axpos[0][1])
        return (xdat, ydat)

    @asaplog_post_dec
    def _get_selected_text(self,event):
        """helper function to return a dictionary of the nearest note to the event."""
        (w,h) = event.canvas.get_width_height()
        dist2 = w*w+h*h
        selected = {}
        for textobj in self.canvas.figure.texts:
            if textobj.contains(event)[0]:
                d2 = self._get_text_dist2(event,textobj)
                if dist2 >= d2:
                    dist2 = d2
                    selected = {'anchor': 'figure', \
                                'parent': event.canvas.figure, 'textobj': textobj}
                    msg = "Fig loop: a text, '"+textobj.get_text()+"', at "
                    msg += str(textobj.get_position())+" detected"
                    print msg
        for ax in self.canvas.figure.axes:
            for textobj in ax.texts:
                if textobj.contains(event)[0]:
                    d2 = self._get_text_dist2(event,textobj)
                    if dist2 >= d2:
                        anchor='axes'
                        if ax.transData == textobj.get_transform():
                            anchor = 'data'                    
                        selected = {'anchor': anchor, \
                                    'parent': ax, 'textobj': textobj}
                        msg = "Ax loop: a text, '"+textobj.get_text()+"', at "
                        msg += str(textobj.get_position())+" detected"
                        print msg 

        if selected:
            msg = "Selected (modify/delete): '"+textobj.get_text()
            msg += "' @"+str(textobj.get_position())
            msg += " ("+selected['anchor']+"-coord)"
            asaplog.push(msg)

        return selected

    def _get_text_dist2(self,event,textobj):
        """
        helper function to calculate square of distance between
        a event position and a text object. 
        """
        (x,y) = textobj._get_xy_display()
        return (x-event.x)**2+(y-event.y)**2

    def delete_note(self):
        """
        Remove selected note.
        """
        #print "You selected 'OK'"
        self._remove_seltext()
        self.canvas.draw()

    @asaplog_post_dec
    def _remove_seltext(self):
        """helper function to remove the selected note"""
        if len(self.seltext) < 3:
            raise ValueError, "Don't under stand selected text obj."
            return
        try:
            self.seltext['textobj'].remove()
        except NotImplementedError:
                self.seltext['parent'].texts.pop(self.seltext['parent'].texts.index(self.seltext['textobj']))
        self.numnote -= 1

        textobj = self.seltext['textobj']
        msg = "Deleted note: '"+textobj.get_text()+"'"
        msg += "@"+str(textobj.get_position())\
               +" ("+self.seltext['anchor']+"-coord)"
        msg += "\ntotal number of notes are "+str(self.numnote)
        asaplog.push( msg )

        self.seltext = {}

    @asaplog_post_dec
    def cancel_delete(self):
        """
        Cancel deleting the selected note.
        Called when 'cancel' button selected on confirmation dialog.
        """
        asaplog.push( "Cancel deleting: '"+self.seltext['textobj'].get_text()+"'" )
        self.seltext = {}


#####################################
##    Backend dependent Classes    ##
#####################################
### TkAgg
if matplotlib.get_backend() == 'TkAgg':
    import Tkinter as Tk
    import tkMessageBox

class NotationWindowTkAgg(NotationWindowCommon):
    """
    Backend based class to create widgets to add, modify, or delete
    note on the plot.

    Note:
    Press LEFT-mouse button on the plot to ADD a note on the canvas.
    A notation window will be loaded for specifying note string and
    anchor. The note will be anchored on a position in whether figure-
    (0-1 relative in a figure), panel- (0-1 relative in a plot axes),
    or data-coordinate (data value in a plot axes).
    Press RIGHT-mouse button on a note to MODIFY/DELETE it. A cascade
    menu will be displayed and you can select an operation.
    """
    def __init__(self,master=None):
        self.parent = master._tkcanvas
        NotationWindowCommon.__init__(self,master=master)
        self.anchval = None
        self.textwin = self._create_textwindow(master=None)
        self.menu = self._create_modmenu(master=self.parent)

    ### Notation window widget
    def _create_textwindow(self,master=None):
        """Create notation window widget and iconfy it"""
        twin = Tk.Toplevel(padx=3,pady=3)
        twin.title("Notation")
        twin.resizable(width=True,height=True)
        self.textbox = self._NotationBox(parent=twin)
        self.radio = self._AnchorRadio(parent=twin)
        self.actionbs = self._ActionButtons(parent=twin)
        
        self.textbox.pack(side=Tk.TOP,fill=Tk.BOTH,expand=True)
        self.actionbs.pack(side=Tk.BOTTOM)
        self.radio.pack(side=Tk.BOTTOM)
        #twin.deiconify()
        #twin.minsize(width=twin.winfo_width(),height=twin.winfo_height())
        twin.lift()
        twin.withdraw()
        return twin

    def _NotationBox(self,parent=None):
        textbox = Tk.Text(master=parent,background='white',
                          height=2,width=20,cursor="xterm",
                          padx=2,pady=2,undo=True,maxundo=10,
                          relief='sunken',borderwidth=3)
        return textbox

    def _AnchorRadio(self,parent=None):
        radio = Tk.LabelFrame(master=parent,text="anchor",
                            labelanchor="nw",padx=5,pady=3)
        self.anchval = Tk.IntVar(master=radio,value=0)
        self.rFig = self._NewRadioButton(radio,"figure",state=Tk.NORMAL,
                                         variable=self.anchval,value=0,
                                         side=Tk.LEFT)
        self.rAxis = self._NewRadioButton(radio,"panel",state=Tk.DISABLED,
                                          variable=self.anchval,value=1,
                                          side=Tk.LEFT)
        self.rData = self._NewRadioButton(radio,"data",state=Tk.DISABLED,
                                          variable=self.anchval,value=2,
                                          side=Tk.LEFT)
        # set initial selection "figure"
        self.anchval.set(0)
        return radio

    def _NewRadioButton(self,parent,text,state=Tk.NORMAL,variable=None,value=None,side=Tk.LEFT):
        rb = Tk.Radiobutton(master=parent,text=text,state=state,
                          variable=variable,value=value)
        rb.pack(side=side)
        return rb

    def _enable_radio(self):
        """Enable 'panel' and 'data' radio button"""
        self.rAxis.config(state=Tk.NORMAL)
        self.rData.config(state=Tk.NORMAL)
        #self.rFig.config(state=Tk.NORMAL)
        self.rFig.select()

    def _reset_radio(self):
        """Disable 'panel' and 'data' radio button"""
        self.rAxis.config(state=Tk.DISABLED)
        self.rData.config(state=Tk.DISABLED)
        self.rFig.config(state=Tk.NORMAL)
        self.rFig.select()

    def _select_radio(self,selection):
        """Select a specified radio button"""
        if not selection in self.anchors:
            return
        if selection == "data":
            self.rData.select()
        elif selection == "axes":
            self.rAxis.select()
        else:
            self.rFig.select()

    def _get_anchval(self):
        """Returns a integer of a selected radio button"""
        return self.anchval.get()

    def _get_note(self):
        """Returns a note string specified in the text box"""
        return self.textbox.get("1.0",Tk.END)

    def _clear_textbox(self):
        """Clear the text box"""
        self.textbox.delete("1.0",Tk.END)

    def _set_note(self,note=None):
        """Set a note string to the text box"""
        self._clear_textbox()
        if len(note) >0:
            self.textbox.insert("1.0",note)

    def _ActionButtons(self,parent=None):
        actbuts = Tk.Frame(master=parent)
        bCancel = self._NewButton(actbuts,"cancel",self._cancel_text,side=Tk.LEFT)
        bPrint = self._NewButton(actbuts,"print", self._print_text,side=Tk.LEFT)
        return actbuts

    def _NewButton(self, parent, text, command, side=Tk.LEFT):
        if(os.uname()[0] == 'Darwin'):
            b = Tk.Button(master=parent, text=text, command=command)
        else:
            b = Tk.Button(master=parent, text=text, padx=2, pady=2, command=command)
        b.pack(side=side)
        return b

    def _cancel_text(self):
        """
        Cancel adding/modifying a note and close notaion window.
        called when 'cancel' is selected.
        """
        self.close_textwindow()

    def _print_text(self):
        """
        Add/Modify a note. Called when 'print' is selected on the
        notation window.
        """
        self.print_text()
        self.close_textwindow()

    def load_textwindow(self,event):
        """
        Load text window at a event position to add a note on a plot.
        Parameter:
            event:   an even object to specify the position to load
                     text window. 
        """
        self.close_modmenu()
        if event.canvas._tkcanvas != self.parent:
            raise RuntimeError, "Got invalid event!"
        
        self.event = event
        is_ax = (event.inaxes != None)
        (xpix, ypix) = self._disppix2screen(event.x, event.y)
        offset = 5
        self.show_textwindow(xpix+offset,ypix+offset,enableaxes=is_ax)
        
    def show_textwindow(self,xpix,ypix,basetext=None,enableaxes=False):
        """
        Load text window at a position of screen to add a note on a plot.
        Parameters:
            xpix, ypix:   a pixel position from Upper-left corner
                          of the screen.
            basetext:     None (default) or any string.
                          A string to be printed on text box when loaded. 
            enable axes:  False (default) or True.
                          If True, 'panel' & 'data' radio button is enabled. 
        """
        if not self.textwin: return
        self._reset_radio()
        if enableaxes: 
            self._enable_radio()
        self.textwin.deiconify()
        (w,h) = self.textwin.minsize()
        if w*h <= 1:
            self.textwin.minsize(width=self.textwin.winfo_width(),
                                 height=self.textwin.winfo_height())
            (w,h) = self.textwin.minsize()
        self.textwin.geometry("%sx%s+%s+%s"%(w,h,xpix,ypix))
        self.textwin.lift()

    def close_textwindow(self):
        """Close text window."""
        self.seltext = {}
        self._reset_radio()
        self._clear_textbox()
        self.textwin.withdraw()


    ### Modify/Delete menu widget
    def _create_modmenu(self,master=None):
        """Create modify/delete menu widget"""
        if master:
            self.parent = master
        if not self.parent:
            return False
        menu = Tk.Menu(master=self.parent,tearoff=False)
        menu.add_command(label="Modify",command=self._modify_note)
        menu.add_command(label="Delete",command=self._delnote_dialog)
        return menu

    def load_modmenu(self,event):
        """
        Load cascade menu at a event position to modify or delete
        selected text.
        Parameter:
            event:  an even object to specify the position to load
                    text window. 
        """
        self.close_textwindow()
        self.seltext = self._get_selected_text(event)
        if len(self.seltext) == 3:
            tkcanvas = event.canvas._tkcanvas
            xpos = tkcanvas.winfo_rootx() + int(event.x)
            ypos = tkcanvas.winfo_rooty() \
                   + tkcanvas.winfo_height() - int(event.y)
            self.menu.post(xpos,ypos)

    def close_modmenu(self):
        """Close cascade menu."""
        self.seltext = {}
        self.menu.unpost()

    ### load text window for modification 
    def _modify_note(self):
        """helper function to load text window to modify selected note"""
        #print "Modify selected!!"
        textobj = self.seltext['textobj']
        (xtx, ytx) = textobj._get_xy_display()
        is_ax = (self.seltext['anchor'] != 'figure')
        if not is_ax:
            # previous anchor is figure
            pos = textobj.get_position()
            is_ax = (self._get_axes_from_pos(pos,self.canvas) != None)

        (xpix, ypix) = self._disppix2screen(xtx,ytx)
        offset = int(textobj.get_size())*2
        self.show_textwindow(xpix,ypix+offset,basetext=textobj.get_text(),\
                             enableaxes=is_ax)
        self._select_radio(self.seltext['anchor'])
        self._set_note(textobj.get_text())

    ### close all widgets
    def close_widgets(self):
        """Close note window and menu"""
        self.close_textwindow()
        self.close_modmenu()

    ### dialog to confirm deleting note 
    def _delnote_dialog(self):
        """Load dialog to confirm deletion of the text"""
        remind = "Delete text?\n '"+self.seltext['textobj'].get_text()+"'"
        answer = tkMessageBox.askokcancel(parent=self.parent,title="Delete?",
                                          message=remind,
                                          default=tkMessageBox.CANCEL)
        if answer:
            self.delete_note()
        else:
            self.cancel_delete()

    ### helper functions
    def _disppix2screen(self,xpixd,ypixd):
        """
        helper function to calculate a pixel position form Upper-left
        corner of the SCREEN from a pixel position (xpixd, ypixd)
        from Lower-left of the CANVAS (which, e.g., event.x/y returns)

        Returns:
            (x, y):  pixel position from Upper-left corner of the SCREEN.
        """
        xpixs = self.parent.winfo_rootx() + xpixd
        ypixs = self.parent.winfo_rooty() + self.parent.winfo_height() \
               - ypixd
        return (int(xpixs), int(ypixs))
        
