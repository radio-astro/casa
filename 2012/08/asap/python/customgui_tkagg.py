import os
import matplotlib, numpy
from asap.logging import asaplog, asaplog_post_dec
from matplotlib.patches import Rectangle
from asap.parameters import rcParams
from asap._asap import stmath
from asap.customgui_base import *

import Tkinter as Tk
import tkMessageBox

######################################
##    Add CASA custom toolbar       ##
######################################
class CustomToolbarTkAgg(CustomToolbarCommon, Tk.Frame):
    def __init__(self,parent):
        from asap.asapplotter import asapplotter
        if not isinstance(parent,asapplotter):
            return False
        if not parent._plotter:
            return False
        self._p = parent._plotter
        self.figmgr = self._p.figmgr
        self.canvas = self.figmgr.canvas
        self.mode = ''
        self.button = True
        self.pagecount = None
        CustomToolbarCommon.__init__(self,parent)
        self.notewin = NotationWindowTkAgg(master=self.canvas)
        self._add_custom_toolbar()

    def _add_custom_toolbar(self):
        Tk.Frame.__init__(self,master=self.figmgr.window)
        #self.bSpec = self._NewButton(master=self,
        #                             text='spec value',
        #                             command=self.spec_show)
        self.bNote = self._NewButton(master=self,
                                     text='notation',
                                     command=self.modify_note)

        self.bStat = self._NewButton(master=self,
                                     text='statistics',
                                     command=self.stat_cal)
        self.bQuit = self._NewButton(master=self,
                                     text='Quit',
                                     command=self.quit,
                                     side=Tk.RIGHT)

        # page change oparations
        frPage = Tk.Frame(master=self,borderwidth=2,relief=Tk.GROOVE)
        frPage.pack(ipadx=2,padx=10,side=Tk.RIGHT)
        self.lPagetitle = Tk.Label(master=frPage,text='Page:',padx=5)
        self.lPagetitle.pack(side=Tk.LEFT)
        self.pagecount = Tk.StringVar(master=frPage)
        self.lPagecount = Tk.Label(master=frPage,
                                   textvariable=self.pagecount,
                                   padx=5,bg='white')
        self.lPagecount.pack(side=Tk.LEFT,padx=3)
        
        self.bNext = self._NewButton(master=frPage,
                                     text=' + ',
                                     command=self.next_page)
        self.bPrev = self._NewButton(master=frPage,
                                     text=' - ',
                                     command=self.prev_page)

        if os.uname()[0] != 'Darwin':
            self.bPrev.config(padx=5)
            self.bNext.config(padx=5)

        self.pack(side=Tk.BOTTOM,fill=Tk.BOTH)
        self.pagecount.set(' '*4)

        self.disable_button()
        return #self

    def _NewButton(self, master, text, command, side=Tk.LEFT):
        if os.uname()[0] == 'Darwin':
            b = Tk.Button(master=master, text=text, command=command)
        else:
            b = Tk.Button(master=master, text=text, padx=2, pady=2,
                          command=command)
        b.pack(side=side)
        return b

    def show_pagenum(self,pagenum,formatstr):
        self.pagecount.set(formatstr % (pagenum))

    def spec_show(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        self.figmgr.toolbar.set_message('spec value: drag on a spec')
        if self.mode == 'spec': return
        #self.bStat.config(relief='raised')
        #self.bSpec.config(relief='sunken')
        #self.bNote.config(relief='raised')
        self.mode = 'spec'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._select_spectrum)

    def stat_cal(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        self.figmgr.toolbar.set_message('statistics: select a region')
        if self.mode == 'stat':
            # go back to spec mode
            self.bStat.config(relief='raised')
            self.spec_show()
            return
        #self.bSpec.config(relief='raised')
        self.bStat.config(relief='sunken')
        self.bNote.config(relief='raised')
        self.mode = 'stat'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._single_mask)

    def modify_note(self):
        if not self.figmgr.toolbar.mode == '': return
        if self.mode == 'note':
            self.bNote.config(relief='raised')
            self.mode = 'none'
            self.spec_show()
            if not self.button:
                self.notewin.close_widgets()
                self.__disconnect_event()
            return
        self.figmgr.toolbar.set_message('text: select a position/text')
        #self.bSpec.config(relief='raised')
        self.bStat.config(relief='raised')
        self.bNote.config(relief='sunken')
        self.mode = 'note'
        self.__disconnect_event()
        self._p.register('button_press',self._mod_note)

    def quit(self):
        self.__disconnect_event()
        #self.delete_bar()
        self.disable_button()
        self.figmgr.window.wm_withdraw()
        self._p.quit()

    def enable_button(self):
        if self.button: return
        #self.bSpec.config(state=Tk.NORMAL)
        self.bStat.config(state=Tk.NORMAL)
        self.button = True
        self.spec_show()

    def disable_button(self):
        if not self.button: return
        #self.bSpec.config(relief='raised', state=Tk.DISABLED)
        self.bStat.config(relief='raised', state=Tk.DISABLED)
        self.bNext.config(state=Tk.DISABLED)
        self.bPrev.config(state=Tk.DISABLED)
        self.button = False
        self.mode = ''
        self.__disconnect_event()

    def enable_next(self):
        self.bNext.config(state=Tk.NORMAL)

    def disable_next(self):
        self.bNext.config(state=Tk.DISABLED)

    def enable_prev(self):
        self.bPrev.config(state=Tk.NORMAL)

    def disable_prev(self):
        self.bPrev.config(state=Tk.DISABLED)

    # pause buttons for slow operations
    def _pause_buttons(self,operation="end",msg=""):
        buttons = ["bStat","bNote","bQuit"]
        if operation == "start":
            state = Tk.DISABLED
        else:
            state = Tk.NORMAL
        for btn in buttons:
            getattr(self,btn).config(state=state)
        self.figmgr.toolbar.set_message(msg)

    def delete_bar(self):
        self.__disconnect_event()
        self.destroy()

    def __disconnect_event(self):
        self._p.register('button_press',None)
        self._p.register('button_release',None)

    def _draw_span(self,axes,x0,x1,**kwargs):
        height = self._p.figure.bbox.height
        y0 = height - axes.bbox.y0
        y1 = height - axes.bbox.y1
        return self._p.canvas._tkcanvas.create_rectangle(x0,y0,x1,y1,**kwargs)

    def _remove_span(self,span):
        self._p.canvas._tkcanvas.delete(span)



