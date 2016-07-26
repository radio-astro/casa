import os
import shutil

import pipeline.hif.tasks.common.flagging_renderer_utils as flagutils
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

class T2_4MDetailsFlagTargetsALMARenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='flagtargets.mako', 
                 description='Target flagging', always_rerender=False):
        super(T2_4MDetailsFlagTargetsALMARenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, result):
        weblog_dir = os.path.join(pipeline_context.report_dir,
                                  'stage%s' % result.stage_number)

        flag_totals = {}
        for r in result:
            flag_totals = utils.dict_merge(flag_totals, 
                                           flagutils.flags_for_result(r, pipeline_context))

            # copy template files across to weblog directory
            toggle_to_filenames = {'template' : 'filetemplate'}

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


        # return all agents so we get ticks and crosses against each one
        agents = ['before', 'template']

        mako_context.update({
            'flags': flag_totals,
            'agents': agents,
            'dirname': weblog_dir,
            'flagcmds': flagcmd_files})

