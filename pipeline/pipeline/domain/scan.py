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
        
        sorted_scan_times = {}
        for dd, dd_times in scan_times.items():
            sorted_scan_times[dd] = sorted(dd_times,
                                           key=lambda t:utils.get_epoch_as_datetime(t[0]))

        # set start time as earliest time over all data descriptions
        min_times = sorted([v[0] for v in sorted_scan_times.values()],
                           key=lambda t:utils.get_epoch_as_datetime(t[0]))
        self.__start_time = min_times[0][0]
        
        # set end time as latest time over all data descriptions
        max_times = sorted([v[-1] for v in sorted_scan_times.values()],
                            key=lambda t:utils.get_epoch_as_datetime(t[1]))
        self.__end_time = max_times[-1][1]

        # calculate mean intervals for each spectral window observed by this scan
        spw_ids = [dd.spw.id for dd in data_descriptions]
        self.__mean_intervals = dict((spw_id, 
                                      self.__calculate_mean_interval(sorted_scan_times, 
                                                                     spw_id))
                                     for spw_id in spw_ids)

    def __repr__(self):
        return ('<Scan #{id}: intents=\'{intents}\' start=\'{start}\' '
                'end=\'{end}\' duration=\'{duration}\'>'.format(
                    id=self.id,
                    intents=','.join(self.intents),
                    start=utils.get_epoch_as_datetime(self.start_time), 
                    end=utils.get_epoch_as_datetime(self.end_time), 
                    duration=str(self.time_on_source)))

    @property
    def start_time(self):
        return self.__start_time

    @property
    def end_time(self):
        return self.__end_time

    @property
    def time_on_source(self):
        # adding up the scan exposures does not give us the total time on 
        # source. Instead we should simply subtract the scan end time from the 
        # scan start time to calculate the total time
        start = utils.get_epoch_as_datetime(self.start_time)
        end = utils.get_epoch_as_datetime(self.end_time)
        return end - start

    def mean_interval(self, spw_id):
        return self.__mean_intervals[spw_id]

    def __calculate_mean_interval(self, scan_times, spw_id):
        """
        Calculate the mean interval for this scan for the given spectral window.
        """         
        dds_with_spw = [dd for dd in self.data_descriptions
                        if spw_id == dd.spw.id]
        if not dds_with_spw:
            raise ValueError('Scan %s not linked to spectral '
                             'window %s' % (self.id, spw_id))
        
        # I don't think it's possible to have the same spw associated with
        # a scan more than once via multiple data descriptions, but assert
        # just in case
        assert len(dds_with_spw) is 1, ('Expected 1 data description for spw '
                                        '%s but got %s' % (spw_id, 
                                                           len(dds_with_spw)))

        times_for_spw = scan_times[dds_with_spw[0]]
        start = utils.get_epoch_as_datetime(times_for_spw[0][0])
        end = utils.get_epoch_as_datetime(times_for_spw[-1][1])
        mean = (end - start) / len(times_for_spw)
        return mean

    @property
    def spws(self):
        return set([dd.spw for dd in self.data_descriptions])
