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
import task_hpc_hif_refant
def hpc_hif_refant(vis=[''], field='', intent='AMPLITUDE,BANDPASS,PHASE', spw='', hm_refant='automatic', refant='', refantignore='', geometry=True, flagging=True, pipelinemode='automatic', dryrun=False, acceptresults=True, parallel='automatic'):

        """Select the best reference antennas
The hif_session_refant task selects a list of reference antennas and outputs them
in priority order. The priority order is determined by a weighted combination
of scores derived by the antenna selection heuristics.


Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode


pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
determines the values of all context defined pipeline inputs automatically.
In interactive mode the user can set the pipeline context defined parameters
manually. In 'getinputs' mode the user can check the settings of all
pipeline parameters without running the task.
default: 'automatic'.

hm_refant -- The heuristics method or mode for selection the reference
antenna. The options are 'manual' and 'automatic. In manual
mode a user supplied referenence antenna refant is supplied.
In 'automatic' mode the antennas are selected automatically.
default: 'automatic'

refant -- The user supplied reference antenna for 'manual' mode. If
no antenna list is supplied an empty list is returned.
default: ''
example: 'DV05'

geometry -- Score antenna by proximity to the center of the array. This
option is quick as only the ANTENNA table must be read.
default: True

flagging -- Score antennas by percentage of unflagged data. This option
requires computing flagging statistics.
default: True


---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of
MeasurementSets in the pipeline context.
default: ''
example: ['M31.ms']

field -- The comma delimited list of field names or field ids for which flagging
scores are computed if hm_refant='automatic' and flagging = True
default: '' (Default to fields with the specified intents)
example: '3C279', '3C279,M82'

intent -- A string containing a comma delimited list of intents against
which the the selected fields are matched. Defaults to all supported
intents.
default: 'AMPLI,BANDPASS,PHASE'
example: 'BANDPASS'

spw -- A string containing the comma delimited list of spectral window ids for
which flagging scores are computed if hm_refant='automatic' and flagging = True.
default: '' (all spws observed with the specified intents)
example: '11,13,15,17'

refantignore -- string list to be ignored as reference antennas.
default: ''
Example: refantignore='ea02,ea03'


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

The hif_refant task selects a list of reference antennas and stores them
in the pipeline context in priority order.

The priority order is determined by a weighted combination of scores derived
by the antenna selection heuristics. In manual mode the reference antennas
can be set by hand.


Issues

Examples

1. Compute the references antennas to be used for bandpass and gain calibration.

hpc_hif_refant()
        
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
        mytmp['hm_refant'] = hm_refant
        mytmp['refant'] = refant
        mytmp['refantignore'] = refantignore
        mytmp['geometry'] = geometry
        mytmp['flagging'] = flagging
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
        mytmp['parallel'] = parallel
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hpc_hif_refant.xml')

        casalog.origin('hpc_hif_refant')
        if trec.has_key('hpc_hif_refant') and casac.utils().verify(mytmp, trec['hpc_hif_refant']) :
	    result = task_hpc_hif_refant.hpc_hif_refant(vis, field, intent, spw, hm_refant, refant, refantignore, geometry, flagging, pipelinemode, dryrun, acceptresults, parallel)

	else :
	  result = False
        return result
