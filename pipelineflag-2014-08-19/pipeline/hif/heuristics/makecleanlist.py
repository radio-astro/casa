import decimal
import math
import numpy as np
import os.path
import re
import types

import pipeline.infrastructure.filenamer as filenamer
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import cleanhelper
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class MakeCleanListHeuristics(object):

    def __init__(self, context, vislist, spw):
        self.context = context
        if type(vislist) is types.ListType:
            self.vislist = vislist
        else:
            self.vislist = [vislist]

        # split spw into list of spw parameters for 'clean' 
        spwlist = spw.replace('[','').replace(']','').strip()
        spwlist = spwlist.split("','")
        spwlist[0] = spwlist[0].strip("'")
        spwlist[-1] = spwlist[-1].strip("'")

        # find all the spwids present in the list
        p=re.compile(r"[ ,]+(\d+)")
        spwids = set()
        for spwclean in spwlist:
            spwidsclean = p.findall(' %s' % spwclean)
            spwidsclean = map(int, spwidsclean)
            spwids.update(spwidsclean)

        # calculate beam radius for all spwids, saves repetition later 
        self.beam_radius = {}

        # get the diameter of the smallest antenna used among all vis sets
        diameters = []
        for vis in self.vislist:
            ms = self.context.observing_run.get_ms(name=vis)
            antennas = ms.antennas
            for antenna in antennas:
                diameters.append(antenna.diameter)
        smallest_diameter = np.min(np.array(diameters))

        # get spw info from first vis set, assume spws uniform
        # across datasets
        ms = self.context.observing_run.get_ms(name=self.vislist[0])
        for spwid in spwids:
            spw = ms.get_spectral_window(spwid)
            ref_frequency = float(
              spw.ref_frequency.to_units(measures.FrequencyUnits.HERTZ))

            # use the smallest antenna diameter and the reference frequency
            # to estimate the primary beam radius -
            # radius of first null in arcsec = 1.22*lambda/D
            self.beam_radius[spwid] = \
              (1.22 * (3.0e8/ref_frequency) / smallest_diameter) * \
              (180.0 * 3600.0 / math.pi)

    def field_intent_list(self, intent, field):
        intent_list = intent.split(',')
        field_list = utils.safe_split(field)

        field_intent_result = set()

        for vis in self.vislist:
            ms = self.context.observing_run.get_ms(name=vis)
            fields = ms.fields

            if intent.strip() is not '':
                for eachintent in intent_list:
                    re_intent = eachintent.replace('*', '.*')
                    field_intent_result.update(
                      [(fld.name, eachintent) for fld in fields if 
                      re.search(pattern=re_intent, string=str(fld.intents))])

                if field.strip() is not '':
                    # remove items from field_intent_result that are not
                    # consistent with the fields in field_list
                    temp_result = set()
                    re_field = field
                    for char in '()+?.^$[]{}|':
                        re_field = re_field.replace(char, '\%s' % char)
                    re_field = re_field.replace('*', '.*')
                    for eachfield in field_list:
                        temp_result.update([fir for fir in field_intent_result
                          if re.search(pattern=re_field, string=fir[0])])
                    field_intent_result = temp_result

            else:
                if field.strip() is not '':
                    for f in field_list:
                        for char in '()+?.^$[]{}|':
                            f = f.replace(char, '\%s' % char)
                        f = f.replace('*', '.*')
                        fintents_list = [fld.intents for fld in fields if 
                          re.search(pattern=f, string=fld.name)]
                        for fintents in fintents_list:
                            for fintent in fintents:
                                field_intent_result.update((f, fintent))

        # eliminate redundant copies of field/intent keys that map to the
        # same data - to prevent duplicate images being produced

        done_vis_scanids = []

        for field_intent in list(field_intent_result):
            field = field_intent[0]
            intent = field_intent[1]
 
            # regex for string matching - escape likely problem chars.
            re_field = field.replace('*', '.*')
            re_field = re_field.replace('[', '\[')
            re_field = re_field.replace(']', '\]')
            re_field = re_field.replace('(', '\(')
            re_field = re_field.replace(')', '\)')
            re_field = re_field.replace('+', '\+')

            vis_scanids = {}
            for vis in self.vislist:
                ms = self.context.observing_run.get_ms(name=vis)

                scanids = [scan.id for scan in ms.scans if
                  intent in scan.intents and
                  re.search(pattern=re_field, string=str(scan.fields))]
                scanids.sort()

                vis_scanids[vis] = scanids

            if vis_scanids in done_vis_scanids:
                LOG.warn(
                  'field: %s intent: %s is a duplicate - removing from cleanlist' %
                  (field, intent))
                field_intent_result.discard(field_intent)
            else:
                done_vis_scanids.append(vis_scanids)
                
        return field_intent_result

    def beam(self, spwspec):
        # reset state of imager
        casatools.imager.done()

        # get the spwids in spwspec
        p=re.compile(r"[ ,]+(\d+)")
        spwids = p.findall(' %s' % spwspec)
        spwids = map(int, spwids)
        spwids = list(set(spwids))

        # find largest beam among spws in spwspec
        beam = 0.0
        for spwid in spwids:
            beam = max(beam, self.beam_radius[spwid])

        return beam

    def cell(self, field_intent_list, spwspec, oversample=3.0):
        # reset state of imager
        casatools.imager.done()

        # get the spwids in spwspec - imager tool likes these rather than 
        # a string
        p=re.compile(r"[ ,]+(\d+)")
        spwids = p.findall(' %s' % spwspec)
        spwids = map(int, spwids)
        spwids = list(set(spwids))

        # find largest beam among spws in spwspec
        beam = self.beam(spwspec)

        # put code in try-block to ensure that imager.done gets
        # called at the end
        cell = None
        valid_data = {}
        try:
            cellvs = []
            for field_intent in field_intent_list:
                # select data to be imaged
                valid_data[field_intent] = False
                for vis in self.vislist:
                    ms = self.context.observing_run.get_ms(name=vis)
                    scanids = [scan.id for scan in ms.scans if
                      field_intent[1] in scan.intents and 
                      field_intent[0] in [fld.name for fld in scan.fields]]
                    scanids = str(scanids)
                    scanids = scanids.replace('[', '')
                    scanids = scanids.replace(']', '')
                    try:
                        casatools.imager.selectvis(vis=vis,
                          field=field_intent[0], spw=spwids, scan=scanids,
                          usescratch=False)
                        # flag to say that imager has some valid data
                        # to work on
                        valid_data[field_intent] = True
                    except:
                        pass

                if not valid_data[field_intent]:
                    # no point carrying on for this field/intent
                    LOG.debug('No data for SpW %s field %s' %
                      (spwids, field_intent[0]))
                    continue

                casatools.imager.weight(type='natural')

                # use imager.advise to get the maximum cell size
                aipsfieldofview = '%4.1farcsec' % (2.0 * beam)
                rtn = casatools.imager.advise(takeadvice=False,
                  amplitudeloss=0.5, fieldofview=aipsfieldofview)
                if not rtn[0]:
                    # advise can fail if all selected data are flagged
                    # - not documented but assuming bool in first field of returned
                    # record indicates success or failure
                    LOG.warning('imager.advise failed for field/intent %s/%s spw %s - no valid data?' 
                      % (field_intent[0], field_intent[1], spwids))
                    valid_data[field_intent] = False
                else:
                    cellv = rtn[2]['value']
                    cellu = rtn[2]['unit']
                    cellv /= oversample
                    cellvs.append(cellv)
                    LOG.debug('Cell (oversample %s) for %s/%s spw %s: %s' % (
                      oversample, field_intent[0], field_intent[1], spwspec, cellv))

            if cellvs:
                # cell that's good for all field/intents
                cell = '%.3f%s' % (min(cellvs), cellu)
                LOG.debug('RESULT cell for spw %s: %s' % (spwspec, cell))
            else:
                cell = 'invalid'

        finally:
            casatools.imager.done()

        return [cell], valid_data

    def has_data(self, field_intent_list, spwspec):
        # reset state of imager
        casatools.imager.done()

        # get the spwids in spwspec - imager tool likes these rather than 
        # a string
        p=re.compile(r"[ ,]+(\d+)")
        spwids = p.findall(' %s' % spwspec)
        spwids = map(int, spwids)
        spwids = list(set(spwids))

        # put code in try-block to ensure that imager.done gets
        # called at the end
        valid_data = {}
        try:
            # select data to be imaged
            for field_intent in field_intent_list:
                valid_data[field_intent] = False
                for vis in self.vislist:
                    ms = self.context.observing_run.get_ms(name=vis)
                    scanids = [scan.id for scan in ms.scans if
                      field_intent[1] in scan.intents and 
                      field_intent[0] in [fld.name for fld in scan.fields]]
                    scanids = str(scanids)
                    scanids = scanids.replace('[', '')
                    scanids = scanids.replace(']', '')
                    try:
                        casatools.imager.selectvis(vis=vis,
                          field=field_intent[0], spw=spwids, scan=scanids,
                          usescratch=False)
                        valid_data[field_intent] = True
                    except:
                        pass

                if not valid_data[field_intent]:
                    LOG.debug('No data for SpW %s field %s' %
                      (spwids, field_intent[0]))

        finally:
            casatools.imager.done()

        return valid_data

    def imagermode(self, intent, field):
        # the field heuristic which decides whether this is a mosaic or not
        self.field(intent, field)

        if self._mosaic:
            return 'mosaic'
        else:
            return 'csclean'

    def phasecenter(self, fields, centreonly=True):

        cme = casatools.measures

        mdirections = []
        for ivis, vis in enumerate(self.vislist):

	    # Get the visibilities
            ms = self.context.observing_run.get_ms(name=vis)
            visfields = fields[ivis]
            if visfields == '':
                continue
            visfields = visfields.split(',')
            visfields = map(int, visfields)

	    # Get the phase directions and convert to ICRS
            for field in visfields:
                # get field centres as measures
                fieldobj = ms.get_fields(field_id=field)[0]
                phase_dir = cme.measure(fieldobj.mdirection, 'ICRS')
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
        if ref=='ICRS' or ref=='J2000' or ref=='B1950':
            m0 = cqa.time(m0, prec=9)[0]
        else:
            m0 = cqa.angle(m0, prec=8)[0]
        m1 = cqa.angle(m1, prec=8)[0]

        if centreonly:
            return '%s %s %s' % (ref, m0, m1)
        else:
            return '%s %s %s' % (ref, m0, m1), xspread, yspread

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

    def imsize(self, fields, cell, beam, max_pixels=None):
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

        beam_radiusq = cqa.quantity(beam)
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

        return [nxpix, nypix]

    def imagename(self, output_dir=None, intent=None, field=None, spwspec=None):
        try:
            nameroot = self.context.project_structure.ousstatus_entity_id
	    if nameroot == 'unknown':
	        nameroot = 'oussid'
            # need to sanitize the nameroot here because when it's added
            # to filenamer as an asdm, os.path.basename is run on it with
            # undesirable results.
            nameroot = filenamer.sanitize(nameroot)
        except:
            nameroot = 'oussid'
        namer = filenamer.Image()
        namer._associations.asdm(nameroot)

        if output_dir:
            namer.output_dir(output_dir)

        namer.stage(self.context.stage)
        if intent:
            namer.intent(intent)
        if field:
            namer.source(field)
        if spwspec:
            # find all the spwids present in the list
            p=re.compile(r"[ ,]+(\d+)")
            spwids = p.findall(' %s' % spwspec)
            spwids = list(set(spwids))
            spwids.sort()
            spw = '_'.join(spwids)
            namer.spectral_window(spw)

        # filenamer returns a sanitized filename (i.e. one with 
        # illegal characters replace by '_'), no need to check
        # the name components individually.
        imagename = namer.get_filename()
        return imagename

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
        width = decimal.Decimal('1.0001') * width
        width = str(width)
        return width

    def ncorr (self, spw):
        # get the spw from the first vis set, assume all others the same for
        # now
        ms = self.context.observing_run.get_ms(name=self.vislist[0])
        spw = ms.get_spectral_window(spw)

        # Get the data description for this spw
        dd = ms.get_data_description(spw=spw)
        if dd is None:
            LOG.debug('Missing data description for spw %s ' % spw.id)
            return 0

        # Determine the number of correlations
        #   Check that they are between 1 and 4
        ncorr = len (dd.corr_axis)
        if ncorr not in set ([1, 2, 4]):
            LOG.debug('Wrong number of correlations %s for spw %s ' % (ncorr, spw.id))
            return 0

	return ncorr

