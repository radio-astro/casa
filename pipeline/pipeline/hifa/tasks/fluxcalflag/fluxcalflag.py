from __future__ import absolute_import
import string
import types
import sys
import os
import numpy as np

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure as infrastructure
from pipeline.infrastructure import casa_tasks
from .solsyslinesdict import SolarSystemLineList

__all__ = [
    'FluxcalFlag',
    'FluxcalFlagInputs',
    'FluxcalFlagResults'
]

LOG = infrastructure.get_logger(__name__)

class FluxcalFlagInputs(vdp.StandardInputs):
    """
    Initialise the flux calibration flagging task inputs object.
    """

    @vdp.VisDependentProperty
    def field(self):
        # Return each field in the current ms that has been observed
        # with the desired intent
        fields = self.ms.get_fields(intent=self.intent)
        field_names = set([f.name for f in fields])
        return ','.join(field_names)

    intent = vdp.VisDependentProperty(default='AMPLITUDE')
    threshold = vdp.VisDependentProperty(default=0.75)
    appendlines = vdp.VisDependentProperty(default=False)

    @vdp.VisDependentProperty
    def linesfile(self):
        vis_root = os.path.splitext(self.vis)[0]
        return vis_root + '_.txt'

    applyflags = vdp.VisDependentProperty(default=True)

    def __init__(self, context, vis=None, output_dir=None,
                 field=None, spw=None, intent=None,
                 threshold=None, appendlines=None, linesfile=None,
                 applyflags=None
                 ):
        super(FluxcalFlagInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir
        self.field = field
        self.spw = spw
        self.intent = intent
        self.threshold = threshold
        self.appendlines = appendlines
        self.linesfile = linesfile
        self.applyflags = applyflags

class FluxcalFlagResults(basetask.Results):
    def __init__(self, vis, fluxcal_linelist=[], fluxcal_flagcmds=[],
        refspwmap=[-1], science_spw_ids=[]):
        """
        Initialise the flux calibration flagging task results object.
        """
        super(FluxcalFlagResults, self).__init__()
        self._vis=vis
        self._fluxcal_linelist = fluxcal_linelist
        self._fluxcal_flagcmds = fluxcal_flagcmds
        self._refspwmap = refspwmap
        self._science_spw_ids = science_spw_ids

    def merge_with_context(self, context):

        if self._vis is None: 
            LOG.error ( ' No results to merge ')
            return

        # Store the refspwmap and flagging commands
        # in the context. Leave out the line list for now
        ms = context.observing_run.get_ms( name = self._vis)
        if ms:
            #ms.fluxcal_linelist = self._fluxcal_linelist
            ms.flagcmds.extend(self._fluxcal_flagcmds)
            ms.reference_spwmap = self._refspwmap

    def __repr__(self):
        if self._vis is None or not self._fluxcal_linelist:
            return('FluxcalFlagResults:\n'
            '\tNo lines detected in flux calibrators')
        else:
            linelist = 'FluxcalFlagResults:\n'
            for line in self._fluxcal_linelist:
                linelist = linelist + \
                    '\tfield=%s line=%s spw=%d:%d~%d nchan=%d\n' % \
                    (line.fieldname, line.species, line.spwid, \
                     line.chanrange[0], line.chanrange[1], line.nchan) 
            return linelist


class FluxcalFlag(basetask.StandardTaskTemplate):
    Inputs = FluxcalFlagInputs

    def prepare(self):
        inputs = self.inputs

        # TODO there's so much mixed tab/tabspace formatting in this file I
        # can't read the identation. Once fixed, summaries should be moved
        # closer to the creation of the results
        summaries = []

        # Return if the MS has no data with the required intent.
        flux_fields = inputs.ms.get_fields(inputs.field, intent=inputs.intent)
        if not flux_fields:
            LOG.warning('Field(s) \'%s\' in %s have no data with intent %s' % 
                (inputs.field, inputs.ms.basename, inputs.intent))
            result = FluxcalFlagResults(inputs.vis)
            result.summaries = summaries
            return result

        # Get the science spws. 
        science_spws = inputs.ms.get_spectral_windows(task_arg=inputs.spw,
            science_windows_only=True)
        if not science_spws:
            LOG.warning('No science spw(s) specified for %s' %
                (inputs.ms.basename)) 
            result = FluxcalFlagResults(inputs.vis)
            result.summaries = summaries
            return result
        science_spw_ids = [spw.id for spw in science_spws]
        
        # Get all the spws. These will be used to construct refspwmap
        all_spws = inputs.ms.get_spectral_windows(task_arg=inputs.spw,
            science_windows_only=False)

        # Read in user defined lines and append them to the builtin
        # lines dictionary.
        UserSolarSystemLineList = SolarSystemLineList.copy()
        if inputs.appendlines:
            LOG.info('Appending user line list %s to builtin dictionary' % \
                inputs.linesfile)
            self._append_linesfile (UserSolarSystemLineList, inputs.linesfile)

        # Get the pipeline to CASA intents mapping to be used in the CASA
        # flagging commands. Should be handled transparently by framework
        # as is the case for the calibration software.
        #    Intent mapping problem ?
        amp_obsmodes = []
        for state in inputs.ms.states:
            amp_obsmodes.extend(state.get_obs_mode_for_intent(inputs.intent))
        amp_obsmodes = list(set(amp_obsmodes))

        # Loop over the list of flux calibrators and create a list of
        # lines ordered by field and spw
        fluxcal_lines = []
        for field in flux_fields:

            # Skip if field not in solar system object line list
            if field.name not in UserSolarSystemLineList:
                continue
            LOG.info('Searching field %s for spectral lines' % (field.name)) 

            # Loop over the science spectral windows for that field
            # NOTE: Use in field.valid_spws() in future
            for spw in science_spws:

                # Loop over the target solar system object lines
                for molecule in UserSolarSystemLineList[field.name]:
                    species = molecule[0]; lines = molecule[1]

                    # Loop over the lines for each molecule
                    for line in lines:

                        # Return channel overlap for each line
                        chanrange = self._get_chanrange (vis=inputs.ms.name,
                            fieldid=field.id, spwid=spw.id,
                            minfreq=1.0e9*line[0],
                            maxfreq=1.0e9*line[1],
                            refframe='GEO')
                        if chanrange == (None, None):
                            continue
                        LOG.info('    Found line %s %.3f~%.3fGHz in spw %d:%d~%d' %
                            (species, line[0], line[1], spw.id, chanrange[0], chanrange[1]))

                        # Create the line object and add it into a list of
                        # line objects.
                        fluxcal_line = MolecularLine(fieldname=field.name,
                            species=species, freqrange=(line[0],line[1]),
                            spwid=spw.id, chanrange=chanrange,
                            nchan=spw.num_channels)
                        fluxcal_lines.append(fluxcal_line)

        # Generate the channel flagging statistics per field and spw.
        flagstats = self._newflagstats (fluxcal_lines)

        # Compute the reference spw map
        flagall, refspwmap = self._refspwmap(all_spws, science_spws, flagstats,
            inputs.threshold)
        LOG.info('Spectral window map for flux scaling %s' % refspwmap)

        # Generate a list of flagging commands
        flagcmds = self._flagcmds (fluxcal_lines, flagstats, amp_obsmodes,
            inputs.threshold, flagall)
        LOG.info('Flagging commands generated')
        if not flagcmds:
            LOG.info('    None')
        else:
            for cmd in flagcmds:
                LOG.info('    %s' % cmd)
            if inputs.applyflags:
                LOG.info('Applying flags')

                # Add the summary commands to the flagging commands
                allflagcmds = ["mode='summary' name='before'"]
                allflagcmds.extend(flagcmds)
                allflagcmds.append("mode='summary' name='after'")

                # Flag the data
                task = casa_tasks.flagdata(vis=inputs.vis, mode='list',
                    action='apply', inpfile=allflagcmds, savepars=False,
                    flagbackup=False)
                flagresults = self._executor.execute(task)

                stats_before = {}; stats_after = {}
                for summary in flagresults.keys():
                    if flagresults[summary]['name'] == 'before':
                        stats_before = flagresults[summary]
                    elif flagresults[summary]['name'] == 'after':
                        stats_after = flagresults[summary]
                if not stats_before or not stats_after:
                    summaries = []
                else:
                    summaries = [stats_before, stats_after]

        result = FluxcalFlagResults(inputs.vis,
            fluxcal_linelist=fluxcal_lines, fluxcal_flagcmds=flagcmds,
                refspwmap=refspwmap, science_spw_ids=science_spw_ids) 
        result.summaries = summaries

        return result

    def analyse(self, result):
        return result

    # Read in a file containing lines regions and append it to 
    # the builtin dictionary.
    #    Blank lines and comments beginning with # are skipped
    #    The data is contained in 4 whitespace delimited fields
    #    containing the solar system object field name, e.g. 'Callisto'
    #    the molecular species name, e.g. '13CO' and the starting
    #    and ending frequency in GHz.
    #
    def _append_linesfile (self, linedict, linefile):
        with open (linefile, 'r') as datafile:
            for line in datafile:

                # Parse the line
                if not line.strip():
                    continue
                if line.startswith('#'):
                    continue
                fields = line.split()
                if len(fields) < 4:
                    continue
                source = fields[0]; species = fields[1]
                region = (float(fields[2]),float(fields[3]))

                # New source
                if source not in linedict:
                    linedict[source] = [(species, [region])]
                # Existing source
                else:
                    # New melcular species
                    #    If not append line to species list
                    #    If not create new species list
                    newspecies = True
                    for item in linedict[source]:
                        if item[0] == species: 
                            item[1].append(region)
                            newspecies = False
                    if newspecies:
                        linedict[source].append((species, [region]))


    # Generate channel flagging statistics per field and spw allowing
    # for line region overlaps. For each field, spwid combination create
    # a mask array of zeros equal to the size of the spw. Populate the
    # mask corresponding to line regions with 1's. When appropriate
    # compute the fraction of the array containing 1's by summing the
    # array and dividing by the total number of channels.
    def _newflagstats (self, fluxcal_lines):

        flagstats = {}; maskarrays = {}
        prev_fieldname = None; prev_spwid = None
        for line in fluxcal_lines:
            if line.fieldname != prev_fieldname:
                flagstats[line.fieldname] = {}
                maskarrays[line.fieldname] = {}
                spwmask = np.zeros (line.nchan, dtype=np.int32)
                spwmask[line.chanrange[0]:line.chanrange[1]:1] = 1 
                flagstats[line.fieldname][line.spwid] = \
                    np.sum(spwmask) / float(line.nchan) 
                maskarrays[line.fieldname][line.spwid] = spwmask
            elif line.spwid != prev_spwid:
                spwmask = np.zeros (line.nchan, dtype=np.int32)
                spwmask[line.chanrange[0]:line.chanrange[1]:1] = 1 
                flagstats[line.fieldname][line.spwid] = \
                    np.sum(spwmask) / float(line.nchan) 
                maskarrays[line.fieldname][line.spwid] = spwmask
            else:
                spwmask = maskarrays[line.fieldname][line.spwid]
                spwmask[line.chanrange[0]:line.chanrange[1]:1] = 1 
                flagstats[line.fieldname][line.spwid] = \
                    np.sum(spwmask) / float(line.nchan) 

            prev_fieldname = line.fieldname
            prev_spwid = line.spwid

        # Delete the mask arrays
        for key1 in maskarrays.keys():
            for key2 in maskarrays[key1].keys():
                del(maskarrays[key1][key2])
            maskarrays[key1].clear()

        # return flagging statistic
        return flagstats
      

    # Compute the reference spwmap
    def _refspwmap (self, allspws, scispws, flagstats, threshold):

        # Determine which science windows should be flagged
        # entirely and which not. Assume that the same spw in
        # different fields will have the same flagging statistics

        max_spwid = 0; flaggedspws = {}; unflaggedspws = {}
        for spw in scispws:

            # Find the maximum science spw id
            if spw.id > max_spwid:
                max_spwid = spw.id

            # Field loop
            flagged = False
            for fkey, fvalue in flagstats.iteritems():
                # Spw loop
                for skey, svalue in fvalue.iteritems():
                    if skey == spw.id:
                        flagged = True
                        if svalue > threshold:
                            flaggedspws[skey] = spw
                        else: 
                            unflaggedspws[skey] = spw
            if not flagged:
                unflaggedspws[spw.id] = spw

        # None need be completely flagged. 
        if not flaggedspws:
            return True, [-1]

        # Initialize the spwmap. All spw ids up to the maximum
        # science spw id must be defined.
        refspwmap = []
        for i in range(max_spwid + 1):
            refspwmap.append(i)

        # All Science windows would be completely flagged
        if not unflaggedspws:
            return False, [-1]

        # For spectral windows which are completely
        # flagged find the closest unflagged window.
        for fkey, fvalue in flaggedspws.iteritems():
            maxdiff = sys.float_info.max
            ctrfreq = fvalue.centre_frequency.value
            closest_spwid = None
            for _, ufvalue in unflaggedspws.iteritems():
                uctrfreq = ufvalue.centre_frequency.value
                diff = abs(ctrfreq - uctrfreq)
                if (diff < maxdiff):
                    closest_spwid = ufvalue.id
                    maxdiff = diff
            if closest_spwid is not None:
                refspwmap[fkey] = closest_spwid
        
        #return True, refspwmap
        return False, refspwmap


    # Generate flagging commands one per field and spw
    def _flagcmds (self, fluxcal_lines, flagstats, obsmodes, threshold,
        flagall):

        # Generate the flagging commands
        flagcmds = []; flagcmd = ''
        prev_fieldname = None; prev_spwid = None
        for line in fluxcal_lines:

            # Field or spw has changed so start a new command
            if line.fieldname != prev_fieldname or line.spwid != prev_spwid:

                # Write out the previous command
                # Note: the first single quote closes the value of the spw field
                if flagcmd != '':
                    flagcmds.append(flagcmd +
                        "' reason='Flux_calibrator_atmospheric_line'")

                # Initialize the command
                flagcmd = ''
                if flagall and flagstats[line.fieldname][line.spwid] > threshold:
                    pass
                else:
                    # Note: the value of spw field is not closed by single quote, 
                    # for in case additional channel ranges are added.
                    flagcmd = "mode='manual' field='%s' intent='%s' spw='%d:%d~%d" % \
                        (line.fieldname, ','.join(obsmodes), line.spwid,
                        line.chanrange[0], line.chanrange[1])

            # Append a new channel range if appropriate.
            elif flagall and flagstats[line.fieldname][line.spwid] > threshold:
                pass
            else:
                flagcmd = flagcmd + ';' + '%d~%d' % \
                    (line.chanrange[0], line.chanrange[1])
            prev_fieldname = line.fieldname
            prev_spwid = line.spwid

        # Finish off last command
        if flagcmd != '':
            # Note: the first single quote closes the value of the spw field
            flagcmds.append(flagcmd + "' reason='Flux_calibrator_atmospheric_line'")
                    
        return flagcmds

    def _get_chanrange (self, vis=None, fieldid=0, spwid=None, minfreq=None,
        maxfreq=None, refframe='TOPO'):
    
        """
        Returns a tuple containing the two channels in an spw corresponding
           to the minimum and maximum frequency in the given ref frame
    
            vis - MS name
        fieldid - field id of the observed field for reference frame \
                  calculations
          spwid - id of the spw 
        minfreq - minimum freq in Hz
        maxfreq - maximum freq in Hz
       refframe - frequency reference frame
        """
    
        if (vis==None or spwid==None or minfreq==None or maxfreq==None):
            raise Exception ('_get_chanrange : Undefined values for vis, \
            spwid, minfreq, and maxfreq')
    
        rval = (None, None)
        if(minfreq > maxfreq):
            return rval
    
        iminfreq = -1
        imaxfreq = -1
    
        # Get frequencies
        mse = casatools.ms
        mse.open(vis)
        a = mse.cvelfreqs(fieldids=[fieldid], spwids=[spwid], mode='frequency',
            outframe=refframe)
        mse.close()
    
        # Initialize
        ichanmax = len(a)-1
        ascending = True
        lowedge = a[0]
        ilowedge = 0
        upedge = a[ichanmax]
        iupedge = ichanmax
        if(ichanmax > 0 and (a[ichanmax] < a[0])): # frequencies are descending
            ascending = False
            lowedge = a[ichanmax]
            upedge = a[0]
            ilowedge = ichanmax
            iupedge = 0
    
        if(minfreq < lowedge):
            if(maxfreq > lowedge):
                if(maxfreq > upedge):
                    iminfreq = ilowedge
                    imaxfreq = iupedge
                else:
                    iminfreq = ilowedge
                    # use imaxfreq from below search
                    imaxfreq = -2
            #else:
                # both imaxfreq and iminfreq are -1
        else:
            if(minfreq < upedge):
                if(maxfreq >= upedge):
                    # take iminfreq from below search
                    iminfreq = -2
                    imaxfreq = iupedge
                else:
                    #take both iminfreq and imaxfreq from above search
                    iminfreq = -2
                    imaxfreq = -2
            #else:
                # both imaxfreq and iminfreq are -1
    
        if ascending:
            if iminfreq == -2:
                for i in xrange(0, len(a)):
                    if a[i] >= minfreq:
                        iminfreq = i
                        break
            if imaxfreq==-2:
                for j in xrange(iminfreq, len(a)):
                    if a[j] >= maxfreq:
                        imaxfreq = j
                        break
            rval = (iminfreq, imaxfreq)
        else:
            if iminfreq == -2:
                for i in xrange(len(a)-1, -1, -1):
                    if a[i] >= minfreq:
                        iminfreq = i
                        break
            if imaxfreq == -2:
                for j in xrange(iminfreq,-1,-1):
                    if a[j] >= maxfreq:
                        imaxfreq = j
                        break

            rval = (imaxfreq, iminfreq)
    
        if rval == (-1, -1):
            return (None, None)
        else:
            return rval

class MolecularLine():
    def __init__(self, fieldname='', species='', freqrange=(), spwid=None,
        chanrange=(), nchan=None): 
        self.fieldname=fieldname
        self.species=species
        self.freqrange = freqrange
        self.spwid = spwid 
        self.chanrange=chanrange
        self.nchan=nchan
