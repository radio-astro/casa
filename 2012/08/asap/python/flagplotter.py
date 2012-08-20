from asap.asapplotter import asapplotter
from asap.logging import asaplog, asaplog_post_dec

from asap.parameters import rcParams
from asap.selector import selector
from asap.scantable import scantable
import matplotlib.axes
from matplotlib.font_manager import FontProperties
from matplotlib.text import Text

class flagplotter(asapplotter):
    """
    The flag plotter
    Only row based panneling is allowed.

    Example:
       scan = asa p.scantable(filename='your_filename',average=False)
       guiflagger = asap.flagplotter(visible=True)
       guiflagger.plot(scan)
       ### flag/Unflag data graphically.
       guiflagger.save_data(name='flagged_file.asap',format='ASAP')
    
    NOTICE: 
       The flagged data is not saved until you explicitly run scantable.save
    """
    def __init__(self, visible=None, **kwargs):
        self._scan = None
        asapplotter.__init__(self,visible=visible, **kwargs)
        self._assert_plotter(action='reload')
        self._plotter._set_window_title('Flag Plotter')
        self._panelling = 'r'
        self.set_stacking('scan')
        self._ismodified = False
        self._showflagged = False
        self.set_colors("blue gray",False)

    def _new_custombar(self):
        backend = matplotlib.get_backend()
        # Flag plotter relys on supported GUI backends
        if not self._visible:
            asaplog.push("GUI backend is not available")
            asaplog.post("ERROR")
        elif backend == "TkAgg":
            from asap.customgui_tkagg import CustomFlagToolbarTkAgg
            return CustomFlagToolbarTkAgg(self)
        elif backend == "Qt4Agg":
            from asap.customgui_qt4agg import CustomFlagToolbarQT4Agg
            return CustomFlagToolbarQT4Agg(self)
        else:
            asaplog.push("Unsupported backend for interactive flagging. Use either TkAgg or PyQt4Agg")
            asaplog.post("ERROR")

    def set_showflagged(self, show):
        """ Whether or not plotting flagged data"""
        if type(show) == bool:
            self._showflagged = show
        else:
            raise TypeError, "Input parameter should be a bool."

    @asaplog_post_dec
    def _invalid_func(self, name):
        msg = "Invalid function 'flagplotter."+name+"'"
        #raise AttributeError(msg)
        asaplog.push(msg)
        asaplog.post('ERROR')

    def set_panelling(self,which='r'):
        """ This function is not available for the class flagplotter """
        if which.lower().startswith('r'):
            return
        msg = "Pannel setting is fixed to row mode in 'flagplotter'"
        asaplog.push(msg)
        asaplog.post('ERROR')
        self._panelling = 'r'

    def set_range(self,xstart=None,xend=None,ystart=None,yend=None,refresh=False, offset=None):
        """ This function is not available for the class flagplotter """
        msg = "Plot range setting is not allowed in 'flagplotter'"
        asaplog.push(msg)
        asaplog.post('ERROR')
        self._panelling = 'r'

    def plotazel(self,*args,**kwargs):
        """ This function is not available for the class flagplotter """
        self._invalid_func(name='plotazel')
    
    def plotpointing(self,*args,**kwargs):
        """ This function is not available for the class flagplotter """
        self._invalid_func(name='plotpointing')

    def plottp(self,*args,**kwargs):
        """ This function is not available for the class flagplotter """
        self._invalid_func(name='plottp')

    def save_data(self, name=None, format=None, overwrite=False):
        """
        Store the plotted scantable on disk.
        This function simply redirects call to scantable.save()
        
        Parameters:
    
            name:        the name of the outputfile. For format "ASCII"
                         this is the root file name (data in 'name'.txt
                         and header in 'name'_header.txt)
    
            format:      an optional file format. Default is ASAP.
                         Allowed are:
                            * 'ASAP' (save as ASAP [aips++] Table),
                            * 'SDFITS' (save as SDFITS file)
                            * 'ASCII' (saves as ascii text file)
                            * 'MS2' (saves as an casacore MeasurementSet V2)
                            * 'FITS' (save as image FITS - not readable by class)
                            * 'CLASS' (save as FITS readable by CLASS)
    
            overwrite:   If the file should be overwritten if it exists.
                         The default False is to return with warning
                         without writing the output. USE WITH CARE.
        """
        if not self._data:
            raise RuntimeError("No scantable has been set yet.")
        # simply calls scantable.save
        self._data.save(name,format,overwrite)

    def set_data(self, scan, refresh=True):
        if self._is_new_scan(scan):
            self._ismodified = False
        asapplotter.set_data(self, scan, refresh)
    set_data.__doc__ = asapplotter.set_data.__doc__

    @asaplog_post_dec
    def plot(self, scan=None):
        if self._is_new_scan(scan):
            self._ismodified = False
        if not self._showflagged:
            self.set_legend(mode=None,refresh=False)
        elif not self._legendloc:
            self.set_legend(mode=1,refresh=False)
        asapplotter.plot(self,scan)
    plot.__doc__ = asapplotter.plot.__doc__

    @asaplog_post_dec
    def _plot(self, scan):
        self._plot_with_flag(scan,self._showflagged)
        #asapplotter._plot(self,scan)
        # rescale x-range of subplots 5% margins
        ganged = (self._plotter.axes._sharex != None)
        if ganged:
            np = 1
        else:
            np = len(self._plotter.subplots)
        for ip in xrange(np):
            ax = self._plotter.subplots[ip]['axes']
            lim0 = ax.get_xlim()
            offset = (lim0[1]-lim0[0])*0.05
            ax.set_xlim(lim0[0]-offset,lim0[1]+offset)
            del ax, lim0, offset
    _plot.__doc__ = asapplotter._plot.__doc__


    @asaplog_post_dec
    def _plot_with_flag(self, scan, showflag=False):
        # total number of panles to plot as a whole
        nptot = scan.nrow()
        # remaining panels to plot
        n = nptot - self._ipanel - 1
        ganged = False
        maxpanel = 25

        if n > 1:
            ganged = rcParams['plotter.ganged']
            if self._rows and self._cols:
                n = min(n,self._rows*self._cols)
                self._plotter.set_panels(rows=self._rows,cols=self._cols,
                                         nplots=n,margin=self._margins,ganged=ganged)
            else:
                n = min(n,maxpanel)
                self._plotter.set_panels(rows=n,cols=0,nplots=n,margin=self._margins,ganged=ganged)
        else:
            self._plotter.set_panels(margin=self._margins)
        #r = 0
        r = self._startrow
        # total row number of scantable
        nr = scan.nrow()
        panelcount = 0
        allylim = []
        allxlim = []
        
        while r < nr:
            # always plot to new panel
            self._plotter.subplot(panelcount)
            self._plotter.palette(0)
            # title and axes labels
            xlab = self._abcissa and self._abcissa[panelcount] \
                       or scan._getabcissalabel()
            if self._offset and not self._abcissa:
                xlab += " (relative)"
            ylab = self._ordinate and self._ordinate[panelcount] \
                   or scan._get_ordinate_label()
            self._plotter.set_axes('xlabel', xlab)
            self._plotter.set_axes('ylabel', ylab)
            lbl = self._get_label(scan, r, mode='title', userlabel=self._title)
            if type(lbl) in (list, tuple):
                if 0 <= panelcount < len(lbl):
                    lbl = lbl[panelcount]
                else:
                    # get default label
                    lbl = self._get_label(scan, r, 'title')
            self._plotter.set_axes('title',lbl)
            panelcount += 1
            # Now get data to plot
            y = scan._getspectrum(r)
            # Check for FLAGROW column
            mr = scan._getflagrow(r)
            from numpy import ma, array
            if mr:
                ys = ma.masked_array(y,mask=mr)
                if showflag:
                    yf = ma.masked_array(y, mask=(not mr))
            else:
                m = scan._getmask(r)
                from numpy import logical_not, logical_and
                if self._maskselection and len(self._usermask) == len(m):
                    if d[self._stacking](r) in self._maskselection[self._stacking]:
                        m = logical_and(m, self._usermask)
                ys = ma.masked_array(y,mask=logical_not(array(m,copy=False)))
                if showflag:
                    yf = ma.masked_array(y,mask=m)

            x = array(scan._getabcissa(r))
            if self._offset:
                x += self._offset
            #llbl = self._get_label(scan, r, mode='legend', userlabel=self._lmap)
            #if type(llbl) in (list, tuple):
            #    llbl = llbl[0]
            #self._plotter.set_line(label=llbl)
            self._plotter.set_line(label="data")
            #plotit = self._plotter.plot
            #if self._hist: plotit = self._plotter.hist
            self._plotter.plot(x,ys)
            if showflag:
                self._plotter.set_line(label="flagged")
                self._plotter.plot(x,yf)
                ylim = self._minmaxy or [min(y),max(y)]
                xlim= self._minmaxx or [min(x),max(x)]
            elif mr or ys.mask.all():
                ylim = self._minmaxy or []
                xlim = self._minmaxx or []
            else:
                ylim = self._minmaxy or [ma.minimum(ys),ma.maximum(ys)]
                xlim= self._minmaxx or [min(x),max(x)]
            allylim += ylim
            allxlim += xlim
            if (panelcount == n) or (r == nr-1):
                break
            r+=1 # next row

        # Set x- and y- limts of subplots
        if ganged:
            xlim = None
            ylim = None
            if len(allylim) > 0:
                allylim.sort()
                ylim = allylim[0],allylim[-1]
            if len(allxlim) > 0:
                allxlim.sort()
                xlim = allxlim[0],allxlim[-1]
            self._plotter.set_limits(xlim=xlim,ylim=ylim)

        # save the current counter for multi-page plotting
        self._startrow = r+1
        self._ipanel += panelcount
        if self.casabar_exists():
            if self._ipanel >= nptot-1:
                self._plotter.figmgr.casabar.disable_next()
            else:
                self._plotter.figmgr.casabar.enable_next()
            if self._ipanel + 1 - panelcount > 0:
                self._plotter.figmgr.casabar.enable_prev()
            else:
                self._plotter.figmgr.casabar.disable_prev()

                

    def _get_label(self, scan, row, mode='title', userlabel=None):
        if isinstance(userlabel, list) and len(userlabel) == 0:
            userlabel = " "
        elif not mode.upper().startswith('T'):
            pms = dict(zip(self._selection.get_pols(), \
                           self._selection.get_poltypes()))
            if len(pms):
                poleval = scan._getpollabel(scan.getpol(row), \
                                            pms[scan.getpol(row)])
            else:
                poleval = scan._getpollabel(scan.getpol(row),scan.poltype())
            label = "IF%d, POL %s, Scan%d" % \
                    (scan.getif(row),poleval,scan.getscan(row))
        else:
            label = "row %d" % (row)
            
        return userlabel or label
        

    def _is_new_scan(self,scan):
        if isinstance(scan, scantable):
            if self._data is not None:
                if scan != self._data:
                    return True
            else:
                return True
        return False
