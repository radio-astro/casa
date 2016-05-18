"""
Demonstration of algorithm to determine continuum channel ranges to 
use from a CASA image cube (dirty or clean).  All dependencies on 
analysisUtils functions have been pasted into this file for convenience.
This function is meant to be run inside CASA.  Simple usage:
  import findContinuum as fc
  fc.findContinuum('my_dirty_cube.image')
"""

import os
try:
    import pyfits
except:
    print 'WARNING: pyfits not available !'
import numpy as np
import matplotlib.pyplot as pl
import matplotlib.ticker
import time as timeUtilities
import scipy
from scipy.stats import scoreatpercentile, percentileofscore
from scipy.ndimage.filters import gaussian_filter
from taskinit import *
from imhead_cli import imhead_cli as imhead
from imregrid_cli import imregrid_cli as imregrid
import warnings
import subprocess
import casadef

def version(showfile=True):
    """
    Returns the CVS revision number.
    """
    myversion = "$Id: findContinuum.py,v 1.69 2016/05/17 15:45:19 we Exp $" 
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

def getMemorySize():
    try:
        return(os.sysconf('SC_PAGE_SIZE') * os.sysconf('SC_PHYS_PAGES'))
    except ValueError:
        # SC_PHYS_PAGES can be missing on OS X
        return int(subprocess.check_output(['sysctl', '-n', 'hw.memsize']).strip())

def findContinuum(img='', spw='', transition='', baselineModeA='min', baselineModeB='min',
                  sigmaCube=3, nBaselineChannels=0.19, sigmaFindContinuum='auto',
                  verbose=False, png='', pngBasename=False, nanBufferChannels=2, 
                  source='', useAbsoluteValue=True, trimChannels='auto', 
                  percentile=20, continuumThreshold=None, narrow='auto', 
                  separator=';', overwrite=False, titleText='', 
                  showAverageSpectrum=False, maxTrim=20, maxTrimFraction=1.0,
                  meanSpectrumFile='', centralArcsec='auto', alternateDirectory='.',
                  header='', plotAtmosphere='transmission', airmass=1.5, pwv=1.0,
                  channelFractionForSlopeRemoval=0.75, mask='', 
                  invert=False, meanSpectrumMethod='auto',peakFilterFWHM=10,
                  skyTempThreshold=20):
    """
    This function calls functions to:
    1) compute the mean spectrum of a dirty cube
    2) find the continuum channels 
    3) plot the results
    It calls runFindContinuum up to 2 times.  It runs it a second time with a reduced 
    field size if it finds only one range of continuum channels.

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
    nBaselineChannels: if integer, then the number of channels to use in:
      a) computing the mean spectrum with the 'meanAboveThreshold' methods.
      b) computing the MAD of the lowest/highest channels in findContinuumChannels
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
    meanSpectrumMethod: 'auto', 'peakOverMad', 'peakOverRms', 'meanAboveThreshold', 
       'meanAboveThresholdOverRms', 'meanAboveThresholdOverMad', 
        where 'peak' refers to the peak in a spatially-smoothed version of cube
        'auto' currently invokes 'meanAboveThreshold' unless sky temperature range (max-min)
        from the atmospheric model is more than skyTempThreshold
    skyTempThreshold: value in Kelvin, above which meanSpectrumMethod is changed in 'auto' mode
    peakFilterFWHM: value used by 'peakOverRms' and 'peakOverMad' methods to presmooth 
        each plane with a Gaussian kernel of this width (in pixels).  Set to 1 to not do any 
        smoothing.  Default = 10 which is typically about 2 synthesized beams.
    meanSpectrumFile: an alternative ASCII text file to use for the mean spectrum.
                      Must also set img=''.
    centralArcsec: radius of central box within which to compute the mean spectrum
        default='auto' means start with whole field, then reduce to 1/10 if only
        one window is found (unless mask is specified); or 'fwhm' for the central square
        with the same radius as the PB FWHM; or a floating point value in arcseconds
    mask: a mask image preferably equal in shape to the parent image that is used to determine
        the region to compute the noise (outside the mask) and (in the 'meanAboveThresholdMethod')
        the mean spectrum (inside the mask).  The spatial union of all masked pixels in all 
        channels is used as the mask for each channel.
        Option 'auto' will look for the <filename>.mask that matches the <filename>.image
        and if it finds it, it will use it; otherwise it will use no mask.
        If the mask does not match in shape but is multi-channel, it will be regridded to match
        and written out as <filename>.mask.regrid.
        If it matches spatially but is single-channel, this channel will be used for all.
    header: dictionary created by imhead(img, mode='list')
    plotAtmosphere: '', 'tsky', or 'transmission'
    airmass: for plot of atmospheric transmission
    pwv: in mm (for plot of atmospheric transmission)
    channelFractionForSlopeRemoval: if this many channels are initially selected, then fit 
         and remove a linear slope and re-identify continuum channels (for the
         meanAboveThreshold methods only)
    invert: if reading previous meanSpectrum file, then invert the sign and add the minimum
    """
    print "\n BEGINNING: findContinuum.findContinuum('%s', centralArcsec='%s', mask='%s', overwrite=%s, sigmaFindContinuum='%s', meanSpectrumMethod='%s', peakFilterFWHM=%.0f)" % (img, str(centralArcsec), mask, overwrite, str(sigmaFindContinuum), meanSpectrumMethod, peakFilterFWHM)
    img = img.rstrip('/')
    imageInfo = [] # information returned from getImageInfo
    if (img != ''):
        if (not os.path.exists(img)):
            print "Could not find image = ", img
            return
        result = getImageInfo(img, header)
        if (result == None): return result
        imageInfo, header = result
        bmaj, bmin, bpa, cdelt1, cdelt2, naxis1, naxis2, freq = imageInfo
        chanInfo = numberOfChannelsInCube(img, returnChannelWidth=True, returnFreqs=True, 
                                          header=header)
        nchan,firstFreq,lastFreq,channelWidth = chanInfo
        if (nchan < 2):
            print "You must have more than one channel in the image."
            return
        channelWidth = abs(channelWidth)
    else:
        header = []
        chanInfo = []
        channelWidth = 0
    if (mask == 'auto'):
        mask = img.rstrip('.image') + '.mask'
        if (not os.path.exists(mask)):
            mask = ''
        else:
            maskInfo, maskhead = getImageInfo(mask)
            if (maskhead['shape'] == header['shape']).all():
                # are there any unmasked pixels?
                if (maskhead['datamax'] >= 0.5):
                    print "Will use the clean mask to limit the pixels averaged."
                    centralArcsec = -1
                else:
                    print "No valid pixels in the mask.  Will ignore it."
                    mask = ''
            else:
                print "Shape of mask (%s) does not match the image (%s)." % (maskhead['shape'],header['shape'])
                print "If you want to autmoatically regrid the mask, then set its name explicitly."
                return
    else:
        if mask == False: 
            mask = ''
        if (mask != ''):
            if (not os.path.exists(mask)):
                print "Could not find image mask = ", mask
                return
            maskInfo, maskhead = getImageInfo(mask)
            if (maskhead['shape'] == header['shape']).all():
                print "Shape of mask matches the image."
            else:
                print "Shape of mask (%s) does not match the image (%s)." % (maskhead['shape'],header['shape'])
                # check if the spatial dimension matches.  If so, then simply add channels
                if (maskhead['shape'][0] == header['shape'][0] and
                    maskhead['shape'][1] == header['shape'][1]):
                    myia = createCasaTool(iatool)
                    myia.open(img)
                    axis = findSpectralAxis(myia)
                    if (header['shape'][axis] != 1):
                        print "Regridding the spectral axis of the mask with replicate."
                        imregrid(mask, output=mask+'.regrid', template=img, axes=[axis], replicate=True, interpolation='nearest')
                    else:
                        print "This single plane mask will be used for all channels."
                else:
                    print "Regridding the mask spatially and spectrally."
                    imregrid(mask, output=mask+'.regrid', template=img, asvelocity=False, interpolation='nearest')
                mask = mask+'.regrid'
                maskInfo, maskhead = getImageInfo(mask)

    bytes = getMemorySize()
    try:
        hostname = os.getenv('HOST')
    except:
        hostname = "?"
    print "Total memory on %s = %.3f GB" % (hostname,
                                            bytes/1e9)
    if (meanSpectrumMethod.find('auto') >= 0):
        meanSpectrumMethod = 'meanAboveThreshold'
        if (img != ''):
            a,b,c,d = atmosphereVariation(img, header, chanInfo, airmass=airmass, pwv=pwv)
            if (c > skyTempThreshold):
                meanSpectrumMethod = 'peakOverMad'
                print "Switching to %s since %f > %f" % (meanSpectrumMethod,c,skyTempThreshold)
    if (centralArcsec == 'auto' and img != ''):
        npixels = float(nchan)*naxis1*naxis2
        maxpixels = bytes/67 # float(1024)*1024*960
        if (npixels > maxpixels): # and meanSpectrumMethod.find('meanAboveThreshold')>=0):
            print "Excessive number of pixels (%.0f > %.0f)" % (npixels,maxpixels)
            totalWidthArcsec = abs(cdelt2*naxis2)
            if (mask == ''):
                centralArcsecField = totalWidthArcsec*maxpixels/npixels
            else:
                print "Finding size of the central square that fully contains the mask."
                centralArcsecField = widthOfMaskArcsec(mask)
            print "Reducing image width examined from %.2f to %.2f arcsec to avoid memory problems." % (totalWidthArcsec,centralArcsecField)
        else:
            print "Using whole field since npixels=%d < maxpixels=%d" % (npixels, maxpixels)
            centralArcsecField = -1  # use the whole field
    elif (centralArcsec == 'fwhm' and img != ''):
        centralArcsecField = primaryBeamArcsec(frequency=getFitsBeam(image)[-1],
                                               showEquation=False)
    else:  
        centralArcsecField = centralArcsec

    result = runFindContinuum(img, spw, transition, baselineModeA,baselineModeB,
                              sigmaCube, nBaselineChannels, sigmaFindContinuum,
                              verbose, png, pngBasename, nanBufferChannels, 
                              source, useAbsoluteValue, trimChannels, 
                              percentile, continuumThreshold, narrow, 
                              separator, overwrite, titleText, 
                              showAverageSpectrum, maxTrim, maxTrimFraction,
                              meanSpectrumFile, centralArcsecField,channelWidth,
                              alternateDirectory, imageInfo, chanInfo, header,
                              plotAtmosphere, airmass, pwv, 
                              channelFractionForSlopeRemoval, mask, 
                              invert, meanSpectrumMethod, peakFilterFWHM, 
                              iteration=0)
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
                                  channelFractionForSlopeRemoval, mask, 
                                  invert, meanSpectrumMethod, peakFilterFWHM, 
                                  iteration=1)
        if result == None:
            return
        selection, png, slope = result
    aggregateBandwidth = computeBandwidth(selection, channelWidth)
    # Write summary of results to text file
    if (meanSpectrumFile == ''): 
        meanSpectrumFile = buildMeanSpectrumFilename(img, meanSpectrumMethod, peakFilterFWHM)
    writeContDat(meanSpectrumFile, selection, png, aggregateBandwidth)
    return(selection, png, aggregateBandwidth)

