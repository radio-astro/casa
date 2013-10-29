from __future__ import absolute_import
import collections
import copy
import os.path

import numpy as np 

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


class ResultAxis(object):
    def __init__(self, name, units, data, channel_width=None):
        self.name = name
        self.units = units
        self.data = data
        if channel_width is None:
            if len(data) > 1:
                self.channel_width = data[1] - data[0]
            else:
                self.channel_width = 0
        else:
            self.channel_width = channel_width


class ResultBase(object):

    @property
    def fieldname(self):
        if self._field_name is None:
            return str(self.field_id)
        else:
            return self._field_name

    @property
    def description(self):
        if self.time is None:
            tstring = None
        else:
            # represent time sensibly relative to day start
            t = self.time - 86400.0 * np.floor(self.time/86400.0)
            h = int(np.floor(t/3600.0))
            t -= h * 3600.0
            m = int(np.floor(t/60.0))
            t -= m * 60.0
            s = int(np.floor(t))
            tstring = '%sh%sm%ss' % (h,m,s)

        temp = self.filename
        if self.filename is not None:
            temp = os.path.basename(self.filename)

        fields = [('File',temp), 
                  ('Intent',self.intent),
                  ('Field',self.fieldname),
                  ('ID',self.field_id),
                  ('SpW',self.spw),
                  ('Pol',self.pol),
                  ('Ant',self.ant),
                  ('Time',tstring)]
        return ' '.join('%s:%s' % (k) for k in fields if k[1] is not None)


class ImageResult(ResultBase):
    def __init__(self, filename, data, datatype, axes, flag=None, nodata=None,
                 intent=None, field_id=None, field_name=None, spw=None,
                 pol=None, ant=None, units=None, time=None):
        self.filename = filename
        self.data = data
        self.axes = axes

        if flag is None:
            self.flag = np.zeros(np.shape(self.data), np.bool)
        else:
            self.flag = flag

        if nodata is None:
            self.nodata = np.zeros(np.shape(self.data), np.bool)
        else:
            self.nodata = nodata

        self.datatype = datatype
        self.field_id = field_id
        self._field_name = field_name
        self.intent = intent
        self.spw = spw
        self.pol = pol
        self.ant = ant
        self.units = units
        self.time = time

        self.children = {}


class SpectrumResult(ResultBase):
    def __init__(self, data, datatype, data_mad=None, axis=None, flag=None,
                 nodata=None, noisychannels=None, filename=None,
                 intent=None, field_id=None, field_name=None, spw=None,
                 pol=None, ant=None,
                 units=None, time=None, normalise=False):
        self.filename = filename

        if flag is None:
            self.flag = np.zeros(np.shape(data), np.bool)
        else:
            self.flag = flag

        valid_data = data[self.flag==False]
        if len(valid_data) > 0:
            self.median = np.median(valid_data)
        else:
            self.median = 0

        if normalise and self.median > 0:
            self.data = data / self.median
        else:
            self.data = data

        if data_mad is None:
            self.data_mad = np.zeros(np.shape(self.data))
        else:
            if normalise and self.median > 0:
                self.data_mad = data_mad / self.median
            else:
                self.data_mad = data_mad

        if axis is None:
            self.axis = ResultAxis('channel', '',
              np.arange(np.shape(self.data)[0]))
        else:
            self.axis = axis
        
        if nodata is None:
            self.nodata = np.zeros(np.shape(self.data), np.bool)
        else:
            self.nodata = nodata

        if noisychannels is None:
            self.noisychannels = np.zeros(np.shape(self.data), np.bool)
        else:
            self.noisychannels = np.array(noisychannels)

        self.ant = ant
        self.datatype = datatype
        self.intent = intent
        self.field_id = field_id
        self._field_name = field_name
        self.pol = pol
        self.spw = spw
        self.time = time
        self.units = units


class Qa2Result(object):
    def __init__(self):
        self.overall_score = None
        self.view_score = {}
        self.view = collections.defaultdict(list)
        self.flagging = []

    def addview(self, description, viewresult):
        self.view[description].append(viewresult)

    def descriptions(self):
        return self.view.keys()

    def first(self, description):
        return copy.deepcopy(self.view[description][0])

    def flagcmds(self):
        return self.flagging

    def last(self, description):
        return copy.deepcopy(self.view[description][-1])


