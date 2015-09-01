"""
Demonstration of algorithm to determine continuum channel ranges to 
use from a CASA image cube (dirty or clean).  All dependencies on 
analysisUtils functions have been pasted into this file for convenience.
This function is meant to be run inside CASA.  Simple usage:
  import findContinuum as fc
  fc.findContinuum('my_dirty_cube.image')
"""

import os
import numpy as np
import matplotlib.pyplot as pl
import matplotlib.ticker
import time as timeUtilities
import scipy
from scipy.stats import scoreatpercentile, percentileofscore
from taskinit import *
from imhead_cli import imhead_cli as imhead

def version(showfile=True):
    """
    Returns the CVS revision number.
    """
    myversion = "$Id: findContinuum.py,v 1.13 2015/08/30 18:22:08 we Exp $" 
    if (showfile):
        print "Loaded from %s" % (__file__)
    return myversion

def findContinuum(img, spw='', transition='', baselineModeA='min', baselineModeB='min',
                  sigmaCube=3, nBaselineChannels=0.19, sigmaFindContinuum='auto',
                  verbose=False, png='', pngBasename=False, nanBufferChannels=2, 
                  source='', useAbsoluteValue=True, trimChannels='auto', 
                  percentile=20, continuumThreshold=None, narrow='auto', 
                  separator=';', overwrite=False, titleText='', 
                  showAverageSpectrum=False, maxTrim=20, maxTrimFraction=0.33,
                  meanSpectrumFile=''):
    """
    This function calls functions to:
    1) compute the mean spectrum of a dirty cube
    2) find the continuum channels 
    3) plot the results

    Returns:
    * A channel selection string suitable for the spw parameter of clean.
    * The name of the png produced

    Inputs:
    img: the image cube to operate upon
    spw: the spw name or number to put in the x-axis label
    transition: the name of the spectral transition (for the plot title)
    baselineModeA: 'min' or 'edge', method to define the baseline in meanSpectrum()
    baselineModeB: 'min' or 'edge', method to define the baseline in findContinuumChannels()
    sigmaCube: multiply this value by the rms to get the threshold above which a pixel
               is included in the mean spectrum
    nBaselineChannels: if integer, then the number of channels to use
          if float, then the fraction of channels to use (i.e. the percentile)
          default = 0.19, which is 24 channels (i.e. 12 on each side) of a TDM window
    sigmaFindContinuum: passed to findContinuumChannels, 'auto' starts with 3
    verbose: if True, then print additional information during processing
    png: the name of the png to produce ('' yields default name)
    pngBasename: if True, then remove the directory from img name before generating png name
    nanBufferChannels: when removing or replacing NaNs, do this many extra channels
                       beyond their extent
    source: the name of the source, to be shown in the title of the spectrum.
            if None, then use the filename, up to the first underscore.
    findContinuum: if True, then find the continuum channels before plotting
    overwrite: if True, or ASCII file does not exist, then recalculate the mean spectrum
                      writing it to <img>.meanSpectrum
               if False, then read the mean spectrum from the existing ASCII file
    trimChannels: after doing best job of finding continuum, remove this many 
         channels from each edge of each block of channels found (for margin of safety)
         If it is a float between 0..1, then trim this fraction of channels in each 
         group (rounding up). If it is 'auto', use 0.1 but not more than maxTrim channels
         and not more than maxTrimFraction
    percentile: control parameter used with baselineMode='min'
    continuumThreshold: if specified, only use pixels above this intensity level
    separator: the character to use to separate groups of channels in the string returned
    narrow: the minimum number of channels that a group of channels must have to survive
            if 0<narrow<1, then it is interpreted as the fraction of all
                           channels within identified blocks
            if 'auto', then use int(ceil(log10(nchan)))
    titleText: default is img name and transition and the control parameter values
    showAverageSpectrum: make a two-panel plot, showing the raw mean spectrum in black
    maxTrim: in trimChannels='auto', this is the max channels to trim per group
    maxTrimFraction: in trimChannels='auto', the max fraction of channels to trim per group
    meanSpectrumFile: an alternative ASCII text file to use for the mean spectrum.
       Note: You still need to have the parent cube since frequency axis is read from there.
    """
    replaceNans = True 
    startTime = timeUtilities.time()
    img = img.rstrip('/')
    if (type(nBaselineChannels) == float):
        nchan, firstFreq, lastFreq = numberOfChannelsInCube(img, returnFreqs=True)
        nBaselineChannels = int(round(nBaselineChannels*nchan))
    if (nBaselineChannels < 2):
        print "You must have at least 2 edge channels"
        return
    print "Found %d channels in the cube" % (nchan)
    if (meanSpectrumFile == ''):
        meanSpectrumFile = img + '.meanSpectrum'
    elif (not os.path.exists(meanSpectrumFile)):
        meanSpectrumFile = os.path.dirname(img) + '/' + os.path.basename(meanSpectrumFile)

    if (overwrite or not os.path.exists(meanSpectrumFile)):
        if (not os.path.exists(meanSpectrumFile)):
            print "Did not find mean spectrum file = ", meanSpectrumFile
        print "Regenerating mean spectrum file."
        avgspectrum, avgSpectrumNansRemoved, avgSpectrumNansReplaced, meanSpectrumThreshold,\
          edgesUsed, nchan, nanmin = meanSpectrum(img, nBaselineChannels, sigmaCube, verbose,
                                                  nanBufferChannels,useAbsoluteValue,
                                                  baselineModeA, percentile,
                                                  continuumThreshold, meanSpectrumFile)
    else:
        result = readPreviousMeanSpectrum(meanSpectrumFile)
        if (result == None):
            print "ASCII file is not valid, re-run with overwrite=True"
            return
        avgspectrum, avgSpectrumNansReplaced, meanSpectrumThreshold, edgesUsed, nchan, nanmin = result
    donetime = timeUtilities.time()
    if verbose:
        print "%.1f sec elapsed in meanSpectrum" % (donetime-startTime)
    if (findContinuum):
        if (sigmaFindContinuum == 'auto' or sigmaFindContinuum == -1):
            sigmaFindContinuumAutomatic = True
            sigmaFindContinuum = 3.5
        else:
            sigmaFindContinuumAutomatic = False
        continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor = \
            findContinuumChannels(avgSpectrumNansReplaced, nBaselineChannels, sigmaFindContinuum, nanmin, 
                                  baselineModeB, trimChannels, narrow, verbose, maxTrim, maxTrimFraction, separator)
        sumAboveMedian, sumBelowMedian, sumRatio, channelsAboveMedian, channelsBelowMedian, channelRatio = \
            aboveBelow(avgSpectrumNansReplaced,medianTrue)
        # If there are a lot of groups or a lot of channels above the median compared to below it,
        # then lower the sigma in order to push the threshold for real lines (or line emission wings) lower.
        # However, if there is only 1 group, then there may be no real lines present, so lowering 
        # the threshold in this case can create needless extra groups.
        if ((groups > 3 or (groups > 1 and channelRatio < 1.0) or (channelRatio < 0.5)) and sigmaFindContinuumAutomatic):
            if (channelRatio < 1.0):
                factor = 0.333
            else:
                factor = np.log(3)/np.log(groups)
            sigmaFindContinuum *= factor
            print "Scaling the threshold by a factor of %.2f to try to reduce the number of groups from %d" % (factor, groups)
            continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor = \
                findContinuumChannels(avgSpectrumNansReplaced, nBaselineChannels, sigmaFindContinuum, nanmin, 
                                      baselineModeB, trimChannels, narrow, verbose, maxTrim, maxTrimFraction, separator)
            sumAboveMedian, sumBelowMedian, sumRatio, channelsAboveMedian, channelsBelowMedian, channelRatio = \
                aboveBelow(avgSpectrumNansReplaced,medianTrue)
        madOfPointsBelowThreshold = MAD(avgSpectrumNansReplaced[np.where(avgSpectrumNansReplaced<threshold)])
    pl.clf()
    if (showAverageSpectrum):
        rows = 2
    else:
        rows = 1
    cols = 1
    fontsize = 10
    ax1 = pl.subplot(rows,cols,1)
    skipchan = 2
    if replaceNans:
        avgspectrumAboveThreshold = avgSpectrumNansReplaced
    else:
        avgspectrumAboveThreshold = avgSpectrumNansRemoved
    if (edgesUsed == 2):
        if (showAverageSpectrum):
            pl.plot(range(len(avgspectrum)), avgspectrum, 'k-')
            pl.ylabel('Average spectrum', size=fontsize)
            ylimTop = pl.ylim()
            pl.subplot(rows,cols,2)
        pl.plot(range(len(avgspectrumAboveThreshold)), 
                avgspectrumAboveThreshold, 'r-')
        pl.ylabel('Avg spectrum > threshold=(%g)' % (meanSpectrumThreshold), size=fontsize)
    elif (edgesUsed == 0):
        # The upper edge is not used and can have an upward spike
        # so don't show it.
        print "Not showing final %d channels of %d" % (skipchan,\
               len(avgspectrumAboveThreshold))
        if (showAverageSpectrum):
            pl.plot(range(len(avgspectrum)), avgspectrum, 'k-')
            pl.ylabel('average spectrum')
            ylimTop = pl.ylim()
            print "A) Computed ylim = ", ylim
            ax1.ylim(ylim)
            pl.subplot(rows,cols,2)
        pl.plot(range(len(avgspectrumAboveThreshold)-skipchan), 
                avgspectrumAboveThreshold[:-skipchan], 'r-')
        pl.ylabel('average spectrum above threshold=(%g)' % meanSpectrumThreshold, size=fontsize)
    elif (edgesUsed == 1):
        # The lower edge channels are not used and the threshold mean can 
        # have an upward spike, so don't show the first channel inward from 
        # there.
        print "Not showing first %d channels of %d" % (skipchan,\
                                   len(avgspectrumAboveThreshold))
        if (showAverageSpectrum):
            pl.plot(range(len(avgspectrum)), avgspectrum, 'k-')
            pl.ylabel('average spectrum')
            ylimTop = pl.ylim()
            pl.subplot(rows,cols,2)
        pl.plot(range(skipchan,len(avgspectrumAboveThreshold)), 
                avgspectrumAboveThreshold[skipchan:], 'r-')
        pl.ylabel('average spectrum above threshold=(%g)' % meanSpectrumThreshold, size=fontsize)
        
    pl.hold(True)
    if (baselineModeA == 'edge'):
        nEdgeChannels = nBaselineChannels/2
        if (edgesUsed == 0 or edgesUsed == 2):
            pl.plot(range(nEdgeChannels), avgspectrum[:nEdgeChannels], 'm-',lw=3)
        if (edgesUsed == 1 or edgesUsed == 2):
            pl.plot(range(nchan-nEdgeChannels,nchan), avgspectrum[-nEdgeChannels:],
                    'm-', lw=3)
    if (findContinuum):
        if (len(continuumChannels) > 0):
            pl.plot(pl.xlim(), [threshold,threshold], 'k:')
            if (negativeThreshold != None):
                pl.plot(pl.xlim(), [negativeThreshold,negativeThreshold], 'k:')
            pl.plot(pl.xlim(), [median,median], 'k--')  # observed median (always lower than true for mode='min')
            pl.plot(pl.xlim(), [medianTrue,medianTrue], 'k-')
            if (baselineModeB == 'edge'):
                pl.plot([nEdgeChannels, nEdgeChannels], pl.ylim(), 'k:')
                pl.plot([nchan-nEdgeChannels, nchan-nEdgeChannels], pl.ylim(), 'k:')
            channelSelections = selection.split(separator) # was ','
            for i,ccstring in enumerate(channelSelections): 
                cc = [int(j) for j in ccstring.split('~')]
                pl.plot(cc, np.ones(len(cc))*np.mean(avgspectrumAboveThreshold), 'c-', lw=1)
                yoffset = np.mean(avgspectrumAboveThreshold)+0.04*(pl.ylim()[1]-pl.ylim()[0])
                pl.text(np.mean(cc), yoffset, ccstring, va='bottom', ha='center',size=8,rotation=90)

    if (source==None):
        source = os.path.basename(img).split('_')[0]
    if (titleText == ''):
        narrowString = pickNarrowString(narrow, len(avgSpectrumNansReplaced)) 
        trimString = pickTrimString(trimChannels, len(avgSpectrumNansReplaced), maxTrim)
        titleText = os.path.basename(img) + ' ' + transition + ' baseline=(%s,%s), narrow=%s, sigmaFindCont.=%.1f, trim=%s' % (baselineModeA,baselineModeB,narrowString,sigmaFindContinuum,trimString)
    ylim = pl.ylim()
    ylim = [ylim[0], ylim[1]+(ylim[1]-ylim[0])*0.1]
    xlim = list(pl.xlim())
    xlim[0] = 0 # Set channel 0 as left edge
    xlim[-1] = nchan-1 # Set channel -1 as right edge
    pl.xlim(xlim)
    titlesize = np.min([fontsize,int(np.floor(fontsize*100.0/len(titleText)))])
    if (spw != ''):
        label = '(Spw %s) Channel' % str(spw)
    else:
        label = 'Channel'
    if (showAverageSpectrum):
        pl.subplot(rows,cols,1)
        pl.plot(xlim, [threshold,threshold],'k:')
        pl.ylim(ylimTop)
        lowerplot = pl.subplot(rows,cols,2)
        pl.setp(ax1.get_yticklabels(), fontsize=fontsize)
        pl.text(0.5, 1.03, titleText, size=titlesize, 
                ha='center', transform=lowerplot.transAxes)
        pl.xlabel(label, size=fontsize)
        pl.ylim(ylim)
    else:
        ax1.set_xlabel(label, size=fontsize)
        pl.text(0.5, 1.08, titleText, size=titlesize, ha='center', transform=ax1.transAxes)
        pl.ylim(ylim)
    ax2 = ax1.twiny()
    pl.setp(ax1.get_xticklabels(), fontsize=fontsize)
    pl.setp(ax1.get_yticklabels(), fontsize=fontsize)
    pl.setp(ax2.get_xticklabels(), fontsize=fontsize)
    ax2.set_xbound(firstFreq*1e-9,lastFreq*1e-9)
    freqRange = np.abs(lastFreq-firstFreq)
    power = int(np.log10(freqRange))-9
    ax2.xaxis.set_major_locator(matplotlib.ticker.MultipleLocator(10**power))
    if (len(ax2.get_xticks()) < 2):
        ax2.xaxis.set_major_locator(matplotlib.ticker.MultipleLocator(0.5*10**power))
    ax2.xaxis.set_minor_locator(matplotlib.ticker.MultipleLocator(0.1*10**power))
    ax2.xaxis.set_major_formatter(matplotlib.ticker.ScalarFormatter(useOffset=False))
    if (spw != ''):
        label = '(Spw %s) Frequency (GHz)' % str(spw)
    else:
        label = 'Frequency (GHz)'
    ax2.set_xlabel(label, size=fontsize)
    inc = 0.03
    if showAverageSpectrum: inc *= 2
    pl.text(0.5,0.99-inc,'rms=MAD*1.482: of baseline chans = %f, scaled by %.1f for all chans = %f'%(mad,correctionFactor,mad*correctionFactor), 
            transform=ax1.transAxes, ha='center',size=fontsize)
    pl.text(0.017,0.99-2*inc,'lineStrength factor: %.2f' % (lineStrengthFactor), transform=ax1.transAxes, ha='left', size=fontsize)
    pl.text(0.983,0.99-2*inc,'MAD*1.482: of points below upper dotted line = %f' % (madOfPointsBelowThreshold),
            transform=ax1.transAxes, ha='right', size=fontsize)
    pl.text(0.5,0.99-3*inc,'median: of %d baseline chans = %f, offset by %.1f*MAD for all chans = %f'%(nBaselineChannels, median,medianCorrectionFactor,medianTrue-median), 
            transform=ax1.transAxes, ha='center', size=fontsize)
    pl.text(0.5,0.99-4*inc,'chans above median: %d (%.5f), below median: %d (%.5f), ratio: %.2f (%.2f)'%(channelsAboveMedian,sumAboveMedian,channelsBelowMedian,sumBelowMedian,channelRatio,sumRatio),
            transform=ax1.transAxes, ha='center', size=fontsize)
    # Write CVS version
    pl.text(1.03, -0.005-2*inc, ' '.join(version().split()[1:4]), size=8, 
            transform=ax1.transAxes, ha='right')
    pl.draw()
    if (png == ''):
        if pngBasename:
            png = os.path.basename(img)
        else:
            png = img
        transition = transition.replace('(','_').replace(')','_').replace(' ','_').replace(',','')
        if (len(transition) > 0):
            transition = '.' + transition
        narrowString = pickNarrowString(narrow, len(avgSpectrumNansReplaced)) # this is used later
        trimString = pickTrimString(trimChannels, len(avgSpectrumNansReplaced), maxTrim)
        png += '.meanSpectrum.%s.%s.%.1fsigma.narrow%s.trim%s%s.png' % (baselineModeA, baselineModeB, sigmaFindContinuum, narrowString, trimString, transition)
    pl.savefig(png)
    print "Wrote png = %s." % (png)
    donetime = timeUtilities.time()
    print "%.1f sec elapsed in plotMeanSpectrum" % (donetime-startTime)
    return(selection, png)
