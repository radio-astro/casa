"""
Demonstration of algorithm to determine continuum channel ranges to 
use from a CASA cube.

This version was sent to D. Muders on 2015-06-11. All dependencies on
analysisUtils functions have been pasted into this file for
convenience.

This function is meant to be run inside CASA.  Simple usage:
import findContinuum as fc
fc.findContinuum('my_dirty_cube.image')
"""

import os
import numpy as np
import pylab as pl
import time as timeUtilities
import scipy
from scipy.stats import scoreatpercentile, percentileofscore
from taskinit import *
from imhead_cli import imhead_cli as imhead

def findContinuum(img, transition='', baselineModeA='min', baselineModeB='min', sigmaCube=3,
                  nBaselineChannels = 0.19, sigmaFindContinuum=-1,
                  replaceNans=True, verbose=False, png='', 
                  pngBasename=False, nanBufferChannels=2, source='',
                  useAbsoluteValue=True, trimChannels=0.1, percentile=20,
                  continuumThreshold=None):
    """
    This function calls tt.plotMeanSpectrum, which calls a function to find 
    the continuum channels in a dirty cube, then plots the results.
    """
    runFindContinuum = True
    cont_freq_ranges, png = plotMeanSpectrum(img, transition, baselineModeA, baselineModeB, sigmaCube, nBaselineChannels,
                                             sigmaFindContinuum, replaceNans, verbose, png, pngBasename,
                                             nanBufferChannels, source, runFindContinuum, useAbsoluteValue, 
                                             trimChannels,percentile,continuumThreshold)
    return(cont_freq_ranges, png)

def plotMeanSpectrum(img='g35.03_KDnh3_11.hline.self.image', 
                     transition='(1,1)', baselineModeA='edge', baselineModeB='min', sigmaCube=3,
                     nBaselineChannels = 0.19, sigmaFindContinuum=-1,
                     replaceNans=True, verbose=False, png='', 
                     pngBasename=False, nanBufferChannels=2, source='',
                     findContinuum=True, useAbsoluteValue=True, trimChannels=0.1, 
                     percentile=20, continuumThreshold=None):
    """
    Computes, then plots the mean spectrum of a cube, above a threshold
    level specified by the rms in the edge channels.  Indicate the channel
    ranges to be used as continuum (if findContinuum==True).

    Inputs:
    nBaselineChannels: if integer, then the number of channels to use
          if float, then the fraction of channels to use (i.e. the percentile)
          default = 0.19, which is 24 channels (i.e. 12 on each side) of a TDM window
    sigmaCube: passed to meanSpectrum 
    sigmaFindContinuum: passed to findContinuumChannels
    replacedNans: if True, the replace NaNs with minimum value,
                  if False, remove the NaNs
    png: the name of the png to produce ('' yields default name)
    pngBasename: if True, then remove the directory from img name
    source: the name of the source, to be shown in the title of the spectrum.
            if None, then use the filename, up to the first underscore.
    nanBufferChannels: when removing or replacing NaNs, do this many extra channels
                       beyond their extent
    findContinuum: find the continuum channels
    trimChannels: after doing best job of finding continuum, remove this many 
         channels from each edge of each block of channels found (for margin of safety)
         if it is a float between 0..1, then trim this fraction of channels
    percentile: used with baselineMode='min'
    continuumThreshold: if specified, only use pixels above this intensity level
    """
    startTime = timeUtilities.time()
    if (type(nBaselineChannels) == float):
        nchan = numberOfChannelsInCube(img)
        nBaselineChannels = int(round(nBaselineChannels*nchan))
    if (nBaselineChannels < 2):
        print "You must have at least 2 edge channels"
        return

    avgspectrum, avgSpectrumNansRemoved, avgSpectrumNansReplaced, threshold,\
        edgesUsed, nchan, nanmin = meanSpectrum(img, nBaselineChannels, sigmaCube, verbose,
                                                nanBufferChannels,useAbsoluteValue,
                                                baselineModeA, percentile,
                                                continuumThreshold)
    donetime = timeUtilities.time()
    print "%.1f sec elapsed in meanSpectrum" % (donetime-startTime)
    cont_freq_ranges = []
    if (findContinuum):
        continuumChannels,selection,threshold,median,sigmaFindContinuum = findContinuumChannels(avgSpectrumNansReplaced,
                                                         nBaselineChannels, sigmaFindContinuum, nanmin, 
                                                         baselineModeB, trimChannels)
        print 'Continuum frequency ranges:'
        myia = createCasaTool(iatool)
        myqa = createCasaTool(qatool)
        myia.open(img)
        for crange in selection.split(','):
            c0, c1 = crange.split('~')
            m0 = myia.coordmeasures([None, None, None, c0])['measure']['spectral']['frequency']['m0']
            m1 = myia.coordmeasures([None, None, None, c1])['measure']['spectral']['frequency']['m0']
            f0 = myqa.convert('%.15e %s' % (m0['value'], m0['unit']), 'GHz')
            f1 = myqa.convert('%.15e %s' % (m1['value'], m1['unit']), 'GHz')
            if (qa.lt(f0, f1)):
                print '%.9f~%.9fGHz' % (f0['value'], f1['value'])
                cont_freq_ranges.append((f0['value'], f1['value']))
            else:
                print '%.9f~%.9fGHz' % (f1['value'], f0['value'])
                cont_freq_ranges.append((f1['value'], f0['value']))
        myia.close()

    pl.clf()
    skipchan = 2
    if replaceNans:
        avgspectrumAboveThreshold = avgSpectrumNansReplaced
    else:
        avgspectrumAboveThreshold = avgSpectrumNansRemoved
    if (edgesUsed == 2):
