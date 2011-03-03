import os
import matplotlib, numpy
from matplotlib.patches import Rectangle
from asap.parameters import rcParams
from asap.logging import asaplog, asaplog_post_dec
from asap import scantable
from asap._asap import stmath

###########################################
##    Add CASA custom Flag toolbar       ##
###########################################
class CustomFlagToolbarCommon:
    def __init__(self,parent):
        self.plotter=parent
        #self.figmgr=self.plotter._plotter.figmgr
        self._selregions = {}
        self._selpanels = []
        self._polygons = []
        self._thisregion = None
        self.xdataold=None
        
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

        xclick=event.xdata
        yclick=event.ydata
        dist2=1000.
        pickline=None
        # If the pannel has picable objects
        pflag=False
        for lin in event.inaxes.lines:
            if not lin.pickable():
                continue
            pflag=True
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
            picked='No line selected.'
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
        titl=pickline.get_label()
        titp=event.inaxes.title.get_text()
        panel0=event.inaxes
        picked="Selected: '"+titl+"' in panel '"+titp+"'."
        thetoolbar.set_message(picked)
        # Generate a navigation window
        #naviwin=Navigationwindow(titp,titl)
        #------------------------------------------------------#
        # Show spectrum data at mouse position
        def spec_data(event):
            # Getting spectrum data of neiboring point
            xclick=event.xdata
            if event.inaxes != panel0:
                return
            ipoint=len(xdata)-1
            for i in range(len(xdata)-1):
                xl=xclick-xdata[i]
                xr=xclick-xdata[i+1]
                if xl*xr <= 0.:
                    ipoint = i
                    break
            # Output spectral value on the navigation window
            posi='[ %s, %s ]:  x = %.2f   value = %.2f'\
                  %(titl,titp,xdata[ipoint],ydata[ipoint])
            #naviwin.posi.set(posi)
            thetoolbar.set_message(posi)
        #------------------------------------------------------#
        # Disconnect from mouse events
        def discon(event):
            #naviwin.window.destroy()
            theplot.register('motion_notify',None)
            # Re-activate the default motion_notify_event
            thetoolbar._idDrag=thecanvas.mpl_connect('motion_notify_event',
                                                     thetoolbar.mouse_move)
            theplot.register('button_release',None)
            return
        #------------------------------------------------------#
        # Show data value along with mouse movement
        theplot.register('motion_notify',spec_data)
        # Finish events when mouse button is released
        theplot.register('button_release',discon)


    ### Notation
    def _mod_note(self,event):
        # Do not fire event when in zooming/panning mode
        if not self.figmgr.toolbar.mode == '':
            return
        if event.button ==1:
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

    ### Region/Panel selection & oparations
    ### add regions to selections
    @asaplog_post_dec
    def _add_region(self,event):
        if not self.figmgr.toolbar.mode == '':
            return
        if event.button != 1 or event.inaxes == None:
            return
        # this row resolution assumes row panelling
        irow = int(self._getrownum(event.inaxes))
        if irow in self._selpanels:
            msg = "The whole spectrum is already selected"
            asaplog.post()
            asaplog.push(msg)
            asaplog.post('WARN')
            return
        #print "add the region to selections"
        self._thisregion = {'axes': event.inaxes,'xs': event.xdata,
                            'worldx': [event.xdata,event.xdata]}
        self.plotter._plotter.register('button_press',None)
        self.plotter._plotter.register('motion_notify', self._xspan_draw)
        self.plotter._plotter.register('button_press', self._xspan_end)

    def _xspan_draw(self,event):
        if event.inaxes == self._thisregion['axes']:
            xnow = event.xdata
            self.xdataold = xnow
        else:
            xnow = self.xdataold
        try: self.lastspan
        except AttributeError: pass
        else: self.lastspan.remove()

        self.lastspan = self._thisregion['axes'].axvspan(self._thisregion['xs'],xnow,facecolor='0.7')
        #self.plotter._plotter.show(False)
        self.plotter._plotter.canvas.draw()
        del xnow

    def _xspan_end(self,event):
        if not self.figmgr.toolbar.mode == '':
            return
        if event.button != 1:
            return
        
        try: self.lastspan
        except AttributeError: pass
        else:
            self.lastspan.remove()
            del self.lastspan
        if event.inaxes == self._thisregion['axes']:
            xdataend = event.xdata
        else:
            xdataend=self.xdataold

        self._thisregion['worldx'][1] = xdataend
        lregion = self._thisregion['worldx']
        pregion = self._thisregion['axes'].axvspan(lregion[0],lregion[1],
                                                   facecolor='0.7')
        #self.plotter._plotter.show(False)
        self.plotter._plotter.canvas.draw()
        self._polygons.append(pregion)
        srow = self._getrownum(self._thisregion['axes'])
        irow = int(srow)
        if not self._selregions.has_key(srow):
            self._selregions[srow] = []
        self._selregions[srow].append(lregion)
        del lregion, pregion, xdataend
        sout = "selected region: "+str(self._thisregion['worldx'])+\
              "(@row "+str(self._getrownum(self._thisregion['axes']))+")"
        asaplog.push(sout)

        # release event
        self.plotter._plotter.register('button_press',None)
        self.plotter._plotter.register('motion_notify',None)
        # Clear up region selection
        self._thisregion = None
        self.xdataold = None
        # finally recover region selection event
        self.plotter._plotter.register('button_press',self._add_region)

    ### add panels to selections
    @asaplog_post_dec
    def _add_panel(self,event):
        if not self.figmgr.toolbar.mode == '':
            return
        if event.button != 1 or event.inaxes == None:
            return
        selax = event.inaxes
        # this row resolution assumes row panelling
        srow = self._getrownum(selax)
        irow = int(srow)
        if srow:
            self._selpanels.append(irow)
        shadow = Rectangle((0,0),1,1,facecolor='0.7',transform=selax.transAxes,visible=True)
        self._polygons.append(selax.add_patch(shadow))
        #self.plotter._plotter.show(False)
        self.plotter._plotter.canvas.draw()
        asaplog.push("row "+str(irow)+" is selected")
        #print "selected rows =",self._selpanels
        ## check for region selection of the spectra and overwrite it.
        ##!!!! currently disabled for consistency with flag tools !!!!
        #if self._selregions.has_key(srow):
        #    self._selregions.pop(srow)
        #    msg = "The whole spectrum is selected for row="+srow+". Region selection will be overwritten."
        #    asaplog.push(msg)

    def _getrownum(self,axis):
        ### returns the row number of selected spectrum as a string ###
        plabel = axis.get_title()
        if plabel.startswith("row "):
            return plabel.strip("row ")
        return None

    def _any_selection(self):
        ### returns if users have selected any spectrum or region ###
        if len(self._selpanels) or len(self._selregions):
            return True
        return False

    def _plot_selections(self,regions=None,panels=None):
        ### mark panels/spectra selections in the page
        if not self._any_selection() and not (regions or panels):
            return
        regions = regions or self._selregions.copy() or {}
        panels = panels or self._selpanels or []
        if not isinstance(regions,dict):
            asaplog.post()
            asaplog.push("Invalid region specification")
            asaplog.post('ERROR')
        if not isinstance(panels,list):
            asaplog.post()
            asaplog.push("Invalid panel specification")
            asaplog.post('ERROR')
        strow = self._getrownum(self.plotter._plotter.subplots[0]['axes'])
        enrow = self._getrownum(self.plotter._plotter.subplots[-1]['axes'])
        for irow in range(int(strow),int(enrow)+1):
            if regions.has_key(str(irow)):
                ax = self.plotter._plotter.subplots[irow - int(strow)]['axes']
                mlist = regions.pop(str(irow))
                for i in range(len(mlist)):
                    self._polygons.append(ax.axvspan(mlist[i][0],mlist[i][1],
                                                     facecolor='0.7'))
                del ax,mlist
            if irow in panels:
                ax = self.plotter._plotter.subplots[irow - int(strow)]['axes']
                shadow = Rectangle((0,0),1,1,facecolor='0.7',
                                   transform=ax.transAxes,visible=True)
                self._polygons.append(ax.add_patch(shadow))
                del ax,shadow
        #self.plotter._plotter.show(False)
        self.plotter._plotter.canvas.draw()
        del regions,panels,strow,enrow

    def _clear_selection_plot(self):
        ### clear up polygons which mark selected spectra and regions ###
        if len(self._polygons) > 0:
            for shadow in self._polygons:
                shadow.remove()
            self.plotter._plotter.canvas.draw()
        self._polygons = []

    def _clearup_selections(self):
        # clear-up selection and polygons
        self._selpanels = []
        self._selregions = {}
        self._clear_selection_plot()

    ### clear up selections
    def cancel_select(self):
        self.figmgr.toolbar.set_message('selections canceled')
        # clear-up selection and polygons
        self._clearup_selections()

    ### flag selected spectra/regions
    @asaplog_post_dec
    def flag(self):
        if not self._any_selection():
            msg = "No selection to be Flagged"
            asaplog.post()
            asaplog.push(msg)
            asaplog.post('WARN')
            return
        self._flag_operation(rows=self._selpanels,
                             regions=self._selregions,unflag=False)
        sout = "Flagged:\n"
        sout += "  rows = "+str(self._selpanels)+"\n"
        sout += "  regions: "+str(self._selregions)
        asaplog.push(sout)
        del sout
        self._clearup_selections()
        self._plot_page(pagemode="current")

    ### unflag selected spectra/regions
    @asaplog_post_dec
    def unflag(self):
        if not self._any_selection():
            msg = "No selection to be Flagged"
            asaplog.push(msg)
            asaplog.post('WARN')
            return
        self._flag_operation(rows=self._selpanels,
                             regions=self._selregions,unflag=True)
        sout = "Unflagged:\n"
        sout += "  rows = "+str(self._selpanels)+"\n"
        sout += "  regions: "+str(self._selregions)
        asaplog.push(sout)
        del sout
        self._clearup_selections()
        self._plot_page(pagemode="current")

    ### actual flag operation
    @asaplog_post_dec
    def _flag_operation(self,rows=None,regions=None,unflag=False):
        scan = self.plotter._data
        if not scan:
            asaplog.post()
            asaplog.push("Invalid scantable")
            asaplog.post("ERROR")
        if isinstance(rows,list) and len(rows) > 0:
            scan.flag_row(rows=rows,unflag=unflag)
        if isinstance(regions,dict) and len(regions) > 0:
            for srow, masklist in regions.iteritems():
                if not isinstance(masklist,list) or len(masklist) ==0:
                    msg = "Ignoring invalid region selection for row = "+srow
                    asaplog.post()
                    asaplog.push(msg)
                    asaplog.post("WARN")
                    continue
                irow = int(srow)
                mask = scan.create_mask(masklist,invert=False,row=irow)
                scan.flag(row=irow,mask=mask,unflag=unflag)
                del irow, mask
            del srow, masklist
        del scan

    ### show statistics of selected spectra/regions
    @asaplog_post_dec
    def stat_cal(self):
        if not self._any_selection():
            msg = "No selection to be calculated"
            asaplog.push(msg)
            asaplog.post('WARN')
            return
        #print "Calculate statistics of selected region and spectra"
        self._selected_stats(rows=self._selpanels,regions=self._selregions)
        #print "Statistics: "
        #print "- rows: ",self._selpanels
        #print "- regions:\n   ",self._selregions
        self._clearup_selections()

    @asaplog_post_dec
    def _selected_stats(self,rows=None,regions=None):
        scan = self.plotter._data
        if not scan:
            asaplog.post()
            asaplog.push("Invalid scantable")
            asaplog.post("ERROR")
        mathobj = stmath( rcParams['insitu'] )
        statval={}
        statstr = ['max', 'min', 'mean', 'median', 'sum', 'stddev', 'rms']
        if isinstance(rows,list) and len(rows) > 0:
            for irow in rows:
                for stat in statstr:
                    #statval[stat] = scan.stats(stat=stat,row=irow)[0]
                    statval[stat] = mathobj._statsrow(scan,[],stat,irow)[0]
                self._print_stats(scan,irow,statval,statstr=statstr)
            del irow
        if isinstance(regions,dict) and len(regions) > 0:
            for srow, masklist in regions.iteritems():
                if not isinstance(masklist,list) or len(masklist) ==0:
                    msg = "Ignoring invalid region selection for row = "+srow
                    asaplog.post()
                    asaplog.push(msg)
                    asaplog.post("WARN")
                    continue
                irow = int(srow)
                mask = scan.create_mask(masklist,invert=False,row=irow)
                for stat in statstr:
                    #statval[stat] = scan.stats(stat=stat,row=irow,mask = mask)[0]
                    statval[stat] = mathobj._statsrow(scan,mask,stat,irow)[0]
                self._print_stats(scan,irow,statval,statstr=statstr,mask=masklist)
                del irow, mask
            del srow, masklist
        del scan, statval, mathobj

    @asaplog_post_dec
    def _print_stats(self,scan,row,stats,statstr=None,mask=None):
        if not isinstance(scan, scantable):
            asaplog.post()
            asaplog.push("Invalid scantable")
            asaplog.post("ERROR")
        if row < 0 or row > scan.nrow():
            asaplog.post()
            asaplog.push("Invalid row number")
            asaplog.post("ERROR")
        if not isinstance(stats,dict) or len(stats) == 0:
            asaplog.post()
            asaplog.push("Invalid statistic value")
            asaplog.post("ERROR")
        maskstr = "All"
        if mask:
            maskstr = str(mask)
        ssep = "-"*70+"\n"
        sout = ssep
        sout += ("Row=%d  Scan=%d  IF=%d  Pol=%d  Time=%s  mask=%s" % \
                 (row, scan.getscan(row), scan.getif(row), scan.getpol(row), scan.get_time(row),maskstr))
        sout += "\n"
        statvals = []
        if not len(statstr):
            statstr = stats.keys()
        for key in statstr:
            sout += key.ljust(10)
            statvals.append(stats.pop(key))
        sout += "\n"
        sout += ("%f "*len(statstr) % tuple(statvals))
        sout += "\n"+ssep
        asaplog.push(sout)
        del sout, ssep, maskstr, statvals, key, scan, row, stats, statstr, mask

    ### Page chages
    ### go to the previous page
    def prev_page(self):
        self.figmgr.toolbar.set_message('plotting the previous page')
        self._clear_selection_plot()
        self._plot_page(pagemode="prev")
        self._plot_selections()

    ### go to the next page
    def next_page(self):
        self.figmgr.toolbar.set_message('plotting the next page')
        self._clear_selection_plot()
        self._plot_page(pagemode="next")
        self._plot_selections()

    ### actual plotting of the new page
    def _plot_page(self,pagemode="next"):
        if self.plotter._startrow <= 0:
            msg = "The page counter is reset due to chages of plot settings. "
            msg += "Plotting from the first page."
            asaplog.post()
            asaplog.push(msg)
            asaplog.post('WARN')
            goback = False
            
        self.plotter._plotter.hold()
        self.plotter._plotter.legend(1)
        #if goback:
        #    self._set_prevpage_counter()
        self._set_plot_counter(pagemode)
        #self.plotter._plotter.clear()
        self.plotter._plot(self.plotter._data)
        self.set_pagecounter(self._get_pagenum())
        self.plotter._plotter.release()
        self.plotter._plotter.tidy()
        self.plotter._plotter.show(hardrefresh=False)

    ### calculate the panel ID and start row to plot a page
    #def _set_prevpage_counter(self):
    def _set_plot_counter(self, pagemode):
        ## page operation should be either "previous", "current", or "next"
        availpage = ["p","c","n"]
        pageop = pagemode[0].lower()
        if not (pageop in availpage):
            asaplog.post()
            asaplog.push("Invalid page operation")
            asaplog.post("ERROR")
        if pageop == "n":
            # nothing necessary to plot the next page
            return
        # set row and panel counters to those of the 1st panel of previous page
        maxpanel = 16
        # the ID of the last panel in current plot
        lastpanel = self.plotter._ipanel
        # the number of current subplots
        currpnum = len(self.plotter._plotter.subplots)
        
        # the nuber of previous subplots
        start_ipanel = None
        if pageop == "c":
            start_ipanel = max(lastpanel-currpnum+1, 0)
        else:
            ## previous page
            prevpnum = None
            if self.plotter._rows and self.plotter._cols:
                # when user set layout
                prevpnum = self.plotter._rows*self.plotter._cols
            else:
                # no user specification
                prevpnum = maxpanel
            start_ipanel = max(lastpanel-currpnum-prevpnum+1, 0)
            del prevpnum

        # set the pannel ID of the last panel of the prev(-prev) page
        self.plotter._ipanel = start_ipanel-1
        if self.plotter._panelling == 'r':
            self.plotter._startrow = start_ipanel
        else:
            # the start row number of the next panel
            self.plotter._startrow = self.plotter._panelrows[start_ipanel]
        del lastpanel,currpnum,start_ipanel

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

