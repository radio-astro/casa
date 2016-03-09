"""
Demonstration of algorithm to determine continuum channel ranges to 
use from a CASA image cube (dirty or clean).  All dependencies on 
analysisUtils functions have been pasted into this file for convenience.
This function is meant to be run inside CASA.  Simple usage:
  import findContinuum as fc
  fc.findContinuum('my_dirty_cube.image')
"""

import os
import pyfits
import numpy as np
import matplotlib.pyplot as pl
import matplotlib.ticker
import time as timeUtilities
import scipy
from scipy.stats import scoreatpercentile, percentileofscore
from taskinit import *
from imhead_cli import imhead_cli as imhead
import warnings

def version(showfile=True):
    """
    Returns the CVS revision number.
    """
    myversion = "$Id: findContinuum.py,v 1.46 2016/01/24 19:35:37 we Exp $" 
    if (showfile):
        print "Loaded from %s" % (__file__)
    return myversion

def is_binary(filename):
    """
    Return true if the given filename appears to be binary.
    File is considered to be binary if it contains a NULL byte.
    This approach returns True for .fits files, but
    incorrectly reports UTF-16 as binary.
    """
    with open(filename, 'rb') as f:
        for block in f:
            if '\0' in block:
                return True
    return False

def findContinuum(img='', spw='', transition='', baselineModeA='min', baselineModeB='min',
                  sigmaCube=3, nBaselineChannels=0.19, sigmaFindContinuum='auto',
                  verbose=False, png='', pngBasename=False, nanBufferChannels=2, 
                  source='', useAbsoluteValue=True, trimChannels='auto', 
                  percentile=20, continuumThreshold=None, narrow='auto', 
                  separator=';', overwrite=False, titleText='', 
                  showAverageSpectrum=False, maxTrim=20, maxTrimFraction=1.0,
                  meanSpectrumFile='', centralArcsec='auto', alternateDirectory='.',
                  header='', plotAtmosphere=True, airmass=1.5, pwv=1.0,
                  channelFractionForSlopeRemoval=0.75, invert=False):
    """
    This function calls functions to:
    1) compute the mean spectrum of a dirty cube
    2) find the continuum channels 
    3) plot the results

    Returns:
    * A channel selection string suitable for the spw parameter of clean.
    * The name of the png produced.
    * The aggregate bandwidth in continuum in GHz.

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
    sigmaFindContinuum: passed to findContinuumChannels, 'auto' starts with 3.5
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
    centralArcsec: radius of central box within which to compute the mean spectrum
           default='auto' means start with whole field, then reduce to 1/10 if only
           one window is found
    header: dictionary created by imhead(img, mode='list')
    airmass: for plot of atmospheric transmission
    pwv: in mm (for plot of atmospheric transmission)
    channelFractionForSlopeRemoval: if this many channels are initially selected, then fit 
         and remove a linear slope and re-identify continuum channels.
    invert: if reading previous meanSpectrum file, then invert the sign and add the minimum
    """
    img = img.rstrip('/')
    imageInfo = [] # information returned from getImageInfo
    if (img != ''):
        if (not os.path.exists(img)):
            print "Could not find image."
            return
        result = getImageInfo(img, header)
        if (result == None): return result
        imageInfo, header = result
        bmaj, bmin, bpa, cdelt1, cdelt2, naxis1, naxis2, freq = imageInfo
        chanInfo = numberOfChannelsInCube(img, returnChannelWidth=True, returnFreqs=True, header=header)
        nchan,firstFreq,lastFreq,channelWidth = chanInfo
        channelWidth = abs(channelWidth)
    else:
        header = []
        chanInfo = []
        channelWidth = 0
    if (centralArcsec == 'auto' and img != ''):
        print "Running findContinuum('%s', centralArcsec='%s')" % (img, centralArcsec)
        npixels = float(nchan)*naxis1*naxis2
        maxpixels = float(1024)*1024*960
        if (npixels > maxpixels):
            print "Excessive number of pixels (%.0f > %.0f)" % (npixels,maxpixels)
            totalWidthArcsec = abs(cdelt2*naxis2)
            centralArcsecField = totalWidthArcsec*maxpixels/npixels
            print "Reducing image width examined from %.2f to %.2f arcsec to avoid memory problems." % (totalWidthArcsec,centralArcsecField)
        else:
            centralArcsecField = -1  # use the whole field
    elif (centralArcsec == 'fwhm' and img != ''):
        print "Running findContinuum('%s', centralArcsec='%s')" % (img, centralArcsec)
        centralArcsecField = primaryBeamArcsec(frequency=getFitsBeam(image)[-1],
                                               showEquation=False)
    else:  
        print "Running findContinuum('%s', centralArcsec=%s)" % (img, str(centralArcsec))
        centralArcsecField = centralArcsec

    result = runFindContinuum(img, spw, transition, baselineModeA, baselineModeB,
                              sigmaCube, nBaselineChannels, sigmaFindContinuum,
                              verbose, png, pngBasename, nanBufferChannels, 
                              source, useAbsoluteValue, trimChannels, 
                              percentile, continuumThreshold, narrow, 
                              separator, overwrite, titleText, 
                              showAverageSpectrum, maxTrim, maxTrimFraction,
                              meanSpectrumFile, centralArcsecField, channelWidth,
                              alternateDirectory, imageInfo, chanInfo, header,
                              plotAtmosphere, airmass, pwv, 
                              channelFractionForSlopeRemoval, invert, iteration=0)
    if result == None:
        return
    selection, png, slope = result
    if (centralArcsec == 'auto' and img != '' and len(selection.split(separator)) < 2):
        # reduce the field size to one tenth of the previous
        bmaj, bmin, bpa, cdelt1, cdelt2, naxis1, naxis2, freq = imageInfo # getImageInfo(img)
        imageWidthArcsec = 0.5*(np.abs(naxis2*cdelt2) + np.abs(naxis1*cdelt1))
        centralArcsec = 0.1*imageWidthArcsec
        overwrite = True
        print "Re-running findContinuum over central %.1f arcsec" % (centralArcsec)
        result = runFindContinuum(img, spw, transition, baselineModeA, baselineModeB,
                                  sigmaCube, nBaselineChannels, sigmaFindContinuum,
                                  verbose, png, pngBasename, nanBufferChannels, 
                                  source, useAbsoluteValue, trimChannels, 
                                  percentile, continuumThreshold, narrow, 
                                  separator, overwrite, titleText, 
                                  showAverageSpectrum, maxTrim, maxTrimFraction,
                                  meanSpectrumFile, centralArcsec, channelWidth,
                                  alternateDirectory, imageInfo, chanInfo, header,
                                  plotAtmosphere, airmass, pwv, 
                                  channelFractionForSlopeRemoval, invert, iteration=1)
        if result == None:
            return
        selection, png, slope = result
    aggregateBandwidth = computeBandwidth(selection, channelWidth)
    # Write summary of results to text file
    if (meanSpectrumFile == ''): meanSpectrumFile = img + '.meanSpectrum'
    writeContDat(meanSpectrumFile, selection, png, aggregateBandwidth)
    return(selection, png, aggregateBandwidth)

def writeContDat(meanSpectrumFile, selection, png, aggregateBandwidth):
    if (meanSpectrumFile.find('.meanSpectrum') > 0):
        contDat = meanSpectrumFile.replace('.meanSpectrum','_findContinuum.dat')
    else:
        contDat = meanSpectrumFile + '_findContinuum.dat'
    contDatDir = os.path.dirname(contDat)
    if (len(contDatDir) < 1):
        contDatDir = '.'
    if (not os.access(contDatDir, os.W_OK) and contDatDir != '.'):
        # Tf there is no write permission, then use the directory of the png
        # since this has been established as writeable in runFindContinuum.
        contDat = os.path.dirname(png) + '/' + os.path.basename(contDat)
    try:
        f = open(contDat, 'w')
        f.write('%s %s %g\n' % (selection, png, aggregateBandwidth))
        f.close()
        print "Wrote ", contDat
    except:
        print "Failed to write ", contDat