# end of findContinuum

def aboveBelow(avgSpectrumNansReplaced, threshold):
    """
    Given an array of values (i.e. a spectrum) and a threshold value, this function
    computes the number of channels above and below that threshold and the ratio, plus
    the sum of the intensities of these respective channels and the ratio of these sums.
    """
    aboveMedian = np.where(avgSpectrumNansReplaced > threshold)
    belowMedian = np.where(avgSpectrumNansReplaced < threshold)
    sumAboveMedian = np.sum(-threshold+avgSpectrumNansReplaced[aboveMedian])
    sumBelowMedian = np.sum(threshold-avgSpectrumNansReplaced[belowMedian])
    channelsAboveMedian = len(aboveMedian[0])
    channelsBelowMedian = len(belowMedian[0])
    channelRatio = channelsAboveMedian*1.0/channelsBelowMedian
    sumRatio = sumAboveMedian/sumBelowMedian
    return(sumAboveMedian, sumBelowMedian, sumRatio, 
           channelsAboveMedian, channelsBelowMedian, channelRatio)

def writeMeanSpectrum(meanSpectrumFile, frequency, avgspectrum, 
                      avgSpectrumNansReplaced, threshold, edgesUsed, nchan, nanmin):
    """
    Writes out the mean spectrum (and the key parameters used to create it), so that
    it can quickly be restored.  This allows the user to quickly experiment with 
    different parameters of findContinuumChannels applied to the same mean spectrum.
    """
    f = open(meanSpectrumFile, 'w')
    f.write('#threshold edgesUsed nchan nanmin\n')
    f.write('%.10f %d %d %.10f\n' % (threshold, edgesUsed, nchan, nanmin))
    f.write('#chan freq(Hz) avgSpectrum avgSpectrumNansReplaced\n')
    for i in range(len(avgspectrum)):
        f.write('%d %.1f %.10f %.10f\n' % (i, frequency[i], avgspectrum[i], avgSpectrumNansReplaced[i]))
    f.close()

