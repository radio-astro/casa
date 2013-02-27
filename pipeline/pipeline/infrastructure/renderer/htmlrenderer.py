from __future__ import absolute_import
import atexit
import collections
import contextlib
import datetime
import os
import shutil
import StringIO
import sys
import tempfile
import types
import zipfile

from mako.lookup import TemplateLookup

import casadef
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays.summary as summary
import pipeline.infrastructure.displays.bandpass as bandpass
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.filenamer as filenamer
from pipeline.infrastructure.renderer import templates
from pipeline.infrastructure.renderer.templates import resources
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.renderer.qa2adapter as qa2adapter
import pipeline.infrastructure.utils as utils

#import pipeline.tasks as tasks
from pipeline import tasks as tasks

import pipeline.hif.tasks.applycal.applycal as applycal
import pipeline.hif.tasks.flagging.flagdeteralma as flagdeteralma
import pipeline.hif.tasks.gaincal.gtypegaincal as gtypegaincal
import pipeline.hif.tasks.bandpass.phcorbandpass as phcorbandpass
import pipeline.hif.tasks.fluxscale.gcorfluxscale as gcorfluxscale
import pipeline.hif.tasks.makecleanlist.makecleanlist as makecleanlist
import pipeline.hif.tasks.fluxscale.normflux as normflux
import pipeline.hif.tasks.refant as refant
import pipeline.hif.tasks.setmodel.setjy as setjy
import pipeline.hif.tasks.tsyscal.tsyscal as tsyscal
import pipeline.hif.tasks.tsysflag.tsysflag as tsysflag
import pipeline.hif.tasks.wvrgcal as wvrgcal
import pipeline.hif.tasks.wvrgcal.wvrgcalflag as wvrgcalflag

#import pipeline.tasks.singledish as singledish 
import pipeline.hsd.tasks.importdata.importdata as sdimportdata
import pipeline.hsd.tasks.exportdata.exportdata as sdexportdata
import pipeline.hsd.tasks.calsky.calibration as calibration
import pipeline.hsd.tasks.inspectdata.inspection as inspection
import pipeline.hsd.tasks.reduce.reduction as reduction

LOG = logging.get_logger(__name__)


def get_task_description(result_obj):
    if not isinstance(result_obj, collections.Iterable):
        return get_task_description([result_obj,])
    
    if len(result_obj) is 0:
        msg = 'Cannot get description for zero-length results list'
        LOG.error(msg)
        return msg
    
    task_cls = result_obj[0].task
    if type(task_cls) is types.NoneType:
        results_cls = result_obj[0].__class__.__name__
        msg = 'No task registered on results of type %s' % results_cls
        LOG.warning(msg)
        return msg

    if task_cls is applycal.Applycal:
        return 'Apply calibrations from context'

    if task_cls is flagdeteralma.FlagDeterALMA:
        return 'ALMA deterministic flagging'

    if task_cls is gcorfluxscale.GcorFluxscale:
        return 'Phased-up fluxscale'

    if task_cls is gtypegaincal.GTypeGaincal:
        return 'G-type gain calibration'
    
    if task_cls is tasks.ImportData:
        names = []
        for result in result_obj:
            names.extend([ms.basename for ms in result.mses])
        return 'Register %s with pipeline' % utils.commafy(names)

    if task_cls is makecleanlist.MakeCleanList:
        return 'Compile a list of cleaned images to be calculated'

    if task_cls is normflux.NormaliseFlux:
        return 'Calculate mean fluxes of calibrators'

    if task_cls is phcorbandpass.PhcorBandpass:
        return 'Phase-up bandpass calibration'

    if task_cls is refant.RefAnt:
        return 'Select reference antennas'

    if task_cls is setjy.Setjy:
        return 'Set calibrator model visibilities'

    if task_cls is sdimportdata.SDImportData:
        names = []
        for result in result_obj:
            names.extend([st.basename for st in result.scantables])
        return 'Register %s with pipeline' % utils.commafy(names)

    if task_cls is reduction.SDReduction:
        return 'Single-dish end-to-end reduction'
   
    if task_cls is sdexportdata.SDExportData:
        return 'Single-dish SDExportData'

    if task_cls is tsyscal.Tsyscal:
        return 'Calculate Tsys calibration'

    if task_cls is tsysflag.Tsysflag:
        return 'Flag Tsys calibration'

    if task_cls is wvrgcal.Wvrgcal:
        return 'Calculate wvr calibration'

    if task_cls is wvrgcalflag.Wvrgcalflag:
        return 'Calculate and flag wvr calibration'

    if task_cls is inspection.SDInspectData:
        datatable = result_obj[0].outcome['instance']
        names = datatable.getkeyword('FILENAMES')
        return 'Inspect %s'%(utils.commafy(names))

    if task_cls is calibration.SDCalibration:
        return 'Calibrate sky ([ON-OFF]/OFF)'

    if LOG.isEnabledFor(LOG.todo):
        LOG.todo('No task description for \'%s\'' % task_cls.__name__)
        return ('\'%s\' (developers should add a task description)'
                '' % task_cls.__name__)
    
    return ('\'%s\'' % task_cls.__name__)

