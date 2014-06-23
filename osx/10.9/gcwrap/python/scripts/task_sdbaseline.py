import numpy
import sdutil

from taskinit import casalog

import asap as sd
from asap.scantable import is_scantable

@sdutil.sdtask_decorator
def sdbaseline(infile, antenna, fluxunit, telescopeparam, field, spw, restfreq, frame, doppler, timerange, scan, pol, tau, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verify, verbose, bloutput, blformat, showprogress, minnrow, outfile, outform, overwrite, plotlevel):
    with sdutil.sdtask_manager(sdbaseline_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()


class sdbaseline_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdbaseline_worker,self).__init__(**kwargs)
        self.suffix = '_bs'

    def parameter_check(self):
        if (self.order < 0) and (self.blfunc in ['poly', 'chebyshev']):
            msg = 'Negative order of baseline polynomial given. Exit without baselining.'
            casalog.post(msg, priority = 'SEVERE')
            raise Exception(msg)

        if (self.npiece <= 0) and (self.blfunc == 'cspline'):
            msg = 'The parameter npiece must be greater than 0. Exit without baselining.'
            casalog.post(msg, priority='SEVERE')
            raise Exception(msg)            

    def initialize_scan(self):
        sorg = sd.scantable(self.infile, average=False, antenna=self.antenna)
        
        sel = self.get_selector(sorg)
        sorg.set_selection(sel)
        del sel

        # Copy scantable when using disk storage not to modify
        # the original table.
        if is_scantable(self.infile) and self.is_disk_storage:
            self.scan = sorg.copy()
        else:
            self.scan = sorg
        del sorg

    def execute(self):
        engine = sdbaseline_engine(self)
        engine.initialize()

        # check if the data contains spectra
        if (self.scan.nchan()==1):
           s = "Cannot process the input data. It contains only single channel data."
           raise Exception, s
   
        # set various attributes to self.scan
        self.set_to_scan()
        
        scanns = self.scan.getscannos()
        sn=list(scanns)
        casalog.post( "Number of scans to be processed: %d" % (len(sn)) )
        
        sdutil.doopacity(self.scan, self.tau)
    
        engine.execute()
        engine.finalize()

    def save(self):
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)

