import os
import weakref
import matplotlib, numpy
from asap.logging import asaplog, asaplog_post_dec
from matplotlib.patches import Rectangle
from asap.parameters import rcParams
from asap import scantable
from asap._asap import stmath
from asap.utils import _n_bools, mask_not, mask_or

######################################
##    Add CASA custom toolbar       ##
######################################
class CustomToolbarCommon:
    def __init__(self,parent):
        self.plotter = weakref.ref(parent)
        #self.figmgr=self.plotter._plotter.figmgr

    def _get_plotter(self):
        # check for the validity of the plotter and
        # return the plotter class instance if its valid.
        if self.plotter() is None:
            raise RuntimeError, "Internal Error. The plotter has been destroyed."
        else:
            return self.plotter()

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

        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

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
        theplot = theplotter._plotter
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


    ### Notation
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


    ### Purely plotter based statistics calculation of a selected area.
    ### No access to scantable
    def _single_mask(self,event):
        # Do not fire event when in zooming/panning mode
        if not self.figmgr.toolbar.mode == '':
            return
        # When selected point is out of panels
        if event.inaxes == None:
            return
        if event.button == 1:
            exclude=False
        elif event.button == 3:
            exclude=True
        else:
            return

        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        self._thisregion = {'axes': event.inaxes,'xs': event.x,
                            'worldx': [event.xdata,event.xdata],
                            'invert': exclude}
        self.xold = event.x
        self.xdataold = event.xdata

        theplotter._plotter.register('button_press',None)
        theplotter._plotter.register('motion_notify', self._xspan_draw)
        theplotter._plotter.register('button_press', self._xspan_end)

    def _xspan_draw(self,event):
        if event.inaxes == self._thisregion['axes']:
            xnow = event.x
            self.xold = xnow
            self.xdataold = event.xdata
        else:
            xnow = self.xold
        try: self.lastspan
        except AttributeError: pass
        else:
            if self.lastspan: self._remove_span(self.lastspan)

        self.lastspan = self._draw_span(self._thisregion['axes'],
                                        self._thisregion['xs'], xnow, fill="")
        del xnow

    def _draw_span(self,axes,x0,x1,**kwargs):
        pass

    def _remove_span(self,span):
        pass

    @asaplog_post_dec
    def _xspan_end(self,event):
        if not self.figmgr.toolbar.mode == '':
            return
        #if event.button != 1:
        #    return

        try: self.lastspan
        except AttributeError: pass
        else:
            self._remove_span(self.lastspan)
            del self.lastspan
        if event.inaxes == self._thisregion['axes']:
            xdataend = event.xdata
        else:
            xdataend = self.xdataold

        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        self._thisregion['worldx'][1] = xdataend
        # print statistics of spectra in subplot
        self._subplot_stats(self._thisregion)
        
        # release event
        theplotter._plotter.register('button_press',None)
        theplotter._plotter.register('motion_notify',None)
        # Clear up region selection
        self._thisregion = None
        self.xdataold = None
        self.xold = None
        # finally recover region selection event
        theplotter._plotter.register('button_press',self._single_mask)

    def _subplot_stats(self,selection):
        statstr = ['max', 'min', 'median', 'mean', 'sum', 'std'] #'rms']
        panelstr = selection['axes'].title.get_text()
        ssep = "-"*70
        asaplog.push(ssep)
        asaplog.post()
        for line in selection['axes'].lines:
            # Don't include annotations
            if line.get_label().startswith("_"):
                continue
            label = panelstr + ", "+line.get_label()
            x = line.get_xdata()
            newmsk = None
            selmsk = self._create_flag_from_array(x,
                                                  selection['worldx'],
                                                  selection['invert'])
            ydat = None
            y = line.get_ydata()
            if numpy.ma.isMaskedArray(y):
                ydat = y.data
                basemsk = y.mask
            else:
                ydat = y
                basemsk = False
            if not isinstance(basemsk, bool):
                # should be ndarray
                newmsk = mask_or(selmsk, basemsk)
            elif basemsk:
                # the whole original spectrum is flagged
                newmsk = basemsk
            else:
                # no channel was flagged originally
                newmsk = selmsk
            mdata = numpy.ma.masked_array(ydat, mask=newmsk)
            statval = {}
            for stat in statstr:
                # need to get the stat functions from the ma module!!!
                statval[stat] = getattr(numpy.ma,stat)(mdata)
            self._print_stats(statval, statstr=statstr, label=label,\
                              mask=selection['worldx'],\
                              unmask=selection['invert'])
            asaplog.push(ssep)
            asaplog.post()

    def _create_flag_from_array(self,x,masklist,invert):
        # Return True for channels which should be EXCLUDED (flag)
        if len(masklist) <= 1:
            asaplog.push()
            asaplog.post("masklist should be a list of 2 elements")
            asaplog.push("ERROR")
        n = len(x)
        # Base mask: flag out all channels
        mask = _n_bools(n, True)
        minval = min(masklist[0:2])
        maxval = max(masklist[0:2])
        for i in range(n):
            if minval <= x[i] <= maxval:
                mask[i] = False
        if invert:
            mask = mask_not(mask)
        return mask

    @asaplog_post_dec
    def _print_stats(self,stats,statstr=None,label="",mask=None,unmask=False):
        if not isinstance(stats,dict) or len(stats) == 0:
            asaplog.post()
            asaplog.push("Invalid statistic value")
            asaplog.post("ERROR")
        maskstr = "Not available"
        if mask:
            masktype = "mask"
            maskstr = str(mask)
            if unmask: masktype = "unmask"

        sout = label + ", " + masktype + " = " + maskstr + "\n"
        statvals = []
        if not len(statstr):
            statstr = stats.keys()
        for key in statstr:
            sout += key.ljust(10)
            statvals.append(stats.pop(key))
        sout += "\n"
        sout += ("%f "*len(statstr) % tuple(statvals))
        asaplog.push(sout)
        #del sout, maskstr, masktype, statvals, key, stats, statstr, mask, label


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
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        top = None
        header = theplotter._headtext
        reset = False
        doheader = (isinstance(header['textobj'],list) and \
                    len(header['textobj']) > 0)
        if doheader:
            top = theplotter._plotter.figure.subplotpars.top
            fontsize = header['textobj'][0].get_fontproperties().get_size()
        if theplotter._startrow <= 0:
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
            theplotter._reset_header()

        theplotter._plotter.hold()
        if goback:
            self._set_prevpage_counter()
        #theplotter._plotter.clear()
        theplotter._plot(theplotter._data)
        pagenum = self._get_pagenum()
        self.set_pagecounter(pagenum)
        # Plot header information
        #if header['textobj']:
        if doheader and pagenum == 1:
            if top and top != theplotter._margins[3]:
                # work around for sdplot in CASA. complete checking in future?
                theplotter._plotter.figure.subplots_adjust(top=top)
            if reset:
                theplotter.print_header(plot=True,fontsize=fontsize,selstr=selstr, extrastr=extrastr)
            else:
                theplotter._header_plot(header['string'],fontsize=fontsize)
        theplotter._plotter.release()
        theplotter._plotter.tidy()
        theplotter._plotter.show(hardrefresh=False)
        del top

    ### calculate the panel ID and start row to plot the previous page
    def _set_prevpage_counter(self):
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        # set row and panel counters to those of the 1st panel of previous page
        maxpanel = 16
        # the ID of the last panel in current plot
        lastpanel = theplotter._ipanel
        # the number of current subplots
        currpnum = len(theplotter._plotter.subplots)
        # the nuber of previous subplots
        prevpnum = None
        if theplotter._rows and theplotter._cols:
            # when user set layout
            prevpnum = theplotter._rows*theplotter._cols
        else:
            # no user specification
            prevpnum = maxpanel

        start_ipanel = max(lastpanel-currpnum-prevpnum+1, 0)
        # set the pannel ID of the last panel of prev-prev page
        theplotter._ipanel = start_ipanel-1
        if theplotter._panelling == 'r':
            theplotter._startrow = start_ipanel
        else:
            # the start row number of the next panel
            theplotter._startrow = theplotter._panelrows[start_ipanel]
        del lastpanel,currpnum,prevpnum,start_ipanel

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
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()
        
        # get the ID of last panel in the current page
        idlastpanel = theplotter._ipanel
        # max panels in a page
        ppp = theplotter._plotter.rows*theplotter._plotter.cols
        return int(idlastpanel/ppp)+1

    # pause buttons for slow operations. implemented at a backend dependent class
    def _pause_buttons(self,operation="end",msg=""):
        pass




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
                    #print msg
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
                        #print msg 

        if selected:
            msg = "Selected (modify/delete): '"+selected['textobj'].get_text()
            msg += "' @"+str(selected['textobj'].get_position())
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



