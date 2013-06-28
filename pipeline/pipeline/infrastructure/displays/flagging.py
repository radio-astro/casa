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
        self.figfile = self._get_figfile()

    def plot(self):
        if os.path.exists(self.figfile):
            return self._get_plot_object()

        args = {'vis' : self.ms.name,
                'action' : 'plot',
                'plotfile' : self.figfile}
        task = casa_tasks.flagcmd(**args)
        try:
            task.execute(dry_run=False)
        except:
            # no problem, probably 'no flags commands in input' exception
            return None

        # flagcmd gives no indication of whether it's succeeded or failed, so
        # check for the presence of the output file
        if os.path.exists(self.figfile):
            return self._get_plot_object()
        else:
            return None

    def _get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            '%s-flags.png' % self.ms.basename)

    def _get_plot_object(self):
        return logger.Plot(self.figfile,
                           x_axis='Time',
                           y_axis='Antenna',
                           parameters={'vis' : self.ms.basename})
