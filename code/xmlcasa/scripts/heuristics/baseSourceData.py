"""Module with Base Class for 'views' of data from a particular source type."""

# History:
# 16-Jul-2007 jfl First version.

# package modules

from numpy import *

# alma modules

import casac
from baseData import *

class BaseSourceData(BaseData):
    """Base Class for 'views' of data from a particular source type. 
    """

    def __init__(self, htmlLogger, msName, sourceType):
        """Constructor.

        Keyword arguments:
        htmlLogger -- Route for logging to html structure.
        msName     -- Name of MeasurementSet
        sourceType -- Type of source to be imaged.
        """

        print 'BaseSourceData.__init__ called'
        BaseData.__init__(self, htmlLogger, msName)

        self._htmlLogger.timing_start('BaseSourceData.__init__')
        self._sourceType = sourceType

# get range of data_desc_ids

        self._ms.open(thems=self._msName)
        msInfo = self._ms.range(items=['data_desc_id'])
        self._data_desc_ids = msInfo['data_desc_id']

# get map of data_desc_id to spectral_window_id

        self._table.open(self._msName + '/DATA_DESCRIPTION')
        self._spectral_window_id = self._table.getcol(columnname=
         'SPECTRAL_WINDOW_ID')
        self._table.close()

# and reference frequencies for each spectral_window_id

        self._table.open(self._msName + '/SPECTRAL_WINDOW')
        self._ref_frequency = self._table.getcol(columnname='REF_FREQUENCY')
        self._num_chan = self._table.getcol(columnname='NUM_CHAN')
        self._table.close()

# get names of fields

        self._table.open(self._msName + '/FIELD')
        self._fieldName = self._table.getcol('NAME')
        self._table.close()

# translate source type to field Ids

        self._field_ids = self.getFieldsOfType(self._sourceType)

# get range of antennas

        self._antennaRange = self._getAntennaRange()
        self._ms.close()

        self._htmlLogger.timing_stop('BaseSourceData.__init__')
