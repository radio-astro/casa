from __future__ import absolute_import

import os
import numpy
import pylab as pl
import collections

import pipeline.infrastructure.renderer.logger as logger
from .common import DPISummary

class K2JyHistDisplay(object):
    '''
    A display class to generate histogram of Jy/K factors
    '''

    def __init__(self, stage, spw, valid_factors, bandname=''):
        '''
        stage: stage number
        spw: spw ID of valid_factors
        valid_factors: a dictionary or an array of Jy/K valid_factors to generate histogram
        '''
        self.stage_dir = stage
        self.spw = spw
        self.band = bandname
        if type(valid_factors)==dict or numpy.iterable(valid_factors)==1:
            self.factors = valid_factors
        else:
            raise ValueError, "valid_factors should be dictionary or an iterable"
    
    def plot(self):
        pl.ioff()
        pl.clf()

        return list(self._plot())

    def _create_plot(self, plotfile, x_axis, y_axis):
        parameters = {}
        parameters['spw'] = self.spw
        parameters['receiver'] = self.band
        plot_obj = logger.Plot(plotfile,
                               x_axis=x_axis,
                               y_axis=y_axis,
                               parameters=parameters)
        return plot_obj

    def _plot(self):
        if type(self.factors) in [dict, collections.defaultdict]:
            labels = []
            factors = []
            for lab, spw_factors in self.factors.items():
                dummy, f = collect_dict_values(spw_factors)
                factors.append(f)
                labels.append(lab)
        elif numpy.iterable(self.factors):
            labels = 'all data'
            factors = list(self.factors)
        # define binning
        factors1d = []
        for f in factors:
            factors1d+=f
        data = numpy.array(factors1d)
        medval = numpy.median(data)
        bin_width = medval*0.05
        maxval = max(data.max(), medval*1.3)
        minval = min(data.min(), medval*0.7)
        num_bin = int(numpy.ceil(max((maxval-minval)/bin_width, 1)))

        pl.hist(factors, range=[minval, maxval], bins=num_bin,
                histtype='barstacked', align='mid', label=labels)
        #pl.hist(factors, bins='auto', histtype='barstacked', align='mid', label=labels)
        pl.xlabel('Jy/K factor', fontsize=11)
        pl.ylabel('Numbers', fontsize=11)
        pl.title('Jy/K factors (SPW %d)' % self.spw, fontsize=11)

        plotfile = os.path.join(self.stage_dir,
                                'jyperk_spw%s.png'%(self.spw))
        pl.savefig(plotfile, format='png', dpi=DPISummary)
        plot = self._create_plot(plotfile, 'Jy/K factor', 'Number of MS, ant, and pol combinations')
        yield plot

def collect_dict_values(in_value):
    '''
    Returns a list of values in in_value. When
    in_value = dict(a=1, b=dict(c=2, d=4))
    the method collects all values in tips of branches and returns,
    [1, 2, 4]
    When in_value is a simple number or an array, it returns a list
    of the number or the array
        
    in_value: a dictionary, number or array to collect values and construct a list
    '''
    if type(in_value) not in [dict, collections.defaultdict]:
        if numpy.iterable(in_value)==0:
            in_value = [in_value]
        return True, list(in_value)
    out_factor = []
    for value in in_value.itervalues():
        done = False
        while not done:
            done, value = collect_dict_values(value)
        out_factor += value
    return done, out_factor