def computeBandwidth(selection, channelWidth):
    """
    selection: a string of format:  '5~6;9~20'
    channelWidth: in Hz
    Returns: bandwidth in GHz
    """
    ranges = selection.split(';')
    channels = 0
    for r in ranges:
        result = r.split('~')
        if (len(result) == 2):
            a,b = result
            channels += int(b)-int(a)+1
    return(channels * abs(channelWidth) * 1e-9)

def runFindContinuum(img='', spw='', transition='', baselineModeA='min', baselineModeB='min',
                     sigmaCube=3, nBaselineChannels=0.19, sigmaFindContinuum='auto',
                     verbose=False, png='', pngBasename=False, nanBufferChannels=2, 
                     source='', useAbsoluteValue=True, trimChannels='auto', 
                     percentile=20, continuumThreshold=None, narrow='auto', 
                     separator=';', overwrite=False, titleText='', 
                     showAverageSpectrum=False, maxTrim=20, maxTrimFraction=1.0,
                     meanSpectrumFile='', centralArcsec=-1, channelWidth=0,
                     alternateDirectory='.', imageInfo=[], chanInfo=[], 
                     header='', plotAtmosphere=True, airmass=1.5, pwv=1.0,
                     channelFractionForSlopeRemoval=0.8, invert=False, iteration=0):
    """
    This function calls functions to:
    1) compute the mean spectrum of a dirty cube
    2) find the continuum channels 
    3) plot the results
    Inputs: channelWidth: in Hz
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
    sigmaFindContinuum: passed to findContinuumChannels, 'auto' starts with 3.5
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
    centralArcsec: radius of central box within which to compute the mean spectrum
               default='auto' means start with whole field, then reduce to 10 arcsec if only
               one window is found
    airmass: for plot of atmospheric transmission
    pwv: in mm (for plot of atmospheric transmission)
    channelFractionForSlopeRemoval: if this many channels are initially selected, then fit 
         and remove a linear slope and re-identify continuum channels.
    """
    slope=None 
    replaceNans = True 
    startTime = timeUtilities.time()
    img = img.rstrip('/')
    fitsTable = False
    if (meanSpectrumFile != '' and os.path.exists(meanSpectrumFile)):
        print "Using existing meanSpectrumFile = ", meanSpectrumFile
        if (is_binary(meanSpectrumFile)):
            fitsTable = True
    if (type(nBaselineChannels) == float and not fitsTable):
        nchan, firstFreq, lastFreq, channelWidth = chanInfo # numberOfChannelsInCube(img, returnFreqs=True)
        channelWidth = abs(channelWidth)
        nBaselineChannels = int(round(nBaselineChannels*nchan))
        print "Found %d channels in the cube" % (nchan)
    if (nBaselineChannels < 2 and not fitsTable):
        print "You must have at least 2 edge channels"
        return
    if (meanSpectrumFile == ''):
        meanSpectrumFile = img + '.meanSpectrum'
    elif (not os.path.exists(meanSpectrumFile)):
        if (len(os.path.dirname(img)) > 0):
            meanSpectrumFile = os.path.dirname(img) + '/' + os.path.basename(meanSpectrumFile)

    if (overwrite or not os.path.exists(meanSpectrumFile)):
        if (not os.path.exists(meanSpectrumFile)):
            print "Did not find mean spectrum file = ", meanSpectrumFile
        print "Regenerating the mean spectrum file."
        avgspectrum, avgSpectrumNansRemoved, avgSpectrumNansReplaced, meanSpectrumThreshold,\
          edgesUsed, nchan, nanmin = meanSpectrum(img, nBaselineChannels, sigmaCube, verbose,
                                                  nanBufferChannels,useAbsoluteValue,
                                                  baselineModeA, percentile,
                                                  continuumThreshold, meanSpectrumFile, 
                                                  centralArcsec, imageInfo, chanInfo)
        if verbose:
            print "len(avgspectrum) = %d, len(avgSpectrumNansReplaced)=%d" % (len(avgspectrum),len(avgSpectrumNansReplaced))
    else:
        if (fitsTable):
            result = readMeanSpectrumFITSFile(meanSpectrumFile)
            if (result == None):
                print "FITS table is not valid."
                return
            chanInfo = [result[4],result[6],result[7],(result[7]-result[6])/(result[4]-1)]
        else:
            print "Running readPreviousMeanSpectrum('%s')" % (meanSpectrumFile)
            result = readPreviousMeanSpectrum(meanSpectrumFile, verbose, invert)
            if (result == None):
                print "ASCII file is not valid, re-run with overwrite=True"
                return
        avgspectrum, avgSpectrumNansReplaced, meanSpectrumThreshold, edgesUsed, nchan, nanmin, firstFreq, lastFreq = result
        if verbose:
            print "len(avgspectrum) = ", len(avgspectrum)
        if (len(avgspectrum) < 2):
            print "ASCII file is too short, re-run with overwrite=True"
            return
        if (firstFreq == 0 and lastFreq == 0):
            # This was an old-format ASCII file, without a frequency column
            n, firstFreq, lastFreq, channelWidth = chanInfo # numberOfChannelsInCube(img,returnFreqs=True)
            channelWidth = abs(channelWidth)
        if (fitsTable):
            nBaselineChannels = int(round(nBaselineChannels*nchan))
            img = meanSpectrumFile
            if (nBaselineChannels < 2):
                print "You must have at least 2 edge channels"
                return
            
    donetime = timeUtilities.time()
    if verbose:
        print "%.1f sec elapsed in meanSpectrum" % (donetime-startTime)
    print "Iteration %d" % (iteration)
    if (sigmaFindContinuum == 'auto' or sigmaFindContinuum == -1):
        sigmaFindContinuumAutomatic = True
        sigmaFindContinuum = 3.5
    else:
        sigmaFindContinuumAutomatic = False
    continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC = \
        findContinuumChannels(avgSpectrumNansReplaced, nBaselineChannels, sigmaFindContinuum, nanmin, 
                              baselineModeB, trimChannels, narrow, verbose, maxTrim, maxTrimFraction, separator)
    sumAboveMedian, sumBelowMedian, sumRatio, channelsAboveMedian, channelsBelowMedian, channelRatio = \
        aboveBelow(avgSpectrumNansReplaced,medianTrue)
    # First, one group must have at least 2 channels (to insure it is real), otherwise raise the sigmaFC.
    # Otherwise, if there are a lot of groups or a lot of channels above the median compared to below it,
    # then lower the sigma in order to push the threshold for real lines (or line emission wings) lower.
    # However, if there is only 1 group, then there may be no real lines present, so lowering 
    # the threshold in this case can create needless extra groups, so don't allow it.
    if (singleChannelPeaksAboveSFC == allGroupsAboveSFC and allGroupsAboveSFC>1):
        # raise the threshold a bit since all the peaks look like all noise
        factor = 1.5
        sigmaFindContinuum *= factor
        print "Scaling the threshold upward by a factor of %.2f to avoid apparent noise spikes (%d==%d)." % (factor, singleChannelPeaksAboveSFC,allGroupsAboveSFC)
        continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC = \
            findContinuumChannels(avgSpectrumNansReplaced, nBaselineChannels, sigmaFindContinuum, nanmin, 
                                  baselineModeB, trimChannels, narrow, verbose, maxTrim, maxTrimFraction, separator)
        sumAboveMedian, sumBelowMedian, sumRatio, channelsAboveMedian, channelsBelowMedian, channelRatio = \
            aboveBelow(avgSpectrumNansReplaced,medianTrue)
    elif ((groups > 3 or (groups > 1 and channelRatio < 1.0) or (channelRatio < 0.5) or (groups == 2 and channelRatio < 1.3)) and sigmaFindContinuumAutomatic):
