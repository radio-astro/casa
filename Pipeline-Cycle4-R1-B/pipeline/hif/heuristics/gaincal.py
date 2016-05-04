import os
import numpy
import pipeline.infrastructure.api as api
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)



class MedianSNRNoAdapter(api.Heuristic):
    def calculate(self, caltable):
        """Opens and reads the SNR column from the given caltable,
        and returns the median value.
        
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
            caldesc_id = numpy.arange(len(caldesc_2_spw))[caldesc_2_spw==int(spw)][0]

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
            
        if len(valid_snr) > 0:
            return numpy.median(valid_snr)
        else:
            return 0

