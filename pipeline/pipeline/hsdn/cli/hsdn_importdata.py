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
import task_hsdn_importdata
def hsdn_importdata(vis=[''], session=[''], pipelinemode='automatic', overwrite=False, nocopy=False, createmms='automatic', dryrun=False, acceptresults=True):

        """Imports Nobeyama data into the single dish pipeline
The hsdn_importdata task loads the specified visibility data into the pipeline
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

overwrite -- Overwrite existing MSs on output.
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


Examples

1. Load an ASDM list in the ../rawdata subdirectory into the context.

   hsdn_importdata (vis=['../rawdata/uid___A002_X30a93d_X43e',
                '../rawdata/uid_A002_x30a93d_X44e'])

2. Load an MS in the current directory into the context.

   hsdn_importdata (vis=[uid___A002_X30a93d_X43e.ms])

3. Load a tarred ASDM in ../rawdata into the context.

   hsdn_importdata (vis=['../rawdata/uid___A002_X30a93d_X43e.tar.gz'])

4. Check the hsdn_importdata inputs, then import the data

   myvislist = ['uid___A002_X30a93d_X43e.ms', 'uid_A002_x30a93d_X44e.ms']
   hsdn_importdata(vis=myvislist, pipelinemode='getinputs')
   hsdn_importdata(vis=myvislist)

5. Load an ASDM but check the results before accepting them into the context.

   results = hsdn_importdata (vis=['uid___A002_X30a93d_X43e.ms'],
       acceptresults=False)
   results.accept()

6. Run in  dryrun mode before running for real
   results = hsdn_importdata (vis=['uid___A002_X30a93d_X43e.ms'], dryrun=True)
   results = hsdn_importdata (vis=['uid___A002_X30a93d_X43e.ms'])


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
        mytmp['overwrite'] = overwrite
        mytmp['nocopy'] = nocopy
        mytmp['createmms'] = createmms
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsdn/cli/"
	trec = casac.utils().torecord(pathname+'hsdn_importdata.xml')

        casalog.origin('hsdn_importdata')
        if trec.has_key('hsdn_importdata') and casac.utils().verify(mytmp, trec['hsdn_importdata']) :
	    result = task_hsdn_importdata.hsdn_importdata(vis, session, pipelinemode, overwrite, nocopy, createmms, dryrun, acceptresults)

	else :
	  result = False
        return result