def readViewerOutputFile(lines, debug=False):
    """
    Reads an ASCII spectrum file produced by the CASA viewer or by tt.ispec.
    """
    x = []; y = []
    pixel = ''
    xunits = 'unknown'
    intensityUnits = 'unknown'
    for line in lines:
        if (line[0] == '#'): 
            if (line.find('pixel') > 0):
                pixel = line.split('[[')[1].split(']]')[0]
            elif (line.find('xLabel') > 0):
                xunits = line.split()[-1]
                if (debug):
                    print "Read xunits = ", xunits
            elif (line.find('yLabel') > 0):
                tokens = line.split()
                if (len(tokens) == 2):
                    intensityUnits = tokens[1]
                else:
                    intensityUnits = tokens[1] + ' (' + tokens[2] + ')'
            continue
        tokens = line.split()
        if (len(tokens) < 2): 
            continue
        x.append(float(tokens[0]))
        y.append(float(tokens[1]))
    return(np.array(x), np.array(y), xunits, intensityUnits)

def readPreviousMeanSpectrum(meanSpectrumFile):
    """
    Read a previous calculated mean spectrum and its parameters,
    or an ASCII file created by the CASA viewer (or tt.ispec).
    Note: only the intensity column(s) are returned, not the 
       channel/frequency columns.
    """
    f = open(meanSpectrumFile, 'r')
    lines  = f.readlines()
    f.close()
    if (len(lines) < 3):
        return None
    i = 0
    # Detect file type:
    if (lines[0].find('title: Spectral profile') > 0):
        # CASA viewer/au.ispec output file
        print "Reading CASA viewer output file"
        freq, intensity, freqUnits, intensityUnits = readViewerOutputFile(lines)
        if (freqUnits.lower().find('ghz')>=0):
            freq *= 1e9
        elif (freqUnits.lower().find('mhz')>=0):
            freq *= 1e6
        elif (freqUnits.lower().find('[hz')<0):
            print "Spectral axis of viewer output file must be in Hz, MHz or GHz, not %s." % freqUnits
            return
        threshold = 0
        edgesUsed = 2
        return(intensity, intensity, threshold,
               edgesUsed, len(intensity), np.nanmin(intensity))
    while (lines[i][0] == '#'):
        i += 1
    a,b,c,d = lines[i].split()
    threshold = float(a)
    edgesUsed = int(b)
    nchan = int(c)
    nanmin = float(d)
    avgspectrum = []
    avgSpectrumNansReplaced = []
    freqs = []
    channels = []
    for line in lines[i+1:]:
        if (line[0] == '#'): continue
        tokens = line.split()
        if (len(tokens) == 2):
            # continue to support the old 2-column format
            a,b = line.split()
        else:
            chan,freq,a,b = line.split()
            channels.append(int(chan))
            freqs.append(float(freq))
        avgspectrum.append(float(a))
        avgSpectrumNansReplaced.append(float(b))
    print "Read previous mean spectrum with %d channels" % (len(avgspectrum))
    avgspectrum = np.array(avgspectrum)    
    avgSpectrumNansReplaced = np.array(avgSpectrumNansReplaced)
    return(avgspectrum, avgSpectrumNansReplaced, threshold,
           edgesUsed, nchan, nanmin)

