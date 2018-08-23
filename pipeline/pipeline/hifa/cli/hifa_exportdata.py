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
import task_hifa_exportdata
def hifa_exportdata(vis=[''], session=[''], imaging_products_only=False, exportmses=False, pprfile='', calintents='', calimages=[''], targetimages=[''], products_dir='', pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Prepare interferometry data for export
The hif_exportdata task exports the data defined in the pipeline context
and exports it to the data products directory, converting and or
packing it as necessary.

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In 'interactive' mode the user can set the pipeline context defined
   parameters manually.  In 'getinputs' mode the user can check the settings
   of all pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'

vis -- List of visibility data files for which flagging and calibration
   information will be exported. Defaults to the list maintained in the
   pipeline context.
   default: []
   example: vis=['X227.ms', 'X228.ms']

session -- List of sessions one per visibility file. Currently defaults
   to a single virtual session containing all the visibility files in vis. 
   In future will default to set of observing sessions defined in the 
   context.
   default: []
   example: session=['session1', 'session2']

exportmses -- Export the final MeasurementSets instead of the final flags,
   calibration tables, and calibration instructions.
   default: False
   example: exportmses = True

pprfile -- Name of the pipeline processing request to be exported. Defaults
   to a file matching the template 'PPR_*.xml'.
   default: []
   example: pprfile=['PPR_GRB021004.xml']

calintents -- List of calibrator image types to be exported. Defaults to
   all standard calibrator intents 'BANDPASS', 'PHASE', 'FLUX'
   default: ''
   example: calintents='PHASE'

calimages -- List of calibrator images to be exported. Defaults to all
   calibrator images recorded in the pipeline context.
   default: []
   example: calimages=['3C454.3.bandpass', '3C279.phase']

targetimages -- List of science target images to be exported. Defaults to all
   science target images recorded in the pipeline context.
   default: []
   example: targetimages=['NGC3256.band3', 'NGC3256.band6']

products_dir -- Name of the data products subdirectory. Defaults to './'
   default: ''
   example: products_dir='../products'


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

The hif_exportdata task exports the data defined in the pipeline context
and exports it to the data products directory, converting and or packing
it as necessary.

The current version of the task exports the following products

o an XML file containing the pipeline processing request
o a tar file per ASDM / MS containing the final flags version
o a text file per ASDM / MS containing the final calibration apply list
o a FITS image for each selected calibrator source image
o a FITS image for each selected science target source image
o a tar file per session containing the caltables for that session
o a tar file containing the file web log
o a text file containing the final list of CASA commands


Issues

Support for merging the calibration state information into the pipeline
context / results structure and retrieving it still needs to be added.

Support for merging the clean results into the pipeline context / results
structure and retrieving it still needs to be added.

Support for creating the final pipeline results entity still needs to
be added.
    
Session information is not currently handled by the pipeline context.
By default all ASDMs are combined into one session.


Examples

1. Export the pipeline results for a single sessions to the data products
directory

    !mkdir ../products
    hif_exportdata (products_dir='../products')

2. Export the pipeline results to the data products directory specify that
only the gain calibrator images be saved.

    !mkdir ../products
    hif_exportdata (products_dir='../products', calintents='*PHASE*')


        """
        if type(vis)==str: vis=[vis]
        if type(session)==str: session=[session]
        if type(calimages)==str: calimages=[calimages]
        if type(targetimages)==str: targetimages=[targetimages]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['session'] = session
        mytmp['imaging_products_only'] = imaging_products_only
        mytmp['exportmses'] = exportmses
        mytmp['pprfile'] = pprfile
        mytmp['calintents'] = calintents
        mytmp['calimages'] = calimages
        mytmp['targetimages'] = targetimages
        mytmp['products_dir'] = products_dir
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli/"
	trec = casac.utils().torecord(pathname+'hifa_exportdata.xml')

        casalog.origin('hifa_exportdata')
        if trec.has_key('hifa_exportdata') and casac.utils().verify(mytmp, trec['hifa_exportdata']) :
	    result = task_hifa_exportdata.hifa_exportdata(vis, session, imaging_products_only, exportmses, pprfile, calintents, calimages, targetimages, products_dir, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
