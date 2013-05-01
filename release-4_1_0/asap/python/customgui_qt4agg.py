import os
import matplotlib, numpy
from asap.logging import asaplog, asaplog_post_dec
from matplotlib.patches import Rectangle
from asap.parameters import rcParams
from asap._asap import stmath
from asap.customgui_base import *

import PyQt4 as qt

######################################
##    Add CASA custom toolbar       ##
######################################
class CustomToolbarQT4Agg(CustomToolbarCommon,  qt.QtGui.QToolBar):
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
        self.notewin = NotationWindowQT4Agg(master=self.canvas)
        self._add_custom_toolbar()

    def _add_custom_toolbar(self):
        qt.QtGui.QToolBar.__init__(self,parent=self.figmgr.window)
        self.figmgr.window.addToolBar(qt.QtCore.Qt.BottomToolBarArea,self)
        self.bNote = self._NewButton(master=self,
                                     text='notation',
                                     command=self.modify_note,
                                     balloon="add note on plot")
        self.bNote.setCheckable(True)

        self.bStat = self._NewButton(master=self,
                                     text='statistics',
                                     command=self.stat_cal,
                                     balloon="calculate statistics")
        self.bStat.setCheckable(True)

        # page change oparations
        frPage = qt.QtGui.QWidget(parent=self,flags=qt.QtCore.Qt.Tool)
        loPage = qt.QtGui.QHBoxLayout(self)
        loPage.addStretch(1)
        self.lPagetitle = qt.QtGui.QLabel('Page:',parent=frPage)
        self.lPagetitle.setMargin(5)
        loPage.addWidget(self.lPagetitle)
        self.pagecount = qt.QtGui.QLabel(parent=frPage)
        self.pagecount.setStyleSheet("background-color: white")
        self.pagecount.setMargin(3)
        self.pagecount.setText('   1')
        loPage.addWidget(self.pagecount)
        
        self.bNext = self._NewButton(master=frPage,
                                     text=' + ',
                                     command=self.next_page,
                                     addit=False,
                                     balloon="plot next page")
        loPage.addWidget(self.bNext)
        self.bPrev = self._NewButton(master=frPage,
                                     text=' - ',
                                     command=self.prev_page,addit=False,
                                     balloon="plot previous page")
        loPage.addWidget(self.bPrev)
        frPage.setLayout(loPage)
        self.addWidget(frPage)

        self.bQuit = self._NewButton(master=self,
                                     text='Quit',
                                     command=self.quit,
                                     balloon="Close window")

        self.pagecount.setText(' '*4)

        self.disable_button()
        return

    def _NewButton(self, master, text, command, balloon=None,addit=True):
        b = qt.QtGui.QPushButton(text,parent=master)
        if balloon: b.setToolTip(balloon)
        if addit: master.addWidget(b)
        master.connect(b,qt.QtCore.SIGNAL('clicked()'),command)
        return b

    def show_pagenum(self,pagenum,formatstr):
        self.pagecount.setText(formatstr % (pagenum))

    def spec_show(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        self.figmgr.toolbar.set_message("spec value: drag on a spec")
        if self.mode == 'spec': return
        self.mode = 'spec'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._select_spectrum)

    def stat_cal(self):
        if not self.figmgr.toolbar.mode == '' or not self.button:
            # Get back button status BEFORE clicked
            self.bStat.setChecked(not self.bStat.isChecked())
            return
        if self.mode == 'stat':
            # go back to spec mode
            self.bStat.setChecked(False)
            self.bStat.setToolTip("calculate statistics")
            self.spec_show()
            return
        self.figmgr.toolbar.set_message("statistics: click at start and end channels")
        self.bStat.setChecked(True)
        self.bStat.setToolTip("Back to spec value mode")
        self.bNote.setChecked(False)
        self.bNote.setToolTip("add note on plot")
        self.mode = 'stat'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._single_mask)

    def modify_note(self):
        if not self.figmgr.toolbar.mode == '':
            # Get back button status BEFORE clicked
            self.bNote.setChecked(not self.bNote.isChecked())
            return
        if self.mode == 'note':
            self.bNote.setChecked(False)
            self.bNote.setToolTip("add note on plot")
            self.mode = 'none'
            self.spec_show()
            if not self.button:
                self.notewin.close_widgets()
                self.__disconnect_event()
            return
        self.figmgr.toolbar.set_message("text: select a position/text")
        self.bStat.setChecked(False)
        self.bStat.setToolTip("calculate statistics")
        self.bNote.setChecked(True)
        self.bNote.setToolTip("Back to spec value mode")
        self.mode = 'note'
        self.__disconnect_event()
        self._p.register('button_press',self._mod_note)

    def quit(self):
        self.__disconnect_event()
        self.disable_button()
        self._p.quit()

    def enable_button(self):
        if self.button: return
        self.bStat.setEnabled(True)
        self.button = True
        self.spec_show()

    def disable_button(self):
        if not self.button: return
        self.bStat.setChecked(False)
        self.bStat.setDisabled(True)
        self.button = False
        self.mode = ''
        self.__disconnect_event()

    def enable_next(self):
        self.bNext.setEnabled(True)

    def disable_next(self):
        self.bNext.setDisabled(True)

    def enable_prev(self):
        self.bPrev.setEnabled(True)

    def disable_prev(self):
        self.bPrev.setDisabled(True)

    # pause buttons for slow operations
    def _pause_buttons(self,operation="end",msg=""):
        buttons = ["bStat","bNote","bQuit"]
        if operation == "start":
            enable = False
        else:
            enable = True
        for btn in buttons:
            getattr(self,btn).setEnabled(enable)
        self.figmgr.toolbar.set_message(msg)

    def delete_bar(self):
        self.__disconnect_event()
        self.destroy()

    def __disconnect_event(self):
        self._p.register('button_press',None)
        self._p.register('button_release',None)

    def _draw_span(self,axes,x0,x1,**kwargs):
        height = self._p.figure.bbox.height
        y1 = height - axes.bbox.y1
        h = axes.bbox.height
        w = abs(x1 - x0)
        rect = [ int(val) for val in min(x0,x1), y1, w, h ]
        self._p.canvas.drawRectangle( rect )
        # nothing is returned by drawRectangle
        return None

    def _remove_span(self,span):
        # Nothing to do with remove. just refresh (call only once)
        self.canvas.draw()



