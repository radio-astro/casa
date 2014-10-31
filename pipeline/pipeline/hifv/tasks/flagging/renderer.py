import collections
import os
import shutil

import pipeline.infrastructure.displays.flagging as flagging
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')

class T2_4MDetailsVLAAgentFlaggerRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')

    def __init__(self, uri='vlaagentflagger.mako', 
                 description='VLA Deterministic flagging', always_rerender=True):
        super(T2_4MDetailsVLAAgentFlaggerRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def get_display_context(self, context, result):
        super_cls = super(T2_4MDetailsVLAAgentFlaggerRenderer, self)
        ctx = super_cls.get_display_context(context, result)

        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % result.stage_number)

        flag_totals = {}
        for r in result:
            flag_totals = utils.dict_merge(flag_totals, 
                                           self.flags_for_result(r, context))

            # copy template files across to weblog directory
            toggle_to_filenames = {'online'   : 'fileonline',
                                   'template' : 'filetemplate'}
            inputs = r.inputs
            for toggle, filenames in toggle_to_filenames.items():
                src = inputs[filenames]
                if inputs[toggle] and os.path.exists(src):
                    LOG.trace('Copying %s to %s' % (src, weblog_dir))
                    shutil.copy(src, weblog_dir)

        flagcmd_files = {}
        for r in result:
            # write final flagcmds to a file
            ms = context.observing_run.get_ms(r.inputs['vis'])
            flagcmds_filename = '%s-agent_flagcmds.txt' % ms.basename
            flagcmds_path = os.path.join(weblog_dir, flagcmds_filename)
            with open(flagcmds_path, 'w') as flagcmds_file:
                terminated = '\n'.join(r.flagcmds())
                flagcmds_file.write(terminated)

            flagcmd_files[ms.basename] = flagcmds_path

#         # collect the agent names
#         agent_names = set()
#         for r in result:
#             agent_names.update([s['name'] for s in r.summaries])
# 
#         # get agent names in execution order
#         order = ['before', 'online', 'template', 'autocorr', 'shadow', 
#                  'intents', 'edgespw']
#         agents = [s for s in order if s in agent_names]

        # return all agents so we get ticks and crosses against each one
        agents = ['before', 'online', 'template', 'autocorr', 'shadow', 
                  'intents', 'edgespw', 'clip', 'quack', 'baseband']

        flagplots = [self.flagplot(r, context) for r in result]
        # plot object may be None if plot failed to generate
        flagplots = [f for f in flagplots if f is not None]

        ctx.update({'flags'     : flag_totals,
                    'agents'    : agents,
                    'dirname'   : weblog_dir,
                    'flagcmds'  : flagcmd_files,
                    'flagplots' : flagplots})

        return ctx

    def flagplot(self, result, context):
        plotter = flagging.PlotAntsChart(context, result)
        return plotter.plot()

    def flags_for_result(self, result, context):
        ms = context.observing_run.get_ms(result.inputs['vis'])
        summaries = result.summaries

        by_intent = self.flags_by_intent(ms, summaries)
        by_spw = self.flags_by_science_spws(ms, summaries)

        return {ms.basename : utils.dict_merge(by_intent, by_spw)}

    def flags_by_intent(self, ms, summaries):
        # create a dictionary of scans per observing intent, eg. 'PHASE':[1,2,7]
        intent_scans = {}
        for intent in ('BANDPASS', 'PHASE', 'AMPLITUDE', 'TARGET'):
            # convert IDs to strings as they're used as summary dictionary keys
            intent_scans[intent] = [str(s.id) for s in ms.scans
                                    if intent in s.intents]

        # while we're looping, get the total flagged by looking in all scans 
        intent_scans['TOTAL'] = [str(s.id) for s in ms.scans]

        total = collections.defaultdict(dict)

        previous_summary = None
        for summary in summaries:

            for intent, scan_ids in intent_scans.items():
                flagcount = 0
                totalcount = 0
    
                for i in scan_ids:
                    flagcount += int(summary['scan'][i]['flagged'])
                    totalcount += int(summary['scan'][i]['total'])
        
                    if previous_summary:
                        flagcount -= int(previous_summary['scan'][i]['flagged'])
    
                ft = T2_4MDetailsVLAAgentFlaggerRenderer.FlagTotal(flagcount, 
                                                                totalcount)
                total[summary['name']][intent] = ft
                
            previous_summary = summary
                
        return total 
    
    def flags_by_science_spws(self, ms, summaries):
        science_spws = ms.get_spectral_windows(science_windows_only=True)
    
        total = collections.defaultdict(dict)
    
        previous_summary = None
        for summary in summaries:
    
            flagcount = 0
            totalcount = 0
    
            for spw in science_spws:
                spw_id = str(spw.id)
                flagcount += int(summary['spw'][spw_id]['flagged'])
                totalcount += int(summary['spw'][spw_id]['total'])
        
                if previous_summary:
                    flagcount -= int(previous_summary['spw'][spw_id]['flagged'])

            ft = T2_4MDetailsVLAAgentFlaggerRenderer.FlagTotal(flagcount, 
                                                            totalcount)
            total[summary['name']]['SCIENCE SPWS'] = ft
                
            previous_summary = summary
                
        return total