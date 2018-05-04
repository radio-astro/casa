import numpy
import re

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure
from .imageparams_base import ImageParamsHeuristics

LOG = infrastructure.get_logger(__name__)


class ImageParamsHeuristicsVlassSeCont(ImageParamsHeuristics):

    def __init__(self, vislist, spw, observing_run, imagename_prefix='', proj_params=None, contfile=None, linesfile=None, imaging_params={}):
        ImageParamsHeuristics.__init__(self, vislist, spw, observing_run, imagename_prefix, proj_params, contfile, linesfile, imaging_params)
        self.imaging_mode = 'VLASS-SE-CONT'

    # niter
    def niter_correction(self, niter, cell, imsize, residual_max, threshold):
        if niter:
            return int(niter)
        else:
            return 20000

    def rms_threshold(self, rms, nsigma):
        try:
            LOG.info('Threshold nsigma multiplier: %s', nsigma)
            threshold = rms * float(nsigma)
            LOG.info('Setting new threshold to [robust rms * nsigma]: {th}'.format(th=threshold))
        except TypeError:
            threshold = None
        return threshold

    def deconvolver(self, specmode, spwspec):
        return 'mtmfs'

    def robust(self):
        return 1.0

    def gridder(self, intent, field):
        return 'mosaic'

    def cell(self, beam=None, pixperbeam=None):
        return ['0.6arcsec']

    def imsize(self, fields=None, cell=None, primary_beam=None, sfpblimit=None, max_pixels=None, centreonly=None):
        return [11520, 11520]

    def threshold_nsigma(self):
        return 4.0

    def reffreq(self):
        return '3.0GHz'

    def cyclefactor(self):
        return 3.0

    def cycleniter(self):
        return 2000

    def scales(self):
        return [0]

    def uvtaper(self, beam_natural=None, protect_long=None):
        return []

    def uvrange(self):
        return None

    def mask(self):
        return ''

    def buffer_radius(self):
        return 1000.

    def specmode(self):
        return 'mfs'

    def intent(self):
        return 'TARGET'

    def nterms(self):
        return 2

    def stokes(self):
        return 'I'

    def pb_correction(self):
        return False

    def conjbeams(self):
        return False

    def get_sensitivity(self, ms_do, field, spw, chansel, specmode, cell, imsize, weighting, robust, uvtaper):
        return 0.0, None, None

    def find_fields(self, distance='0deg', phase_center=None, matchregex=''):

        # Created STM 2016-May-16 use center direction measure
        # Returns list of fields from msfile within a rectangular box of size distance

        qa = casatools.quanta
        me = casatools.measures
        tb = casatools.table

        msfile = self.vislist[0]

        fieldlist = []

        phase_center = phase_center.split()
        center_dir = me.direction(phase_center[0], phase_center[1], phase_center[2])
        center_ra = center_dir['m0']['value']
        center_dec = center_dir['m1']['value']

        try:
            qdist = qa.toangle(distance)
            qrad = qa.convert(qdist, 'rad')
            maxrad = qrad['value']
        except:
            print('ERROR: cannot parse distance {}'.format(distance))
            return

        try:
            tb.open(msfile + '/FIELD')
        except:
            print('ERROR: could not open {}/FIELD'.format(msfile))
            return
        field_dirs = tb.getcol('PHASE_DIR')
        field_names = tb.getcol('NAME')
        tb.close()

        (nd, ni, nf) = field_dirs.shape
        print('Found {} fields'.format(nf))

        # compile field dictionaries
        ddirs = {}
        flookup = {}
        for i in range(nf):
            fra = field_dirs[0, 0, i]
            fdd = field_dirs[1, 0, i]
            rapos = qa.quantity(fra, 'rad')
            decpos = qa.quantity(fdd, 'rad')
            ral = qa.angle(rapos, form=["tim"], prec=9)
            decl = qa.angle(decpos, prec=10)
            fdir = me.direction('J2000', ral[0], decl[0])
            ddirs[i] = {}
            ddirs[i]['ra'] = fra
            ddirs[i]['dec'] = fdd
            ddirs[i]['dir'] = fdir
            fn = field_names[i]
            ddirs[i]['name'] = fn
            if fn in flookup:
                flookup[fn].append(i)
            else:
                flookup[fn] = [i]
        print('Cataloged {} fields'.format(nf))

        # Construct offset separations in ra,dec
        print('Looking for fields with maximum separation {}'.format(distance))
        nreject = 0
        skipmatch = matchregex == '' or matchregex == []
        for i in range(nf):
            dd = ddirs[i]['dir']
            dd_ra = dd['m0']['value']
            dd_dec = dd['m1']['value']
            sep_ra = abs(dd_ra - center_ra)
            if sep_ra > numpy.pi:
                sep_ra = 2.0 * numpy.pi - sep_ra
            # change the following to use dd_dec 2017-02-06
            sep_ra_sky = sep_ra * numpy.cos(dd_dec)

            sep_dec = abs(dd_dec - center_dec)

            ddirs[i]['offset_ra'] = sep_ra_sky
            ddirs[i]['offset_ra'] = sep_dec

            if sep_ra_sky <= maxrad:
                if sep_dec <= maxrad:
                    if skipmatch:
                        fieldlist.append(i)
                    else:
                        # test regex against name
                        foundmatch = False
                        fn = ddirs[i]['name']
                        for rx in matchregex:
                            mat = re.findall(rx, fn)
                            if len(mat) > 0:
                                foundmatch = True
                        if foundmatch:
                            fieldlist.append(i)
                        else:
                            nreject += 1

        print('Found {} fields within {}'.format(len(fieldlist), distance))
        if not skipmatch:
            print('Rejected {} distance matches for regex'.format(nreject))

        return fieldlist
