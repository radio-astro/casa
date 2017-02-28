###
###  Make a PB
###  - MS and selections
###  - Defineimage (try to just reuse coordinatesystem)
###  - im.makePB

#!/usr/bin/env python

import os;
import shutil;
from  casac import *;
from tasks import imregrid
from numpy import fabs

def makePB(vis='',field='',spw='',timerange='',uvrange='',antenna='',observation='',intent='',scan='', imtemplate='',outimage='',pblimit=0.2):
    
    """ Make a PB image using the imager tool, onto a specified image coordinate system 

         This function can be used along with tclean to make .pb images for gridders that
         do not already do it (i.e. other than mosaic, awproject)

         This script takes an image to use as a template coordinate system, 
         attempts to set up an identical coordinate system with the old imager tool, 
         makes a PB for the telescope listed in the MS observation subtable, and 
         regrids it (just in case) to the target coordinate system). This can be used for
         single fields and mosaics.

    """

    tb = casac.table()
    im = casac.imager()
    ia = casac.image()
    me = casac.measures()
    qa = casac.quanta()

    print 'MAKEPB : Making a PB image using the imager tool'

    tb.open(vis+'/OBSERVATION')
    tel = tb.getcol('TELESCOPE_NAME')[0]
    tb.close()

    tb.open(vis+'/SPECTRAL_WINDOW')
    mfreqref = tb.getcol('MEAS_FREQ_REF')[0]
    tb.close()
    if mfreqref == 64:
       print 'MAKEPB : This function is using old imager tool, Undefined frame may not be handled properly.'

    print 'MAKEPB : Making PB for ', tel

    ia.open(imtemplate)
    csysa = ia.coordsys()
    csys = csysa.torecord()
    shp = ia.shape()
    ia.close()
    stokes = 'I'
    dirs = csys['direction0']
    phasecenter = me.direction(dirs['system'], qa.quantity(dirs['crval'][0],dirs['units'][0]) , qa.quantity(dirs['crval'][1],dirs['units'][1]) )
    cellx=qa.quantity(fabs(dirs['cdelt'][0]),dirs['units'][0])
    celly=qa.quantity(fabs(dirs['cdelt'][1]),dirs['units'][1])
    nchan=shp[3]
    start=qa.quantity( csysa.referencevalue()['numeric'][3], csysa.units()[3] )  ## assumes refpix is zero
    mestart = me.frequency('LSRK',start)
    step=qa.quantity( csysa.increment()['numeric'][3], csysa.units()[3] )

    smode='mfs'
    if nchan>1:
        smode='frequency'

    print 'MAKEPB : Starting imager tool'

    im.open(vis)
    im.selectvis(field=field,spw=spw,time=timerange,intent=intent,scan=scan,uvrange=uvrange,baseline=antenna,observation=observation)
    im.defineimage(nx=shp[0],ny=shp[0],phasecenter=phasecenter,cellx=qa.tos(cellx),celly=qa.tos(celly),nchan=nchan,start=mestart,step=step,mode=smode)
    im.setvp(dovp=True,telescope=tel)
    im.makeimage(type='pb',image=outimage+'.tmp')
    im.close()

    if mfreqref == 64: # skip this step if the frame is 'Undefined'
 
        shutil.copytree(outimage+'.tmp', outimage)
   
    else:
    
        print 'MAKEPB : Regrid to desired coordinate system'
    
        imregrid(imagename=outimage+'.tmp', template=imtemplate,output=outimage,overwrite=True,asvelocity=False)

    

    shutil.rmtree(outimage+'.tmp')

    print 'MAKEPB : Set mask to pblimit'

    ia.open(outimage)
    ia.calcmask( "'"+outimage+"'>"+str(pblimit) )
    ia.close()

