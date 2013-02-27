from __future__ import absolute_import
import os
import types

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)


class CalibrationTable(object):
    """    
    Caltable represents a CASA calibration table used for pipeline on-the-fly
    calibrations. It holds the calibration table name along with the job that
    created it.
    """
    
    def __init__(self, job, description=""):
        self.job = job
        self.description = description
        self._spwmap = None
        self._spw = None
        self.applied = False
        self.interp = None
        self.to_field = None

    def exists(self):
        if self.name is None:
            return False
        return os.path.exists(self.name)

    @property
    def casa_task(self):
        return self.job.fn.__name__

    @property
    def channels(self):
        if 'spw' in self.job.kw:
            return self.job.kw['spw'].split(':')[1]
        return None

    @property
    def field(self):
        return self.job.kw.get('field', None)

    @property
    def gaintable(self):
        if 'gaintable' in self.job.kw:
            return self.job.kw['gaintable']
        return None

    @property
    def intent(self):
        return self.job.kw.get('intent', None)

    @property
    def interp(self):
        return self._interp

    @interp.setter
    def interp(self, value):
        if value is None:
            value = 'linear,linear'
        self._interp = value
    
    @property
    def name(self):
        if 'caltable' in self.job.kw:
            return self.job.kw['caltable']
        return None

    @property
    def basename(self):
        name = self.name
        return os.path.basename(name) if name is not None else None

    @property
    def spw(self):
        if self._spw is not None:
            return self._spw
        if 'spw' in self.job.kw:
            return self.job.kw['spw'].split(':')[0]
        return None

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def spwmap(self):
        if self._spwmap is not None:
            return self._spwmap
        if 'spw' in self.job.kw:
            return self.job.kw['spw'].split(':')[0]
        return None

    @spwmap.setter
    def spwmap(self, value):
        if type(value) is types.ListType and len(value) is 1:
            value = value[0]
        self._spwmap = value

    @property
    def to_field(self):
        return self._to_field

    @to_field.setter
    def to_field(self, value=None):
        if value is None:
            value = set()
        self._to_field = value

    @property
    def vis(self):
        return self.job.kw['vis']

    def __repr__(self):
        return '<CalibrationTable({name})>'.format(name=self.basename)



class UserCalibrationTable(object):
    """    
    A CalibrationTable wrapper for a caltable created outside the pipeline.
    """
    
    def __init__(self, name):
        if not os.path.exists(name):
            msg = '%s not found' % name
            LOG.critical(msg)
            raise IOError, msg
        
        self._name = os.path.abspath(name)
        with casatools.TableReader(self.name) as caltable:
            self._vis = caltable.getkeyword('MSName')
            
        self.interp = None
        self.spwmap = None
        self.applied = False
        self.to_field = None

    def exists(self):
        # we checked whether this table exists on construction
        return True

    @property
    def basename(self):
        return os.path.basename(self.name)

    @property
    def interp(self):
        return self._interp

    @interp.setter
    def interp(self, value):
        if value is None:
            value = 'linear,linear'
        self._interp = value

    @property
    def name(self):
        return self._name

    @property
    def spwmap(self):
        return self._spwmap

    @spwmap.setter
    def spwmap(self, value):
        if value is None:
            value = []

#        if type(value) is types.ListType and len(value) is 1:
#            value = value[0]
        self._spwmap = value

    @property
    def to_field(self):
        return self._to_field

    @to_field.setter
    def to_field(self, value=None):
        if value is None:
            value = set()
        if type(value) is types.StringType:
            value = set(','.join(value.split(',')))
        if isinstance(value, list):
            value = set(value)
        self._to_field = value

    @property
    def vis(self):
        return self._vis

    def __repr__(self):
        return '<UserCalibrationTable({name})>'.format(name=self.basename)
