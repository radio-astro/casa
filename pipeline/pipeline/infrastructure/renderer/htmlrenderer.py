from __future__ import absolute_import, division
import collections
import contextlib
import datetime
import json
import math
import operator
import os
import pydoc
import re
import shutil
import zipfile

import casadef
import mako
import numpy as np

import pipeline as pipeline
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.casataskdict as casataskdict
import pipeline.infrastructure.displays.clean as clean
import pipeline.infrastructure.displays.flagging as flagging
import pipeline.infrastructure.displays.image as image
import pipeline.infrastructure.displays.summary as summary
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.sharedrenderer as sharedrenderer
import pipeline.infrastructure.renderer.rendererutils as rendererutils
from pipeline.infrastructure.renderer.templates import resources
from . import logger
from . import qaadapter
from .. import utils
from . import weblog
import pipeline.hif as hif
import pipeline.hsd as hsd
import pipeline.hifa as hifa
import pipeline.hifv as hifv
import pipeline.hif.tasks.applycal.renderer as applycal_renderer

LOG = infrastructure.get_logger(__name__)


def get_task_description(result_obj):
    if not isinstance(result_obj, (list, basetask.ResultsList)):
        return get_task_description([result_obj, ])

    if len(result_obj) is 0:
        msg = 'Cannot get description for zero-length results list'
        LOG.error(msg)
        return msg
 
    task_cls = getattr(result_obj[0], 'task', None)
    if task_cls is None:
        results_cls = result_obj[0].__class__.__name__
        msg = 'No task registered on results of type %s' % results_cls
        LOG.warning(msg)
        return msg

    # TODO remove once refactor is complete
    renderer = renderer_map[T2_4MDetailsRenderer].get(task_cls, None)
    if renderer:
        description = getattr(renderer, 'description', None)
    else:
        LOG.error('No renderer registered for task %s' % task_cls.__name__)
        raise TypeError()

    if description is None:
        description = _get_task_description_for_class(task_cls)    

    d = {'description' : description,
         'task_name'   : get_task_name(result_obj, include_stage=False),
         'stage'       : get_stage_number(result_obj)}
    return '{stage}. <strong>{task_name}</strong>: {description}'.format(**d)

def _get_task_description_for_class(task_cls):
    if task_cls is hif.tasks.Atmflag:
        return 'Flag on atmospheric transmission'

    if task_cls is hif.tasks.Clean:
        return 'Produce a cleaned image'

    if task_cls is hif.tasks.CleanList:
        return 'Calculate clean products'

    if task_cls is hif.tasks.ExportData:
        return 'Export data products'

    if task_cls is hif.tasks.Rawflagchans:
        return 'Flag channels in raw data'

    if task_cls in (hsd.tasks.SDImportData, hsd.tasks.SDImportData2):
        return 'Register measurement sets with the pipeline'

    if task_cls is hifa.tasks.Linpolcal:
        return 'Linear polarization calibration'
        
    if task_cls is hifa.tasks.Fluxdb:
        return 'Flux calibration database'

    if task_cls is hif.tasks.MakeCleanList:
        return 'Compile a list of cleaned images to be calculated'

    if task_cls is hif.tasks.NormaliseFlux:
        return 'Calculate mean fluxes of calibrators'

    if task_cls is hsd.tasks.SDMsToScantable:
        return 'Convert MS to Scantables'

    if task_cls is hsd.tasks.SDExportData:
        return 'Single-dish SDExportData'

    if task_cls is hsd.tasks.SDInspectData:
        return 'Inspect data'

    if task_cls is hsd.tasks.SDCalTsys:
        return 'Generate Tsys calibration table'

    if task_cls is hsd.tasks.SDCalSky:
        return 'Generate Sky calibration table'

    if task_cls is hsd.tasks.SDApplyCal:
        return 'Apply calibration tables'

    if task_cls is hsd.tasks.SDImaging:
        return 'Image single dish data'

    if task_cls is hsd.tasks.SDBaseline:
        return 'Generate Baseline tables and subtract spectral baseline'

    if task_cls is hsd.tasks.SDBLFlag:
        return 'Flag data by Tsys, weather, and statistics of spectra'
    
    if task_cls is hsd.tasks.SDFlagBaseline:
        return 'Iterative execution of baseline subtraction and flagging'

    if task_cls is hsd.tasks.SDPlotFlagBaseline:
        return 'Plot whole spectra with baseline fit and flag result'

    if task_cls is hsd.tasks.SDMsToScantable:
        return 'Convert MSs into Scantables'
        
    if task_cls is hifv.tasks.flagging.uncalspw.Uncalspw:
        return 'Flag spws that have no calibration'
    
    if task_cls is hifv.tasks.Applycals:
        return 'Apply all calibrations'

    if LOG.isEnabledFor(LOG.todo):
        LOG.todo('No task description for \'%s\'' % task_cls.__name__)
        return ('\'%s\' (developers should add a task description)'
                '' % task_cls.__name__)

    return ('\'%s\'' % task_cls.__name__)


def get_task_name(result_obj, include_stage=True):
    if not isinstance(result_obj, (list, basetask.ResultsList)):
        return get_task_name([result_obj, ])

    if len(result_obj) is 0:
        msg = 'Cannot get task name for zero-length results list'
        LOG.error(msg)
        return msg

    task_cls = result_obj[0].task
    if task_cls is None:
        results_cls = result_obj[0].__class__.__name__
        msg = 'No task registered on results of type %s' % results_cls
        LOG.warning(msg)
        return msg
    
    stage = '%s. ' % get_stage_number(result_obj) if include_stage else ''
    return '%s%s' % (stage,
                     casataskdict.classToCASATask.get(task_cls, task_cls.__name__))


def get_stage_number(result_obj):
    if not isinstance(result_obj, collections.Iterable):
        return get_stage_number([result_obj, ])

    if len(result_obj) is 0:
        msg = 'Cannot get stage number for zero-length results list'
        LOG.error(msg)
        return msg

    return result_obj[0].stage_number


def get_plot_dir(context, stage_number):
    stage_dir = os.path.join(context.report_dir, 'stage%d' % stage_number)
    plots_dir = os.path.join(stage_dir, 'plots')
    return plots_dir


class Session(object):
    def __init__(self, mses=None, name='Unnamed Session'):
        self.mses = [] if mses is None else mses
        self.name = name

    @staticmethod
    def get_sessions(context):
        d = collections.defaultdict(list)
        for ms in context.observing_run.measurement_sets:
            d[ms.session].append(ms)

        return [Session(v, k) for k, v in d.items()]


class RendererBase(object):
    """
    Base renderer class.
    """
    @classmethod
    def rerender(cls, context):
        LOG.todo('RendererBase: I think I\'m rerendering all pages!')
        return True

    @classmethod
    def get_path(cls, context):
        return os.path.join(context.report_dir, cls.output_file)

    @classmethod
    def get_file(cls, context):
        path = cls.get_path(context)
        file_obj = open(path, 'w')
        return contextlib.closing(file_obj)

    @classmethod
    def render(cls, context):
        # give the implementing class a chance to bypass rendering. This is
        # useful when the page has not changed, eg. MS description pages when
        # no subsequent ImportData has been performed
        path = cls.get_path(context)
        if os.path.exists(path) and not cls.rerender(context):
            return

        with cls.get_file(context) as fileobj:
            template = weblog.TEMPLATE_LOOKUP.get_template(cls.template)
            display_context = cls.get_display_context(context)
            fileobj.write(template.render(**display_context))


class T1_1Renderer(RendererBase):
    """
    T1-1 OUS Splash Page renderer
    """
    output_file = 't1-1.html'
    template = 't1-1.html'
    
    
    # named tuple holding values for each row in the main summary table
    TableRow = collections.namedtuple(
                'Tablerow', 
                'ousstatus_entity_id schedblock_id session '
                'execblock_id ms href filesize ' 
                'receivers '
                'num_antennas beamsize_min beamsize_max '
                'time_start time_end time_on_source '
                'baseline_min baseline_max baseline_rms')
    

    @staticmethod
    def get_display_context(context):
        obs_start = context.observing_run.start_datetime
        obs_end = context.observing_run.end_datetime

        project_uids = ', '.join(context.observing_run.project_ids)
        schedblock_uids = ', '.join(context.observing_run.schedblock_ids)
        execblock_uids = ', '.join(context.observing_run.execblock_ids)
        observers = ', '.join(context.observing_run.observers)

        array_names = set([ms.antenna_array.name
                           for ms in context.observing_run.measurement_sets])

        # pipeline execution start, end and duration
        exec_start = context.results[0].timestamps.start
        exec_end = context.results[-1].timestamps.end
        # remove unnecessary precision for execution duration
        dt = exec_end - exec_start
        exec_duration = datetime.timedelta(days=dt.days, seconds=dt.seconds)

