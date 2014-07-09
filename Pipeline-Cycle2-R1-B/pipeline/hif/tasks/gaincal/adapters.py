import os

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.adapters as adapters 

LOG = infrastructure.get_logger(__name__)


class PhaseUpSolIntAdapter(adapters.Adapter):
    def __init__(self, heuristic):
        super(PhaseUpSolIntAdapter, self).__init__(heuristic)

    def calculate(self, ms, intent):
        """Get the range of phase-up solution intervals.
        
        :ms: the ms object under consideration
        :intent: the data intent of interest
        """
        int_time = ms.get_median_integration_time(intent)
        return self._adaptee(int_time)


class MedianSNRAdapter(adapters.Adapter):
    def __init__(self, heuristic):
        """Create a new MedianSNRAdapter, wrapping the given heuristic.
        
        :param heuristic: the target heuristic
        :type heuristic: :class:`~pipeline.heuristics.bandpass.MedianSNR`
        """
        super(MedianSNRAdapter, self).__init__(heuristic)
    
    def calculate(self, caltable):
        """Opens and reads the SNR column from the given caltable,
        passing the column to the wrapped heuristic and returning the result.
        
        :param caltable: the caltable filename 
        :type caltable: string
        """
        # return early if the file does not exist
        filename = caltable.name
        if not os.path.exists(filename):
            LOG.warning('Table {0} does not exist. '
                        'Returning a score of -1'.format(filename))
            return -1

        # get the target spectral window from the job request 
        spw = caltable.job.kw['spw']

        cal_desc = os.path.join(filename, 'CAL_DESC')        
        with casatools.TableReader(cal_desc) as table:
            caldesc_2_spw = table.getcol('SPECTRAL_WINDOW_ID')[0]
            caldesc_id = numpy.arange(len(caldesc_2_spw))[caldesc_2_spw==spw]

        # Read the results themselves. Limiting the length of the gain and
        # cal_flag arrays is to correct for the fact that the calibration
        # table arrays all have the number of channels of the largest spw.
        # This should change in the future.
        with casatools.TableReader(filename) as table:
            taql = 'CAL_DESC_ID=={0}'.format(caldesc_id) 
            subtable = table.query(query=taql)
            snr = subtable.getcol('SNR')
            calflag = subtable.getcol('FLAG')
	    subtable.close()

            valid_snr = snr[numpy.logical_not(calflag)]
            
            return self._adaptee(valid_snr)
