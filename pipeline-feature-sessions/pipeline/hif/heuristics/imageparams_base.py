import os.path
import decimal
import math
import numpy as np
import re
import types
import collections

import cleanhelper

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.contfilehandler as contfilehandler
import pipeline.domain.measures as measures

LOG = infrastructure.get_logger(__name__)


class ImageParamsHeuristics(object):

    '''Image parameters heuristics base class. One instance is made per make/editimlist
       call. There are subclasses for different imaging modes such as ALMA
       or VLASS.'''

    def __init__(self, vislist, spw, observing_run, imagename_prefix='', science_goals=None, contfile=None, linesfile=None):
        self.imaging_mode = 'BASE'

        self.observing_run = observing_run
        self.imagename_prefix = imagename_prefix
        self.science_goals = science_goals

        if type(vislist) is types.ListType:
            self.vislist = vislist
        else:
            self.vislist = [vislist]

        self.spw = spw
        self.contfile = contfile
        self.linesfile = linesfile

        # split spw into list of spw parameters for 'clean' 
        spwlist = spw.replace('[','').replace(']','').strip()
        spwlist = spwlist.split("','")
        spwlist[0] = spwlist[0].strip("'")
        spwlist[-1] = spwlist[-1].strip("'")

        # find all the spwids present in the list
        p=re.compile(r"[ ,]+(\d+)")
        self.spwids = set()
        for spwclean in spwlist:
            spwidsclean = p.findall(' %s' % spwclean)
            spwidsclean = map(int, spwidsclean)
            self.spwids.update(spwidsclean)

    def beam_radius(self):

        '''Calculate beam radius for all spwids'''

        cqa = casatools.quanta

        beam_radius = {}

        # get the diameter of the smallest antenna used among all vis sets
        diameters = []
        for vis in self.vislist:
            ms = self.observing_run.get_ms(name=vis)
            antennas = ms.antennas
            for antenna in antennas:
                diameters.append(antenna.diameter)
        smallest_diameter = np.min(np.array(diameters))

        # get spw info from first vis set, assume spws uniform
        # across datasets
        ms = self.observing_run.get_ms(name=self.vislist[0])
        for spwid in self.spwids:
            spw = ms.get_spectral_window(spwid)
            ref_frequency = float(
              spw.ref_frequency.to_units(measures.FrequencyUnits.HERTZ))

            # use the smallest antenna diameter and the reference frequency
            # to estimate the primary beam radius -
            # radius of first null in arcsec = 1.22*lambda/D
            beam_radius[spwid] = \
              (1.22 * (cqa.constants('c')['value'] / ref_frequency) / smallest_diameter) * \
              (180.0 * 3600.0 / math.pi)

        return beam_radius

    def cont_ranges_spwsel(self):

        '''Determine spw selection parameters to exclude lines for mfs and cont images.'''

        # initialize lookup dictionary for all possible source names
        cont_ranges_spwsel = {}
        for ms_ref in self.observing_run.get_measurement_sets():
            for source_name in [s.name for s in ms_ref.sources]:
                cont_ranges_spwsel[source_name] = {}
                for spwid in self.spwids:
                    cont_ranges_spwsel[source_name][str(spwid)] = ''

        contfile = self.contfile if self.contfile is not None else ''
        linesfile = self.linesfile if self.linesfile is not None else ''

        # read and merge continuum regions if contfile exists
        if (os.path.isfile(contfile)):
            LOG.info('Using continuum frequency ranges from %s to calculate continuum frequency selections.' % (contfile))

            contfile_handler = contfilehandler.ContFileHandler(contfile, warn_nonexist=True)

            # Collect the merged the ranges
            for field_name in cont_ranges_spwsel.iterkeys():
                for spw_id in cont_ranges_spwsel[field_name].iterkeys():
                    cont_ranges_spwsel[field_name][spw_id] = contfile_handler.get_merged_selection(field_name, spw_id)

        # alternatively read and merge line regions and calculate continuum regions
        elif (os.path.isfile(linesfile)):
            LOG.info('Using line frequency ranges from %s to calculate continuum frequency selections.' % (linesfile))

            p=re.compile('([\d.]*)(~)([\d.]*)(\D*)')
            try:
                line_regions = p.findall(open(linesfile, 'r').read().replace('\n','').replace(';','').replace(' ',''))
            except Exception as e:
                line_regions = []
            line_ranges_GHz = []
            for line_region in line_regions:
                try:
                    fLow = casatools.quanta.convert('%s%s' % (line_region[0], line_region[3]), 'GHz')['value']
                    fHigh = casatools.quanta.convert('%s%s' % (line_region[2], line_region[3]), 'GHz')['value']
                    line_ranges_GHz.append((fLow, fHigh))
                except:
                    pass
            merged_line_ranges_GHz = [r for r in utils.merge_ranges(line_ranges_GHz)]

            # get source and spw info from first vis set, assume spws uniform
            # across datasets
            for spwid in self.spwids:
                spw = ms.get_spectral_window(spwid)
                # assemble continuum spw selection
                min_frequency = float(spw.min_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                max_frequency = float(spw.max_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                spw_sel_intervals = utils.spw_intersect([min_frequency, max_frequency], merged_line_ranges_GHz)
                spw_selection = ';'.join(['%s~%sGHz' % (spw_sel_interval[0], spw_sel_interval[1]) for spw_sel_interval in spw_sel_intervals])

                # Skip selection syntax completely if the whole spw is selected
                if (spw_selection == '%s~%sGHz' % (min_frequency, max_frequency)):
                    spw_selection = ''

                for source_name in [s.name for s in ms.sources]:
                    cont_ranges_spwsel[source_name][str(spwid)] = '%s LSRK' % (spw_selection)

        return cont_ranges_spwsel

    def field_intent_list(self, intent, field):
        intent_list = intent.split(',')
        field_list = utils.safe_split(field)

        field_intent_result = set()

        for vis in self.vislist:
            ms = self.observing_run.get_ms(name=vis)
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
                    for eachfield in field_list:
                        re_field = utils.dequote(eachfield)
                        temp_result.update([fir for fir in field_intent_result if utils.dequote(fir[0]) == re_field])
                    field_intent_result = temp_result

            else:
                if field.strip() is not '':
                    for f in field_list:
                        fintents_list = [fld.intents for fld in fields if utils.dequote(fld.name) == utils.dequote(f)]
                        for fintents in fintents_list:
                            for fintent in fintents:
                                field_intent_result.update((f, fintent))

        # eliminate redundant copies of field/intent keys that map to the
        # same data - to prevent duplicate images being produced

        done_vis_scanids = []

        for field_intent in list(field_intent_result):
            field = field_intent[0]
            intent = field_intent[1]
 
            re_field = utils.dequote(field)

            vis_scanids = {}
            for vis in self.vislist:
                ms = self.observing_run.get_ms(name=vis)

                scanids = [scan.id for scan in ms.scans if
                  intent in scan.intents and
                  re_field in [utils.dequote(f.name) for f in scan.fields]]
                scanids.sort()

                vis_scanids[vis] = scanids

            if vis_scanids in done_vis_scanids:
                LOG.warn(
                  'field: %s intent: %s is a duplicate - removing from imlist' %
                  (field, intent))
                field_intent_result.discard(field_intent)
            else:
                done_vis_scanids.append(vis_scanids)

        return field_intent_result

    def get_scanidlist(self, vis, field, intent):
        # Use scanids to select data with the specified intent
        # Note CASA clean now supports intent selectin but leave
        # this logic in place and use it to eliminate vis that
        # don't contain the requested data.
        scanidlist = []
        visindexlist = []

        for i in xrange(len(vis)):
            allscanids = []
            for fieldname in field.split(','):
                re_field = utils.dequote(fieldname)
                ms = self.observing_run.get_ms(name=vis[i])
                scanids = [scan.id for scan in ms.scans if
                           intent in scan.intents and
                           re_field in [utils.dequote(f.name) for f in scan.fields]]
                if scanids == []:
                    continue
                allscanids.extend(scanids)
            if allscanids != []:
                allscanids = ','.join(map(str, list(set(allscanids))))
                scanidlist.append(allscanids)
                visindexlist.append(i)

        return scanidlist, visindexlist

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
            beam = max(beam, self.beam_radius()[spwid])

        return beam

    def cell(self, field_intent_list, spwspec, oversample=2.5):
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
            for field, intent in field_intent_list:
                # select data to be imaged
                valid_data[(field, intent)] = False
                for vis in self.vislist:
                    ms = self.observing_run.get_ms(name=vis)
                    ms.get_scans()
                    scanids = [str(scan.id) for scan in ms.scans
                               if intent in scan.intents
                               and field in [fld.name for fld in scan.fields]]
                    scanids = ','.join(scanids)
                    try:
                        casatools.imager.selectvis(vis=vis,
                          field=field, spw=spwids, scan=scanids,
                          usescratch=False)
                        # flag to say that imager has some valid data to work
                        # on
                        valid_data[(field, intent)] = True
                    except:
                        pass

                if not valid_data[(field, intent)]:
                    # no point carrying on for this field/intent
                    LOG.debug('No data for SpW %s field %s' % (spwids, field))
                    continue

                casatools.imager.weight(type='natural')

                # use imager.advise to get the maximum cell size
                aipsfieldofview = '%4.1farcsec' % (2.0 * beam)
                rtn = casatools.imager.advise(takeadvice=False,
                  amplitudeloss=0.5, fieldofview=aipsfieldofview)
                casatools.imager.done()
                if not rtn[0]:
                    # advise can fail if all selected data are flagged
                    # - not documented but assuming bool in first field of returned
                    # record indicates success or failure
                    LOG.warning('imager.advise failed for field/intent %s/%s spw %s - no valid data?' 
                      % (field, intent, spwids))
                    valid_data[(field, intent)] = False
                else:
                    cellv = rtn[2]['value']
                    cellu = rtn[2]['unit']
                    cellv /= oversample
                    cellvs.append(cellv)
                    LOG.debug('Cell (oversample %s) for %s/%s spw %s: %s' % (
                      oversample, field, intent, spwspec, cellv))

            if cellvs:
                # cell that's good for all field/intents
                cell = '%.2g%s' % (min(cellvs), cellu)
                LOG.debug('RESULT cell for spw %s: %s' % (spwspec, cell))
            else:
                cell = 'invalid'

        finally:
            casatools.imager.done()

        return [cell], valid_data

    def nchan_and_width(self, field_intent, spwspec):
        if field_intent == 'TARGET':
            # get the spwids in spwspec
            p=re.compile(r"[ ,]+(\d+)")
            spwids = p.findall(' %s' % spwspec)
            spwids = map(int, spwids)
            spwids = list(set(spwids))
            ms = self.observing_run.get_ms(name=self.vislist[0])
            # Use the first spw for the time being. TBD if this needs to be
            # improved.
            spw = ms.get_spectral_window(spwids[0])
            bandwidth = spw.bandwidth
            chan_widths = [c.getWidth() for c in spw.channels]

            # Currently imaging with a channel width of 15 MHz or the native
            # width if larger and at least 8 channels
            image_chan_width = measures.Frequency('15e6', measures.FrequencyUnits.HERTZ)
            min_nchan = 8
            if (any([chan_width >= image_chan_width for chan_width in chan_widths])):
                nchan = -1
                width = ''
            else:
                if (bandwidth >= min_nchan * image_chan_width):
                    nchan = int(round(float(bandwidth.to_units(measures.FrequencyUnits.HERTZ)) /
                                      float(image_chan_width.to_units(measures.FrequencyUnits.HERTZ))))
                    width = str(image_chan_width)
                else:
                    nchan = min_nchan
                    width = str(bandwidth / nchan)
        else:
            nchan = -1
            width = ''

        return nchan, width

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
                    ms = self.observing_run.get_ms(name=vis)
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

    def gridder(self, intent, field):
        # the field heuristic which decides whether this is a mosaic or not
        self.field(intent, field)

        if self._mosaic:
            return 'mosaic'
        else:
            return 'standard'

    def phasecenter(self, fields, centreonly=True):

        cme = casatools.measures
        cqa = casatools.quanta

        mdirections = []
        for ivis, vis in enumerate(self.vislist):

	    # Get the visibilities
            ms = self.observing_run.get_ms(name=vis)
            visfields = fields[ivis]
            if visfields == '':
                continue
            visfields = visfields.split(',')
            visfields = map(int, visfields)

	    # Get the phase directions and convert to ICRS
            for field in visfields:
                # get field centres as measures
                fieldobj = ms.get_fields(field_id=field)[0]
                ref =  cme.getref(fieldobj.mdirection)
                if ref=='ICRS' or ref=='J2000' or ref=='B1950':
                    phase_dir = cme.measure(fieldobj.mdirection, 'ICRS')
                else:
                    phase_dir = fieldobj.mdirection
                mdirections.append(phase_dir)

        # sanity check - for single field images the field centres from
        # different measurement sets should be coincident and can
        # collapse the list 
        # Set the maximum separation to 200 microarcsec and test via
        # a tolerance rather than an equality.
        max_separation = cqa.quantity('200uarcsec')
        if not self._mosaic:
            for mdirection in mdirections:
                separation = cme.separation(mdirection, mdirections[0])
                #if mdirection != mdirections[0]:
                if cqa.gt (separation, max_separation):
                    LOG.warning('Separation between single field centres: %s is greater than %s' % (separation, max_separation))
#                      casatools.measures.separation(mdirection, mdirections[0])))
#                    raise Exception, \
#                      'non-identical field centers in single field image' 
            mdirections = [mdirections[0]]

        # it should be easy to calculate some 'average' direction
        # from the contributing fields but it doesn't seem to be
        # at the moment - no conversion beween direction measures,
        # no calculation of a direction from a direction and an
        # offset. Consequently, what follows is a bit crude.

        # First, find the offset of all field from field 0.
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
        ref = cme.getref(mdirections[0])
        md = cme.getvalue(mdirections[0])
        m0 = cqa.quantity(md['m0'])
        m1 = cqa.quantity(md['m1'])

        m0 = cqa.add(m0, cqa.div('%sarcsec' % xcen, cqa.cos(m1)))
        m1 = cqa.add(m1, '%sarcsec' % ycen)

        # convert to strings (CASA 4.0 returns as list for some reason
        # hence 0 index)
        if ref=='ICRS' or ref=='J2000' or ref=='B1950':
            m0 = cqa.time(m0, prec=10)[0]
        else:
            m0 = cqa.angle(m0, prec=9)[0]
        m1 = cqa.angle(m1, prec=9)[0]

        if centreonly:
            return '%s %s %s' % (ref, m0, m1)
        else:
            return '%s %s %s' % (ref, m0, m1), xspread, yspread

    def field(self, intent, field):
        result = []

        for vis in self.vislist:
            ms = self.observing_run.get_ms(name=vis)
            fields = ms.fields

            field_list = []
            if field is not None:
                # field set explicitly
                if type(field) is not types.ListType:
                    field_names = [field]
                else:
                    field_names = field

                # convert to field ids
                for field_name in field_names:
                    field_list += [fld.id for fld in fields if 
                      field_name.replace(' ','')==fld.name.replace(' ','')]

            if intent is not None:
                # pattern matching to allow intents of form *TARGET* to work
                re_intent = intent.replace('*', '.*')
                field_list = [fld.id for fld in fields if
                  fld.id in field_list and re_intent in fld.intents]

            # this will be a mosaic if there is more than 1 field_id for any 
            # measurement set - probably needs more work, what if want to
            # mosaic together single fields in multiple measurement sets?
            self._mosaic = len(field_list) > 1
 
            field_string = ','.join(str(fld_id) for fld_id in field_list)
            result.append(field_string)

        return result

    def imsize(self, fields, cell, beam, sfpblimit=None, max_pixels=None):
        # get spread of beams
        ignore, xspread, yspread = self.phasecenter(fields, centreonly=False)

        cqa = casatools.quanta

        cellx = cell[0]
        if len(cell) > 1:
            celly = cell[1]
        else:
            celly = cell[0]

        if (cellx == 'invalid') or (celly == 'invalid'):
            return [0, 0]

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

        if (not self._mosaic) and (sfpblimit is not None):
            beam_fwhp = 1.12 / 1.22 * beam_radiusv
            nxpix = int(round(1.1 * beam_fwhp * math.sqrt(-math.log(sfpblimit) / math.log(2.)) / cellvx))
            nypix = int(round(1.1 * beam_fwhp * math.sqrt(-math.log(sfpblimit) / math.log(2.)) / cellvy))

        if max_pixels is not None:
            nxpix = min(nxpix, max_pixels)
            nypix = min(nypix, max_pixels)

        # set nxpix, nypix to next highest 'composite number'
        nxpix = cleanhelper.cleanhelper.getOptimumSize(nxpix)
        nypix = cleanhelper.cleanhelper.getOptimumSize(nypix)

        return [nxpix, nypix]

    def imagename(self, output_dir=None, intent=None, field=None, spwspec=None, specmode=None):
        try:
            nameroot = self.imagename_prefix
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

        namer.stage('STAGENUMBER')
        if intent:
            namer.intent(intent)
        if field:
            namer.source(field)
        if spwspec:
            # find all the spwids present in the list
            p=re.compile(r"[ ,]+(\d+)")
            spwids = p.findall(' %s' % spwspec)
            spwids = list(set(spwids))
            spwids = map(str, sorted(map(int, spwids)))
            spw = '_'.join(spwids)
            namer.spectral_window(spw)
        if specmode:
            namer.specmode(specmode)

        # filenamer returns a sanitized filename (i.e. one with 
        # illegal characters replace by '_'), no need to check
        # the name components individually.
        imagename = namer.get_filename()
        return imagename

    def width(self, spw):
        # get the spw from the first vis set, assume all others the same for
        # now
        ms = self.observing_run.get_ms(name=self.vislist[0])
        spw = ms.get_spectral_window(spw)
        # negative widths appear to confuse clean,
        if spw.channels[1].high > spw.channels[0].high:
            width = spw.channels[1].high - spw.channels[0].high
        else:
            width = spw.channels[0].high - spw.channels[1].high
        # increase width slightly to try to avoid the error:
        # WARN SubMS::convertGridPars *** Requested new channel width is
        # smaller than smallest original channel width.
        # This should no longer be necessary (CASA >=4.7) and this width
        # method does not seem to be used anywhere anyways.
        #width = decimal.Decimal('1.0001') * width
        width = str(width)
        return width

    def ncorr (self, spw):
        # get the spw from the first vis set, assume all others the same for
        # now
        ms = self.observing_run.get_ms(name=self.vislist[0])
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

    def pblimits(self, pb):

        pblimit_image = 0.2
        pblimit_cleanmask = 0.3

        if (pb not in [None, '']):
            try:
                iaTool = casatools.image
                iaTool.open(pb)
                nx, ny, np, nf = iaTool.shape()
                pb_edge = 0.0
                i_pb_edge = -1
                # There are cases with zero edged PBs (due to masking)
                while ((pb_edge == 0.0) and (i_pb_edge < ny/2)):
                    i_pb_edge += 1
                    pb_edge = iaTool.getchunk([nx/2, i_pb_edge, 0, nf/2], [nx/2, i_pb_edge, 0, nf/2]).flatten()[0]
                if (pb_edge > 0.2):
                    i_pb_image = max(i_pb_edge, int(0.05 * ny))
                    pblimit_image = iaTool.getchunk([nx/2, i_pb_image, 0, nf/2], [nx/2, i_pb_image, 0, nf/2]).flatten()[0]
                    i_pb_cleanmask = i_pb_image + int(0.05 * ny)
                    pblimit_cleanmask = iaTool.getchunk([nx/2, i_pb_cleanmask, 0, nf/2], [nx/2, i_pb_cleanmask, 0, nf/2]).flatten()[0]
            except Exception as e:
                LOG.warning('Could not analyze PB: %s. Using default pblimit values.' % (e))
            finally:
                iaTool.close()

        return pblimit_image, pblimit_cleanmask

    def deconvolver(self, specmode, spwspec):
        if (specmode == 'cont'):
            abs_min_frequency = 1.0e15
            abs_max_frequency = 0.0
            ms = self.observing_run.get_ms(name=self.vislist[0])
            for spwid in spwspec.split(','):
                spw = ms.get_spectral_window(spwid)
                min_frequency = float(spw.min_frequency.to_units(measures.FrequencyUnits.HERTZ))
                if (min_frequency < abs_min_frequency):
                    abs_min_frequency = min_frequency
                max_frequency = float(spw.max_frequency.to_units(measures.FrequencyUnits.HERTZ))
                if (max_frequency > abs_max_frequency):
                    abs_max_frequency = max_frequency
            if (2.0 * (abs_max_frequency - abs_min_frequency) / (abs_min_frequency + abs_max_frequency) > 0.1):
                return 'mtmfs'
            else:
                return 'hogbom'
        else:
            return 'hogbom'

    def robust(self, spw):

        '''Default robust value.'''

        return 0.5

    def calc_topo_ranges(self, inputs):

        '''Calculate TOPO ranges for hif_tclean inputs.'''

        spw_topo_freq_param_lists = []
        spw_topo_chan_param_lists = []
        spw_topo_freq_param_dict = collections.defaultdict(dict)
        spw_topo_chan_param_dict = collections.defaultdict(dict)
        p = re.compile('([\d.]*)(~)([\d.]*)(\D*)')
        total_topo_freq_ranges = []
        topo_freq_ranges = []
        num_channels = []

        # get spw info from first vis set, assume spws uniform
        # across datasets
        ms = self.observing_run.get_ms(name=inputs.vis[0])

        # Get ID of field closest to the phase center
        meTool = casatools.measures
        qaTool = casatools.quanta
        ref_field_ids = []

        # Phase center coordinates
        pc_direc = meTool.source(inputs.phasecenter)

        for msname in inputs.vis:
            try:
                ms_obj = self.observing_run.get_ms(msname)
                field_ids = [f.id for f in ms_obj.fields if (f.name == inputs.field) and (inputs.intent in f.intents)]
                separations = [meTool.separation(pc_direc, f.mdirection)['value'] for f in ms_obj.fields if f.id in field_ids]
                ref_field_ids.append(field_ids[separations.index(min(separations))])
            except:
                ref_field_ids.append(-1)

        # Get a cont file handler for the conversion to TOPO
        contfile_handler = contfilehandler.ContFileHandler(self.contfile)

        aggregate_lsrk_bw = '0.0GHz'

        for spwid in inputs.spw.split(','):
            spw_info = ms.get_spectral_window(spwid)

            num_channels.append(spw_info.num_channels)

            min_frequency = float(spw_info.min_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
            max_frequency = float(spw_info.max_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))

            # Save spw width
            total_topo_freq_ranges.append((min_frequency, max_frequency))

            if (inputs.spwsel_lsrk.has_key('spw%s' % (spwid))):
                if (inputs.spwsel_lsrk['spw%s' % (spwid)] not in ['', 'NONE']):
                    freq_selection, refer = inputs.spwsel_lsrk['spw%s' % (spwid)].split()
                    if (refer == 'LSRK'):
                        # Convert to TOPO
                        topo_freq_selections, topo_chan_selections, aggregate_spw_lsrk_bw = contfile_handler.lsrk_to_topo(inputs.spwsel_lsrk['spw%s' % (spwid)], inputs.vis, ref_field_ids, spwid)
                        spw_topo_freq_param_lists.append(['%s:%s' % (spwid, topo_freq_selection.split()[0]) for topo_freq_selection in topo_freq_selections])
                        spw_topo_chan_param_lists.append(['%s:%s' % (spwid, topo_chan_selection.split()[0]) for topo_chan_selection in topo_chan_selections])
                        for i in xrange(len(inputs.vis)):
                            spw_topo_freq_param_dict[os.path.basename(inputs.vis[i])][spwid] = topo_freq_selections[i].split()[0]
                            spw_topo_chan_param_dict[os.path.basename(inputs.vis[i])][spwid] = topo_chan_selections[i].split()[0]
                        # Count only one selection !
                        for topo_freq_range in topo_freq_selections[0].split(';'):
                            f1, sep, f2, unit = p.findall(topo_freq_range)[0]
                            topo_freq_ranges.append((float(f1), float(f2)))
                    else:
                        LOG.warning('Cannot convert frequency selection properly to TOPO. Using plain ranges for all MSs.')
                        spw_topo_freq_param_lists.append(['%s:%s' % (spwid, freq_selection)] * len(inputs.vis))
                        # TODO: Need to derive real channel ranges
                        spw_topo_chan_param_lists.append(['%s:0~%s' % (spwid, spw_info.num_channels - 1)] * len(inputs.vis))
                        for i in xrange(len(inputs.vis)):
                            spw_topo_freq_param_dict[os.path.basename(inputs.vis[i])][spwid] = freq_selection.split()[0]
                            # TODO: Need to derive real channel ranges
                            spw_topo_chan_param_dict[os.path.basename(inputs.vis[i])][spwid] = '0~%d' % (spw_info.num_channels - 1)
                        # Count only one selection !
                        aggregate_spw_lsrk_bw = '0.0GHz'
                        for freq_range in freq_selection.split(';'):
                            f1, sep, f2, unit = p.findall(freq_range)[0]
                            topo_freq_ranges.append((float(f1), float(f2)))
                            delta_f = qaTool.sub('%s%s' % (f2, unit), '%s%s' % (f1, unit))
                            aggregate_spw_lsrk_bw = qaTool.add(aggregate_spw_lsrk_bw, delta_f)
                else:
                    spw_topo_freq_param_lists.append([spwid] * len(inputs.vis))
                    spw_topo_chan_param_lists.append([spwid] * len(inputs.vis))
                    for msname in inputs.vis:
                        spw_topo_freq_param_dict[os.path.basename(msname)][spwid] = ''
                        spw_topo_chan_param_dict[os.path.basename(msname)][spwid] = ''
                    topo_freq_ranges.append((min_frequency, max_frequency))
                    aggregate_spw_lsrk_bw = '%sGHz' % (max_frequency - min_frequency)
                    if (inputs.intent == 'TARGET') and (inputs.specmode in ('mfs', 'cont')):
                        LOG.warning('No continuum frequency selection for Target Field %s SPW %s' % (inputs.field, spwid))
            else:
                spw_topo_freq_param_lists.append([spwid] * len(inputs.vis))
                spw_topo_chan_param_lists.append([spwid] * len(inputs.vis))
                for msname in inputs.vis:
                    spw_topo_freq_param_dict[os.path.basename(msname)][spwid] = ''
                    spw_topo_chan_param_dict[os.path.basename(msname)][spwid] = ''
                topo_freq_ranges.append((min_frequency, max_frequency))
                aggregate_spw_lsrk_bw = '%sGHz' % (max_frequency - min_frequency)
                if (inputs.intent == 'TARGET') and (inputs.specmode in ('mfs', 'cont')):
                    LOG.warning('No continuum frequency selection for Target Field %s SPW %s' % (inputs.field, spwid))

            aggregate_lsrk_bw = qaTool.add(aggregate_lsrk_bw, aggregate_spw_lsrk_bw)

        spw_topo_freq_param = [','.join(spwsel_per_ms) for spwsel_per_ms in [[spw_topo_freq_param_list_per_ms[i] for spw_topo_freq_param_list_per_ms in spw_topo_freq_param_lists] for i in xrange(len(inputs.vis))]]
        spw_topo_chan_param = [','.join(spwsel_per_ms) for spwsel_per_ms in [[spw_topo_chan_param_list_per_ms[i] for spw_topo_chan_param_list_per_ms in spw_topo_chan_param_lists] for i in xrange(len(inputs.vis))]]

        # Calculate total bandwidth
        total_topo_bw = '0.0GHz'
        for total_topo_freq_range in utils.merge_ranges(total_topo_freq_ranges):
            total_topo_bw = qaTool.add(total_topo_bw, qaTool.sub('%sGHz' % (total_topo_freq_range[1]), '%sGHz' % (total_topo_freq_range[0])))

        # Calculate aggregate selected bandwidth
        aggregate_topo_bw = '0.0GHz'
        for topo_freq_range in utils.merge_ranges(topo_freq_ranges):
            aggregate_topo_bw = qaTool.add(aggregate_topo_bw, qaTool.sub('%sGHz' % (topo_freq_range[1]), '%sGHz' % (topo_freq_range[0])))

        return spw_topo_freq_param, spw_topo_chan_param, spw_topo_freq_param_dict, spw_topo_chan_param_dict, total_topo_bw, aggregate_topo_bw, aggregate_lsrk_bw

    def lsrk_freq_intersection(self, vis, field, spw):
        """
        Calculate LSRK frequency intersection of a list of MSs for a
        given field and spw. Exclude flagged channels.
        """
        lsrk_freq_ranges = []
        lsrk_channel_widths = []

        for msname in vis:
            # CAS-8997
            #
            # ms.selectinit(datadescid=x) is required to avoid the 'Data shape
            # varies...' warning message. There's no guarantee that a single
            # spectral window will resolve to a single data description, e.g.
            # the polarisation setup may change. There's not enough
            # information passed into this function to consistently identify the
            # data description that should be specified so on occasion the
            # warning message will reoccur.
            #
            # TODO refactor method signature to include data description ID
            #
            msDO = self.observing_run.get_ms(msname)
            spwDO = msDO.get_spectral_window(spw)
            data_description_ids = {dd.id for dd in msDO.data_descriptions if dd.spw is spwDO}
            if len(data_description_ids) == 1:
                dd_id = data_description_ids.pop()
            else:
                msg = 'Could not resolve {!s} spw {!s} to a single data description'.format(msname, spwDO.id)
                LOG.warning(msg)
                dd_id = 0

            try:
                with casatools.MSReader(msname) as msTool:
                    # CAS-8997 selectinit is required to avoid the 'Data shape varies...' warning message
                    msTool.selectinit(datadescid=dd_id)
                    # Antenna selection does not work (CAS-8757)
                    #staql={'field': field, 'spw': spw, 'antenna': '*&*'}
                    staql={'field': field, 'spw': spw}
                    msTool.msselect(staql)
                    msTool.iterinit(maxrows = 500)
                    msTool.iterorigin()

                    result = np.array([True] * spwDO.num_channels)

                    iterating = True
                    while iterating:
                        flag_ants = msTool.getdata(['flag','antenna1','antenna2'])

                        # Calculate averaged flagging vector keeping all unflagged
                        # channels from any baseline.
                        if flag_ants != {}:
                            for i in xrange(flag_ants['flag'].shape[2]):
                                # Antenna selection does not work (CAS-8757)
                                if (flag_ants['antenna1'][i] != flag_ants['antenna2'][i]):
                                    for j in xrange(flag_ants['flag'].shape[0]):
                                         result = np.logical_and(result, flag_ants['flag'][j,:,i])

                        iterating = msTool.iternext()

                nfi = np.where(result == False)[0]

            except Exception as e:
                LOG.error('Exception while determining edge flags: %s' % e)
                nfi = np.array([])

            if nfi.shape != (0,):
                with casatools.ImagerReader(msname) as imager:
                    # Just the edges. Skip one extra channel in final frequency range.
                    imager.selectvis(field=field, spw='%s:%d~%d' % (spw, nfi[0], nfi[-1]))
                    result = imager.advisechansel(getfreqrange=True, freqframe='LSRK')

                f0 = result['freqstart']
                f1 = result['freqend']

                lsrk_freq_ranges.append((f0, f1))
                # The frequency range from advisechansel is from channel edge
                # to channel edge. To get the width, one needs to divide by the
                # number of channels in the selection.
                lsrk_channel_widths.append((f1-f0)/(nfi[-1]-nfi[0]+1))

        intersect_range = utils.intersect_ranges(lsrk_freq_ranges)
        if intersect_range != ():
            if0, if1 = intersect_range
            return if0, if1, max(lsrk_channel_widths)
        else:
            return -1, -1, 0

    def calc_sensitivities(self, vis, field, intent, spw, nbin, spw_topo_chan_param_dict, specmode, gridder, cell, imsize, weighting, robust):
        """Compute sensitivity estimate using CASA."""

        # Calculate sensitivities
        sensitivities = []

        detailed_field_sensitivities = {}
        min_sensitivities = []
        max_sensitivities = []
        min_field_ids = []
        max_field_ids = []
        eff_ch_bw = 0.0
        for msname in vis:
            ms = self.observing_run.get_ms(name=msname)
            detailed_field_sensitivities[os.path.basename(msname)] = {}
            for intSpw in [int(s) for s in spw.split(',')]:
                spw_do = ms.get_spectral_window(intSpw)
                detailed_field_sensitivities[os.path.basename(msname)][intSpw] = {}
                try:
                    if (specmode == 'cube'):
                        # Use the center channel selection
                        if nbin != -1:
                            chansel = '%d~%d' % (int(spw_do.num_channels / 2.0), int(spw_do.num_channels / 2.0) + nbin - 1)
                        else:
                            chansel = '%d~%d' % (int(spw_do.num_channels / 2.0), int(spw_do.num_channels / 2.0))
                    else:
                        if (spw_topo_chan_param_dict[os.path.basename(msname)][str(intSpw)] != ''):
                            # Use continuum frequency selection
                            chansel = spw_topo_chan_param_dict[os.path.basename(msname)][str(intSpw)]
                        else:
                            # Use full spw
                            chansel = '0~%d' % (spw_do.num_channels - 1)

                    if (gridder == 'mosaic'):
                        field_sensitivities = []
                        for field_id in [f.id for f in ms.fields if (utils.dequote(f.name) == utils.dequote(field) and intent in f.intents)]:
                            try:
                                field_sensitivity, eff_ch_bw = self.get_sensitivity(ms, field_id, intSpw, chansel, specmode, cell, imsize, weighting, robust)
                                if (field_sensitivity > 0.0):
                                    field_sensitivities.append(field_sensitivity)
                                    detailed_field_sensitivities[os.path.basename(msname)][intSpw][field_id] = field_sensitivity
                            except Exception as e:
                                LOG.warning('Could not calculate sensitivity for MS %s Field %s (ID %d) SPW %d ChanSel %s' % (os.path.basename(msname), utils.dequote(field), field_id, intSpw, chansel))

                        median_sensitivity = np.median(field_sensitivities)
                        min_field_id, min_sensitivity = min(detailed_field_sensitivities[os.path.basename(msname)][intSpw].iteritems(), key=operator.itemgetter(1))
                        max_field_id, max_sensitivity = max(detailed_field_sensitivities[os.path.basename(msname)][intSpw].iteritems(), key=operator.itemgetter(1))

                        # Correct for mosaic overlap factor
                        source_name = [f.source.name for f in ms.fields if (utils.dequote(f.name) == utils.dequote(field) and intent in f.intents)][0]
                        diameter = np.median([a.diameter for a in ms.antennas])
                        overlap_factor = mosaicoverlap.mosaicOverlapFactorMS(ms, source_name, intSpw, diameter)
                        LOG.info('Dividing by mosaic overlap improvement factor of %s.' % (overlap_factor))
                        median_sensitivity /= overlap_factor
                        min_sensitivity /= overlap_factor
                        max_sensitivity /= overlap_factor

                        # Final values
                        sensitivities.append(median_sensitivity)
                        min_sensitivities.append(min_sensitivity)
                        max_sensitivities.append(max_sensitivity)
                        min_field_ids.append(min_field_id)
                        max_field_ids.append(max_field_id)
                        LOG.info('Using median of all mosaic field sensitivities for MS %s, Field %s, SPW %s: %s Jy' % (os.path.basename(msname), field, str(intSpw), median_sensitivity))
                        LOG.info('Minimum mosaic field sensitivity for MS %s, Field %s (ID: %s), SPW %s: %s Jy' % (os.path.basename(msname), field, min_field_id, str(intSpw), min_sensitivity))
                        LOG.info('Maximum mosaic field sensitivity for MS %s, Field %s (ID: %s), SPW %s: %s Jy' % (os.path.basename(msname), field, max_field_id, str(intSpw), max_sensitivity))
                    else:
                        # Still need to loop over field ID with proper intent for single field case
                        field_sensitivities = []
                        for field_id in [f.id for f in ms.fields if (utils.dequote(f.name) == utils.dequote(field) and intent in f.intents)]:
                            field_sensitivity, eff_ch_bw = self.get_sensitivity(ms, field_id, intSpw, chansel, specmode, cell, imsize, weighting, robust)
                            if (field_sensitivity > 0.0):
                                field_sensitivities.append(field_sensitivity)
                        # Check if we have anything
                        if (len(field_sensitivities) > 0):
                            # If there is more than one result (shouldn't be), combine them to one number
                            field_sensitivity = 1.0 / np.sqrt(np.sum(1.0 / np.array(field_sensitivities)**2))
                            sensitivities.append(field_sensitivity)
                            detailed_field_sensitivities[os.path.basename(msname)][intSpw][field] = field_sensitivity
                except Exception as e:
                    # Simply pass as this could be a case of a source not
                    # being present in the MS.
                    pass

        if (len(sensitivities) > 0):
            sensitivity = 1.0 / np.sqrt(np.sum(1.0 / np.array(sensitivities)**2))
            if (gridder == 'mosaic'):
                min_sensitivity = 1.0 / np.sqrt(np.sum(1.0 / np.array(min_sensitivities)**2))
                max_sensitivity = 1.0 / np.sqrt(np.sum(1.0 / np.array(max_sensitivities)**2))
                min_field_id = int(np.median(min_field_ids))
                max_field_id = int(np.median(max_field_ids))
            else:
                min_sensitivity = None
                max_sensitivity = None
                min_field_id = None
                max_field_id = None
        else:
            defaultSensitivity = 0.1
            if (specmode == 'cube'):
                LOG.warning('Exception in calculating sensitivity. Cube center channel seems to be flagged. Assuming default value of %g Jy/beam.' % (defaultSensitivity))
            else:
                LOG.warning('Exception in calculating sensitivity. Assuming default value of %g Jy/beam.' % (defaultSensitivity))
            sensitivity = defaultSensitivity
            min_sensitivity = None
            max_sensitivity = None
            min_field_id = None
            max_field_id = None

        return sensitivity, min_sensitivity, max_sensitivity, min_field_id, max_field_id, eff_ch_bw

    def get_sensitivity(self, ms_do, field, spw, chansel, specmode, cell, imsize, weighting, robust):
        """
        Get sensitivity for a field / spw / chansel combination from CASA's
        apparentsens method and a correction for effective channel widths
        in case of online smoothing.

        This heuristic is currently optimized for ALMA data only.
        """

        spw_do = ms_do.get_spectral_window(spw)
        spwchan = spw_do.num_channels
        physicalBW_of_1chan = float(spw_do.channels[0].getWidth().convert_to(measures.FrequencyUnits.HERTZ).value)
        effectiveBW_of_1chan = float(spw_do.channels[0].effective_bw.convert_to(measures.FrequencyUnits.HERTZ).value)

        BW_ratio = effectiveBW_of_1chan / physicalBW_of_1chan

        if (BW_ratio <= 1.0):
            N_smooth = 0
        elif (utils.approx_equal(BW_ratio, 2.667, 4)):
            N_smooth = 1
        elif (utils.approx_equal(BW_ratio, 1.600, 4)):
            N_smooth = 2
        elif (utils.approx_equal(BW_ratio, 1.231, 4)):
            N_smooth = 4
        elif (utils.approx_equal(BW_ratio, 1.104, 4)):
            N_smooth = 8
        elif (utils.approx_equal(BW_ratio, 1.049, 4)):
            N_smooth = 16
        else:
            LOG.warning('Could not evaluate channel bandwidths ratio. Physical: %s Effective: %s Ratio: %s' % (physicalBW_of_1chan, effectiveBW_of_1chan, BW_ratio))
            N_smooth = 0

        chansel_sensitivities = []
        for chanrange in chansel.split(';'):

            try:
                with casatools.ImagerReader(ms_do.name) as imTool:
                    imTool.selectvis(spw='%s:%s' % (spw, chanrange), field=field)
                    # TODO: Add scan selection ?
                    imTool.defineimage(mode=specmode if specmode=='cube' else 'mfs', spw=spw,
                                       cellx=cell[0], celly=cell[0],
                                       nx=imsize[0], ny=imsize[1])
                    imTool.weight(type=weighting, robust=robust)
                    result = imTool.apparentsens()

                if (result[1] == 0.0):
                    raise Exception('Empty selection')

                apparentsens_value = result[1]
                LOG.info('apparentsens result for MS %s Field %s SPW %s ChanRange %s: %s Jy/beam' % (os.path.basename(ms_do.name), field, spw, chanrange, apparentsens_value))
                if (N_smooth > 0):
                    cstart, cstop = map(int, chanrange.split('~'))
                    nchan = cstop - cstart + 1
                    if (nchan > 1):
                        optimisticBW = nchan * float(effectiveBW_of_1chan)
                        approximateEffectiveBW = (nchan + 1.12 * (spwchan - nchan) / spwchan / N_smooth) * float(physicalBW_of_1chan)
                        SCF = (optimisticBW / approximateEffectiveBW)**0.5
                        corrected_apparentsens_value = apparentsens_value * SCF
                        LOG.info('Effective BW heuristic: Correcting apparentsens result by %s from %s Jy/beam to %s Jy/beam' % (SCF, apparentsens_value, corrected_apparentsens_value))
                    else:
                        corrected_apparentsens_value = apparentsens_value
                else:
                    corrected_apparentsens_value = apparentsens_value 

                chansel_sensitivities.append(corrected_apparentsens_value)

            except Exception as e:
                if (str(e) != 'Empty selection'):
                    LOG.info('Could not calculate sensitivity for MS %s Field %s SPW %s ChanRange %s: %s' % (os.path.basename(ms_do.name), field, spw, chanrange, e))

        if (len(chansel_sensitivities) > 0):
            return 1.0 / np.sqrt(np.sum(1.0 / np.array(chansel_sensitivities)**2)), effectiveBW_of_1chan
        else:
            return 0.0, effectiveBW_of_1chan

    def dr_correction(self, threshold, dirty_dynamic_range, residual_max, intent, tlimit):

        '''Adjustment of cleaning threshold due to dynamic range limitations.'''

        DR_correction_factor = 1.0
        maxEDR_used = False

        return threshold, DR_correction_factor, maxEDR_used

    def niter_correction(self, niter, cell, imsize, residual_max, threshold):
        # TODO: parameter should be clean mask to be able to count the pixels

        '''Adjustment of number of cleaning iterations due to mask size.'''

        return niter

    def get_autobox_params(self):

        '''Default auto-boxing parameters.'''

        sidelobethreshold = None
        noisethreshold = None
        lownoisethreshold = None
        minbeamfrac = None

        return sidelobethreshold, noisethreshold, lownoisethreshold, minbeamfrac

    def cyclefactor(self):
        return None


    def cycleniter(self):
        return None


    def scales(self):
        return None


    def uvtaper(self):
        return None


    def uvrange(self):
        return None


    def reffreq(self):
        return None
