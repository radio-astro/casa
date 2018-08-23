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
import task_hsd_flagdata
def hsd_flagdata(vis=[''], autocorr=False, shadow=True, scan=True, scannumber='', intents='POINTING,FOCUS,ATMOSPHERE,SIDEBAND,CHECK', edgespw=True, fracspw='1.875GHz', fracspwfps=0.048387, online=True, fileonline='', template=True, filetemplate=[''], hm_tbuff='halfint', tbuff=0.0, qa0=True, qa2=True, pipelinemode='automatic', flagbackup=False, dryrun=False, acceptresults=True):

        """Do basic flagging of a list of MeasurementSets
The hsd_flagdata data performs basic flagging operations on a list of
MeasurementSets.


Keyword arguments:

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.


---- pipeline parameter arguments which can be set in any pipeline mode

autocorr -- Flag autocorrelation data.
    default: False

shadow -- Flag shadowed antennas.
    default: True

scan -- Flag a list of specified scans.
    default: True

scannumber -- A string containing a  comma delimited list of scans to be
    flagged.
    example: '3,5,6'
    default: ''

intents -- A string containing a comma delimited list of intents against
    which the scans to be flagged are matched.
    example: '*BANDPASS*'
    default: 'POINTING,FOCUS,ATMOSPHERE,SIDEBAND'

edgespw -- Flag the edge spectral window channels.
    default: True

fracspw -- Fraction of the baseline correlator TDM edge channels to be flagged.
    default: 0.0625

fracspwfps -- Fraction of the ACS correlator TDM edge channels to be flagged.
    default: 0.48387

online -- Apply the online flags.
   default: True

fileonline -- File containing the online flags. These are computed by the
    h_init or hif_importdata data tasks. If the online flags files
    are undefined a name of the form 'msname.flagonline.txt' is assumed.
    default: ''

template -- Apply flagging templates
    default: True

filetemplate -- The name of an text file that contains the flagging template
    for RFI, birdies, telluric lines, etc.  If the template flags files
    is undefined a name of the form 'msname.flagtemplate.txt' is assumed.
    default: ''

hm_tbuff -- The heuristic for computing the default time interval padding parameter.
    The options are 'halfint' and 'manual'. In 'halfint' mode tbuff is set to
    half the maximum of the median integration time of the science and calibrator target
    observations.
    default: 'halfint'

tbuff -- The time in seconds used to pad flagging command time intervals if
    hm_tbuff='manual'.
    default: 0.0

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

The hsd_t_flagdata data performs basic flagging operations on a list of measurements including:

o applying online flags
o apply a flagging template
o autocorrelation data flagging
o shadowed antenna data flagging
o scan based flagging by intent or scan number
o edge channel flagging

Issues


Examples

1. Do basic flagging on a MeasurementSet
    
    hsd_flagdata()

2. Do basic flagging on a MeasurementSet flagging additional scans selected
   by number as well.

    hsd_flagdata(scannumber='13,18')


        """
        if type(vis)==str: vis=[vis]
        if type(filetemplate)==str: filetemplate=[filetemplate]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['autocorr'] = autocorr
        mytmp['shadow'] = shadow
        mytmp['scan'] = scan
        mytmp['scannumber'] = scannumber
        mytmp['intents'] = intents
        mytmp['edgespw'] = edgespw
        mytmp['fracspw'] = fracspw
        mytmp['fracspwfps'] = fracspwfps
        mytmp['online'] = online
        mytmp['fileonline'] = fileonline
        mytmp['template'] = template
        mytmp['filetemplate'] = filetemplate
        mytmp['hm_tbuff'] = hm_tbuff
        mytmp['tbuff'] = tbuff
        mytmp['qa0'] = qa0
        mytmp['qa2'] = qa2
        mytmp['pipelinemode'] = pipelinemode
        mytmp['flagbackup'] = flagbackup
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli/"
	trec = casac.utils().torecord(pathname+'hsd_flagdata.xml')

        casalog.origin('hsd_flagdata')
        if trec.has_key('hsd_flagdata') and casac.utils().verify(mytmp, trec['hsd_flagdata']) :
	    result = task_hsd_flagdata.hsd_flagdata(vis, autocorr, shadow, scan, scannumber, intents, edgespw, fracspw, fracspwfps, online, fileonline, template, filetemplate, hm_tbuff, tbuff, qa0, qa2, pipelinemode, flagbackup, dryrun, acceptresults)

	else :
	  result = False
        return result