#        print "Showing all %d channels" % (len(avgspectrumAboveThreshold))
        pl.plot(range(len(avgspectrum)), avgspectrum, 'k-',
                range(len(avgspectrumAboveThreshold)), 
                avgspectrumAboveThreshold, 'r-')
    elif (edgesUsed == 0):
        # The upper edge is not used and can have an upward spike
        # so don't show it.
        print "Not showing final %d channels of %d" % (skipchan,\
               len(avgspectrumAboveThreshold))
        pl.plot(range(len(avgspectrum)), avgspectrum, 'k-',
                range(len(avgspectrumAboveThreshold)-skipchan), 
                avgspectrumAboveThreshold[:-skipchan], 'r-')
    elif (edgesUsed == 1):
        # The lower edge channels are not used and the threshold mean can 
        # have an upward spike, so don't show the first channel inward from 
        # there.
        print "Not showing first %d channels of %d" % (skipchan,\
                                   len(avgspectrumAboveThreshold))
        pl.plot(range(len(avgspectrum)), avgspectrum, 'k-',
                range(skipchan,len(avgspectrumAboveThreshold)), 
                avgspectrumAboveThreshold[skipchan:], 'r-')
        
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
            pl.plot(pl.xlim(), [median,median], 'k--')
            if (baselineModeB == 'edge'):
                pl.plot([nEdgeChannels, nEdgeChannels], pl.ylim(), 'k:')
                pl.plot([nchan-nEdgeChannels, nchan-nEdgeChannels], pl.ylim(), 'k:')
            channelSelections = selection.split(',')
            for i,ccstring in enumerate(channelSelections): 
                cc = [int(j) for j in ccstring.split('~')]
                pl.plot(cc, np.ones(len(cc))*np.mean(avgspectrumAboveThreshold), 'c-', lw=3)
                yoffset = np.mean(avgspectrumAboveThreshold)-0.02*(pl.ylim()[1]-pl.ylim()[0])
#                print "i=%d, " % (i), cc
                pl.text(np.mean(cc), yoffset, ccstring, va='top', ha='center',size=8)

    if (source==None):
        source = os.path.basename(img).split('_')[0]
    pl.title(source.upper() + ' ' + transition + ' baselineMode=(%s,%s), sigma=%.1f'%(baselineModeA,baselineModeB,sigmaFindContinuum))
    # Set channel 0 as left edge
    xlim = list(pl.xlim())
    xlim[0] = 0
    pl.xlim(xlim)
    pl.plot(xlim, [threshold,threshold],'k:')
    pl.xlabel('Channel')
    pl.draw()
    if (png == ''):
        if pngBasename:
            png = os.path.basename(img)
        else:
            png = img
        transition = transition.replace('(','_').replace(')','_').replace(' ','_').replace(',','')
        if (transition==''):
            transition = 'none'
        png += '.meanSpectrum.%s.%s.%.1fsigma.%s.png'%(baselineModeA, baselineModeB, sigmaFindContinuum, transition)
    pl.savefig(png)
    donetime = timeUtilities.time()
    print "%.1f sec elapsed in plotMeanSpectrum" % (donetime-startTime)
    return(cont_freq_ranges, png)

