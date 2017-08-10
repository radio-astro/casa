from __future__ import absolute_import

import collections

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks

from pipeline.hif.tasks import applycal
from pipeline.h.tasks import applycal as happlycal






LOG = infrastructure.get_logger(__name__)


class ApplycalsInputs(applycal.IFApplycalInputs):
    """
    ApplycalInputs defines the inputs for the Applycal pipeline task.
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None,
                 #
                 vis=None,
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # preapply calibrations
                 opacity=None, parang=None, applymode=None, calwt=None,
                 flagbackup=None, flagsum=None, flagdetailedsum=None, gainmap=None):
        self._init_properties(vars())

    flagdetailedsum = basetask.property_with_default('flagdetailedsum', True)
    parang = basetask.property_with_default('parang', True)
    field = basetask.property_with_default('field', '')
    spw = basetask.property_with_default('spw', '')
    intent = basetask.property_with_default('intent', '')
    flagbackup = basetask.property_with_default('flagbackup', False)
    calwt = basetask.property_with_default('calwt', False)
    gainmap = basetask.property_with_default('gainmap', False)

    def to_casa_args(self):
        d = super(ApplycalsInputs, self).to_casa_args()
        d['intent'] = ''
        d['field'] = ''
        d['spw'] = ''

        return d


class Applycals(applycal.IFApplycal):
    Inputs = ApplycalsInputs
    
    def prepare(self):
        
        # Run applycal
        applycal_results = self._do_applycal()

        return applycal_results
    
    def analyse(self, results):
        return results
    
    def _do_applycal(self):

        result = self.applycal_run()

        return result

    def applycal_run(self):
        inputs = self.inputs

        # Get the calibration state for the user's target data selection. This
        # dictionary of CalTo:CalFroms gives us which calibrations should be
        # applied and how.
        merged = inputs.calstate.merged()

        # run a flagdata job to find the flagged state before applycal
        if inputs.flagsum:
            # 20170406 TN
            # flagdata task arguments are indirectly given so that sd applycal task is
            # able to edit them
            flagdata_summary_job = casa_tasks.flagdata(**self._get_flagsum_arg(vis=inputs.vis))
            stats_before = self._executor.execute(flagdata_summary_job)
            stats_before['name'] = 'before'

        merged = callibrary.fix_cycle0_data_selection(inputs.context, merged)

        if inputs.gainmap:
            applycalgroups = self.match_fields_scans()
        else:
            applycalgroups = collections.defaultdict(list)
            applycalgroups['1'] = ['']

        jobs = []

        for gainfield, scanlist in applycalgroups.iteritems():
            for calto, calfroms in merged.items():
                # if there's nothing to apply for this data selection, continue
                if not calfroms:
                    continue

                # arrange a calibration job for the unique data selection
                inputs.spw = calto.spw
                inputs.field = calto.field
                inputs.intent = calto.intent

                args = inputs.to_casa_args()
                # Do this a different way ?
                args.pop('flagsum', None)  # Flagsum is not a CASA applycal task argument
                args.pop('flagdetailedsum', None)  # Flagdetailedsum is not a CASA applycal task argument

                # set the on-the-fly calibration state for the data selection.
                calapp = callibrary.CalApplication(calto, calfroms)
                ### Note this is a temporary workaround ###
                args['antenna'] = self.antenna_to_apply
                ### Note this is a temporary workaround ###
                args['gaintable'] = calapp.gaintable
                args['gainfield'] = calapp.gainfield
                args['spwmap'] = calapp.spwmap
                args['interp'] = calapp.interp
                args['calwt'] = calapp.calwt
                args['applymode'] = inputs.applymode

                if inputs.gainmap:
                    # Determine what tables gainfield should used with if mode='gainmap'
                    for i, table in enumerate(args['gaintable']):
                        if 'finalampgaincal' in table or 'finalphasegaincal' in table:
                            args['interp'][i] = 'linear'
                            args['gainfield'][i] = gainfield

                    # args['interp'] = ['', '', '', '', 'linear,freqflag', '', 'linear', 'linear']
                    # args['gainfield'] = ['','','','','','', gainfield, gainfield]
                    args['scan'] = ','.join(scanlist)
                    LOG.info("Using gainfield {!s} and scan={!s}".format(gainfield, ','.join(scanlist)))

                args.pop('gainmap', None)

                jobs.append(casa_tasks.applycal(**args))


        if inputs.gainmap:
            for calto, calfroms in merged.items():
                # if there's nothing to apply for this data selection, continue
                if not calfroms:
                    continue

                # arrange a calibration job for the unique data selection
                inputs.spw = calto.spw
                inputs.field = calto.field
                inputs.intent = 'AMPLITUDE, BANDPASS, CHECK, POLARIZATION'

                args = inputs.to_casa_args()
                # Do this a different way ?
                args.pop('flagsum', None)  # Flagsum is not a CASA applycal task argument
                args.pop('flagdetailedsum', None)  # Flagdetailedsum is not a CASA applycal task argument

                # set the on-the-fly calibration state for the data selection.
                calapp = callibrary.CalApplication(calto, calfroms)
                ### Note this is a temporary workaround ###
                args['antenna'] = self.antenna_to_apply
                ### Note this is a temporary workaround ###
                args['gaintable'] = calapp.gaintable
                args['gainfield'] = calapp.gainfield
                args['spwmap'] = calapp.spwmap
                args['interp'] = calapp.interp
                args['calwt'] = calapp.calwt
                args['applymode'] = inputs.applymode

                args.pop('gainmap', None)

                jobs.append(casa_tasks.applycal(**args))


        # execute the jobs

        for job in jobs:
            self._executor.execute(job)

        # run a final flagdata job to get the flagging statistics after
        # application of the potentially flagged caltables
        if inputs.flagsum:
            stats_after = self._executor.execute(flagdata_summary_job)
            stats_after['name'] = 'applycal'

        applied = [callibrary.CalApplication(calto, calfroms)
                   for calto, calfroms in merged.items()]

        result = happlycal.ApplycalResults(applied)

        if inputs.flagsum:
            result.summaries = [stats_before, stats_after]

        # Flagging stats by spw and antenna

        if inputs.flagsum and inputs.flagdetailedsum:
            ms = self.inputs.context.observing_run.get_ms(inputs.vis)
            spws = ms.get_spectral_windows()
            spwids = [spw.id for spw in spws]

            # Note should intent be set to inputs.intent as shown below or is there
            # a reason not to do this.
            # fields = ms.get_fields(intent=inputs.intent)
            fields = ms.get_fields(intent='BANDPASS,PHASE,AMPLITUDE,CHECK,TARGET')

            if 'VLA' in self.inputs.context.project_summary.telescope:
                calfields = ms.get_fields(intent='AMPLITUDE,PHASE,BANDPASS')
                alltargetfields = ms.get_fields(intent='TARGET')

                fields = calfields

                Nplots = (len(alltargetfields) / 30) + 1

                targetfields = [field for field in alltargetfields[0:len(alltargetfields):Nplots]]

                fields.extend(targetfields)

            flagsummary = collections.OrderedDict()
            flagkwargs = []

            for field in fields:
                flagsummary[field.name.strip('"')] = {}

            for spwid in spwids:
                flagline = "spw='" + str(spwid) + "' fieldcnt=True mode='summary' name='AntSpw" + str(spwid).zfill(3)
                flagkwargs.append(flagline)

            # 20170406 TN
            # Tweak flagkwargs (default is do nothing)
            flagkwargs = self._tweak_flagkwargs(flagkwargs)

            # BRK note - Added kwarg fieldcnt based on Justo's changes, July 2015
            # Need to have fieldcnt in the flagline above
            flaggingjob = casa_tasks.flagdata(vis=inputs.vis, mode='list', inpfile=flagkwargs, flagbackup=False)
            flagdicts = self._executor.execute(flaggingjob)

            # BRK note - for Justo's new flagging scheme, need to rearrrange
            # the dictionary keys in the order of field, spw report, antenna, with added name and type keys
            #   on the third dictionary level.

            # Set into single dictionary report (single spw) if only one dict returned
            if len(flagkwargs) == 1:
                flagdictssingle = flagdicts
                flagdicts = {}
                flagdicts['report0'] = flagdictssingle

            for key in flagdicts.keys():  # report level
                fieldnames = flagdicts[key].keys()
                fieldnames.remove('name')
                fieldnames.remove('type')
                for fieldname in fieldnames:
                    try:
                        flagsummary[fieldname][key] = flagdicts[key][fieldname]
                        spwid = flagdicts[key][fieldname]['spw'].keys()[0]
                        flagsummary[fieldname][key]['name'] = 'AntSpw' + str(spwid).zfill(3) + 'Field_' + str(fieldname)
                        flagsummary[fieldname][key]['type'] = 'summary'
                    except:
                        LOG.debug("No flags to report for " + str(key))

            result.flagsummary = flagsummary

        return result

    def match_fields_scans(self):
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

        # Count the number of groups
        intentslist = [list(scan.intents) for scan in m.get_scans(scan_intent='PHASE,TARGET')]
        intents = []
        from itertools import groupby
        groups = [list(j) for i, j in groupby(intents)]
        primarygroups = [list(set(group))[0] for group in groups]
        ngroups = primarygroups.count('TARGET')

        gcount = 0
        phaselist = []
        targetscans = []
        groups = []
        phase1 = False
        phase2 = False
        prev_phase2 = False

        TargetGroup = collections.namedtuple("TargetGroup", "phase1 targetscans phase2")
        scans = m.get_scans(scan_intent='PHASE,TARGET')
        for idx, scan in enumerate(scans):
            fieldset = scan.fields
            fieldobj = list(fieldset)[0]
            fieldid = fieldobj.id

            # print scan.id, list(scan.intents), "Field id: ", fieldid, scan.start_time
            if 'PHASE' in list(scan.intents):
                if not phase1 or (phase1 and bool(targetscans) == False):
                    phase1 = scan
                else:
                    phase2 = scan
                    prev_phase2 = phase2
                targetscans.append(scan)
            elif 'TARGET' in list(scan.intents):
                if not phase1 or prev_phase2:
                    phase1 = prev_phase2

                targetscans.append(scan)

            # Check for consecutive time ranges
            

            # see if this scan is the last one in the relevant scan list
            # or see if we have a phase2
            # if so, end the group
            if phase2 or idx == len(scans)-1:
                groups.append(TargetGroup(phase1,targetscans,phase2))
                phase1 = False
                phase2 = False
                targetscans = []

        #applycalgroups = []
        applycalgroups = collections.defaultdict(list)

        for idx, group in enumerate(groups):
            #print "Applycal Group", idx
            if group.phase2:
                #print "\tPhase cals....", group.phase1.id, group.phase2.id
                fieldset = group.phase1.fields
                fieldobj = list(fieldset)[0]
                phase1fieldid = fieldobj.id
                fieldset = group.phase2.fields
                fieldobj = list(fieldset)[0]
                phase2fieldid = fieldobj.id
                gainfield = ','.join([str(phase1fieldid), str(phase2fieldid)])

            else:
                #print "\tPhase cal.....", group.phase1.id
                fieldset = group.phase1.fields
                fieldobj = list(fieldset)[0]
                gainfield = str(fieldobj.id)

            targetscans = [str(targetscan.id) for targetscan in group.targetscans]
            #print "\tTargets...........", ','.join(targetscans)

            #applycalgroups.append({'gainfield' : gainfield,
            #                       'scan'      : targetscans
            #                       })
            gainfieldkey = gainfield.split(',')[1]
            applycalgroups[gainfieldkey].extend(targetscans)

        for gainfield, scanlist in applycalgroups.iteritems():
            print "Applycal Group"
            print "\tGainfield....", gainfield
            print "\t\tScanList...", ','.join(scanlist)
            print " "

        return applycalgroups




