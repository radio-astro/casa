import os
import matplotlib, numpy
from asap.logging import asaplog, asaplog_post_dec

######################################
##    Add CASA custom toolbar       ##
######################################
class CustomToolbarCommon:
    def __init__(self,parent):
        self.plotter = parent
        #self.figmgr=self.plotter._plotter.figmgr

    ### select the nearest spectrum in pick radius
    ###    and display spectral value on the toolbar.
    def _select_spectrum(self,event):
        # Do not fire event when in zooming/panning mode
        mode = self.figmgr.toolbar.mode
        if not mode == '':
            return
            # When selected point is out of panels
        if event.inaxes == None:
            return
        # If not left button
        if event.button != 1:
            return

        xclick = event.xdata
        yclick = event.ydata
        dist2 = 1000.
        pickline = None
        # If the pannel has picable objects
        pflag = False
        for lin in event.inaxes.lines:
            if not lin.pickable():
                continue
            pflag = True
            flag,pind = lin.contains(event)
            if not flag:
                continue
            # Get nearest point
            inds = pind['ind']
            xlin = lin.get_xdata()
            ylin = lin.get_ydata()
            for i in inds:
                d2=(xlin[i]-xclick)**2+(ylin[i]-yclick)**2
                if dist2 >= d2:
                    dist2 = d2
                    pickline = lin
        # No pickcable line in the pannel
        if not pflag:
            return
        # Pickable but too far from mouse position
        elif pickline is None:
            picked = 'No line selected.'
            self.figmgr.toolbar.set_message(picked)
            return
        del pind, inds, xlin, ylin
        # Spectra are Picked
        theplot = self.plotter._plotter
        thetoolbar = self.figmgr.toolbar
        thecanvas = self.figmgr.canvas
        # Disconnect the default motion notify event
        # Notice! the other buttons are also diabled!!!
        thecanvas.mpl_disconnect(thetoolbar._idDrag)
        # Get picked spectrum
        xdata = pickline.get_xdata()
        ydata = pickline.get_ydata()
        titl = pickline.get_label()
        titp = event.inaxes.title.get_text()
        panel0 = event.inaxes
        picked = "Selected: '"+titl+"' in panel '"+titp+"'."
        thetoolbar.set_message(picked)
        # Generate a navigation window
        #naviwin=Navigationwindow(titp,titl)
        #------------------------------------------------------#
        # Show spectrum data at mouse position
        def spec_data(event):
            # Getting spectrum data of neiboring point
            xclick = event.xdata
            if event.inaxes != panel0:
                return
            ipoint = len(xdata)-1
            for i in range(len(xdata)-1):
                xl = xclick - xdata[i]
                xr = xclick - xdata[i+1]
                if xl*xr <= 0.:
                    ipoint = i
                    break
            # Output spectral value on the navigation window
            posi = '[ %s, %s ]:  x = %.2f   value = %.2f'\
                   %(titl,titp,xdata[ipoint],ydata[ipoint])
            #naviwin.posi.set(posi)
            thetoolbar.set_message(posi)
        #------------------------------------------------------#
        # Disconnect from mouse events
        def discon(event):
            #naviwin.window.destroy()
            theplot.register('motion_notify',None)
            # Re-activate the default motion_notify_event
            thetoolbar._idDrag = thecanvas.mpl_connect('motion_notify_event',
                                                       thetoolbar.mouse_move)
            theplot.register('button_release',None)
            return
        #------------------------------------------------------#
        # Show data value along with mouse movement
        theplot.register('motion_notify',spec_data)
        # Finish events when mouse button is released
        theplot.register('button_release',discon)


    ### Calculate statistics of the selected area.
    def _single_mask(self,event):
        # Do not fire event when in zooming/panning mode
        if not self.figmgr.toolbar.mode == '':
            return
        # When selected point is out of panels
        if event.inaxes == None:
            return
        if event.button == 1:
            baseinv=True
        elif event.button == 3:
            baseinv=False
        else:
            return

        def _calc_stats():
            msk=mymask.get_mask()
            statstr = ['max', 'min', 'mean', 'median', 'sum', 'stddev', 'rms']
            for stat in statstr:
                mymask.scan.stats(stat=stat,mask=msk)

        # Interactive mask definition
        from asap.interactivemask import interactivemask
        mymask = interactivemask(plotter=self.plotter,scan=self.plotter._data)
        # Create initial mask
        mymask.set_basemask(invert=baseinv)
        # Inherit event
        mymask.set_startevent(event)
        # Set callback func
        mymask.set_callback(_calc_stats)
        # Selected mask
        mymask.select_mask(once=True,showmask=False)

    def _mod_note(self,event):
        # Do not fire event when in zooming/panning mode
        if not self.figmgr.toolbar.mode == '':
            return
        if event.button == 1:
            self.notewin.load_textwindow(event)
        elif event.button == 3 and self._note_picked(event):
            self.notewin.load_modmenu(event)
        return

    def _note_picked(self,event):
        # just briefly check if any texts are picked
        for textobj in self.canvas.figure.texts:
            if textobj.contains(event)[0]:
                return True
        for ax in self.canvas.figure.axes:
            for textobj in ax.texts:
                if textobj.contains(event)[0]:
                    return True
        #print "No text picked"
        return False

    ### Page chages
    ### go to the previous page
    def prev_page(self):
        self.figmgr.toolbar.set_message('plotting the previous page')
        #self._pause_buttons(operation="start",msg='plotting the previous page')
        self._new_page(goback=True)
        #self._pause_buttons(operation="end")

    ### go to the next page
    def next_page(self):
        self.figmgr.toolbar.set_message('plotting the next page')
        #self._pause_buttons(operation="start",msg='plotting the next page')
        self._new_page(goback=False)
        #self._pause_buttons(operation="end")

    ### actual plotting of the new page
    def _new_page(self,goback=False):
        top = None
        header = self.plotter._headtext
        reset = False
        doheader = (isinstance(header['textobj'],list) and \
                    len(header['textobj']) > 0)
        if self.plotter._startrow <= 0:
            msg = "The page counter is reset due to chages of plot settings. "
            msg += "Plotting from the first page."
            asaplog.push(msg)
            asaplog.post('WARN')
            reset = True
            goback = False
            if doheader:
                extrastr = selstr = ''
                if header.has_key('extrastr'):
                    extrastr = header['extrastr']
                if header.has_key('selstr'):
                    selstr = header['selstr']
            self.plotter._reset_header()
        if doheader:
            top = self.plotter._plotter.figure.subplotpars.top
            fontsize = header['textobj'][0].get_fontproperties().get_size()

        self.plotter._plotter.hold()
        if goback:
            self._set_prevpage_counter()
        #self.plotter._plotter.clear()
        self.plotter._plot(self.plotter._data)
        self.set_pagecounter(self._get_pagenum())
        # Plot header information
        if header['textobj']:
            if top and top != self.plotter._margins[3]:
                # work around for sdplot in CASA. complete checking in future?
                self.plotter._plotter.figure.subplots_adjust(top=top)
            if reset:
                self.plotter.print_header(plot=True,fontsize=fontsize,selstr=selstr, extrastr=extrastr)
            else:
                self.plotter._header_plot(header['string'],fontsize=fontsize)
        self.plotter._plotter.release()
        self.plotter._plotter.tidy()
        self.plotter._plotter.show(hardrefresh=False)
        del top

    ### calculate the panel ID and start row to plot the previous page
    def _set_prevpage_counter(self):
        # set row and panel counters to those of the 1st panel of previous page
        maxpanel = 16
        # the ID of the last panel in current plot
        lastpanel = self.plotter._ipanel
        # the number of current subplots
        currpnum = len(self.plotter._plotter.subplots)
        # the nuber of previous subplots
        prevpnum = None
        if self.plotter._rows and self.plotter._cols:
            # when user set layout
            prevpnum = self.plotter._rows*self.plotter._cols
        else:
            # no user specification
            prevpnum = maxpanel

        start_ipanel = max(lastpanel-currpnum-prevpnum+1, 0)
        # set the pannel ID of the last panel of prev-prev page
        self.plotter._ipanel = start_ipanel-1
        if self.plotter._panelling == 'r':
            self.plotter._startrow = start_ipanel
        else:
            # the start row number of the next panel
            self.plotter._startrow = self.plotter._panelrows[start_ipanel]
        del lastpanel,currpnum,prevpnum,start_ipanel

    ### refresh the page counter
    ### refresh the page counter
    def set_pagecounter(self,page):
        nwidth = int(numpy.ceil(numpy.log10(max(page,1))))+1
        nwidth = max(nwidth,4)
        formatstr = '%'+str(nwidth)+'d'
        self.show_pagenum(page,formatstr)

    def show_pagenum(self,pagenum,formatstr):
        # passed to backend dependent class
        pass        

    def _get_pagenum(self):
        maxpanel = 16
        # get the ID of last panel in the current page
        idlastpanel = self.plotter._ipanel
        if self.plotter._rows and self.plotter._cols:
            ppp = self.plotter._rows*self.plotter._cols
        else:
            ppp = maxpanel
        return int(idlastpanel/ppp)+1

    # pause buttons for slow operations. implemented at a backend dependent class
    def _pause_buttons(self,operation="end",msg=""):
        pass


#####################################
##    Backend dependent Classes    ##
#####################################
### TkAgg
if matplotlib.get_backend() == 'TkAgg':
    import Tkinter as Tk
    from notationwindow import NotationWindowTkAgg

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
        self.figmgr.toolbar.set_message('text: select a position/text')
        if self.mode == 'note':
            self.bNote.config(relief='raised')
            self.mode = 'none'
            self.spec_show()
            return
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
        #self.bNext.config(state=Tk.DISABLED)
        #self.bPrev.config(state=Tk.DISABLED)
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