#         qaresults = qaadapter.ResultsToQAAdapter(context.results)

        out_fmt = '%Y-%m-%d %H:%M:%S'
        
        
        #Observation Summary (formerly the T1-2 page)
        ms_summary_rows = []
        for ms in context.observing_run.measurement_sets:
            href = os.path.join('t2-1.html?ms=%s' % ms.basename)

            num_antennas = len(ms.antennas)
            # times should be passed as Python datetimes
            time_start = utils.get_epoch_as_datetime(ms.start_time)
            time_start = utils.format_datetime(time_start)
            time_end = utils.get_epoch_as_datetime(ms.end_time)
            time_end = utils.format_datetime(time_end)

            target_scans = [s for s in ms.scans if 'TARGET' in s.intents]
            time_on_source = utils.total_time_on_source(target_scans)
            time_on_source = utils.format_timedelta(time_on_source)
           
            baseline_min = ms.antenna_array.min_baseline.length
            baseline_max = ms.antenna_array.max_baseline.length
            
            # compile a list of primitive numbers representing the baseline 
            # lengths in metres..
            bls = [bl.length.to_units(measures.DistanceUnits.METRE)
                   for bl in ms.antenna_array.baselines]
            # .. so that we can calculate the RMS baseline length with 
            # consistent units
            baseline_rms = math.sqrt(sum(bl**2 for bl in bls)/len(bls))
            baseline_rms = measures.Distance(baseline_rms,
                                             units=measures.DistanceUnits.METRE)
 
            science_spws = ms.get_spectral_windows(science_windows_only=True)
            receivers = sorted(set(spw.band for spw in science_spws))

            row = T1_1Renderer.TableRow(ousstatus_entity_id=context.project_structure.ousstatus_entity_id,
                                        schedblock_id=ms.schedblock_id,
                                        session=ms.session,
                                        execblock_id=ms.execblock_id,
                                        ms=ms.basename,
                                        href=href,
                                        filesize=ms.filesize,
                                        receivers=receivers,                           
                                        num_antennas=num_antennas,
                                        beamsize_min='TODO',
                                        beamsize_max='TODO',
                                        time_start=time_start,
                                        time_end=time_end,
                                        time_on_source=time_on_source,
                                        baseline_min=baseline_min,
                                        baseline_max=baseline_max,
                                        baseline_rms=baseline_rms)
        
            ms_summary_rows.append(row)
        
        return {'pcontext'          : context,
                'casa_version'      : casadef.casa_version,
                'casa_revision'     : casadef.subversion_revision,
                'pipeline_revision' : pipeline.revision,
                'obs_start'         : obs_start.strftime(out_fmt),
                'obs_end'           : obs_end.strftime(out_fmt),
                'array_names'       : utils.commafy(array_names),
                'exec_start'        : exec_start.strftime(out_fmt),
                'exec_end'          : exec_end.strftime(out_fmt),
                'exec_duration'     : str(exec_duration),
                'project_uids'      : project_uids,
                'schedblock_uids'   : schedblock_uids,
                'execblock_uids'    : execblock_uids,
                'ous_uid'           : context.project_structure.ous_entity_id,
                'ousstatus_entity_id'     : context.project_structure.ousstatus_entity_id,
                'ppr_uid'           : None,
                'observers'         : observers,
#                 'qaadapter'        : qaresults,
                'ms_summary_rows'   : ms_summary_rows}


class T1_2Renderer(RendererBase):
    """
    T1-2 Observation Summary renderer
    """
    output_file = 't1-2.html'
    template = 't1-2.html'

    # named tuple holding values for each row in the main summary table
    TableRow = collections.namedtuple(
                'Tablerow', 
                'ms href filesize ' 
                'receivers '
                'num_antennas beamsize_min beamsize_max '
                'time_start time_end time_on_source '
                'baseline_min baseline_max baseline_rms')

    @staticmethod
    def get_display_context(context):
        ms_summary_rows = []
        for ms in context.observing_run.measurement_sets:
            href = os.path.join('t2-1.html?ms=%s' % ms.basename)

            num_antennas = len(ms.antennas)
            # times should be passed as Python datetimes
            time_start = utils.get_epoch_as_datetime(ms.start_time)
            time_start = utils.format_datetime(time_start)
            time_end = utils.get_epoch_as_datetime(ms.end_time)
            time_end = utils.format_datetime(time_end)

            target_scans = [s for s in ms.scans if 'TARGET' in s.intents]
            time_on_source = utils.total_time_on_source(target_scans)
            time_on_source = utils.format_timedelta(time_on_source)
           
            baseline_min = ms.antenna_array.min_baseline.length
            baseline_max = ms.antenna_array.max_baseline.length
            
            # compile a list of primitive numbers representing the baseline 
            # lengths in metres..
            bls = [bl.length.to_units(measures.DistanceUnits.METRE)
                   for bl in ms.antenna_array.baselines]
            # .. so that we can calculate the RMS baseline length with 
            # consistent units
            baseline_rms = math.sqrt(sum(bl**2 for bl in bls)/len(bls))
            baseline_rms = measures.Distance(baseline_rms,
                                             units=measures.DistanceUnits.METRE)
 
            science_spws = ms.get_spectral_windows(science_windows_only=True)
            receivers = sorted(set(spw.band for spw in science_spws))

            row = T1_2Renderer.TableRow(ms=ms.basename,
                                        href=href,
                                        filesize=ms.filesize,
                                        receivers=receivers,                           
                                        num_antennas=num_antennas,
                                        beamsize_min='TODO',
                                        beamsize_max='TODO',
                                        time_start=time_start,
                                        time_end=time_end,
                                        time_on_source=time_on_source,
                                        baseline_min=baseline_min,
                                        baseline_max=baseline_max,
                                        baseline_rms=baseline_rms)
        
            ms_summary_rows.append(row)
        
        return {'pcontext'        : context,
                'ms_summary_rows' : ms_summary_rows}


class T1_3MRenderer(RendererBase):
    """
    T1-3M renderer
    """
    output_file = 't1-3.html'
    template = 't1-3m.html'
    
    MsgTableRow = collections.namedtuple('MsgTableRow', 'stage task type message')
    
    @classmethod
    def get_display_context(cls, context):
        registry = qaadapter.registry
        # distribute results between topics
        registry.assign_to_topics(context.results)

        scores = {}
        tablerows = []
        results_list = []
        flagtables = {}
        for result in context.results:
            scores[result.stage_number] = result.qa.representative
            results_list = result
        
            qa_errors = cls._filter_qascores(results_list, -0.1, 0.1)
            tablerows.extend(cls._qascores_to_tablerows(qa_errors,
                                                            results_list,
                                                            'QA Error'))
        
            qa_warnings = cls._filter_qascores(results_list, 0.1, 0.5)
            tablerows.extend(cls._qascores_to_tablerows(qa_warnings,
                                                            results_list,
                                                            'QA Warning'))

            error_msgs = utils.get_logrecords(results_list, logging.ERROR)
            tablerows.extend(cls._logrecords_to_tablerows(error_msgs,
                                                              results_list,
                                                              'Error'))

            warning_msgs = utils.get_logrecords(results_list, logging.WARNING)
            tablerows.extend(cls._logrecords_to_tablerows(warning_msgs,
                                                              results_list,
                                                              'Warning'))
            
            if 'applycal' in get_task_description(result):
                try:
                    for resultitem in result:
                        ms_name = os.path.basename(resultitem.inputs['vis'])
                        ms = [i for i in context.observing_run.get_measurement_sets() if ms_name in i.name][0]
                        flagtable = {}
			for field in resultitem.flagsummary.keys():
			    #each field
			    intents = ','.join([f.intents for f in ms.get_fields(intent='BANDPASS,PHASE,AMPLITUDE,CHECK,TARGET') if field in f.name][0])
			    
			    flagsummary = resultitem.flagsummary[field]
			
			    fieldtable = {}
			    for k,v in flagsummary.iteritems():
				myname = v['name']
				myspw = v['spw']
				myant = v['antenna']
				antkeys = myant.keys()
				spwkey = myspw.keys()[0]
			    
				fieldtable.update({myname:{spwkey:myant}})
				
			    flagtable['Source name: '+ field + ', Intents: ' + intents] = fieldtable
			
			flagtables[ms_name] = flagtable
			
                except:
                    LOG.debug("No flag summary table available yet from applycal")
                 

        return {'pcontext' : context,
                'registry' : registry,
                'scores'   : scores,
                'tablerows': tablerows,
                'flagtables': flagtables}


    @classmethod
    def _filter_qascores(cls, results_list, lo, hi):
        qa_pool = results_list.qa.pool
        #print qa_pool
        with_score = [s for s in qa_pool if s.score not in ('', 'N/A', None)]
        return [s for s in with_score if s.score > lo and s.score <= hi]

    @classmethod
    def _create_tablerow(cls, results, message, msgtype):
        return cls.MsgTableRow(stage=results.stage_number,
                               task=get_task_name(results, False),
                               type=msgtype,
                               message=message)

    @classmethod
    def _qascores_to_tablerows(cls, qascores, results, msgtype='ERROR'):
        return [cls._create_tablerow(results, qascore.longmsg, msgtype)
                for qascore in qascores]

    @classmethod
    def _logrecords_to_tablerows(cls, records, results, msgtype='ERROR'):
        return [cls._create_tablerow(results, record.msg, msgtype)
                for record in records]

