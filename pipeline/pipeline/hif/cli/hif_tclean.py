#
# This file was generated using xslt from its XML file
#
# Copyright 2009, Associated Universities Inc., Washington DC
#
import sys
import os
from  casac import *
import string
from taskinit import casalog
from taskinit import xmlpath
#from taskmanager import tm
import task_hif_tclean
def hif_tclean(vis=[''], imagename='', intent='', field='', spw='', spwsel_lsrk={}, spwsel_topo={}, uvrange='', specmode='', gridder='', deconvolver='', nterms=2, outframe='', imsize=[], cell=[''], phasecenter='', stokes='I', nchan=-1, start='', width='', nbin=-1, restoringbeam=[''], hm_masking='auto', hm_sidelobethreshold=-999.0, hm_noisethreshold=-999.0, hm_lownoisethreshold=-999.0, hm_negativethreshold=-999.0, hm_minbeamfrac=-999.0, hm_growiterations=-999, hm_dogrowprune=True, hm_minpercentchange=-999.0, hm_cleaning='rms', mask=[], niter=5000, threshold='0.0mJy', tlimit=2.0, masklimit=4, maxncleans=1, cleancontranges=False, calcsb=False, parallel='automatic', pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Compute clean map

Compute a cleaned image for a particular target source/intent and spectral
window.

Keyword arguments:

--- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
       determines the values of all context defined pipeline inputs
       automatically.  In interactive mode the user can set the pipeline
       context defined parameters manually.  In 'getinputs' mode the user
       can check the settings of all pipeline parameters without running
       the task.
       default: 'automatic'.


---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of
    MeasurementSets in the context.
    default: ''
    example: vis=['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']

imagename -- Prefix of output images. Defaults to one of the following options
   depending on the availability of project information. 
   '{ousstatus uid}.{field}.[{intent}.]s{stage number}.spw{spw}'
   'multivis.{field}.[{intent}.]s{stage number}.spw{spw}'

   cleanboxes and thresholds to use as it goes. For each iteration the
   output images are:
      {prename}.iter{n}.image; cleaned and restored image
      {prename}.iter{n}.psf; point spread function (dirty beam)
      {prename}.iter{n}.flux; relative sky sensitivity over field
      {prename}.iter{n}.flux.pbcoverage; relative pb coverage over field
                                        (only for mosaics)
      {prename}.iter{n}.model; image of clean components
      {prename}.iter{n}.residual; image of residuals
      {prename}.iter{n}.cleanmask; image of cleanmask used
   default: ''
   example: 'test1'
  
intent -- An intent against which the selected fields are matched. Default
    means select all data from fields specified by 'field' paramete
    default: ''
    example: '', 'TARGET'

field -- Fields id(s) or name(s) to image or mosaic. Must be set.
    default:  
    example: '3C279', 'Centaurus*'

spw -- Spectral window/channels to image. \'\' for all science data.
    default: ''
    example: '9', '9,11'

spwsel_lsrk -- Spectral window LSRK frequency selection for continuum. {} for all science data.
    default: {}
    example: {'spw16': '89.1~89.5GHz;90.2~90.3GHz', 'spw18': '101.2~102.1GHz'}

spwsel_topo -- Per MS spectral window TOPO frequency selection for continuum. [] for all science data.
    default: {}
    example: {['spw16:89.1~89.5GHz;90.2~90.3GHz']}

specmode -- Frequency imaging mode, 'mfs', 'cont', 'cube'. \'\' defaults to
    'cube' if intent parameter includes 'TARGET' otherwise 'mfs'.
    default: ''
    example: 'mfs', 'cont', 'cube'

gridder -- Gridding options, 'standard', 'mosaic'.
   Derived as follows:
      1. The 'field' parameter is converted into a list of field_ids for
         each MeasurementSet in 'vis'.
      2. If there is more than 1 field_id in the list for any MeasurementSet
         then gridder is set to 'mosaic', otherwise it will be set
         to 'standard'. 
    default: ''
    'standard'

deconvolver -- Minor cycle algorithm e.g. hogbom or clark clean. \'\' defaults to
    'hogbom'

outframe -- The reference frame of the output image. The only supported option
    is 'LSRK'
    default: ''
    example: 'LSRK'

imsize -- X and Y image size in pixels).  Must be even and contain factors
    2,3,5,7 only.
    Default derived as follows:
      1. Determine 'phasecenter' value and spread of field centres around it. 
      2. Set size of image to cover spread of field centres plus a border of
         width 0.75 * beam radius (to first null).
      3. Divide x and y extents by 'cell' values to arrive at the numbers of
         pixels required. 
    default: ''
    example: [320,320]

cell -- X and Y cell size. Derived from maximum UV spacing. Details TBD
    default ''
    example: ['0.5arcsec', '0.5arcsec']

phasecenter -- Direction measure or field id for the mosaic center.
   Default derived as follows:
       1. Make an array containing all the field centers to be imaged together.
       2. Derive the mean direction from the directions array.
    default: \'\'
    example: 2

stokes --

nchan -- Number of channels or planes in the output image, -1 for all
    default: -1
    example: 128

width -- Width of spectral dimension in frequency, \'\' for default.
    default: \'\'
    example: '7.8125MHz'

