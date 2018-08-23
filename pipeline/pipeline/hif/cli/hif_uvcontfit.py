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
import task_hif_uvcontfit
def hif_uvcontfit(vis=[''], caltable=[''], contfile='', field='', intent='', spw='', combine='', solint='int', fitorder=1, pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Fit the continuum in the UV plane

This task estimates the continuum emission by fitting polynomials to
the real and imaginary parts of the spectral windows and channels
selected by spw and exclude spw. This fit represents a model of
the continuum in all channels. Fit orders less than 2 are strongly
recommended. Spw window combination is not currently supported.
	

Keyword Arguments


pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
    determines the values of all context defined pipeline inputs automatically.
    In 'interactive' mode the user can set the pipeline context defined
    parameters manually.  In 'getinputs' mode the user can check the settings
    of all pipeline parameters without running the task.
    default: 'automatic'.

--- pipeline parameter arguments which can be set in any pipeline mode

solint --
    default: 'int'
    example: solint='30s'

fitorder -- The fit order of the polynomials.
    default: 1
    example: fitorder = 0

---- pipeline context defined parameter arguments which can be set only in
     'interactive mode'

vis -- The list of input MeasurementSets for which the UV continuum fits are
    to be generated. Defaults to the list of imaging MeasurementSets specified
    in the pipeline import data task.
    default '': Compute the continuum fit for all calibration MeasurementSets in the context.
    example: 'ngc5921.ms', ['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']

caltable -- The list of output calibration tables one per input MS. 
    default: '', The output name defaults to the standard pipeline name scheme
    example: ['M51.uvcont']

contfile -- The file containing the continuum frequency ranges used for the 
    continuum fit.
    default: '', Defaults first to the file named in the context, next to a file called
             'cont.dat' in the pipeline working directory, or ''
    example: contfile = 'mycontfile'

field -- The list of field names or field ids for which UV continuum fits are
    computed. Defaults to all fields.
    default: ''
    example: '3C279', '3C279, M82'

intent -- A string containing a comma delimited list of intents against
    which the the selected fields are matched.
    default: '', Defaults to all data with TARGET intent.
    example: 'PHASE'

spw -- The list of spectral windows and channels for which uv continuum
    fits are computed.
    default: '', Defaults to all science spectral windows.
    example: '11,13,15,17'

combine -- Data axes to be combined for solving. Axes are 'scan', 'spw', or ''.
    This option is currently not supported.
    default: '', None.
    example: combine= 'scan'



        """
        if type(vis)==str: vis=[vis]
        if type(caltable)==str: caltable=[caltable]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['caltable'] = caltable
        mytmp['contfile'] = contfile
        mytmp['field'] = field
        mytmp['intent'] = intent
        mytmp['spw'] = spw
        mytmp['combine'] = combine
        mytmp['solint'] = solint
        mytmp['fitorder'] = fitorder
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_uvcontfit.xml')

        casalog.origin('hif_uvcontfit')
        if trec.has_key('hif_uvcontfit') and casac.utils().verify(mytmp, trec['hif_uvcontfit']) :
	    result = task_hif_uvcontfit.hif_uvcontfit(vis, caltable, contfile, field, intent, spw, combine, solint, fitorder, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