class T1_4MRenderer(RendererBase):
    """
    T1-4M renderer
    """
    output_file = 't1-4.html'
    # TODO get template at run-time
    template = 't1-4m.html'

    @staticmethod
    def get_display_context(context):
        scores = {}
        for result in context.results:
            scores[result.stage_number] = result.qa.representative
                
        return {'pcontext' : context,
                'results'  : context.results,
                'scores'   : scores}
        

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
    def get_file(cls, context, session, ms):
        ms_dir = os.path.join(context.report_dir, 
                              'session%s' % session.name,
                              ms.basename)
        if not os.path.exists(ms_dir):
            os.makedirs(ms_dir)
        filename = os.path.join(ms_dir, cls.output_file)
        file_obj = open(filename, 'w')
        return contextlib.closing(file_obj)

    @staticmethod
    def write_listobs(context, ms):
        listfile = os.path.join(context.report_dir, 
                                'session%s' % ms.session,
                                ms.basename,
                                'listobs.txt')

        if not os.path.exists(listfile):
            LOG.debug('Writing listobs output to %s' % listfile)
            task = infrastructure.casa_tasks.listobs(vis=ms.name,
                                                     listfile=listfile)
            task.execute(dry_run=False)

    @staticmethod
    def get_display_context(context, ms):
        T2_1DetailsRenderer.write_listobs(context, ms)
        
        inputs = summary.IntentVsTimeChart.Inputs(context, vis=ms.basename)
        task = summary.IntentVsTimeChart(inputs)
        intent_vs_time = task.plot()

        inputs = summary.FieldVsTimeChart.Inputs(context, vis=ms.basename)
        task = summary.FieldVsTimeChart(inputs)
        field_vs_time = task.plot()

        science_spws = ms.get_spectral_windows(science_windows_only=True)
        all_bands = sorted(set([spw.band for spw in ms.spectral_windows]))
        science_bands = sorted(set([spw.band for spw in science_spws]))
        
        science_sources = sorted(set([source.name for source in ms.sources 
                                      if 'TARGET' in source.intents]))

        calibrators = sorted(set([source.name for source in ms.sources 
                                  if 'TARGET' not in source.intents]))

        baseline_min = ms.antenna_array.min_baseline.length
        baseline_max = ms.antenna_array.max_baseline.length

        time_start = utils.get_epoch_as_datetime(ms.start_time)
        time_end = utils.get_epoch_as_datetime(ms.end_time)

        time_on_source = utils.total_time_on_source(ms.scans) 
        science_scans = [scan for scan in ms.scans if 'TARGET' in scan.intents]
        time_on_science = utils.total_time_on_source(science_scans)
        
#         dirname = os.path.join(context.report_dir, 
#                                'session%s' % ms.session,
#                                ms.basename)

        task = summary.WeatherChart(context, ms)
        weather_plot = task.plot()


        dirname = os.path.join('session%s' % ms.session,
                               ms.basename)
        
        return {
            'pcontext'        : context,
            'ms'              : ms,
            'science_sources' : utils.commafy(science_sources),
            'calibrators'     : utils.commafy(calibrators),
            'all_bands'       : utils.commafy(all_bands),
            'science_bands'   : utils.commafy(science_bands),
            'baseline_min'    : baseline_min,
            'baseline_max'    : baseline_max,
            'time_start'      : utils.format_datetime(time_start),
            'time_end'        : utils.format_datetime(time_end),
            'time_on_source'  : utils.format_timedelta(time_on_source),
            'time_on_science' : utils.format_timedelta(time_on_science),
            'intent_vs_time'  : intent_vs_time,
            'field_vs_time'   : field_vs_time,
            'dirname'         : dirname,
            'weather_plot'    : weather_plot
        }

    @classmethod
    def render(cls, context):
        for session in Session.get_sessions(context):
            for ms in session.mses:
                with cls.get_file(context, session, ms) as fileobj:
                    template = weblog.TEMPLATE_LOOKUP.get_template(cls.template)
                    display_context = cls.get_display_context(context, ms)
                    fileobj.write(template.render(**display_context))


class MetadataRendererBase(RendererBase):
    @classmethod
    def rerender(cls, context):
        # TODO: only rerender when a new ImportData result is queued
        if cls in DEBUG_CLASSES:
            LOG.warning('Always rerendering %s' % cls.__name__)
            return True
        return False


class T2_2_XRendererBase(object):
    """
    Base renderer for T2-2-X series of pages.
    """
    @classmethod
    def get_file(cls, filename):
        ms_dir = os.path.dirname(filename)

        if not os.path.exists(ms_dir):
            os.makedirs(ms_dir)

        file_obj = open(filename, 'w')
        return contextlib.closing(file_obj)

    @classmethod
    def get_filename(cls, context, ms):
        return os.path.join(context.report_dir,
                            'session%s' % ms.session,
                            ms.basename,
                            cls.output_file)

    @classmethod
    def render(cls, context):
        for ms in context.observing_run.measurement_sets:
            filename = cls.get_filename(context, ms)
            # now that the details pages are written per MS rather than having
            # tabs for each MS, we don't need to write them each time as
            # importdata will not affect their content.
            if not os.path.exists(filename):
                with cls.get_file(filename) as fileobj:
                    template = weblog.TEMPLATE_LOOKUP.get_template(cls.template)
                    display_context = cls.get_display_context(context, ms)
                    fileobj.write(template.render(**display_context))


class T2_2_1Renderer(T2_2_XRendererBase):
    """
    T2-2-1 renderer - spatial setup
    """
    output_file = 't2-2-1.html'
    template = 't2-2-1.html'

    @staticmethod
    def get_display_context(context, ms):
        mosaics = []
        for source in ms.sources:
            num_pointings = len([f for f in ms.fields 
                                 if f.source_id == source.id])
            if num_pointings > 1:
                task = summary.MosaicChart(context, ms, source)
                mosaics.append((source, task.plot()))

        return {'pcontext' : context,
                'ms'       : ms,
                'mosaics'  : mosaics}


class T2_2_2Renderer(T2_2_XRendererBase):
    """
    T2-2-2 renderer
    """
    output_file = 't2-2-2.html'
    template = 't2-2-2.html'

    @staticmethod
    def get_display_context(context, ms):
        return {'pcontext' : context,
                'ms'       : ms}