def maskArgumentMismatch(mask, meanSpectrumFile):
    """
    Determines if the requested mask does not match what was used to generate 
    the specified meanSpectrumFile.
    """
    if ((mask == '' and grep(meanSpectrumFile,'mask')[0] != '') or
        ((mask != '' and mask != False) and grep(meanSpectrumFile,'mask')[0] == '')):
        return True
    else:
        return False

def centralArcsecArgumentMismatch(centralArcsec, meanSpectrumFile):
    """
    Determines if the requested centralArcsec value does not match what was used to 
    generate the specified meanSpectrumFile.
    """
    if (centralArcsec == 'auto' or centralArcsec == -1):
        if (grep(meanSpectrumFile,'centralArcsec=auto')[0] == '' and 
            grep(meanSpectrumFile,'centralArcsec=-1')[0] == ''):
            print "request for auto but 'centralArcsec=auto' not found and 'centralArcsec=-1' not found"
            return True
        else:
            return False
    elif (grep(meanSpectrumFile,'centralArcsec=auto %s'%(str(centralArcsec)))[0] == '' and
          grep(meanSpectrumFile,'centralArcsec=%s'%(str(centralArcsec)))[0] == ''):
        print "request for specific value but 'centralArcsec=auto %s' not found" % (str(centralArcsec))
        return True
    else:
        return False

def writeContDat(meanSpectrumFile, selection, png, aggregateBandwidth):
    """
    Writes out an ASCII file called <meanSpectrumFile>_findContinuum.dat
    that contains the selected channels, the png name and the aggregate 
    bandwidth in GHz.
    """
    if (meanSpectrumFile.find('.meanSpectrum') > 0):
        contDat = meanSpectrumFile.split('.meanSpectrum')[0] + '_findContinuum.dat'
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

def drawYlabel(img, typeOfMeanSpectrum, meanSpectrumMethod, meanSpectrumThreshold,
               peakFilterFWHM, fontsize):
    """
    Draws a descriptive y-axis label based on the origin and type of mean spectrum used.
    """
    if (img == ''):
        label = 'Mean spectrum passed in as ASCII file'
    else:
        if (meanSpectrumMethod.find('meanAboveThreshold') >= 0):
            if (meanSpectrumMethod.find('OverMad') > 0):
                label = '(Average spectrum > threshold=(%g))/MAD' % (roundFigures(meanSpectrumThreshold,3))
            elif (meanSpectrumMethod.find('OverRms') > 0):
                label = '(Average spectrum > threshold=(%g))/RMS' % (roundFigures(meanSpectrumThreshold,3))
            else:
                label = 'Average spectrum > threshold=(%g)' % (roundFigures(meanSpectrumThreshold,3))
        elif (meanSpectrumMethod.find('peakOverMad')>=0):
            if peakFilterFWHM > 1:
                label = 'Per-channel (Peak / MAD) of image smoothed by FWHM=%d pixels' % (peakFilterFWHM)
            else:
                label = 'Per-channel (Peak / MAD)'
        elif (meanSpectrumMethod.find('peakOverRms')>=0):
            if peakFilterFWHM > 1:
                label = 'Per-channel (Peak / RMS) of image smoothed by FWHM=%d pixels' % (peakFilterFWHM)
            else:
                label = 'Per-channel (Peak / RMS)'
    pl.ylabel(typeOfMeanSpectrum+' '+label, size=fontsize)

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