######################################
##    Notation box window           ##
######################################
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
        #twin.lift()
        twin.withdraw()
        return twin

    def _NotationBox(self,parent=None):
        textbox = Tk.Text(master=parent,background='white',
                          height=2,width=20,cursor="xterm",
                          padx=2,pady=2,undo=True,maxundo=10,
                          relief='sunken',borderwidth=3,
                          state=Tk.NORMAL,takefocus=1)
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
        # A WORKAROUND for OSX 10.7 (Tk.Text returns unicode but asaplog doesn't accept it)
        #return self.textbox.get("1.0",Tk.END)
        return str(self.textbox.get("1.0",Tk.END))

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
        # WORKAROUND for too small default minsize on OSX 10.7
        #if w*h <= 1:
        if w*h <= 1500:
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
        





###########################################
##    Add CASA custom Flag toolbar       ##
###########################################
class CustomFlagToolbarTkAgg(CustomFlagToolbarCommon, Tk.Frame):
    def __init__(self,parent):
        from asap.asapplotter import asapplotter
        if not isinstance(parent,asapplotter):
            return False
        if not parent._plotter:
            return False
        self._p = parent._plotter
        self.figmgr = self._p.figmgr
        self.canvas = self.figmgr.canvas
        self.mode = ''
        self.button = True
        self.pagecount = None
        CustomFlagToolbarCommon.__init__(self,parent)
        self.notewin=NotationWindowTkAgg(master=self.canvas)
        self._add_custom_toolbar()

    def _add_custom_toolbar(self):
        Tk.Frame.__init__(self,master=self.figmgr.window)
        #self.bSpec = self._NewButton(master=self,
        #                             text='spec value',
        #                             command=self.spec_show)

        self.bRegion = self._NewButton(master=self,
                                       text='region',
                                       command=self.select_region)
        self.bPanel = self._NewButton(master=self,
                                      text='panel',
                                      command=self.select_panel)
        self.bClear = self._NewButton(master=self,
                                      text='clear',
                                      command=self.cancel_select)
        self.bFlag = self._NewButton(master=self,
                                     text='flag',
                                     command=self.flag)
        self.bUnflag = self._NewButton(master=self,
                                       text='unflag',
                                       command=self.unflag)
        
        self.bStat = self._NewButton(master=self,
                                     text='statistics',
                                     command=self.stat_cal)

        self.bNote = self._NewButton(master=self,
                                     text='notation',
                                     command=self.modify_note)

        self.bQuit = self._NewButton(master=self,
                                     text='Quit',
                                     #file="stock_close.ppm",
                                     command=self.quit,
                                     side=Tk.RIGHT)

        # page change oparations
        frPage = Tk.Frame(master=self,borderwidth=2,relief=Tk.GROOVE)
        frPage.pack(ipadx=2,padx=10,side=Tk.RIGHT)
        self.lPagetitle = Tk.Label(master=frPage,text='Page:',padx=5)
                                   #width=8,anchor=Tk.E,padx=5)
        self.lPagetitle.pack(side=Tk.LEFT)
        self.pagecount = Tk.StringVar(master=frPage)
        self.lPagecount = Tk.Label(master=frPage,
                                   textvariable=self.pagecount,
                                   padx=5,bg='white')
        self.lPagecount.pack(side=Tk.LEFT,padx=3)

        self.bNext=self._NewButton(master=frPage,
                                   text=' + ',
                                   #file="hand.ppm",
                                   command=self.next_page)
        self.bPrev=self._NewButton(master=frPage,
                                   text=' - ',
                                   command=self.prev_page)

        if os.uname()[0] != 'Darwin':
            self.bPrev.config(padx=5)
            self.bNext.config(padx=5)

        self.pack(side=Tk.BOTTOM,fill=Tk.BOTH)
        self.pagecount.set(' '*4)

        self.disable_button()
        return #self

    def _NewButton(self, master, text, command, side=Tk.LEFT,file=None):
        img = None
        if file:
            file = os.path.join(matplotlib.rcParams['datapath'], 'images', file)
            img = Tk.PhotoImage(master=master, file=file)

        if os.uname()[0] == 'Darwin':
            b = Tk.Button(master=master, text=text, image=img,
                          command=command)
            if img: b.image = img
        else:
            b = Tk.Button(master=master, text=text, image=img, padx=2, pady=2,
                          command=command)
            if img: b.image = img
        b.pack(side=side)
        return b

    def show_pagenum(self,pagenum,formatstr):
        self.pagecount.set(formatstr % (pagenum))

    def spec_show(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        self.figmgr.toolbar.set_message('spec value: drag on a spec')
        if self.mode == 'spec': return
        self.mode = 'spec'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._select_spectrum)

    def modify_note(self):
        if not self.figmgr.toolbar.mode == '': return
        if self.mode == 'note':
            self.bNote.config(relief='raised')
            self.mode = 'none'
            self.spec_show()
            if not self.button:
                self.notewin.close_widgets()
                self.__disconnect_event()
            return
        self.figmgr.toolbar.set_message('text: select a position/text')
        self.bNote.config(relief='sunken')
        self.bRegion.config(relief='raised')
        self.bPanel.config(relief='raised')
        self.mode = 'note'
        self.__disconnect_event()
        self._p.register('button_press',self._mod_note)

    def select_region(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        if self.mode == 'region':
            self.bRegion.config(relief='raised')
            self.mode = 'none'
            self.spec_show()
            return
        self.figmgr.toolbar.set_message('select regions: click at start and end channels')
        self.bNote.config(relief='raised')
        self.bRegion.config(relief='sunken')
        self.bPanel.config(relief='raised')
        self.mode = 'region'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._add_region)

    def select_panel(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        if self.mode == 'panel':
            self.bPanel.config(relief='raised')
            self.mode = 'none'
            self.spec_show()
            return
        self.figmgr.toolbar.set_message('select spectra: click on subplots')
        self.bNote.config(relief='raised')
        self.bRegion.config(relief='raised')
        self.bPanel.config(relief='sunken')
        self.mode = 'panel'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._add_panel)

    def quit(self):
        self.__disconnect_event()
        self.disable_button()
        self.figmgr.window.wm_withdraw()
        self._p.quit()

    def enable_button(self):
        if self.button: return
        self.bRegion.config(state=Tk.NORMAL)
        self.bPanel.config(state=Tk.NORMAL)
        self.bClear.config(state=Tk.NORMAL)
        self.bFlag.config(state=Tk.NORMAL)
        self.bUnflag.config(state=Tk.NORMAL)
        self.bStat.config(state=Tk.NORMAL)
        self.button = True
        self.spec_show()

    def disable_button(self):
        ## disable buttons which don't work for plottp
        if not self.button: return
        self.bRegion.config(relief='raised')
        self.bPanel.config(relief='raised')
        self.bRegion.config(state=Tk.DISABLED)
        self.bPanel.config(state=Tk.DISABLED)
        self.bClear.config(state=Tk.DISABLED)
        self.bFlag.config(state=Tk.DISABLED)
        self.bUnflag.config(state=Tk.DISABLED)
        self.bStat.config(state=Tk.DISABLED)
        self.bNext.config(state=Tk.DISABLED)
        self.bPrev.config(state=Tk.DISABLED)
        self.button = False
        self.mode = ''
        self.notewin.close_widgets()
        self.__disconnect_event()

    def enable_next(self):
        self.bNext.config(state=Tk.NORMAL)

    def disable_next(self):
        self.bNext.config(state=Tk.DISABLED)

    def enable_prev(self):
        self.bPrev.config(state=Tk.NORMAL)

    def disable_prev(self):
        self.bPrev.config(state=Tk.DISABLED)

    # pause buttons for slow operations
    def _pause_buttons(self,operation="end",msg=""):
        buttons = ["bRegion","bPanel","bClear","bFlag","bUnflag","bStat",
                   "bNote","bQuit"]
        if operation == "start":
            state=Tk.DISABLED
        else:
            state=Tk.NORMAL
        for btn in buttons:
            getattr(self,btn).config(state=state)
        self.figmgr.toolbar.set_message(msg)

    def delete_bar(self):
        self.__disconnect_event()
        self.destroy()

    def __disconnect_event(self):
        self._p.register('button_press',None)
        self._p.register('button_release',None)

    def _draw_span(self,axes,x0,x1,**kwargs):
        height = self._p.figure.bbox.height
        y0 = height - axes.bbox.y0
        y1 = height - axes.bbox.y1
        return self._p.canvas._tkcanvas.create_rectangle(x0,y0,x1,y1,**kwargs)

    def _remove_span(self,span):
        self._p.canvas._tkcanvas.delete(span)
