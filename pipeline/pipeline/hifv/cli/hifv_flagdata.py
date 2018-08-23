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
import task_hifv_flagdata
def hifv_flagdata(vis=[''], autocorr=True, shadow=True, scan=True, scannumber='', quack=True, clip=True, baseband=True, intents='*POINTING*,*FOCUS*,*ATMOSPHERE*,*SIDEBAND_RATIO*, *UNKNOWN*, *SYSTEM_CONFIGURATION*, *UNSPECIFIED#UNSPECIFIED*', edgespw=True, fracspw=0.05, online=True, fileonline='', template=True, filetemplate=[''], hm_tbuff='1.5int', tbuff=0.0, pipelinemode='automatic', flagbackup=False, dryrun=False, acceptresults=True):

        """Do basic deterministic flagging of a list of MeasurementSets
The hifv_flagdata data performs basic flagging operations on a list of
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
    default: True

shadow -- Flag shadowed antennas.
    default: True

scan -- Flag a list of specified scans.
    default: True

scannumber -- A string containing a  comma delimited list of scans to be
    flagged.
    example: '3,5,6'
    default: ''

quack -- Quack scans
    default: True

clip -- Clip mode
    default: True

baseband -- Flag 20MHz of each edge of basebands
    default: True

intents -- A string containing a comma delimited list of intents against
    which the scans to be flagged are matched.
    example: '*BANDPASS*'
    default: '*POINTING*,*FOCUS*,*ATMOSPHERE*,*SIDEBAND_RATIO*'

edgespw -- Flag the edge spectral window channels.
    default: True

fracspw -- Fraction of the baseline correlator TDM edge channels to be flagged.
    default: 0.05



online -- Apply the online flags.
   default: True

fileonline -- File containing the online flags. These are computed by the
    h_init or hif_importdata data tasks. If the online flags files
    are undefined a name of the form 'msname.flagonline.txt' is assumed.
    default: ''

template -- Apply flagging templates
    default: False

filetemplate -- The name of an text file that contains the flagging template
    for RFI, birdies, telluric lines, etc.  If the template flags files
    is undefined a name of the form 'msname.flagtemplate.txt' is assumed.
    default: ''


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

The hif_flagdata data performs basic flagging operations on a list of measurements including:

o autocorrelation data flagging
o shadowed antenna data flagging
o scan based flagging
o edge channel flagging
o baseband edge flagging
o applying online flags
o apply a flagging template
o quack, shadow, and basebands
o Antenna not-on-source (ANOS)

Issues


Examples

1. Do basic flagging on a MeasurementSet
    
    hifv_flagdata()

2. Do basic flagging on a MeasurementSet as well as flag pointing and
   atmosphere data

    hifv_flagdata(scan=True intent='*BANDPASS*')


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
        mytmp['quack'] = quack
        mytmp['clip'] = clip
        mytmp['baseband'] = baseband
        mytmp['intents'] = intents
        mytmp['edgespw'] = edgespw
        mytmp['fracspw'] = fracspw
        mytmp['online'] = online
        mytmp['fileonline'] = fileonline
        mytmp['template'] = template
        mytmp['filetemplate'] = filetemplate
        mytmp['hm_tbuff'] = hm_tbuff
        mytmp['tbuff'] = tbuff
        mytmp['pipelinemode'] = pipelinemode
        mytmp['flagbackup'] = flagbackup
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli/"
	trec = casac.utils().torecord(pathname+'hifv_flagdata.xml')

        casalog.origin('hifv_flagdata')
        if trec.has_key('hifv_flagdata') and casac.utils().verify(mytmp, trec['hifv_flagdata']) :
	    result = task_hifv_flagdata.hifv_flagdata(vis, autocorr, shadow, scan, scannumber, quack, clip, baseband, intents, edgespw, fracspw, online, fileonline, template, filetemplate, hm_tbuff, tbuff, pipelinemode, flagbackup, dryrun, acceptresults)

	else :
	  result = False
        return result
