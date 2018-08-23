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
import task_hif_setmodels
def hif_setmodels(vis='', reference='', refintent='AMPLITUDE', transfer='', transintent='BANDPASS', reffile='', normfluxes=True, scalebychan=True, pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Set calibrator source models

Derive flux densities for point source transfer calibrators using flux models
for reference calibrators.

---- pipeline parameter arguments which can be set in any pipeline mode
pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
    determines the values of all context defined pipeline inputs automatically.
    In interactive mode the user can set the pipeline context defined
    parameters manually.  In 'getinputs' mode the users can check the settings
    of all pipeline parameters without running the task.
    default: 'automatic'.

reffile -- The reference file containing a lookup table of point source models
    This file currently defaults to 'flux.csv' in the working directory. This
    file must conform to the standard pipeline 'flux.csv' format
    default: ''
    example: 'myfluxes.csv'

normfluxes --  Normalize the transfer field lookup fluxes to 1.0. The reference
    field fluxes are never normalized.
    default: True
    example: False

scalebychan -- Scale the flux density on a per channel basis or else on a per spw basis
    default: True
    example: False


---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of
    MeasurementSets specified in the pipeline context
    default: ''
    example: ['M32A.ms', 'M32B.ms']

reference -- A string containing a comma delimited list of  field names
    defining the reference calibrators. Defaults to field names with
    intent 'AMPLITUDE'.
    default: ''
    example: 'M82,3C273'

refintent -- A string containing a comma delimited list of intents 
    used to select the reference calibrators. Defaults to 'AMPLITUDE'.
    default: 'AMPLITUDE'. '' Means no sources.
    example: 'BANDPASS'

transfer -- A string containing a comma delimited list of  field names
    defining the transfer calibrators. Defaults to field names with
    intent ''.
    default: 'BANDPASS,PHASE,CHECK'
    example: 'J1328+041,J1206+30'

transintent -- A string containing a comma delimited list of intents 
    defining the transfer calibrators. Defaults to 'BANDPASS,PHASE,CHECK'.
    '' stands for no transfer sources.
    default: 'BANDPASS,PHASE,CHECK'
    example: 'PHASE'

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: False

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned


Description

Set model fluxes values for calibrator reference and transfer sources using lookup
values. By default the reference sources are the flux calibrators and the transfer
sources are the bandpass, phase, and check source calibrators. Reference sources
which are also in the transfer source list are removed from the transfer source list.

Builtin lookup tables are sued to compute models for solar system object calibrators.
Point source models for other calibrators are provided in the reference file.
Normalize fluxes are computed for transfer sources if the normfluxes parameter is
set to True.

The reference file default to a file called 'flux.csv' in the current working
directory. This file is normal created in the importdata step. The file is in 
'csv' format and contains the following comma delimited columns.

vis,fieldid,spwid,I,Q,U,V,pix,comment

Issues

Examples

1. Set model fluxes for the flux, bandpass, phase, and check sources.

   hif_setmodels()


        """

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['reference'] = reference
        mytmp['refintent'] = refintent
        mytmp['transfer'] = transfer
        mytmp['transintent'] = transintent
        mytmp['reffile'] = reffile
        mytmp['normfluxes'] = normfluxes
        mytmp['scalebychan'] = scalebychan
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_setmodels.xml')

        casalog.origin('hif_setmodels')
        if trec.has_key('hif_setmodels') and casac.utils().verify(mytmp, trec['hif_setmodels']) :
	    result = task_hif_setmodels.hif_setmodels(vis, reference, refintent, transfer, transintent, reffile, normfluxes, scalebychan, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
