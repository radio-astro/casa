from __future__ import absolute_import
from . import measurementset
from . import singledish
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


class VirtualMeasurementSet(measurementset.MeasurementSet):    
    def __init__(self, scantable_list):
        if isinstance(scantable_list, singledish.ScantableRep):
            name = scantable_list.name
            session = scantable_list.session
            scantable_list = [scantable_list]
        else:
            name = ':'.join([s.name for s in scantable_list])
            session = scantable_list[0].session
        super(VirtualMeasurementSet,self).__init__(name, session)
        self.scantables = scantable_list
