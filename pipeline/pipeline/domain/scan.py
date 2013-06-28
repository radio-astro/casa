from __future__ import absolute_import
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class Scan(object):
    """
    Class containing info about a single scan.
    """
    def __init__(self, id=None, antennas=[], intents=[], fields=[],
                 states=[], data_descriptions=[], scan_times=[]):
        self.id = id
        
        self.antennas = set(antennas)
        self.fields = set(fields)
        self.intents = set(intents)
        self.states = set(states)
        self.data_descriptions = set(data_descriptions)
        
        self.scan_times = sorted(scan_times,
                                 key=lambda t:utils.get_epoch_as_datetime(t[0]))

    def __repr__(self):
        mt = casatools.measures
        return ('<Scan #{id}: intents=\'{intents}\' start=\'{start}\' '
                'end=\'{end}\' duration=\'{duration}\'>'.format(
                    id=self.id,
                    intents=','.join(self.intents),
                    start=mt.show(self.start_time), 
                    end=mt.show(self.end_time), 
                    duration=str(self.time_on_source)))

    @property
    def start_time(self):
        return self.scan_times[0][0]

    @property
    def end_time(self):
        return self.scan_times[-1][1]

    @property
    def time_on_source(self):
        # adding up the scan exposures does not give us the total time on 
        # source. Instead we should simply subtract the scan end time from the 
        # scan start time to calculate the total time
        start = utils.get_epoch_as_datetime(self.start_time)
        end = utils.get_epoch_as_datetime(self.end_time)
        return end - start

    @property
    def mean_exposure(self):
        qt = casatools.quanta
        return qt.div(self.time_on_source, len(self.scan_times))

    @property
    def max_exposure(self):
        qt = casatools.quanta
        max_exposure = self.scan_times[0][2]
        for exposure in [s[2] for s in self.scan_times]:
            if qt.gt(exposure, max_exposure):
                max_exposure = exposure
        return max_exposure
        
    @property
    def spws(self):
        return set([dd.spw for dd in self.data_descriptions])