nbin -- Channel binning factor
    default: -1
    example: 2

restoringbeam -- Gaussian restoring beam for clean, \'\' for default
    default: \'\'
    example:

hm_masking -- Clean masking mode. Options are 'centralregion',
    'psf', 'psfiter', 'auto', 'manual' and 'none'
    default: 'auto'
    example: 'manual'

hm_sidelobethreshold -- Auto-boxing sidelobetheshold.

hm_noisethreshold -- Auto-boxing noisethreshold.

hm_lownoisethreshold -- Auto-boxing lownoisethreshold.

hm_negativethreshold -- Auto-boxing negativethreshold.

hm_minbeamfrac -- Auto-boxing minbeamfrac.

hm_growiterations -- number of binary dilation iterations for growing the mask.

hm_dogrowprune -- Do pruning on the grow mask

hm_minpercentchange -- Mask size change threshold

mask -- Image mask for hm_masking manual mode. User responsible for
    matching image sizes, coordinates, etc.
    default: \'\'
    example: 'mymask.mask'

niter -- Maximum number of iterations per clean call
    default: 5000
    example: 500

threshold -- Threshold for cleaning
    default: '0.0'
    example: '0.05'

maxncleans -- Maximum number of clean calls
    default: 1
    example: 10

cleancontranges -- Clean continuum frequency ranges in cubes
                   default=False
                   Options: False, True

calcsb -- Force (re-)calculation of sensitivities and beams
            default=False
            Options: False, True

parallel -- use MPI cluster to clean images
    default: \'automatic\'

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: False

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.


Examples:

Make an 'mfs' image of calibrator 3c279 using data in spectral window 1. 
The cell size is set to 0.2 arcsec in RA and Dec. Other clean parameters
are derived from heuristics:

hif_tclean(field='3c279', cell='0.2arcsec', spw='1', specmode='mfs')

Make a cube of calibrator 3c279 using data in spectral window 1. The
cube planes will be evenly spaced in frequency in the LSRK frame. Other
clean parameters are derived from heuristics.
 
hif_tclean(field='3c279', cell='0.2arcsec', spw='1', specmode='cube',
 outframe='LSRK')


        """
        if type(vis)==str: vis=[vis]
        if type(imsize)==int: imsize=[imsize]
        if type(cell)==str: cell=[cell]
        if type(restoringbeam)==str: restoringbeam=[restoringbeam]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['imagename'] = imagename
        mytmp['intent'] = intent
        mytmp['field'] = field
        mytmp['spw'] = spw
        mytmp['spwsel_lsrk'] = spwsel_lsrk
        mytmp['spwsel_topo'] = spwsel_topo
        mytmp['uvrange'] = uvrange
        mytmp['specmode'] = specmode
        mytmp['gridder'] = gridder
        mytmp['deconvolver'] = deconvolver
        mytmp['nterms'] = nterms
        mytmp['outframe'] = outframe
        mytmp['imsize'] = imsize
        mytmp['cell'] = cell
        mytmp['phasecenter'] = phasecenter
        mytmp['stokes'] = stokes
        mytmp['nchan'] = nchan
        mytmp['start'] = start
        mytmp['width'] = width
        mytmp['nbin'] = nbin
        mytmp['restoringbeam'] = restoringbeam
        mytmp['hm_masking'] = hm_masking
        mytmp['hm_sidelobethreshold'] = hm_sidelobethreshold
        mytmp['hm_noisethreshold'] = hm_noisethreshold
        mytmp['hm_lownoisethreshold'] = hm_lownoisethreshold
        mytmp['hm_negativethreshold'] = hm_negativethreshold
        mytmp['hm_minbeamfrac'] = hm_minbeamfrac
        mytmp['hm_growiterations'] = hm_growiterations
        mytmp['hm_dogrowprune'] = hm_dogrowprune
        mytmp['hm_minpercentchange'] = hm_minpercentchange
        mytmp['hm_cleaning'] = hm_cleaning
        mytmp['mask'] = mask
        mytmp['niter'] = niter
        if type(threshold) == str :
           mytmp['threshold'] = casac.quanta().quantity(threshold)
        else :
           mytmp['threshold'] = threshold
        mytmp['tlimit'] = tlimit
        mytmp['masklimit'] = masklimit
        mytmp['maxncleans'] = maxncleans
        mytmp['cleancontranges'] = cleancontranges
        mytmp['calcsb'] = calcsb
        mytmp['parallel'] = parallel
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_tclean.xml')

        casalog.origin('hif_tclean')
        if trec.has_key('hif_tclean') and casac.utils().verify(mytmp, trec['hif_tclean']) :
	    result = task_hif_tclean.hif_tclean(vis, imagename, intent, field, spw, spwsel_lsrk, spwsel_topo, uvrange, specmode, gridder, deconvolver, nterms, outframe, imsize, cell, phasecenter, stokes, nchan, start, width, nbin, restoringbeam, hm_masking, hm_sidelobethreshold, hm_noisethreshold, hm_lownoisethreshold, hm_negativethreshold, hm_minbeamfrac, hm_growiterations, hm_dogrowprune, hm_minpercentchange, hm_cleaning, mask, niter, threshold, tlimit, masklimit, maxncleans, cleancontranges, calcsb, parallel, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