def get_task_name(result_obj):
    if not isinstance(result_obj, collections.Iterable):
        return get_task_name([result_obj,])
    
    if len(result_obj) is 0:
        msg = 'Cannot get task name for zero-length results list'
        LOG.error(msg)
        return msg
    
    task_cls = result_obj[0].task
    if type(task_cls) is types.NoneType:
        results_cls = result_obj[0].__class__.__name__
        msg = 'No task registered on results of type %s' % results_cls
        LOG.warning(msg)
        return msg
    
    return task_cls.__name__

def get_stage_number(result_obj):
    if not isinstance(result_obj, collections.Iterable):
        return get_stage_number([result_obj,])
    
    if len(result_obj) is 0:
        msg = 'Cannot get stage number for zero-length results list'
        LOG.error(msg)
        return msg
    
    return result_obj[0].stage_number

def get_plot_dir(context, stage_number):
    stage_dir = os.path.join(context.report_dir, 'stage%d' % stage_number)
    plots_dir = os.path.join(stage_dir, 'plots')
    return plots_dir


class StdOutFile(StringIO.StringIO):
    def close(self):
        sys.stdout.write(self.getvalue())
        StringIO.StringIO.close(self)


class Session(object):
    def __init__(self, mses=[], name='Unnamed Session'):
        self.mses = mses
        self.name = name
    
    @staticmethod
    def get_sessions(context):
        d = collections.defaultdict(list)
        for ms in context.observing_run.measurement_sets:
            d[ms.session].append(ms)

        return [Session(v, k) for k,v in d.items()]


class RendererBase(object):
    """
    Base renderer class.
    """
    @classmethod
    def get_file(cls, context, hardcopy):
        filename = os.path.join(context.report_dir, cls.output_file)
        file_obj = open(filename, 'w') if hardcopy else StdOutFile()
        return contextlib.closing(file_obj)

    @classmethod
    def render(cls, context, hardcopy=False):
        with cls.get_file(context, hardcopy) as fileobj:
            template = TemplateFinder.get_template(cls.template)
            display_context = cls.get_display_context(context)
            fileobj.write(template.render(**display_context))


class TemplateFinder(object):
    _templates_dir = os.path.dirname(templates.__file__)
    _tmp_directory = tempfile.mkdtemp()
    LOG.trace('Mako working directory: %s' % _tmp_directory)
    # Remove temporary Mako codegen directory on termination of Python process 
    atexit.register(lambda: shutil.rmtree(TemplateFinder._tmp_directory,
                                          ignore_errors=True))
    _lookup = TemplateLookup(directories=[_templates_dir], 
                             module_directory=_tmp_directory)
    
    @staticmethod
    def get_template(filename, **kwargs):
        return TemplateFinder._lookup.get_template(filename)


