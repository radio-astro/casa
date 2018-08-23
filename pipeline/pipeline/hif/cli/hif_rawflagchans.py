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
import task_hif_rawflagchans
def hif_rawflagchans(vis=[''], spw='', intent='', flag_hilo=True, fhl_limit=20.0, fhl_minsample=5, flag_bad_quadrant=True, fbq_hilo_limit=8.0, fbq_antenna_frac_limit=0.2, fbq_baseline_frac_limit=1.0, pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Flag deviant baseline/channels in raw data

hif_rawflagchans flags deviant baseline/channels in the raw data. 

The flagging views used are derived from the raw data for the specified 
intent - default is BANDPASS. 

Bad baseline/channels are flagged for all intents, not just the
one that is the basis of the flagging views.

For each spectral window the flagging view is a 2d image with axes 
'channel' and 'baseline'. The pixel for each channel,baseline is the 
time average of the underlying unflagged raw data.

The baseline axis is labeled by numbers of form id1.id2 where id1 and id2
are the IDs of the baseline antennas. Both id1 and id2 run over all 
antenna IDs in the observation. This means that each baseline is shown 
twice but has the benefit that 'bad' antennas are easily identified by 
eye.

Three flagging methods are available:

If parameter flag_hilo is set True then outliers from the median
of each flagging view will be flagged.

If parameter flag_bad_quadrant is set True then a simple 2 part 
test is used to check for bad antenna quadrants and/or bad baseline 
quadrants. Here a 'quadrant' is defined simply as one quarter of the
channel axis. The first part of the test is to note as 'suspect' those
points further from the view median than fbq_hilo_limit * MAD.
The second part is to flag entire antenna/quadrants if their
fraction of suspect points exceeds fbq_antenna_frac_limit. 
Failing that, entire baseline/quadrants may be flagged if their 
fraction of suspect points exceeds fbq_baseline_frac_limit. 
Suspect points are not flagged unless as part of a bad antenna or 
baseline quadrant. 

Keyword arguments:

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.

---- pipeline parameter arguments which can be set in any pipeline mode

flag_hilo -- True to flag channel/baseline data further from the view
             median than fhl_limit * MAD.
    default: True

fhl_limit -- If flag_hilo is True then flag channel/baseline data
             further from the view median than fhl_limit * MAD.
    default: 20

fhl_minsample -- Do no flagging if the view median and MAD are derived
                 from fewer than fhl_minsample view pixels.
    default: 5

flag_bad_quadrant -- True to search for and flag bad antenna quadrants
                     and baseline quadrants. Here a /'quadrant/' is one 
                     quarter of the channel axis.
    default: True

fbq_hilo_limit -- If flag_bad_quadrant is True then channel/baselines
                  further from the the view median than fbq_hilo_limit
                  * MAD will be noted as 'suspect'. If there are 
                  enough of them to indicate that an antenna or baseline
                  quadrant is bad then all channel/baselines in that
                  quadrant will be flagged.
    default: 8.0

fbq_antenna_frac_limit -- If flag_bad_quadrant is True and the fraction
                          of suspect channel/baselines in a particular
                          antenna/quadrant exceeds fbq_antenna_frac_limit
                          then all data for that antenna/quadrant will
                          be flagged.
    default: 0.2

fbq_baseline_frac_limit -- If flag_bad_quadrant is True and the fraction
                           of suspect channel/baselines in a particular
                           baseline/quadrant exceeds fbq_baseline_frac_limit
                           then all data for that baseline/quadrant will
                           be flagged.
    default: 1.0 (i.e. no flagging)

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

-- Pipeline task execution modes

dryrun -- Run the commands (True) or generate the commands to be run but
          do not execute (False).
    default: True

acceptresults -- This parameter has no effect. The Tsyscal file is already
                 in the pipeline context and is flagged in situ.

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.


Examples:

1. Flag bad quadrants and wild outliers, default method.

   hif_rawflagchans()
       equivalent to:
   hif_rawflagchans(flag_hilo=True, fhl_limit=20,
     flag_bad_quadrant=True, fbq_hilo_limit=8, 
     fbq_antenna_frac_limit=0.2, fbq_baseline_frac_limit=1.0)


        """
        if type(vis)==str: vis=[vis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['spw'] = spw
        mytmp['intent'] = intent
        mytmp['flag_hilo'] = flag_hilo
        mytmp['fhl_limit'] = fhl_limit
        mytmp['fhl_minsample'] = fhl_minsample
        mytmp['flag_bad_quadrant'] = flag_bad_quadrant
        mytmp['fbq_hilo_limit'] = fbq_hilo_limit
        mytmp['fbq_antenna_frac_limit'] = fbq_antenna_frac_limit
        mytmp['fbq_baseline_frac_limit'] = fbq_baseline_frac_limit
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_rawflagchans.xml')

        casalog.origin('hif_rawflagchans')
        if trec.has_key('hif_rawflagchans') and casac.utils().verify(mytmp, trec['hif_rawflagchans']) :
	    result = task_hif_rawflagchans.hif_rawflagchans(vis, spw, intent, flag_hilo, fhl_limit, fhl_minsample, flag_bad_quadrant, fbq_hilo_limit, fbq_antenna_frac_limit, fbq_baseline_frac_limit, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
