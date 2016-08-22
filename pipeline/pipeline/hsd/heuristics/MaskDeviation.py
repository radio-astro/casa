from __future__ import absolute_import

import os
import pylab as PL
import numpy as NP
import asap as sd

import pipeline.infrastructure.api as api

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

def _calculate(worker, consider_flag=False):
    worker.SubtractMedian(threshold=3.0, consider_flag=consider_flag)
    worker.CalcStdSpectrum(consider_flag=consider_flag)
    #worker.PlotSpectrum()
    worker.CalcRange(threshold=3.0, detection=5.0, extension=2.0, iteration=10, consider_flag=consider_flag)
    #worker.SavePlot()
    mask_list = worker.masklist
    return mask_list

class MaskDeviationHeuristic(api.Heuristic):
    def calculate(self, infile, spw=None):
        """
        Channel mask heuristics using MaskDeviation algorithm implemented 
        in MaskDeviation class. 
        
        infile -- input Scantable filename
        spw -- target spw id
        """
        worker = MaskDeviation(infile, spw)
        worker.ReadData()
        mask_list = _calculate(worker)
        del worker
        return mask_list
    
class MaskDeviationHeuristicForMS(api.Heuristic):
    def calculate(self, vis, field_id='', antenna_id='', spw_id='', consider_flag=False):
        worker = MaskDeviation(vis, spw_id)
        worker.ReadDataFromMS(field=field_id, antenna=antenna_id)
        mask_list = _calculate(worker, consider_flag=consider_flag)
        del worker
        return mask_list
        

def VarPlot(infile):
    # infile is asap format
    import MaskDeviation
    reload(MaskDeviation)
    s = MaskDeviation.MaskDeviation(infile)
    s.ReadData()
    s.SubtractMedian(threshold=3.0)
    s.CalcStdSpectrum()
    s.PlotSpectrum()
    s.CalcRange(threshold=3.0, detection=5.0, extension=2.0, iteration=10)
    s.SavePlot()