def findContinuumChannels(spectrum, nBaselineChannels=16, sigmaFindContinuum=3, 
                          nanmin=None, baselineMode='min', trimChannels='auto',
                          narrow='auto', verbose=False, maxTrim=20, 
                          maxTrimFraction=0.33, separator=';'): 
    """
    Trys to find continuum channels in a spectrum, based on a threshold or
    some number of edge channels and their median and standard deviation.
    Inputs:
    spectrum: a one-dimensional array of intensity values
    nBaselineChannels: number of channels over which to compute standard deviation
    sigmaFindContinuum: value to multiply the standard deviation by then add 
      to median to get threshold.  Default = 3.  
    narrow: the minimum number of channels in a contiguous block to accept 
            if 0<narrow<1, then it is interpreted as the fraction of all
                           channels within identified blocks
    nanmin: the value that NaNs were replaced by in previous steps
    baselineMode: 'min' or 'edge',  'edge' will use nBaselineChannels/2 from each edge
    trimChannels: if integer, use that number of channels.  If float between
      0 and 1, then use that fraction of channels in each contiguous list
      (rounding up). If 'auto', then use 0.1 but not more than maxTrim channels.
    maxTrim: in trimChannels='auto', this is the max channels to trim per group
    maxTrimFraction: in trimChannels='auto', the max fraction of channels to trim per group
    separator: the character to use to separate groups of channels in the string returned

    Returns:
    1  list of channels to use (separated by the specified separator)
    2  list converted to ms channel selection syntax
    3  positive threshold used
    4  median of the baseline-defining channels
    5  number of groups found
    6  correctionFactor used
    7  inferred true median
    8  scaled MAD of the baseline-defining channels
    9  correction factor applied to get the inferred true median
    10 negative threshold used
    11 lineStrength factor
    """
    if (narrow == 'auto'):
        narrow = pickNarrow(len(spectrum))
    npts = len(spectrum)
    percentile = 100.0*nBaselineChannels/npts
    correctionFactor = sigmaCorrectionFactor(baselineMode, npts, percentile)
    sigmaEffective = sigmaFindContinuum*correctionFactor
    print "Using sigmaFindContinuum=%.1f, sigmaEffective=%.1f, percentile=%.0f for mode=%s, channels=%d/%d" % (sigmaFindContinuum, sigmaEffective, percentile, baselineMode, nBaselineChannels, len(spectrum))
    if (baselineMode == 'edge'):
        # pick n channels on both edges
        lowerChannels = spectrum[:nBaselineChannels/2]
        upperChannels = spectrum[-nBaselineChannels/2:]
        allBaselineChannels = list(lowerChannels) + list(upperChannels)
        otherChannels = spectrum[nBaselineChannels/2:-nBaselineChannels/2]
        if (np.std(lowerChannels) == 0):
            mad = MAD(upperChannels)
            median = nanmedian(upperChannels)
            print "edge method: Dropping lower channels from median and std calculations"
        elif (np.std(upperChannels) == 0):
            mad = MAD(lowerChannels)
            median = nanmedian(lowerChannels)
            print "edge method: Dropping upper channels from median and std calculations"
        else:
            mad = MAD(allBaselineChannels)
            median = nanmedian(allBaselineChannels)
    else:
        # pick the n channels with the n lowest values
        idx = np.argsort(spectrum)
        allBaselineChannels = spectrum[idx[:nBaselineChannels]] 
        otherChannels = spectrum[idx[nBaselineChannels:]] 
        mad = MAD(allBaselineChannels)
        if (mad < 1e-17):  # avoid blocks of identically-zero values
            myspectrum = spectrum[np.where(spectrum != allBaselineChannels[0])]
            allBaselineChannels = myspectrum[np.argsort(myspectrum)[:nBaselineChannels]] 
            mad = MAD(allBaselineChannels)
        mad = MAD(allBaselineChannels)
        median = nanmedian(allBaselineChannels)
        print "min method: median intensity of %d channels used as the baseline: %f, " % (len(allBaselineChannels), median)
    # will be 1.0 if no lines present and 0.25 if half the channels have lines, etc.
    signalRatio = (1.0 - 1.0*len(np.where(spectrum>(median+sigmaEffective*mad*2.0))[0]) / len(spectrum))**2
    lineStrengthFactor = 1.0/signalRatio
    medianTrue = medianCorrected(baselineMode, percentile, median, mad, signalRatio)
    threshold = sigmaEffective*mad + medianTrue
    # Use a 40% lower negative threshold to help prevent false identification of absorption features.
    negativeThreshold = -1.4*sigmaEffective*mad + medianTrue
    print "MAD = %f, median = %f, trueMedian=%f" % (mad, median, medianTrue)
    print "findContinuumChannels: computed threshold = ", threshold

    channels = np.where(spectrum < threshold)[0]
    if (negativeThreshold != None):
        channels2 = np.where(spectrum > negativeThreshold)[0]
        channels = np.intersect1d(channels,channels2)

    selection = convertChannelListIntoSelection(channels)
    print "Found %d potential continuum channels: %s" % (len(channels), str(selection))
    if (len(channels) == 0):
        selection = ''
        groups = 0
    else:
        channels = splitListIntoContiguousListsAndRejectZeroStd(channels, spectrum, nanmin, verbose=verbose)
        if verbose: 
            print "channels = ", channels
        selection = convertChannelListIntoSelection(channels,separator=separator)
        groups = len(selection.split(separator))
        print "Found %d channels after rejecting zero std: %s" % (len(channels), str(selection))
        if (len(channels) == 0):
            selection = ''
        else:
            if verbose:
                print "Calling splitListIntoContiguousListsAndTrim(channels=%s, trimChan=%s)" % (str(channels), str(trimChannels))
            channels = splitListIntoContiguousListsAndTrim(channels, trimChannels, maxTrim, maxTrimFraction)
            selection = convertChannelListIntoSelection(channels)
            groups = len(selection.split(separator))
            if verbose:
                print "Found %d groups of channels = " % (groups), channels
            if (groups > 1):
                if verbose:
                    print "Calling splitListIntoContiguousListsAndRejectNarrow(channels=%s, narrow=%s)" % (str(channels), str(narrow))
                trialChannels = splitListIntoContiguousListsAndRejectNarrow(channels, narrow)
                if (len(trialChannels) > 0):
                    channels = trialChannels
                    print "Found %d channels after trimming %s channels and rejecting narrow groups." % (len(channels),str(trimChannels))
                    selection = convertChannelListIntoSelection(channels)
                    groups = len(selection.split(separator))
            else:
                print "Not rejecting narrow groups since there is only one group!"
    print "Found %d continuum channels in %d groups: %s" % (len(channels), groups, selection)
    return(channels, selection, threshold, median, groups, correctionFactor, 
           medianTrue, mad, computeMedianCorrectionFactor(baselineMode, percentile)*signalRatio,
           negativeThreshold, lineStrengthFactor)

