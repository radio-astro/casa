from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
from . import SDChannelAveragedImageDisplay
from . import SDSpectralImageDisplay

LOG = infrastructure.get_logger(__name__)

def SDImageDisplayFactory(mode):
    LOG.debug('MODE=%s'%(mode))
    if mode == 'TP':
        return SDChannelAveragedImageDisplay

    else:
        # mode should be 'SP'
        return SDSpectralImageDisplay