class T2_2_3Renderer(T2_2_XRendererBase):
    """
    T2-2-3 renderer
    """
    output_file = 't2-2-3.html'
    template = 't2-2-3.html'

    @staticmethod
    def get_display_context(context, ms):
        task = summary.PlotAntsChart2(context, ms)
        plot_ants = task.plot()

        dirname = os.path.join('session%s' % ms.session,
                               ms.basename)
        
        return {'pcontext'  : context,
                'plot_ants' : plot_ants,
                'ms'        : ms,
                'dirname'   : dirname}


class T2_2_4Renderer(T2_2_XRendererBase):
    """
    T2-2-4 renderer
    """
    output_file = 't2-2-4.html'
    template = 't2-2-4.html'

    @staticmethod
    def get_display_context(context, ms):
        task = summary.AzElChart(context, ms)
        azel_plot = task.plot()

        task = summary.ElVsTimeChart(context, ms)
        el_vs_time_plot = task.plot()

        dirname = os.path.join('session%s' % ms.session,
                               ms.basename)

        return {'pcontext'        : context,
                'ms'              : ms,
                'azel_plot'       : azel_plot,
                'el_vs_time_plot' : el_vs_time_plot,
                'dirname'         : dirname}


class T2_2_5Renderer(T2_2_XRendererBase):
    """
    T2-2-5 renderer - weather page
    """
    output_file = 't2-2-5.html'
    template = 't2-2-5.html'

    @staticmethod
    def get_display_context(context, ms):
        task = summary.WeatherChart(context, ms)
        weather_plot = task.plot()
        dirname = os.path.join('session%s' % ms.session,
                               ms.basename)

        return {'pcontext'     : context,
                'ms'           : ms,
                'weather_plot' : weather_plot,
                'dirname'      : dirname}


class T2_2_6Renderer(T2_2_XRendererBase):
    """
    T2-2-6 renderer - scans page
    """
    output_file = 't2-2-6.html'
    template = 't2-2-6.html'

    TableRow = collections.namedtuple(
        'TableRow', 
        'id time_start time_end duration intents fields spws'
    )

    @staticmethod
    def get_display_context(context, ms):
        tablerows = []
        for scan in ms.scans:
            scan_id = scan.id
            epoch_start = utils.get_epoch_as_datetime(scan.start_time)
            time_start = utils.format_datetime(epoch_start)
            epoch_end = utils.get_epoch_as_datetime(scan.end_time)
            time_end = utils.format_datetime(epoch_end)
            duration = utils.format_timedelta(scan.time_on_source)
            intents = sorted(scan.intents)
            fields = utils.commafy(sorted([f.name for f in scan.fields]))
            
            spw_ids = sorted([spw.id for spw in scan.spws])
            spws = ', '.join([str(spw_id) for spw_id in spw_ids])

            row = T2_2_6Renderer.TableRow(
                id=scan_id,
                time_start=time_start,
                time_end=time_end,
                duration=duration,
                intents=intents,
                fields=fields,
                spws=spws
            )

            tablerows.append(row)
            
        return {'pcontext'     : context,
                'ms'           : ms,
                'tablerows'    : tablerows}


class T2_3_XMBaseRenderer(RendererBase):
    # the filename to which output will be directed
    output_file = 'overrideme'
    # the template file for this renderer
    template = 'overrideme'

    MsgTableRow = collections.namedtuple('MsgTableRow', 'stage task type message')

    @classmethod
    def get_display_context(cls, context):
        topic = cls.get_topic()

        scores = {}
        for result in context.results:
            scores[result.stage_number] = result.qa.representative

        tablerows = []
        for list_of_results_lists in topic.results_by_type.values():
            if not list_of_results_lists:
                continue
            
            for results_list in list_of_results_lists:
                qa_errors = cls._filter_qascores(results_list, -0.1, 0.1)
                tablerows.extend(cls._qascores_to_tablerows(qa_errors,
                                                            results_list,
                                                            'QA Error'))
                    
                qa_warnings = cls._filter_qascores(results_list, 0.1, 0.5)
                tablerows.extend(cls._qascores_to_tablerows(qa_warnings,
                                                            results_list,
                                                            'QA Warning'))

                error_msgs = utils.get_logrecords(results_list, logging.ERROR)
                tablerows.extend(cls._logrecords_to_tablerows(error_msgs,
                                                              results_list,
                                                              'Error'))

                warning_msgs = utils.get_logrecords(results_list, logging.WARNING)
                tablerows.extend(cls._logrecords_to_tablerows(warning_msgs,
                                                              results_list,
                                                              'Warning'))
                
        return {'pcontext'  : context,
                'scores'    : scores,
                'tablerows' : tablerows,
                'topic'     : topic     }
                
    @classmethod
    def _filter_qascores(cls, results_list, lo, hi):
        qa_pool = results_list.qa.pool
        with_score = [s for s in qa_pool if s.score not in ('', 'N/A', None)]
        return [s for s in with_score if s.score > lo and s.score <= hi]

    @classmethod
    def _create_tablerow(cls, results, message, msgtype):
        return cls.MsgTableRow(stage=results.stage_number,
                               task=get_task_name(results, False),
                               type=msgtype,
                               message=message)

    @classmethod
    def _qascores_to_tablerows(cls, qascores, results, msgtype='ERROR'):
        return [cls._create_tablerow(results, qascore.longmsg, msgtype)
                for qascore in qascores]

    @classmethod
    def _logrecords_to_tablerows(cls, records, results, msgtype='ERROR'):
        return [cls._create_tablerow(results, record.msg, msgtype)
                for record in records]


class T2_3_1MRenderer(T2_3_XMBaseRenderer):
    """
    Renderer for T2-3-1M, the data set topic.
    """
    # the filename to which output will be directed
    output_file = 't2-3-1m.html'
    # the template file for this renderer
    template = 't2-3-1m.html'

    @classmethod
    def get_topic(cls):
        return qaadapter.registry.get_dataset_topic()        


class T2_3_2MRenderer(T2_3_XMBaseRenderer):
    """
    Renderer for T2-3-2M: the QA calibration section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-2m.html'
    # the template file for this renderer
    template = 't2-3-2m.html'

    @classmethod
    def get_topic(cls):
        return qaadapter.registry.get_calibration_topic()        


class T2_3_3MRenderer(T2_3_XMBaseRenderer):
    """
    Renderer for T2-3-3M: the QA flagging section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-3m.html'
    # the template file for this renderer
    template = 't2-3-3m.html'

    @classmethod
    def get_topic(cls):
        return qaadapter.registry.get_flagging_topic()        


class T2_3_4MRenderer(T2_3_XMBaseRenderer):
    """
    Renderer for T2-3-4M: the QA line finding section.
    """
    # the filename to which output will be directed
    output_file = 't2-3-4m.html'
    # the template file for this renderer
    template = 't2-3-4m.html'

    @classmethod
    def get_topic(cls):
        return qaadapter.registry.get_linefinding_topic()        


class T2_3_5MRenderer(T2_3_XMBaseRenderer):
    """
    Renderer for T2-3-5M: the imaging topic
    """
    # the filename to which output will be directed
    output_file = 't2-3-5m.html'
    # the template file for this renderer
    template = 't2-3-5m.html'

    @classmethod
    def get_topic(cls):
        return qaadapter.registry.get_imaging_topic()        


class T2_3_6MRenderer(T2_3_XMBaseRenderer):
    """
    Renderer for T2-3-6M: the miscellaneous topic
    """
    # the filename to which output will be directed
    output_file = 't2-3-6m.html'
    # the template file for this renderer
    template = 't2-3-6m.html'

    @classmethod
    def get_topic(cls):
        return qaadapter.registry.get_miscellaneous_topic()        


