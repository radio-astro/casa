'''
Created on 11 Sep 2014

@author: sjw
'''
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.rendererutils as rendererutils

LOG = logging.get_logger(__name__)


class T2_4MDetailsLowgainFlagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    '''
    Renders detailed HTML output for the Lowgainflag task.
    '''
    def __init__(self, uri='lowgainflag.mako', 
                 description='Flag antennas with low gain',
                 always_rerender=False):
        super(T2_4MDetailsLowgainFlagRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        htmlreports = self.get_htmlreports(pipeline_context, results)        
        mako_context.update({'htmlreports' : htmlreports})

    def get_htmlreports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        htmlreports = {}
        for result in results:
#            if not hasattr(result, 'flagcmdfile'):
#                continue

            flagcmd_abspath = self.write_flagcmd_to_disk(weblog_dir, result)
            flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
            table_basename = os.path.basename(result.table)
            htmlreports[table_basename] = (flagcmd_relpath,)

        return htmlreports

    def write_flagcmd_to_disk(self, weblog_dir, result):
        tablename = os.path.basename(result.table)
        filename = os.path.join(weblog_dir, '%s.html' % tablename)
        if os.path.exists(filename):
            return filename

        rendererutils.renderflagcmds(result.flagcmds(), filename)
        return filename