class T1_1Renderer(RendererBase):
    """
    T1-1 OUS Splash Page renderer
    """
    output_file = 't1-1.html'
    template = 't1-1.html'

    @staticmethod
    def get_display_context(context):
        obs_start = context.observing_run.start_datetime
        obs_end = context.observing_run.end_datetime

        array_names = set([ms.antenna_array.name 
                           for ms in context.observing_run.measurement_sets])

        # pipeline execution start, end and duration
        exec_start = context.results[0].timestamps.start
        exec_end = context.results[-1].timestamps.end
        # remove unnecessary precision for execution duration
        dt = exec_end - exec_start
        exec_duration = datetime.timedelta(days=dt.days, seconds=dt.seconds)

        out_fmt = '%Y-%m-%d %H:%M:%S'        
        return {'pcontext'      : context,
                'casa_version'  : casadef.casa_version,
                'casa_revision' : casadef.subversion_revision,
                'obs_start'     : obs_start.strftime(out_fmt),
                'obs_end'       : obs_end.strftime(out_fmt),
                'array_names'   : utils.commafy(array_names),
                'exec_start'    : exec_start.strftime(out_fmt),
                'exec_end'      : exec_end.strftime(out_fmt),
                'exec_duration' : str(exec_duration)           }


class T1_2Renderer(RendererBase):
    """
    T1-2 Observation Summary renderer
    """
    output_file = 't1-2.html'
    template = 't1-2.html'

    @staticmethod
    def get_display_context(context):
        bands = set()
        science_freqs = []
        for ms in context.observing_run.measurement_sets:
            science_spws = ms.get_spectral_windows(science_windows_only=True)
            science_freqs.extend([spw.ref_frequency for spw in science_spws])
            bands.update([spw.band for spw in science_spws])
        bands = sorted(bands)
        
        science_sources = set()
        for ms in context.observing_run.measurement_sets:
            science_sources.update([s.name for s in ms.sources 
                                    if 'TARGET' in s.intents])
        science_sources = sorted(science_sources)

        calibrators = set()
        for ms in context.observing_run.measurement_sets:
            calibrators.update([s.name for s in ms.sources 
                                if 'TARGET' not in s.intents])
        calibrators = sorted(calibrators)
        
        return {'pcontext'        : context,
                'mses'            : context.observing_run.measurement_sets,
                'science_sources' : utils.commafy(science_sources),
                'calibrators'     : utils.commafy(calibrators),
                'science_bands'   : utils.commafy(bands)}


class T1_3MRenderer(RendererBase):
    """
    T1-3M renderer
    """
    output_file = 't1-3.html'
    template = 't1-3m.html'
    
    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)

        return {'pcontext'   : context,
                'qa2adapter' : qa2results}


class T1_4MRenderer(RendererBase):
    """
    T1-4M renderer
    """
    output_file = 't1-4.html'
    # TODO get template at run-time
    template = 't1-4m.html'
    
    @staticmethod
    def get_display_context(context):
        return {'pcontext' : context,
                'results'  : context.results}
        

class T2_1Renderer(RendererBase):
    """
    T2-4M renderer
    """
    output_file = 't2-1.html'
    template = 't2-1.html'

    @staticmethod
    def get_display_context(context):
        sessions = Session.get_sessions(context)
        return {'pcontext' : context,
                'sessions' : sessions}