#        print "A: groups,channelRatio=", groups, channelRatio, channelRatio < 1.0, channelRatio>0.1, channelWidth<0.5*15.624e6, groups>2
        if (channelRatio < 0.9 and channelRatio > 0.1 and (firstFreq>60e9 and channelWidth<0.5*15.625e6) and groups>2):  # was nchan>256
            # Don't allow this much reduction in ALMA TDM mode as it chops up line-free quasar spectra too much.
            # The channelRatio>0.1 requirement prevents failures due to ALMA TFB platforming.
            factor = 0.333
        elif (groups <= 2):
            if (channelRatio < 1.3 and channelRatio > 0.1 and groups == 2 and 
                channelWidth<15.625e6 and channelWidth>=1875e6/480.):
                if (channelWidth<0.5*15.625e6):
                    factor = 0.5  # i.e. for galaxy spectra with FDM 240 channel (online-averaging) resolution
                else:
                    factor = 0.7
            else:
                # prevent sigmaFindContinuum going to inf if groups==1
                # prevent sigmaFindContinuum going > 1 if groups==2
                factor = 0.9
        else:
            if (channelWidth>=0.5*15.625e6):
                factor = 1.0
            else:
                factor = np.log(3)/np.log(groups)
        print "setting factor to %f because groups=%d, channelRatio=%g, firstFreq=%g, nchan=%d, channelWidth=%e" % (factor,groups,channelRatio,firstFreq,nchan,channelWidth)
        print "Scaling the threshold by a factor of %.2f (groups=%d, channelRatio=%f)" % (factor, groups,channelRatio)
        sigmaFindContinuum *= factor
        continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC = \
            findContinuumChannels(avgSpectrumNansReplaced, nBaselineChannels, sigmaFindContinuum, nanmin, 
                                  baselineModeB, trimChannels, narrow, verbose, maxTrim, maxTrimFraction, separator)
        sumAboveMedian, sumBelowMedian, sumRatio, channelsAboveMedian, channelsBelowMedian, channelRatio = \
            aboveBelow(avgSpectrumNansReplaced,medianTrue)
    else:
        print "Not adjusting sigmaFindContinuum, because groups=%d, channelRatio=%g, firstFreq=%g, nchan=%d" % (groups,channelRatio,firstFreq,nchan)

    selectedChannels = countChannels(selection)
    largestGroup = channelsInLargestGroup(selection)
    selections = len(selection.split(separator))
    if (selectedChannels > channelFractionForSlopeRemoval*nchan or 
        (largestGroup>nchan/3 and selections <= 2)):
        previousResult = continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC        
        # remove linear slope from mean spectrum and run it again
        index = channelSelectionRangesToIndexArray(selection)
        print "Fitting slope to %d channels (largestGroup=%d,nchan/3=%d,selectedChannels=%d)" % (len(index), largestGroup, nchan/3, selectedChannels)
        slope, intercept = linfit(index, avgSpectrumNansReplaced[index], MAD(avgSpectrumNansReplaced[index]))
        avgSpectrumNansReplaced -= np.array(range(len(avgSpectrumNansReplaced)))*slope
        print "Removing slope = ", slope
        continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC = \
        findContinuumChannels(avgSpectrumNansReplaced, nBaselineChannels, sigmaFindContinuum, nanmin, 
                              baselineModeB, trimChannels, narrow, verbose, maxTrim, maxTrimFraction, 
                              separator, slope)
        # If we had only one group and only added one or two more group after removing slope, and the
        # smallest is small compared to the original group, then discard the new solution.
        discardSlopeResult = False
        if (groups <= 3 and previousResult[4] == 1):
            counts = countChannelsInRanges(selection)
            if (float(min(counts))/max(counts) < 0.2):
                print "*** Restoring result prior to linfit ***"
                discardSlopeResult = True
                continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC = previousResult
    else:
        print "selected channels (%d) < 0.8 * nchan(%d)" % (selectedChannels,nchan)
        
    idx = np.where(avgSpectrumNansReplaced < threshold)
    madOfPointsBelowThreshold = MAD(avgSpectrumNansReplaced[idx])
    pl.clf()
    if (showAverageSpectrum):
        rows = 2
    else:
        rows = 1
    cols = 1
    fontsize = 10
    ax1 = pl.subplot(rows,cols,1)
    skipchan = 1 # was 2
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
        pl.ylabel('Average spectrum > threshold=(%g)' % (roundFigures(meanSpectrumThreshold,3)), size=fontsize)
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
        pl.ylabel('Average spectrum above threshold=(%g)' % (roundFigures(meanSpectrumThreshold,3)), size=fontsize)
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
        pl.ylabel('Average spectrum above threshold=(%g)' % roundFigures(meanSpectrumThreshold,3), size=fontsize)
        
    pl.hold(True)
    if (baselineModeA == 'edge'):
        nEdgeChannels = nBaselineChannels/2
        if (edgesUsed == 0 or edgesUsed == 2):
            pl.plot(range(nEdgeChannels), avgspectrum[:nEdgeChannels], 'm-',lw=3)
        if (edgesUsed == 1 or edgesUsed == 2):
            pl.plot(range(nchan-nEdgeChannels,nchan), avgspectrum[-nEdgeChannels:],
                    'm-', lw=3)
    channelSelections = []
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
            pl.plot(cc, np.ones(len(cc))*np.mean(avgspectrumAboveThreshold), 'c-', lw=2)
            yoffset = np.mean(avgspectrumAboveThreshold)+0.04*(pl.ylim()[1]-pl.ylim()[0])
            pl.text(np.mean(cc), yoffset, ccstring, va='bottom', ha='center',size=8,rotation=90)

    if (source==None):
        source = os.path.basename(img)
        if (not fitsTable):
            source = source.split('_')[0]
    if (titleText == ''):
        narrowString = pickNarrowString(narrow, len(avgSpectrumNansReplaced)) 
        trimString = pickTrimString(trimChannels, len(avgSpectrumNansReplaced), maxTrim)
        titleText = os.path.basename(img) + ' ' + transition + ' baseline=(%s,%s), narrow=%s, sigmaFindCont.=%.1f, trim=%s' % (baselineModeA,baselineModeB,narrowString,sigmaFindContinuum,trimString)
    ylim = pl.ylim()
    ylim = [ylim[0], ylim[1]+(ylim[1]-ylim[0])*0.1]
    xlim = [0,nchan-1]
    pl.xlim(xlim)
    titlesize = np.min([fontsize,int(np.floor(fontsize*100.0/len(titleText)))])
    if (spw != ''):
        label = '(Spw %s) Channel' % (str(spw))
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
    else:
        ax1.set_xlabel(label, size=fontsize)
        pl.text(0.5, 1.08, titleText, size=titlesize, ha='center', transform=ax1.transAxes)
    pl.ylim(ylim)
    ax2 = ax1.twiny()
    pl.setp(ax1.get_xticklabels(), fontsize=fontsize)
    pl.setp(ax1.get_yticklabels(), fontsize=fontsize)
    pl.setp(ax2.get_xticklabels(), fontsize=fontsize)
    ax2.set_xlim(firstFreq*1e-9,lastFreq*1e-9)
    freqRange = np.abs(lastFreq-firstFreq)
    power = int(np.log10(freqRange))-9
    ax2.xaxis.set_major_locator(matplotlib.ticker.MultipleLocator(10**power))
    if (len(ax2.get_xticks()) < 2):
        ax2.xaxis.set_major_locator(matplotlib.ticker.MultipleLocator(0.5*10**power))
    ax2.xaxis.set_minor_locator(matplotlib.ticker.MultipleLocator(0.1*10**power))
    ax2.xaxis.set_major_formatter(matplotlib.ticker.ScalarFormatter(useOffset=False))
    if (channelWidth > 0):
        channelWidthString = ', channel width: %g kHz' % (channelWidth*1e-3)
    else:
        channelWidthString = ''
    freqType = ''
    if (type(header) == dict):
        if ('reffreqtype' in header.keys()):
            freqType = header['reffreqtype']
    if (spw != ''):
        label = '(Spw %s) %s Frequency (GHz)' % (str(spw),freqType) + channelWidthString
    else:
        label = '%s Frequency (GHz)' % freqType + channelWidthString
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
    pl.text(0.5,0.99-4*inc,'chans > median: %d (%.5f), chans < median: %d (%.5f), ratio: %.2f (%.2f)'%(channelsAboveMedian,sumAboveMedian,channelsBelowMedian,sumBelowMedian,channelRatio,sumRatio),
            transform=ax1.transAxes, ha='center', size=fontsize)
    areaString = 'maxTrimFraction=%g, %d ranges found, ' % (maxTrimFraction, len(channelSelections))
    if (centralArcsec == 'auto'):
        areaString += 'mean over area: (unknown)'
    elif (centralArcsec < 0):
        areaString += 'mean over area: whole field'
    else:
        areaString += 'mean over area: central box of radius %.1f arcsec' % (centralArcsec)
    pl.text(0.5,0.99-5*inc,areaString, transform=ax1.transAxes, ha='center', size=fontsize)
    if (slope != None):
        if discardSlopeResult:
            discarded = ' (result discarded)'
        else:
            discarded = ''
        pl.text(0.5, 0.99-6*inc, 'linear slope removed: %g %s' % (roundFigures(slope,3),discarded), transform=ax1.transAxes, ha='center', size=fontsize)
    # Write CVS version
    pl.text(1.03, -0.005-2*inc, ' '.join(version().split()[1:4]), size=8, 
            transform=ax1.transAxes, ha='right')
    if (plotAtmosphere):
        freqs, atm = CalcAtmTransmissionForImage(img, header, chanInfo, airmass, pwv)
        print "freqs: min=%g, max=%g n=%d" % (np.min(freqs),np.max(freqs), len(freqs))