def splitListIntoContiguousListsAndRejectNarrow(channels, narrow=3):
    """
    Split a list of channels into contiguous lists, and reject those that
    have a small number of channels.
    narrow: if >=1, then interpret it as the minimum number of channels that
                  a group must have in order to survive
            if <1, then interpret it as the minimum fraction of channels that
                  a group must have relative to the total number of channels
    Returns: a new single list
    Example:  [1,2,3,4,6,7,9,10,11] --> [ 1,  2,  3,  4,  9, 10, 11]
    """
    length = len(channels)
    mylists = splitListIntoContiguousLists(channels)
    channels = []
    for mylist in mylists:
        if (narrow < 1):
            if (len(mylist) <= fraction*length):
                continue
        elif (len(mylist) < narrow):
            continue
        channels += mylist
    return(np.array(channels))

def splitListIntoContiguousListsAndTrim(channels, trimChannels=0.1, maxTrim=20, 
                                        maxTrimFraction=0.33):
    """
    Split a list of channels into contiguous lists, and trim some number
    of channels from each edge.
    trimChannels: if integer, use that number of channels.  If float between
        0 and 1, then use that fraction of channels in each contiguous list
        (rounding up). If 'auto', then use 0.1 but not more than maxTrim 
        channels and not more than maxTrimFraction of channels.
    maxTrim: used in 'auto' mode
    Returns: a new single list
    """
    if (trimChannels <= 0):
        return(np.array(channels))
    length = len(channels)
    if (trimChannels == 'auto'):
        trimChannels = pickAutoTrimChannels(length, maxTrim)
    mylists = splitListIntoContiguousLists(channels)
    channels = []
    trimChan = trimChannels
    for mylist in mylists:
        if (trimChannels < 1):
            trimChan = int(np.ceil(len(mylist)*trimChannels))
        if (len(mylist) < 1+trimChan*2):
            continue
        if (trimChannels == 'auto' and 1.0*trimChan/len(mylist) > maxTrimFraction):
            trimChan = int(np.floor(maxTrimFraction*len(mylist)))
        channels += mylist[trimChan:-trimChan]
    return(np.array(channels))

