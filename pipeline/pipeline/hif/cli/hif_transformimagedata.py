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
import task_hif_transformimagedata
def hif_transformimagedata(vis=[''], outputvis='', field='', intent='', spw='', datacolumn='corrected', chanbin=1, timebin='0s', replace=False, clear_pointing=True, modify_weights=False, wtmode='', pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Base transformimagedata task
The hif_transformimagedata task

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

vis -- List of visibility data files. These may be ASDMs, tar files of ASDMs,
   MSs, or tar files of MSs, If ASDM files are specified, they will be
   converted  to MS format.
   default: []
   example: vis=['X227.ms', 'asdms.tar.gz']



pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'

datacolumn -- Select spectral windows to split. The standard CASA options are
    supported
    default: 'corrected',
    example: 'data', 'model'

replace -- If a split was performed delete the parent MS and remove it from the context.
    default:  False
    example: True or False

clear_pointing -- Clear the pointing table
    
modify_weights -- Re-initialize the weights
    
wtmode -- optional weight initialization mode when modify_weights=True

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


Examples

1. Basic transformimagedata task

   hif_transformimagedata()



        """
        if type(vis)==str: vis=[vis]

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
        mytmp['clear_pointing'] = clear_pointing
        mytmp['modify_weights'] = modify_weights
        mytmp['wtmode'] = wtmode
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_transformimagedata.xml')

        casalog.origin('hif_transformimagedata')
        if trec.has_key('hif_transformimagedata') and casac.utils().verify(mytmp, trec['hif_transformimagedata']) :
	    result = task_hif_transformimagedata.hif_transformimagedata(vis, outputvis, field, intent, spw, datacolumn, chanbin, timebin, replace, clear_pointing, modify_weights, wtmode, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
