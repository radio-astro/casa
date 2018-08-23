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
import task_hif_makeimlist
def hif_makeimlist(vis=[''], imagename='', intent='TARGET', field='', spw='', contfile='', linesfile='', uvrange='', specmode='', outframe='', hm_imsize=[''], hm_cell=[''], calmaxpix=300, phasecenter='', nchan=-1, start='', width='', nbins='', robust=-999.0, uvtaper=[''], clearlist=True, per_eb=False, calcsb=False, pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Compute list of clean images to be produced

Create a list of images to be cleaned.

Keyword Arguments


pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
    determines the values of all context defined pipeline inputs automatically.
    In 'interactive' mode the user can set the pipeline context defined
    parameters manually.  In 'getinputs' mode the user can check the settings
    of all pipeline parameters without running the task.
    default: 'automatic'.

--- pipeline parameter arguments which can be set in any pipeline mode

specmode -- Frequency imaging mode, 'mfs', 'cont', 'cube', 'repBW'. \'\' defaults to
    'cube' if intent parameter includes 'TARGET' otherwise 'mfs'.
    default: ''
    example: 'mfs', 'cont', 'cube', 'repBW'
    example:
        specmode='mfs' produce one image per source and spw
        specmode='cont' produce one image per source and aggregate over all
             specified spws
        specmode='cube' produce an LSRK frequency cube, channels are specified
             in frequency
        specmode='repBW' produce an LSRK frequency cube at representative channel width

hm_cell -- Cell size (x, y)
    default '' Compute cell size based on the UV coverage of all the fields
         to be imaged and use a 5 pix per beam sampling.
         The pix per beam specification uses the above default cell size
         ('5ppb') and scales it accordingly.
    example: ['0.5arcsec', '0.5arcsec'] '3ppb'

hm_imsize -- Image X and Y size in pixels or PB level for single field. The
    explicit sizes must be even and divisible by 2,3,5,7 only.
    default: '' The default values are derived as follows:
         1. Determine phase center and spread of field centers around it. 
         2. Set the size of the image to cover the spread of field centers plus
            a border of width 0.75 * beam radius, to first null.
         3. Divide X and Y extents by cell size to arrive at the number of
	    pixels required. 
         The PB level setting for single fields leads to an imsize extending
         to the specified level plus 5% padding in all directions.
    example: [120, 120], '0.3pb'

calmaxpix -- Maximum image X or Y size in pixels if a calibrator is being 
    imaged ('PHASE', 'BANDPASS', 'AMPLITUDE' or 'FLUX').
    default: 300
    example: 300

width -- Output channel width.
    default: '' Difference in frequency between first 2 selected channels.
        for frequency mode images.
    example: '24.2kHz'
             'pilotimage' for 15 MHz / 8 channel heuristic

nbins -- Channel binning factors per spw
    default: '' Binning factors for each spw. Format: 'spw1:nb1,spw2:nb2,...'
             Optional wildcard: '*:nb'
    example: '9:2,11:4,13:2,15:8'
             '*:2'

robust -- Briggs robustness parameter
    default=-999.0; example: robust=0.7;
    Options: -2.0 to 2.0; -2 (uniform)/+2 (natural)

uvtaper -- uv-taper on outer baselines
    default=[]
    example: ['10arcsec']

---- pipeline context defined parameter arguments which can be set only in
     'interactive mode'

vis -- The list of input MeasurementSets. Defaults to the list of MeasurementSets
    specified in the h_init or hif_importdata sets.
    default '': use all MeasurementSets in the context 
    example: 'ngc5921.ms', ['ngc5921a.ms', ngc5921b.ms', 'ngc5921c.ms']

intent -- Select intents for which associated fields will be imaged.
    default: 'TARGET'
    example: 'PHASE,BANDPASS'

field -- Select fields to image. Use field name(s) NOT id(s). Mosaics
    are assumed to have common source / field names.  If intent is specified
    only fields with data matching the intent will be selected. The fields
    will be selected from MeasurementSets in 'vis'.
    default: '' Fields matching matching intent, one image per target source.
    example: '3C279', 'Centaurus*', '3C279,J1427-421'

spw -- Select spectral window/channels to image.
    default: '' Individual images will be computed for all science spectral
    windows.
    example: '9'

contfile -- Name of file with frequency ranges to use for continuum images.
    default: 'cont.dat'
    example: 'mycont.dat'

linesfile -- Name of file with line frequency ranges to exclude for continuum images.
    default: 'lines.dat'
    example: 'mylines.dat'

uvrange -- Select a set of uv ranges ro image.
    default: '' All uv data is included
    example: '0~1000klambda', ['0~100klambda', 100~1000klambda]

phasecenter -- Direction measure or field id of the image center.
    default: '' The default phase center is set to the mean of the field
        directions of all fields that are to be image together.
    example: 0, 'J2000 19h30m00 -40d00m00' 

nchan -- Total number of channels in the output image(s)
    default: -1 Selects enough channels to cover the data selected by
        spw consistent with start and width.
    example: 100

start -- First channel for frequency mode images.
    default '' Starts at first input channel of the spw.
    example: '22.3GHz'

clearlist -- Clear previous iamge target list.
    default True
    example: False

per_eb -- Make an image target per EB.
    default False
    example: True

calcsb -- Force (re-)calculation of sensitivities and beams
            default=False
            Options: False, True

--- pipeline task execution modes
dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: False

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

Output

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
    the results object for the pipeline task is returned.

Description

Generate a list of images to be cleaned. By default the list will include
one image per science target per spw. Calibrator targets can be selected
by setting appropriate values for intent.

By default the output image cellsize is set to the minimum cell size
consistent with the UV coverage.

By default the image size in pixels is set to values determined by the
cell size and the single dish beam size. If a calibrator is being
imaged (intents 'PHASE', 'BANDPASS', 'FLUX' or 'AMPLITUDE') then the
image dimensions are limited to 'calmaxpix' pixels.

By default science target images are cubes and calibrator target images
are single channel. Science target images may be mosaics or single fields.

Issues

TBD
Examples

1. Make a list of science target images to be cleaned, one image per science 
spw.

    hif_makeimlist()

2. Make a list of PHASE and BANDPASS calibrator targets to be imaged,
one image per science spw.

    hif_makeimlist(intent='PHASE,BANDPASS')

3. Make a list of PHASE calibrator images observed in spw 1, images limited to 
50 pixels on a side.

    hif_makeimlist(intent='PHASE',spw='1',calmaxpix=50)


        """
        if type(vis)==str: vis=[vis]
        if type(hm_imsize)==str: hm_imsize=[hm_imsize]
        if type(hm_cell)==str: hm_cell=[hm_cell]
        if type(uvtaper)==str: uvtaper=[uvtaper]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['imagename'] = imagename
        mytmp['intent'] = intent
        mytmp['field'] = field
        mytmp['spw'] = spw
        mytmp['contfile'] = contfile
        mytmp['linesfile'] = linesfile
        mytmp['uvrange'] = uvrange
        mytmp['specmode'] = specmode
        mytmp['outframe'] = outframe
        mytmp['hm_imsize'] = hm_imsize
        mytmp['hm_cell'] = hm_cell
        mytmp['calmaxpix'] = calmaxpix
        mytmp['phasecenter'] = phasecenter
        mytmp['nchan'] = nchan
        mytmp['start'] = start
        mytmp['width'] = width
        mytmp['nbins'] = nbins
        mytmp['robust'] = robust
        mytmp['uvtaper'] = uvtaper
        mytmp['clearlist'] = clearlist
        mytmp['per_eb'] = per_eb
        mytmp['calcsb'] = calcsb
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_makeimlist.xml')

        casalog.origin('hif_makeimlist')
        if trec.has_key('hif_makeimlist') and casac.utils().verify(mytmp, trec['hif_makeimlist']) :
	    result = task_hif_makeimlist.hif_makeimlist(vis, imagename, intent, field, spw, contfile, linesfile, uvrange, specmode, outframe, hm_imsize, hm_cell, calmaxpix, phasecenter, nchan, start, width, nbins, robust, uvtaper, clearlist, per_eb, calcsb, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
