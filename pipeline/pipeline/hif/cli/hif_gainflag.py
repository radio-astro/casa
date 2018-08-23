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
import task_hif_gainflag
def hif_gainflag(vis=[''], intent='', spw='', refant='', flag_mediandeviant=False, fmeddev_limit=3.0, flag_rmsdeviant=False, frmsdev_limit=8.0, flag_nrmsdeviant=True, fnrmsdev_limit=6.0, metric_order='mediandeviant, rmsdeviant, nrmsdeviant', pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Flag antennas with deviant gain

hif_gainflag flags data for antennas with deviant median gains and/or high 
gain rms.

Keyword arguments:

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline parameter arguments which can be set in any pipeline mode

flag_mediandeviant -- True to flag antennas with deviant median gain, 
    calculated as:
    
    abs( median(antenna) - median(all antennas) ) / MAD(all antennas) 
      > fmeddev_limit
    
    default False

fmeddev_limit -- Flag antennas with \'mediandeviant\' metric larger than 
    fmeddev_limit.
    default: 3.0

flag_rmsdeviant -- True to flag antennas with deviant gain rms, calculated as:

    stdev(antenna) / MAD(all antennas) > frmsdev_limit

    default: False

frmsdev_limit -- Flag antennas with 'rmsdeviant' metric larger than 
    frmsdev_limit.
    default: 8.0

flag_nrmsdeviant -- True to flag antennas with deviant normalised gain rms, 
    calculated as:

    deviation (ant_i) = ( sm(ant_i) - med_sm_allant ) / sigma_sm_allant
    
    where: 
     sm(ant_i) = sigma(ant_i) / median(ant_i)
     sigma_sm_allant = 1.4826 * mad({sm(ant_1), sm(ant_2), .., sm(ant_nants)})
     med_sm_allant = median( {sm(ant_1), sm(ant_2), ..., sm(ant_nants)} )

    default: True

fnrmsdev_limit -- Flag antennas with 'nrmsdeviant' metric larger than 
    fnrmsdev_limit.
    default: 6.0

metric_order -- Order in which the flagging metrics are evaluated.
    default: 'mediandeviant, rmsdeviant, nrmsdeviant'

---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- List of input MeasurementSets.
    default: [] - Use the MeasurementSets currently known to the pipeline
    context. 

intent -- A string containing the list of intents to be checked for antennas
    with deviant gains. The default is blank, which causes the task to select
    the 'BANDPASS' intent.
    default: ''
    example: '*BANDPASS*'

spw -- The list of spectral windows and channels to which the calibration
    will be applied. Defaults to all science windows in the pipeline
    context.
    default: '' 
    example: '17', '11, 15'

refant -- A string containing a prioritized list of reference antenna name(s)
    to be used to produce the gain table. Defaults to the value(s) stored in 
    the pipeline context. If undefined in the pipeline context defaults to the
    CASA reference antenna naming scheme.
    default: ''
    example: refant='DV01', refant='DV06,DV07'

-- Pipeline task execution modes

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
Deviant antennas are detected by analysis of a view showing their calibration 
gains. Three flagging metrics are available (mediandeviant, rmsdeviant, 
nrmsdeviant) and a flagging view is created for each metric that has been 
enabled. Each view is a list of 2D images with axes 'Time' and 'Antenna'; 
there is one image for each spectral window and intent. If any of the flagging
metrics exceeds their corresponding limit (fmeddev_limit, frmsdev_limit, 
fnrmsdev_limit) for a given antenna in a given spw, then all data for that 
antenna and for all spws within the same baseband will be flagged with a 
flagcmd.

The following heuristics are used for the flagging metrics:

   mediandeviant:
     abs( median(antenna) - median(all antennas) ) / MAD(all antennas) 
       > fmeddev_limit

   rmsdeviant:
     stdev(antenna) / MAD(all antennas) > frmsdev_limit

   nrmsdeviant:
    deviation (ant_i) = ( sm(ant_i) - med_sm_allant ) / sigma_sm_allant
    with: 
     sm(ant_i) = sigma(ant_i) / median(ant_i)
     sigma_sm_allant = 1.4826 * mad({sm(ant_1), sm(ant_2), .., sm(ant_nants)})
     med_sm_allant = median( {sm(ant_1), sm(ant_2), ..., sm(ant_nants)} )
     
   where MAD is the median absolute deviation from the median.


        """
        if type(vis)==str: vis=[vis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['intent'] = intent
        mytmp['spw'] = spw
        mytmp['refant'] = refant
        mytmp['flag_mediandeviant'] = flag_mediandeviant
        mytmp['fmeddev_limit'] = fmeddev_limit
        mytmp['flag_rmsdeviant'] = flag_rmsdeviant
        mytmp['frmsdev_limit'] = frmsdev_limit
        mytmp['flag_nrmsdeviant'] = flag_nrmsdeviant
        mytmp['fnrmsdev_limit'] = fnrmsdev_limit
        mytmp['metric_order'] = metric_order
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_gainflag.xml')

        casalog.origin('hif_gainflag')
        if trec.has_key('hif_gainflag') and casac.utils().verify(mytmp, trec['hif_gainflag']) :
	    result = task_hif_gainflag.hif_gainflag(vis, intent, spw, refant, flag_mediandeviant, fmeddev_limit, flag_rmsdeviant, frmsdev_limit, flag_nrmsdeviant, fnrmsdev_limit, metric_order, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