def roundFigures(value, digits):
    """
    This function rounds a floating point value to a number of significant figures.
    value: value to be rounded (between 1e-20 and 1e+20)
    digits: number of significant digits, both before or after decimal point
    """
    for r in range(-20,20):
        if (round(value,r) != 0.0):
            value = round(value,r+digits)
    return(value)

def pickAutoTrimChannels(length, maxTrim):
    """
    Automatic choice of number of trimChannels as a function of the number of channels in an spw.
    """
    trimChannels = 0.1
    if (length*trimChannels > maxTrim):
        trimChannels = maxTrim
    return(trimChannels)

def pickTrimString(trimChannels, length, maxTrim):
    """
    Generate a string describing the setting of the trimChannels parameter.
    """
    if (trimChannels=='auto'):
        trimString = 'auto=%g' % pickAutoTrimChannels(length, maxTrim)
    else:
        trimString = '%g' % trimChannels
    return(trimString)

def pickNarrowString(narrow, length):
    """
    Generate a string describing the setting of the narrow parameter.
    """
    if (narrow=='auto'):
        narrowString = 'auto=%g' % pickNarrow(length)
    else:
        narrowString = '%g' % narrow
    return(narrowString)

def pickNarrow(length):
    """
    Automatically picks a setting for the narrow parameter based on the number 
    of channels in the spectrum.  This formula results in:
    length: 64,128,240,480,960,1920,3840,7680:
    return:  2,  3,  3,  3,  3,   4,   4,   4  (ceil(log10))
             2,  2,  2,  3,  3,   3,   4,   4  (round(log10))
             1,  2,  2,  2,  2,   3,   3,   3  (floor(log10))
             5,  5,  6,  7,  7,   8,   9,   9  (ceil(log))
             4,  5,  5,  6,  7,   8,   8,   9  (round(log))
             4,  4,  5,  6,  6,   7,   8,   8  (floor(log))
    """
    return(int(np.ceil(np.log10(length))))

def sigmaCorrectionFactor(baselineMode, npts, percentile):
    """
    Computes the correction factor for the fact that the measured rms (or MAD) on the 
    N%ile lowest points of a datastream will be less than the true rms (or MAD) of all 
    points.
    """
    edgeValue = (npts/128.)**0.08
    if (baselineMode == 'edge'):
        return(edgeValue)
    return(edgeValue*2.8*(percentile/10.)**-0.25)

def medianCorrected(baselineMode, percentile, median, mad, signalRatio):
    """
    Computes the true median of a datastream from the observed median and MAD of
    the lowest Nth percentile points.
    signalRatio: when there is a lot of signal, we need to reduce the correction 
                 factor because it is less like Gaussian noise
                 It is 1.0 if no lines present, 0.25 if half the channels have signal, etc.
    """
    return(median + computeMedianCorrectionFactor(baselineMode, percentile)*mad*signalRatio)

def computeMedianCorrectionFactor(baselineMode, percentile):
    """
    Computes the effect due to the fact that taking the median of the
    N%ile lowest points of a Gaussian noise datastream will be lower than the true 
    median of all the points.  This is (TrueMedian-ObservedMedian)/ObservedMAD
    """
    if (baselineMode == 'edge'):
        return(0)
    return(6.3*(5.0/percentile)**0.5)

def numberOfChannelsInCube(img, returnFreqs=False, verbose=False):
    """
    Finds the number of channels in a CASA or FITS image cube.
    returnfreqs: if True, then also return the frequency of the
           first and last channel (in Hz)
    verbose: if True, then print the frequencies of first and last channel
    -Todd Hunter
    """
    header = imhead(img,mode='list')
    if (header == None):
        print "imhead failed -- this may not be a CASA or FITS image cube."
        return
    nchan = 1
    for axis in range(3,5):
        if ('ctype'+str(axis) in header.keys()):
            if (header['ctype'+str(axis)] in ['Frequency','Velocity']):
                nchan = header['shape'][axis-1]
                break
    firstFreq = 0
    lastFreq = 0
    if ('ctype4' not in header.keys()):
        print "There is no fourth axis in this image."
    elif (header['ctype4'].lower().find('freq') >= 0):
        crpix = header['crpix4']
        crval = header['crval4']
        cdelt = header['cdelt4']
        firstFreq = crval + (0-crpix)*cdelt
        lastFreq = crval + (nchan-1-crpix)*cdelt
        if (verbose):
            print "Channel  0 = %.0f Hz" % (firstFreq)
            print "Channel %d = %.0f Hz" % (nchan-1,lastFreq)
    elif (header['ctype3'].lower().find('freq') >= 0):
        crpix = header['crpix3']
        crval = header['crval3']
        cdelt = header['cdelt3']
        firstFreq = crval + (0-crpix)*cdelt
        lastFreq = crval + (nchan-1-crpix)*cdelt
        if (verbose):
            print "Channel  0 = %.0f Hz" % (firstFreq)
            print "Channel %d = %.0f Hz" % (nchan-1,lastFreq)
    else:
        print "Neither the third or fourth axis is frequency."
    if (returnFreqs):
        return(nchan,firstFreq,lastFreq)
    else:
        return(nchan)

def nanmean(a, axis=0):
    """
    Takes the mean of an array, ignoring the nan entries
    """
    if (np.__version__ < '1.81'):
        return(scipy.stats.nanmean(a,axis)) 
#        length = len(np.array(a)[np.where(np.isnan(a)==False)])
#        return(np.nansum(a,axis)/length)
    else:
        return(np.nanmean(a,axis))

def nanmedian(a, axis=0):
    """
    Takes the mean of an array, ignoring the nan entries
    """
    if (np.__version__ < '1.81'):
        return(scipy.stats.nanmedian(a,axis)) 
    else:
        return(np.nanmedian(a,axis))

def avgOverCube(pixels, useAbsoluteValue=False, threshold=None, median=False):
    """
    Computes the average spectrum across a multi-dimensional
    array read from an image cube, ignoring any NaN values.
    If threshold is specified, then it only includes pixels
    with an intensity above that value.
    """
    if (useAbsoluteValue):
        pixels = np.abs(pixels)
    for i in range(len(np.shape(pixels))-1):
        if (median):
            pixels = nanmedian(pixels, axis=0)
        else:
            if (threshold != None):
                pixels[np.where(pixels < threshold)] = np.nan
            pixels = nanmean(pixels, axis=0)
    return(pixels)