#        atmRescaled = (ylim[1]-ylim[0])*0.25*atm + np.mean(ylim)
        atmRange = 0.5
        yrange = ylim[1]-ylim[0]
        atmRescaled = ylim[0] + 0.3*yrange + atm*atmRange*yrange
#        print "before plot: ylim=%.6f,%.6f, mean atm=%f " % (ylim[0], ylim[1], np.mean(atm))
        pl.text(1.015, 0.3, '0%', color='m', transform=ax1.transAxes, ha='left', va='center')
        pl.text(1.015, 0.8, '100%', color='m', transform=ax1.transAxes, ha='left', va='center')
        pl.text(1.03, 0.55, 'Atmospheric Transmission', color='m', ha='center', va='center', 
                rotation=90, transform=ax1.transAxes, size=11)
        pl.text(1.06, 0.55, '(%.1f mm PWV, 1.5 airmass)'%pwv, color='m', ha='center', va='center', 
                rotation=90, transform=ax1.transAxes, size=11)
        pl.plot(freqs, atmRescaled, 'w.', ms=0)  # need this to finalize the ylim value
        ticks = 10
        ylim = pl.ylim()
#        print "after plot: ylim=%.6f,%.6f, mean atm=%f " % (ylim[0], ylim[1], np.mean(atm))
        yrange = ylim[1]-ylim[0]
        atmRescaled = ylim[0] + 0.3*yrange + atm*atmRange*yrange
        pl.plot(freqs, atmRescaled, 'm-')
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
    pngdir = os.path.dirname(png)
    if (len(pngdir) < 1):
        pngdir = '.'
    if (not os.access(pngdir, os.W_OK) and pngdir != '.'):
        print "No permission to write to specified directory. Will try alternateDirectory."
        if (len(alternateDirectory) < 1):
            alternateDirectory = '.'
        png = alternateDirectory + '/' + os.path.basename(png)
        print "png = ", png
    pl.savefig(png)
    print "Wrote png = %s." % (png)
    donetime = timeUtilities.time()
    print "%.1f sec elapsed in plotMeanSpectrum" % (donetime-startTime)
    return(selection, png, slope)
# end of runFindContinuum

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

def readMeanSpectrumFITSFile(meanSpectrumFile, unit=0, nanBufferChannels=1):
    """
    Reads a spectrum from a FITS table, such as one output by spectralcube.
    """
    f = pyfits.open(meanSpectrumFile)
    tbheader = f[unit].header
    tbdata = f[unit].data
    nchan = len(tbdata)
    crpix = tbheader['CRPIX1']
    crval = tbheader['CRVAL1']
    cdelt = tbheader['CDELT1']
    units = tbheader['CUNIT1']
    if (units.lower() != ('hz')):
        print "Warning: frequency units are not Hz = ", units.lower()
        return
    firstFreq = crval - (crpix-1)*cdelt
    lastFreq = firstFreq + cdelt*(nchan-1)
    avgspectrum = tbdata
    edgesUsed = 2
    threshold = 0
    avgSpectrumNansReplaced,nanmin = removeNaNs(tbdata, replaceWithMin=True, 
                                     nanBufferChannels=nanBufferChannels)
    return(avgspectrum, avgSpectrumNansReplaced, threshold,
           edgesUsed, nchan, nanmin, firstFreq, lastFreq)
    
def readPreviousMeanSpectrum(meanSpectrumFile, verbose=False, invert=False):
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
               edgesUsed, len(intensity), np.nanmin(intensity), freq[0], freq[-1])
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
        if invert:
            a = -float(a)
            b = -float(b)
        avgspectrum.append(float(a))
        avgSpectrumNansReplaced.append(float(b))
    avgspectrum = np.array(avgspectrum)    
    avgSpectrumNansReplaced = np.array(avgSpectrumNansReplaced)
    if invert:
        avgspectrum += abs(np.min(avgspectrum))
        avgSpectrumNansReplaced += abs(np.min(avgSpectrumNansReplaced))
        
    if (len(freqs) > 0):
        firstFreq = freqs[0]
        lastFreq = freqs[-1]
    else:
        firstFreq = 0
        lastFreq = 0
    if verbose:
        print "Read previous mean spectrum with %d channels, (%d freqs: %f-%f)" % (len(avgspectrum),len(freqs),firstFreq,lastFreq)
    return(avgspectrum, avgSpectrumNansReplaced, threshold,
           edgesUsed, nchan, nanmin, firstFreq, lastFreq)