def findContinuumChannels(spectrum, nBaselineChannels=16, sigmaFindContinuum=-1, 
                          nanmin=None, baselineMode='min', trimChannels=1,
                          threshold=None, minFraction=0.05): 
    """
    Trys to find continuum channels in a spectrum, based on a threshold or
    some number of edge channels and their median and standard deviation.
    * spectrum: a one-dimensional array of intensity values
    * threshold: select channels below this value; if None then use sigma & nBaselineChannels:
    * nBaselineChannels: number of over which to compute standard deviation
    * sigmaFindContinuum: value to multiply the standard deviation by to get threshold
          -1 or 'auto': use automatic determination based on number of channels
    * minFraction: the minimum fraction of channels in a contiguous block to accept as a block
                   compared to all channels within identified blocks
    * nanmin: the value that NaNs were replaced by in previous steps
    * baselineMode: 'min' or 'edge',  'edge' will use nBaselineChannels/2 from each edge
    Returns:
    * the list of channels to use
    * the list converted to ms channel selection syntax
    * the threshold used
    * the median used in setting the threshold
    """
    if (sigmaFindContinuum < 0 or sigmaFindContinuum=='auto'):
        percentile = 100.0*nBaselineChannels/len(spectrum)
        sigmaFindContinuum = pickSigma(baselineMode, len(spectrum), percentile)
    print "Using sigma=%.1f for mode=%s, channels=%d/%d" % (sigmaFindContinuum, baselineMode, nBaselineChannels, len(spectrum))
    if (threshold == None):
        print "findContinuumChannels: input threshold=None"
        if (baselineMode == 'edge'):
            # pick n channels on both edges
            lowerChannels = spectrum[:nBaselineChannels/2]
            upperChannels = spectrum[-nBaselineChannels/2:]
            allChannels = list(lowerChannels) + list(upperChannels)
            if (np.std(lowerChannels) == 0):
                rms = MAD(upperChannels)
                median = np.median(upperChannels)
                print "edge method: Dropping lower channels from median and std calculations"
            elif (np.std(upperChannels) == 0):
                rms = MAD(lowerChannels)
                median = np.median(lowerChannels)
                print "edge method: Dropping upper channels from median and std calculations"
            else:
                rms = MAD(allChannels)
                median = np.median(allChannels)
        else:
            # pick the n channels with the n lowest values
            allChannels = spectrum[np.argsort(spectrum)[:nBaselineChannels]] 
            rms = MAD(allChannels)
            if (rms < 1e-17):  # avoid blocks of identically-zero values
                myspectrum = spectrum[np.where(spectrum != allChannels[0])]
                allChannels = myspectrum[np.argsort(myspectrum)[:nBaselineChannels]] 
                rms = MAD(allChannels)
            rms = MAD(allChannels)
            median = np.median(allChannels)
            print "min method: median intensity used as the baseline: %f, " % (median)#, allChannels
        threshold = sigmaFindContinuum*rms + median
        negativeThreshold = -sigmaFindContinuum*rms + median
        print "findContinuumChannels: computed threshold = ", threshold
    else:
        negativeThreshold = None
        print "findContinuumChannels: input threshold=%f" % (threshold)

    channels = np.where(spectrum < threshold)[0]
    if (negativeThreshold != None):
        channels2 = np.where(spectrum > negativeThreshold)[0]
        channels = np.intersect1d(channels,channels2)

    selection = convertChannelListIntoSelection(channels)
    print "Found %d potential continuum channels: %s" % (len(channels), str(selection))
    if (len(channels) == 0):
        selection = ''
    else:
        channels = splitListIntoContiguousListsAndRejectZeroStd(channels, spectrum, nanmin)
        selection = convertChannelListIntoSelection(channels)
        print "Found %d channels after rejecting zero std: %s" % (len(channels), str(selection))
        if (len(channels) == 0):
            selection = ''
        else:
            channels = splitListIntoContiguousListsAndTrim(channels, trimChannels)
            selection = convertChannelListIntoSelection(channels)
            trialChannels = splitListIntoContiguousListsAndRejectNarrow(channels,fraction=minFraction)
            if (len(trialChannels) > 0):
                channels = trialChannels
                print "Found %d channels after rejecting narrow groups" % (len(channels))
                selection = convertChannelListIntoSelection(channels)
    groups = len(channels)
    print "Found %d continuum channels in %d groups: %s" % (len(channels), groups, selection)
    return(channels, selection, threshold, median, sigmaFindContinuum)