class T2_3MDetailsDefaultRenderer(object):
    def __init__(self, template='t2-3m_details.html', always_rerender=False):
        self.template = template
        self.always_rerender = always_rerender
        
    def get_display_context(self, context, result):
        return {'pcontext' : context,
                'result'   : result}

    def render(self, context, result):
        display_context = self.get_display_context(context, result)
        t = weblog.TEMPLATE_LOOKUP.get_template(self.template)
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

    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :rtype: a file object
    """
    @classmethod
    def get_file(cls, context, result):
        # construct the relative filename, eg. 'stageX/t2-3m_details.html'
        path = cls.get_path(context, result)

        # to avoid any subsequent file not found errors, create the directory
        # if a hard copy is requested and the directory is missing
        stage_dir = os.path.dirname(path)
        if not os.path.exists(stage_dir):
            os.makedirs(stage_dir)
        
        # create a file object that writes to a file if a hard copy is 
        # requested, otherwise return a file object that flushes to stdout
        file_obj = open(path, 'w')
        
        # return the file object wrapped in a context manager, so we can use
        # it with the autoclosing 'with fileobj as f:' construct
        return contextlib.closing(file_obj)

    """
    Get the path to which the template will be written.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :rtype: string
    """
    @classmethod
    def get_path(cls, context, result):
        # HTML output will be written to the directory 'stageX' 
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, stage)

        # construct the relative filename, eg. 'stageX/t2-4m_details.html'
        return os.path.join(stage_dir, cls.output_file)

    """
    Render the detailed QA perspective of each Results in the given context.
    
    This renderer creates detailed T2_3M output for each Results. Each Results
    in the context is passed to a specialised renderer, which generates
    custom output and plots for the Result in question.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    """
    @classmethod
    def render(cls, context):
        # get the map of t2_3m renderers from the dictionary
        t2_3m_renderers = renderer_map[T2_3MDetailsRenderer]
        
        # for each result accepted and stored in the context..
        for result in context.results:
            # we only handle lists of results, so wrap single objects in a
            # list if necessary
            if not isinstance(result, collections.Iterable):
                l = basetask.ResultsList()
                l.append(result)
                l.timestamps = result.timestamps
                l.inputs = result.inputs
                l.stage_number = result.stage_number
                result = l
            task = result[0].task

            # find the renderer appropriate to the task..
            renderer = t2_3m_renderers.get(task, cls._default_renderer)
            LOG.trace('Using %s to render %s result' % (
                renderer.__class__.__name__, task.__name__))
            
            # details pages do not need to be updated once written
            path = cls.get_path(context, result)
            force_rerender = getattr(renderer, 'always_rerender', False)
            if os.path.exists(path) and not force_rerender:
                continue
            
            # .. get the file object to which we'll render the result
            with cls.get_file(context, result) as fileobj:
                # .. and write the renderer's interpretation of this result to
                # the file object  
                fileobj.write(renderer.render(context, result))


class T2_3MDetailsWvrgcalflagRenderer(T2_3MDetailsDefaultRenderer):
    """
    T2_43DetailsBandpassRenderer generates the QA output specific to the
    Wvrgcalflag task.
    """
    
    def __init__(self, template='t2-3m_details-wvrgcalflag.html',
                 always_rerender=False):
        # set the name of our specialised Mako template via the superclass
        # constructor 
        super(T2_3MDetailsWvrgcalflagRenderer, self).__init__(template,
                                                              always_rerender)

    """
    Get the Mako context appropriate to the results created by a Bandpass
    task.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param results: the Wvrgcalflag results to describe
    :type results: 
        :class:`~pipeline.infrastructure.tasks.wvrgcal.resultobjects.WvrgcalflagResults`
    :rtype a dictionary that can be passed to the matching bandpass Mako 
        template
    """
    def get_display_context(self, context, results):
        # get the standard Mako context from the superclass implementation 
        super_cls = super(T2_3MDetailsWvrgcalflagRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        plots_dir = os.path.join(context.report_dir, 
                                 'stage%d' % results.stage_number)
        if not os.path.exists(plots_dir):
            os.mkdir(plots_dir)

        plotter = image.ImageDisplay()
        plots = []
        for result in results:
            if result.qa_wvr.view:
                plot = plotter.plot(context, result.qa_wvr, reportdir=plots_dir, 
                                    prefix='qa', change='WVR')
                plots.append(plot)
    
        # Group the Plots by axes and plot types; each logical grouping will
        # be contained in a PlotGroup  
        plot_groups = logger.PlotGroup.create_plot_groups(plots)
        for plot_group in plot_groups:
            # Write the thumbnail pages for each plot grouping to disk 
            renderer = sharedrenderer.PlotGroupRenderer(context, results, plot_group, 'qa')
            plot_group.filename = renderer.basename
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'plot_groups' : plot_groups})
        return ctx


class T2_3MDetailsBandpassRenderer(T2_3MDetailsDefaultRenderer):
    """
    T2_43DetailsBandpassRenderer generates the QA output specific to the
    bandpass task.
    """
    
    def __init__(self, template='t2-3m_details-bandpass.html',
                 always_rerender=False):
        # set the name of our specialised Mako template via the superclass
        # constructor 
        super(T2_3MDetailsBandpassRenderer, self).__init__(template,
                                                           always_rerender)

    def get_display_context(self, context, results):
        """
        Get the Mako context appropriate to the results created by a Bandpass
        task.

        :param context: the pipeline Context
        :type context: :class:`~pipeline.infrastructure.launcher.Context`
        :param results: the bandpass results to describe
        :type results:
            :class:`~pipeline.infrastructure.tasks.bandpass.common.BandpassResults`
        :rtype a dictionary that can be passed to the matching bandpass Mako
            template
        """
        # get the standard Mako context from the superclass implementation
        super_cls = super(T2_3MDetailsBandpassRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        plots = []
        flagged = []
        num_flagged_feeds = 0
        for result in results:
            # return early if there are no QA results
            if not result.qa:
                return ctx
            adapter = qaadapter.QABandpassAdapter(context, result)
            plots.append(adapter.amplitude_plots)
            plots.append(adapter.phase_plots)
            
            for antenna, feeds in adapter.flagged_feeds.items():
                flagged.append((os.path.basename(result.inputs['vis']),
                                antenna.identifier,
                                ', '.join(feeds)))
                num_flagged_feeds += len(feeds)

        # Group the Plots by axes and plot types; each logical grouping will
        # be contained in a PlotGroup  
        plot_groups = logger.PlotGroup.create_plot_groups(plots)
        # Write the thumbnail pages for each plot grouping to disk 
        for plot_group in plot_groups:
            renderer = sharedrenderer.QAPlotRenderer(context, results, plot_group, 'qa')
            plot_group.filename = renderer.basename 
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'plot_groups'       : plot_groups,
                    'flagged'           : flagged,
                    'num_flagged_feeds' : num_flagged_feeds})

        return ctx


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
    def __init__(self, template='t2-4m_details-generic.html', 
                 always_rerender=False):
        self.template = template
        self.always_rerender = always_rerender

    def get_display_context(self, context, result):
        mako_context = {'pcontext' : context,
                        'result'   : result,
                        'stagelog' : self._get_stagelog(context, result),
                        'taskhelp' : self._get_help(context, result),
                        'dirname'  : 'stage%s' % result.stage_number}
        self.update_mako_context(mako_context, context, result)
        return mako_context

    def update_mako_context(self, mako_context, pipeline_context, result):
        LOG.trace('No-op update_mako_context for %s', self.__class__.__name__)
        
    def render(self, context, result):
        display_context = self.get_display_context(context, result)
        # TODO remove fallback access once all templates are converted 
        uri = getattr(self, 'uri', None)
        if uri is None:
            uri = self.template
        template = weblog.TEMPLATE_LOOKUP.get_template(uri)
        return template.render(**display_context)

    def _get_stagelog(self, context, result):
        """
        Read in the CASA log extracts from the file in the stage directory.
        """
        stagelog_path = os.path.join(context.report_dir,
                                     'stage%s' % result.stage_number,
                                     'casapy.log')

        if not os.path.exists(stagelog_path):
            return None
        
        with open(stagelog_path, 'r') as f:
            return ''.join([l.expandtabs() for l in f.readlines()])

    def _get_help(self, context, result):
        try:
            # get hif-prefixed taskname from the result from which we can
            # retrieve the XML documentation, otherwise fall back to the
            # Python class documentation              
            taskname = getattr(result, 'taskname', result[0].task)

            obj, _ = pydoc.resolve(taskname, forceload=0)
            page = pydoc.render_doc(obj)
            return '<pre>%s</pre>' % re.sub('\x08.', '', page)
        except Exception:
            return None


class CleanPlotsRenderer(object):
    template = 'cleanplots.html'

    def __init__(self, context, result, plots_dict, field, spw, pol):
        self.context = context
        self.result = result
        self.plots_dict = plots_dict
        self.field = field
        self.spw = spw
        self.pol = pol
        
    @property
    def dirname(self):
        stage = 'stage%s' % self.result.stage_number
        return os.path.join(self.context.report_dir, stage)
    
    @property
    def filename(self):
        filename = 'field%s-spw%s-pol%s-cleanplots.html' % (
          self.field, 
          self.spw,
          self.pol)
        filename = filenamer.sanitize(filename)
        return filename
    
    @property
    def path(self):
        return os.path.join(self.dirname, self.filename)
    
    def get_file(self):
        if not os.path.exists(self.dirname):
            os.makedirs(self.dirname)
        file_obj = open(self.path, 'w') 
        return contextlib.closing(file_obj)
    
    def _get_display_context(self):
        return {'pcontext'   : self.context,
                'result'     : self.result,
                'plots_dict' : self.plots_dict,
                'dirname'    : self.dirname,
                'field'      : self.field,
                'spw'        : self.spw}

    def render(self):
        display_context = self._get_display_context()
        t = weblog.TEMPLATE_LOOKUP.get_template(self.template)
        return t.render(**display_context)


class T2_4MDetailsRawflagchansRenderer(T2_4MDetailsDefaultRenderer):
    '''
    Renders detailed HTML output for the Rawflagchans task.
    '''
    def __init__(self, template='t2-4m_details-hif_rawflagchans.html',
            always_rerender=False):
        super(T2_4MDetailsRawflagchansRenderer, self).__init__(template,
                always_rerender)

    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailsRawflagchansRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        htmlreports = self.get_htmlreports(context, results)
        
        ctx.update({'htmlreports' : htmlreports})
        return ctx

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




#-----------------------------------------------------------------------

class T2_4MDetailsfinalcalsRenderer(T2_4MDetailsDefaultRenderer):
    def __init__(self, template='t2-4m_details-hifv_finalcals.html', 
                 always_rerender=False):
        super(T2_4MDetailsfinalcalsRenderer, self).__init__(template,
                                                          always_rerender)
    
    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailsfinalcalsRenderer, self)
        ctx = super_cls.get_display_context(context, results)
        
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        summary_plots = {}
        finaldelay_subpages = {}
        phasegain_subpages = {}
        bpsolamp_subpages = {}
        bpsolphase_subpages = {}
        bpsolphaseshort_subpages = {}
        finalamptimecal_subpages = {}
        finalampfreqcal_subpages = {}
        finalphasegaincal_subpages = {}
        
        for result in results:
            
            plotter = finalcalsdisplay.finalcalsSummaryChart(context, result)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots
            
            # generate testdelay plots and JSON file
            plotter = finalcalsdisplay.finalDelaysPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'finaldelays')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                finaldelay_subpages[ms] = renderer.filename
            
                
            # generate phase Gain plots and JSON file
            plotter = finalcalsdisplay.finalphaseGainPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'phasegain')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                phasegain_subpages[ms] = renderer.filename
                
            # generate amp bandpass solution plots and JSON file
            plotter = finalcalsdisplay.finalbpSolAmpPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'bpsolamp')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolamp_subpages[ms] = renderer.filename
                
            # generate phase bandpass solution plots and JSON file
            plotter = finalcalsdisplay.finalbpSolPhasePerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'bpsolphase')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolphase_subpages[ms] = renderer.filename
                
            # generate phase short bandpass solution plots and JSON file
            plotter = finalcalsdisplay.finalbpSolPhaseShortPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'bpsolphaseshort')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolphaseshort_subpages[ms] = renderer.filename
            
            # generate final amp time cal solution plots and JSON file
            plotter = finalcalsdisplay.finalAmpTimeCalPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'finalamptimecal')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                finalamptimecal_subpages[ms] = renderer.filename
                
            # generate final amp freq cal solution plots and JSON file
            plotter = finalcalsdisplay.finalAmpFreqCalPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'finalampfreqcal')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                finalampfreqcal_subpages[ms] = renderer.filename
        
            # generate final phase gain cal solution plots and JSON file
            plotter = finalcalsdisplay.finalPhaseGainCalPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'finalphasegaincal')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                finalphasegaincal_subpages[ms] = renderer.filename
        
        ctx.update({'summary_plots'   : summary_plots,
                    'finaldelay_subpages' : finaldelay_subpages,
                    'phasegain_subpages' : phasegain_subpages,
                    'bpsolamp_subpages'  : bpsolamp_subpages,
                    'bpsolphase_subpages' : bpsolphase_subpages,
                    'bpsolphaseshort_subpages' : bpsolphaseshort_subpages,
                    'finalamptimecal_subpages' : finalamptimecal_subpages,
                    'finalampfreqcal_subpages' : finalampfreqcal_subpages,
                    'finalphasegaincal_subpages' : finalphasegaincal_subpages,
                    'dirname'         : weblog_dir})
                
        return ctx


class T2_4MDetailsHeuristicFlagRenderer(T2_4MDetailsDefaultRenderer):
    def __init__(self, template='t2-4m_details-hifv_semifinalbpdcals.html', 
                 always_rerender=False):
        super(T2_4MDetailsHeuristicFlagRenderer, self).__init__(template,
                                                          always_rerender)
    
    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailsHeuristicFlagRenderer, self)
        ctx = super_cls.get_display_context(context, results)
        
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        testsummary_plots = {}
        testdelay_subpages = {}
        testampgain_subpages = {}
        testphasegain_subpages = {}
        testbpsolamp_subpages = {}
        testbpsolphase_subpages = {}
        
        semifinalsummary_plots1 = {}
        semifinaldelay_subpages1 = {}
        semifinalampgain_subpages1 = {}
        semifinalphasegain_subpages1 = {}
        semifinalbpsolamp_subpages1 = {}
        bpsolphase_subpages1 = {}
        
        semifinalsummary_plots2 = {}
        semifinaldelay_subpages2 = {}
        semifinalampgain_subpages2 = {}
        semifinalphasegain_subpages2 = {}
        semifinalbpsolamp_subpages2 = {}
        bpsolphase_subpages2 = {}
        
        for result in results:
            
            plotter = semifinalBPdcalsdisplay.semifinalBPdcalsSummaryChart(context, result, suffix='1')
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots
            
            # generate testdelay plots and JSON file
            plotter = semifinalBPdcalsdisplay.DelaysPerAntennaChart(context, result, suffix='1')
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'semifinalcals_plots.html', 'delays')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                delay_subpages[ms] = renderer.filename
            
                
            # generate phase Gain plots and JSON file
            plotter = semifinalBPdcalsdisplay.semifinalphaseGainPerAntennaChart(context, result, suffix='1')
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'semifinalcals_plots.html', 'phasegain')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                phasegain_subpages[ms] = renderer.filename
                
            # generate amp bandpass solution plots and JSON file
            plotter = semifinalBPdcalsdisplay.semifinalbpSolAmpPerAntennaChart(context, result, suffix='1')
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'semifinalcals_plots.html', 'bpsolamp')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolamp_subpages[ms] = renderer.filename
                
            # generate phase bandpass solution plots and JSON file
            plotter = semifinalBPdcalsdisplay.semifinalbpSolPhasePerAntennaChart(context, result, suffix='1')
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'semifinalcals_plots.html', 'bpsolphase')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolphase_subpages[ms] = renderer.filename
        
        ctx.update({'summary_plots'   : summary_plots,
                    'delay_subpages' : delay_subpages,
                    'phasegain_subpages' : phasegain_subpages,
                    'bpsolamp_subpages'  : bpsolamp_subpages,
                    'bpsolphase_subpages' : bpsolphase_subpages,
                    'dirname'         : weblog_dir})
                
        return ctx













class VLASubPlotRenderer(object):
    #template = 'testdelays_plots.html'
    
    def __init__(self, context, result, plots, json_path, template, filename_prefix):
        self.context = context
        self.result = result
        self.plots = plots
        self.ms = os.path.basename(self.result.inputs['vis'])
        self.template = template
        self.filename_prefix=filename_prefix

        if os.path.exists(json_path):
            with open(json_path, 'r') as json_file:
                self.json = json_file.readlines()[0]
        else:
            self.json = '{}'
            
    def _get_display_context(self):
        return {'pcontext'   : self.context,
                'result'     : self.result,
                'plots'      : self.plots,
                'dirname'    : self.dirname,
                'json'       : self.json}

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


#----------------------------------------------------------------------



class T2_4MDetailsCleanRenderer(T2_4MDetailsDefaultRenderer):
    def __init__(self, template='t2-4m_details-hif_cleanlist.html',
                 always_rerender=False):
        super(T2_4MDetailsCleanRenderer, self).__init__(template,
                                                        always_rerender)

    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailsCleanRenderer, self)
        ctx = super_cls.get_display_context(context, results)

        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        # There is only ever one CleanListResult in the ResultsList as it
        # operates over multiple measurement sets, so we can set the result to
        # the first item in the list

        # Get results info
        info_dict = {}

        if results[0]:
            cresults = results[0].results

            for r in cresults:
                if r.empty():
                    continue
                if not r.iterations:
                    continue

                maxiter = max(r.iterations.keys())

                with casatools.ImageReader(r.iterations[maxiter]['image']) as image:
                   info = image.miscinfo()
                   spw = pol = field = None
                   if info.has_key('spw'): 
                       spw = info['spw']
                   if info.has_key('field'):
                       field = '%s (%s)' % (info['field'], r.intent)

                   coordsys = image.coordsys()
                   coord_names = np.array(coordsys.names())
                   coord_refs = coordsys.referencevalue(format='s')
                   pol = coord_refs['string'][coord_names=='Stokes'][0]
                   frequency = coord_refs['string'][coord_names=='Frequency'][0]
                   frequency = casatools.quanta.convert(frequency, 'GHz')
                   info_dict[(field,spw,pol,'frequency')] = frequency

                   info_dict[(field,spw,pol,'image name')] = image.name(strippath=True)
                   stats = image.statistics(robust=False)
                   info_dict[(field,spw,pol,'max')] = stats.get('max')[0]
                   beam = image.restoringbeam()
                   major = casatools.quanta.convert(beam['major'], 'arcsec')
                   info_dict[(field,spw,pol,'beam major')] = major
                   minor = casatools.quanta.convert(beam['minor'], 'arcsec')
                   info_dict[(field,spw,pol,'beam minor')] = minor
                   pa = casatools.quanta.convert(beam['positionangle'], 'deg')
                   info_dict[(field,spw,pol,'beam pa')] = pa
                   info_dict[(field,spw,pol,'brightness unit')] = image.brightnessunit()

                   stats = image.statistics(robust=False)
                   info_dict[(field,spw,pol,'image rms')] = stats.get('rms')[0]

                with casatools.ImageReader(r.iterations[maxiter]['residual']) as residual:
                   stats = image.statistics(robust=False)
                   info_dict[(field,spw,pol,'residual rms')] = stats.get('rms')[0]

        # Make the plots
        plotter = clean.CleanSummary(context, results[0])
        plots = plotter.plot()        

        fields = sorted(set([p.parameters['field'] for p in plots]))
        spws = sorted(set([p.parameters['spw'] for p in plots]))
        iterations = sorted(set([p.parameters['iter'] for p in plots]))
        types = sorted(set([p.parameters['type'] for p in plots]))
        
        iteration_dim = lambda : collections.defaultdict(dict)
        spw_dim = lambda : collections.defaultdict(iteration_dim)
        plots_dict = collections.defaultdict(spw_dim)
        for field in fields:
            for spw in spws:
                for iteration in iterations:
                    for t in types:
                        matching = [p for p in plots
                                    if p.parameters['field'] == field
                                    and p.parameters['spw'] == spw
                                    and p.parameters['iter'] == iteration
                                    and p.parameters['type'] == t]
                        if matching:
                            plots_dict[field][spw][iteration][t] = matching[0]
                    
        ctx.update({'fields'     : fields,
                    'spws'       : spws,
                    'iterations' : iterations,
                    'plots'      : plots,
                    'plots_dict' : plots_dict,
                    'info_dict'  : info_dict,
                    'dirname'    : weblog_dir})

#         import pprint
#         d = {}
#         d.update(plots_dict)
#         pprint.pprint(d)

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

    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :rtype: a file object
    """
    @classmethod
    def get_file(cls, context, result):
        # construct the relative filename, eg. 'stageX/t2-4m_details.html'
        path = cls.get_path(context, result)

        # to avoid any subsequent file not found errors, create the directory
        # if a hard copy is requested and the directory is missing
        stage_dir = os.path.dirname(path)
        if not os.path.exists(stage_dir):
            os.makedirs(stage_dir)
        
        # create a file object that writes to a file if a hard copy is 
        # requested, otherwise return a file object that flushes to stdout
        file_obj = open(path, 'w')
        
        # return the file object wrapped in a context manager, so we can use
        # it with the autoclosing 'with fileobj as f:' construct
        return contextlib.closing(file_obj)

    """
    Get the path to which the template will be written.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :rtype: string
    """
    @classmethod
    def get_path(cls, context, result):
        # HTML output will be written to the directory 'stageX' 
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, stage)

        # construct the relative filename, eg. 'stageX/t2-4m_details.html'
        return os.path.join(stage_dir, cls.output_file)

    """
    Render the detailed task-centric view of each Results in the given
    context.
    
    This renderer creates detailed, T2_4M output for each Results. Each
    Results in the context is passed to a specialised renderer, which 
    generates customised output and plots for the Result in question.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    """
    @classmethod
    def render(cls, context):
        # get the map of t2_4m renderers from the dictionary
        t2_4m_renderers = renderer_map[T2_4MDetailsRenderer]

        # for each result accepted and stored in the context..
        for result in context.results:
            # we only handle lists of results, so wrap single objects in a
            # list if necessary
            if not isinstance(result, collections.Iterable):
                l = basetask.ResultsList()
                l.append(result)
                l.timestamps = result.timestamps
                l.stage_number = result.stage_number
                l.inputs = result.inputs
                if hasattr(result, 'taskname'):
                    l.taskname = result.taskname
                result = l
            task = result[0].task
            
            # find the renderer appropriate to the task..
            renderer = t2_4m_renderers.get(task, cls._default_renderer)
            LOG.trace('Using %s to render %s result' % (
                renderer.__class__.__name__, task.__name__))

            # details pages do not need to be updated once written unless the
            # renderer specifies that an update is required
            path = cls.get_path(context, result)
            force_rerender = getattr(renderer, 'always_rerender', False)
            debug_cls = renderer.__class__ in DEBUG_CLASSES
            force_rerender = force_rerender or debug_cls
            if force_rerender:
                LOG.trace('Forcing rerendering for %s' % renderer.__class__.__name__)
            if os.path.exists(path) and not force_rerender:
                continue
            
            # .. get the file object to which we'll render the result
            with cls.get_file(context, result) as fileobj:
                # .. and write the renderer's interpretation of this result to
                # the file object  
                try:
                    fileobj.write(renderer.render(context, result))
                except:
                    LOG.warning('Template generation failed for '
                                '%s.' % cls.__name__)
                    LOG.debug(mako.exceptions.text_error_template().render())
                    fileobj.write(mako.exceptions.html_error_template().render())