#####################################
##    Backend dependent Classes    ##
#####################################
### TkAgg
if matplotlib.get_backend() == 'TkAgg':
    import Tkinter as Tk
    from notationwindow import NotationWindowTkAgg

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
        #self.bSpec=self._NewButton(master=self,
        #                           text='spec value',
        #                           command=self.spec_show)

        self.bRegion=self._NewButton(master=self,
                                   text='region',
                                   command=self.select_region)
        self.bPanel=self._NewButton(master=self,
                                   text='panel',
                                   command=self.select_panel)
        self.bClear=self._NewButton(master=self,
                                   text='clear',
                                   command=self.cancel_select)
        self.bFlag=self._NewButton(master=self,
                                   text='flag',
                                   command=self.flag)
        self.bUnflag=self._NewButton(master=self,
                                   text='unflag',
                                   command=self.unflag)
        
        self.bStat=self._NewButton(master=self,
                                   text='statistics',
                                   command=self.stat_cal)

        self.bNote=self._NewButton(master=self,
                                   text='notation',
                                   command=self.modify_note)

        self.bQuit=self._NewButton(master=self,
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
        self.mode='spec'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._select_spectrum)

    def modify_note(self):
        if not self.figmgr.toolbar.mode == '': return
        self.figmgr.toolbar.set_message('text: select a position/text')
        if self.mode == 'note':
            self.bNote.config(relief='raised')
            self.mode='none'
            self.spec_show()
            return
        self.bNote.config(relief='sunken')
        self.bRegion.config(relief='raised')
        self.bPanel.config(relief='raised')
        self.mode='note'
        self.__disconnect_event()
        self._p.register('button_press',self._mod_note)

    def select_region(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        self.figmgr.toolbar.set_message('select rectangle regions')
        if self.mode == 'region':
            self.bRegion.config(relief='raised')
            self.mode='none'
            self.spec_show()
            return
        self.bNote.config(relief='raised')
        self.bRegion.config(relief='sunken')
        self.bPanel.config(relief='raised')
        self.mode='region'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._add_region)

    def select_panel(self):
        if not self.figmgr.toolbar.mode == '' or not self.button: return
        self.figmgr.toolbar.set_message('select panels')
        if self.mode == 'panel':
            self.bPanel.config(relief='raised')
            self.mode='none'
            self.spec_show()
            return
        self.bNote.config(relief='raised')
        self.bRegion.config(relief='raised')
        self.bPanel.config(relief='sunken')
        self.mode='panel'
        self.notewin.close_widgets()
        self.__disconnect_event()
        self._p.register('button_press',self._add_panel)

    def quit(self):
        self.__disconnect_event()
        #self.delete_bar()
        self.disable_button()
        self.figmgr.window.wm_withdraw()

    def enable_button(self):
        if self.button: return
        self.bRegion.config(state=Tk.NORMAL)
        self.bPanel.config(state=Tk.NORMAL)
        self.bClear.config(state=Tk.NORMAL)
        self.bFlag.config(state=Tk.NORMAL)
        self.bUnflag.config(state=Tk.NORMAL)
        self.bStat.config(state=Tk.NORMAL)
        self.button=True
        self.spec_show()

    def disable_button(self):
        ## disable buttons which don't work for plottp
        if not self.button: return
        self.bRegion.config(relief='raised', state=Tk.DISABLED)
        self.bPanel.config(relief='raised', state=Tk.DISABLED)
        self.bClear.config(state=Tk.DISABLED)
        self.bFlag.config(state=Tk.DISABLED)
        self.bUnflag.config(state=Tk.DISABLED)
        self.bStat.config(state=Tk.DISABLED)
        self.bNext.config(state=Tk.DISABLED)
        self.bPrev.config(state=Tk.DISABLED)
        self.button=False
        self.mode=''
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

    def delete_bar(self):
        self.__disconnect_event()
        self.destroy()

    def __disconnect_event(self):
        self._p.register('button_press',None)
        self._p.register('button_release',None)