def buildMeanSpectrumFilename(img, meanSpectrumMethod, peakFilterFWHM):
    """
    Creates the name of the meanSpectrumFile to search for and/or create.
    """
    if (meanSpectrumMethod.find('peak')>=0):
        return(img + '.meanSpectrum.'+meanSpectrumMethod+'_%d'%peakFilterFWHM)
    else:
        if (meanSpectrumMethod == 'meanAboveThreshold'):
            return(img + '.meanSpectrum')
        else:
            return(img + '.meanSpectrum.'+meanSpectrumMethod)

def tdmSpectrum(channelWidth, nchan):
    """
    Use 15e6 instead of 15.625e6 because LSRK chan width can be slightly narrower than TOPO
    """
    if ((channelWidth >= 15e6/2. and nchan>240) or # 1 pol TDM, must avoid 240-chan FDM
         (channelWidth >= 15e6)):
#        (channelWidth >= 15e6 and nchan<96) or     # 2 pol TDM (128 chan)
#        (channelWidth >= 30e6 and nchan<96)):      # 4 pol TDM (64 chan)
        return True
    else:
        return False

def atmosphereVariation(img, header, chanInfo, airmass=1.5, pwv=-1):
    """
    Computes the absolue and percentage variation in atmospheric transmission 
    and sky temperature across an image cube.
    Returns 4 values: max(Trans)-min(Trans), and as percentage of mean,
                      Max(Tsky)-min(Tsky), and as percentage of mean
    """
    freqs, values = CalcAtmTransmissionForImage(img, header, chanInfo, airmass=airmass, pwv=pwv, value='transmission')
    maxMinusMin = np.max(values)-np.min(values)
    percentage = maxMinusMin*100/np.mean(values)
    freqs, values = CalcAtmTransmissionForImage(img, header, chanInfo, airmass=airmass, pwv=pwv, value='tsky')
    TmaxMinusMin = np.max(values)-np.min(values)
    Tpercentage = maxMinusMin*100/np.mean(values)
    return(maxMinusMin, percentage, TmaxMinusMin, Tpercentage)