######################################
##    Notation box window           ##
######################################
class NotationWindowQT4Agg(NotationWindowCommon):
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
        self.parent = master
        NotationWindowCommon.__init__(self,master=master)
        self.anchval = None
        self.textwin = self._create_textwindow(master=None)
        self.menu = self._create_modmenu(master=self.parent)

    ### Notation window widget
    def _create_textwindow(self,master=None):
        """Create notation window widget and iconfy it"""
        #twin = qt.QtGui.QWidget(parent=master, flags=qt.QtCore.Qt.Popup)
        twin = qt.QtGui.QWidget(parent=master, flags=qt.QtCore.Qt.Dialog)
        twin.setWindowTitle("Notation")
        self.textbox = self._NotationBox(parent=twin)
        radiobox = self._AnchorRadio(parent=twin)
        self.actionbs = self._ActionButtons(parent=twin)
        vbox = qt.QtGui.QVBoxLayout(twin)
        vbox.addWidget(self.textbox)
        vbox.addWidget(radiobox)
        vbox.addLayout(self.actionbs)
        twin.setLayout(vbox)
        #twin.setCentralWidget(self.textbox)
        twin.hide()
        return twin

    def _NotationBox(self,parent=None):
        textbox = qt.QtGui.QPlainTextEdit(parent=parent)
        textbox.setStyleSheet("background-color: white")
        fmetric = qt.QtGui.QFontMetrics(textbox.currentCharFormat().font())
        textbox.resize(fmetric.width("A")*20+fmetric.leading()*2,
                       fmetric.height()*2+fmetric.ascent()+fmetric.descent())
        del fmetric
        textbox.setMinimumSize(textbox.size())
        textbox.setUndoRedoEnabled(True)
        textbox.setMidLineWidth(3)
        textbox.setFrameShadow(qt.QtGui.QFrame.Sunken)
        textbox.setCursor(qt.QtCore.Qt.IBeamCursor)
        textbox.setFocus()
        return textbox

    def _AnchorRadio(self,parent=None):
        # Returns a QGoupBox object which includes radio butons to
        # select an anchor
        anchbox = qt.QtGui.QGroupBox("anchor",parent=parent)
        self.radio = qt.QtGui.QButtonGroup(parent=anchbox)
        self.rFig = self._NewRadioButton(anchbox,"figure",\
                                         bgr=self.radio,value=0,\
                                         balloon="a fixed position in figure")
        self.rAxis = self._NewRadioButton(anchbox,"panel",\
                                          bgr=self.radio,value=1,\
                                          balloon="a fixed realtive position in subplot")
        self.rData = self._NewRadioButton(anchbox,"data",\
                                          bgr=self.radio,value=2,\
                                          balloon="a fixed data position in subplot")
        hbox = qt.QtGui.QHBoxLayout(anchbox)
        hbox.addWidget(self.rFig)
        hbox.addWidget(self.rAxis)
        hbox.addWidget(self.rData)
        anchbox.setLayout(hbox)
        # set initial selection "figure"
        self.rFig.setChecked(True)
        self.radio.setExclusive(True)
        self.anchval = self.radio.checkedId()
        return anchbox

    def _NewRadioButton(self,parent,text,balloon=None,bgr=None,value=None):
        rb= qt.QtGui.QRadioButton(text,parent=parent)
        if bgr:
            if value is not None:
                bgr.addButton(rb,value)
            else:
                bgr.addButton(rb)
        if balloon: rb.setToolTip(balloon)
        return rb

    def _enable_radio(self):
        """Enable 'panel' and 'data' radio button"""
        self.rAxis.setEnabled(True)
        self.rData.setEnabled(True)
        # select Figure as the default value
        self.rFig.setChecked(True)
        self.anchval = self.radio.checkedId()

    def _reset_radio(self):
        """Disable 'panel' and 'data' radio button"""
        self.rAxis.setDisabled(True)
        self.rData.setDisabled(True)
        self.rFig.setEnabled(True)
        # select Figure as the default value
        self.rFig.setChecked(True)
        self.anchval = self.radio.checkedId()

    def _select_radio(self,selection):
        """Select a specified radio button"""
        if not selection in self.anchors:
            return
        if selection == "data":
            self.rData.setChecked(True)
        elif selection == "axes":
            self.rAxis.setChecked(True)
        else:
            self.rFig.setChecked(True)
        self.anchval = self.radio.checkedId()

    def _get_anchval(self):
        """Returns a integer of a selected radio button"""
        self.anchval = self.radio.checkedId()
        return self.anchval

    def _get_note(self):
        """Returns a note string specified in the text box"""
        return str(self.textbox.toPlainText())

    def _clear_textbox(self):
        """Clear the text box"""
        self.textbox.clear()

    def _set_note(self,note=None):
        """Set a note string to the text box"""
        self._clear_textbox()
        if len(note) >0:
            self.textbox.setPlainText(note)

    def _ActionButtons(self,parent=None):
        # Returns a layout object which includes "cancel" and "print" buttons
        actbuts = qt.QtGui.QHBoxLayout()
        bCancel = self._NewButton(parent,"cancel",self._cancel_text,\
                                  addit=False,\
                                  balloon="cancel printing/modifying")
        bPrint = self._NewButton(parent,"print", self._print_text,\
                                 addit=False,\
                                 balloon="print text on plot")
        actbuts.addWidget(bCancel)
        actbuts.addWidget(bPrint)
        return actbuts

    def _NewButton(self, parent, text, command, balloon=None, addit=True):
        b = qt.QtGui.QPushButton(text,parent=parent)
        if balloon: b.setToolTip(balloon)
        if addit: parent.addWidget(b)
        parent.connect(b,qt.QtCore.SIGNAL('clicked()'),command)
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
        if event.canvas != self.parent:
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
            enableaxes:   False (default) or True.
                          If True, 'panel' & 'data' radio button is enabled. 
        """
        if not self.textwin: return
        self._reset_radio()
        if enableaxes: 
            self._enable_radio()
        self.textwin.activateWindow()
        h = self.textwin.minimumHeight()
        w = self.textwin.minimumWidth()
        self.textwin.resize(w,h)
        self.textwin.move(xpix,ypix)
        self.textbox.setFocus()
        self.textwin.raise_()
        self.textwin.show()
        if w*h <= 1: # Initial load
            self.textwin.setMinimumSize(self.textwin.size())

    def close_textwindow(self):
        """Close text window."""
        self.seltext = {}
        self._reset_radio()
        self._clear_textbox()
        self.textwin.hide()


    ### Modify/Delete menu widget
    def _create_modmenu(self,master=None):
        """Create modify/delete menu widget"""
        if master:
            self.parent = master
        if not self.parent:
            return False
        menu = qt.QtGui.QMenu(parent=self.parent)
        menu.setTearOffEnabled(False)
        menu.addAction("Modify",self._modify_note)
        menu.addAction("Delete",self._delnote_dialog)
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
            canvas = event.canvas
            corig = canvas.mapToGlobal(qt.QtCore.QPoint(0,0))
            xpixs = corig.x() + int(event.x)
            ypixs = corig.y() + canvas.height() - int(event.y)
            self.menu.activateWindow()
            self.menu.move(xpixs,ypixs)
            self.menu.show()

    def close_modmenu(self):
        """Close cascade menu."""
        self.seltext = {}
        self.menu.hide()

    ### load text window for modification 
    def _modify_note(self):
        """helper function to load text window to modify selected note"""
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
        from PyQt4.QtGui import QMessageBox as mbox
        answer = mbox.question(self.parent,"Delete?",remind,
                               buttons = mbox.Ok | mbox.Cancel,
                               defaultButton=mbox.Cancel)
        if answer == mbox.Ok:
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
        corig = self.parent.mapToGlobal(qt.QtCore.QPoint(0,0))
        xpixs = corig.x() + xpixd
        ypixs = corig.y() + self.parent.height() - ypixd
        return (int(xpixs), int(ypixs))
        





###########################################
##    Add CASA custom Flag toolbar       ##
###########################################
class CustomFlagToolbarQT4Agg(CustomFlagToolbarCommon,  qt.QtGui.QToolBar):
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
        self.notewin=NotationWindowQT4Agg(master=self.canvas)
        self._add_custom_toolbar()

    def _add_custom_toolbar(self):
        qt.QtGui.QToolBar.__init__(self,parent=self.figmgr.window)
        self.figmgr.window.addToolBar(qt.QtCore.Qt.BottomToolBarArea,self)
        self.bRegion = self._NewButton(master=self,
                                       text='region',
                                       command=self.select_region,
                                       balloon="select channel regions")
        self.bRegion.setCheckable(True)
        
        self.bPanel = self._NewButton(master=self,
                                      text='panel',
                                      command=self.select_panel,
                                      balloon="select a whole spectrum")
        self.bPanel.setCheckable(True)

        self.bClear = self._NewButton(master=self,
                                      text='clear',
                                      command=self.cancel_select,
                                      balloon="clear selections")

        self.bFlag = self._NewButton(master=self,
                                     text='flag',
                                     command=self.flag,
                                     balloon="flag selections")

        self.bUnflag = self._NewButton(master=self,
                                       text='unflag',
                                       command=self.unflag,
                                       balloon="unflag selections")

        self.bStat = self._NewButton(master=self,
                                     text='statistics',
                                     command=self.stat_cal,
                                     balloon="print statistics of selections")

        self.bNote = self._NewButton(master=self,
                                     text='notation',
                                     command=self.modify_note,
                                     balloon="add note on plot")
        self.bNote.setCheckable(True)

        # page change oparations
        frPage = qt.QtGui.QWidget(parent=self,flags=qt.QtCore.Qt.Tool)
        loPage = qt.QtGui.QHBoxLayout(self)
        loPage.addStretch(1)
        self.lPagetitle = qt.QtGui.QLabel('Page:',parent=frPage)
        self.lPagetitle.setMargin(5)
        loPage.addWidget(self.lPagetitle)
        self.pagecount = qt.QtGui.QLabel(parent=frPage)
        self.pagecount.setStyleSheet("background-color: white")
        self.pagecount.setMargin(3)
        self.pagecount.setText('   1')
        loPage.addWidget(self.pagecount)

        self.bNext = self._NewButton(master=frPage,
                                     text='+',
                                     #imagename="hand.ppm",
                                     command=self.next_page,
                                     addit=False,
                                     balloon="plot next page")
        loPage.addWidget(self.bNext)
        self.bPrev = self._NewButton(master=frPage,
                                     text='-',
                                     command=self.prev_page,
                                     addit=False,
                                     balloon="plot previous page")
        loPage.addWidget(self.bPrev)
        frPage.setLayout(loPage)
        self.addWidget(frPage)

        self.bQuit = self._NewButton(master=self,
                                     text='Quit',
                                     #imagename="stock_close.ppm",
                                     command=self.quit,
                                     balloon="Close window")

        self.pagecount.setText(' '*4)
        self.disable_button()
        return

    def _NewButton(self, master, text, command, balloon=None,addit=True,imagename=None):
        img = None
        if imagename:
            imagename = os.path.join(matplotlib.rcParams['datapath'], 'images', imagename)
#             img = Tk.PhotoImage(master=master, file=imagename)

        b = qt.QtGui.QPushButton(text,parent=master)
        if balloon: b.setToolTip(balloon)
        if addit: master.addWidget(b)
        master.connect(b,qt.QtCore.SIGNAL('clicked()'),command)
#         if img: b.image = img
        return b

    def show_pagenum(self,pagenum,formatstr):
        self.pagecount.setText(formatstr % (pagenum))

    def spec_show(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        self.figmgr.toolbar.set_message('spec value: drag on a spec')
        if self.mode == 'spec': return
        self.mode = 'spec'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._select_spectrum)

    def modify_note(self):
        if not self.figmgr.toolbar.mode == '':
            # Get back button status BEFORE clicked
            self.bNote.setChecked(not self.bNote.isChecked())
            return
        if self.mode == 'note':
            self.bNote.setChecked(False)
            self.bNote.setToolTip("add note on plot")
            self.mode = 'none'
            self.spec_show()
            if not self.button:
                self.notewin.close_widgets()
                self.__disconnect_event()
            return
        self.figmgr.toolbar.set_message('text: select a position/text')
        self.bNote.setChecked(True)
        self.bNote.setToolTip("Back to spec value mode")
        self.bRegion.setChecked(False)
        self.bRegion.setToolTip("select channel regions")
        self.bPanel.setChecked(False)
        self.bPanel.setToolTip("select a whole spectrum")
        self.mode = 'note'
        self.__disconnect_event()
        self._p.register('button_press',self._mod_note)

    def select_region(self):
        if not self.figmgr.toolbar.mode == '' or not self.button:
            # Get back button status BEFORE clicked
            self.bRegion.setChecked(not self.bRegion.isChecked())
        if self.mode == 'region':
            self.bRegion.setChecked(False)
            self.bRegion.setToolTip("select channel regions")
            self.mode = 'none'
            self.spec_show()
            return
        self.figmgr.toolbar.set_message('select regions: click at start and end channels')
        self.bNote.setChecked(False)
        self.bNote.setToolTip("add note on plot")
        self.bRegion.setChecked(True)
        self.bRegion.setToolTip("Back to spec value mode")
        self.bPanel.setChecked(False)
        self.bPanel.setToolTip("select a whole spectrum")
        self.mode = 'region'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._add_region)

    def select_panel(self):
        if not self.figmgr.toolbar.mode == '' or not self.button:
            # Get back button status BEFORE clicked
            self.bPanel.setChecked(not self.bPanel.isChecked())
            return
        if self.mode == 'panel':
            self.bPanel.setChecked(False)
            self.bPanel.setToolTip("select subplots")
            self.mode = 'none'
            self.spec_show()
            return
        self.figmgr.toolbar.set_message('select spectra: click on subplots')
        self.bNote.setChecked(False)
        self.bNote.setToolTip("add note on plot")
        self.bRegion.setChecked(False)
        self.bRegion.setToolTip("select channel regions")
        self.bPanel.setChecked(True)
        self.bPanel.setToolTip("Back to spec value mode")
        self.mode = 'panel'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._add_panel)

    def quit(self):
        self.__disconnect_event()
        self.disable_button()
        self._p.unmap()

    def enable_button(self):
        if self.button: return
        self.bRegion.setEnabled(True)
        self.bPanel.setEnabled(True)
        self.bClear.setEnabled(True)
        self.bFlag.setEnabled(True)
        self.bUnflag.setEnabled(True)
        self.bStat.setEnabled(True)
        self.button = True
        self.spec_show()

    def disable_button(self):
        ## disable buttons which don't work for plottp
        if not self.button: return
        self.bRegion.setChecked(False)
        self.bRegion.setToolTip("select channel regions")
        self.bPanel.setChecked(False)
        self.bPanel.setToolTip("select subplots")

        self.bRegion.setDisabled(True)
        self.bPanel.setDisabled(True)
        self.bClear.setDisabled(True)
        self.bFlag.setDisabled(True)
        self.bUnflag.setDisabled(True)
        self.bStat.setDisabled(True)
        self.bNext.setDisabled(True)
        self.bPrev.setDisabled(True)
        self.button = False
        self.mode = ''
        self.notewin.close_widgets()
        self.__disconnect_event()

    def enable_next(self):
        self.bNext.setEnabled(True)

    def disable_next(self):
        self.bNext.setDisabled(True)

    def enable_prev(self):
        self.bPrev.setEnabled(True)

    def disable_prev(self):
        self.bPrev.setDisabled(True)

    # pause buttons for slow operations
    def _pause_buttons(self,operation="end",msg=""):
        buttons = ["bRegion","bPanel","bClear","bFlag","bUnflag","bStat",
                   "bNote","bQuit"]
        if operation == "start":
            enable = False
        else:
            enable = True
        for btn in buttons:
            getattr(self,btn).setEnabled(enable)
        self.figmgr.toolbar.set_message(msg)

    def delete_bar(self):
        self.__disconnect_event()
        self.destroy()

    def __disconnect_event(self):
        self._p.register('button_press',None)
        self._p.register('button_release',None)

    def _draw_span(self,axes,x0,x1,**kwargs):
        height = self._p.figure.bbox.height
        y1 = height - axes.bbox.y1
        h = axes.bbox.height
        w = abs(x1 - x0)
        rect = [ int(val) for val in min(x0,x1), y1, w, h ]
        self._p.canvas.drawRectangle( rect )
        # nothing is returned by drawRectangle
        return None

    def _remove_span(self,span):
        # Nothing to do with remove.
        pass
