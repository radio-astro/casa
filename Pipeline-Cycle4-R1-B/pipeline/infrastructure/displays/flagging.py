from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class PlotAntsChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.figfile_root = os.path.join(self.context.report_dir,
                                         'stage%s' % self.result.stage_number,
                                         '%s-flags' % self.ms.basename)
        self.figfiles = self._get_figfiles(self.figfile_root)

    def plot(self):
        if all([os.path.exists(f) for f in self.figfiles]):
            return [self._get_plot_object(f) for f in self.figfiles]

        args = {
            'vis': self.ms.name,
            'action': 'plot',
            'plotfile': '%s.png' % self.figfile_root
        }
        task = casa_tasks.flagcmd(**args)
        try:
            result = task.execute(dry_run=False)
        except:
            # no problem, probably 'no flags commands in input' exception
            return None

        # flagcmd gives no indication of whether it's succeeded or failed, so
        # check for the presence of the output file
        return [self._get_plot_object(f)
                for f in result['plotfiles'] if os.path.exists(f)]

    def _get_figfiles(self, figfile_root):
        suffix = ['', '-001', '-002', '-003']

        num_antennas = len(self.ms.antennas)
        threshold = 28
        num_plots = num_antennas / threshold + 1
        multiple_plots = num_plots > 1

        start_idx = 0 + int(multiple_plots)
        end_idx = num_plots + int(multiple_plots)

        return ['%s%s.png' % (figfile_root, suffix[i])
                for i in range(start_idx, end_idx)]

    def _get_plot_object(self, figfile):
        return logger.Plot(figfile,
                           x_axis='Time',
                           y_axis='Antenna',
                           parameters={'vis': self.ms.basename})