def meanSpectrum(img='g35.03_KDnh3_11.hline.self.image', nBaselineChannels=16,
                 sigmaCube=3, verbose=False, nanBufferChannels=2, useAbsoluteValue=False,
                 baselineMode='edge', percentile=20, continuumThreshold=None,
                 meanSpectrumFile=''):
    """
    Computes the average spectrum across a CASA image cube, using a selection of
    baseline channels to compute the rms to be used as a threshold value (similar to
    constructing a moment map).
    nBaselineChannels: number of channels to use as the baseline
    baselineMode: how to select the channels to use as the baseline:
              'edge': use an equal number of channels on each edge of the spw
               'min': use the percentile channels with the lowest absolute intensity
    sigmaCube: multiply this value by the rms to get the threshold above which a pixel
               is included in the mean spectrum
    nanBufferChannels: when removing or replacing NaNs, do this many extra channels
                       beyond their actual extent
    percentile: used with baselineMode='min'
    continuumThreshold: if specified, only use pixels above this intensity level
    meanSpectrumFile: name of ASCII file to produce to speed up future runs 
    Returns 6 items:
       * avgspectrum (vector)
       * avgspectrumAboveThresholdNansRemoved (vector)
       * avgspectrumAboveThresholdNansReplaced (vector)
       * threshold (scalar) 
       * edgesUsed: 0=lower, 1=upper, 2=both
       * nchan (scalar)
    """
    if (not os.path.exists(img)):
        print "Could not find image = ", img
        return
    axis = 3 # findSpectralAxis(img)
    if verbose: print "Assuming spectral axis = ", axis
    myia = createCasaTool(iatool)
    myia.open(img)
    pixels = myia.getregion()
    if (continuumThreshold != None):
        pixels[np.where(pixels < continuumThreshold)] = 0.0
    std = MAD(pixels)
    if verbose: print "MAD of whole cube = ", std
    naxes = len(np.shape(pixels))
    nchan = np.shape(pixels)[axis]

    # Method #1: Use the two edges of the spw to find the line-free rms of the spectrum
    nEdgeChannels = nBaselineChannels/2
    # lower edge
    blc = np.zeros(naxes)
    trc = [i-1 for i in list(np.shape(pixels))]
    trc[3] = nEdgeChannels
    myrg = createCasaTool(rgtool)
    region = myrg.box(blc=blc, trc=trc)
    lowerEdgePixels = myia.getregion(region=region)
    # drop all floating point zeros (which will drop pixels outside the mosaic image mask)
    lowerEdgePixels = lowerEdgePixels[np.where(lowerEdgePixels!=0.0)]
    stdLowerEdge = MAD(lowerEdgePixels)
    medianLowerEdge = nanmedian(lowerEdgePixels)
    if verbose: print "MAD of %d channels on lower edge = %f" % (nBaselineChannels, stdLowerEdge)

    # upper edge
    blc = np.zeros(naxes)
    trc = [i-1 for i in list(np.shape(pixels))]
    blc[3] = trc[3] - nEdgeChannels
    region = myrg.box(blc=blc, trc=trc)
    upperEdgePixels = myia.getregion(region=region)
    myrg.done()
    # drop all floating point zeros
    upperEdgePixels = upperEdgePixels[np.where(upperEdgePixels!=0.0)]
    stdUpperEdge = MAD(upperEdgePixels)
    medianUpperEdge = nanmedian(upperEdgePixels)
    print "meanSpectrum(): edge medians: lower=%.10f, upper=%.10f" % (medianLowerEdge, medianUpperEdge)

    if verbose: 
        print "MAD of %d channels on upper edge = %f" % (nEdgeChannels, stdUpperEdge)
    if (stdLowerEdge <= 0.0):
        edgesUsed = 1
        stdEdge = stdUpperEdge
        medianEdge = medianUpperEdge
    elif (stdUpperEdge <= 0.0):
        edgesUsed = 0
        stdEdge = stdLowerEdge
        medianEdge = medianLowerEdge
    else:
        edgesUsed = 2
        stdEdge = np.mean([stdLowerEdge,stdUpperEdge])
        medianEdge = np.mean([medianLowerEdge,medianUpperEdge])

    # Method #2: pick the N channels with the lowest absolute values (to avoid
    #          confusion from absorption lines and negative bowls of missing flux)
    xpix =  np.shape(pixels)[0]
    ypix =  np.shape(pixels)[1]
    npixFraction = nBaselineChannels*1.0/nchan
    allPixels = myia.getregion()
    myia.close()
    # Convert all NaNs to zero
    allPixels[np.isnan(allPixels)] = 0
    # Drop all floating point zeros from calculation
    allPixels = allPixels[np.where(allPixels != 0)]
    # Take absolute value
    absPixels = np.abs(allPixels)
    # Find the lowest pixel values
    percentileThreshold = scoreatpercentile(absPixels, percentile)
    idx = np.where(absPixels < percentileThreshold)
    # Take their statistics
    stdMin = MAD(allPixels[idx])
    medianMin = nanmedian(allPixels[idx])

    print "meanSpectrum(): edge mode:  median=%f  MAD=%f  threshold=%f (edgesUsed=%d)" % (medianEdge, stdEdge, medianEdge+stdEdge*sigmaCube, edgesUsed)
    print "meanSpectrum(): min mode:  median=%f  MAD=%f  threshold=%f" % (medianMin, stdMin, medianMin+stdMin*sigmaCube)
    if (baselineMode == 'edge'):
        std = stdEdge
        median = medianEdge
    else:
        std = stdMin
        median = medianMin
        
    avgspectrum = avgOverCube(pixels, useAbsoluteValue)
    threshold = median + sigmaCube*std
    if verbose: 
        print "Using threshold above which to compute mean spectrum = ", threshold
    pixels[np.where(pixels < threshold)] = 0.0
    avgspectrumAboveThreshold = avgOverCube(pixels, useAbsoluteValue, threshold)
    nansRemoved = removeNaNs(avgspectrumAboveThreshold)
    nansReplaced,nanmin = removeNaNs(avgspectrumAboveThreshold, replaceWithMin=True, 
                                     nanBufferChannels=nanBufferChannels)
    nchan, firstFreq, lastFreq = numberOfChannelsInCube(img, returnFreqs=True)
    frequency = np.linspace(firstFreq, lastFreq, nchan)
    writeMeanSpectrum(meanSpectrumFile, frequency, avgspectrum, nansReplaced, threshold,
                      edgesUsed, nchan, nanmin)
    return(avgspectrum, nansRemoved, nansReplaced, threshold, 
           edgesUsed, nchan, nanmin)

