'''
Created on 9 Sep 2014

@author: sjw
'''
import os
import shutil

from ..common import flagging_renderer_utils as flagutils
import pipeline.infrastructure.displays.flagging as flagging
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

class T2_4MDetailsAgentFlaggerRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='agentflagger.mako', 
                 description='Deterministic flagging', always_rerender=False):
        super(T2_4MDetailsAgentFlaggerRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, result):
        weblog_dir = os.path.join(pipeline_context.report_dir,
                                  'stage%s' % result.stage_number)

        flag_totals = {}
        for r in result:
            flag_totals = utils.dict_merge(flag_totals, 
                                           flagutils.flags_for_result(r, pipeline_context))

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
            ms = pipeline_context.observing_run.get_ms(r.inputs['vis'])
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
                  'intents', 'edgespw']

        flagplots = [self.flagplot(r, pipeline_context) for r in result]
        # plot object may be None if plot failed to generate
        flagplots = [f for f in flagplots if f is not None]

        mako_context.update({'flags'     : flag_totals,
                             'agents'    : agents,
                             'dirname'   : weblog_dir,
                             'flagcmds'  : flagcmd_files,
                             'flagplots' : flagplots})

    def flagplot(self, result, context):
        plotter = flagging.PlotAntsChart(context, result)
        return plotter.plot()
