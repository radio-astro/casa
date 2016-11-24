import os
import decimal
import math
import numpy as np
import re
import types
import collections

import cleanhelper

import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.contfilehandler as contfilehandler

LOG = infrastructure.get_logger(__name__)


class TcleanHeuristics(object):

    def __init__(self, context, vislist, spw):
        self.context = context
        if type(vislist) is types.ListType:
            self.vislist = vislist
        else:
            self.vislist = [vislist]

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

    def deconvolver(self, specmode, spwspec):
        if (specmode == 'cont'):
            abs_min_frequency = 1.0e15
            abs_max_frequency = 0.0
            ms = self.context.observing_run.get_ms(name=self.vislist[0])
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
        # Check if there is a non-zero desired angular resolution
        cqa = casatools.quanta
        desired_angular_resolution = cqa.convert(self.context.project_performance_parameters.desired_angular_resolution, '')['value']
        if (desired_angular_resolution == 0.0):
            LOG.info('No value for desired angular resolution. Setting "robust" parameter to 0.5.')
            return 0.5

        # Get maximum baseline length in metres
        bmax = 0.0
        for ms in self.context.observing_run.get_measurement_sets():
            if (ms.antenna_array.max_baseline.length.to_units(measures.DistanceUnits.METRE) > bmax):
                bmax = float(ms.antenna_array.max_baseline.length.to_units(measures.DistanceUnits.METRE))

        if (bmax == 0.0):
            LOG.warning('Bmax is zero. Setting "robust" parameter to 0.5.')
            return 0.5

        # Get spw center wavelength

        # get the spw from the first vis set, assume all others the same for now
        ms = self.context.observing_run.get_ms(name=self.vislist[0])
        spw = ms.get_spectral_window(spw)

        centre_frequency = float(spw.centre_frequency.to_units(measures.FrequencyUnits.HERTZ))
        centre_lambda = cqa.constants('c')['value'] / centre_frequency

        # Smallest spatial scale
        smallest_spatial_scale = 1.2 * centre_lambda / bmax

        if (desired_angular_resolution > 1.2 * smallest_spatial_scale):
            robust = 1.0
        elif (desired_angular_resolution < 0.8 * smallest_spatial_scale):
            robust = 0.0
        else:
            robust = 0.5

        return robust

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
        ms = self.context.observing_run.get_ms(name=inputs.vis[0])

        # Get ID of field closest to the phase center
        meTool = casatools.measures
        qaTool = casatools.quanta
        ref_field_ids = []

        # Phase center coordinates
        pc_direc = meTool.source(inputs.phasecenter)

        for msname in inputs.vis:
            try:
                ms_obj = self.context.observing_run.get_ms(msname)
                field_ids = [f.id for f in ms_obj.fields if (f.name == inputs.field) and (inputs.intent in f.intents)]
                separations = [meTool.separation(pc_direc, f.mdirection)['value'] for f in ms_obj.fields if f.id in field_ids]
                ref_field_ids.append(field_ids[separations.index(min(separations))])
            except:
                ref_field_ids.append(-1)

        # Get a cont file handler for the conversion to TOPO
        contfile_handler = contfilehandler.ContFileHandler(self.context.contfile)

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
            msDO = self.context.observing_run.get_ms(msname)
            n_ants = len(msDO.antennas)

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
            spwDO = msDO.get_spectral_window(spw)
            data_description_ids = {dd.id for dd in msDO.data_descriptions if dd.spw is spwDO}
            if len(data_description_ids) == 1:
                dd_id = data_description_ids.pop()
            else:
                msg = 'Could not resolve {!s} spw {!s} to a single data description'.format(msname, spwDO.id)
                LOG.warning(msg)
                dd_id = 0

            with casatools.MSReader(msname) as msTool:
                # CAS-8997 selectinit is required to avoid the 'Data shape varies...' warning message
                msTool.selectinit(datadescid=dd_id)
                msTool.iterinit(maxrows = int(n_ants * ((n_ants - 1) / 2.0 + 1)))
                msTool.iterorigin()
                # Antenna selection does not work (CAS-8757)
                #staql={'field': field, 'spw': spw, 'antenna': '*&*'}
                staql={'field': field, 'spw': spw}
                msTool.msselect(staql)
                flag_ants = msTool.getdata(['flag', 'antenna1', 'antenna2'])

            # Calculate averaged flagging vector keeping all unflagged channel
            # from any baseline.
            result = np.array([True] * flag_ants['flag'].shape[1])
            for i in xrange(flag_ants['flag'].shape[2]):
                # Antenna selection does not work (CAS-8757)
                if flag_ants['antenna1'][i] != flag_ants['antenna2'][i]:
                    for j in xrange(flag_ants['flag'].shape[0]):
                         result = np.logical_and(result, flag_ants['flag'][j,:,i])

            nfi = np.where(result == False)[0]

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