def removeNaNs(a, replaceWithMin=False, verbose=False, nanBufferChannels=0, replaceWithZero=False):
    """
    Remove or replace the nan values from an array.
    replaceWithMin: if True, then replace NaNs with np.nanmin of array
                    if False, then simply remove the NaNs
    replaceWithZero: if True, then replace NaNs with np.nanmin of array
                    if False, then simply remove the NaNs
    nanBufferChannels: only active if replaceWithMin=True
    Returns:
    * new array
    * if replaceWithMin=True, then also return the value used to replace NaNs
    """
    a = np.array(a) 
    if (len(a) < 1): return(a)
    startLength = len(a)
    if (replaceWithMin or replaceWithZero):
        idx = np.isnan(a)
        a_nanmin = np.nanmin(a)
        if (nanBufferChannels > 0):
            idxlist = splitListIntoHomogeneousLists(idx)
            idx = []
            for i,mylist in enumerate(idxlist):
                if (mylist[0]):
                    idx += mylist
                else:
                    newSubString = nanBufferChannels*[True] 
                    if (i < len(idxlist)-1):
                        newSubString += mylist[nanBufferChannels:-nanBufferChannels] + nanBufferChannels*[True]
                    else:
                        newSubString += mylist[-nanBufferChannels:]
                    # If the channel block was less than 2*nanBufferChannels wide, then only insert up to its width
                    idx += newSubString[:len(mylist)]
            idx = np.array(idx)
        if (verbose):
            print "Replaced %d NaNs" % (len(idx))
        if (replaceWithMin):
            a[idx] = a_nanmin
            return(a, a_nanmin)
        elif (replaceWithZero):
            a[idx] = 0
            return(a)
    else:
        a = a[np.where(np.isnan(a) == False)]
        if (verbose):
            print "Removed %d NaNs" % (startLength-len(a))
        return(a)

def createCasaTool(mytool):
    """
    A wrapper to handle the changing ways in which casa tools are invoked.
    Todd Hunter
    """
    if (type(casac.Quantity) != type):  # casa 4.x
        myt = mytool()
    else:  # casa 3.x
        myt = mytool.create()
    return(myt)

def MAD(a, c=0.6745, axis=0):
    """
    Median Absolute Deviation along given axis of an array:
         median(abs(a - median(a))) / c
    c = 0.6745 is the constant to convert from MAD to std 
    """
    a = np.array(a)
    good = (a==a)
    a = np.asarray(a, np.float64)
    if a.ndim == 1:
        d = nanmedian(a[good])
        m = nanmedian(np.fabs(a[good] - d) / c)
    else:
        d = nanmedian(a[good], axis=axis)
        # I don't want the array to change so I have to copy it?
        if axis > 0:
            aswp = swapaxes(a[good],0,axis)
        else:
            aswp = a[good]
        m = nanmedian(np.fabs(aswp - d) / c, axis=0)
    return m

def splitListIntoContiguousLists(mylist):
    """
    Called by copyweights. See also splitListIntoHomogenousLists.
    Converts [1,2,3,5,6,7] into [[1,2,3],[5,6,7]], etc.
    -Todd Hunter
    """
    mylists = []
    if (len(mylist) < 1):
        return(mylists)
    newlist = [mylist[0]]
    for i in range(1,len(mylist)):
        if (mylist[i-1] != mylist[i]-1):
            mylists.append(newlist)
            newlist = [mylist[i]]
        else:
            newlist.append(mylist[i])
    mylists.append(newlist)
    return(mylists)

def splitListIntoContiguousListsAndRejectZeroStd(channels, values, nanmin=None, verbose=False):
    """
    Takes a list of numerical values, splits into contiguous lists and 
    removes those that have zero standard deviation in their associated values.
    Note that values *must* hold the values of the whole array, while channels 
    can be a subset.
    If nanmin is specified, then reject lists that contain more than 3 
    appearances of this value.
    """
    if verbose:
        print "splitListIntoContiguousListsAndRejectZeroStd:  len(values)=%d, len(channels)=%d" % (len(values), len(channels))
    values = np.array(values)
    mylists = splitListIntoContiguousLists(channels)
    channels = []
    for i,mylist in enumerate(mylists):
        if verbose: print "Running np.std(%s)" % (str(values[mylist]))
        mystd = np.std(values[mylist])
        if (mystd > 1e-17):  # avoid blocks of identically-zero values
            if (nanmin != None):
                minvalues = len(np.where(values[i] == nanmin)[0])
                if (float(minvalues)/len(mylist) > 0.1 and minvalues > 3):
                    print "Rejecting list %d with multiple min values (%d)" % (i,minvalues)
                    continue
            channels += mylist
    return(np.array(channels))

def splitListIntoHomogeneousLists(mylist):
    """
    Converts [1,1,1,2,2,3] into [[1,1,1],[2,2],[3]], etc.
    -Todd Hunter
    """
    mylists = []
    newlist = [mylist[0]]
    for i in range(1,len(mylist)):
        if (mylist[i-1] != mylist[i]):
            mylists.append(newlist)
            newlist = [mylist[i]]
        else:
            newlist.append(mylist[i])
    mylists.append(newlist)
    return(mylists)
    
def convertChannelListIntoSelection(channels, trim=0, separator=';'):
    """
    Converts a list of channels into casa selection string syntax.
    channels: a list of channels
    trim: the number of channels to trim off of each edge of a block of channels
    """
    selection = ''
    firstList = True
    if (len(channels) > 0):
        mylists = splitListIntoContiguousLists(channels)
        for mylist in mylists:
            if (mylist[0]+trim <= mylist[-1]-trim):
                if (not firstList):
                    selection += separator
                selection += '%d~%d' % (mylist[0]+trim, mylist[-1]-trim)
                firstList = False
    return(selection)
