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
import task_h_mssplit
def h_mssplit(vis=[''], outputvis=[''], field='', intent='', spw='', datacolumn='data', chanbin=1, timebin='0s', replace=True, pipelinemode='automatic', dryrun=False, acceptresults=True):

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
    default '': Split all MeasurementSets in the context. 
    example: 'ngc5921.ms', ['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']

outputvis -- The list of output split MeasurementSets. The output list must
    be the same length as the input list and the output names must be different
    from the input names.
    default '', The output name defaults to <msrootname>_split.ms
    example: 'ngc5921.ms', ['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']

field -- Select fields name(s) or id(s) to split.
    default: '', All fields will be selected
    example: '3C279', '"5795"' Note the double quotes around names which can be interpreted as numbers

intent -- Select intents to split
    default: '', All data is selected.
    example: 'TARGET'

spw -- Select spectral windows to split.
    default: '', All spws are selected
    example: '9', '9,13,15'

datacolumn -- Select spectral windows to split. The standard CASA options are
    supported
    default: 'data', 
    example: 'corrected', 'model'

chanbin -- The channel binning factor. 1 for no binning, otherwise 2, 4, 8, or 16.
    supported
    default: 1, 
    example: 2, 4

timebin -- The time binning factor. '0s' for no binning
    default: '0s' 
    example: '10s' for 10 second binning

replace -- If a split was performed delete the parent MS and remove it from the context.
    default:  True
    example: False

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

1. Create a 4X channel smoothed output ms from the input ms 

   h_mssplit(chanbin=4)


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
        mytmp['datacolumn'] = datacolumn
        mytmp['chanbin'] = chanbin
        mytmp['timebin'] = timebin
        mytmp['replace'] = replace
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli/"
	trec = casac.utils().torecord(pathname+'h_mssplit.xml')

        casalog.origin('h_mssplit')
        if trec.has_key('h_mssplit') and casac.utils().verify(mytmp, trec['h_mssplit']) :
	    result = task_h_mssplit.h_mssplit(vis, outputvis, field, intent, spw, datacolumn, chanbin, timebin, replace, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