def runFindContinuum(img='', spw='', transition='', baselineModeA='min', baselineModeB='min',
                     sigmaCube=3, nBaselineChannels=0.19, sigmaFindContinuum='auto',
                     verbose=False, png='', pngBasename=False, nanBufferChannels=2, 
                     source='', useAbsoluteValue=True, trimChannels='auto', 
                     percentile=20, continuumThreshold=None, narrow='auto', 
                     separator=';', overwrite=False, titleText='', 
                     showAverageSpectrum=False, maxTrim=20, maxTrimFraction=1.0,
                     meanSpectrumFile='', centralArcsec=-1, channelWidth=0,
                     alternateDirectory='.', imageInfo=[], chanInfo=[], 
                     header='', plotAtmosphere='transmission', airmass=1.5, pwv=1.0,
                     channelFractionForSlopeRemoval=0.8, mask='', 
                     invert=False, meanSpectrumMethod='peakOverMad', 
                     peakFilterFWHM=15, fullLegend=False, iteration=0):
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
    nBaselineChannels: if integer, then the number of channels to use in:
      a) computing the mean spectrum with the 'meanAboveThreshold' methods.
      b) computing the MAD of the lowest/highest channels in findContinuumChannels
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
                      Must also set img=''.
    meanSpectrumMethod: 'peakOverMad', 'peakOverRms', 'meanAboveThreshold', 
       'meanAboveThresholdOverRms', 'meanAboveThresholdOverMad', 
        where 'peak' refers to the peak in a spatially-smoothed version of cube
    peakFilterFWHM: value used by 'peakOverRms' and 'peakOverMad' to presmooth 
        each plane with a Gaussian kernel of this width (in pixels)
        Set to 1 to not do any smoothing.
    centralArcsec: radius of central box within which to compute the mean spectrum
        default='auto' means start with whole field, then reduce to 1/10 if only
        one window is found (unless mask is specified); or 'fwhm' for the central square
        with the same radius as the PB FWHM; or a floating point value in arcseconds
    mask: a mask image equal in shape to the parent image that is used to determine the
        region to compute the noise (outside the mask) and the mean spectrum (inside the mask)
        option 'auto' will look for the <filename>.mask that matches the <filename>.image
        and if it finds it, it will use it; otherwise it will use no mask
    plotAtmosphere: '', 'tsky', or 'transmission'
    airmass: for plot of atmospheric transmission
    pwv: in mm (for plot of atmospheric transmission)
    channelFractionForSlopeRemoval: if this many channels are initially selected, then fit 
         and remove a linear slope and re-identify continuum channels.
    """
    startTime = timeUtilities.time()
    slope=None 
    replaceNans = True # This used to be a command-line option, but no longer.
    img = img.rstrip('/')
    fitsTable = False
    typeOfMeanSpectrum = 'Existing'
    if (meanSpectrumFile != '' and os.path.exists(meanSpectrumFile)):
        print "Using existing meanSpectrumFile = ", meanSpectrumFile
        if (is_binary(meanSpectrumFile)):
            fitsTable = True
    if (type(nBaselineChannels) == float and not fitsTable):
        # chanInfo will be == [] if an ASCII meanSpectrumFile is specified
        if len(chanInfo) >= 4:
            nchan, firstFreq, lastFreq, channelWidth = chanInfo # numberOfChannelsInCube(img, returnFreqs=True)
            channelWidth = abs(channelWidth)
            nBaselineChannels = int(round(nBaselineChannels*nchan))
            print "Found %d channels in the cube" % (nchan)
    if (nBaselineChannels < 2 and not fitsTable and len(chanInfo) >= 4):
        print "You must have at least 2 edge channels"
        return
    if (meanSpectrumFile == ''):
        meanSpectrumFile = buildMeanSpectrumFilename(img, meanSpectrumMethod, peakFilterFWHM)
    elif (not os.path.exists(meanSpectrumFile)):
        if (len(os.path.dirname(img)) > 0):
            meanSpectrumFile = os.path.dirname(img) + '/' + os.path.basename(meanSpectrumFile)
    if not overwrite:
        if (os.path.exists(meanSpectrumFile) and img != ''):
            if (maskArgumentMismatch(mask, meanSpectrumFile) and not fitsTable):
                print "Regenerating the meanSpectrum since there is a mismatch in the mask argument."
                overwrite = True
            else:
                print "No mismatch in the mask argument vs. the meanSpectrum file."
            if (centralArcsecArgumentMismatch(centralArcsec, meanSpectrumFile) and not fitsTable):
                print "Regenerating the meanSpectrum since there is a mismatch in the centralArcsec argument (%s)." % (str(centralArcsec))
                overwrite = True
            else:
                print "No mismatch in the centralArcsec argument vs. the meanSpectrum file."
        elif (img != ''):
            print "Did not find mean spectrum file = ", meanSpectrumFile
    if (overwrite or not os.path.exists(meanSpectrumFile)):
        if (overwrite):
            print "Regenerating the mean spectrum file with centralArcsec=%s, mask='%s'." % (str(centralArcsec),mask)
        else:
            print "Generating the mean spectrum file with centralArcsec=%s, mask='%s'." % (str(centralArcsec),mask)
        typeOfMeanSpectrum = 'Computed'
        result = meanSpectrum(img, nBaselineChannels, sigmaCube, verbose,
                              nanBufferChannels,useAbsoluteValue,
                              baselineModeA, percentile,
                              continuumThreshold, meanSpectrumFile, 
                              centralArcsec, imageInfo, chanInfo, mask,
                              meanSpectrumMethod, peakFilterFWHM, iteration)
        if result == None:
            return
        avgspectrum, avgSpectrumNansRemoved, avgSpectrumNansReplaced, meanSpectrumThreshold,\
          edgesUsed, nchan, nanmin = result
        if verbose:
            print "len(avgspectrum) = %d, len(avgSpectrumNansReplaced)=%d" % (len(avgspectrum),len(avgSpectrumNansReplaced))
    else:
        if (fitsTable):
            result = readMeanSpectrumFITSFile(meanSpectrumFile)
            if (result == None):
                print "FITS table is not valid."
                return
        else:
            # An ASCII file was specified as the spectrum to process
            print "Running readPreviousMeanSpectrum('%s')" % (meanSpectrumFile)
            result = readPreviousMeanSpectrum(meanSpectrumFile, verbose, invert)
            if (result == None):
                print "ASCII file is not valid, re-run with overwrite=True"
                return
        chanInfo = [result[4], result[6], result[7], abs(result[7]-result[6])/(result[4]-1)]
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
        if (fitsTable or img==''):
            nBaselineChannels = int(round(nBaselineChannels*nchan))
            n, firstFreq, lastFreq, channelWidth = chanInfo
            channelWidth = abs(channelWidth)
            if (nBaselineChannels < 2):
                print "You must have at least 2 edge channels"
                return
            
    donetime = timeUtilities.time()
    if verbose:
        print "%.1f sec elapsed in meanSpectrum" % (donetime-startTime)
    print "Iteration %d" % (iteration)
    if (meanSpectrumMethod.find('meanAboveThreshold') >= 0):
        sFC_TDM = 4.5
    else:
        sFC_TDM = 6.5
    if (sigmaFindContinuum == 'auto' or sigmaFindContinuum == -1):
        sigmaFindContinuumAutomatic = True
        if (tdmSpectrum(channelWidth, nchan)):
            sigmaFindContinuum = sFC_TDM
        elif (meanSpectrumMethod.find('meanAboveThreshold') >= 0):
            sigmaFindContinuum = 3.5
        else:
            sigmaFindContinuum = 6.0
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
    spwBandwidth = nchan*channelWidth
    factor = 1.0
    if (singleChannelPeaksAboveSFC == allGroupsAboveSFC and allGroupsAboveSFC>1):
        if (sigmaFindContinuum < sFC_TDM):  # this seems to screw up on G19.01_B7 spw3
            # raise the threshold a bit since all the peaks look like all noise
            factor = 1.5
            sigmaFindContinuum *= factor
            print "Scaling the threshold upward by a factor of %.2f to avoid apparent noise spikes (%d==%d)." % (factor, singleChannelPeaksAboveSFC,allGroupsAboveSFC)
            continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC = \
                findContinuumChannels(avgSpectrumNansReplaced, nBaselineChannels, sigmaFindContinuum, nanmin, 
                                      baselineModeB, trimChannels, narrow, verbose, maxTrim, maxTrimFraction, separator)
            sumAboveMedian, sumBelowMedian, sumRatio, channelsAboveMedian, channelsBelowMedian, channelRatio = \
                aboveBelow(avgSpectrumNansReplaced,medianTrue)
    elif ((groups > 3 or (groups > 1 and channelRatio < 1.0) or (channelRatio < 0.5) or (groups == 2 and channelRatio < 1.3)) and sigmaFindContinuumAutomatic and meanSpectrumMethod.find('peakOver') < 0):
        print "A: groups,channelRatio=", groups, channelRatio, channelRatio < 1.0, channelRatio>0.1, tdmSpectrum(channelWidth,nchan), groups>2
        if (channelRatio < 0.9 and channelRatio > 0.1 and (firstFreq>60e9 and not tdmSpectrum(channelWidth,nchan)) and groups>2):  # was nchan>256
            # Don't allow this much reduction in ALMA TDM mode as it chops up line-free quasar spectra too much.
            # The channelRatio>0.1 requirement prevents failures due to ALMA TFB platforming.
            factor = 0.333
        elif (groups <= 2):
            if (channelRatio < 1.3 and channelRatio > 0.1 and groups == 2 and 
                not tdmSpectrum(channelWidth,nchan) and channelWidth>=1875e6/480.):
                if (channelWidth < 1875e6/360.):
                    factor = 0.5  # i.e. for galaxy spectra with FDM 480 channel (online-averaging) resolution
                else:
                    factor = 0.7  # i.e. for galaxy spectra with FDM 240 channel (online-averaging) resolution
            else:
                # prevent sigmaFindContinuum going to inf if groups==1
                # prevent sigmaFindContinuum going > 1 if groups==2
                factor = 0.9
        else:
            if tdmSpectrum(channelWidth,nchan):
                factor = 1.0
            elif channelWidth>0:  # the second factor tempers the reduction as the spw bandwidth decreases
                factor = (np.log(3)/np.log(groups)) ** (spwBandwidth/1.875e9)
            else:
                factor = (np.log(3)/np.log(groups))
        print "setting factor to %f because groups=%d, channelRatio=%g, firstFreq=%g, nchan=%d, channelWidth=%e" % (factor,groups,channelRatio,firstFreq,nchan,channelWidth)
        print "---------------------"
        print "Scaling the threshold by a factor of %.2f (groups=%d, channelRatio=%f)" % (factor, groups,channelRatio)
        print "---------------------"
        sigmaFindContinuum *= factor
        continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC = \
            findContinuumChannels(avgSpectrumNansReplaced, nBaselineChannels, sigmaFindContinuum, nanmin, 
                                  baselineModeB, trimChannels, narrow, verbose, maxTrim, maxTrimFraction, separator)
        sumAboveMedian, sumBelowMedian, sumRatio, channelsAboveMedian, channelsBelowMedian, channelRatio = \
            aboveBelow(avgSpectrumNansReplaced,medianTrue)
    else:
        if (meanSpectrumMethod.find('peakOver') < 0):
            print "Not adjusting sigmaFindContinuum, because groups=%d, channelRatio=%g, firstFreq=%g, nchan=%d" % (groups,channelRatio,firstFreq,nchan)
        else:
            print "Not adjusting sigmaFindContinuum because meanSpectrumMethod = ", meanSpectrumMethod

    selectedChannels = countChannels(selection)
    largestGroup = channelsInLargestGroup(selection)
    selections = len(selection.split(separator))
    if (selectedChannels > channelFractionForSlopeRemoval*nchan or 
        (largestGroup>nchan/3 and selections <= 2 and channelFractionForSlopeRemoval<1)):
        previousResult = continuumChannels,selection,threshold,median,groups,correctionFactor,medianTrue,mad,medianCorrectionFactor,negativeThreshold,lineStrengthFactor,singleChannelPeaksAboveSFC,allGroupsAboveSFC        
        # remove linear slope from mean spectrum and run it again
        index = channelSelectionRangesToIndexArray(selection)
        print "Fitting slope to %d channels (largestGroup=%d,nchan/3=%d,selectedChannels=%d)" % (len(index), largestGroup, nchan/3, selectedChannels)
        slope, intercept = linfit(index, avgSpectrumNansReplaced[index], MAD(avgSpectrumNansReplaced[index]))
        avgSpectrumNansReplaced -= np.array(range(len(avgSpectrumNansReplaced)))*slope
        print "Removing slope = ", slope
        if (factor >= 1.5):
            sigmaFindContinuum /= factor
            print "Restoring sigmaFindContinuum to ", sigmaFindContinuum

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
        drawYlabel(img, typeOfMeanSpectrum,meanSpectrumMethod, meanSpectrumThreshold,
                   peakFilterFWHM, fontsize)
    elif (edgesUsed == 0):
        # The upper edge is not used and can have an upward spike
        # so don't show it.
        print "Not showing final %d channels of %d" % (skipchan,\
               len(avgspectrumAboveThreshold))
        if (showAverageSpectrum):
            pl.plot(range(len(avgspectrum)), avgspectrum, 'k-')
            pl.ylabel('average spectrum')
            ylimTop = pl.ylim()
            ax1.ylim(ylim)
            pl.subplot(rows,cols,2)
        pl.plot(range(len(avgspectrumAboveThreshold)-skipchan), 
                avgspectrumAboveThreshold[:-skipchan], 'r-')
        drawYlabel(img, typeOfMeanSpectrum,meanSpectrumMethod, meanSpectrumThreshold, 
                   peakFilterFWHM, fontsize)
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
        drawYlabel(img, typeOfMeanSpectrum,meanSpectrumMethod, meanSpectrumThreshold, 
                   peakFilterFWHM, fontsize)
        
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

    if (fitsTable or img==''):
        img = meanSpectrumFile
    if (source==None):
        source = os.path.basename(img)
        if (not fitsTable):
            source = source.split('_')[0]
    if (titleText == ''):
        narrowString = pickNarrowString(narrow, len(avgSpectrumNansReplaced)) 
        trimString = pickTrimString(trimChannels, len(avgSpectrumNansReplaced), maxTrim)
        titleText = os.path.basename(img) + ' ' + transition
    ylim = pl.ylim()
    ylim = [ylim[0], ylim[1]+(ylim[1]-ylim[0])*0.2]
    xlim = [0,nchan-1]
    pl.xlim(xlim)
    titlesize = np.min([fontsize,int(np.floor(fontsize*100.0/len(titleText)))])
    if (spw != ''):
        label = '(Spw %s) Channels (%d)' % (str(spw), nchan)
    else:
        label = 'Channels (%d)' % (nchan)
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
    aggregateBandwidth = computeBandwidth(selection, channelWidth)
    if (channelWidth > 0):
        channelWidthString = ', channel width: %g kHz, BW: %g MHz, contBW: %g MHz' % (channelWidth*1e-3, spwBandwidth*1e-6, aggregateBandwidth*1000)
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
    i = 1
    if meanSpectrumMethod.find('mean')>=0:
        pl.text(0.5,0.99-i*inc,' baseline=(%s,%s), narrow=%s, sCube=%.1f, sigmaFC=%.1f, trim=%s' % (baselineModeA,baselineModeB,narrowString,sigmaCube,sigmaFindContinuum,trimString),transform=ax1.transAxes, ha='center',size=fontsize)
    else:
        pl.text(0.5,0.99-i*inc,' baselineModeB=%s, narrow=%s, sigmaFindContinuum=%.1f, trim=%s' % (baselineModeB,narrowString,sigmaFindContinuum,trimString),transform=ax1.transAxes, ha='center',size=fontsize)
    i += 1
    if (fullLegend):
        pl.text(0.5,0.99-i*inc,'rms=MAD*1.482: of baseline chans = %f, scaled by %.1f for all chans = %f'%(mad,correctionFactor,mad*correctionFactor), 
                transform=ax1.transAxes, ha='center',size=fontsize)
        i += 1
        pl.text(0.017,0.99-i*inc,'lineStrength factor: %.2f' % (lineStrengthFactor), transform=ax1.transAxes, ha='left', size=fontsize)
        pl.text(0.983,0.99-i*inc,'MAD*1.482: of points below upper dotted line = %f' % (madOfPointsBelowThreshold),
                transform=ax1.transAxes, ha='right', size=fontsize)
        i += 1
        pl.text(0.5,0.99-i*inc,'median: of %d baseline chans = %f, offset by %.1f*MAD for all chans = %f'%(nBaselineChannels, median,medianCorrectionFactor,medianTrue-median), 
                transform=ax1.transAxes, ha='center', size=fontsize)
        i += 1
        pl.text(0.5,0.99-i*inc,'chans>median: %d (sum=%.4f), chans<median: %d (sum=%.4f), ratio: %.2f (%.2f)'%(channelsAboveMedian,sumAboveMedian,channelsBelowMedian,sumBelowMedian,channelRatio,sumRatio),
                transform=ax1.transAxes, ha='center', size=fontsize-1)
    if (negativeThreshold != None):
        pl.text(0.5,0.99-i*inc,'mad: %.3f; thresholds: %.3f, %.3f (dotted); median: %.3f (solid), meanmin: %.3f (dashed)'%(mad, threshold,negativeThreshold,medianTrue,median), transform=ax1.transAxes, ha='center', size=fontsize-1)
    else:
        pl.text(0.5,0.99-i*inc,'mad: %.3f; threshold: %.3f (dotted); median: %.3f (solid), meanmin: %.3f (dashed)'%(mad,threshold,medianTrue,median), 
                transform=ax1.transAxes, ha='center', size=fontsize)
    i += 1
    areaString = 'maxTrimFraction=%g; found %d ranges; ' % (maxTrimFraction, len(channelSelections))
    if (centralArcsec == 'auto'):
        areaString += 'mean over area: (unknown)'
    elif (centralArcsec < 0):
        areaString += 'mean over area: whole field'
    else:
        areaString += 'mean over area: central box of radius %.1f arcsec' % (centralArcsec)
    pl.text(0.5,0.99-i*inc,areaString, transform=ax1.transAxes, ha='center', size=fontsize)
    finalLine = ''
    if (mask != ''):
        finalLine += 'mask=%s; ' % (os.path.basename(mask))
    if (slope != None):
        if discardSlopeResult:
            discarded = ' (result discarded)'
        else:
            discarded = ''
        finalLine += 'linear slope removed: %g %s' % (roundFigures(slope,3),discarded)
    i += 1
    pl.text(0.5, 0.99-i*inc, finalLine, transform=ax1.transAxes, ha='center', size=fontsize)
    i += 1
#    if (img != meanSpectrumFile):
#        a,b,c,d = atmosphereVariation(img, header, chanInfo, airmass=airmass, pwv=pwv)
#        finalLine = 'transmission: %f, %f  skyTemp: %f, %f' % (a,b,c,d)
#        pl.text(0.5, 0.99-i*inc, finalLine, transform=ax1.transAxes, ha='center', size=fontsize)
    # Write CVS version
    pl.text(1.03, -0.005-2*inc, ' '.join(version().split()[1:4]), size=8, 
            transform=ax1.transAxes, ha='right')
    # Write CASA version
    pl.text(-0.03, -0.005-2*inc, "CASA "+casadef.casa_version+" r"+casadef.subversion_revision, 
             size=8, transform=ax1.transAxes, ha='left')
    if (plotAtmosphere != ''):
        if (plotAtmosphere == 'tsky'):
            value = 'tsky'
        else:
            value = 'transmission'
        freqs, atm = CalcAtmTransmissionForImage(img, header, chanInfo, airmass, pwv, value=value)
        print "freqs: min=%g, max=%g n=%d" % (np.min(freqs),np.max(freqs), len(freqs))
        atmRange = 0.5  # how much of the y-axis should it take up
        yrange = ylim[1]-ylim[0]
#        print "before plot: ylim=%.6f,%.6f, mean atm=%f " % (ylim[0], ylim[1], np.mean(atm))
        if (value == 'transmission'):
            atmRescaled = ylim[0] + 0.3*yrange + atm*atmRange*yrange
            pl.text(1.015, 0.3, '0%', color='m', transform=ax1.transAxes, ha='left', va='center')
            pl.text(1.015, 0.8, '100%', color='m', transform=ax1.transAxes, ha='left', va='center')
            pl.text(1.03, 0.55, 'Atmospheric Transmission', color='m', ha='center', va='center', 
                    rotation=90, transform=ax1.transAxes, size=11)
        else:
            atmRescaled = ylim[0] + 0.3*yrange + atm*atmRange*yrange/300.
            pl.text(1.015, 0.3, '0', color='m', transform=ax1.transAxes, ha='left', va='center')
            pl.text(1.015, 0.8, '300', color='m', transform=ax1.transAxes, ha='left', va='center')
            pl.text(1.03, 0.55, 'Sky temperature (K)', color='m', ha='center', va='center', 
                    rotation=90, transform=ax1.transAxes, size=11)
        pl.text(1.06, 0.55, '(%.1f mm PWV, 1.5 airmass)'%pwv, color='m', ha='center', va='center', 
                rotation=90, transform=ax1.transAxes, size=11)
        pl.plot(freqs, atmRescaled, 'w.', ms=0)  # need this to finalize the ylim value
        ticks = 10
        ylim = pl.ylim()
#        print "after plot: ylim=%.6f,%.6f, mean atm=%f " % (ylim[0], ylim[1], np.mean(atm))
        yrange = ylim[1]-ylim[0]
        if (value == 'transmission'):
            atmRescaled = ylim[0] + 0.3*yrange + atm*atmRange*yrange
        else:
            atmRescaled = ylim[0] + 0.3*yrange + atm*atmRange*yrange/300.
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
        png += '.meanSpectrum.%s.%s.%s.%.1fsigma.narrow%s.trim%s%s.png' % (meanSpectrumMethod, baselineModeA, baselineModeB, sigmaFindContinuum, narrowString, trimString, transition)
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
    print "Wrote png = %s" % (png)
    donetime = timeUtilities.time()
    print "%.1f sec elapsed in runFindContinuum" % (donetime-startTime)
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
                      avgSpectrumNansReplaced, threshold, edgesUsed, 
                      nchan, nanmin, centralArcsec='auto', mask='', iteration=0):
    """
    Writes out the mean spectrum (and the key parameters used to create it), so that
    it can quickly be restored.  This allows the user to quickly experiment with 
    different parameters of findContinuumChannels applied to the same mean spectrum.
    """
    f = open(meanSpectrumFile, 'w')
    if (iteration == 0):
        f.write('#threshold edgesUsed nchan nanmin centralArcsec=%s %s\n' % (str(centralArcsec),mask))
    else:
        f.write('#threshold edgesUsed nchan nanmin centralArcsec=auto %s %s\n' % (str(centralArcsec),mask))
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
    nBaselineChannels: number of channels over which to compute standard deviation/MAD
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
    print "****** starting findContinuumChannels (slope=%g) ***********" % (slope)
    print "Using sigmaFindContinuum=%.2f, sigmaEffective=%.1f, percentile=%.0f for mode=%s, channels=%d/%d" % (sigmaFindContinuum, sigmaEffective, percentile, baselineMode, nBaselineChannels, len(spectrum))
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
#        print "allBaselineChannels = ", allBaselineChannels
        print "MAD of all baseline channels = ", mad
        print "MAD of original baseline channels = ", madOriginal
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
    if (abs(originalSpectrum[np.min(channels)] - np.min(originalSpectrum)) < abs(1e-10*np.min(originalSpectrum))):
        lastmin = np.min(channels)
        channels.remove(lastmin)
        removed = 1
        for c in range(np.min(channels),np.max(channels)):
            mydiff = abs(originalSpectrum[c] - np.min(originalSpectrum))
            mycrit = abs(1e-10*np.min(originalSpectrum))
            if (mydiff > mycrit):
                break
            channels.remove(c)
            removed += 1
        print "Removed %d channels on low channel edge that were at the minimum." % (removed)
    # Now come in from the upper side
    if (abs(originalSpectrum[np.max(channels)] - np.min(originalSpectrum)) < abs(1e-10*np.min(originalSpectrum))):
        lastmin = np.max(channels)
        channels.remove(lastmin)
        removed = 1
        for c in range(np.max(channels),np.min(channels)-1,-1):
            mydiff = abs(originalSpectrum[c] - np.min(originalSpectrum))
            mycrit = abs(1e-10*np.min(originalSpectrum))
            if (mydiff > mycrit):
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
                print "Not rejecting narrow groups since there is only %d group!" % (groups)
    print "Found %d continuum channels in %d groups: %s" % (len(channels), groups, selection)
    if True:
        channels = rejectNarrowInnerWindowsChannels(channels)
        selection = convertChannelListIntoSelection(channels)
        groups = len(selection.split(separator))
        print "Final: found %d continuum channels (sFC=%.2f) in %d groups: %s" % (len(channels), sigmaFindContinuum, groups, selection)
    return(channels, selection, threshold, median, groups, correctionFactor, 
           medianTrue, mad, computeMedianCorrectionFactor(baselineMode, percentile)*signalRatio,
           negativeThreshold, lineStrengthFactor, singleChannelPeaksAboveSFC, 
           allGroupsAboveSFC)

def rejectNarrowInnerWindowsChannels(channels):
    """
    If there are 3,4,5,6 or 7 groups of channels, then remove any inner window that is narrower than
    both edge windows.
    """
    mylists = splitListIntoContiguousLists(channels)
    groups = len(mylists)
    if (groups > 2 and groups < 8):
        channels = []
        lenFirstGroup = len(mylists[0])
        lenLastGroup = len(mylists[-1])
        channels += mylists[0]
        for group in range(1,groups):
            if (len(mylists[group]) >= lenFirstGroup or len(mylists[group]) >= lenLastGroup):
                channels += mylists[group]
    return(channels)

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
            if (len(mylists) == 1):
                # If there was only one list of 1 or 2 channels, then don't trim it away!
                channels += mylist[:1]
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

def avgOverCube(pixels, useAbsoluteValue=False, threshold=None, median=False, mask=''):
    """
    Computes the average spectrum across a multi-dimensional
    array read from an image cube, ignoring any NaN values.
    Inputs:
    threshold: value in Jy
    median: if True, compute the median instead of the mean
    mask: use pixels inside this spatial mask (i.e. with value==1 or True) to compute 
          the average (the spectral mask is taken as the union of all channels)
    If threshold is specified, then it only includes pixels
    with an intensity above that value.
    Note: This function assumes that the spectral axis is the final axis.
        If it is not, there is no single setting of the axis parameter that 
        can make it work.
    """
    if (useAbsoluteValue):
        pixels = np.abs(pixels)
    if (mask != ''):
        pixels[np.where(mask==False)] = np.nan
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

def submask(mask, region):
    """
    Takes a spectral mask array, and picks out a subcube defined by a blc,trc-defined region
    """
    mask = mask[region['blc'][0]:region['trc'][0]+1, region['blc'][1]:region['trc'][1]+1]
    return mask

def propagateMaskToAllChannels(mask, axis=3):
    """
    Takes a spectral mask array, and propagates every masked spatial pixel to 
    all spectral channels of the array.
    """
    newmask = np.sum(mask,axis=axis)
    newmask[np.where(newmask>0)] = 1
    return(newmask)

def meanSpectrum(img='g35.03_KDnh3_11.hline.self.image', nBaselineChannels=16,
                 sigmaCube=3, verbose=False, nanBufferChannels=2, useAbsoluteValue=False,
                 baselineMode='edge', percentile=20, continuumThreshold=None,
                 meanSpectrumFile='', centralArcsec=-1, imageInfo=[], chanInfo=[], mask='',
                 meanSpectrumMethod='peakOverRms', peakFilterFWHM=15, iteration=0):
    """
    Computes the average spectrum across a CASA image cube, via the specified method.
    Inputs:
    nBaselineChannels: number of channels to use as the baseline in 
                       the 'meanAboveThreshold' methods.
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
    centralArcsec: default=-1 means whole field, or a floating point value in arcsec
    meanSpectrumMethod: 'peakOverMad', 'peakOverRms', 'meanAboveThreshold', 
       'meanAboveThresholdOverRms', 'meanAboveThresholdOverMad', 
    * 'meanAboveThreshold': uses a selection of baseline channels to compute the 
        rms to be used as a threshold value (similar to constructing a moment map).
    * 'meanAboveThresholdOverRms/Mad': scale spectrum by RMS or MAD        
    * 'peakOverRms/Mad' computes the ratio of the peak in a spatially-smoothed 
        version of cube to the RMS or MAD.  Smoothing is set by peakFilterFWHM.
    peakFilterFWHM: value used by 'peakOverRms' and 'peakOverMad' to presmooth 
        each plane with a Gaussian kernel of this width (in pixels)
        Set to 1 to not do any smoothing.
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
    if (mask != ''):
        myia.open(mask)
        usermaskdata = myia.getregion()
        if (verbose): print "shape(usermask) = ", np.array(np.shape(usermaskdata))
        maskAxis = findSpectralAxis(myia)
        if (np.shape(usermaskdata)[maskAxis] > 1):
            singlePlaneUserMask = False
        else:
            singlePlaneUserMask = True
            if (meanSpectrumMethod.find('meanAboveThreshold') >= 0):
                print "single plane user masks not supported by meanSpectrumMethod='meanAboveThreshold', try peakOverMad."
                return
        myia.close()
    myia.open(img)
    axis = findSpectralAxis(myia)
    if verbose: print "Found spectral axis = ", axis
    if (centralArcsec < 0 or centralArcsec == 'auto'):
        centralArcsec = -1
        pixels = myia.getregion()
        maskdata = myia.getregion(getmask=True)
    else:
        myrg = createCasaTool(rgtool)
        bmaj, bmin, bpa, cdelt1, cdelt2, naxis1, naxis2, freq = imageInfo
        nchan = chanInfo[0] # numberOfChannelsInCube(img)
        x0 = int(np.round(naxis1*0.5 - centralArcsec*0.5/np.abs(cdelt1)))
        x1 = int(np.round(naxis1*0.5 + centralArcsec*0.5/np.abs(cdelt1)))
        y0 = int(np.round(naxis2*0.5 - centralArcsec*0.5/cdelt2))
        y1 = int(np.round(naxis2*0.5 + centralArcsec*0.5/cdelt2))
        blc = [x0,y0,0,0]
        trc = [x1,y1,0,0]
        trc[axis] = nchan
        region = myrg.box(blc=blc, trc=trc)
        pixels = myia.getregion(region=region)
        print "Taking submask for central area of image: blc=%s, trc=%s" % (str(blc),str(trc))
        maskdata = myia.getregion(region=region,getmask=True)
        myrg.done()
        if (mask != ''):
            usermaskdata = submask(usermaskdata, region)
    if verbose:
        print "shape of pixels = ", np.array(np.shape(pixels))
    if mask != '':
        if not (np.array(np.shape(pixels)) == np.array(np.shape(usermaskdata))).all():
            print "Mismatch in shape between image (%s) and mask (%s)" % (np.shape(pixels),np.shape(usermaskdata))
            return
    if (meanSpectrumMethod.find('OverRms') > 0 or meanSpectrumMethod.find('OverMad') > 0):
        # compute myrms, ignoring masked values
        if (meanSpectrumMethod.find('OverMad') < 0):
            print "Computing std on each plane"
        else:
            print "Computing mad on each plane"
        myvalue = []
        for a in range(np.shape(pixels)[axis]):
            if verbose:
                if ((a+1)%100 == 0): 
                    print "Done %d/%d" % (a+1, np.shape(pixels)[axis])
            # Extract this one channel
            if (axis == 2):
                mypixels = pixels[:,:,a,0]
                mymask = maskdata[:,:,a,0]
                if (mask != ''):
                    if (singlePlaneUserMask):
                        myusermask = usermaskdata[:,:,0,0]
                    else:
                        myusermask = usermaskdata[:,:,a,0]
            elif (axis == 3):
                mypixels = pixels[:,:,0,a]
                mymask = maskdata[:,:,0,a]
                if (mask != ''):
                    if (singlePlaneUserMask):
                        myusermask = usermaskdata[:,:,0,0]
                    else:
                        myusermask = usermaskdata[:,:,0,a]
            if (mask != ''):
                # user mask is typically a clean mask, so we want to use the region outside the
                # clean mask for computing the MAD, but also avoiding the masked edges of the image,
                # which are generally masked to False
                pixelsForStd = mypixels[np.where((myusermask<1) * (mymask==True))]
            else: 
                # avoid the masked (typically outer) edges of the image using the built-in mask
                pixelsForStd = mypixels[np.where(mymask==True)]
            if (meanSpectrumMethod.find('OverMad') < 0):
                myvalue.append(np.std(pixelsForStd))
            else:
                myvalue.append(MAD(pixelsForStd))
