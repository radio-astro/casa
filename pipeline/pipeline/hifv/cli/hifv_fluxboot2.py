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
import task_hifv_fluxboot2
def hifv_fluxboot2(vis=[''], caltable='', fitorder=1, pipelinemode='automatic', dryrun=False, acceptresults=True, refantignore=''):

        """Fluxboot2
The hifv_fluxboot task performs the flux density bootstrapping stage of the VLA pipeline.

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

vis -- List of visibility data files. These may be ASDMs, tar files of ASDMs,
   MSs, or tar files of MSs, If ASDM files are specified, they will be
   converted  to MS format.
   default: []
   example: vis=['X227.ms', 'asdms.tar.gz']

caltable -- string name of flagged caltable

fitorder -- Polynomial order of the spectral fitting for valid flux densities
               with multiple spws.  Currently only support 1 (spectral index only) or
               2 (spectral index and curvature).  It falls back to a lower fitorder if
               there are not enough solutions to fit with the requested fitorder.

refantignore -- string list to be ignored as reference antennas.
    default: ''
    Example:  refantignore='ea02,ea03'

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'


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

1. VLA CASA pipeline flux density bootstrapping.

hifv_fluxboot2()



        """
        if type(vis)==str: vis=[vis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['caltable'] = caltable
        mytmp['fitorder'] = fitorder
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
        mytmp['refantignore'] = refantignore
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli/"
	trec = casac.utils().torecord(pathname+'hifv_fluxboot2.xml')

        casalog.origin('hifv_fluxboot2')
        if trec.has_key('hifv_fluxboot2') and casac.utils().verify(mytmp, trec['hifv_fluxboot2']) :
	    result = task_hifv_fluxboot2.hifv_fluxboot2(vis, caltable, fitorder, pipelinemode, dryrun, acceptresults, refantignore)

	else :
	  result = False
        return result