class T2_1DetailsRenderer(object):
    output_file = 't2-1_details.html'
    template = 't2-1_details.html'

    @classmethod
    def get_file(cls, context, session, ms, hardcopy):
        ms_dir = os.path.join(context.report_dir, 
                              'session%s' % session.name,
                              ms.basename)
        if hardcopy and not os.path.exists(ms_dir):
            os.makedirs(ms_dir)
        filename = os.path.join(ms_dir, cls.output_file)
        file_obj = open(filename, 'w') if hardcopy else StdOutFile()
        return contextlib.closing(file_obj)

    @staticmethod
    def get_display_context(context, ms):
        inputs = summary.IntentVsTimeChart.Inputs(context, vis=ms.basename)
        task = summary.IntentVsTimeChart(inputs)
        intent_vs_time = task.plot()

        inputs = summary.FieldVsTimeChart.Inputs(context, vis=ms.basename)
        task = summary.FieldVsTimeChart(inputs)
        field_vs_time = task.plot()
        
        return {'pcontext'       : context,
                'ms'             : ms,
                'intent_vs_time' : intent_vs_time,
                'field_vs_time'  : field_vs_time  }

    @classmethod
    def render(cls, context, hardcopy=False):
        for session in Session.get_sessions(context):
            for ms in session.mses:
                with cls.get_file(context, session, ms, hardcopy) as fileobj:
                    template = TemplateFinder.get_template(cls.template)
                    display_context = cls.get_display_context(context, ms)
                    fileobj.write(template.render(**display_context))


class T2_2_1Renderer(RendererBase):
    """
    T2-2-1 renderer
    """
    output_file = 't2-2-1.html'
    template = 't2-2-1.html'

    @staticmethod
    def get_display_context(context):
        mosaics = []
        for ms in context.observing_run.measurement_sets:
            for source in ms.sources:
                num_pointings = len([f for f in ms.fields 
                                     if f.source_id == source.id])
                if num_pointings > 1:
                    task = summary.MosaicChart(context, ms, source)
                    mosaics.append((ms, source, task.plot()))

        return {'pcontext' : context,
                'mses'     : context.observing_run.measurement_sets,
                'mosaics'  : mosaics}


class T2_2_2Renderer(RendererBase):
    """
    T2-2-2 renderer
    """
    output_file = 't2-2-2.html'
    template = 't2-2-2.html'

    @staticmethod
    def get_display_context(context):
        return {'pcontext' : context,
                'mses'     : context.observing_run.measurement_sets}


class T2_2_3Renderer(RendererBase):
    """
    T2-2-3 renderer
    """
    output_file = 't2-2-3.html'
    template = 't2-2-3.html'

    @staticmethod
    def get_display_context(context):
        plot_ants = []
        for ms in context.observing_run.measurement_sets:
            task = summary.PlotAntsChart2(context, ms)
            plot_ants.append((ms, task.plot()))
        
        return {'pcontext'  : context,
                'plot_ants' : plot_ants}


class T2_2_4Renderer(RendererBase):
    """
    T2-2-4 renderer
    """
    output_file = 't2-2-4.html'
    template = 't2-2-4.html'

    @staticmethod
    def get_display_context(context):
        azel_plots = []
#        for ms in context.observing_run.measurement_sets:
#            task = summary.AzElChart(context, ms)
#            azel_plots.append((ms, task.plot()))

        el_vs_time_plots = []
#        for ms in context.observing_run.measurement_sets:
#            task = summary.ElVsTimeChart(context, ms)
#            el_vs_time_plots.append((ms, task.plot()))

        return {'pcontext'         : context,
                'azel_plots'       : azel_plots,
                'el_vs_time_plots' : el_vs_time_plots}


class T2_3_1MRenderer(RendererBase):
    """
    Renderer for T2-3-1M: the QA2 calibration section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-1m.html'
    # the template file for this renderer
    template = 't2-3-1m.html'

    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)
        
        # we only need to pass the calibration section through to the template
        qa2section = qa2results.sections[qa2adapter.CalibrationQA2Section]

        return {'pcontext'   : context,
                'qa2section' : qa2section}


class T2_3_2MRenderer(RendererBase):
    """
    Renderer for T2-3-2M: the QA2 line finding section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-2m.html'
    # the template file for this renderer
    template = 't2-3-2m.html'

    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)
        
        # we only need to pass the calibration section through to the template
        qa2section = qa2results.sections[qa2adapter.LineFindingQA2Section]

        return {'pcontext'   : context,
                'qa2section' : qa2section}