#            print "channel %4d: Using %d of %d pixels for MAD/std" % (a,len(pixelsForStd),len(mypixels.flatten()))
        if (meanSpectrumMethod.find('OverMad') < 0):
            myrms = np.array(myvalue)
        else:
            mymad = np.array(myvalue)
        print "Finished"
    if (meanSpectrumMethod.find('peakOver') == 0):
        # compute mymax, then divide by either myrms or mymad
        gaussianSigma = peakFilterFWHM/2.355
        myvalue = []
        print "Smoothing and computing peak on each plane."
        for a in range(np.shape(pixels)[axis]):
            if verbose:
                if ((a+1)%100 == 0): 
                    print "Done %d/%d" % (a+1, np.shape(pixels)[axis])
            if (axis == 2):
                if (gaussianSigma > 1.1/2.355):
                    myvalue.append(np.max(gaussian_filter(pixels[:,:,a,0],sigma=gaussianSigma)))
                else:
                    myvalue.append(np.max(pixels[:,:,a,0]))
            elif (axis == 3):
                if (gaussianSigma > 1.1/2.355):
                    myvalue.append(np.max(gaussian_filter(pixels[:,:,0,a],sigma=gaussianSigma)))
                else:
                    myvalue.append(np.max(pixels[:,:,0,a]))
        print "finished"
        mymax = np.array(myvalue)
        if (meanSpectrumMethod == 'peakOverRms'):
            avgspectrum = mymax/myrms
        elif (meanSpectrumMethod == 'peakOverMad'):
            avgspectrum = mymax/mymad
        nansRemoved = removeNaNs(avgspectrum, verbose=True)
        threshold = 0
        edgesUsed = 0
        nansReplaced,nanmin = removeNaNs(avgspectrum, replaceWithMin=True, 
                                         nanBufferChannels=nanBufferChannels, verbose=True)
    elif (meanSpectrumMethod.find('meanAboveThreshold') == 0):
        if (continuumThreshold != None):
            belowThreshold = np.where(pixels < continuumThreshold)
            if verbose:
                print "shape of belowThreshold = ", np.shape(belowThreshold)
            pixels[belowThreshold] = 0.0
        if (mask != ''):
            pixelsWithinUserMask = len(np.where(usermaskdata<1)[0])
            pixelsWithinCubeMask = len(np.where(maskdata==1)[0])
            pixelsForMAD = pixels[np.where((maskdata==1) * (usermaskdata<1))]
            print "Using %d of %d pixels for MAD (%d outside user mask, %d satisfy cube mask)" % (len(pixelsForMAD),len(pixels.flatten()), pixelsWithinUserMask, pixelsWithinCubeMask)
        else:
            pixelsForMAD = pixels[np.where(maskdata==1)]  # ignore the outer mask edges of the cube
            print "Using %d of %d pixels for MAD" % (len(pixelsForMAD),len(pixels.flatten()))