class WebLogGenerator(object):
    renderers = [T1_1Renderer,         # OUS splash page
                 T1_2Renderer,         # observation summary
                 T1_3MRenderer,        # by topic page
                 T2_1Renderer,         # session tree
                 T2_1DetailsRenderer,  # session details
                 T2_2_1Renderer,       # spatial setup
                 T2_2_2Renderer,       # spectral setup
                 T2_2_3Renderer,       # antenna setup
                 T2_2_4Renderer,       # sky setup
                 T2_2_5Renderer,       # weather
                 T2_2_6Renderer,       # scans
                 T2_3_1MRenderer,      # data set topic
                 T2_3_2MRenderer,      # calibration topic
                 T2_3_3MRenderer,      # flagging topic
                 # disable unused line finding topic for July 2014 release
#                  T2_3_4MRenderer,      # line finding topic
                 T2_3_5MRenderer,      # imaging topic
                 T2_3_6MRenderer,      # miscellaneous topic
                 T2_3MDetailsRenderer, # QA details pages
                 T2_4MRenderer,        # task tree
                 T2_4MDetailsRenderer, # task details
                 # some summary renderers are placed last for access to scores
                 T1_4MRenderer]        # task summary

    @staticmethod
    def copy_resources(context):
        outdir = os.path.join(context.report_dir, 'resources')
        
        # shutil.copytree complains if the output directory exists
        if os.path.exists(outdir):
            shutil.rmtree(outdir)
        
        # copy all uncompressed non-python resources to output directory
        src = os.path.dirname(resources.__file__)
        dst = outdir
        ignore_fn = shutil.ignore_patterns('*.zip', '*.py', '*.pyc', 'CVS*',
                                           '.svn')
        shutil.copytree(src, 
                        dst, 
                        symlinks=False, 
                        ignore=ignore_fn)

        # unzip fancybox to output directory
        infile = os.path.join(src, 'fancybox.zip')
        z = zipfile.ZipFile(infile, 'r')        
        z.extractall(outdir)

    @staticmethod
    def render(context):
        # TODO remove this once all task imports have been removed
        update_with_temp_renderers()
        
        # copy CSS, javascript etc. to weblog directory
        WebLogGenerator.copy_resources(context)

        # We could seriously optimise the rendering process by only unpickling
        # those objects that we need to render.  
        LOG.todo('Add results argument to renderer interfaces!')
        proxies = context.results
        
        try:
            # unpickle the results objects ready for rendering
            context.results = [proxy.read() for proxy in context.results]

            for renderer in WebLogGenerator.renderers:
                try:
                    LOG.trace('%s rendering...' % renderer.__name__)
                    renderer.render(context)
                except Exception as e:
                    LOG.exception('Error generating weblog: %s', e)

            # create symlink to t1-1.html 
            link_relsrc = T1_1Renderer.output_file
            link_abssrc = os.path.join(context.report_dir, link_relsrc)
            link_dst = os.path.join(context.report_dir, 'index.html')
            if os.path.exists(link_abssrc) and not os.path.exists(link_dst):
                os.symlink(link_relsrc, link_dst)
        finally:
            context.results = proxies

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
        output_file = os.path.join(context.report_dir, 'casapy.log')

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

