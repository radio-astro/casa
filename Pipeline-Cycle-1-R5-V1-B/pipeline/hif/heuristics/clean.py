import decimal
import math
import numpy as np
import re
import types

import cleanhelper

import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)



class CleanHeuristics(object):

    def __init__(self, context, vislist, spw):
        self.context = context
        if type(vislist) is types.ListType:
            self.vislist = vislist
        else:
            self.vislist = [vislist]
        self.namer = filenamer.Image()

        self.beam_radius = {}

        # base heuristics on first spw in list from first vis set, assume all
        # other vis sets same for now
        ms = self.context.observing_run.get_ms(name=self.vislist[0])
        spwid = int(spw.split(',')[0])
        spw = ms.get_spectral_window(spwid)

        # get the diameter of the smallest antenna used among all vis sets
        diameters = []
        for vis in self.vislist:
            ms = self.context.observing_run.get_ms(name=vis)
            antennas = ms.antennas
            for antenna in antennas:
                diameters.append(antenna.diameter)
        smallest_diameter = np.min(np.array(diameters))

        ref_frequency = float(
          spw.ref_frequency.to_units(measures.FrequencyUnits.HERTZ))
        # use the smallest antenna diameter and the reference frequency
        # to estimate the primary beam radius -
        #radius of first null = 1.22*lambda/D
        self.beam_radius = '%sarcsec' % (
          (1.22 * (3.0e8/ref_frequency) / smallest_diameter) * \
          (180.0 * 3600.0 / math.pi))

    def field(self, intent, field):
        result = []

        for vis in self.vislist:
            ms = self.context.observing_run.get_ms(name=vis)
            fields = ms.fields

            field_list = []
            if field is not None:
                # field set explicitly
                if type(field) is not types.ListType:
                    field_names = [field]
                else:
                    field_names = field

                # 'escape' regexp control characters in field name, except for
                # * which people use as a wildcard character - replace * by .*
                # to convert it to regexp form
                for i,fld in enumerate(field_names):
                    for char in '()+?.^$[]{}|': 
                        fld = fld.replace(char, '\%s' % char)
                    fld = fld.replace('*', '.*')
                    field_names[i] = fld

                # convert to field ids
                for field_name in field_names:
                    field_list += [fld.id for fld in fields if re.search(
                      pattern=field_name, string=fld.name)]

            if intent is not None:
                # pattern matching to allow intents of form *TARGET* to work
                re_intent = intent.replace('*', '.*')
                field_list = [fld.id for fld in fields if
                  fld.id in field_list and re.search(pattern=re_intent,
                  string=str(fld.intents))]

            # this will be a mosaic if there is more than 1 field_id for any 
            # measurement set - probably needs more work, what if want to
            # mosaic together single fields in multiple measurement sets?
            self._mosaic = len(field_list) > 1
 
            field_string = ','.join(str(fld_id) for fld_id in field_list)
            result.append(field_string)

        return result

    def imagename(self, output_dir=None, intent=None, field=None, spw=None):
        try:
            nameroot = self.context.project_structure.ousstatus_entity_id
            # need to sanitize the nameroot here because when it's added
            # to filenamer as an asdm, os.path.basename is run on it with
            # undesirable results.
            nameroot = filenamer.sanitize(nameroot)
        except:
            nameroot = 'multivis'
        self.namer._associations.asdm(nameroot)

        if output_dir:
            self.namer.output_dir(output_dir)

        self.namer.stage(self.context.stage)
        if intent:
            self.namer.intent(intent)
        if field:
            self.namer.source(field)
        if spw:
            self.namer.spectral_window(spw)

        # filenamer returns a sanitized filename (i.e. one with 
        # illegal characters replace by '_'), no need to check
        # the name components individually.
        imagename = self.namer.get_filename()
        return imagename

    def imagermode(self, intent, field):
        # the field heuristic which decides whether this is a mosaic or not
        self.field(intent, field)

        if self._mosaic:
            return 'mosaic'
        else:
            return 'csclean'

    def imsize(self, fields, cell, max_pixels=None):
        # get spread of beams
        ignore, xspread, yspread = self.phasecenter(fields, centreonly=False)

        cqa = casatools.quanta

        cellx = cell[0]
        if len(cell) > 1:
            celly = cell[1]
        else:
            celly = cell[0]

        # get cell and beam sizes in arcsec
        cellq = cqa.quantity(cellx)
        cqa.convert(cellq, 'arcsec')
        cellvx = cellq['value']

        cellq = cqa.quantity(celly)
        cqa.convert(cellq, 'arcsec')
        cellvy = cellq['value']

        beam_radiusq = cqa.quantity(self.beam_radius)
        cqa.convert(beam_radiusq, 'arcsec')
        beam_radiusv = beam_radiusq['value']

        # set size of image to spread of field centres plus a
        # border of 0.75 * beam radius (radius is to first null)
        # wide
        nxpix = int((1.5 * beam_radiusv + xspread) / cellvx)
        nypix = int((1.5 * beam_radiusv + yspread) / cellvy)

        if max_pixels is not None:
            nxpix = min(nxpix, max_pixels)
            nypix = min(nypix, max_pixels)

        # set nxpix, nypix to next highest 'composite number'
        nxpix = cleanhelper.cleanhelper.getOptimumSize(nxpix)
        nypix = cleanhelper.cleanhelper.getOptimumSize(nypix)

        return (nxpix, nypix)

    def phasecenter(self, fields, centreonly=True):
        mdirections = []
        for ivis, vis in enumerate(self.vislist):
            ms = self.context.observing_run.get_ms(name=vis)
            visfields = fields[ivis]
            if visfields == '':
                continue
            visfields = visfields.split(',')
            visfields = map(int, visfields)

            for field in visfields:
                # get field centres as measures
                fieldobj = ms.get_fields(field_id=field)[0]
                phase_dir = fieldobj.mdirection
                mdirections.append(phase_dir)

        # sanity check - for single field images the field centres from
        # different measurement sets should be coincident and can
        # collapse the list 
        if not self._mosaic:
            for mdirection in mdirections:
                if mdirection != mdirections[0]:
                    LOG.warning('Separation between field centres: %s' % (
                      casatools.measures.separation(mdirection, mdirections[0])))
