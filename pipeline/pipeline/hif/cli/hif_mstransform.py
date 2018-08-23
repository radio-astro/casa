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
import task_hif_mstransform
def hif_mstransform(vis=[''], outputvis=[''], field='', intent='', spw='', pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Select data from calibrated MS(s) to form new MS(s) for imaging

Create a list of science target MS(s) for imaging 

Keyword Arguments


pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
    determines the values of all context defined pipeline inputs automatically.
    In 'interactive' mode the user can set the pipeline context defined
    parameters manually.  In 'getinputs' mode the user can check the settings
    of all pipeline parameters without running the task.
    default: 'automatic'.

--- pipeline parameter arguments which can be set in any pipeline mode



---- pipeline context defined parameter arguments which can be set only in
     'interactive mode'

vis -- The list of input MeasurementSets to be transformed. Defaults to the
    list of MeasurementSets specified in the pipeline import data task.
    default '': Transform all calibration MeasurementSets in the context. 
    example: 'ngc5921.ms', ['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']

outputvis -- The list of output transformed MeasurementSets to be used for imaging.
    The output list must be the same length as the input list.
    default '', The output name defaults to <msrootname>_target.ms
    example: 'ngc5921.ms', ['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']

field -- Select fields name(s) or id(s) to transform. Only fields with data matching
    the intent will be selected. 
    default: '', Fields matching matching intent.
    example: '3C279', 'Centaurus*', '3C279,J1427-421'

intent -- Select intents for which associated fields will be imaged.
    default: '', Only TARGET data is selected.
    example: 'PHASE,BANDPASS'

spw -- Select spectral window/channels to image.
    default: '', All science spws for which the specified intent is valid are selected 
    example: '9'

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: False

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

Output

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.

Description

Create new MeasurementSets for imaging from the corrected column of the input
MeasurementSet. By default all science target data is copied to the new ms. The new
MeasurementSet is not re-indexed to the selected data in the new ms will have the
same source, field, and spw names and ids as it does in the parent ms. 

Issues

TBD
Examples

1. Create a science target ms from the corrected column in the input ms. 

    hif_mstransform()

2. Make a phase and bandpass calibrator targets ms from the corrected
column in the input ms.

    hif_mstransform(intent='PHASE,BANDPASS')



        """
        if type(vis)==str: vis=[vis]
        if type(outputvis)==str: outputvis=[outputvis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['outputvis'] = outputvis
        mytmp['field'] = field
        mytmp['intent'] = intent
        mytmp['spw'] = spw
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_mstransform.xml')

        casalog.origin('hif_mstransform')
        if trec.has_key('hif_mstransform') and casac.utils().verify(mytmp, trec['hif_mstransform']) :
	    result = task_hif_mstransform.hif_mstransform(vis, outputvis, field, intent, spw, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
