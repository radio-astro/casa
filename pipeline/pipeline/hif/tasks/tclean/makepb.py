###
###  Make a PB
###  - MS and selections
###  - Defineimage (try to just reuse coordinatesystem)
###  - im.makePB
import os.path
import shutil

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.jobrequest import casa_tasks

LOG = logging.get_logger(__name__)


def makePB(vis='', field='', spw='', timerange='', uvrange='',
           antenna='', observation='', intent='', scan='', mode='mfs',
           imtemplate='', outimage='', pblimit=0.2):
    """
    Make a PB image using the imager tool, onto a specified image coordinate
    system.

    This function can be used along with tclean to make .pb images for
    gridders that do not already do it (i.e. other than mosaic, awproject).

    This script takes an image to use as a template coordinate system,
    attempts to set up an identical coordinate system with the old imager
    tool, makes a PB for the telescope listed in the MS observation subtable,
    and regrids it (just in case) to the target coordinate system). This can
    be used for single fields and mosaics.
    """
    me = casatools.measures
    qa = casatools.quanta

    with casatools.TableReader(os.path.join(vis, 'OBSERVATION')) as tb:
        telescope_name = tb.getcol('TELESCOPE_NAME')[0]

    LOG.info('MAKEPB : Making a PB image using the imager tool')
    with casatools.ImageReader(imtemplate) as ia:
        csysa = ia.coordsys()
        csys = csysa.torecord()
        shp = ia.shape()

    dirs = csys['direction0']
    phasecenter = me.direction(dirs['system'],
                               qa.quantity(dirs['crval'][0], dirs['units'][0]),
                               qa.quantity(dirs['crval'][1], dirs['units'][1]))
    cellx = qa.quantity(dirs['cdelt'][0], dirs['units'][0])
    celly = qa.quantity(dirs['cdelt'][1], dirs['units'][1])
    nchan = shp[3]
    start = qa.quantity(csysa.referencevalue()['numeric'][3],
                        csysa.units()[3])  # assumes refpix is zero
    step = qa.quantity(csysa.increment()['numeric'][3],
                       csysa.units()[3])

    LOG.info('MAKEPB : Starting imager tool')
    with casatools.ImagerReader(vis) as im:
        im.selectvis(field=field, spw=spw, time=timerange, intent=intent,
                     scan=scan, uvrange=uvrange, baseline=antenna,
                     observation=observation)
        im.defineimage(nx=shp[0], ny=shp[0], phasecenter=phasecenter,
                       cellx=qa.tos(cellx), celly=qa.tos(celly), nchan=nchan,
                       start=start, step=step, mode=mode)
        im.setvp(dovp=True, telescope=telescope_name)
        im.makeimage(type='pb', image=outimage+'.tmp')

    LOG.info('MAKEPB : Regrid to desired coordinate system')
    job = casa_tasks.imregrid(imagename=outimage+'.tmp', template=imtemplate,
                              output=outimage, overwrite=True,
                              asvelocity=False)
    job.execute(dry_run=False)

    shutil.rmtree(outimage+'.tmp')

    LOG.info('MAKEPB : Set mask to pblimit')
    with casatools.ImageReader(outimage) as ia:
        ia.calcmask('"%s" > %s' % (outimage, str(pblimit)))