#                    raise Exception, \
#                      'non-identical field centers in single field image' 
            mdirections = [mdirections[0]]

        # it should be easy to calculate some 'average' direction
        # from the contributing fields but it doesn't seem to be
        # at the moment - no conversion beween direction measures,
        # no calculation of a direction from a direction and an
        # offset. Consequently, what follows is a bit crude.

        # First, find the offset of all field from field 0.
        cme = casatools.measures
        cqa = casatools.quanta
        xsep = []
        ysep = []
        for mdirection in mdirections:
            pa = cme.posangle(mdirections[0], mdirection)
            sep = cme.separation(mdirections[0], mdirection)
            xs = cqa.mul(sep, cqa.sin(pa))
            ys = cqa.mul(sep, cqa.cos(pa))
            xs = cqa.convert(xs, 'arcsec')
            ys = cqa.convert(ys, 'arcsec')
            xsep.append(cqa.getvalue(xs))
            ysep.append(cqa.getvalue(ys))

        xsep = np.array(xsep)
        ysep = np.array(ysep)
        xspread = xsep.max() - xsep.min()
        yspread = ysep.max() - ysep.min()
        xcen = xsep.min() + xspread / 2.0
        ycen = ysep.min() + yspread / 2.0

        # get direction of image centre crudely by adding offset
        # of centre to ref values of first field.
        ref =  cme.getref(mdirections[0])
        md = cme.getvalue(mdirections[0])
        m0 = cqa.quantity(md['m0'])
        m1 = cqa.quantity(md['m1'])

        m0 = cqa.add(m0, cqa.div('%sarcsec' % xcen, cqa.cos(m1)))
        m1 = cqa.add(m1, '%sarcsec' % ycen)

        # convert to strings (CASA 4.0 returns as list for some reason
        # hence 0 index)
        if ref=='J2000' or ref=='B1950':
            m0 = cqa.time(m0, prec=7)[0]
        else:
            m0 = cqa.angle(m0)[0]
        m1 = cqa.angle(m1)[0]

        if centreonly:
            return '%s %s %s' % (ref, m0, m1)
        else:
            return '%s %s %s' % (ref, m0, m1), xspread, yspread

    def width(self, spw):
        # get the spw from the first vis set, assume all others the same for
        # now
        ms = self.context.observing_run.get_ms(name=self.vislist[0])
        spw = ms.get_spectral_window(spw)
        # negative widths appear to confuse clean,
        if spw.channels[1].high > spw.channels[0].high:
            width = spw.channels[1].high - spw.channels[0].high
        else:
            width = spw.channels[0].high - spw.channels[1].high
        # increase width slightly to try to avoid the error:
        # WARN SubMS::convertGridPars *** Requested new channel width is
        # smaller than smallest original channel width.
        width = decimal.Decimal('1.00001') * width
        width = str(width)
        return width


