from pipeline.extern.findContinuum import findContinuum

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class FindContHeuristics(object):
    def __init__(self, context):
        self.context = context

    def find_continuum(self, dirty_cube, pb_cube=None, psf_cube=None, single_continuum=False):
        with casatools.ImageReader(dirty_cube) as image:
            stats = image.statistics()

        if (stats['min'][0] == stats['max'][0]):
            LOG.error('Cube %s is constant at level %s.' % (dirty_cube, stats['max'][0]))
            return ([{'range': 'NONE', 'refer': 'LSRK'}], 'none')

        # Run continuum finder on cube
        channel_selection, png_name, aggregate_bw = findContinuum(img=dirty_cube, pbcube=pb_cube, psfcube=psf_cube, singleContinuum=single_continuum)
        frequency_ranges_GHz = \
            [{'range': item, 'refer': 'LSRK'} for item in utils.chan_selection_to_frequencies(dirty_cube, channel_selection, 'GHz')]
        return (frequency_ranges_GHz, png_name)
