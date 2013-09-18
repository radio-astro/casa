import os
from numpy import ma, array, logical_not, logical_and

from taskinit import casalog

import asap as sd
from asap.scantable import is_scantable, is_ms
from asap.flagplotter import flagplotter
import sdutil

def sdflag2(infile, antenna, specunit, restfreq, frame, doppler, mode, unflag, scans, field, timerange, ifs, pols, maskflag, clipminmax, clipoutside, showflagged, rows, outfile, outform, overwrite, plotlevel):
    with sdutil.sdtask_manager(sdflag_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()

class sdflag_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdflag_worker,self).__init__(**kwargs)

        # initialize plotter
        self.__init_plotter()
        # map selection parameters
        self.__map_selection_param()
        # define valid modes
        self.valid_modes = ('m', 'c', 'i', 'r')

    def parameter_check(self):
        # by default, the task overwrite infile
        if len(self.outfile)==0: 
            self.project = self.infile
        else:
            self.project = self.outfile

        sdutil.assert_outfile_canoverwrite_or_nonexistent(self.project,
                                                          self.outform,
                                                          self.overwrite)
        
        #check the format of the infile
        filename = sdutil.get_abspath(self.infile)
        if isinstance(self.infile, str):
            if is_scantable(filename):
                informat = 'ASAP'
            elif is_ms(filename):
                informat = 'MS2'
            else:
                informat = 'SDFITS'
        else:
            informat = 'UNDEFINED'
                
        # Check the formats of infile and outfile are identical when overwrite=True.
        # (CAS-3096). If not, print warning message and exit.
        outformat = self.outform.upper()
        if (outformat == 'MS'): outformat = 'MS2'
        if self.overwrite and os.path.exists(self.project) \
           and (sdutil.get_abspath(self.project) == sdutil.get_abspath(self.infile)) \
           and (outformat != informat):
            msg = "The input and output data format must be identical when "
            msg += "their names are identical and overwrite=True. "
            msg += "%s and %s given for input and output, respectively." % (informat, outformat)
            raise Exception, msg

        # check restfreq
        self.rfset = (self.restfreq != '') and (self.restfreq != [])
        self.restore = (self.specunit == 'km/s') and self.rfset

        if self.mode.lower()[0] not in self.valid_modes:
            raise Exception, "Invalide mode, '%s'" % self.mode

        # check whether any flag operation is done or not
        self.anyflag = False
        
    def initialize_scan(self):
        sorg = sd.scantable(self.infile,average=False,antenna=self.antenna)

        if ( abs(self.plotlevel) > 1 ):
            casalog.post( "Initial Scantable:" )
            sorg._summary()

        # Copy the original data (CAS-3987)
        if self.is_disk_storage \
           and (sdutil.get_abspath(self.project) != sdutil.get_abspath(self.infile)):
            self.scan = sorg.copy()
        else:
            self.scan = sorg

        # data selection
        self.scan.set_selection(self.get_selector())
        
    def execute(self):
        self.set_to_scan()

        if self.mode.lower().startswith('m') and (len(self.maskflag) > 0):
            # channel flag
            self.masks = self.scan.create_mask(self.maskflag)
        else:
            self.masks = [False for i in xrange(self.scan.nchan())]
        
        self.threshold = [None,None]
        if isinstance(self.clipminmax, list):
            if (len(self.clipminmax) == 2):
                self.threshold = self.clipminmax[:]
                self.threshold.sort()
        
        if self.mode.lower().startswith('i'):
            # interactive mode
            self.interactive_flag()
        else:
            # the other mode
            if (abs(self.plotlevel) > 0):
                # plot flag and update self.docmdflag by the user input
                self.prior_plot()

            self.command_flag()

        if not self.anyflag:
            raise Exception, 'No flag operation. Finish without saving'

        if abs(self.plotlevel) > 0:
            self.posterior_plot()

    def command_flag(self):
        # Actual flag operations
        shortmode = self.mode.lower()[0]
        if shortmode == 'c':
            # clip
            self.do_clip()
        elif shortmode == 'm' and len(self.maskflag) > 0:
            # channel flag (manual mode)
            self.do_channel_flag()
        else:
            # manual or rowid mode
            if shortmode == 'r':
                # reset selection (not necessary indeed)
                self.scan.set_selection(None)
            if len(self.rows) == 0:
                # select all rows
                self.rows = range(self.scan.nrow())
            self.do_row_flag()
        self.anyflag = True

        # Add history entry
        operation = ('unflag' if self.unflag else 'flag')
        mask = (self.maskflag if len(self.maskflag) > 0 else 'all channel')
        params={'mode': operation,'maskflag': mask}
        sel = self.scan.get_selection()
        keys=['pol','if','scan']
        for key in keys:
            val = getattr(sel,'get_%ss'%(key))()
            params['%ss'%(key)] = val if len(val)>0 else list(getattr(self.scan,'get%snos'%(key))())
        #print "input parameters:\n", params
        self.scan._add_history( "sdflag", params ) 

    def do_clip(self):
        casalog.post('Number of spectra to be flagged: %d\nApplying clipping...'%(self.scan.nrow()))
        #casalog.post('flagrow and maskflag will be ignored',priority='WARN')

        if self.threshold[1] > self.threshold[0]:
            self.scan.clip(self.threshold[1], self.threshold[0], self.clipoutside, self.unflag)

    def do_channel_flag(self):
        casalog.post('Number of spectra to be flagged: %d\nApplying channel flagging...'%(self.scan.nrow()))

        self.scan.flag(mask=self.masks, unflag=self.unflag)

    def do_row_flag(self):
        casalog.post('Number of rows to be flagged: %d\nApplying row flagging...'%(len(self.rows)))
        #casalog.post('maskflag will be ignored',priority='WARN')

        self.scan.flag_row(self.rows, self.unflag)

    def interactive_flag(self):
        from matplotlib import rc as rcp
        rcp('lines', linewidth=1)
        guiflagger = flagplotter(visible=True)
        #guiflagger.set_legend(loc=1,refresh=False)
        guiflagger.set_showflagged(self.showflagged)
        guiflagger.plot(self.scan)
        finish=raw_input("Press enter to finish interactive flagging:")
        guiflagger._plotter.unmap()
        ismodified = guiflagger._ismodified
        guiflagger._plotter = None
        self.anyflag = ismodified

    def save(self):
        self.scan.set_selection(None)
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)

    def prior_plot(self):
        nr = self.scan.nrow()
        np = min(nr,16)
        if nr >16:
            casalog.post( "Only first 16 spectra is plotted.", priority = 'WARN' )

        self.myp.set_panels(rows=np,cols=0,nplots=np)
        self.myp.legend(loc=1)
        labels = ['Spectrum','current flag masks','previously flagged data']
        masklist =  [ None,      None,                None]
        idefaultmask = logical_not(array(self.masks))
        for row in xrange(np):
            self.myp.subplot(row)
            x = self.scan._getabcissa(row)
            y = self.scan._getspectrum(row)
            nchan = len(y)

            if self.scan._getflagrow(row):
                masklist[2] = array([False]*(nchan))
            else:
                masklist[2] = array(self.scan._getmask(row))

            if self.clip:
                if self.threshold[0] == self.threshold[1]:
                    masklist[1] = array([True]*nchan)
                else:
                    masklist[1] = array(self.scan._getclipmask(row, self.threshold[1], self.threshold[0], (not self.clipoutside), self.unflag))
            elif len(self.flagrow) > 0:
                masklist[1] = array([(row not in self.flagrow) or self.unflag]*nchan)
            else:
                masklist[1] = idefaultmask
            masklist[0] = logical_not(logical_and(masklist[1],masklist[2]))
            for i in xrange(3):
                plot_data(self.myp,x,y,masklist[i],i,labels[i])
            xlim=[min(x),max(x)]
            self.myp.axes.set_xlim(xlim)

            labels = ['spec','flag','prev']
        self.myp.release()
        
        #Apply flag
        if self.plotlevel > 0 and sd.rcParams['plotter.gui']:
            ans=raw_input("Apply %s (y/N)?: " % self.flagmode)
        else:
            casalog.post("Applying selected flags")
            ans = 'Y'

        # update self.docmdflag
        self.docmdflag = (ans.upper() == 'Y')
    def posterior_plot(self):
        #Plot the result
        #print "Showing only the first spectrum..."
        casalog.post( "Showing only the first spectrum..." )
        row=0

        self.myp.set_panels()
        x = self.scan._getabcissa(row)
        y = self.scan._getspectrum(row)
        allmskarr=array(self.scan._getmask(row))
        plot_data(self.myp,x,y,logical_not(allmskarr),0,"Spectrum after %s" % self.flagmode+'ging')
        plot_data(self.myp,x,y,allmskarr,2,"Flagged")
        xlim=[min(x),max(x)]
        self.myp.axes.set_xlim(xlim)
        if ( self.plotlevel < 0 ):
            # Hardcopy - currently no way w/o screen display first
            pltfile=self.project+'_flag.eps'
            self.myp.save(pltfile)
        self.myp.release()
    
    def __init_plotter(self):
        colormap = ["green","red","#dddddd","#777777"]
        self.myp = sdutil.get_plotter(self.plotlevel)
        casalog.post('Create new plotter')
        self.myp.palette(0,colormap)
        self.myp.hold()
        self.myp.clear()

    def __map_selection_param(self):
        selection_map = {'scans': 'scanlist',
                         'ifs': 'iflist',
                         'pols': 'pollist'}
        for a, b in selection_map.items():
            if hasattr(self, a):
                setattr(self, b, getattr(self, a))


def plot_data(myp,x,y,msk,color=0,label=None):
    if label:
        myp.set_line(label=label)
    myp.palette(color)
    ym = ma.masked_array(y,mask=msk)
    myp.plot(x,ym)