#            pixelsForMAD = pixels  # previous method
        std = MAD(pixelsForMAD, axis=None)
        if verbose: print "MAD of cube = ", std
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
            # Drop all floating point zeros and internally-masked pixels from calculation
            if (mask == ''):
                allPixels = allPixels[np.where((allPixels != 0) * (maskdata==True))]
            else:
                # avoid identical zeros and clean mask when looking for lowest pixels
                allPixels = allPixels[np.where((allPixels != 0) * (maskdata==True) * (usermaskdata<1))]
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
        if (mask != ''):
            print "Propagating mask to all channels"
            maskdata = propagateMaskToAllChannels(maskdata, axis)
        else:
            maskdata = ''
        avgspectrum = avgOverCube(pixels, useAbsoluteValue, mask=maskdata)
        if meanSpectrumMethod.find('OverRms') > 0:
            avgspectrum /= myrms
        elif meanSpectrumMethod.find('OverMad') > 0:
            avgspectrum /= mymad
        threshold = median + sigmaCube*std
        if verbose: 
            print "Using threshold above which to compute mean spectrum = ", threshold
        pixels[np.where(pixels < threshold)] = 0.0
        avgspectrumAboveThreshold = avgOverCube(pixels, useAbsoluteValue, threshold, mask=maskdata)
        if meanSpectrumMethod.find('OverRms') > 0:
            avgspectrumAboveThreshold /= myrms
        elif meanSpectrumMethod.find('OverMad') > 0:
            avgspectrumAboveThreshold /= mymad
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
                      edgesUsed, nchan, nanmin, centralArcsec, mask, iteration)
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
        idx2 = np.isinf(a)
        print "Found %d inf channels" % (len(np.where(idx2==True)[0]))
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
            print "Replaced %d infs" % (len(idx2))
        if (replaceWithMin):
            a[idx] = a_nanmin
            a[idx2] = a_nanmin
            return(a, a_nanmin)
        elif (replaceWithZero):
            a[idx] = 0
            a[idx2] = 0
            return(a, 0)
    else:
        a = a[np.where(np.isnan(a) == False)]
        if (verbose):
            print "Removed %d NaNs" % (startLength-len(a))
        startLength = len(a)
        a = a[np.where(np.isinf(a) == False)]
        if (verbose):
            print "Removed %d infs" % (startLength-len(a))
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
    Returns:
    2 arrays: frequencies and values
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
    Convert a channel selection range string to integer array of indices.
    e.g.:  '3~8;10~11' -> [3,4,5,6,7,8,10,11]
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

