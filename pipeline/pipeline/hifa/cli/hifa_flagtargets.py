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
import task_hifa_flagtargets
def hifa_flagtargets(vis=[''], template=True, filetemplate=[''], pipelinemode='automatic', flagbackup=False, dryrun=False, acceptresults=True):

        """Do science target flagging
The hifa_flagtargets task performs basic flagging operations on a list of
science target MeasurementSets.


Keyword arguments:

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.


---- pipeline parameter arguments which can be set in any pipeline mode

template -- Apply flagging templates
    default: True

filetemplate -- The name of an text file that contains the flagging template
    for issues with the science target data  etc.  If the template flags files
    is undefined a name of the form 'msname_flagtargetstemplate.txt' is assumed.
    default: ''

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of MeasurementSets
    defined in the pipeline context.
    example:
    default: ''

flagbackup -- Back up any pre-existing flags.
       default: False

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

The hifa_flagdata data performs basic flagging operations on a list of measurements including:

o apply a flagging template

Issues


Examples

1. Do basic flagging on a science target MeasurementSet
    
    hifa_flagtargets()


        """
        if type(vis)==str: vis=[vis]
        if type(filetemplate)==str: filetemplate=[filetemplate]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['template'] = template
        mytmp['filetemplate'] = filetemplate
        mytmp['pipelinemode'] = pipelinemode
        mytmp['flagbackup'] = flagbackup
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.utils().torecord(pathname+'hifa_flagtargets.xml')

        casalog.origin('hifa_flagtargets')
        if trec.has_key('hifa_flagtargets') and casac.utils().verify(mytmp, trec['hifa_flagtargets']) :
	    result = task_hifa_flagtargets.hifa_flagtargets(vis, template, filetemplate, pipelinemode, flagbackup, dryrun, acceptresults)

	else :
	  result = False
        return result
