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
import task_hsd_blflag
def hsd_blflag(iteration=5, edge=[0,0], flag_tsys=True, tsys_thresh=3.0, flag_weath=False, weath_thresh=3.0, flag_prfre=True, prfre_thresh=3.0, flag_pofre=True, pofre_thresh=1.3333, flag_prfr=True, prfr_thresh=4.5, flag_pofr=True, pofr_thresh=4.0, flag_prfrm=True, prfrm_thresh=5.5, prfrm_nmean=5, flag_pofrm=True, pofrm_thresh=5.0, pofrm_nmean=5, flag_user=False, user_thresh=5.0, plotflag=True, pipelinemode='automatic', infiles=[''], antenna='', field='', spw='', pol='', dryrun=False, acceptresults=True, parallel='automatic'):

        """Flag spectra based on predefined criteria of single-dish pipeline

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode
iteration -- Number of iterations to perform sigma clipping to calculate
       threshold value of flagging.
       default: 5

edge -- Number of channels to be dropped from the edge.
       The value must be a list of integer with length of one or two.
                       If list length is one, same number will be applied both side of 
       the band.
       default: [0,0]
       example: [10,20], [10]

flag_tsys -- Activate (True) or deactivate (False) Tsys flag.
       default: True

tsys_thresh -- Threshold value for Tsys flag.
       default: 3.0

flag_weath -- Activate (True) or deactivate (False) weather flag. 
       Since weather flagging is not implemented yet. Setting True 
       has no effect at the moment.
       default: False

weath_thresh -- Threshold value for weather flag.
       default: 3.0

flag_prfre -- Activate (True) or deactivate (False) flag by expected rms 
       of pre-fit spectra.
       default: True

                prfre_thresh -- Threshold value for flag by expected rms of pre-fit 
       spectra. 
       default: 3.0 

flag_pofre -- Activate (True) or deactivate (False) flag by expected rms 
       of post-fit spectra.
       default: True

pofre_thresh -- Threshold value for flag by expected rms of post-fit 
       spectra. 
       default: 1.3333

flag_prfr -- Activate (True) or deactivate (False) flag by rms of pre-fit 
       spectra.
       default: True

prfr_thresh -- Threshold value for flag by rms of pre-fit spectra. 
       default: 4.5

flag_pofr -- Activate (True) or deactivate (False) flag by rms of post-fit 
       spectra.
       default: True

                pofr_thresh -- Threshold value for flag by rms of post-fit spectra. 
       default: 4.0

flag_prfrm -- Activate (True) or deactivate (False) flag by running mean 
       of pre-fit spectra.
       default: True

prfrm_thresh -- Threshold value for flag by running mean of pre-fit 
       spectra. 
       default: 5.5

prfrm_nmean -- Number of channels for running mean of pre-fit spectra.
       default: 5

flag_pofrm -- Activate (True) or deactivate (False) flag by running mean 
       of post-fit spectra.
       default: True

pofrm_thresh -- Threshold value for flag by running mean of post-fit 
       spectra. 
       default: 5.0

pofrm_nmean -- Number of channels for running mean of post-fit spectra.
                       default: 5

flag_user -- Activate (True) or deactivate (False) user-defined flag. 
       Since user flagging is not implemented yet. Setting True 
       has no effect at the moment.
       default: False

user_thresh -- Threshold value for flag by user-defined rule.
       default: 3.0

plotflag -- True to plot result of data flagging.
       default: True

pipelinemode -- The pipeline operating mode. In 'automatic' mode the 
       pipeline determines the values of all context defined pipeline 
       inputs automatically.  In interactive mode the user can set the 
       pipeline context defined parameters manually. In 'getinputs' mode 
       the user can check the settings of all pipeline parameters without 
       running the task.
       default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode' or 'getinputs' modes 
                
infiles -- ASDM or MS files to be processed. This parameter behaves as 
       data selection parameter. The name specified by infiles must be 
       registered to context before you run hsd_blflag.
       default: [] (process all data in context)

antenna -- Data selection by antenna names or ids.
   default: '' (all antennas)
   example: 'PM03,PM04'
   
field -- Data selection by field names or ids.
   default: '' (all fields)
   example: '*Sgr*,M100'
   
spw -- Data selection by spw IDs.
   default: '' (all spws)
   example: '3,4' (spw 3 and 4)

pol -- Data selection by pol.
   default: '' (all polarizations)
   example: 'XX,YY' (correlation XX and YY)

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
                       do not execute (False).
       default: True

acceptresults -- Add the results of the task to the pipeline context 
       (True) or reject them (False).
       default: True


Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
       the results object for the pipeline task is returned.


Description

The hsd_blflag
6. Flagging 
Data are flagged based on several flagging rules. Available rules are: 
expected rms, calculated rms, and running mean of both pre-fit and 
post-fit spectra. Tsys flagging is also available. 

In addition, the heuristics script creates many plots for each stage. 
Those plots are included in the weblog.


Issues


Examples
                                

        """
        if type(edge)==int: edge=[edge]
        if type(infiles)==str: infiles=[infiles]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['iteration'] = iteration
        mytmp['edge'] = edge
        mytmp['flag_tsys'] = flag_tsys
        mytmp['tsys_thresh'] = tsys_thresh
        mytmp['flag_weath'] = flag_weath
        mytmp['weath_thresh'] = weath_thresh
        mytmp['flag_prfre'] = flag_prfre
        mytmp['prfre_thresh'] = prfre_thresh
        mytmp['flag_pofre'] = flag_pofre
        mytmp['pofre_thresh'] = pofre_thresh
        mytmp['flag_prfr'] = flag_prfr
        mytmp['prfr_thresh'] = prfr_thresh
        mytmp['flag_pofr'] = flag_pofr
        mytmp['pofr_thresh'] = pofr_thresh
        mytmp['flag_prfrm'] = flag_prfrm
        mytmp['prfrm_thresh'] = prfrm_thresh
        mytmp['prfrm_nmean'] = prfrm_nmean
        mytmp['flag_pofrm'] = flag_pofrm
        mytmp['pofrm_thresh'] = pofrm_thresh
        mytmp['pofrm_nmean'] = pofrm_nmean
        mytmp['flag_user'] = flag_user
        mytmp['user_thresh'] = user_thresh
        mytmp['plotflag'] = plotflag
        mytmp['pipelinemode'] = pipelinemode
        mytmp['infiles'] = infiles
        mytmp['antenna'] = antenna
        mytmp['field'] = field
        mytmp['spw'] = spw
        mytmp['pol'] = pol
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
        mytmp['parallel'] = parallel
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli/"
	trec = casac.utils().torecord(pathname+'hsd_blflag.xml')

        casalog.origin('hsd_blflag')
        if trec.has_key('hsd_blflag') and casac.utils().verify(mytmp, trec['hsd_blflag']) :
	    result = task_hsd_blflag.hsd_blflag(iteration, edge, flag_tsys, tsys_thresh, flag_weath, weath_thresh, flag_prfre, prfre_thresh, flag_pofre, pofre_thresh, flag_prfr, prfr_thresh, flag_pofr, pofr_thresh, flag_prfrm, prfrm_thresh, prfrm_nmean, flag_pofrm, pofrm_thresh, pofrm_nmean, flag_user, user_thresh, plotflag, pipelinemode, infiles, antenna, field, spw, pol, dryrun, acceptresults, parallel)

	else :
	  result = False
        return result