def splitListIntoContiguousListsAndRejectNarrow(channels, fraction=0.05):
    """
    Split a list of channels into contiguous lists, and reject those that
    have a small number of channels relative to the total channels in 
    all lists.
    Returns: a new single list
    """
    length = len(channels)
    mylists = splitListIntoContiguousLists(channels)
    channels = []
    for mylist in mylists:
        if (len(mylist) <= fraction*length):
            continue
        channels += mylist
    return(np.array(channels))

def splitListIntoContiguousListsAndTrim(channels, trimChannels=0.1):
    """
    Split a list of channels into contiguous lists, and trim some number
    of channels from each edge.
    trimChannels: if integer, use that number of channels.  If float between
        0 and 1, then use that fraction of channels in each contiguous list
    Returns: a new single list
    """
    if (trimChannels <= 0):
        return(np.array(channels))
    length = len(channels)
    mylists = splitListIntoContiguousLists(channels)
    channels = []
    trimChan = trimChannels
    for mylist in mylists:
        if (trimChannels < 1):
            trimChan = int(np.ceil(len(mylist)*trimChannels))
        if (len(mylist) < 1+trimChan*2):
            continue
        channels += mylist[trimChan:-trimChan]
    return(np.array(channels))

def pickSigma(baselineMode, npts, percentile=20):
    """
    Picks an appropriate sigma value to use for findContinuumChannels
    based on the expected behavior of Gaussian statistics.
    """
    edgeValue = 3.0*(npts/64.)**0.08
    if (baselineMode == 'edge'):
        return(edgeValue)
    elif (baselineMode == 'min'):
        # First term corrects for the fact that the measured MAD on the lowest points
        # will be less than the true MAD of all points.
        # The second term computes the effect due to the fact that the median of the
        # lowest points will be lower than the true median of all points.
        return(edgeValue*2.5*(percentile/10.)**-0.25 + 6.3*(5.0/percentile)**0.5)
    else:
        print "Unrecognized baselineMode = ", baselineMode
        return(3)

def numberOfChannelsInCube(img, returnFreqs=False, verbose=False):
    """
    Finds the number of channels in a CASA image cube.
    returnfreqs: if True, then also return the frequency of the
           first and last channel (in Hz)
    verbose: if True, then print the frequencies of first and last channel
    -Todd Hunter
    """
    header = imhead(img,mode='list')
    if (header == None):
        print "imhead failed -- this may not be a CASA image cube."
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
    elif (header['ctype4'] == 'Frequency'):
        crpix = header['crpix4']
        crval = header['crval4']
        cdelt = header['cdelt4']
        firstFreq = crval + (0-crpix)*cdelt
        lastFreq = crval + (nchan-1-crpix)*cdelt
        if (verbose):
            print "Channel  0 = %.0f Hz" % (firstFreq)
            print "Channel %d = %.0f Hz" % (nchan-1,lastFreq)
    else:
        print "The fourth axis is not frequency."
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
            pixels = np.median(pixels, axis=0)
        else:
            if (threshold != None):
                pixels[np.where(pixels < threshold)] = np.nan
            pixels = nanmean(pixels, axis=0)
    return(pixels)

