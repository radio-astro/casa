from __future__ import absolute_import

import string
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from pipeline.hifa.heuristics import snr as snr_heuristics

LOG = infrastructure.get_logger(__name__)

class BpSolintInputs(basetask.StandardInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None,
         intent=None, spw=None, phaseupsnr=None, minphaseupints=None,
         bpsnr=None, minbpnchan=None, hm_nantennas=None, maxfracflagged=None): 

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
            value = 'BANDPASS'
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
    def phaseupsnr(self):
        if self._phaseupsnr is not None:
            return self._phaseupsnr
        return None

    @phaseupsnr.setter
    def phaseupsnr(self, value):
        if value is None:
            value = 20.0
        self._phaseupsnr = value

    @property
    def minphaseupints(self):
        if self._minphaseupints is not None:
            return self._minphaseupints
        return None

    @minphaseupints.setter
    def minphaseupints(self, value):
        if value is None:
            value = 2
        self._minphaseupints = value

    @property
    def bpsnr(self):
        if self._bpsnr is not None:
            return self._bpsnr
        return None

    @bpsnr.setter
    def bpsnr(self, value):
        if value is None:
            value = 50.0
        self._bpsnr = value

    @property
    def minbpnchan(self):
        if self._minbpnchan is not None:
            return self._minbpnchan
        return None

    @minbpnchan.setter
    def minbpnchan(self, value):
        if value is None:
            value = 8
        self._minbpnchan = value

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


class BpSolint(basetask.StandardTaskTemplate):
    Inputs = BpSolintInputs

    def prepare(self, **parameters):

        # Simplify the inputs
        inputs = self.inputs

        # Turn the CASA field name and spw id lists into Python lists
        fieldlist = inputs.field.split(',')
        spwlist = [int(spw) for spw in inputs.spw.split(',')]

        # Log the data selection choices
        LOG.info('Estimating bandpass solution intervals for MS %s' % \
            inputs.ms.basename)
        LOG.info('    Setting bandpass intent to %s ' % inputs.intent)
        LOG.info('    Selecting bandpass fields %s ' % fieldlist)
        LOG.info('    Selecting bandpass spws %s ' % spwlist)
        LOG.info('    Setting requested phaseup snr to %0.1f ' % \
            (inputs.phaseupsnr))
        LOG.info('    Setting requested bandpass snr to %0.1f ' % \
            (inputs.bpsnr))
        if len(fieldlist) <= 0 or len(spwlist) <= 0:
            LOG.info('    No bandpass data')
            return BpSolintResults(vis=inputs.vis)


        # Compute the bandpass solint parameters and return a solution
        # dictionary
        solint_dict = snr_heuristics.estimate_bpsolint(inputs.ms, fieldlist,
	    inputs.intent, spwlist, inputs.hm_nantennas, inputs.maxfracflagged,
	    inputs.phaseupsnr, inputs.minphaseupints, inputs.bpsnr,
	    inputs.minbpnchan)

        if not solint_dict:
            LOG.info('No solution internval dictionary')
            return BpSolintResults(vis=inputs.vis)

        # Construct the results object
        result = self._get_results (inputs.vis, spwlist, solint_dict)

        # Return the results
        return result

    def analyse(self, result):
        return result

    # Get final results from the spw dictionary
    def _get_results (self, vis, spwidlist, solint_dict):

        # Initialize result structure.
        result = BpSolintResults(vis=vis, spwids=spwidlist)

        # Initialize the lists
        phsolints = []
        phintsolints = []
        nphsolutions = []
        phsensitivities = []
        phintsnrs = []

        bpsolints = []
        bpchansolints = []
        nbpsolutions = []
        bpsensitivities = []
        bpchansnrs = []

        # Loop over the spws. Values for spws with
        # not dictionary entries are set to None
        for spwid in spwidlist:

            if not solint_dict.has_key(spwid):

                phsolints.append(None)
                phintsolints.append(None)
                nphsolutions.append(None)
                phsensitivities.append(None)
                phintsnrs.append(None)

                bpsolints.append(None)
                bpchansolints.append(None)
                nbpsolutions.append(None)
                bpsensitivities.append(None)
                bpchansnrs.append(None)

            else:

                phsolints.append(solint_dict[spwid]['phaseup_solint'])
                phintsolints.append(solint_dict[spwid]['nint_phaseup_solint'])
                nphsolutions.append(solint_dict[spwid]['nphaseup_solutions'])
                phsensitivities.append( \
                    '%fmJy' % solint_dict[spwid]['sensitivity_per_integration_mJy'])
                phintsnrs.append(solint_dict[spwid]['snr_per_integration'])

                bpsolints.append(solint_dict[spwid]['bpsolint'])
                bpchansolints.append(solint_dict[spwid]['nchan_bpsolint'])
                nbpsolutions.append(solint_dict[spwid]['nbandpass_solutions'])
                bpsensitivities.append( \
                    '%fmJy' % solint_dict[spwid]['sensitivity_per_channel_mJy'])
                bpchansnrs.append(solint_dict[spwid]['snr_per_channel'])

        # Populate the result.
        result.phsolints = phsolints
        result.phintsolints = phintsolints
        result.nphsolutions = nphsolutions
        result.phsensitivities = phsensitivities
        result.phintsnrs = phintsnrs

        result.bpsolints = bpsolints
        result.bpchansolints = bpchansolints
        result.nbpsolutions = nbpsolutions
        result.bpchansensitivities = bpsensitivities
        result.bpchansnrs = bpchansnrs

        return  result

            
