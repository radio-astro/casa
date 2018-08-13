"""
Created on 24 Oct 2014

@author: brk
"""

import contextlib
import os

import display as syspowerdisplay
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog

LOG = logging.get_logger(__name__)


class VLASubPlotRenderer(object):

    def __init__(self, context, result, plots, json_path, template, filename_prefix):
        self.context = context
        self.result = result
        self.plots = plots
        self.ms = os.path.basename(self.result.inputs['vis'])
        self.template = template
        self.filename_prefix = filename_prefix

        self.summary_plots = {}
        self.syspowerspgain_subpages = {}
        self.pdiffspgain_subpages = {}

        self.syspowerspgain_subpages[self.ms] = filenamer.sanitize('spgainrq' + '-%s.html' % self.ms)
        self.pdiffspgain_subpages[self.ms] = filenamer.sanitize('spgainpdiff' + '-%s.html' % self.ms)

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
                'syspowerspgain_subpages': self.syspowerspgain_subpages,
                'pdiffspgain_subpages': self.pdiffspgain_subpages}

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


class T2_4MDetailssyspowerRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='syspower.mako',
                 description='Syspower (modified rq gains)',
                 always_rerender=False):
        super(T2_4MDetailssyspowerRenderer, self).__init__(uri=uri, description=description,
                                                           always_rerender=always_rerender)

    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailssyspowerRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        opacity_plots = {}
        spw = {}
        center_frequencies = {}
        opacities = {}
        swpowspgain_subpages = {}
        pdiffspgain_subpages = {}
        box_plots = {}
        bar_plots = {}

        for result in results:

            plotter = syspowerdisplay.syspowerBoxChart(context, result)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            box_plots[ms] = plots

            plotter = syspowerdisplay.syspowerBarChart(context, result)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            bar_plots[ms] = plots

            # generate switched power plots and JSON file
            plotter = syspowerdisplay.syspowerPerAntennaChart(context, result, 'spgain',
                                                              result.gaintable, 'syspower', 'rq')
            plots = plotter.plot()
            json_path = plotter.json_filename

            # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'syspower_plots.mako', 'spgainrq')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                swpowspgain_subpages[ms] = renderer.filename

            # plot template pdiff table
            plotter = syspowerdisplay.syspowerPerAntennaChart(context, result, 'spgain',
                                                              result.template_table, 'syspower', 'pdiff')
            plots = plotter.plot()
            json_path = plotter.json_filename

            # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'syspower_plots.mako', 'spgainpdiff')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                pdiffspgain_subpages[ms] = renderer.filename

        ctx.update({'opacity_plots': opacity_plots,
                    'spw': spw,
                    'center_frequencies': center_frequencies,
                    'opacities': opacities,
                    'dirname': weblog_dir,
                    'box_plots': box_plots,
                    'bar_plots': bar_plots,
                    'syspowerspgain_subpages': swpowspgain_subpages,
                    'pdiffspgain_subpages': pdiffspgain_subpages,
                    'tec_plotfile': ''})

        return ctx
