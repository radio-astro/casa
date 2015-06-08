from __future__ import absolute_import
import datetime

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
        
        self.antennas = frozenset(antennas)
        self.fields = frozenset(fields)
        self.intents = frozenset(intents)
        self.states = frozenset(states)
        self.data_descriptions = frozenset(data_descriptions)

        # the integration time should not vary per subscan, so we can
        # construct the integration-time-per-spw mapping directly from the
        # first subscan entry. The MS spec states that these integration 
        # values are given in seconds, which is just what we want
        int_seconds = {k : v[0][1]['value']
                       for k,v in scan_times.iteritems()}
        self.__mean_intervals = {k : datetime.timedelta(seconds=v)
                                 for k, v in int_seconds.items()}

        # will hold the start and end epochs per spw
        self.__start_time = None
        self.__end_time = None
        
        # midpoints is a list of tuple of (midpoint epochs, integation time)            
        sorted_epochs = {spw_id : sorted(midpoints, key=lambda e: e[0]['m0']['value'])
                             for spw_id, midpoints in scan_times.iteritems()}

        qt = casatools.quanta
        mt = casatools.measures            
                
        self.__exposure_time = {}
        for spw_id, epochs in sorted_epochs.iteritems():
            (min_epoch, exposure) = epochs[0]
            max_epoch = epochs[-1][0]
                         
            # add and subtract half the exposure to get the start and
            # end exposure times for this spw in the scan
            half_exposure = qt.div(exposure, 2)
            min_val = qt.sub(mt.getvalue(min_epoch)['m0'], half_exposure)
            max_val = qt.add(mt.getvalue(max_epoch)['m0'], half_exposure)

            # recalculate epochs for these adjusted times, which we can use to
            # set the mean interval for this spw and potentially for the
            # global start and end epochs for this scan
            range_start_epoch = mt.epoch(v0=min_val, rf=mt.getref(min_epoch))
            range_end_epoch = mt.epoch(v0=max_val, rf=mt.getref(max_epoch))

            dt_start = utils.get_epoch_as_datetime(range_start_epoch)
            dt_end = utils.get_epoch_as_datetime(range_end_epoch)
            self.__exposure_time[spw_id] = dt_end - dt_start            

            # set start time as earliest time over all spectral windows
            if self.__start_time is None or qt.lt(min_val, self.__start_time['m0']):
                self.__start_time = range_start_epoch

            # set end time as latest time over all spectral windows
            if self.__end_time is None or qt.gt(max_val, self.__end_time['m0']):
                self.__end_time =  range_end_epoch

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

    def exposure_time(self, spw_id):
        return self.__exposure_time[spw_id]

    def mean_interval(self, spw_id):
        return self.__mean_intervals[spw_id]

    @property
    def spws(self):
        return set([dd.spw for dd in self.data_descriptions])
