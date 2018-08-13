"""
The adapters module contains classes that adapt Heuristics in the
:class:`pipeline.heuristics` package, allowing them to be called directly with
low-level objects from the pipeline domain.

Heuristics in the pipeline.heuristics package have simple interfaces; to
maximize their ease of use, their interfaces accept only Python and NumPy
primitives. 

As the pipeline deals with complex lower-level objects and structures, we need
a way to convert from these structures to the simple primitives demanded by
the Heuristics. The classes in this module encapsulate that logic, converting
objects or reading calibration tables as necessary and extracting the
information needed by the wrapped Heuristic.
"""
import os

import numpy

import pipeline.infrastructure.casatools as casatools
from pipeline.h.heuristics import echoheuristic as echoheuristic
import pipeline.infrastructure.adapters as adapters 
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)



class PhaseUpSolIntAdapter(adapters.Adapter):
    """PhaseUpSolIntAdapter adapts the
    :class:`~pipeline.hif.heuristics.solint.PhaseUpSolInt` heuristic, taking in a
    MeasurementSet and intent and passing the median integration time for
    observations with that intent through to the science heuristic.
    
    """
    def __init__(self, heuristic):
        """Create a new PhaseUpSolIntAdapter, wrapping the given heuristic.
        
        :param heuristic: the target heuristic
        :type heuristic: :class:`~pipeline.hif.heuristics.solint.PhaseUpSolInt`
        """
        super(PhaseUpSolIntAdapter, self).__init__(heuristic)

    def calculate(self, ms, intent):
        """Extract the integration time for observations with the given intent
        from the measurement set and pass it to the wrapped heuristic,
        returning the result.
        
        :param ms: the MeasurementSet to examine 
        :type ms: :class:`~pipeline.domain.measurementset.MeasurementSet`
        :param intent: the intent for which to find the integration time 
        :type intent: string
        """
        int_time = ms.get_median_integration_time(intent)
        return self._adaptee(int_time)


class PolynomialHeuristicAdapter(adapters.Adapter):
    """
    PolynomialHeuristicAdapter converts from the pipeline interface for a
    polynomial order heuristic, the string name of a caltable, to the data
    types needed by the heuristic itself, which are numerical data arrays.    
    """    
        
    def __init__(self, heuristic):
        """
        Create a new PolynomialHeuristicAdapter, wrapping the given heuristic.
        
        :param heuristic: the target heuristic
        :type heuristic: :class:`~pipeline.hifheuristics.bporder.BPOrder`
        """
        super(PolynomialHeuristicAdapter, self).__init__(heuristic)

    def calculate(self, ms, caltable, spw):        
        if isinstance(self._adaptee, echoheuristic.EchoHeuristic):
            return self._adaptee.calculate()

        data = self.read_caltable(ms, caltable, spw)
 
        channel_data = self._get_channel_data(data, ms.antennas)

        return self._adaptee(channel_data=channel_data)

    def read_caltable(self, ms, caltable, spw):
        """
        Method to read channel calibration table and return it as a Python
        array.

        Keyword arguments:
        table  -- The name of the calibration table
        spw    -- The SpW of interest.
        msobject -- MeasurementSet domain object from which the calibration
                    was calculated.

        Returns:
        data - 'view' of calibration gains in numpy complex array[antenna,channel]
        flag - flags associated with data array bool[antenna,channel]
        """
        if not os.path.exists(caltable.name):
            LOG.warning('Table {0} does not exist. Returning a dummy array'
                        ''.format(caltable.name))
            spw = ms.get_spectral_window(spw)
            num_antenna = len(ms.antennas)            
            return numpy.ones([num_antenna+1, spw.num_channels], numpy.complex)

        spw = int(spw)
        
        # The number of channels in the SpW (needed to correct
        # for the fact that the calibration table arrays all have
        # the number of channels of the largest spw)      
        nchannels = ms.get_spectral_window(spw).num_channels
        npol = 2

        # open the CAL_DESC sub-table and read the column that maps
        # CAL_DESC_ID to SPECTRAL_WINDOW_ID
        cal_desc = os.path.join(caltable.name, 'CAL_DESC')        
        with casatools.TableReader(cal_desc) as table:
            caldesc_2_spw = table.getcol('SPECTRAL_WINDOW_ID')[0]
            caldesc_id = numpy.arange(len(caldesc_2_spw))[caldesc_2_spw==spw]
            if len(caldesc_id) > 0:
                caldesc_id = caldesc_id[0]
            else:
                raise KeyError, 'SpW %s not found' % spw

        # For channel by channel calibrations read the results
        # themselves. Limiting the length of the gain and
        # cal_flag arrays is to correct for the fact that the
        # calibration table arrays all have the number of channels
        # of the largest spw. This should change in the future.
        with casatools.TableReader(caltable.name) as table:
            taql = 'CAL_DESC_ID=={0}'.format(caldesc_id) 
            subtable = table.query(query=taql)
            antenna1 = subtable.getcol('ANTENNA1')
            gain = subtable.getcol('GAIN')[:,:nchannels,:]
            cal_flag = subtable.getcol('FLAG')[:,:nchannels,:]
	    subtable.close()

            # average the polarization results
            antenna_ids = [antenna.id for antenna in ms.antennas]
            num_antenna = len(antenna_ids)        
            data = numpy.zeros([num_antenna+1, nchannels], numpy.complex)
            flag = numpy.zeros([num_antenna+1, nchannels], numpy.bool)

            antennas1 = [antenna1[i] for i in range(numpy.shape(gain)[2])
                         if i in antenna_ids]
            for ant in antennas1:
                for p in range(npol):
                    data[ant,:] += gain[p,:,i]
                    flag[ant,:] = numpy.logical_or(flag[ant,:],
                     cal_flag[p,:,i])
                data[ant,:] /= float(npol)

        return data


class DegAmpAdapter(PolynomialHeuristicAdapter):
    def __init__(self, heuristic):
        """Create a new DegAmpAdapter, wrapping the given heuristic.
        
        :param heuristic: the target heuristic
        :type heuristic: :class:`~pipeline.hif.heuristics.bandpass.BPOrder`
        """
        super(DegAmpAdapter, self).__init__(heuristic)

    def _get_channel_data(self, data, antennas):
        amplitudes = {}
        for antenna in antennas:
            identifier = antenna.name if antenna.name else antenna.id
            amplitudes[identifier] = numpy.abs(data[antenna.id,:])
        return amplitudes


class DegPhaseAdapter(PolynomialHeuristicAdapter):
    def __init__(self, heuristic):
        """Create a new DegPhaseAdapter, wrapping the given heuristic.
        
        :param heuristic: the target heuristic
        :type heuristic: :class:`~pipeline.hif.heuristics.bandpass.BPOrder`
        """
        super(DegPhaseAdapter, self).__init__(heuristic)

    def _get_channel_data(self, data, antennas):
        phases = {}
        for antenna in antennas:
            identifier = antenna.name if antenna.name else antenna.id
            phases[identifier] = numpy.angle(data[antenna.id,:])
        return phases