def grep(filename, arg):
    """
    Runs grep in a subprocess.
    -Todd Hunter
    """
    process = subprocess.Popen(['grep', '-n', arg, filename], stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    return stdout, stderr

    
def widthOfMaskArcsec(mask):
    """
    Finds width of the smallest central square that circumscribes all masked pixels.
    Returns the value in arcsec.
    """
    myia = createCasaTool(iatool)
    myia.open(mask)
    pixels = myia.getregion()
    myia.close()
    idx = np.where(pixels==True)
    leftRadius = np.shape(pixels)[0]/2 - np.min(idx[0])
    rightRadius = np.max(idx[0]) - np.shape(pixels)[0]/2
    width = 2*np.max(np.abs([leftRadius,rightRadius]))
    topRadius = np.max(idx[1]) - np.shape(pixels)[1]/2
    bottomRadius = np.shape(pixels)[1]/2 - np.min(idx[1]) 
    height = 2*np.max(np.abs([topRadius,bottomRadius]))
    cdelt1 = imhead(mask,mode='get',hdkey='cdelt1')
    if (cdelt1['unit'] == 'rad'):
        cdelt1 = np.degrees(cdelt1['value'])*3600.
    elif (cdelt1['unit'] == 'deg'):
        cdelt1 *= 3600.
    else:
        print "Unrecognized angular unit: ", cdelt1['unit']
        return
    width = np.abs(cdelt1)*(np.max([width,height])+1)
    return width