def findContinuumChannels(spectrum, nBaselineChannels=16, sigmaFindContinuum=3, 
                          nanmin=None, baselineMode='min', trimChannels='auto',
                          narrow='auto', verbose=False, maxTrim=20, 
                          maxTrimFraction=1.0, separator=';', slope=0.0): 
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
    originalSpectrum = spectrum + slope*np.array(range(len(spectrum)))
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
        useLowBaseline = True
    else:
        # pick the n channels with the n lowest values (or highest if those have smallest MAD)
        idx = np.argsort(spectrum)
        allBaselineChannels = spectrum[idx[:nBaselineChannels]] 
        allBaselineOriginalChannels = originalSpectrum[idx[:nBaselineChannels]]
        highestChannels = spectrum[idx[-nBaselineChannels:]] 
        if (MAD(allBaselineChannels) > MAD(highestChannels)):
            # This may be an absorption spectrum, so use highest values to define the continuum level
            print "Using highest %d channels as baseline" % (nBaselineChannels)
            allBaselineChannels = highestChannels[::-1] # reversed it so that first channel is highest value
            useLowBaseline = False
        else:
            print "Using lowest %d channels as baseline" % (nBaselineChannels)
            useLowBaseline = True

        print "min method: computing MAD and median of %d channels used as the baseline" % (len(allBaselineChannels))
        mad = MAD(allBaselineChannels)
        madOriginal = MAD(allBaselineOriginalChannels)
        print "MAD of all baseline channels = ", mad
        print "MAD of original baseline channels = ", mad
        if (mad < 1e-17 or madOriginal < 1e-17): 
            print "min method: avoiding blocks of identical-valued channels"
#            myspectrum = spectrum[np.where(spectrum != allBaselineChannels[0])]
            if (len(originalSpectrum) > 10):
                myspectrum = spectrum[np.where((originalSpectrum != originalSpectrum[0]) * (originalSpectrum != originalSpectrum[-1]))]
            else: # original logic, prior to linear fit removal
                myspectrum = spectrum[np.where(spectrum != allBaselineChannels[0])]
            allBaselineChannels = myspectrum[np.argsort(myspectrum)[:nBaselineChannels]] 
            print "            computing MAD and median of %d channels used as the baseline" % (len(allBaselineChannels)), allBaselineChannels
            mad = MAD(allBaselineChannels)
        mad = MAD(allBaselineChannels)
        median = nanmedian(allBaselineChannels)
        print "min method: median intensity of %d channels used as the baseline: %f, " % (len(allBaselineChannels), median)
    # will be 1.0 if no lines present and 0.25 if half the channels have lines, etc.
    signalRatio = (1.0 - 1.0*len(np.where(np.abs(spectrum-median)>(sigmaEffective*mad*2.0))[0]) / len(spectrum))**2
    lineStrengthFactor = 1.0/signalRatio
    medianTrue = medianCorrected(baselineMode, percentile, median, mad, signalRatio, useLowBaseline)
    threshold = sigmaEffective*mad + medianTrue
    # Use a 15% lower negative threshold to help prevent false identification of absorption features.
    negativeThreshold = -1.15*sigmaEffective*mad + medianTrue
    print "MAD = %f, median = %f, trueMedian=%f, signalRatio=%f" % (mad, median, medianTrue, signalRatio)
    print "findContinuumChannels: computed threshold = ", threshold
    channels = np.where(spectrum < threshold)[0]
    if (negativeThreshold != None):
        channels2 = np.where(spectrum > negativeThreshold)[0]
        channels = np.intersect1d(channels,channels2)

    # for CAS-8059: remove channels that are equal to the minimum if all channels from 
    # it toward the nearest edge are also equal to the minimum: 
    channels = list(channels)
    if (originalSpectrum[np.min(channels)] == np.min(originalSpectrum)):
        lastmin = np.min(channels)
        channels.remove(lastmin)
        removed = 1
        for c in range(np.min(channels),np.max(channels)):
            if (originalSpectrum[c] != np.min(originalSpectrum)):
                break
            channels.remove(c)
            removed += 1
        print "Removed %d channels on low channel edge that were at the minimum." % (removed)
    # Now come in from the upper side
    if (originalSpectrum[np.max(channels)] == np.min(originalSpectrum)):
        lastmin = np.max(channels)
        channels.remove(lastmin)
        removed = 1
        for c in range(np.max(channels),np.min(channels)-1,-1):
            if (originalSpectrum[c] != np.min(originalSpectrum)):
                break
            channels.remove(c)
            removed += 1
        print "Removed %d channels on high channel edge that were at the minimum." % (removed)
    peakChannels = np.where(spectrum > threshold)[0]
    peakChannelsLists = splitListIntoContiguousLists(peakChannels)
    peakMultiChannelsLists = splitListIntoContiguousListsAndRejectNarrow(peakChannels, narrow=2)
    allGroupsAboveSFC = len(peakChannelsLists)
    singleChannelPeaksAboveSFC = allGroupsAboveSFC - len(peakMultiChannelsLists)

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
                print "Calling splitListIntoContiguousListsAndTrim(channels=%s, trimChannels=%s, maxTrim=%d, maxTrimFraction=%f)" % (str(channels), str(trimChannels), maxTrim, maxTrimFraction)
            channels = splitListIntoContiguousListsAndTrim(channels, trimChannels, 
                                                 maxTrim, maxTrimFraction, verbose)
            if verbose:
                print "channels = ", channels
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
           negativeThreshold, lineStrengthFactor, singleChannelPeaksAboveSFC, allGroupsAboveSFC)

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
                                        maxTrimFraction=1.0, verbose=False):
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
    trimChannelsMode = trimChannels
    if (trimChannels == 'auto'):
        trimChannels = pickAutoTrimChannels(length, maxTrim)
    mylists = splitListIntoContiguousLists(channels)
    channels = []
    for mylist in mylists:
        trimChan = trimChannels
        if verbose:
            print "trimChan=%d, Checking list = " % (trimChan), mylist
        if (trimChannels < 1):
            trimChan = int(np.ceil(len(mylist)*trimChannels))
        if (trimChannelsMode == 'auto' and 1.0*trimChan/len(mylist) > maxTrimFraction):
            trimChan = int(np.floor(maxTrimFraction*len(mylist)))
        if verbose:
            print "trimChan for this window = %d" % (trimChan)
        if (len(mylist) < 1+trimChan*2):
            continue
        channels += mylist[trimChan:-trimChan]
    return(np.array(channels))

def roundFigures(value, digits):
    """
    This function rounds a floating point value to a number of significant figures.
    value: value to be rounded (between 1e-20 and 1e+20)
    digits: number of significant digits, both before or after decimal point
    """
    if (value != 0.0):
        if (np.log10(np.abs(value)) % 1 < np.log10(5)):
            digits -= 1
    for r in range(-20,20):
        if (round(value,r) != 0.0):
            value = round(value,r+digits)
            break
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
        trimString = 'auto_max=%g' % pickAutoTrimChannels(length, maxTrim)
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

def medianCorrected(baselineMode, percentile, median, mad, signalRatio, useLowBaseline):
    """
    Computes the true median of a datastream from the observed median and MAD of
    the lowest Nth percentile points.
    signalRatio: when there is a lot of signal, we need to reduce the correction 
                 factor because it is less like Gaussian noise
                 It is 1.0 if no lines present, 0.25 if half the channels have signal, etc.
    """
    if useLowBaseline:
        corrected = median + computeMedianCorrectionFactor(baselineMode, percentile)*mad*signalRatio
    else:
        corrected = median - computeMedianCorrectionFactor(baselineMode, percentile)*mad*signalRatio
    return(corrected)

def computeMedianCorrectionFactor(baselineMode, percentile):
    """
    Computes the effect due to the fact that taking the median of the
    N%ile lowest points of a Gaussian noise datastream will be lower than the true 
    median of all the points.  This is (TrueMedian-ObservedMedian)/ObservedMAD
    """
    if (baselineMode == 'edge'):
        return(0)
    return(6.3*(5.0/percentile)**0.5)

def headerToArcsec(mydict, unit=None):
    """
    Converts an angle quantity dictionary to the angle
    in arcsec.
    """
    if (unit == None):
        value = mydict['value']
    else:
        value = mydict
        mydict = {'value': mydict, 'unit': unit}
    if (mydict['unit'].find('rad') >= 0):
        value = 3600*np.degrees(value)
    elif (mydict['unit'].find('deg') >= 0):
        value *= 3600
    return(value)

