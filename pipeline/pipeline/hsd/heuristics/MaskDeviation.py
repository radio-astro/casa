import pylab as PL
import numpy as NP
import asap as sd

import pipeline.infrastructure.api as api

import pipeline.infrastructure as infrastructure
LOG = infrastructure.get_logger(__name__)

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
        worker.SubtractMedian(threshold=3.0)
        worker.CalcStdSpectrum()
        worker.PlotSpectrum()
        worker.CalcRange(threshold=3.0, detection=5.0, extension=2.0, iteration=10)
        worker.SavePlot()
        mask_list = worker.masklist
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
        LOG.trace('MaskDeviation.__init__: MaskDeviation')
        self.infile = infile.rstrip('/')
        self.spw = spw
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

    def SubtractMedian(self, threshold=3.0):
        """
        Subtract median value of the spectrum from the spectrum: re-bias the spectrum.

        Initial median (MED_0) and standard deviation (STD) are caluculated for each
        spectrum. Final median value is determined by using the channels having the value
        inside the range: MED_0 - threshold * STD < VALUE < MED_0 + threshold * STD
        """
        for i in range(self.nrow):
            median = NP.median(self.data[i])
            std = self.data[i].std()
            mask = (self.data[i]<(median+threshold*std)) * (self.data[i]>(median-threshold*std))
            medianval = NP.median(self.data[i][NP.where(mask == True)])
            LOG.trace('MaskDeviation.SubtractMedian: %s %s %s %s %s'%(median, std, medianval, mask.sum(), self.nchan))
            self.data[i] -= medianval

    def CalcStdSpectrum(self):
        """
        meanSP, maxSP, minSP, ymax, ymin: used only for plotting and should be
         commented out when implemented in the pipeline
        """
        self.stdSP = self.data.std(axis=0)
        self.meanSP = self.data.mean(axis=0)
        self.maxSP = self.data.max(axis=0)
        self.minSP = self.data.min(axis=0)
        self.ymax = self.maxSP.max()
        self.ymin = self.minSP.min()

    def CalcRange(self, threshold=3.0, detection=5.0, extension=2.0, iteration=10):
        """
        Find regions which value is greater than threshold.
        'threshold' is used for median calculation
        'detection' is used to detect mask region
        'extension' is used to extend the mask region
        Used data:
            self.stdSp: 1D spectrum with self.nchan channels calculated in CalcStdSpectrum
                        Each channel records standard deviation of the channel in all original spectra
        """
        mask = (self.stdSP>-99999)
        Nmask0 = 0
        for i in range(iteration):
            median = NP.median(self.stdSP[NP.where(mask == True)])
            std = self.stdSP[NP.where(mask == True)].std()
            mask = self.stdSP<(median+threshold*std)
            #mask = (self.stdSP<(median+threshold*std)) * (self.stdSP>(median-threshold*std))
            Nmask = mask.sum()
            LOG.trace('MaskDeviation.CalcRange: %s %s %s %s'%(median, std, Nmask, self.nchan))
            if Nmask == Nmask0: break
            else: Nmask0 = Nmask
        mask = self.stdSP<(median+detection*std)
        mask = self.ExtendMask(mask, median+extension*std)

        self.mask = NP.arange(self.nchan)[NP.where(mask == False)]
        RL = (mask*1)[1:]-(mask*1)[:-1]
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