class T2_3_3MRenderer(RendererBase):
    """
    Renderer for T2-3-3M: the QA2 flagging section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-3m.html'
    # the template file for this renderer
    template = 't2-3-3m.html'

    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)
        
        # we only need to pass the calibration section through to the template
        qa2section = qa2results.sections[qa2adapter.FlaggingQA2Section]

        return {'pcontext'   : context,
                'qa2section' : qa2section}


class T2_3_4MRenderer(RendererBase):
    """
    Renderer for T2-3-4M: the QA2 imaging section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-4m.html'
    # the template file for this renderer
    template = 't2-3-4m.html'

    @staticmethod
    def get_display_context(context):
        # QA2 templates need a QA2 perspective of the results. Wrap the results
        # in an adapter that provides this perspective.
        qa2results = qa2adapter.ResultsToQA2Adapter(context.results)
        
        # we only need to pass the calibration section through to the template
        qa2section = qa2results.sections[qa2adapter.ImagingQA2Section]

        return {'pcontext'   : context,
                'qa2section' : qa2section}


class T2_3MDetailsDefaultRenderer(object):
    def __init__(self, template='t2-3m_details.html'):
        self.template = template
        
    def get_display_context(self, context, result):
        return {'pcontext' : context,
                'result'   : result}

    def render(self, context, result):
        display_context = self.get_display_context(context, result)
        t = TemplateFinder.get_template(self.template)
        return t.render(**display_context)


class T2_3MDetailsRenderer(object):
    # the filename component of the output file. While this is the same for
    # all results, the directory is stage-specific, so there's no risk of
    # collisions  
    output_file = 't2-3m_details.html'
    
    # the default renderer used should the task:renderer mapping not specify a
    # specialised renderer
    _default_renderer = T2_3MDetailsDefaultRenderer()

    """
    Get the file object for this renderer.

    The returned file object will be either direct output to a file or to
    stdout, depending on whether hardcopy is set to true or false
    respectively. 

    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :param hardcopy: render to disk (true) or to stdout (false). Default is
        true
    :type hardcopy: boolean
    :rtype: a file object
    """
    @classmethod
    def get_file(cls, context, result, hardcopy):
        # HTML output will be written to the directory 'stageX' 
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, stage)

        # to avoid any subsequent file not found errors, create the directory
        # if a hard copy is requested and the directory is missing
        if hardcopy and not os.path.exists(stage_dir):
            os.makedirs(stage_dir)

        # construct the relative filename, eg. 'stageX/t2-3m_details.html'
        filename = os.path.join(stage_dir, cls.output_file)
        
        # create a file object that writes to a file if a hard copy is 
        # requested, otherwise return a file object that flushes to stdout
        file_obj = open(filename, 'w') if hardcopy else StdOutFile()
        
        # return the file object wrapped in a context manager, so we can use
        # it with the autoclosing 'with fileobj as f:' construct
        return contextlib.closing(file_obj)

    """
    Render the detailed QA2 perspective of each Results in the given context.
    
    This renderer creates detailed T2_3M output for each Results. Each Results
    in the context is passed to a specialised renderer, which generates
    custom output and plots for the Result in question.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param hardcopy: render to disk (true) or to stdout (false). Default is true.
    :type hardcopy: boolean
    """
    @classmethod
    def render(cls, context, hardcopy=False):
        # for each result accepted and stored in the context..
        for result in context.results:
            # .. get the file object to which we'll render the result
            with cls.get_file(context, result, hardcopy) as fileobj:
                # we only handle lists of results, so wrap single objects in a
                # list if necessary
                if not isinstance(result, collections.Iterable):
                    l = basetask.ResultsList()
                    l.append(result)
                    l.timestamps = result.timestamps
                    l.inputs = result.inputs
                    result = l
                task = result[0].task

                # get the map of t2_3m renderers from the dictionary
                t2_3m_renderers = renderer_map[T2_3MDetailsRenderer]
                # find the renderer appropriate to the task..
                renderer = t2_3m_renderers.get(task, cls._default_renderer)
                # .. and write the renderer's interpretation of this result to
                # the file object  
                LOG.trace('Using %s to render %s result' % (
                    renderer.__class__.__name__, task.__name__))
                fileobj.write(renderer.render(context, result))