def getImageInfo(img, header=''):
    """
    Extract the beam and pixel information from a CASA image.
    Returns: bmaj, bmin, bpa, cdelt1, cdelt2, naxis1, naxis2, frequency
             Angles are in arcseconds (bpa in degrees)
             Frequency is in GHz and is the central frequency
    -Todd Hunter
    """
    if (os.path.exists(img) == False):
        print "image not found: ", img
        return
    # Assume this is a CASA image
    if (header == ''):
        try:
            print "imhead",
            header = imhead(img, mode = 'list')  # This will work for most CASA builds
        except:
            print "imhead",
            header = imhead(img)  # needed to prevent crash in early CASA 4.6 builds (see CAS-8214)
            print "imhead",
            header = imhead(img, mode = 'list')
        if (header == None):
            print "imhead returned NoneType. This image header is not sufficiently standard."
            return
    if ('beammajor' in header.keys()):
        bmaj = header['beammajor']
        bmin = header['beamminor']
        bpa = header['beampa']
    elif ('perplanebeams' in header.keys()):
        beammajor = []
        beamminor = []
        beampa = []
        for beamchan in range(header['perplanebeams']['nChannels']):
            beamdict = header['perplanebeams']['*'+str(beamchan)]
            beammajor.append(beamdict['major']['value'])
            beamminor.append(beamdict['minor']['value'])
            beampa.append(beamdict['positionangle']['value'])
        bmaj = np.median(beammajor)
        bmin = np.median(beamminor)
        sinbpa = np.sin(np.radians(np.array(beampa)))
        cosbpa = np.cos(np.radians(np.array(beampa)))
        bpa = np.degrees(np.median(np.arctan2(np.median(sinbpa), np.median(cosbpa))))
    else:
        bmaj = 0
        bmin = 0
        bpa = 0
    naxis1 = header['shape'][0]
    naxis2 = header['shape'][1]
    cdelt1 = header['cdelt1']
    cdelt2 = header['cdelt2']
    if (header['cunit1'].find('rad') >= 0):
        # convert from rad to arcsec
        cdelt1 *= 3600*180/np.pi
    elif (header['cunit1'].find('deg') >= 0):
        # convert from deg to arcsec
        cdelt1 *= 3600
    if (header['cunit2'].find('rad') >= 0):
        cdelt2 *= 3600*180/np.pi
        # convert from rad to arcsec
    elif (header['cunit2'].find('deg') >= 0):
        # convert from deg to arcsec
        cdelt2 *= 3600
    if (type(bmaj) == dict):
        # casa >= 4.1.0  (previously these were floats)
        bmaj = headerToArcsec(bmaj)
        bmin = headerToArcsec(bmin)
        bpa = headerToArcsec(bpa)/3600.
    ghz = 0
    if ('ctype4' in header.keys()):
        if (header['ctype4'] == 'Frequency'):
            imgfreq = header['crval4']
            cdelt = header['cdelt4']
            crpix = header['crpix4']
            npix = header['shape'][3]
            ghz = imgfreq*1e-9
    if (ghz == 0):
        if ('ctype3' in header.keys()):
            if (header['ctype3'] == 'Frequency'):
                imgfreq = header['crval3']
                cdelt = header['cdelt3']
                crpix = header['crpix3']
                npix = header['shape'][2]
                ghz = imgfreq*1e-9
    return([bmaj,bmin,bpa,cdelt1,cdelt2,naxis1,naxis2,ghz], header)
                                                                
def numberOfChannelsInCube(img, returnFreqs=False, returnChannelWidth=False, verbose=False, header=None):
    """
    Finds the number of channels in a CASA or FITS image cube.
    returnFreqs: if True, then also return the frequency of the
           first and last channel (in Hz)
    returnChannelWidth: if True, then also return the channel width (in Hz)
    verbose: if True, then print the frequencies of first and last channel
    -Todd Hunter
    """
    if (header == None):
        print "imhead", # the comma prevents the newline so that ...10...20 will be on same line
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
        if (returnChannelWidth):
            return(nchan,firstFreq,lastFreq,cdelt)
        else:
            return(nchan,firstFreq,lastFreq)
    else:
        if (returnChannelWidth):
            return(nchan,cdelt)
        else:
            return(nchan)

def nanmean(a, axis=0):
    """
    Takes the mean of an array, ignoring the nan entries
    """
    if (map(int, np.__version__.split('.')[:3]) < (1,8,1)):
        return(scipy.stats.nanmean(a,axis)) 
#        length = len(np.array(a)[np.where(np.isnan(a)==False)])
#        return(np.nansum(a,axis)/length)
    else:
        return(np.nanmean(a,axis))

def _nanmedian(arr1d, preop=None):  # This only works on 1d arrays
    """Private function for rank a arrays. Compute the median ignoring Nan.

    Parameters
    ----------
    arr1d : ndarray
        Input array, of rank 1.

    Results
    -------
    m : float
        The median.
    """
    x = arr1d.copy()
    c = np.isnan(x)
    s = np.where(c)[0]
    if s.size == x.size:
        warnings.warn("All-NaN slice encountered", RuntimeWarning)
        return np.nan
    elif s.size != 0:
        # select non-nans at end of array
        enonan = x[-s.size:][~c[-s.size:]]
        # fill nans in beginning of array with non-nans of end
        x[s[:enonan.size]] = enonan
        # slice nans away
        x = x[:-s.size]
    if preop:
        x = preop(x)
    return np.median(x, overwrite_input=True)

def nanmedian(x, axis=0, preop=None):
    """
    Compute the median along the given axis ignoring nan values.

    Parameters
    ----------
    x : array_like
        Input array.
    axis : int or None, optional
        Axis along which the median is computed. Default is 0.
        If None, compute over the whole array `x`.
    preop : function
        function to apply on 1d slice after removing the NaNs and before
        computing median

    Returns
    -------
    m : float
        The median of `x` along `axis`.

    See Also
    --------
    nanstd, nanmean, numpy.nanmedian

    Examples
    --------
    >>> from scipy import stats
    >>> a = np.array([0, 3, 1, 5, 5, np.nan])
    >>> stats.nanmedian(a)
    array(3.0)

    >>> b = np.array([0, 3, 1, 5, 5, np.nan, 5])
    >>> stats.nanmedian(b)
    array(4.0)

    Example with axis:

    >>> c = np.arange(30.).reshape(5,6)
    >>> idx = np.array([False, False, False, True, False] * 6).reshape(5,6)
    >>> c[idx] = np.nan
    >>> c
    array([[  0.,   1.,   2.,  nan,   4.,   5.],
           [  6.,   7.,  nan,   9.,  10.,  11.],
           [ 12.,  nan,  14.,  15.,  16.,  17.],
           [ nan,  19.,  20.,  21.,  22.,  nan],
           [ 24.,  25.,  26.,  27.,  nan,  29.]])
    >>> stats.nanmedian(c, axis=1)
    array([  2. ,   9. ,  15. ,  20.5,  26. ])

    """
    x = np.asarray(x)
    if axis is None:
        x = x.ravel()
        axis = 0
    if x.ndim == 0:
        return float(x.item())
    if preop is None and hasattr(np, 'nanmedian'):
        return np.nanmedian(x, axis)
    x = np.apply_along_axis(_nanmedian, axis, x, preop)
    if x.ndim == 0:
        x = float(x.item())
    return x

