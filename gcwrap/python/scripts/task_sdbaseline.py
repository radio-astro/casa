import os
from taskinit import *

import sdutil
import asap as sd
from asap._asap import Scantable
from asap import _to_list
from asap.scantable import is_scantable
import math
import pylab as pl

@sdutil.sdtask_decorator
def sdbaseline(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, tau, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verify, verbose, bloutput, blformat, showprogress, minnrow, outfile, outform, overwrite, plotlevel):
    worker = sdbaseline_worker(**locals())
    worker.initialize()
    worker.execute()
    worker.finalize()

class sdbaseline_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdbaseline_worker,self).__init__(**kwargs)
        self.suffix = '_bs'

    def parameter_check(self):
        if (self.order < 0) and (self.blfunc in ['poly', 'chevishev']):
            msg = 'Negative order of baseline polynomial given. Exit without baselining.'
            casalog.post(msg, priority = 'SEVERE')
            raise Exception(msg)

        if (self.npiece <= 0) and (self.blfunc == 'cspline'):
            msg = 'The parameter npiece must be greater than 0. Exit without baselining.'
            casalog.post(msg, priority='SEVERE')
            raise Exception(msg)            

    def initialize_scan(self):
        sorg = sd.scantable(self.infile, average=False, antenna=self.antenna)
        
        sel = self.get_selector()
        sorg.set_selection(sel)
        del sel

        # Copy scantable when usign disk storage not to modify
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
        
        # Warning for multi-IF data
        #if ( len(s.getifnos()) > 1 and not maskmode == 'auto' ):
        if ( len(self.scan.getifnos()) > 1 and isinstance(self.masklist,list) and not self.maskmode == 'auto' ):
            casalog.post( 'The scantable contains multiple IF data.', priority = 'WARN' )
            casalog.post( 'Note the same mask(s) are applied to all IFs based on CHANNELS.', priority = 'WARN' )
            casalog.post( 'Baseline ranges may be incorrect for all but IF=%d.' % (self.scan.getif(0)), priority = 'WARN' )
            
        # do opacity (atmospheric optical depth) correction
        sdutil.doopacity(self.scan, self.tau)
    
        if (self.order < 0):
            casalog.post('Negative order of baseline polynomial given. Exit without baselining.', priority = 'WARN')
            return
        engine.execute()

        # Plot final spectrum
        engine.finalize()

    def save(self):
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)

class sdbaseline_engine(sdutil.sdtask_engine):
    def __init__(self, worker):
        super(sdbaseline_engine,self).__init__(worker)

        clip_keys = ['clipthresh', 'clipniter']
        self.poly_keys = ['order']
        self.chevishev_keys = self.poly_keys + clip_keys
        self.cspline_keys = ['npiece'] + clip_keys
        self.sinusoid_keys = ['applyfft', 'fftmethod', 'fftthresh', 'addwn', 'rejwn'] + clip_keys
        self.auto_keys = ['thresh', 'avg_limit', 'edge']
        self.list_keys = ['masklist']
        self.interact_keys = []

        self.baseline_param = sdutil.parameter_registration(self)
        self.baseline_func = None

    def __del__(self):
        del self.baseline_param

    def __register(self, key, attr=None):
        self.baseline_param.register(key, attr)

    def __get_param(self):
        return self.baseline_param.get_registered()

    def initialize(self):
        if ( abs(self.plotlevel) > 1 ):
            casalog.post( "Initial Raw Scantable:" )
            self.worker.scan._summary()

    def execute(self):
        scan = self.worker.scan
        self.__init_blfile()

        nrow = scan.nrow()

        # parse string masklist
        if isinstance(self.masklist,list):
                maskdict = {'': self.masklist}
        else:
                maskdict = scan.parse_maskexpr(self.masklist)
        basesel = scan.get_selection()

        # configure baseline function and its parameters
        self.__configure_baseline()
        
        for sif, lmask in maskdict.iteritems():
            if len(sif) > 0:
                #s.set_selection(selection=(basesel+sd.selector(ifs=[int(sif)])))
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
            self.__register('mask', msk)

            # call target baseline function with appropriate parameter set 
            self.baseline_func(**self.__get_param())