class T2_4MRenderer(RendererBase):
    """
    T2-4M renderer
    """
    output_file = 't2-4m.html'
    template = 't2-4m.html'

    @staticmethod
    def get_display_context(context):
        return {'pcontext' : context,
                'results'  : context.results }


class T2_4MDetailsDefaultRenderer(object):
    def __init__(self, template='t2-4m_details.html'):
        self.template = template
        
    def get_display_context(self, context, result):
        return {'pcontext' : context,
                'result'   : result  }

    def render(self, context, result):
        display_context = self.get_display_context(context, result)
        t = TemplateFinder.get_template(self.template)
        return t.render(**display_context)


class PlotGroupRenderer(object):
    template = 'plotgroup.html'

    def __init__(self, context, result, plot_group, prefix=''):
        self.context = context
        self.result = result
        self.plot_group = plot_group
        self.prefix = prefix
        if self.prefix != '':
            self.prefix = '%s-' % prefix
        
    @property
    def dirname(self):
        stage = 'stage%s' % self.result.stage_number
        return os.path.join(self.context.report_dir, stage)
    
    @property
    def filename(self):
        filename = '%s%s-%s-thumbnails.html' % (self.plot_group.x_axis, 
                                                self.plot_group.y_axis,
                                                self.prefix)
        filename = filenamer.sanitize(filename)
        return filename
    
    @property
    def path(self):
        return os.path.join(self.dirname, self.filename)
    
    def get_file(self, hardcopy=True):
        if hardcopy and not os.path.exists(self.dirname):
            os.makedirs(self.dirname)
            
        file_obj = open(self.path, 'w') if hardcopy else StdOutFile()
        return contextlib.closing(file_obj)
    
    def _get_display_context(self):
        return {'pcontext'   : self.context,
                'result'     : self.result,
                'plot_group' : self.plot_group }

    def render(self):
        display_context = self._get_display_context()
        t = TemplateFinder.get_template(self.template)
        return t.render(**display_context)


"""
T2_4MDetailsBandpassRenderer generates the detailed T2_4M-level plots and
output specific to the bandpass calibration task.
"""
class T2_4MDetailsBandpassRenderer(T2_4MDetailsDefaultRenderer):
    def __init__(self, template='t2-4m_details-bandpass.html'):
        # set the name of our specialised Mako template via the superclass
        # constructor 
        super(T2_4MDetailsBandpassRenderer, self).__init__(template)

    """
    Get the Mako context appropriate to the results created by a Bandpass
    task.
    
    This routine triggers the creation of the plots specific to the bandpass
    task, creating thumbnail pages as necessary. The returned dictionary  
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param results: the bandpass results to describe
    :type results: 
        :class:`~pipeline.infrastructure.tasks.bandpass.common.BandpassResults`
    :rtype a dictionary that can be passed to the matching bandpass Mako 
        template
    """
    def get_display_context(self, context, results):
        # get the standard Mako context from the superclass implementation 
        super_cls = super(T2_4MDetailsBandpassRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        # generate the bandpass-specific plots, collecting the Plot objects
        # returned by the plot generator 
        plots = []
        for result in results:
            inputs = bandpass.BandpassDisplay.Inputs(context, result)
            task = bandpass.BandpassDisplay(inputs)
            plots.append(task.plot())

        # Group the Plots by axes and plot types; each logical grouping will
        # be contained in a PlotGroup  
        plot_groups = logger.PlotGroup.create_plot_groups(plots)
        # Write the thumbnail pages for each plot grouping to disk 
        for plot_group in plot_groups:
            renderer = PlotGroupRenderer(context, result, plot_group)
            plot_group.filename = renderer.filename 
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'plot_groups' : plot_groups })

        return ctx


