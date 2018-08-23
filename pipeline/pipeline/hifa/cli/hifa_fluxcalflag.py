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
import task_hifa_fluxcalflag
def hifa_fluxcalflag(vis=[''], field='', intent='', spw='', pipelinemode='automatic', threshold=0.75, appendlines=False, linesfile='', applyflags=True, dryrun=False, acceptresults=True):

        """Locate and flag line regions in solar system flux calibrators

Fills the model column with the model visibilities.

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline parameter arguments which can be set in any pipeline mode

threshold -- If the fraction of an spw occupied by line regions is greater
   then threshold flag the entire spectral window.

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of MeasurementSets
    defined in the pipeline context.
    default: []
    example:

field -- The list of field names or field ids for which the models are
    to be set. Defaults to all fields with intent 'AMPLITUDE'.
    default: ''
    example: '3C279', '3C279, M82'

intent -- A string containing a comma delimited list of intents against
    which the the selected fields are matched.  Defaults to all data
    with amplitude intent.
    default: ''
    example: 'AMPLITUDE'

spw -- The list of spectral windows and channels for which bandpasses are
    computed. Defaults to all science spectral windows.
    default: ''
    example: '11,13,15,17'

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
       do not execute (False).
       default: True

acceptresults -- Add the results of the task to the pipeline context (True) or
       reject them (False).
       default: True

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.

Description

Search the bultin solar system flux calibrater line catalog for overlaps with the
science spectral windows.  Generate a list of line overlap regions and flagging
commands.

Issues


Examples

1. Locate known lines in any solar system object flux calibrators. 

    hifa_fluxcalflag()


        """
        if type(vis)==str: vis=[vis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['field'] = field
        mytmp['intent'] = intent
        mytmp['spw'] = spw
        mytmp['pipelinemode'] = pipelinemode
        mytmp['threshold'] = threshold
        mytmp['appendlines'] = appendlines
        mytmp['linesfile'] = linesfile
        mytmp['applyflags'] = applyflags
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.utils().torecord(pathname+'hifa_fluxcalflag.xml')

        casalog.origin('hifa_fluxcalflag')
        if trec.has_key('hifa_fluxcalflag') and casac.utils().verify(mytmp, trec['hifa_fluxcalflag']) :
	    result = task_hifa_fluxcalflag.hifa_fluxcalflag(vis, field, intent, spw, pipelinemode, threshold, appendlines, linesfile, applyflags, dryrun, acceptresults)

	else :
	  result = False
        return result