def avgOverCube(pixels, useAbsoluteValue=False, threshold=None, median=False):
    """
    Computes the average spectrum across a multi-dimensional
    array read from an image cube, ignoring any NaN values.
    If threshold is specified, then it only includes pixels
    with an intensity above that value.
    Note: This function assumes that the spectral axis is the final axis.
        If it is not, there is no single setting of the axis parameter that 
        can make it work.
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

def findSpectralAxis(myia):
    mycs = myia.coordsys()
    try:
        iax = mycs.findaxisbyname("spectral")
    except:
        print "ERROR: can't find spectral axis.  Assuming it is 3."
        iax = 3
    mycs.done()
    return iax

def meanSpectrum(img='g35.03_KDnh3_11.hline.self.image', nBaselineChannels=16,
                 sigmaCube=3, verbose=False, nanBufferChannels=2, useAbsoluteValue=False,
                 baselineMode='edge', percentile=20, continuumThreshold=None,
                 meanSpectrumFile='', centralArcsec=-1, imageInfo=[], chanInfo=[]):
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
    centralArcsec: default=-1 means whole field
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
    myia = createCasaTool(iatool)
    myia.open(img)
    axis = findSpectralAxis(myia)
    if verbose: print "Found spectral axis = ", axis
    if (centralArcsec < 0):
        pixels = myia.getregion()
    else:
        myrg = createCasaTool(rgtool)
        bmaj, bmin, bpa, cdelt1, cdelt2, naxis1, naxis2, freq = imageInfo # getImageInfo(img)
        nchan = chanInfo[0] # numberOfChannelsInCube(img)
        x0 = naxis1*0.5 - centralArcsec*0.5/np.abs(cdelt1)
        x1 = naxis1*0.5 + centralArcsec*0.5/np.abs(cdelt1)
        y0 = naxis2*0.5 - centralArcsec*0.5/cdelt2
        y1 = naxis2*0.5 + centralArcsec*0.5/cdelt2
        trc = [x1,y1,0,0]
        trc[axis] = nchan
        region = myrg.box(blc=[x0,y0,0,0], trc=trc)
        pixels = myia.getregion(region=region)
        myrg.done()
    if verbose:
        print "shape of pixels = ", np.shape(pixels)
    if (continuumThreshold != None):
        belowThreshold = np.where(pixels < continuumThreshold)
        if verbose:
            print "shape of belowThreshold = ", np.shape(belowThreshold)
        pixels[belowThreshold] = 0.0
    std = MAD(pixels, axis=None)
    if verbose: print "MAD of whole cube = ", std
    naxes = len(np.shape(pixels))
    nchan = np.shape(pixels)[axis]

    if (baselineMode == 'edge'):
        # Method #1: Use the two edges of the spw to find the line-free rms of the spectrum
        nEdgeChannels = nBaselineChannels/2
        # lower edge
        blc = np.zeros(naxes)
        trc = [i-1 for i in list(np.shape(pixels))]
        trc[axis] = nEdgeChannels
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
        blc[axis] = trc[axis] - nEdgeChannels
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
    
    if (baselineMode != 'edge'):
        # Method #2: pick the N channels with the lowest absolute values (to avoid
        #          confusion from absorption lines and negative bowls of missing flux)
        npixFraction = nBaselineChannels*1.0/nchan
        if (centralArcsec < 0):
            allPixels = myia.getregion()
        else:
            allPixels = pixels
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

    if (baselineMode == 'edge'):
        std = stdEdge
        median = medianEdge
        print "meanSpectrum(): edge mode:  median=%f  MAD=%f  threshold=%f (edgesUsed=%d)" % (medianEdge, stdEdge, medianEdge+stdEdge*sigmaCube, edgesUsed)
    else:
        std = stdMin
        median = medianMin
        edgesUsed = 0
        print "meanSpectrum(): min mode:  median=%f  MAD=%f  threshold=%f" % (medianMin, stdMin, medianMin+stdMin*sigmaCube)
    
    if (axis == 2 and naxes == 4):
        # drop the degenerate axis so that avgOverCube will work with nanmean(axis=0)
        pixels = pixels[:,:,:,0]
    avgspectrum = avgOverCube(pixels, useAbsoluteValue)
    threshold = median + sigmaCube*std
    if verbose: 
        print "Using threshold above which to compute mean spectrum = ", threshold
    pixels[np.where(pixels < threshold)] = 0.0
    avgspectrumAboveThreshold = avgOverCube(pixels, useAbsoluteValue, threshold)
    if verbose: 
        print "Running removeNaNs (len(avgspectrumAboveThreshold)=%d)" % (len(avgspectrumAboveThreshold))
    nansRemoved = removeNaNs(avgspectrumAboveThreshold)
    nansReplaced,nanmin = removeNaNs(avgspectrumAboveThreshold, replaceWithMin=True, 
                                     nanBufferChannels=nanBufferChannels)
    nchan, firstFreq, lastFreq, channelWidth = chanInfo # numberOfChannelsInCube(img, returnFreqs=True)
    frequency = np.linspace(firstFreq, lastFreq, nchan)
    if verbose: 
        print "Running writeMeanSpectrum"
    writeMeanSpectrum(meanSpectrumFile, frequency, avgspectrum, nansReplaced, threshold,
                      edgesUsed, nchan, nanmin)
    return(avgspectrum, nansRemoved, nansReplaced, threshold, 
           edgesUsed, nchan, nanmin)

def removeNaNs(a, replaceWithMin=False, verbose=False, nanBufferChannels=0, 
               replaceWithZero=False):
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
        print "Found %d nan channels" % (len(np.where(idx==True)[0]))
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
            return(a, 0)
    else:
        a = a[np.where(np.isnan(a) == False)]
        if (verbose):
            print "Removed %d NaNs" % (startLength-len(a))
        return(a)

def create_casa_quantity(myqatool,value,unit):
    """
    A wrapper to handle the changing ways in which casa quantities are invoked.
    Todd Hunter
    """
    if (type(casac.Quantity) != type):  # casa 4.x
        myqa = myqatool.quantity(value, unit)
    else:  # casa 3.x
        myqa = casac.Quantity(value, unit)
    return(myqa)

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
    a = np.asarray(a, np.float64)
    m = nanmedian(a, axis=axis,
                  preop=lambda x:
                        np.fabs(np.subtract(x, np.median(x, axis=None), out=x), out=x))
    return m / c

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

def getFreqsForImage(img, header='', spectralaxis=-1, unit='GHz'):
    """
    Returns an array of frequencies of an image cube in the specified unit.
    Not currently used.
    """
    if (header == ''):
        header = imhead(img,mode='list')
    myia = createCasaTool(iatool)
    myia.open(img)
    if (spectralaxis < 0):
        spectralaxis = findSpectralAxis(myia)
    startchan = [None,None,None,None]
    stopchan = [None,None,None,None]
    startchan[spectralaxis] = 0
    nchan = header['shape'][spectralaxis]
    stopchan[spectralaxis] = nchan
    myqa = createCasaTool(qatool)
    m0 = myia.coordmeasures(startchan)['measure']['spectral']['frequency']['m0']
    m1 = myia.coordmeasures(stopchan)['measure']['spectral']['frequency']['m0']
    f0 = myqa.convert('%.15e %s' % (m0['value'], m0['unit']), unit)
    f1 = myqa.convert('%.15e %s' % (m1['value'], m1['unit']), unit)
    freqs = np.linspace(f0['value'],f1['value'],nchan)
    myia.close()
    myqa.done()
    return freqs

def CalcAtmTransmissionForImage(img, header='', chanInfo='', airmass=1.5, pwv=-1, spectralaxis=-1, 
                                maxAtmCalcChannels=960, value='transmission', P=-1, H=-1, 
                                T=-1, altitude=-1):
    """
    supported telescopes are VLA and ALMA (needed for default weather and PWV)
    value: 'transmission' or 'tsky'
    pwv: in mm
    P: in mbar
    H: in percent
    T: in Kelvin
    """
    if (header == ''):
        print "imhead", # the comma prevents the newline so that ...10...20 will be on same line
        header = imhead(img,mode='list')
    if (type(header) != dict):
        # Input was a spectrum rather than an image
        if (chanInfo[1] < 60e9):
            telescopeName = 'ALMA'
        else:
            telescopeName = 'VLA'
        freqs = np.linspace(chanInfo[1]*1e-9,chanInfo[2]*1e-9,chanInfo[0])
    else:
        telescopeName = header['telescope']
        # this will not match up with the plot, which uses numberOfChannelsInCube
#        freqs = getFreqsForImage(img, header, spectralaxis)
        freqs = np.linspace(chanInfo[1]*1e-9,chanInfo[2]*1e-9,chanInfo[0])
    P0 = 1000.0 # mbar
    H0 = 20.0   # percent
    T0 = 273.0  # Kelvin
    if (telescopeName.find('ALMA') >= 0 or telescopeName.find('ACA') >= 0):
        pwv0 = 1.0   
        P0 = 563.0
        H0 = 20.0
        T0 = 273.0
        altitude0 = 5059
    elif (telescopeName.find('VLA') >= 0):
        P0 = 786.0
        pwv0 = 5.0  
        altitude0 = 2124
    else:
        pwv0 = 10.0  
        altitude0 = 0
    if (pwv < 0):
        pwv = pwv0
    if (T < 0):
        T = T0
    if (H < 0):
        H = H0
    if (P < 0):
        P = P0
    if (altitude < 0):
        altitude = altitude0
    tropical = 1
    midLatitudeSummer = 2
    midLatitudeWinter = 3
#    print "image bandwidth = %f GHz" % (np.max(freqs)-np.min(freqs))
    numchan = len(freqs)
    reffreq = 0.5*(freqs[numchan/2-1]+freqs[numchan/2])
    while (numchan > maxAtmCalcChannels):
        numchan /= 2
    freqs = np.linspace(freqs[0], freqs[-1], numchan)
    chansep = (freqs[-1]-freqs[0])/(numchan-1)
#    print "regridded bandwidth = %f GHz" % (np.max(freqs)-np.min(freqs))
    nbands = 1
    myqa = createCasaTool(qatool)
    fCenter = create_casa_quantity(myqa, reffreq, 'GHz')
    fResolution = create_casa_quantity(myqa, chansep, 'GHz')
    fWidth = create_casa_quantity(myqa, numchan*chansep, 'GHz')
    myat = casac.atmosphere()
    myat.initAtmProfile(humidity=H, temperature=create_casa_quantity(myqa,T,"K"),
                        altitude=create_casa_quantity(myqa,altitude,"m"),
                        pressure=create_casa_quantity(myqa,P,'mbar'),atmType=midLatitudeWinter)
    myat.initSpectralWindow(nbands, fCenter, fWidth, fResolution)
    myat.setUserWH2O(create_casa_quantity(myqa, pwv, 'mm'))
#    myat.setAirMass()  # This does not affect the opacity, but it does effect TebbSky, so do it manually.
    myqa.done()
    rc = myat.getRefChan()
    n = myat.getNumChan()
    dry = np.array(myat.getDryOpacitySpec(0)[1])
    wet = np.array(myat.getWetOpacitySpec(0)[1]['value'])
    TebbSky = myat.getTebbSkySpec(spwid=0)[1]['value']
    # readback the values to be sure they got set
    
    rf = myat.getRefFreq()['value']
    cs = myat.getChanSep()['value']
    if (myat.getRefFreq()['unit'] != 'GHz'):
        print "There is a unit mismatch for refFreq in the code."
    if (myat.getChanSep()['unit'] != 'MHz'):
        print "There is a unit mismatch for chanSep in the code."
    numchan = myat.getNumChan()
    freq0 = myat.getChanFreq(0)['value']
    freq1 = myat.getChanFreq(numchan-1)['value']
#    print "atm return bandwidth = %f GHz" % (freq1-freq0)
    freqs = np.linspace(freq0, freq1, numchan)
    transmission = np.exp(-airmass*(wet+dry))
    TebbSky *= (1-np.exp(-airmass*(wet+dry)))/(1-np.exp(-wet-dry))
    if value=='transmission':
        values = transmission
    else:
        values = TebbSky
    return(freqs, values)

def channelSelectionRangesToIndexArray(selection, separator=';'):
    """
    Convert a channel selection range string to integer array of indicies.
    e.g.:  '3~8;'10~11' -> [3,4,5,6,7,8,10,11]
    """
    index = []
    for s in selection.split(separator):
        a,b = s.split('~')
        index += range(int(a),int(b)+1,1)
    return(np.array(index))

def linfit(x, y, yerror, pinit=[0,0]):
    """
    Basic linear function fitter with error bars in y-axis data points.
    Uses scipy.optimize.leastsq().  Accepts either lists or arrays.
    Example:
         lf = au.linfit()
         lf.linfit(x, y, yerror, pinit)
    Input:
         x, y: x and y axis values
         yerror: uncertainty in the y-axis values (vector or scalar)
         pinit contains the initial guess of [slope, intercept]
    Output:
       The fit result as: [slope, y-intercept]
    - Todd Hunter
    """
    x = np.array(x)
    y = np.array(y)
    if (type(yerror) != list and type(yerror) != np.ndarray):
        yerror = np.ones(len(x)) * yerror
    fitfunc = lambda p, x: p[1] + p[0]*x
    errfunc = lambda p,x,y,err: (y-fitfunc(p,x))/(err**2)
    out = scipy.optimize.leastsq(errfunc, pinit, args=(x,y,yerror/y), full_output=1)
    p = out[0]
    covar = out[1]
    return(p)

def channelsInLargestGroup(selection):
    """
    Returns the number of channels in the largest group of channels in a
    CASA selection string.
    """
    if (selection == ''):
        return 0
    ranges = selection.split(';')
    largest = 0
    for r in ranges:
        c0 = int(r.split('~')[0])
        c1 = int(r.split('~')[1])
        channels = c1-c0+1
        if (channels > largest):
            largest = channels
    return largest

def countChannelsInRanges(channels, separator=';'):
    """
    Counts the number of channels in one spw of a CASA channel selection string
    and return a list of numbers.
    e.g. "5~20;30~40"  yields [16,11]
    """
    tokens = channels.split(separator)
    count = []
    for i in range(len(tokens)):
        c0 = int(tokens[i].split('~')[0])
        c1 = int(tokens[i].split('~')[1])
        count.append(c1-c0+1)
    return count

def countChannels(channels):
    """
    Counts the number of channels in a CASA channel selection string.
    If multiple spws, then return a dictionary of counts
    e.g. "1:5~20;30~40"  yields 27; or  '6~30' yields 25
         "1:5~20;30~40,2:6~30" yields {1:27, 2:25}
    """
    if (channels == ''):
        return 0
    tokens = channels.split(',')
    nspw = len(tokens)
    count = {}
    for i in range(nspw):
        string = tokens[i].split(':')
        if (len(string) == 2):
            spw,string = string
        else:
            string = string[0]
            spw = 0
        ranges = string.split(';')
        for r in ranges:
            c0 = int(r.split('~')[0])
            c1 = int(r.split('~')[1])
            if (c0 > c1):
                print "Invalid channel range: c0 > c1 (%d > %d)" % (c0,c1)
                return
        channels = [1+int(r.split('~')[1])-int(r.split('~')[0]) for r in ranges]
        count[spw] = np.sum(channels)
    if (nspw == 1):
        count = count[spw]
    return(count)

