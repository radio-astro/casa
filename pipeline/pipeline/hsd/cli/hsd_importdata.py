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
import task_hsd_importdata
def hsd_importdata(vis=[''], session=[''], pipelinemode='automatic', asis='SBSummary ExecBlock Antenna Station Receiver Source CalAtmosphere CalWVR', process_caldevice=False, overwrite=False, nocopy=False, bdfflags=True, lazy=False, with_pointing_correction=True, createmms='false', dryrun=False, acceptresults=True):

        """Imports data into the single dish pipeline
The hsd_importdata task loads the specified visibility data into the pipeline
context unpacking and / or converting it as necessary.

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

vis -- List of visibility data files. These may be ASDMs, tar files of ASDMs,
   MSs, or tar files of MSs, If ASDM files are specified, they will be
   converted  to MS format.
   default: []
   example: vis=['X227.ms', 'asdms.tar.gz']

session -- List of sessions to which the visibility files belong. Defaults
   to a single session containing all the visibility files, otherwise
   a session must be assigned to each vis file.
   default: []
   example: session=['Session_1', 'Sessions_2']

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'

asis -- ASDM tables to convert as is
   default: 'Antenna Station Receiver CalAtmosphere'
   example: 'Receiver', ''

process_caldevice -- Ingest the ASDM caldevice table
   default: False
   example: True

overwrite -- Overwrite existing MSs on output.
   default: False

bdfflags -- Apply BDF flags on line
   default: True
   
lazy -- Use the lazy filter import
   default: False

with_pointing_correction -- add (ASDM::Pointing::encoder - ASDM::Pointing::pointingDirection) 
   to the value to be written in MS::Pointing::direction
   default: True

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

1. Load an ASDM list in the ../rawdata subdirectory into the context.

   hsd_importdata (vis=['../rawdata/uid___A002_X30a93d_X43e',
                '../rawdata/uid_A002_x30a93d_X44e'])

2. Load an MS in the current directory into the context.

   hsd_importdata (vis=[uid___A002_X30a93d_X43e.ms])

3. Load a tarred ASDM in ../rawdata into the context.

   hsd_importdata (vis=['../rawdata/uid___A002_X30a93d_X43e.tar.gz'])

4. Check the hsd_importdata inputs, then import the data

   myvislist = ['uid___A002_X30a93d_X43e.ms', 'uid_A002_x30a93d_X44e.ms']
   hsd_importdata(vis=myvislist, pipelinemode='getinputs')
   hsd_importdata(vis=myvislist)

5. Load an ASDM but check the results before accepting them into the context.

   results = hsd_importdata (vis=['uid___A002_X30a93d_X43e.ms'],
       acceptresults=False)
   results.accept()

6. Run in  dryrun mode before running for real
   results = hsd_importdata (vis=['uid___A002_X30a93d_X43e.ms'], dryrun=True)
   results = hsd_importdata (vis=['uid___A002_X30a93d_X43e.ms'])


        """
        if type(vis)==str: vis=[vis]
        if type(session)==str: session=[session]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['session'] = session
        mytmp['pipelinemode'] = pipelinemode
        mytmp['asis'] = asis
        mytmp['process_caldevice'] = process_caldevice
        mytmp['overwrite'] = overwrite
        mytmp['nocopy'] = nocopy
        mytmp['bdfflags'] = bdfflags
        mytmp['lazy'] = lazy
        mytmp['with_pointing_correction'] = with_pointing_correction
        mytmp['createmms'] = createmms
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli/"
	trec = casac.utils().torecord(pathname+'hsd_importdata.xml')

        casalog.origin('hsd_importdata')
        if trec.has_key('hsd_importdata') and casac.utils().verify(mytmp, trec['hsd_importdata']) :
	    result = task_hsd_importdata.hsd_importdata(vis, session, pipelinemode, asis, process_caldevice, overwrite, nocopy, bdfflags, lazy, with_pointing_correction, createmms, dryrun, acceptresults)

	else :
	  result = False
        return result
