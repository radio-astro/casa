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
        self._plotter._set_window_title('Flag Plotter')
        self._panelling = 'r'
        self.set_stacking('scan')
        self._ismodified = False

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
        asapplotter.plot(self,scan)
    plot.__doc__ = asapplotter.plot.__doc__

    @asaplog_post_dec
    def _plot(self, scan):
        asapplotter._plot(self,scan)
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

    def _is_new_scan(self,scan):
        if isinstance(scan, scantable):
            if self._data is not None:
                if scan != self._data:
                    return True
            else:
                return True
        return False