class sdbaseline_engine(sdutil.sdtask_engine):
    #keywords for each baseline function
    clip_keys = ['clipthresh', 'clipniter']
    poly_keys = ['order']
    chebyshev_keys = poly_keys
    cspline_keys = ['npiece']
    sinusoid_keys = ['applyfft', 'fftmethod', 'fftthresh', 'addwn', 'rejwn']
    auto_keys = ['thresh', 'avg_limit', 'edge']
    list_keys = []
    interact_keys = []

    def __init__(self, worker):
        super(sdbaseline_engine,self).__init__(worker)

        self.params = {}
        self.funcname = None

    def __del__(self):
        del self.params

    def initialize(self):
        if ( abs(self.plotlevel) > 1 ):
            casalog.post( "Initial Raw Scantable:" )
            self.worker.scan._summary()

    def execute(self):
        scan = self.worker.scan
        self.__init_blfile()

        nrow = scan.nrow()

        # parse string masklist
        maskdict = scan.parse_spw_selection(self.spw)
        valid_spw_list = []
        for (k,v) in maskdict.items():
            if len(v) > 0 and numpy.all(numpy.array(map(len, v)) > 0):
                valid_spw_list.append(k)
        
        basesel = scan.get_selection()

        # configure baseline function and its parameters
        self.__configure_baseline()
        
        for ifno in valid_spw_list:
            lmask = maskdict[ifno]
            sif = str(ifno)
            if len(sif) > 0:
                sel = sd.selector(basesel)
                sel.set_ifs([int(sif)])
                scan.set_selection(sel)
                del sel
                msg = "Working on IF%s" % (sif)
                casalog.post(msg)
                if (self.maskmode == 'interact'): print "===%s===" % (msg)
                del msg

            msk = None

            if (self.maskmode == 'interact'):
                msk = sdutil.interactive_mask(scan, lmask, False, purpose='to baseline spectra')
                msks = scan.get_masklist(msk)
                if len(msks) < 1:
                    msg = 'No channel is selected. Exit without baselining.'
                    casalog.post(msg, priorigy='SEVERE')
                    raise Exception(msg)

                casalog.post( 'final mask list ('+scan._getabcissalabel()+') ='+str(msks) )
                #header += "   Fit Range: "+str(msks)+"\n"
                del msks
            else:
                # Use baseline mask for regions to INCLUDE in baseline fit
                # Create mask using list, e.g. masklist=[[500,3500],[5000,7500]]
                if (len(lmask) > 0): msk = scan.create_mask(lmask)

            # register IF dependent mask
            self.params['mask'] = msk

            # call target baseline function with appropriate parameter set
            baseline_func = getattr(scan, self.funcname)
            baseline_func(**self.params)

            # reset selection
            if len(sif) > 0: scan.set_selection(basesel)

        ifs_org = basesel.get_ifs()
        ifs_new = list(set(ifs_org) & set(valid_spw_list))
        basesel.set_ifs(ifs_new)
        scan.set_selection(basesel)
            
    def finalize(self):
        if ( abs(self.plotlevel) > 0 ):
            pltfile=self.project+'_bsspec.eps'
            sdutil.plot_scantable(self.worker.scan, pltfile, self.plotlevel)

    def __configure_baseline(self):
        # determine what baseline function should be called
        self.funcname = '%s_baseline'%(getattr(self,'blfunc').lower())
        if self.maskmode.lower() == 'auto':
            self.funcname = 'auto_' + self.funcname

        # register parameters for baseline function
        # parameters for auto mode
        if self.maskmode.lower() == 'auto':
            self.params['threshold'] = self.thresh
            self.params['chan_avg_limit'] = self.avg_limit
            self.params['edge'] = self.edge

        # parameters that depends on baseline function
        keys = getattr(self, '%s_keys'%(self.blfunc.lower()))
        for k in keys:
            self.params[k] = getattr(self,k)
            
        # parameters for clipping
        keys = getattr(self, 'clip_keys')
        for k in keys:
            self.params[k] = getattr(self,k)

        # common parameters
        self.params['mask'] = []
        self.params['plot'] = self.verify
        self.params['showprogress'] = self.showprogress
        self.params['minnrow'] = self.minnrow
        self.params['outlog'] = self.verbose
        self.params['blfile'] = self.blfile
        self.params['csvformat'] = (self.blformat.lower() == 'csv')
        self.params['insitu'] = True

##         for (k,v) in self.params.items():
##             casalog.post('%s=%s'%(k,v),'WARN')

    def __init_blfile(self):
        if self.bloutput:
            self.blfile = self.project + "_blparam.txt"

            if (self.blformat.lower() != "csv"):
                f = open(self.blfile, "w")

                blf = self.blfunc.lower()
                mm = self.maskmode.lower()
                if blf == 'poly':
                    ftitles = ['Fit order']
                elif blf == 'chebyshev':
                    ftitles = ['Fit order']
                elif blf == 'cspline':
                    ftitles = ['nPiece']
                else: # sinusoid
                    ftitles = ['applyFFT', 'fftMethod', 'fftThresh', 'addWaveN', 'rejWaveN']
                if mm == 'auto':
                    mtitles = ['Threshold', 'avg_limit', 'Edge']
                elif mm == 'list':
                    mtitles = []
                else: # interact
                    mtitles = []
                ctitles = ['clipThresh', 'clipNIter']
                    
                fkeys = getattr(self, '%s_keys'%(self.blfunc))
                mkeys = getattr(self, '%s_keys'%(self.maskmode))

                info = [['Source Table', self.infile],
                        ['Output File', self.project],
                        ['Flux Unit', self.worker.scan.get_fluxunit()],
                        ['Abscissa', self.worker.scan.get_unit()],
                        ['Function', self.blfunc]]
                for i in xrange(len(ftitles)):
                    info.append([ftitles[i],getattr(self,fkeys[i])])
                if blf != 'poly':
                    for i in xrange(len(ctitles)):
                        info.append([ctitles[i],self.clip_keys[i]])
                info.append(['Mask mode', self.maskmode])
                for i in xrange(len(mtitles)):
                    info.append([mtitles[i],getattr(self,mkeys[i])])

                separator = "#"*60 + "\n"
                
                f.write(separator)
                for i in xrange(len(info)):
                    f.write('%12s: %s\n'%tuple(info[i]))
                f.write(separator)
                f.close()
        else:
            self.blfile = ""        