class MaskDeviation(object):
    """
    The class is used to detect channels having large variation or deviation. If there's any
    emission lines or atmospheric absorption/emission on some channels, their values largely
    change according to the positional and environmental changes. Emission lines and atmospheric
    features often degrade the quality of the baseline subtraction. Therefore, channels with
    large valiation should be masked before baseline fitting order determination and baseline
    subtraction.
    """
    def __init__(self, infile, spw=None):
        self.infile = infile.rstrip('/')
        self.spw = spw
        LOG.debug('MaskDeviation.__init__: infile %s spw %s'%(os.path.basename(self.infile), self.spw))
        self.masklist = []

    def ReadData(self, infile=''):
        """
        Use casa.sd to read asap format data
        This method is used for the testing purpose
        """
        if infile != '': self.infile=infile
        ss = sd.scantable(self.infile, average=False)
        sel = sd.selector(types=[sd.srctype.pson])
        if self.spw is not None:
            sel.set_ifs([self.spw])
        ss.set_selection(sel)
        self.nrow = ss.nrow()
        s = ss._getspectrum(0)
        #self.nchan = ss.nchan()
        self.nchan = len(s)
        LOG.trace('MaskDeviation.ReadData: %s %s'%(self.nrow, self.nchan))
        self.data = NP.zeros((self.nrow, self.nchan), NP.float)
        for i in range(self.nrow):
            #sp = ss._getspectrum(i)
            #self.data[i] = NP.array(sp, NP.float)
            self.data[i] = NP.array(ss._getspectrum(i))
        ss.set_selection()
        del ss, s, sel
        
    def ReadDataFromMS(self, vis='', field='', antenna='', colname='CORRECTED_DATA'):
        """
        Reads data from input MS. 
        """
        import time
        start_time = time.time()
        if vis != '': 
            self.infile=vis
        if vis == '': 
            vis = self.infile
        spwsel = '' if self.spw is None else str(self.spw)
        mssel = {'field': str(field), 
                 'spw': str(spwsel), 
                 'scanintent': 'OBSERVE_TARGET#ON_SOURCE*'}
        LOG.debug('vis="%s"'%(vis))
        LOG.debug('mssel=%s'%(mssel))
            
        with casatools.MSReader(vis) as myms:
            mssel['baseline'] = '%s&&&'%(antenna)
            myms.msselect(mssel)
            r = myms.getdata([colname, 'FLAG'])
            npol, nchan, nrow = r['flag'].shape
            self.nrow = npol * nrow
            self.nchan = nchan
            self.data= NP.real(r[colname.lower()]).transpose((2,0,1)).reshape((nrow * npol, nchan))
            self.flag = r['flag'].transpose((2,0,1)).reshape((nrow * npol, nchan))
                        
        LOG.debug('MaskDeviation.ReadDataFromMS: %s %s'%(self.nrow, self.nchan))
        end_time = time.time()
        print 'Elapsed %s sec'%(end_time - start_time)
        
        return r
        

    def SubtractMedian(self, threshold=3.0, consider_flag=False):
        """
        Subtract median value of the spectrum from the spectrum: re-bias the spectrum.

        Initial median (MED_0) and standard deviation (STD) are caluculated for each
        spectrum. Final median value is determined by using the channels having the value
        inside the range: MED_0 - threshold * STD < VALUE < MED_0 + threshold * STD
        """
        if hasattr(self, 'flag') and consider_flag == True:
            with_flag = True
        else:
            with_flag = False
        for i in range(self.nrow):
            if with_flag:
                if NP.all(self.flag[i] == True):
                    continue
                median = NP.median(self.data[i][self.flag[i] == False])
                std = self.data[i][self.flag[i] == False].std()
            else:
                median = NP.median(self.data[i])
                std = self.data[i].std()
            # mask: True => valid, False => invalid
            mask = (self.data[i]<(median+threshold*std)) * (self.data[i]>(median-threshold*std))
            if with_flag:
                medianval = NP.median(self.data[i][NP.logical_and(mask == True, self.flag[i] == False)])
            else:
                medianval = NP.median(self.data[i][NP.where(mask == True)])
            LOG.trace('MaskDeviation.SubtractMedian: row %s %s %s %s %s %s'%(i, median, std, medianval, mask.sum(), self.nchan))
            self.data[i] -= medianval

    def CalcStdSpectrum(self, consider_flag=False):
        """
        meanSP, maxSP, minSP, ymax, ymin: used only for plotting and should be
         commented out when implemented in the pipeline
        """
        if hasattr(self, 'flag') and consider_flag == True:
            with_flag = True
        else:
            with_flag = False
            
        if with_flag:
            work_data = NP.ma.masked_array(self.data, self.flag)
        else:
            work_data = self.data
            
        self.stdSP = work_data.std(axis=0)
        self.meanSP = work_data.mean(axis=0)
        self.maxSP = work_data.max(axis=0)
        self.minSP = work_data.min(axis=0)
        self.ymax = self.maxSP.max()
        self.ymin = self.minSP.min()
        
        LOG.trace('std %s\nmean %s\n max %s\n min %s\n ymax %s ymin %s'%(self.stdSP,self.meanSP,self.maxSP,self.minSP,self.ymax,self.ymin))

    def CalcRange(self, threshold=3.0, detection=5.0, extension=2.0, iteration=10, consider_flag=False):
        """
        Find regions which value is greater than threshold.
        'threshold' is used for median calculation
        'detection' is used to detect mask region
        'extension' is used to extend the mask region
        Used data:
            self.stdSp: 1D spectrum with self.nchan channels calculated in CalcStdSpectrum
                        Each channel records standard deviation of the channel in all original spectra
        """
        if hasattr(self.stdSP, 'mask') and consider_flag == True:
            with_flag = True
            stdSP = self.stdSP.data
        else:
            with_flag = False
            stdSP = self.stdSP
            
        # mask: True => valid, False => invalid
        mask = (stdSP>-99999)
        
        if with_flag:
            mask = NP.logical_and(mask, self.stdSP.mask == False)
        
        Nmask0 = 0
        for i in range(iteration):
            median = NP.median(stdSP[NP.where(mask == True)])
            std = stdSP[NP.where(mask == True)].std()
            mask = stdSP<(median+threshold*std)
            #mask = (self.stdSP<(median+threshold*std)) * (self.stdSP>(median-threshold*std))
            if with_flag:
                mask = NP.logical_and(mask, self.stdSP.mask == False)
            Nmask = mask.sum()
            LOG.trace('MaskDeviation.CalcRange: %s %s %s %s'%(median, std, Nmask, self.nchan))
            if Nmask == Nmask0: break
            else: Nmask0 = Nmask
        # TODO
        mask = stdSP<(median+detection*std)
        if with_flag:
            mask = NP.logical_and(mask, self.stdSP.mask == False)
        LOG.trace('MaskDeviation.CalcRange: before ExtendMask %s'%(mask))
        mask = self.ExtendMask(mask, median+extension*std)
        LOG.trace('MaskDeviation.CalcRange: after ExtendMask %s'%(mask))

        self.mask = NP.arange(self.nchan)[NP.where(mask == False)]
        LOG.trace('MaskDeviation.CalcRange: self.mask=%s'%(self.mask))
        RL = (mask*1)[1:]-(mask*1)[:-1]
        LOG.trace('MaskDeviation.CalcRange: RL=%s'%(RL))
        L = NP.arange(self.nchan)[NP.where(RL==-1)]+1
        R = NP.arange(self.nchan)[NP.where(RL==1)]
        if len(self.mask) > 0 and self.mask[0] == 0: L = NP.insert(L, 0, 0)
        if len(self.mask) > 0 and self.mask[-1] == self.nchan-1: R = NP.insert(R, len(R), self.nchan-1)
        self.masklist = []
        for i in range(len(L)):
            self.masklist.append([L[i], R[i]])
        self.PlotRange(L, R)
        if len(self.mask) > 0:
            LOG.trace('MaskDeviation.CalcRange: %s %s %s %s %s'%(self.masklist, L, R, self.mask[0], self.mask[-1]))
        else:
            LOG.trace('MaskDeviation.CalcRange: %s %s %s'%(self.masklist, L, R))
        del mask, RL

    def ExtendMask(self, mask, threshold):
        """
        Extend the mask region as long as Standard Deviation value is higher than the given threshold
        """
        LOG.trace('MaskDeviation.ExtendMask: threshold = %s'%(threshold))
        for i in range(len(mask)-1):
            if mask[i] == False and self.stdSP[i+1]>threshold: mask[i+1] = False
        for i in range(len(mask)-1, 1, -1):
            if mask[i] == False and self.stdSP[i-1]>threshold: mask[i-1] = False
        return mask

    def PlotSpectrum(self):
        """
        plot max, min, mean, and standard deviation of the spectra
        """
        color = ['r', 'm', 'b', 'g', 'k']
        label = ['max', 'mean', 'min', 'STD', 'MASK']
        PL.clf()
        PL.plot(self.maxSP, color=color[0])
        PL.plot(self.meanSP, color=color[1])
        PL.plot(self.minSP, color=color[2])
        PL.plot(self.stdSP, color=color[3])
        PL.xlim(-10, self.nchan+9)
        posx = (self.nchan + 20) * 0.8 - 10
        deltax = (self.nchan + 20) * 0.05
        posy = (self.ymax - self.ymin) * 0.95 + self.ymin
        deltay = (self.ymax - self.ymin) * 0.06
        for i in range(len(label)):
            PL.text(posx, posy - i * deltay, label[i], color=color[i])
        PL.title(self.infile)

    def PlotRange(self, L, R):
        """
        Plot masked range
        """
        if len(L)>0:
            PL.vlines(L, self.ymin, self.ymax)
            PL.vlines(R, self.ymin, self.ymax)
            Y = [(self.ymax-self.ymin)*0.8+self.ymin for x in range(len(L))]
            PL.hlines(Y, L, R)

    def SavePlot(self):
        """
        Save the plot in PNG format
        """
        PL.savefig(self.infile+'.png', format='png')

