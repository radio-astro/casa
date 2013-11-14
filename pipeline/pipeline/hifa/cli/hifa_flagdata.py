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
#from taskmanager import tm
import task_hifa_flagdata
def hifa_flagdata(vis=[''], autocorr=True, shadow=True, scan=True, scannumber='', intents='POINTING,FOCUS,ATMOSPHERE,SIDEBAND', edgespw=True, fracspw=0.0625, fracspwfps=0.048387, online=True, fileonline='', template=True, filetemplate=[''], pipelinemode='automatic', flagbackup=False, dryrun=False, acceptresults=True):

        """Do basic flagging of a list of measurement sets
The hifa_flagdata data performs basic flagging operations on a list of
measurement sets.


Keyword arguments:

pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs automatically.
   In interactive mode the user can set the pipeline context defined parameters
   manually.  In 'getinputs' mode the user can check the settings of all
   pipeline parameters without running the task.
   default: 'automatic'.


---- pipeline parameter arguments which can be set in any pipeline mode

autocorr -- Flag autocorrelation data.
    default: True

shadow -- Flag shadowed antennas.
    default: True

scan -- Flag a list of specifed scans.
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
    are undefined a name of the form 'msname_flagonline.txt' is assumed.
    default: ''

template -- Apply flagging templates
    default: True

filetemplate -- The name of an text file that contains the flagging template
    for RFI, birdies, telluric lines, etc.  If the template flags files
    is undefined a name of the form 'msname_flagtemplate.txt' is assumed.
    default: ''


---- pipeline context defined parameter arguments which can be set only in
'interactive mode'

vis -- The list of input measurement sets. Defaults to the list of measurement
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

o autocorrelation data flagging
o shadowed antenna data flagging
o scan based flagging
o edge channel flagging
o applying online flags
o apply a flagging template

Issues


Examples

1. Do basic flagging on a measurement set
    
    hifa_flagdata()

2. Do basic flagging on a measurement set as well as flag pointing and
   atmosphere data

    hifa_flagdata(scan=True intent='BANDPASS')


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
        mytmp['pipelinemode'] = pipelinemode
        mytmp['flagbackup'] = flagbackup
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///export/home/skye/pipecode/casa/pipeline/pipeline/hifa/cli/"
        trec = casac.utils().torecord(pathname+'hifa_flagdata.xml')

        casalog.origin('hifa_flagdata')
        if trec.has_key('hifa_flagdata') and casac.utils().verify(mytmp, trec['hifa_flagdata']) :
	    result = task_hifa_flagdata.hifa_flagdata(vis, autocorr, shadow, scan, scannumber, intents, edgespw, fracspw, fracspwfps, online, fileonline, template, filetemplate, pipelinemode, flagbackup, dryrun, acceptresults)

	else :
	  result = False
        return result