def meanSpectrum(img='g35.03_KDnh3_11.hline.self.image', nBaselineChannels=16,
                 sigmaCube=3, verbose=False, nanBufferChannels=2, useAbsoluteValue=False,
                 baselineMode='edge', percentile=20, continuumThreshold=None):
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
    Returns 6 items:
       * avgspectrum (vector)
       * avgspectrumAboveThresholdNansRemoved (vector)
       * avgspectrumAboveThresholdNansReplaced (vector)
       * threshold (scalar) 
       * edgesUsed: 0=lower, 1=upper, 2=both
       * nchan

    """
    if (not os.path.exists(img)):
        print "Could not find image = ", img
        return
    axis = 3 # findSpectralAxis(img)
    if verbose: print "spectral axis = ", axis
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
    region = rg.box(blc=blc, trc=trc)
    lowerEdgePixels = myia.getregion(region=region)
    # drop all floating point zeros (which will drop pixels outside the mosaic image mask)
    lowerEdgePixels = lowerEdgePixels[np.where(lowerEdgePixels!=0.0)]
    stdLowerEdge = MAD(lowerEdgePixels)
    medianLowerEdge = np.median(lowerEdgePixels)
    if verbose: print "MAD of %d channels on lower edge = %f" % (nBaselineChannels, stdLowerEdge)

    # upper edge
    blc = np.zeros(naxes)
    trc = [i-1 for i in list(np.shape(pixels))]
    blc[3] = trc[3] - nEdgeChannels
    region = rg.box(blc=blc, trc=trc)
    upperEdgePixels = myia.getregion(region=region)
    # drop all floating point zeros
    upperEdgePixels = upperEdgePixels[np.where(upperEdgePixels!=0.0)]
    stdUpperEdge = MAD(upperEdgePixels)
    medianUpperEdge = np.median(upperEdgePixels)
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
    medianMin = np.median(allPixels[idx])
#    idx = np.argsort(absPixels)
#    stdMin = MAD(absPixels[idx][:npix])
#    medianMin = np.median(absPixels[idx][:npix])

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

    c = 0.6745 is the constant to convert from MAD to std; it is used by
    default

    """
    a = np.array(a)
    good = (a==a)
    a = np.asarray(a, np.float64)
    if a.ndim == 1:
        d = np.median(a[good])
        m = np.median(np.fabs(a[good] - d) / c)
#        print  "mad = %f" % (m)
    else:
        d = np.median(a[good], axis=axis)
        # I don't want the array to change so I have to copy it?
        if axis > 0:
            aswp = swapaxes(a[good],0,axis)
        else:
            aswp = a[good]
        m = np.median(np.fabs(aswp - d) / c, axis=0)

    return m

def splitListIntoContiguousLists(mylist):
    """
    Called by copyweights. See also splitListIntoHomogenousLists.
    Converts [1,2,3,5,6,7] into [[1,2,3],[5,6,7]], etc.
    -Todd Hunter
    """
    mylists = []
    newlist = [mylist[0]]
    for i in range(1,len(mylist)):
        if (mylist[i-1] != mylist[i]-1):
            mylists.append(newlist)
            newlist = [mylist[i]]
        else:
            newlist.append(mylist[i])
    mylists.append(newlist)
    return(mylists)
    
def splitListIntoContiguousListsAndRejectZeroStd(channels, values, nanmin=None):
    """
    Takes a list of numerical values, splits into contiguous lists and 
    removes those that have zero standard deviation in their associated values.
    Note that values holds the values of the whole array, while channels 
    can be a subset.
    If nanmin is specified, then reject lists that contain more than 3 
    appearances of this value.
    """
#    print "splitListIntoContiguousListsAndRejectZeroStd:  len(values)=%d, len(channels)=%d" % (len(values), len(channels))
    mylists = splitListIntoContiguousLists(channels)
    channels = []
    for i,mylist in enumerate(mylists):
        mystd = np.std(values[mylist])
        if (mystd > 1e-17):  # avoid blocks of identically-zero values
            if (nanmin != None):
                minvalues = len(np.where(values[mylist] == nanmin)[0])
                if (float(minvalues)/len(mylist) > 0.1 and minvalues > 3):
                    print "Rejecting list %d with multiple min values (%d)" % (i,minvalues)
                    continue
            channels += mylist
#            print "Not rejecting channels %d~%d as std=%.30f" % (mylist[0],mylist[-1],mystd)
#        else:
#            print "Rejecting a list with zero st.dev = %.30f" % (mystd)
#            print "        ", mylist
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
    
def convertChannelListIntoSelection(channels, trim=0):
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
            if (mylist[0]+trim < mylist[-1]-trim):
                if (not firstList):
                    selection += ','
                selection += '%d~%d' % (mylist[0]+trim, mylist[-1]-trim)
                firstList = False
    return(selection)

