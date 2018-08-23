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
import task_hsd_imaging
def hsd_imaging(mode='line', restfreq=[''], pipelinemode='automatic', infiles=[''], field='', spw='', dryrun=False, acceptresults=True):

        """Generate single dish images
The hsd_imaging task generates single dish images per antenna as well as 
combined image over whole antennas for each field and spectral window. 
Image configuration (grid size, number of pixels, etc.) is automatically 
determined based on meta data such as antenna diameter, map extent, etc.

Note that generated images are always in LSRK frame.
  
Keyword arguments:

mode -- imaging mode controls imaging parameters in the task.
   Accepts either 'line' (spectral line imaging) or 'ampcal' (image settings
   for amplitude calibrator)
   default: 'line'
   options: 'line', 'ampcal',

---- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the 
   pipeline determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'

infiles -- List of data files. These must be a name of MeasurementSets that 
   are registered to context via hsd_importdata task.
   default: []
   example: vis=['uid___A002_X85c183_X36f.ms', 'uid___A002_X85c183_X60b.ms']

field -- Data selection by field names or ids.
   default: '' (all fields)
   example: '*Sgr*,M100'
   
spw -- Data selection by spw IDs.
   default: '' (all spws)
   example: '3,4' (generate images for spw 3 and 4)

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



        """
        if type(restfreq)==str: restfreq=[restfreq]
        if type(infiles)==str: infiles=[infiles]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['mode'] = mode
        mytmp['restfreq'] = restfreq
        mytmp['pipelinemode'] = pipelinemode
        mytmp['infiles'] = infiles
        mytmp['field'] = field
        mytmp['spw'] = spw
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli/"
	trec = casac.utils().torecord(pathname+'hsd_imaging.xml')

        casalog.origin('hsd_imaging')
        if trec.has_key('hsd_imaging') and casac.utils().verify(mytmp, trec['hsd_imaging']) :
	    result = task_hsd_imaging.hsd_imaging(mode, restfreq, pipelinemode, infiles, field, spw, dryrun, acceptresults)

	else :
	  result = False
        return result
