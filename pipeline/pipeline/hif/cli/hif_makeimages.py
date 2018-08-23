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
import task_hif_makeimages
def hif_makeimages(vis=[''], target_list={}, hm_masking='auto', hm_sidelobethreshold=-999.0, hm_noisethreshold=-999.0, hm_lownoisethreshold=-999.0, hm_negativethreshold=-999.0, hm_minbeamfrac=-999.0, hm_growiterations=-999, hm_dogrowprune=True, hm_minpercentchange=-999.0, hm_cleaning='', tlimit=2.0, masklimit=4, maxncleans=1, cleancontranges=False, calcsb=False, parallel='automatic', pipelinemode='automatic', dryrun=False, acceptresults=True):

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
    MeasurementSets specified in the h_init or hif_importdata sets.
    example: vis='ngc5921.ms'
             vis=['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']
    default: use all MeasurementSets in the context 

hm_masking -- Clean masking mode. Options are 'centralregion',
    'psf', 'psfiter', 'auto', 'manual' and 'none'
    default: 'auto'
    example: 'manual'

hm_sidelobethreshold -- sidelobethreshold * the max sidelobe level

hm_noisethreshold -- noisethreshold * rms in residual image

hm_lownoisethreshold -- lownoisethreshold * rms in residual image

hm_negativethreshold -- negativethreshold * rms in residual image

hm_minbeamfrac -- negativethreshold * rms in residual image

hm_growiterations -- Number of binary dilation iterations for growing the mask

hm_dogrowprune -- Do pruning on the grow mask

hm_minpercentchange -- Mask size change threshold

cleancontranges -- Clean continuum frequency ranges in cubes
                   default=False
                   Options: False, True

calcsb -- Force (re-)calculation of sensitivities and beams
            default=False
            Options: False, True

parallel -- use multiple CPU nodes to clean images
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


        """
        if type(vis)==str: vis=[vis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['target_list'] = target_list
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
	trec = casac.utils().torecord(pathname+'hif_makeimages.xml')

        casalog.origin('hif_makeimages')
        if trec.has_key('hif_makeimages') and casac.utils().verify(mytmp, trec['hif_makeimages']) :
	    result = task_hif_makeimages.hif_makeimages(vis, target_list, hm_masking, hm_sidelobethreshold, hm_noisethreshold, hm_lownoisethreshold, hm_negativethreshold, hm_minbeamfrac, hm_growiterations, hm_dogrowprune, hm_minpercentchange, hm_cleaning, tlimit, masklimit, maxncleans, cleancontranges, calcsb, parallel, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
