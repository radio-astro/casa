from __future__ import absolute_import

import os
import abc
import numpy

import pipeline.infrastructure.utils as utils

DPISummary = 90
#DPIDetail = 120
DPIDetail = 130

class SingleDishDisplayInputs(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result

class SDImageDisplayInputs(SingleDishDisplayInputs):
    @property
    def imagename(self):
        return self.result.outcome['image'].imagename

    @property
    def spw(self):
        return self.result.outcome['image'].spwlist

    @property
    def antenna(self):
        return self.result.outcome['image'].antenna

    @property
    def stage_number(self):
        return self.result.stage_number

    @property
    def stage_dir(self):
        return os.path.join(self.context.report_dir,
                            'stage%d'%(self.stage_number))

    @property
    def source(self):
        return self.result.outcome['image'].sourcename

    @property
    def num_valid_spectrum(self):
        return self.result.outcome['validsp']

    @property
    def rms(self):
        return self.result.outcome['rms']


class SDInspectionDisplay(object):
    __metaclass__ = abc.ABCMeta
    Inputs = SingleDishDisplayInputs
    
    def __init__(self, inputs):
        self.inputs = inputs
        self.context = self.inputs.context
        self.result = self.inputs.result
        self.datatable = self.result.outcome['instance']

    def plot(self):
        inputs = self.inputs
        result = inputs.result
        
        if result.outcome is None or result.success is None or result.success is False:
            # result object seems to be empty, return empty list
            return []

        plots = []
        report_dir = self.context.report_dir
        stage_dir = os.path.join(report_dir, 'stage%d'%(result.stage_number))
        print report_dir
        filenames = self.datatable.getkeyword('FILENAMES')
        print filenames
        for idx in xrange(len(filenames)):
            plot = self.doplot(idx, stage_dir)
            if plot is not None:
                plots.append(plot)
        return [plots]

    @abc.abstractmethod
    def doplot(self, idx, stage_dir):
        raise NotImplementedError()


class SDCalibrationDisplay(object):
    __metaclass__ = abc.ABCMeta
    Inputs = SingleDishDisplayInputs

    def __init__(self, inputs):
        self.inputs = inputs

    def plot(self):
        results = self.inputs.result
        report_dir = self.inputs.context.report_dir
        stage_dir = os.path.join(report_dir, 'stage%d'%(results.stage_number))
        plots = []
        for result in results:
            #table = result.outcome.applytable
            plot = self.doplot(result, stage_dir)
            if plot is not None:
                plots.append(plot)
        return plots

    @abc.abstractmethod
    def doplot(self, result, stage_dir):
        raise NotImplementedError()

def get_base_frequency(table, freqid, nchan):
    freq_table = os.path.join(table, 'FREQUENCIES')
    with utils.open_table(freq_table) as tb:
        refpix = tb.getcell('REFPIX', freqid)
        refval = tb.getcell('REFVAL', freqid)
        increment = tb.getcell('INCREMENT', freqid)
        chan_freq = numpy.array([refval + (i - refpix) * increment 
                                 for i in xrange(nchan)])
    return chan_freq

def get_base_frame(table):
    freq_table = os.path.join(table, 'FREQUENCIES')
    with utils.open_table(freq_table) as tb:
        base_frame = tb.getkeyword('BASEFRAME')
    return base_frame

def drop_edge(array):
    # array should be two-dimensional (nchan,nrow)
    nchan = array.shape[0]
    a = None
    if nchan > 2:
        echan = max(1,int(nchan * 0.05))
        a = array[echan:-echan,::]
    return a