##             if (maskmode == 'auto'):
##                 if (blfunc == 'poly'):
##                     scan.auto_poly_baseline(mask=msk,order=order,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
##                 elif (blfunc == 'chebyshev'):
##                     scan.auto_chebyshev_baseline(mask=msk,order=order,clipthresh=clipthresh,clipniter=clipniter,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
##                 elif (blfunc == 'cspline'):
##                     scan.auto_cspline_baseline(mask=msk,npiece=npiece,clipthresh=clipthresh,clipniter=clipniter,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
##                 elif (blfunc == 'sinusoid'):
##                     scan.auto_sinusoid_baseline(mask=msk,applyfft=applyfft,fftmethod=fftmethod,fftthresh=fftthresh,addwn=addwn,rejwn=rejwn,clipthresh=clipthresh,clipniter=clipniter,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
##             else:
##                 if (blfunc == 'poly'):
##                     scan.poly_baseline(mask=msk,order=order,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
##                 elif (blfunc == 'chebyshev'):
##                     scan.chebyshev_baseline(mask=msk,order=order,clipthresh=clipthresh,clipniter=clipniter,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
##                 elif (blfunc == 'cspline'):
##                     scan.cspline_baseline(mask=msk,npiece=npiece,clipthresh=clipthresh,clipniter=clipniter,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
##                 elif (blfunc == 'sinusoid'):
##                     scan.sinusoid_baseline(mask=msk,applyfft=applyfft,fftmethod=fftmethod,fftthresh=fftthresh,addwn=addwn,rejwn=rejwn,clipthresh=clipthresh,clipniter=clipniter,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)

                # the above 14 lines will eventually shrink into the following 2 commands:
                #
                #sbinfo = s.create_sbinfo(blfunc=blfunc,order=order,npiece=npiece,applyfft=applyfft,fftmethod=fftmethod,fftthresh=fftthresh,addwn=addwn,rejwn=rejwn,\
                #                         masklist=masklist,maskmode=maskmode,edge=edge,threshold=threshold,chan_avg_limit=chan_avg_limit,\
                #                         clipthresh=clipthresh,clipniter=clipniter)
                #s.sub_baseline(sbinfo=sbinfo,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile)
                #
                # where
                # sbinfo = {'func':funcinfo, 'mask':maskinfo, 'clip':clipinfo}
                # and
                # funcinfo should be one of the follows:
                #     funcinfo = {'type':'poly', 'params':{'order':order}}
                #     funcinfo = {'type':'cspline', 'params':{'npiece':npiece}}
                #     funcinfo = {'type':'sspline', 'params':{'lambda':lambda}}
                #     funcinfo = {'type':'sinusoid', 'params':{'applyfft':applyfft, 'fftmethod':fftmethod, 'fftthresh':fftthresh, 'addwn':addwn, 'rejwn':rejwn}}
                # maskinfo should be one of the follows:
                #     maskinfo = {'base':masklist, 'aux':{'type':'auto', 'params':{'edge':edge, 'threshold':thresh, 'chan_avg_limit':avg_limit}}}
                #     maskinfo = {'base':masklist, 'aux':{'type':'list'}}
                #     maskinfo = {'base':masklist, 'aux':{'type':'interactive'}}
                # clipinfo should be:
                #     clipinfo = {'clipthresh':clipthresh, 'clipniter':clipniter}

            # reset selection
            if len(sif) > 0: scan.set_selection(basesel)
            
    def finalize(self):
        if ( abs(self.plotlevel) > 0 ):
            pltfile=self.project+'_bsspec.eps'
            sdutil.plot_scantable(self.worker.scan, pltfile, self.plotlevel)

    def __configure_baseline(self):
        # determine what baseline function should be called
        funcname = '%s_baseline'%(getattr(self,'blfunc').lower())
        if self.maskmode.lower() == 'auto':
            funcname = 'auto_' + funcname
        self.baseline_func = getattr(self.worker.scan,funcname)

        # register parameters for baseline function
        # parameters for auto mode
        if self.maskmode.lower() == 'auto':
            self.__register('threshold', 'thresh')
            self.__register('chan_avg_limit', 'avg_limit')
            self.__register('edge')

        # parameters that depends on baseline function
        keys = getattr(self, '%s_keys'%(self.blfunc.lower()))
        for k in keys:
            self.__register(k)

        # common parameters
        self.__register('plot', 'verify')
        self.__register('showprogress')
        self.__register('minnrow')
        self.__register('outlog', 'verbose')
        self.__register('blfile')
        self.__register('csvformat', (self.blformat.lower() == "csv"))
        self.__register('insitu', True)

##         for (k,v) in self.__get_param().items():
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
                elif blf == 'chevishev':
                    ftitles = ['Fit order', 'clipThresh', 'clipNIter']
                elif blf == 'cspline':
                    ftitles = ['nPiece', 'clipThresh', 'clipNIter']
                else: # sinusoid
                    ftitles = ['applyFFT', 'fftMethod', 'fftThresh', 'addWaveN', 'rejWaveN', 'clipThresh', 'clipNIter']
                if mm == 'auto':
                    mtitles = ['Threshold', 'avg_limit', 'Edge']
                elif mm == 'list':
                    mtitles = ['Fit Range']
                else: # interact
                    mtitles = []
                    
                fkeys = getattr(self, '%s_keys'%(self.blfunc))
                mkeys = getattr(self, '%s_keys'%(self.maskmode))

                info = [['Source Table', self.infile],
                        ['Output File', self.project],
                        ['Flux Unit', self.worker.scan.get_fluxunit()],
                        ['Abscissa', self.worker.scan.get_unit()],
                        ['Function', self.blfunc]]
                for i in xrange(len(ftitles)):
                    info.append([ftitles[i],getattr(self,fkeys[i])])
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