# The results class

class BpSolintResults(basetask.Results):
    def __init__(self, vis=None, spwids=[],
        phsolints=[], phintsolints=[], nphsolutions=[],
        phsensitivities=[], phintsnrs=[],
        bpsolints=[], bpchansolints=[], nbpsolutions=[],
        bpsensitivities=[], bpchansnrs=[]):

        """
        Initialise the results object.
        """
        super(BpSolintResults, self).__init__()

        self.vis=vis

        # Spw list
        self.spwids = spwids

        # Phaseup solutions
        self.phsolints = phsolints
        self.phintsolints = phintsolints
        self.nphsolutions = nphsolutions
        self.phsensitivities = phsensitivities
        self.phintsnrs = phintsnrs

        # Bandpass solutions
        self.bpsolints = bpsolints
        self.bpchansolints = bpchansolints
        self.nbpsolutions = nbpsolutions
        self.bpchansensitivities = bpsensitivities
        self.bpchansnrs = bpchansnrs

#    def merge_with_context(self, context):
#
#        if self.vis is None:
#            LOG.error ( ' No results to merge ')
#            return
#
#        if not self.phaseup_result.final:
#            LOG.error ( ' No results to merge ')
#            return
#
#        # Merge the spw phaseup offset table
#        self.phaseup_result.merge_with_context(context)
#
#        # Merge the phaseup spwmap
#        ms = context.observing_run.get_ms( name = self.vis)
#        if ms:
#            ms.phaseup_spwmap = self.phaseup_spwmap

    def __repr__(self):
        if self.vis is None or not self.spwids:
            return('BpSolintResults:\n'
            '\tNo bandpass solution intervals computed')
        else:
            line = 'BpSolintResults:\nvis %s\n' % (self.vis)
            line = line + 'Phaseup solution time intervals\n'
            for i in range(len(self.spwids)):
                line = line + \
                    "    spwid %2d solint '%s' intsolint %2d sensitivity %s intsnr %0.1f\n" % \
                    (self.spwids[i], self.phsolints[i], self.phintsolints[i], \
                    self.phsensitivities[i], self.phintsnrs[i])
            line = line + 'Bandpass frequency solution intervals\n'
            for i in range(len(self.spwids)):
                line = line + \
                    "    spwid %2d solint '%s' channels %2d sensitivity %s chansnr %0.1f\n" % \
                    (self.spwids[i], self.bpsolints[i], self.bpchansolints[i], \
                    self.bpchansensitivities[i], self.bpchansnrs[i])
            return line
