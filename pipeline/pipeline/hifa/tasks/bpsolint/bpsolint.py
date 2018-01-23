from __future__ import absolute_import

import string
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp

from pipeline.hifa.heuristics import snr as snr_heuristics

LOG = infrastructure.get_logger(__name__)

class BpSolintInputs(vdp.StandardInputs):

    @vdp.VisDependentProperty
    def field(self):
        # Return field names in the current ms that have been
        # observed with the desired intent
        fields = self.ms.get_fields(intent=self.intent)
        fieldids = set(sorted([f.id for f in fields])) 
        fieldnames = []
        for fieldid in fieldids: 
            field = self.ms.get_fields(field_id=fieldid)
            fieldnames.append(field[0].name)
        field_names = set(fieldnames)
        return ','.join(field_names)

    intent = vdp.VisDependentProperty(default = 'BANDPASS')

    @vdp.VisDependentProperty
    def spw(self):

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

    phaseupsnr = vdp.VisDependentProperty(default = 20.0)
    minphaseupints = vdp.VisDependentProperty(default = 2)
    evenbpints = vdp.VisDependentProperty(default = False)
    bpsnr = vdp.VisDependentProperty(default = 50.0)
    minbpnchan = vdp.VisDependentProperty(default = 8)
    hm_nantennas = vdp.VisDependentProperty(default = 'unflagged')
    maxfracflagged = vdp.VisDependentProperty(default = 0.90)

    def __init__(self, context, output_dir=None, vis=None, field=None,
         intent=None, spw=None, phaseupsnr=None, minphaseupints=None,
         evenbpints=None, bpsnr=None, minbpnchan=None, hm_nantennas=None, maxfracflagged=None): 

         super(BpSolintInputs, self).__init__()

         self.context = context
         self.vis = vis
         self.output_dir = output_dir

         self.field = field
         self.intent = intent
         self.spw = spw

         self.phaseupsnr = phaseupsnr
         self.minphaseupints = minphaseupints
         self.evenbpints = evenbpints
         self.bpsnr = bpsnr
         self.minbpnchan = minbpnchan 
         self.hm_natennas = hm_nantennas
         self.maxfracflagged = maxfracflagged

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
	    inputs.minbpnchan, evenbpsolints=inputs.evenbpints)

        if not solint_dict:
            LOG.info('No solution interval dictionary')
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
