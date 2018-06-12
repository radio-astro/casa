#!/usr/bin/env python
"""
Class to handle continuum frequency range files.

The text files contain ranges per source and spw using CASA syntax.
The keyword "NONE" can be written in case of non-detection of a continuum
frequency range.
"""

import re

import numpy as np

from . import casatools
from . import utils
from . import logging

LOG = logging.get_logger(__name__)


class ContFileHandler(object):

    def __init__(self, filename, warn_nonexist=False):
        self.filename = filename
        self.p = re.compile('([\d.]*)(~)([\d.]*)(\D*)')
        self.cont_ranges = self.read(warn_nonexist=warn_nonexist)

    def read(self, skip_none=False, warn_nonexist=False):

        if self.filename is None:
            return {}

        cont_ranges = {'fields': {}, 'version': 2}

        try:
            cont_region_data = [item.replace('\n', '') for item in open(self.filename, 'r').readlines() if item != '\n']
        except:
            cont_region_data = []
            if warn_nonexist:
                LOG.warning('Could not read file %s. Using empty selection.' % self.filename)

        for item in cont_region_data:
            try:
                if ((item.find('SpectralWindow:') == -1) and
                        (item.find('SPW') == -1) and
                        (item.find('~') == -1) and
                        (item != 'NONE') and
                        (item != 'ALL')):
                    if item.find('Field:') == 0:
                        field_name = item.split('Field:')[1].strip()
                    else:
                        field_name = item
                    field_name = utils.dequote(field_name).strip()
                    if field_name != '':
                        cont_ranges['fields'][field_name] = {}
                elif item.find('SPW') == 0:
                    cont_ranges['version'] = 1
                    spw_id = item.split('SPW')[1].strip()
                    cont_ranges['fields'][field_name][spw_id] = []
                elif item.find('SpectralWindow:') == 0:
                    cont_ranges['version'] = 2
                    spw_id = item.split('SpectralWindow:')[1].strip()
                    cont_ranges['fields'][field_name][spw_id] = []
                elif item == 'ALL':
                    cont_ranges['fields'][field_name][spw_id].append('ALL')
                elif (item == 'NONE') and not skip_none:
                    cont_ranges['fields'][field_name][spw_id].append('NONE')
                else:
                    cont_regions = self.p.findall(item.replace(';', ''))
                    for cont_region in cont_regions:
                        if cont_ranges['version'] == 1:
                            unit = cont_region[3]
                            refer = 'TOPO'
                        elif cont_ranges['version'] == 2:
                            unit, refer = cont_region[3].split()
                        fLow = casatools.quanta.convert('%s%s' % (cont_region[0], unit), 'GHz')['value']
                        fHigh = casatools.quanta.convert('%s%s' % (cont_region[2], unit), 'GHz')['value']
                        cont_ranges['fields'][field_name][spw_id].append({'range': (fLow, fHigh), 'refer': refer})
            except:
                pass

        for fkey in cont_ranges['fields'].iterkeys():
            for skey in cont_ranges['fields'][fkey].iterkeys():
                if cont_ranges['fields'][fkey][skey] == []:
                    cont_ranges['fields'][fkey][skey] = ['NONE']

        return cont_ranges

    def write(self, cont_ranges=None):

        if self.filename is None:
            return

        if cont_ranges is None:
            cont_ranges = self.cont_ranges

        fd = open(self.filename, 'w+')
        if cont_ranges != {}:
            for field_name in cont_ranges['fields'].iterkeys():
                if cont_ranges['version'] == 1:
                    fd.write('%s\n\n' % (field_name.replace('"', '')))
                elif cont_ranges['version'] == 2:
                    fd.write('Field: %s\n\n' % (field_name.replace('"', '')))
                for spw_id in cont_ranges['fields'][field_name].iterkeys():
                    if cont_ranges['version'] == 1:
                        fd.write('SPW%s\n' % spw_id)
                    elif cont_ranges['version'] == 2:
                        fd.write('SpectralWindow: %s\n' % spw_id)
                    if cont_ranges['fields'][field_name][spw_id] in ([], ['NONE']):
                        fd.write('NONE\n')
                    elif cont_ranges['fields'][field_name][spw_id] == ['ALL']:
                        fd.write('ALL\n')
                    else:
                        for freq_range in cont_ranges['fields'][field_name][spw_id]:
                            if freq_range['range'] == 'NONE':
                                fd.write('NONE\n')
                            elif freq_range['range'] == 'ALL':
                                fd.write('ALL\n')
                            else:
                                if cont_ranges['version'] == 1:
                                    fd.write('%s~%sGHz\n' % (freq_range['range'][0], freq_range['range'][1]))
                                elif cont_ranges['version'] == 2:
                                    fd.write('%s~%sGHz %s\n' % (freq_range['range'][0], freq_range['range'][1],
                                                                freq_range['refer']))
                    fd.write('\n')
        fd.close()

    def get_merged_selection(self, field_name, spw_id, cont_ranges=None):

        field_name = str(field_name)
        spw_id = str(spw_id)

        if cont_ranges is None:
            cont_ranges = self.cont_ranges

        if field_name in cont_ranges['fields']:
            if spw_id in cont_ranges['fields'][field_name]:
                if cont_ranges['fields'][field_name][spw_id] not in (['ALL'], [], ['NONE']):
                    merged_cont_ranges = utils.merge_ranges(
                        [cont_range['range'] for cont_range in cont_ranges['fields'][field_name][spw_id]])
                    cont_ranges_spwsel = ';'.join(['%s~%sGHz' % (spw_sel_interval[0], spw_sel_interval[1])
                                                   for spw_sel_interval in merged_cont_ranges])
                    refers = np.array([spw_sel_interval['refer']
                                       for spw_sel_interval in cont_ranges['fields'][field_name][spw_id]])
                    if (refers == 'TOPO').all():
                        refer = 'TOPO'
                    elif (refers == 'LSRK').all():
                        refer = 'LSRK'
                    else:
                        refer = 'UNDEFINED'
                    cont_ranges_spwsel = '%s %s' % (cont_ranges_spwsel, refer)
                elif cont_ranges['fields'][field_name][spw_id] == ['ALL']:
                    cont_ranges_spwsel = 'ALL'
                else:
                    cont_ranges_spwsel = 'NONE'
            else:
                cont_ranges_spwsel = ''
        else:
            cont_ranges_spwsel = ''

        return cont_ranges_spwsel

    def lsrk_to_topo(self, selection, msnames, fields, spw_id, observing_run, ctrim=0, ctrim_nchan=-1):

        lsrk_freq_selection, refer = selection.split()
        if refer != 'LSRK':
            LOG.error('Original reference frame must be LSRK.')
            raise Exception, 'Original reference frame must be LSRK.'

        if len(msnames) != len(fields):
            LOG.error('MS names and fields lists must match in length.')
            raise Exception, 'MS names and fields lists must match in length.'

        spw_id = int(spw_id)

        imTool = casatools.imager
        qaTool = casatools.quanta

        freq_ranges = []
        aggregate_lsrk_bw = '0.0GHz'
        cont_regions = self.p.findall(lsrk_freq_selection.replace(';', ''))
        for cont_region in cont_regions:
            fLow = qaTool.convert('%s%s' % (cont_region[0], cont_region[3]), 'Hz')['value']
            fHigh = qaTool.convert('%s%s' % (cont_region[2], cont_region[3]), 'Hz')['value']
            freq_ranges.append((fLow, fHigh))
            delta_f = qaTool.sub('%sHz' % fHigh, '%sHz' % fLow)
            aggregate_lsrk_bw = qaTool.add(aggregate_lsrk_bw, delta_f)

        topo_chan_selections = []
        topo_freq_selections = []
        for i in xrange(len(msnames)):
            msname = msnames[i]
            real_spw_id = observing_run.virtual2real_spw_id(spw_id, observing_run.get_ms(msname))
            field = int(fields[i])
            topo_chan_selection = []
            topo_freq_selection = []
            try:
                if field != -1:
                    for freq_range in freq_ranges:
                        imTool.selectvis(vis=msname, field=field, spw=real_spw_id, writeaccess=False)
                        result = imTool.advisechansel(freqstart=freq_range[0], freqend=freq_range[1], freqstep=100.,
                                                      freqframe='LSRK')
                        imTool.done()
                        spw_index = result['ms_0']['spw'].tolist().index(real_spw_id)
                        start = result['ms_0']['start'][spw_index]
                        stop = start + result['ms_0']['nchan'][spw_index] - 1
                        # Optionally skip edge channels
                        if ctrim_nchan != -1:
                            start = start if (start >= ctrim) else ctrim
                            stop = stop if (stop < (ctrim_nchan-ctrim)) else ctrim_nchan-ctrim-1
                        if stop >= start:
                            topo_chan_selection.append((start, stop))
                            result = imTool.advisechansel(msname=msname, fieldid=field,
                                                          spwselection='%d:%d~%d' % (real_spw_id, start, stop),
                                                          freqframe='TOPO', getfreqrange=True)
                            fLow = qaTool.convert('%sHz' % (result['freqstart']), 'GHz')['value']
                            fHigh = qaTool.convert('%sHz' % (result['freqend']), 'GHz')['value']
                            topo_freq_selection.append((fLow, fHigh))
            except Exception as e:
                LOG.info('Cannot calculate TOPO range for MS %s Field %s SPW %s' % (msname, field, real_spw_id))
                print('ERROR_DM: {}'.format(e))

            topo_chan_selections.append(';'.join('%d~%d' % (item[0], item[1]) for item in topo_chan_selection))
            topo_freq_selections.append('%s TOPO' % (';'.join('%s~%sGHz' %
                                                              (item[0], item[1]) for item in topo_freq_selection)))

        return topo_freq_selections, topo_chan_selections, aggregate_lsrk_bw
