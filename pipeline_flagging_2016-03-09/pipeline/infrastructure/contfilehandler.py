#!/usr/bin/env python

'''
Class to handle continuum frequency range files.

The text files contain ranges per source and spw using CASA syntax.
The keyword "NONE" can be written in case of non-detection of a continuum
frequency range.
'''

import re
import numpy as np

from . import casatools
from . import utils
from . import logging

LOG = logging.get_logger(__name__)


class ContFileHandler(object):

    def __init__(self, filename):
        self.filename = filename
        self.cont_ranges = self.read()

    def read(self, skip_none=False):

        cont_ranges = {'fields': {}, 'version': 2}

        p = re.compile('([\d.]*)(~)([\d.]*)(\D*)')

        try:
            cont_region_data = [item.replace('\n', '') for item in open(self.filename, 'r').readlines() if item != '\n']
        except:
            cont_region_data = []
            LOG.warning('Could not read file %s. Using empty selection.' % (self.filename))

        for item in cont_region_data:
            try:
                if ((item.find('SpectralWindow:') == -1) and \
                    (item.find('SPW') == -1) and \
                    (item.find('~') == -1) and \
                    (item != 'NONE')):
                    if (item.find('Field:') == 0):
                        field_name = item.split('Field:')[1].strip()
                    else:
                        field_name = item
                    cont_ranges['fields'][field_name] = {}
                elif (item.find('SPW') == 0):
                    cont_ranges['version'] = 1
                    spw_id = item.split('SPW')[1].strip()
                    cont_ranges['fields'][field_name][spw_id] = []
                elif (item.find('SpectralWindow:') == 0):
                    cont_ranges['version'] = 2
                    spw_id = item.split('SpectralWindow:')[1].strip()
                    cont_ranges['fields'][field_name][spw_id] = []
                elif ((item == 'NONE') and not skip_none):
                    cont_ranges['fields'][field_name][spw_id].append('NONE')
                else:
                    cont_regions = p.findall(item.replace(';',''))
                    for cont_region in cont_regions:
                        if (cont_ranges['version'] == 1):
                            unit = cont_region[3]
                            refer = 'TOPO'
                        elif (cont_ranges['version'] == 2):
                            unit, refer = cont_region[3].split()
                        fLow = casatools.quanta.convert('%s%s' % (cont_region[0], unit), 'GHz')['value']
                        fHigh = casatools.quanta.convert('%s%s' % (cont_region[2], unit), 'GHz')['value']
                        cont_ranges['fields'][field_name][spw_id].append({'range': (fLow,fHigh), 'refer': refer})
            except:
                pass

        return cont_ranges

    def write(self, cont_ranges = None):

        if (cont_ranges is None):
            cont_ranges = self.cont_ranges

        fd = open(self.filename, 'w+')
        for field_name in cont_ranges['fields'].iterkeys():
            if (cont_ranges['version'] == 1):
                fd.write('%s\n\n' % (field_name.replace('"','')))
            elif (cont_ranges['version'] == 2):
                fd.write('Field: %s\n\n' % (field_name.replace('"','')))
            for spw_id in cont_ranges['fields'][field_name].iterkeys():
                if (cont_ranges['version'] == 1):
                    fd.write('SPW%s\n' % (spw_id))
                elif (cont_ranges['version'] == 2):
                    fd.write('SpectralWindow: %s\n' % (spw_id))
                if (cont_ranges['fields'][field_name][spw_id] == []):
                    fd.write('NONE\n')
                else:
                    for freq_range in cont_ranges['fields'][field_name][spw_id]:
                        if (freq_range == 'NONE'):
                            fd.write('NONE\n')
                        else:
                            if (cont_ranges['version'] == 1):
                                fd.write('%s~%sGHz\n' % (freq_range['range'][0], freq_range['range'][1]))
                            elif (cont_ranges['version'] == 2):
                                fd.write('%s~%sGHz %s\n' % (freq_range['range'][0], freq_range['range'][1], freq_range['refer']))
                fd.write('\n')
        fd.close()

    def get_merged_selection(self, field_name, spw_id, cont_ranges = None):

        if ( cont_ranges is None):
            cont_ranges = self.cont_ranges

        if (cont_ranges['fields'].has_key(field_name)):
            if (cont_ranges['fields'][field_name].has_key(spw_id)):
                if (cont_ranges['fields'][field_name][spw_id] != ['NONE']):
                    merged_cont_ranges = utils.merge_ranges([cont_range['range'] for cont_range in cont_ranges['fields'][field_name][spw_id]])
                    cont_ranges_spwsel = ';'.join(['%s~%sGHz' % (spw_sel_interval[0], spw_sel_interval[1]) for spw_sel_interval in merged_cont_ranges])
                    refers = np.array([spw_sel_interval['refer'] for spw_sel_interval in cont_ranges['fields'][field_name][spw_id]])
                    if ((refers == 'TOPO').all()):
                        refer = 'TOPO'
                    elif ((refers == 'LSRK').all()):
                        refer = 'LSRK'
                    else:
                        refer = 'UNDEFINED'
                    cont_ranges_spwsel = '%s %s' % (cont_ranges_spwsel, refer)
                else:
                    cont_ranges_spwsel = 'NONE'
            else:
                cont_ranges_spwsel = ''
        else:
            cont_ranges_spwsel = ''

        return cont_ranges_spwsel
