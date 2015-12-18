#!/usr/bin/env python

'''
Class to handle continuum frequency range files.

The text files contain ranges per source and spw using CASA syntax.
The keyword "NONE" can be written in case of non-detection of a continuum
frequency range.
'''

from . import casatools
import re

class ContFileHandler(object):

    def __init__(self, filename):
        self.filename = filename

    def read(self, skip_none=False):
        cont_ranges = {}

        p = re.compile('([\d.]*)(~)([\d.]*)(\D*)')

        try:
            cont_region_data = [item.replace('\n', '') for item in open(self.filename, 'r').readlines() if item != '\n']
        except:
            cont_region_data = []

        for item in cont_region_data:
            try:
                if ((item.upper().find('SPW') == -1) and (item.find('~') == -1) and (item != 'NONE')):
                    source_name = item
                    #if ((source_name.find(' ') != -1) or (source_name.find(';') != -1)):
                    #    source_name = '"%s"' % (source_name)
                    cont_ranges[source_name] = {}
                elif (item.upper().find('SPW') == 0):
                    spw_id = item[3:]
                    cont_ranges[source_name][spw_id] = []
                elif ((item == 'NONE') and not skip_none):
                    cont_ranges[source_name][spw_id].append('NONE')
                else:
                    cont_regions = p.findall(item.replace(';','').replace(' ',''))
                    for cont_region in cont_regions:
                        fLow = casatools.quanta.convert('%s%s' % (cont_region[0], cont_region[3]), 'GHz')['value']
                        fHigh = casatools.quanta.convert('%s%s' % (cont_region[2], cont_region[3]), 'GHz')['value']
                        cont_ranges[source_name][spw_id].append((fLow,fHigh))
            except:
                pass

        return cont_ranges

    def write(self, cont_ranges):
        fd = open(self.filename, 'w+')
        for source_name in cont_ranges.iterkeys():
            fd.write('%s\n\n' % (source_name.replace('"','')))
            for spw_id in cont_ranges[source_name].iterkeys():
                fd.write('SPW%s\n' % (spw_id))
                if (cont_ranges[source_name][spw_id] == []):
                    fd.write('NONE\n')
                else:
                    for freq_range in cont_ranges[source_name][spw_id]:
                        if (freq_range == 'NONE'):
                            fd.write('NONE\n')
                        else:
                            fd.write('%s~%sGHz\n' % (freq_range[0], freq_range[1]))
                fd.write('\n')
        fd.close()