#     @staticmethod    
#     def write_stage_logs(context):
#         """
#         Take the CASA log snippets attached to each result and write them to
#         the appropriate weblog directory. The log snippet is deleted from the
#         result after a successful write to keep the pickle size down. 
#         """
#         for result in context.results:
#             if not hasattr(result, 'casalog'):
#                 continue
# 
#             stage_dir = os.path.join(context.report_dir,
#                                      'stage%s' % result.stage_number)
#             if not os.path.exists(stage_dir):                
#                 os.makedirs(stage_dir)
# 
#             stagelog_entries = result.casalog
#             start = result.timestamps.start
#             end = result.timestamps.end
# 
#             stagelog_path = os.path.join(stage_dir, 'casapy.log')
#             with open(stagelog_path, 'w') as stagelog:
#                 LOG.debug('Writing CASA log entries for stage %s (%s -> %s)' %
#                           (result.stage_number, start, end))                          
#                 stagelog.write(stagelog_entries)
#                 
#             # having written the log entries, the CASA log entries have no 
#             # further use. Remove them to keep the size of the pickle small
#             delattr(result, 'casalog')
#
#    @staticmethod    
#    def write_stage_logs(context):
#        casalog = os.path.join(context.report_dir, 'casapy.log')
#
#        for result in context.results:
#            stage_dir = os.path.join(context.report_dir,
#                                     'stage%s' % result.stage_number)
#            stagelog_path = os.path.join(stage_dir, 'casapy.log')
#            if os.path.exists(stagelog_path):
#                LOG.trace('CASA log exists for stage %s, continuing..' 
#                          % result.stage_number)
##                continue
#
#            if not os.path.exists(stage_dir):                
#                os.makedirs(stage_dir)
#
#            # CASA log timestamps have seconds resolution, whereas our task
#            # timestamps have microsecond resolution. Cast down to seconds 
#            # resolution to make a comparison, taking care to leave the 
#            # original timestamp unaltered
#            start = result.timestamps.start.replace(microsecond=0)
#            end = result.timestamps.end.replace(microsecond=0)
#            end += datetime.timedelta(seconds=1)
#            
#            # get the hif_XXX command from the task attribute if possible,
#            # otherwise fall back to the Python class name accessible at
#            # taskname
#            task = result.taskname
#             
#            stagelog_entries = LogCopier._extract(casalog, start, end, task)
#            with open(stagelog_path, 'w') as stagelog:
#                LOG.debug('Writing CASA log entries for stage %s (%s -> %s)' %
#                          (result.stage_number, start, end))                          
#                stagelog.writelines(stagelog_entries)
#
#    @staticmethod
#    def _extract(filename, start, end, task=None):
#        with open(filename, 'r') as logfile:
#            rows = logfile.readlines()
#
#        # find the indices of the log entries recorded just after and just 
#        # before the end of task execution. We do this so that our subsequent
#        # search can begin these times, giving a more optimal search
#        pattern = re.compile('^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}')
#        timestamps = [pattern.match(r).group(0) for r in rows]
#        datetimes = [datetime.datetime.strptime(t, '%Y-%m-%d %H:%M:%S')
#                     for t in timestamps]
#        within_timestamps = [n for n, elem in enumerate(datetimes) 
#                             if elem > start and elem < end]            
#        start_idx, end_idx = within_timestamps[0], within_timestamps[-1]
#
#        # Task executions are bookended by statements log entries like this:
#        #
#        # 2013-02-15 13:55:47 INFO    hif_importdata::::casa+ ##########################################
#        #
#        # This regex matches this pattern, and therefore the start and end
#        # sections of the CASA log for this task 
#        pattern = re.compile('^.*?%s::::casa\+\t\#{42}$' % task)
#
#        # Rewinding from the starting timestamp, find the index of the task
#        # start log entry
#        for idx in range(within_timestamps[0], 0, -1):
#            if pattern.match(rows[idx]):
#                start_idx = idx
#                break
#
#        # looking forward from the end timestamp, find the index of the task
#        # end log entry
#        for idx in range(within_timestamps[-1], len(rows)-1):
#            if pattern.match(rows[idx]):
#                end_idx = min(idx+1, len(rows))
#                break
#
#        return rows[start_idx:end_idx]