class T2_4MDetailsRenderer(object):
    # the filename component of the output file. While this is the same for
    # all results, the directory is stage-specific, so there's no risk of
    # collisions  
    output_file = 't2-4m_details.html'
    
    # the default renderer should the task:renderer mapping not specify a
    # specialised renderer
    _default_renderer = T2_4MDetailsDefaultRenderer()

    """
    Get the file object for this renderer.

    The returned file object will be either direct output to a file or to
    stdout, depending on whether hardcopy is set to true or false
    respectively. 

    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :param hardcopy: render to disk (true) or to stdout (false). Default is
        true
    :type hardcopy: boolean
    :rtype: a file object
    """
    @classmethod
    def get_file(cls, context, result, hardcopy):
        # HTML output will be written to the directory 'stageX' 
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, stage)

        # to avoid any subsequent file not found errors, create the directory
        # if a hard copy is requested and the directory is missing
        if hardcopy and not os.path.exists(stage_dir):
            os.makedirs(stage_dir)

        # construct the relative filename, eg. 'stageX/t2-4m_details.html'
        filename = os.path.join(stage_dir, cls.output_file)
        
        # create a file object that writes to a file if a hard copy is 
        # requested, otherwise return a file object that flushes to stdout
        file_obj = open(filename, 'w') if hardcopy else StdOutFile()
        
        # return the file object wrapped in a context manager, so we can use
        # it with the autoclosing 'with fileobj as f:' construct
        return contextlib.closing(file_obj)

    """
    Render the detailed task-centric view of each Results in the given
    context.
    
    This renderer creates detailed, T2_4M output for each Results. Each
    Results in the context is passed to a specialised renderer, which 
    generates customised output and plots for the Result in question.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param hardcopy: render to disk (true) or to stdout (false). Default is true.
    :type hardcopy: boolean
    """
    @classmethod
    def render(cls, context, hardcopy=False):
        # for each result accepted and stored in the context..
        for result in context.results:
            # .. get the file object to which we'll render the result
            with cls.get_file(context, result, hardcopy) as fileobj:
                # we only handle lists of results, so wrap single objects in a
                # list if necessary
                if not isinstance(result, collections.Iterable):
                    l = basetask.ResultsList()
                    l.append(result)
                    l.timestamps = result.timestamps
                    l.inputs = result.inputs
                    result = l
                task = result[0].task

                # get the map of t2_4m renderers from the dictionary
                t2_4m_renderers = renderer_map[T2_4MDetailsRenderer]
                # find the renderer appropriate to the task..
                renderer = t2_4m_renderers.get(task, cls._default_renderer)
                # .. and write the renderer's interpretation of this result to
                # the file object  
                LOG.trace('Using %s to render %s result' % (
                    renderer.__class__.__name__, task.__name__))
                fileobj.write(renderer.render(context, result))


