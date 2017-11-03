import contextlib
import os

from . import display as polarization
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

LOG = logging.get_logger(__name__)


class VLASubPlotRenderer(object):
    # template = 'testdelays_plots.html'

    def __init__(self, context, result, plots, json_path, template, filename_prefix):
        self.context = context
        self.result = result
        self.plots = plots
        self.ms = os.path.basename(self.result.inputs['vis'][0])
        # print 'MY MS IS: ' + self.ms
        self.template = template
        self.filename_prefix = filename_prefix

        self.summary_plots = {}
        self.ampfreq_subpages = {}

        self.ampfreq_subpages[self.ms] = filenamer.sanitize('ampfreq' + '-%s.html' % self.ms)


        if os.path.exists(json_path):
            with open(json_path, 'r') as json_file:
                self.json = json_file.readlines()[0]
        else:
            self.json = '{}'

    def _get_display_context(self):
        return {'pcontext': self.context,
                'result': self.result,
                'plots': self.plots,
                'dirname': self.dirname,
                'json': self.json,
                'ampfreq_subpages': self.ampfreq_subpages}

    @property
    def dirname(self):
        stage = 'stage%s' % self.result.stage_number
        return os.path.join(self.context.report_dir, stage)

    @property
    def filename(self):
        filename = filenamer.sanitize(self.filename_prefix + '-%s.html' % self.ms)
        return filename

    @property
    def path(self):
        return os.path.join(self.dirname, self.filename)

    def get_file(self):
        if not os.path.exists(self.dirname):
            os.makedirs(self.dirname)

        file_obj = open(self.path, 'w')
        return contextlib.closing(file_obj)

    def render(self):
        display_context = self._get_display_context()
        t = weblog.TEMPLATE_LOOKUP.get_template(self.template)
        return t.render(**display_context)


class T2_4MDetailsCircfeedpolcalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='circfeedpolcal.mako',
                 description='CircFeed Polarization', always_rerender=False):
        super(T2_4MDetailsCircfeedpolcalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % result.stage_number)

        polarization_plotcal_plots = {}
        ampfreq_subpages = {}

        for r in result:
            if r.final:
                ktable = r.final[0].gaintable
                Dtable = r.final[1].gaintable
                Xtable = r.final[2].gaintable

                plotter = polarization.PolarizationPlotCalChart(context, r,
                                                                caltable=ktable,
                                                                yaxis='delay', xaxis='freq', antenna=r.refant,
                                                                caption='RL delay vs. freq. ')
                plots = plotter.plot()
                # -------------
                plotter = polarization.PolarizationPlotCalChart(context, r,
                                                                caltable=Xtable,
                                                                yaxis='phase', xaxis='freq', antenna=r.refant,
                                                                caption='RL phase offset vs. freq. ')
                plots.extend(plotter.plot())
                # -------------
                plotter = polarization.PolarizationPlotCalChart(context, r,
                                                                caltable=Dtable,
                                                                yaxis='amp', xaxis='antenna', antenna='',
                                                                caption='Inst. pol. amp vs. antenna')
                plots.extend(plotter.plot())
                # -------------

                ms = os.path.basename(r.inputs['vis'])
                polarization_plotcal_plots[ms] = plots

                # generate amp vs. frequency plots per antenna and JSON file
                plotter = polarization.ampfreqPerAntennaChart(context, result, Dtable)
                plots = plotter.plot()
                json_path = plotter.json_filename

                # write the html for each MS to disk
                renderer = VLASubPlotRenderer(context, result, plots, json_path, 'ampfreq_plots.mako', 'ampfreq')
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                    ampfreq_subpages[ms] = renderer.filename

            ctx.update({'dirname'                    : weblog_dir,
                        'polarization_plotcal_plots' : polarization_plotcal_plots,
                        'ampfreq_subpages'           : ampfreq_subpages})

        return ctx
