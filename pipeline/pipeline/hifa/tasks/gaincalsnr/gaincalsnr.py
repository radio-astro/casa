from __future__ import absolute_import

import string
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from pipeline.hifa.heuristics import snr as snr_heuristics

LOG = infrastructure.get_logger(__name__)

class GaincalSnrInputs(basetask.StandardInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None,
         intent=None, spw=None, phasesnr=None, bwedgefrac=None,
         hm_nantennas=None, maxfracflagged=None): 

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def field(self):
        # If field was explicitly set, return that value
        if self._field is not None:
            return self._field

        # If invoked with multiple ms's, return a list of fields
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('field')

        # Otherwise return each field name in the current ms that has been
        # observed with the desired intent
        fields = self.ms.get_fields(intent=self.intent)
        fieldids = set(sorted([f.id for f in fields])) 
        fieldnames = []
        for fieldid in fieldids: 
            field = self.ms.get_fields(field_id=fieldid)
            fieldnames.append(field[0].name)
        field_names = set(fieldnames)
        return ','.join(field_names)

    @field.setter
    def field(self, value):
        self._field = value

    @property
    def intent(self):
        if self._intent is not None:
            return self._intent.replace('*', '')
        return None

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'PHASE'
        self._intent = string.replace(value, '*', '')

    @property
    def spw(self):
        # If spw was explicitly set, return that value
        if self._spw is not None:
            return self._spw

        # If invoked with multiple mses, return a list of spws
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')

        # Get the science spw ids
        sci_spws = set([spw.id for spw in \
            self.ms.get_spectral_windows(science_windows_only=True)])

        # Get the bandpass spw ids
        bandpass_spws = []
        for scan in self.ms.get_scans(scan_intent=self.intent):
            bandpass_spws.extend(spw.id for spw in scan.spws)
        bandpass_spws = set (bandpass_spws).intersection(sci_spws)

        # Get science target spw ids
        target_spws = []
        for scan in self.ms.get_scans(scan_intent='TARGET'):
            target_spws.extend([spw.id for spw in scan.spws])
        target_spws = set(target_spws).intersection(sci_spws)
        
        # Compute the intersection of the bandpass and science target spw
        # ids
        spws = list(bandpass_spws.intersection(target_spws))
        spws = [str(spw) for spw in sorted(spws)]
        return ','.join(spws)

    @spw.setter
    def spw(self, value):
        self._spw = value

    @property
    def phasesnr(self):
        if self._phasesnr is not None:
            return self._phasesnr
        return None

    @phasesnr.setter
    def phasesnr(self, value):
        if value is None:
            value = 25.0
        self._phasesnr = value

    @property
    def bwedgefrac(self):
        if self._bwedgefrac is not None:
            return self._bwedgefrac
        return None

    @bwedgefrac.setter
    def bwedgefrac(self, value):
        if value is None:
            value = 0.03125
        self._bwedgefrac = value

    @property
    def hm_nantennas (self):
        if self._hm_nantennas is not None:
            return self._hm_nantennas
        return None

    # Options are 'all' and 'unflagged'
    @hm_nantennas.setter
    def hm_nantennas (self, value):
        if value is None:
            value = 'unflagged'
        self._hm_nantennas = value

    @property
    def maxfracflagged(self):
        if self._maxfracflagged is not None:
            return self._maxfracflagged
        return None

    @maxfracflagged.setter
    def maxfracflagged(self, value):
        if value is None:
            value = 0.90
        self._maxfracflagged = value


class GaincalSnr(basetask.StandardTaskTemplate):
    Inputs = GaincalSnrInputs

    def prepare(self, **parameters):

        # Simplify the inputs
        inputs = self.inputs

        # Turn the CASA field name and spw id lists into Python lists
        fieldlist = inputs.field.split(',')
        spwlist = [int(spw) for spw in inputs.spw.split(',')]

        # Log the data selection choices
        LOG.info('Estimating gaincal solution SNR for MS %s' % \
            inputs.ms.basename)
        LOG.info('    Setting gaincal intent to %s ' % inputs.intent)
        LOG.info('    Selecting gaincal fields %s ' % fieldlist)
        LOG.info('    Selecting gaincal spws %s ' % spwlist)
        LOG.info('    Setting requested gaincal snr to %0.1f ' % \
            (inputs.phasesnr))
        if len(fieldlist) <= 0 or len(spwlist) <= 0:
            LOG.info('    No gaincal data')
            return GaincalSnrResults(vis=inputs.vis)

        # Compute the bandpass solint parameters and return a solution
        # dictionary
        snr_dict = snr_heuristics.estimate_gaincalsnr(inputs.ms, fieldlist,
	    inputs.intent, spwlist, inputs.hm_nantennas, inputs.maxfracflagged,
	    inputs.bwedgefrac)

        if not snr_dict:
            LOG.info('No SNR dictionary')
            return GaincalSnrResults(vis=inputs.vis)

        # Construct the results object
        result = self._get_results (inputs.vis, spwlist, snr_dict)

        # Return the results
        return result

    def analyse(self, result):
        return result

    # Get final results from the spw dictionary
    def _get_results (self, vis, spwidlist, snr_dict):

        # Initialize result structure.
        result = GaincalSnrResults(vis=vis, spwids=spwidlist)

        # Initialize the lists
        scantimes = []
        inttimes = []
        sensitivities = []
        snrs = []

        # Loop over the spws. Values for spws with
        # not dictionary entries are set to None
        for spwid in spwidlist:

            if not snr_dict.has_key(spwid):

                scantimes.append(None)
                inttimes.append(None)
                sensitivities.append(None)
                snrs.append(None)

            else:

                scantimes.append(snr_dict[spwid]['scantime_minutes'])
                inttimes.append(snr_dict[spwid]['inttime_minutes'])
                sensitivities.append(snr_dict[spwid]['sensitivity_per_scan_mJy'])
                snrs.append(snr_dict[spwid]['snr_per_scan'])

        # Populate the result.
        result.scantimes = scantimes
        result.inttimes = inttimes
        result.sensitivities = sensitivities
        result.snrs = snrs

        return  result

            
# The results class

class GaincalSnrResults(basetask.Results):
    def __init__(self, vis=None, spwids=[], scantimes=[], inttimes=[],
        sensitivities=[], snrs=[]):

        """
        Initialise the results object.
        """
        super(GaincalSnrResults, self).__init__()

        self.vis=vis

        # Spw list
        self.spwids = spwids

        self.scantimes = scantimes
        self.inttimes = inttimes
        self.sensitivities = sensitivities
        self.snrs = snrs


    def __repr__(self):
        if self.vis is None or not self.spwids:
            return('GaincalSnrResults:\n'
            '\tNo gaincal SNRs computed')
        else:
            line = 'GaincalSnrResults:\nvis %s\n' % (self.vis)
            line = line + 'Gaincal SNRs\n'
            for i in range(len(self.spwids)):
                line = line + \
                    "    spwid %2d sensitivity: %10.3f SNR: %8.3f\n" % \
                    (self.spwids[i], self.sensitivities[i], self.snrs[i])
            return line
