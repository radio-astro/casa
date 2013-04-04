import numpy as np
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools

class BPOrder:
    def __init__(self):
        self._amporder = None
        self._phaseorder = None
        self._jobs = []

    @staticmethod
    def getbandpassview(table, spw, msobject, dryrun):
        """Method to read channel calibration table and return it as a Python
        array.

        Keyword arguments:
        table  -- The name of the calibration table
        spw    -- The SpW of interest.
        msobject -- MeasurementSet domain object from which the calibration
                    was calculated.
        dryrun -- True if want to return dummy arrays.

        Returns:
        data - 'view' of calibration gains in numpy complex array[antenna,channel]
        flag - flags associated with data array bool[antenna,channel]
        """
        print 'in getbandpassview'

        spw = int(spw)

        # The number of channels in the SpW (needed to correct
        # for the fact that the calibration table arrays all have
        # the number of channels of the largest spw)
      
        nchannels = msobject.get_spectral_window(spw).num_channels
        npol = 2
        maxantennaid = max([antenna.id for antenna in msobject.antennas])

        if dryrun:
            print maxantennaid, nchannels
            data = np.ones([maxantennaid+1, nchannels], np.complex)
            flag = np.zeros([maxantennaid+1, nchannels], np.bool),
            return data, flag

        with casatools.TableReader('%s/CAL_DESC' % table) as htb:

            # open the CAL_DESC sub-table and read the column that maps
            # CAL_DESC_ID to SPECTRAL_WINDOW_ID

            caldesc_2_spw = htb.getcol('SPECTRAL_WINDOW_ID')[0]

            caldesc_id = np.arange(len(caldesc_2_spw))[caldesc_2_spw==spw]
            if len(caldesc_id) > 0:
                caldesc_id = caldesc_id[0]
            else:
                raise Exception, 'SpW %s not found' % spw

        with casatools.TableReader(table) as htb:

            # For channel by channel calibrations read the results
            # themselves. Limiting the length of the gain and
            # cal_flag arrays is to correct for the fact that the
            # calibration table arrays all have the number of channels
            # of the largest spw. This should change in the future.

            taql = 'CAL_DESC_ID==%s' % (caldesc_id) 
            subtable = htb.query(query=taql)
            antenna1 = subtable.getcol('ANTENNA1')
            gain = subtable.getcol('GAIN')[:,:nchannels,:]
            cal_flag = subtable.getcol('FLAG')[:,:nchannels,:]
	    subtable.close()

            # average the polarization results

            data = np.zeros([maxantennaid+1, nchannels], np.complex)
            flag = np.zeros([maxantennaid+1, nchannels], np.bool)

            for i in range(np.shape(gain)[2]):
                antenna = antenna1[i]
                if antenna > maxantennaid:
                    print \
                     '    Antenna1 %s from file %s is outside antenna range' % (
                     antenna, table)
                    continue

                for p in range(npol):
                    data[antenna,:] += gain[p,:,i]
                    flag[antenna,:] = np.logical_or(flag[antenna,:],
                     cal_flag[p,:,i])
                data[antenna,:] /= float(npol)

        return data, flag

    @staticmethod
    def polyOrder(data):
        """Method to estimate the order of polynomial required to fit the
        give data sequence. Does this by:
           Fourier transform the sequence
           Find highest frequency in the amplitude spectrum > 10 * mean
           Order = max(2, 3 * frequency)
        
        Keyword arguments:
        data -- The input data sequence.  
        
        Returns: order - order of polynomial required.
        """
        transform = np.abs(np.fft.fft(data)) 
        ntransform = np.array(transform)
        ntransform /= np.mean(ntransform) 
        
        if max(ntransform) < 3.0:
            order = 1
        elif max(ntransform) < 10.0:
            order = 2
        else:
            for i in range(int(len(ntransform)/2.0), -1, -1):
                if ntransform[i] > 10.0:
                    break 
            order = max(2, 3*i)
        return order


    def getorders(self, **kwargs):
        print kwargs

        dryrun = kwargs.pop('dryrun')

        # if necessary calculate a channel bandpass result

        if self._amporder is None:
            channelbpr = 'channel.tbl'
            ms = kwargs.pop('ms')
            print 'ms', ms
            print 'name', ms.name
            kwargs['vis'] = ms.name
            self._jobs.extend([
                casa_tasks.comment('#'),
                casa_tasks.comment('# heuristics.bporder calculating a channel bandpass'),
                casa_tasks.bandpass(caltable=channelbpr,
                                    bandtype='B',
                                    **kwargs)])

            for job in self._jobs:
                job.execute(dry_run=dryrun)

        # analyse the power spectrum to derive a suitable degamp    
        spw = int(kwargs['spw'])
        
        data,flag = self.getbandpassview(table=channelbpr,
         spw=spw,
         msobject=ms,
         dryrun=dryrun)

        # estimate the polynomial order for amp and phase in each antenna
    
        amporder = {}
        phaseorder = {}
        for antenna in ms.antennas:
            amporder[antenna.id] = self.polyOrder(np.abs(data[antenna.id,:]))
            phaseorder[antenna.id] = self.polyOrder(np.angle(data[antenna.id,:]))

            self._jobs.extend([
                casa_tasks.comment('# antenna  %s(%s)' % (antenna.name, antenna.id)),
                casa_tasks.comment('# amp order  phase order'),
                casa_tasks.comment('#    %s         %s' % (amporder[antenna.id],
                                                           phaseorder[antenna.id]))])
        self._amporder = max(amporder.values())
        self._phaseorder = max(phaseorder.values())
        self._jobs.extend([
            casa_tasks.comment('#'),
            casa_tasks.comment('# heuristic orders: '
                               'amp %s  phase %s' % (self._amporder,
                                                     self._phaseorder))])

    def degamp(self, **kwargs):
        self.getorders(**kwargs)
        return self._amporder, list(self._jobs)


    def degphase(self, **kwargs):
        self.getorders(**kwargs)
        return self._phaseorder, list(self._jobs)