# renderer_map holds the mapping of tasks to result renderers for
# task-dependent weblog sections. This lets us write customised output for
# each task type.
renderer_map = {
    T2_3MDetailsRenderer : {
        hifa.tasks.Wvrgcalflag    : T2_3MDetailsWvrgcalflagRenderer(),
        # hif.tasks.Bandpass       : T2_3MDetailsBandpassRenderer(),
    },
    T2_4MDetailsRenderer : {
        hif.tasks.Atmflag        : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_atmflag.html'),
        hif.tasks.Clean          : T2_4MDetailsCleanRenderer(),
        hif.tasks.CleanList      : T2_4MDetailsCleanRenderer(),
        hif.tasks.ExportData     : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_exportdata.html'),
        hif.tasks.Rawflagchans   : T2_4MDetailsRawflagchansRenderer(),
        hif.tasks.Fluxscale      : T2_4MDetailsDefaultRenderer('t2-4m_details-fluxscale.html'),
        hifa.tasks.Fluxdb        : T2_4MDetailsDefaultRenderer('t2-4m_details-hifa_fluxdb.html'),
        hif.tasks.MakeCleanList  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_makecleanlist.html'),
        hif.tasks.NormaliseFlux  : T2_4MDetailsDefaultRenderer('t2-4m_details-hif_normflux.html'),
        hifv.tasks.flagging.uncalspw.Uncalspw    : T2_4MDetailsDefaultRenderer('t2-4m_details-hifv_uncalspw.html', always_rerender=False),
        hifv.tasks.Applycals                     : applycal_renderer.T2_4MDetailsApplycalRenderer(always_rerender=False)
        
    }
}

# adding classes to this tuple always rerenders their content, bypassing the
# cache or 'existing file' checks. This is useful for developing and debugging
# as you can just call WebLogGenerator.render(context) 
DEBUG_CLASSES = []

    
def update_with_temp_renderers():
    """
    This is a temporary function built to decouple task import in this 
    (deprecated) module from weblog functionality. It should be factored out
    once templates and renderers have been migrated to task packages.
    """  
    from . import weblog
    renderer_map[T2_4MDetailsRenderer].update(weblog.TEMP_RENDERER_MAP)