###########################################
##    Add CASA custom Flag toolbar       ##
###########################################
class CustomFlagToolbarCommon:
    def __init__(self,parent):
        self.plotter=weakref.ref(parent)
        #self.figmgr=self.plotter._plotter.figmgr
        self._selregions = {}
        self._selpanels = []
        self._polygons = []
        self._thisregion = None
        self.xdataold=None

    def _get_plotter(self):
        # check for the validity of the plotter and
        # return the plotter class instance if its valid.
        if self.plotter() is None:
            raise RuntimeError, "Internal Error. The plotter has been destroyed."
        else:
            return self.plotter()

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

        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

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
        theplot = theplotter._plotter
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
                xl = xclick-xdata[i]
                xr = xclick-xdata[i+1]
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
        return False

    ### Region/Panel selection & oparations
    ### add regions to selections
    @asaplog_post_dec
    def _add_region(self,event):
        if not self.figmgr.toolbar.mode == '':
            return
        if event.button != 1 or event.inaxes == None:
            return
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()
        # this row resolution assumes row panelling
        irow = int(self._getrownum(event.inaxes))
        if irow in self._selpanels:
            msg = "The whole spectrum is already selected"
            asaplog.post()
            asaplog.push(msg)
            asaplog.post('WARN')
            return
        self._thisregion = {'axes': event.inaxes,'xs': event.x,
                            'worldx': [event.xdata,event.xdata]}
        theplotter._plotter.register('button_press',None)
        self.xold = event.x
        self.xdataold = event.xdata
        theplotter._plotter.register('motion_notify', self._xspan_draw)
        theplotter._plotter.register('button_press', self._xspan_end)

    def _xspan_draw(self,event):
        if event.inaxes == self._thisregion['axes']:
            xnow = event.x
            self.xold = xnow
            self.xdataold = event.xdata
        else:
            xnow = self.xold
        try: self.lastspan
        except AttributeError: pass
        else:
            if self.lastspan: self._remove_span(self.lastspan)

        #self.lastspan = self._draw_span(self._thisregion['axes'],self._thisregion['xs'],xnow,fill="#555555",stipple="gray50")
        self.lastspan = self._draw_span(self._thisregion['axes'],self._thisregion['xs'],xnow,fill="")
        del xnow

    def _draw_span(self,axes,x0,x1,**kwargs):
        pass

    def _remove_span(self,span):
        pass

    @asaplog_post_dec
    def _xspan_end(self,event):
        if not self.figmgr.toolbar.mode == '':
            return
        if event.button != 1:
            return

        try: self.lastspan
        except AttributeError: pass
        else:
            self._remove_span(self.lastspan)
            del self.lastspan
        if event.inaxes == self._thisregion['axes']:
            xdataend = event.xdata
        else:
            xdataend = self.xdataold

        self._thisregion['worldx'][1] = xdataend
        lregion = self._thisregion['worldx']
        # WORKAROUND for the issue axvspan started to reset xlim.
        axlimx = self._thisregion['axes'].get_xlim()
        pregion = self._thisregion['axes'].axvspan(lregion[0],lregion[1],
                                                   facecolor='0.7')
        self._thisregion['axes'].set_xlim(axlimx)
        
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        theplotter._plotter.canvas.draw()
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
        theplotter._plotter.register('button_press',None)
        theplotter._plotter.register('motion_notify',None)
        # Clear up region selection
        self._thisregion = None
        self.xdataold = None
        self.xold = None
        # finally recover region selection event
        theplotter._plotter.register('button_press',self._add_region)

    ### add panels to selections
    @asaplog_post_dec
    def _add_panel(self,event):
        if not self.figmgr.toolbar.mode == '':
            return
        if event.button != 1 or event.inaxes == None:
            return
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        selax = event.inaxes
        # this row resolution assumes row panelling
        srow = self._getrownum(selax)
        irow = int(srow)
        if srow:
            self._selpanels.append(irow)
        shadow = Rectangle((0,0),1,1,facecolor='0.7',transform=selax.transAxes,visible=True)
        self._polygons.append(selax.add_patch(shadow))
        #theplotter._plotter.show(False)
        theplotter._plotter.canvas.draw()
        asaplog.push("row "+str(irow)+" is selected")
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

        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        strow = self._getrownum(theplotter._plotter.subplots[0]['axes'])
        enrow = self._getrownum(theplotter._plotter.subplots[-1]['axes'])
        for irow in range(int(strow),int(enrow)+1):
            if regions.has_key(str(irow)):
                ax = theplotter._plotter.subplots[irow - int(strow)]['axes']
                mlist = regions.pop(str(irow))
                # WORKAROUND for the issue axvspan started to reset xlim.
                axlimx = ax.get_xlim()
                for i in range(len(mlist)):
                    self._polygons.append(ax.axvspan(mlist[i][0],mlist[i][1],
                                                     facecolor='0.7'))
                ax.set_xlim(axlimx)
                del ax,mlist,axlimx
            if irow in panels:
                ax = theplotter._plotter.subplots[irow - int(strow)]['axes']
                shadow = Rectangle((0,0),1,1,facecolor='0.7',
                                   transform=ax.transAxes,visible=True)
                self._polygons.append(ax.add_patch(shadow))
                del ax,shadow
        theplotter._plotter.canvas.draw()
        del regions,panels,strow,enrow

    def _clear_selection_plot(self, refresh=True):
        ### clear up polygons which mark selected spectra and regions ###
        if len(self._polygons) > 0:
            for shadow in self._polygons:
                shadow.remove()
            if refresh:
                # check for the validity of plotter and get the plotter
                theplotter = self._get_plotter()
                theplotter._plotter.canvas.draw()
        self._polygons = []

    def _clearup_selections(self, refresh=True):
        # clear-up selection and polygons
        self._selpanels = []
        self._selregions = {}
        self._clear_selection_plot(refresh=refresh)

    ### clear up selections
    def cancel_select(self):
        self.figmgr.toolbar.set_message('selections canceled')
        # clear-up selection and polygons
        self._clearup_selections(refresh=True)

    ### flag selected spectra/regions
    @asaplog_post_dec
    def flag(self):
        if not self._any_selection():
            msg = "No selection to be Flagged"
            asaplog.post()
            asaplog.push(msg)
            asaplog.post('WARN')
            return

        self._pause_buttons(operation="start",msg="Flagging data...")
        self._flag_operation(rows=self._selpanels,
                             regions=self._selregions,unflag=False)
        sout = "Flagged:\n"
        sout += "  rows = "+str(self._selpanels)+"\n"
        sout += "  regions: "+str(self._selregions)
        asaplog.push(sout)
        del sout
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        theplotter._ismodified = True
        self._clearup_selections(refresh=False)
        self._plot_page(pagemode="current")
        self._pause_buttons(operation="end")

    ### unflag selected spectra/regions
    @asaplog_post_dec
    def unflag(self):
        if not self._any_selection():
            msg = "No selection to be Flagged"
            asaplog.push(msg)
            asaplog.post('WARN')
            return
        self._pause_buttons(operation="start",msg="Unflagging data...")
        self._flag_operation(rows=self._selpanels,
                             regions=self._selregions,unflag=True)
        sout = "Unflagged:\n"
        sout += "  rows = "+str(self._selpanels)+"\n"
        sout += "  regions: "+str(self._selregions)
        asaplog.push(sout)
        del sout

        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()
        theplotter._ismodified = True
        self._clearup_selections(refresh=False)
        self._plot_page(pagemode="current")
        self._pause_buttons(operation="end")

    ### actual flag operation
    @asaplog_post_dec
    def _flag_operation(self,rows=None,regions=None,unflag=False):
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        scan = theplotter._data
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
        self._selected_stats(rows=self._selpanels,regions=self._selregions)
        self._clearup_selections(refresh=True)

    @asaplog_post_dec
    def _selected_stats(self,rows=None,regions=None):
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        scan = theplotter._data
        if not scan:
            asaplog.post()
            asaplog.push("Invalid scantable")
            asaplog.post("ERROR")
        mathobj = stmath( rcParams['insitu'] )
        statval = {}
        statstr = ['max', 'min', 'mean', 'median', 'sum', 'stddev', 'rms']
        if isinstance(rows, list) and len(rows) > 0:
            for irow in rows:
                for stat in statstr:
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
                    statval[stat] = mathobj._statsrow(scan,mask,stat,irow)[0]
                self._print_stats(scan,irow,statval,statstr=statstr,
                                  mask=masklist)
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
        self._pause_buttons(operation="start",msg='plotting the previous page')
        self._clear_selection_plot(refresh=False)
        self._plot_page(pagemode="prev")
        self._plot_selections()
        self._pause_buttons(operation="end")

    ### go to the next page
    def next_page(self):
        self._pause_buttons(operation="start",msg='plotting the next page')
        self._clear_selection_plot(refresh=False)
        self._plot_page(pagemode="next")
        self._plot_selections()
        self._pause_buttons(operation="end")

    ### actual plotting of the new page
    def _plot_page(self,pagemode="next"):
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()
        if theplotter._startrow <= 0:
            msg = "The page counter is reset due to chages of plot settings. "
            msg += "Plotting from the first page."
            asaplog.post()
            asaplog.push(msg)
            asaplog.post('WARN')
            goback = False

        theplotter._plotter.hold()
        #theplotter._plotter.legend(1)
        self._set_plot_counter(pagemode)
        theplotter._plot(theplotter._data)
        self.set_pagecounter(self._get_pagenum())
        theplotter._plotter.release()
        theplotter._plotter.tidy()
        theplotter._plotter.show(hardrefresh=False)

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

        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()

        # set row and panel counters to those of the 1st panel of previous page
        maxpanel = 25
        # the ID of the last panel in current plot
        lastpanel = theplotter._ipanel
        # the number of current subplots
        currpnum = len(theplotter._plotter.subplots)

        # the nuber of previous subplots
        start_ipanel = None
        if pageop == "c":
            start_ipanel = max(lastpanel-currpnum+1, 0)
        else:
            ## previous page
            prevpnum = None
            if theplotter._rows and theplotter._cols:
                # when user set layout
                prevpnum = theplotter._rows*theplotter._cols
            else:
                # no user specification
                prevpnum = maxpanel
            start_ipanel = max(lastpanel-currpnum-prevpnum+1, 0)
            del prevpnum

        # set the pannel ID of the last panel of the prev(-prev) page
        theplotter._ipanel = start_ipanel-1
        if theplotter._panelling == 'r':
            theplotter._startrow = start_ipanel
        else:
            # the start row number of the next panel
            theplotter._startrow = theplotter._panelrows[start_ipanel]
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
        # check for the validity of plotter and get the plotter
        theplotter = self._get_plotter()
        # get the ID of last panel in the current page
        idlastpanel = theplotter._ipanel
        # max panels in a page
        ppp = theplotter._plotter.rows*theplotter._plotter.cols
        return int(idlastpanel/ppp)+1

    # pause buttons for slow operations. implemented at a backend dependent class
    def _pause_buttons(self,operation="end",msg=""):
        pass
