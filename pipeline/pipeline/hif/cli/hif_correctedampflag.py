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
import task_hif_correctedampflag
def hif_correctedampflag(vis=[''], intent='', field='', spw='', antnegsig=4.0, antpossig=4.6, tmantint=0.063, tmint=0.085, tmbl=0.175, antblnegsig=3.4, antblpossig=3.2, relaxed_factor=2.0, niter=2, pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Flag corrected - model amplitudes based on calibrators.

Keyword arguments:

--- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of MeasurementSets
    specified in the pipeline context.
    default: ''
    example: ['M51.ms']

field -- The list of field names or field ids for which bandpasses are
    computed. If undefined (default), it will select all fields.
    default: ''
    example: '3C279', '3C279, M82'

intent -- A string containing a comma delimited list of intents against
    which the selected fields are matched. If undefined (default), it
    will select all data with the BANDPASS intent.
    default: ''
    example: '*PHASE*'

spw -- The list of spectral windows and channels for which bandpasses are
    computed. If undefined (default), it will select all science spectral
    windows.
    default: ''
    example: '11,13,15,17'

antnegsig -- Lower sigma threshold for identifying outliers as a result of bad
    antennas within individual timestamps.
    default: 4.0

antpossig -- Upper sigma threshold for identifying outliers as a result of bad
    antennas within individual timestamps.
    default: 4.6

tmantint -- Threshold for maximum fraction of timestamps that are allowed to
    contain outliers.
    default: 0.063

tmint -- Initial threshold for maximum fraction of "outlier timestamps" over
    "total timestamps" that a baseline may be a part of.
    default: 0.085

tmbl -- Initial threshold for maximum fraction of "bad baselines" over "all
    timestamps" that an antenna may be a part of.
    default: 0.175

antblnegsig -- Lower sigma threshold for identifying outliers as a result of
    "bad baselines" and/or "bad antennas" within baselines, across all
    timestamps.
    default: 3.4

antblpossig -- Upper sigma threshold for identifying outliers as a result of
    "bad baselines" and/or "bad antennas" within baselines, across all
    timestamps.
    default: 3.2

relaxed_factor -- Relaxed value to set the threshold scaling factor to under
    certain conditions (see task description).
    default: 2.0

niter -- Maximum number of times to iterate on evaluation of flagging
    heuristics. If an iteration results in no new flags, then subsequent
    iterations are skipped.
    default: 2

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

1. run default flagging on bandpass calibrator with recommended settings:

    hif_correctedampflag()


        """
        if type(vis)==str: vis=[vis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['intent'] = intent
        mytmp['field'] = field
        mytmp['spw'] = spw
        mytmp['antnegsig'] = antnegsig
        mytmp['antpossig'] = antpossig
        mytmp['tmantint'] = tmantint
        mytmp['tmint'] = tmint
        mytmp['tmbl'] = tmbl
        mytmp['antblnegsig'] = antblnegsig
        mytmp['antblpossig'] = antblpossig
        mytmp['relaxed_factor'] = relaxed_factor
        mytmp['niter'] = niter
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_correctedampflag.xml')

        casalog.origin('hif_correctedampflag')
        if trec.has_key('hif_correctedampflag') and casac.utils().verify(mytmp, trec['hif_correctedampflag']) :
	    result = task_hif_correctedampflag.hif_correctedampflag(vis, intent, field, spw, antnegsig, antpossig, tmantint, tmint, tmbl, antblnegsig, antblpossig, relaxed_factor, niter, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