class WebLogGenerator(object):
    renderers = [T1_1Renderer,         # OUS splash page
                 T1_2Renderer,         # observation summary
                 T1_3MRenderer,        # QA2 summary
                 T1_4MRenderer,        # task summary
                 T2_1Renderer,         # session tree
                 T2_1DetailsRenderer,  # session details
                 T2_2_1Renderer,       # spatial setup
                 T2_2_2Renderer,       # spectral setup
                 T2_2_3Renderer,       # antenna setup
                 T2_2_4Renderer,       # sky setup
                 T2_3_1MRenderer,      # QA2 calibration
                 T2_3_2MRenderer,      # QA2 line finding
                 T2_3_3MRenderer,      # QA2 flagging
                 T2_3_4MRenderer,      # QA2 imaging
                 T2_3MDetailsRenderer, # QA2 details pages
                 T2_4MRenderer,        # task tree
                 T2_4MDetailsRenderer] # task details

    @staticmethod
    def copy_resources(context):
        outdir = os.path.join(context.report_dir, 'resources')
        
        # shutil.copytree complains if the output directory exists
        if os.path.exists(outdir):
            shutil.rmtree(outdir)
        
        # copy all uncompressed non-python resources to output directory
        src = os.path.dirname(resources.__file__)
        dst = outdir
        ignore_fn = shutil.ignore_patterns('*.zip','*.py','*.pyc','CVS*')
        shutil.copytree(src, 
                        dst, 
                        symlinks=False, 
                        ignore=ignore_fn)

        # unzip fancybox to output directory
        infile = os.path.join(src, 'fancybox.zip')
        z = zipfile.ZipFile(infile, 'r')        
        z.extractall(outdir)
    
    @staticmethod
    def render(context, hardcopy=True):
        WebLogGenerator.copy_resources(context)

        for renderer in WebLogGenerator.renderers:
            try:
                renderer.render(context, hardcopy)
            except Exception as e:
                LOG.exception('Error generating weblog: %s', e)


class LogCopier(object):
    """
    LogCopier copies and handles the CASA logs so that they may be referenced
    by the pipeline web logs. 
    
    Capturing the CASA log gives us a few problems:
    The previous log is renamed upon starting a new session. To be reliably
    referenced from the web log, we must give it an immutable name and copy it
    to a safe location within the web log directory.

    The user may want to view the web log at any time during a pipeline
    session. To avoid broken links to the CASA log, the log should be copied
    across to the web log location at the end of each task.

    Pipeline sessions may be interrupted and restored, resulting in multiple
    CASA logs for such sessions. These logs must be consolidated into one file
    alongside any previous log information.

    Adding HTML tags such as '<pre>' and HTML anchors causes the CASA log
    reader to render such entries as empty entries at the bottom of the log.
    The result is that you must scroll up to find the last log entry. To
    prevent this, we need to output anchors as CASA log comments, possibly
    timestamps, and then use javascript to navigate to the log location.
    """
    
    # Thanks to the unique timestamps in the CASA log, the implementation
    # turns out to be quite simple. Is a class overkill?
    
    @staticmethod
    def copy(context):
        output_dir = context.report_dir
        output_file = os.path.join(output_dir, 'casapy.log')

        existing_entries = []
        if os.path.exists(output_file):
            with open(output_file, 'r') as weblog:
                existing_entries.extend(weblog.readlines())
                    
        # read existing log, appending any non-duplicate entries to our casapy
        # web log. This is Python 2.6 so we can't define the context managers
        # on the same line
        with open(output_file, 'a') as weblog:
            with open(casatools.log.logfile(), 'r') as casalog:
                to_append = [entry for entry in casalog 
                             if entry not in existing_entries]
            weblog.writelines(to_append)


# renderer_map holds the mapping of tasks to result renderers for
# task-dependent weblog sections. This lets us write customised output for
# each task type.
renderer_map = {
    T2_3MDetailsRenderer : {
    },
    T2_4MDetailsRenderer : {
        tasks.Bandpass       : T2_4MDetailsBandpassRenderer(),
        tasks.SDReduction    : T2_4MDetailsDefaultRenderer('t2-4-singledish.html'),
        tasks.CleanList      : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_cleanlist.html'),
        tasks.GcorFluxscale  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_gfluxscale.html'),
        tasks.MakeCleanList  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_makecleanlist.html'),
        tasks.NormaliseFlux  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_normflux.html'),
        tasks.RefAnt         : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_refant.html'),
        tasks.Setjy          : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_setjy.html'),
        tasks.Tsysflag       : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_tsysflag.html'),
        tasks.Wvrgcal        : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_wvrgcal.html'),
        tasks.Wvrgcalflag    : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_wvrgcalflag.html'),
    }    
}                
