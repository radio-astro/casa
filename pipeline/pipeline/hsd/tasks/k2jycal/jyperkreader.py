import cStringIO as StringIO
import os
import numpy
import csv
import contextlib

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)

def read(context, filename):
    """
    Reads jyperk factors from a file and returns a string list
    of [['MS','ant','spwid','polid','factor'], ...]
    """
    filetype = inspect_type(filename)
    if filetype == 'MS-Based':
        LOG.debug('MS-Based Jy/K factors file is specified')
        return read_ms_based(filename)
    else:
        LOG.debug('Session-Based Jy/K factors file is specified')
        return read_session_based(context, filename)

def inspect_type(filename):
    with open(filename, 'r') as f:
        line = f.readline()
    if line[0] == '#':
        return 'Session-Based'
    else:
        return 'MS-Based'

def read_ms_based(reffile):
    #factor_list = []
    with open(reffile, 'r') as f:
        return list(_read_stream(f))
                
def read_session_based(context, reffile):
    parser = JyPerKDataParser
    storage = JyPerK()
    with open(reffile, 'r') as f:
        for line in f:
            if line[0] == '#':
                # header
                meta = parser.parse_header(line)
                #print meta
                storage.register_meta(meta)
            else:
                # data
                data = parser.parse_data(line)
                #print data
                storage.register_data(data)
                
    with associate(context, storage) as f:
        return list(_read_stream(f))

def _read_stream(stream):
    reader = csv.reader(stream)
    # Check if first line is header or not
    line = reader.next()
    if line[0].strip().upper() == 'MS' or line[0].strip()[0] == '#':
        # must be a header or commented line
        pass
    elif len(line) == 5:
        # may be a data record
        #factor_list.append(line)
        yield line
    else:
        LOG.error('Jy/K factor file is invalid format')
    for line in reader:
        if len(line) == 0 or len(line[0]) == 0 or line[0][0] == '#':
            continue
        elif len(line) == 5:
            yield line
        else:
            LOG.error('Jy/K factor file is invalid format')
            
# Utility classes/functions to convert session based factors file 
# to MS based one are defined below
class JyPerKDataParser(object):
    @classmethod
    def get_content(cls, line):
        return line.strip('# \n\t')

    @classmethod
    def parse_header(cls, line):
        content = cls.get_content(line)
        if content.find('=') != -1:
            # this must be a meta data
            return tuple(content.split('='))
        elif content.find(',') != -1 and not content[0].isdigit():
            # this must be a data header
            return content.split(',')
        else:
            # empty line or commented data, ignored
            return None

    @classmethod
    def parse_data(cls, line):
        content = cls.get_content(line)
        if content.find(',') != -1:
            # data 
            return content.split(',')
        else:
            # invalid or empty line, ignored
            return None

class JyPerK(object):
    """
    Parse session based jyperk csv and store.
    * meta stores meta data information from the lines in the form, '#name=value',
        as a dictionary, meta[name]=value.
    * header stores column label from the line in the form '#header0, header1, ...'
        as a list, header = ['header0', 'header1', ...]
    * data stores values in csv file as a dictionary,
        data['header0'] = [data00, data01, ...]
    """
    def __init__(self):
        self.meta = dict()
        self.header = []
        self.data = []

    def register_meta(self, content):
        if isinstance(content, list):
            # this should be data header
            self.header = content
            self.data = dict(((k,[]) for k in self.header))
        elif isinstance(content, tuple):
            self.meta[content[0]] = content[1]

    def register_data(self, content):
        assert len(self.header) > 0
        assert len(self.header) == len(content)
        for (k,v) in zip(self.header, content):
            self.data[k].append(v)

@contextlib.contextmanager
def associate(context, factors):
    """
    Convert data collected from session based jyperk csv as JyPerK object
    to MS-beased csv, i.e., a string list of ['MS,ant,spwid,polid,factor', ...]
    """
    stream = StringIO.StringIO()
    try:
        data = factors.data
        for ms in context.observing_run.measurement_sets:
            session_name = ms.session
            if session_name == 'Session_default':
                # Session_default is not supported, use Session_1 instead
                LOG.warn('Session for %s is \'Session_default\'. Use \'Session_1\' for application of Jy/K factor. '%(ms.basename))
                session_id = 1
            else:
                # session_name should be 'Session_X' where X is an integer
                session_id = int(session_name.split('_')[-1])
            #print 'ms', ms.basename, 'session_id', session_id
            session_list = numpy.array(map(lambda x: int(x), data['sessionID']))
            #print 'session_list', session_list
            idx = numpy.where(session_list == session_id)
            #print 'index list', idx
            #print 'selected sessions', session_list[idx]
    
            antennas = map(lambda x: x.name, ms.antennas)
            antenna_list = data['Antenna']
    
            factor_list = numpy.array(map(lambda x: float(x), data['Factor']))
            #print factor_list
            
            spws = ms.get_spectral_windows()
            bandcenter = numpy.array(map(lambda x: float(x) * 1.0e6, data['BandCenter(MHz)']))
            bandwidth = numpy.array(map(lambda x: float(x) * 1.0e6, data['BandWidth(MHz)']))
            range_min = bandcenter - 0.5 * bandwidth
            range_max = bandcenter + 0.5 * bandwidth
            for spw in spws:
                max_freq = float(spw.max_frequency.value)
                min_freq = float(spw.min_frequency.value)
                tot_bandwidth = float(spw.bandwidth.value)
                #print 'spw', spw.id, min_freq, max_freq
    
                spwid = spw.id
                d = {}
                for i in idx[0]:
                    #coverage = inspect_coverage(min_freq, max_freq, range_min[i], range_max[i])
                    antenna = antenna_list[i]
                    if d.has_key(antenna):
                        #d[antenna].append([i, coverage, bandwidth[i]])
                        d[antenna].append(i)
                    else:
                        #d[antenna] = [[i, coverage, bandwidth[i]]]
                        d[antenna] = [i]
    
                for ant in antennas:
                    if d.has_key(ant):
                        f = d[ant]
                    else:
                        LOG.info('%s: No factors available for spw %s antenna %s use ANONYMOUS'%(session_name,spwid,ant))
                        f = d['ANONYMOUS']
                    #print ant, f
                    coverage_list = map(lambda x: inspect_coverage(min_freq, max_freq, range_min[x], range_max[x]), f)
                    #print coverage_list
                    #_best_index = numpy.argmax(coverage_list)
                    best_index = f[0]
                    _best_score = inspect_coverage(min_freq, max_freq, range_min[f[0]], range_max[f[0]])
                    #print f[0], _best_score
                    for _i in f[1:]:
                        coverage = inspect_coverage(min_freq, max_freq, range_min[_i], range_max[_i])
                        #print _i, coverage
                        if coverage > _best_score:
                            best_index = _i
                            _best_score = coverage
                    #print best_index
                    line = '%s,%s,%s,%s,%s'%(ms.basename,ant,spwid,data['POL'][best_index],factor_list[best_index])
                    LOG.debug(line)
                    stream.write(line + '\n')
        
        stream.seek(0,0)
        yield stream
        
    finally:
        stream.close()
    
def inspect_coverage(minval, maxval, minref, maxref):
    if minval > maxval or minref > maxref:
        return 0.0

    coverage = (min(maxval, maxref) - max(minval, minref)) / (maxval - minval)
    #print minval, maxval, minref, maxref, coverage

    bandwidth_ratio = (maxref - minref) / (maxval - minval)
    
    if coverage < 0.0 or coverage > 1.0 or bandwidth_ratio > 1.1:
        return 0.0

    return coverage
