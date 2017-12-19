from __future__ import absolute_import, division
import collections
import contextlib
import datetime
import itertools
import math
import os
import pydoc
import pkg_resources
import re
import shutil
import zipfile

from casa_system import casa as casasys
import mako

import pipeline as pipeline
import pipeline.domain.measures as measures
import pipeline.extern.adopted as adopted
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.casataskdict as casataskdict
import pipeline.infrastructure.displays.summary as summary
import pipeline.infrastructure.displays.pointing as pointing
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.renderer.templates import resources
from . import qaadapter
from .. import utils
from . import weblog

LOG = infrastructure.get_logger(__name__)


def get_task_description(result_obj, context):
    if not isinstance(result_obj, (list, basetask.ResultsList)):
        return get_task_description([result_obj, ], context)

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

    description = None

    if hasattr(result_obj, 'metadata'):
        metadata = result_obj.metadata
    elif hasattr(result_obj[0], 'metadata'):
        metadata = result_obj[0].metadata
    else:
        LOG.trace('No metadata property found on result for task {!s}'.format(task_cls.__name__))
        metadata = {}

    if 'long description' in metadata:
        description = metadata['long description']

    if not description:
        try:
            # taking index 0 should be safe as entry to function ensures
            # result_obj is a list
            renderer = weblog.get_renderer(task_cls, context, result_obj[0])
        except KeyError:
            LOG.error('No renderer registered for task %s'.format(task_cls.__name__))
            raise
        else:
            description = getattr(renderer, 'description', None)

    if description is None:
        description = _get_task_description_for_class(task_cls)    

    d = {'description': description,
         'task_name': get_task_name(result_obj, include_stage=False),
         'stage': get_stage_number(result_obj)}
    return '{stage}. <strong>{task_name}</strong>: {description}'.format(**d)


def _get_task_description_for_class(task_cls):
    if LOG.isEnabledFor(LOG.todo):
        LOG.todo('No task description for \'%s\'' % task_cls.__name__)
        return ('\'%s\' (developers should add a task description)'
                '' % task_cls.__name__)

    return '\'%s\'' % task_cls.__name__


def get_task_name(result_obj, include_stage=True):
    stage = '%s. ' % get_stage_number(result_obj) if include_stage else ''

    if hasattr(result_obj, 'task'):
        task = result_obj.task
        return '%s%s' % (stage,
                         casataskdict.classToCASATask.get(task, task.__name__))

    else:
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

def is_singledish_ms(context):
    # importdata results
    result0 = context.results[0]
    
    # if ResultsProxy, read pickled result
    if isinstance(result0, basetask.ResultsProxy):
        result0 = result0.read()
                
    # if RestoreDataResults, get importdata_results
    if hasattr(result0, 'importdata_results'):
        result0 = result0.importdata_results[0]
    
    result_repr = str(result0)
    return result_repr.find('SDImportDataResults') != -1

def get_casa_version():
    casa_version = casatools.utils.version()
    casa_version_str = "%d.%d.%d-%d" % (casa_version[0], casa_version[1], casa_version[2], casa_version[3])
    return casa_version_str


class Session(object):
    def __init__(self, mses=None, name='Unnamed Session'):
        self.mses = [] if mses is None else mses
        self.name = name

    @staticmethod
    def get_sessions(context):
        # eventually we will need to sort by OUS ID too, but at the moment data
        # is all registered against a single OUS ID.

        d = {}
        for ms in get_mses_by_time(context):
            d.setdefault(ms.session, []).append(ms)

        session_names = []
        for session_name, session_mses in d.iteritems():
            oldest_ms = min(session_mses, key=lambda ms: utils.get_epoch_as_datetime(ms.start_time))
            session_names.append((utils.get_epoch_as_datetime(oldest_ms.start_time), session_name, session_mses))

        # primary sort sessions by their start time then by session name
        def mycmp(t1, t2):
            if t1[0] != t2[0]:
                return cmp(t1[0], t2[0])
            elif t1[1] != t2[1]:
                # natural sort so that session9 comes before session10
                name_sorted = sorted((t1[1], t2[1]), key=utils.natural_sort)
                return -1 if name_sorted[0] == t1[1] else 1
            else:
                return 0

        return [Session(mses, name) for _, name, mses in sorted(session_names, cmp=mycmp)]


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
    template = 't1-1.mako'
    
    
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

        array_names = {ms.antenna_array.name
                       for ms in context.observing_run.measurement_sets}

        # pipeline execution start, end and duration
        exec_start = context.results[0].timestamps.start
        exec_end = context.results[-1].timestamps.end
        # remove unnecessary precision for execution duration
        dt = exec_end - exec_start
        exec_duration = datetime.timedelta(days=dt.days, seconds=dt.seconds)

#         qaresults = qaadapter.ResultsToQAAdapter(context.results)

        out_fmt = '%Y-%m-%d %H:%M:%S'
        
        # Set link to pipeline documentation depending on which observatory
        # this context is for.
        observatory = context.project_summary.telescope
        if observatory == 'ALMA':
            pipeline_doclink = pipeline.__pipeline_documentation_weblink_alma__
        else:
            pipeline_doclink = None
        
        #Observation Summary (formerly the T1-2 page)
        ms_summary_rows = []
        for ms in get_mses_by_time(context):
            link = 'sidebar_%s' % re.sub('[^a-zA-Z0-9]', '_', ms.basename)
            href = os.path.join('t2-1.html?sidebar=%s' % link)

            num_antennas = len(ms.antennas)
            # times should be passed as Python datetimes
            time_start = utils.get_epoch_as_datetime(ms.start_time)
            time_end = utils.get_epoch_as_datetime(ms.end_time)

            target_scans = [s for s in ms.scans if 'TARGET' in s.intents]
            # check for REFERENCE (OFF-source) in TARGET scans
            has_reference_scan = False
            for s in target_scans:
                if 'REFERENCE' in s.intents:
                    has_reference_scan = True
                    break
            if has_reference_scan:
                # target scan has OFF-source need to go harder way
                autocorr_only = is_singledish_ms(context)
                time_on_source =  utils.total_time_on_target_on_source(ms, autocorr_only)
            else:
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

        return {
            'pcontext': context,
            # 'casa_version': get_casa_version(),
            'casa_version': casasys['build']['version'],
            # 'casa_revision': casadef.subversion_revision,
            'casa_revision': casasys['build']['number'],
            'pipeline_revision': pipeline.revision,
            'pipeline_doclink': pipeline_doclink,
            'obs_start': obs_start.strftime(out_fmt),
            'obs_end': obs_end.strftime(out_fmt),
            'array_names': utils.commafy(array_names),
            'exec_start': exec_start.strftime(out_fmt),
            'exec_end': exec_end.strftime(out_fmt),
            'exec_duration': str(exec_duration),
            'project_uids': project_uids,
            'schedblock_uids': schedblock_uids,
            'execblock_uids': execblock_uids,
            'ous_uid': context.project_structure.ous_entity_id,
            'ousstatus_entity_id': context.project_structure.ousstatus_entity_id,
            'ppr_uid': None,
            'observers': observers,
            'ms_summary_rows': ms_summary_rows
        }


class T1_2Renderer(RendererBase):
    """
    T1-2 Observation Summary renderer
    """
    output_file = 't1-2.html'
    template = 't1-2.mako'

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
        for ms in get_mses_by_time(context):
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
        
        return {'pcontext': context,
                'ms_summary_rows': ms_summary_rows}


class T1_3MRenderer(RendererBase):
    """
    T1-3M renderer
    """
    output_file = 't1-3.html'
    template = 't1-3m.mako'
    
    MsgTableRow = collections.namedtuple('MsgTableRow', 'stage task type message target')
    
    @classmethod
    def get_display_context(cls, context):
        registry = qaadapter.registry
        # distribute results between topics
        registry.assign_to_topics(context.results)

        scores = {}
        tablerows = []
        flagtables = {}
        for result in context.results:
            scores[result.stage_number] = result.qa.representative
            results_list = get_results_by_time(context, result)
        
            qa_errors = cls._filter_qascores(results_list, -0.1, 0.33)
            tablerows.extend(cls._qascores_to_tablerows(qa_errors,
                                                        results_list,
                                                        'QA Error'))
        
            qa_warnings = cls._filter_qascores(results_list, 0.33, 0.66)
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
            
            if 'applycal' in get_task_description(result, context):
                try:
                    for resultitem in result:
                        vis = os.path.basename(resultitem.inputs['vis'])
                        ms = context.observing_run.get_ms(vis)
                        flagtable = collections.OrderedDict()
                        for field in resultitem.flagsummary:
                            intents = ','.join([f.intents for f in ms.get_fields(intent='BANDPASS,PHASE,AMPLITUDE,CHECK,TARGET')
                                                if field in f.name][0])
                            
                            flagsummary = resultitem.flagsummary[field]
                            
                            if len(flagsummary) == 0:
                                continue
                        
                            fieldtable = {}
                            for _, v in flagsummary.iteritems():
                                myname = v['name']
                                myspw = v['spw']
                                myant = v['antenna']
                                spwkey = myspw.keys()[0]
                            
                                fieldtable[myname] = {spwkey: myant}
                                
                            flagtable['Source name: '+ field + ', Intents: ' + intents] = fieldtable
                        
                        flagtables[ms.basename] = flagtable
                        
                except:
                    LOG.debug('No flag summary table available yet from applycal')

        return {'pcontext': context,
                'registry': registry,
                'scores': scores,
                'tablerows': tablerows,
                'flagtables': flagtables}

    @classmethod
    def _filter_qascores(cls, results_list, lo, hi):
        qa_pool = results_list.qa.pool
        #print qa_pool
        with_score = [s for s in qa_pool if s.score not in ('', 'N/A', None)]
        return [s for s in with_score if s.score > lo and s.score <= hi]

    @classmethod
    def _create_tablerow(cls, results, message, msgtype, target=''):
        return cls.MsgTableRow(stage=results.stage_number,
                               task=get_task_name(results, False),
                               type=msgtype,
                               message=message,
                               target=target)

    @classmethod
    def _qascores_to_tablerows(cls, qascores, results, msgtype='ERROR'):
        def get_target(qascore):
            vis = qascore.target.get('vis', None)
            return '&ms=%s' % vis if vis else ''
        
        return [cls._create_tablerow(results, qascore.longmsg, msgtype, 
                                     get_target(qascore))
                for qascore in qascores]

    @classmethod
    def _logrecords_to_tablerows(cls, records, results, msgtype='ERROR'):
        def get_target(logrecord):
            try:
                vis = logrecord.target['vis']
                return '&ms=%s' % vis if vis else ''
            except AttributeError:
                return ''
            except KeyError:
                return ''

        return [cls._create_tablerow(results, record.msg, msgtype,
                                     get_target(record))
                for record in records]


class T1_4MRenderer(RendererBase):
    """
    T1-4M renderer
    """
    output_file = 't1-4.html'
    # TODO get template at run-time
    template = 't1-4m.mako'

    @staticmethod
    def get_display_context(context):
        scores = {}
        for result in context.results:
            scores[result.stage_number] = result.qa.representative

        # take first MS of first session
#        s = sorted(context.observing_run.measurement_sets,
#                   key=lambda ms: (ms.session, ms.basename)) 
#        root = s[0].basename
                
        return {'pcontext' : context,
                'results'  : context.results,
#                'root'     : root,
                'scores'   : scores}
        

class T2_1Renderer(RendererBase):
    """
    T2-4M renderer
    """
    output_file = 't2-1.html'
    template = 't2-1.mako'

    @staticmethod
    def get_display_context(context):
        sessions = Session.get_sessions(context)
        return {'pcontext' : context,
                'sessions' : sessions}


class T2_1DetailsRenderer(object):
    output_file = 't2-1_details.html'
    template = 't2-1_details.mako'

    @classmethod
    def get_file(cls, filename):
        ms_dir = os.path.dirname(filename)

        if not os.path.exists(ms_dir):
            os.makedirs(ms_dir)

        file_obj = open(filename, 'w')
        return contextlib.closing(file_obj)

    @classmethod
    def get_filename(cls, context, session, ms):
        return os.path.join(context.report_dir,
                            'session%s' % session.name,
                            ms.basename,
                            cls.output_file)

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

        num_antennas = len(ms.antennas)
        num_baselines = int(num_antennas * (num_antennas-1) / 2)

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

        task = summary.PWVChart(context, ms)
        pwv_plot = task.plot()
        
        task = summary.AzElChart(context, ms)
        azel_plot = task.plot()

        task = summary.ElVsTimeChart(context, ms)
        el_vs_time_plot = task.plot()

        # Get min, max elevation
        el_min = "%.2f" % adopted.computeAzElForMS(ms.name, value='min')[1]
        el_max = "%.2f" % adopted.computeAzElForMS(ms.name, value='max')[1]

        dirname = os.path.join('session%s' % ms.session,
                               ms.basename)

        vla_basebands = ''

        if context.project_summary.telescope not in ('ALMA', 'NRO'):
        # All VLA basebands

            vla_basebands = []

            banddict = collections.defaultdict(lambda: collections.defaultdict(list))

            for spw in ms.get_spectral_windows():
                try:
                    band = spw.name.split('#')[0].split('_')[1]
                    baseband = spw.name.split('#')[1]
                    banddict[band][baseband].append({str(spw.id):(spw.min_frequency,spw.max_frequency)})
                except:
                    LOG.debug("Baseband name cannot be parsed and will not appear in the weblog.")

            for band in banddict.keys():
                basebands = banddict[band].keys()
                for baseband in basebands:
                    spws = []
                    minfreqs = []
                    maxfreqs = []
                    for spwitem in banddict[band][baseband]:
                        spws.append(spwitem.keys()[0])
                        minfreqs.append(spwitem[spwitem.keys()[0]][0])
                        maxfreqs.append(spwitem[spwitem.keys()[0]][1])
                    bbandminfreq = min(minfreqs)
                    bbandmaxfreq = max(maxfreqs)
                    vla_basebands.append(band+': '+baseband+':  '+ str(bbandminfreq)+ ' to '+ str(bbandmaxfreq)+':   ['+','.join(spws)+']   ')

            vla_basebands = '<tr><th>VLA Bands: Basebands:  Freq range: [spws]</th><td>'+'<br>'.join(vla_basebands)+'</td></tr>'

        if is_singledish_ms(context):
            # Single dish specific 
            # to get thumbnail for representative pointing plot
            antenna = ms.antennas[0]
            field_strategy = ms.calibration_strategy['field_strategy']
            target = field_strategy.keys()[0]
            reference = field_strategy[target]
            LOG.debug('target field id %s / reference field id %s'%(target,reference))
            task = pointing.SingleDishPointingChart(context, ms, antenna, 
                                                        target_field_id=target,
                                                        reference_field_id=reference,
                                                        target_only=True)
            pointing_plot = task.plot()
        else:
            pointing_plot = None

        return {
            'pcontext'        : context,
            'ms'              : ms,
            'science_sources' : utils.commafy(science_sources),
            'calibrators'     : utils.commafy(calibrators),
            'all_bands'       : utils.commafy(all_bands),
            'science_bands'   : utils.commafy(science_bands),
            'baseline_min'    : baseline_min,
            'baseline_max'    : baseline_max,
            'num_antennas'    : num_antennas,
            'num_baselines'   : num_baselines,
            'time_start'      : utils.format_datetime(time_start),
            'time_end'        : utils.format_datetime(time_end),
            'time_on_source'  : utils.format_timedelta(time_on_source),
            'time_on_science' : utils.format_timedelta(time_on_science),
            'intent_vs_time'  : intent_vs_time,
            'field_vs_time'   : field_vs_time,
            'dirname'         : dirname,
            'weather_plot'    : weather_plot,
            'pwv_plot'        : pwv_plot,
            'azel_plot'       : azel_plot,
            'el_vs_time_plot' : el_vs_time_plot,
            'is_singledish'   : is_singledish_ms(context),
            'pointing_plot'   : pointing_plot,
            'el_min'          : el_min,
            'el_max'          : el_max,
            'vla_basebands'   : vla_basebands
        }

    @classmethod
    def render(cls, context):
        for session in Session.get_sessions(context):
            for ms in session.mses:
                filename = cls.get_filename(context, session, ms)
                # now that the details pages are written per MS rather than having
                # tabs for each MS, we don't need to write them each time as
                # importdata will not affect their content.
                if os.path.exists(filename):
                    continue

                with cls.get_file(filename) as fileobj:
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
    template = 't2-2-1.mako'

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
    template = 't2-2-2.mako'

    @staticmethod
    def get_display_context(context, ms):
        return {'pcontext' : context,
                'ms'       : ms}


class T2_2_3Renderer(T2_2_XRendererBase):
    """
    T2-2-3 renderer
    """
    output_file = 't2-2-3.html'
    template = 't2-2-3.mako'

    @staticmethod
    def get_display_context(context, ms):
        if context.project_summary.telescope in ('NRO',):
            # antenna plots are useless for Nobeyama
            plot_ants = None
            plot_ants_plog = None
        else:
            # Create regular antenna positions plot.
            task = summary.PlotAntsChart(context, ms)
            plot_ants = task.plot()
            
            # Create polar-log antenna positions plot.
            task = summary.PlotAntsChart(context, ms, polarlog=True)
            plot_ants_plog = task.plot()

        dirname = os.path.join('session%s' % ms.session,
                               ms.basename)
        
        return {'pcontext'       : context,
                'plot_ants'      : plot_ants,
                'plot_ants_plog' : plot_ants_plog,
                'ms'             : ms,
                'dirname'        : dirname}


class T2_2_4Renderer(T2_2_XRendererBase):
    """
    T2-2-4 renderer
    """
    output_file = 't2-2-4.html'
    template = 't2-2-4.mako'

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
    template = 't2-2-5.mako'

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
    template = 't2-2-6.mako'

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


class T2_2_7Renderer(T2_2_XRendererBase):
    """
    T2-2-7 renderer (single dish specific)
    """
    output_file = 't2-2-7.html'
    template = 't2-2-7.mako'

    @classmethod
    def render(cls, context):
        if is_singledish_ms(context):
            super(T2_2_7Renderer, cls).render(context)

    @staticmethod
    def get_display_context(context, ms):
        target_pointings = []
        whole_pointings = []
        if is_singledish_ms(context):
            for antenna in ms.antennas:
                for (target, reference) in ms.calibration_strategy['field_strategy'].items():
                    LOG.debug('target field id %s / reference field id %s'%(target,reference))
                    task = pointing.SingleDishPointingChart(context, ms, antenna, 
                                                                target_field_id=target,
                                                                reference_field_id=reference,
                                                                target_only=True)
                    plotres = task.plot()
                    # for missing antenna, spw, field combinations
                    if plotres is None: continue
                    target_pointings.append(plotres)
                    task = pointing.SingleDishPointingChart(context, ms, antenna, 
                                                                target_field_id=target,
                                                                reference_field_id=reference,
                                                                target_only=False)
                    plotres = task.plot()
                    if plotres is not None:
                        whole_pointings.append(plotres)

        dirname = os.path.join('session%s' % ms.session,
                               ms.basename)

        return {'pcontext'        : context,
                'ms'              : ms,
                'target_pointing' : target_pointings,
                'whole_pointing'  : whole_pointings,
                'dirname'         : dirname}


class T2_3_XMBaseRenderer(RendererBase):
    # the filename to which output will be directed
    output_file = 'overrideme'
    # the template file for this renderer
    template = 'overrideme'

    MsgTableRow = collections.namedtuple('MsgTableRow', 'stage task type message target')

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
                qa_errors = cls._filter_qascores(results_list, -0.1, 0.33)
                tablerows.extend(cls._qascores_to_tablerows(qa_errors,
                                                            results_list,
                                                            'QA Error'))
                    
                qa_warnings = cls._filter_qascores(results_list, 0.33, 0.66)
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
    def _create_tablerow(cls, results, message, msgtype, target=''):
        return cls.MsgTableRow(stage=results.stage_number,
                               task=get_task_name(results, False),
                               type=msgtype,
                               message=message,
                               target=target)

    @classmethod
    def _qascores_to_tablerows(cls, qascores, results, msgtype='ERROR'):
        def get_target(qascore):
            vis = qascore.target.get('vis', None)
            return '&ms=%s' % vis if vis else ''
        
        return [cls._create_tablerow(results, qascore.longmsg, msgtype, 
                                     get_target(qascore))
                for qascore in qascores]

    @classmethod
    def _logrecords_to_tablerows(cls, records, results, msgtype='ERROR'):
        def get_target(logrecord):
            try:
                vis = logrecord.target['vis']
                return '&ms=%s' % vis if vis else ''
            except AttributeError:
                return ''
            except KeyError:
                return ''

        return [cls._create_tablerow(results, record.msg, msgtype,
                                     get_target(record))
                for record in records]


class T2_3_1MRenderer(T2_3_XMBaseRenderer):
    """
    Renderer for T2-3-1M, the data set topic.
    """
    # the filename to which output will be directed
    output_file = 't2-3-1m.html'
    # the template file for this renderer
    template = 't2-3-1m.mako'

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
    template = 't2-3-2m.mako'

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
    template = 't2-3-3m.mako'

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
    template = 't2-3-4m.mako'

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
    template = 't2-3-5m.mako'

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
    template = 't2-3-6m.mako'

    @classmethod
    def get_topic(cls):
        return qaadapter.registry.get_miscellaneous_topic()        


class T2_4MRenderer(RendererBase):
    """
    T2-4M renderer
    """
    output_file = 't2-4m.html'
    template = 't2-4m.mako'

    @staticmethod
    def get_display_context(context):
        return {'pcontext' : context,
                'results'  : context.results}
        
#     @classmethod
#     def get_file(cls, context, root):
#         path = cls.get_path(context, root)
# 
#         # to avoid any subsequent file not found errors, create the directory
#         # if a hard copy is requested and the directory is missing
#         session_dir = os.path.dirname(path)
#         if not os.path.exists(session_dir):
#             os.makedirs(session_dir)
#         
#         # create a file object that writes to a file
#         file_obj = open(path, 'w')
#         
#         # return the file object wrapped in a context manager, so we can use
#         # it with the autoclosing 'with fileobj as f:' construct
#         return contextlib.closing(file_obj)
# 
#     @classmethod
#     def get_path(cls, context, root):
#         path = os.path.join(context.report_dir, root)
#         return os.path.join(path, cls.output_file)
# 
#     @classmethod
#     def render(cls, context):
#         # dict that will map session ID to session results
#         collated = collections.defaultdict(list)
#         for result in context.results:
#             # we only handle lists of results, so wrap single objects in a
#             # list if necessary
#             if not isinstance(result, collections.Iterable):
#                 result = wrap_in_resultslist(result)
#             
#             # split the results in the list into streams, divided by session
#             d = group_by_root(context, result)
#             for root, session_results in d.items():
#                 collated[root].extend(session_results)
# 
#         for root, session_results in collated.items():
#             cls.render_root(context, root, session_results)
# 
#     @classmethod
#     def render_root(cls, context, root, results):                
#         template = weblog.TEMPLATE_LOOKUP.get_template(cls.template)
# 
#         mako_context = {'pcontext' : context,
#                         'root'     : root,
#                         'results'  : results}
#         
#         with cls.get_file(context, root) as fileobj:
#             fileobj.write(template.render(**mako_context))


class T2_4MDetailsDefaultRenderer(object):
    def __init__(self, template='t2-4m_details-generic.mako',
                 always_rerender=False):
        self.template = template
        self.always_rerender = always_rerender

    def get_display_context(self, context, result):
        mako_context = {'pcontext' : context,
                        'result'   : result,
                        'casalog_url' : self._get_log_url(context, result),
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

    def _get_log_url(self, context, result):
        """
        Get the URL of the stage log relative to the report directory.
        """
        stagelog_path = os.path.join(context.report_dir,
                                     'stage%s' % result.stage_number,
                                     'casapy.log')

        if not os.path.exists(stagelog_path):
            return None

        return os.path.relpath(stagelog_path, context.report_dir)        

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



#----------------------------------------------------------------------

class T2_4MDetailsContainerRenderer(RendererBase):
    output_file = 't2-4m_details-container.html'
    template = 't2-4m_details-container.mako'

    @classmethod
    def get_path(cls, context, result):
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, stage)
        return os.path.join(stage_dir, cls.output_file)

    @classmethod
    def get_file(cls, context, result):
        path = cls.get_path(context, result)
        file_obj = open(path, 'w')
        return contextlib.closing(file_obj)

    @classmethod
    def render(cls, context, result, urls):
        # give the implementing class a chance to bypass rendering. This is
        # useful when the page has not changed, eg. MS description pages when
        # no subsequent ImportData has been performed
        path = cls.get_path(context, result)
        if os.path.exists(path) and not cls.rerender(context):
            return

        mako_context = {'pcontext' : context,
                        'container_urls': urls,
                        'active_ms' : 'N/A'}

        template = weblog.TEMPLATE_LOOKUP.get_template(cls.template)
        with cls.get_file(context, result) as fileobj:
            fileobj.write(template.render(**mako_context))


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
    :param root: filename component to insert
    :type root: string
    :rtype: a file object
    """
    @classmethod
    def get_file(cls, context, result, root):
        # construct the relative filename, eg. 'stageX/t2-4m_details.html'
        path = cls.get_path(context, result, root)

        # to avoid any subsequent file not found errors, create the directory
        # if a hard copy is requested and the directory is missing
        dirname = os.path.dirname(path)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        
        # create a file object that writes to a file if a hard copy is 
        # requested, otherwise return a file object that flushes to stdout
        file_obj = open(path, 'w')
        
        # return the file object wrapped in a context manager, so we can use
        # it with the autoclosing 'with fileobj as f:' construct
        return contextlib.closing(file_obj)

    """
    Get the template output path.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param result: the task results object to render
    :type result: :class:`~pipeline.infrastructure.api.Result`
    :param root: the optional directory component to insert before the stage
    :type root: string
    :rtype: string
    """
    @classmethod
    def get_path(cls, context, result, root=''):
        # HTML output will be written to the directory 'stageX' 
        stage = 'stage%s' % result.stage_number
        stage_dir = os.path.join(context.report_dir, root, stage)

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
        # for each result accepted and stored in the context..
        for task_result in context.results:
            # we only handle lists of results, so wrap single objects in a
            # list if necessary
            if not isinstance(task_result, collections.Iterable):
                task_result = wrap_in_resultslist(task_result)
            
            # find the renderer appropriate to the task..
            task = task_result[0].task
            try:
                renderer = weblog.get_renderer(task, context, task_result)
            except KeyError:
                LOG.warning('No renderer was registered for task %s'.format(task.__name__))
                renderer = cls._default_renderer
            LOG.trace('Using %s to render %s result',
                      renderer.__class__.__name__, task.__name__)

            container_urls = {}

            if task.__name__ in weblog.RENDER_UNGROUPED:
                cls.render_result(renderer, context, task_result)

                ms_weblog_path = cls.get_path(context, task_result, '')
                relpath = os.path.relpath(ms_weblog_path, context.report_dir)
                container_urls['combined session'] = {
                        'all' : (relpath, task_result)
                }

                # create new container
                container = T2_4MDetailsContainerRenderer
                container.render(context, task_result, container_urls)

            elif task.__name__ in weblog.RENDER_BY_SESSION:
                session_grouped = group_into_sessions(context, task_result)
                for session_id, session_results in session_grouped.items():
                    container_urls[session_id] = {}
                    ms_grouped = group_into_measurement_sets(context, session_results)
        
                    for ms_id, ms_result in ms_grouped.items():
                        cls.render_result(renderer, context, ms_result, ms_id)
    
                        ms_weblog_path = cls.get_path(context, ms_result, ms_id)
                        relpath = os.path.relpath(ms_weblog_path, context.report_dir)
                        container_urls[session_id][ms_id] = (relpath, ms_result)
    
                # create new container
                container = T2_4MDetailsContainerRenderer
                container.render(context, task_result, container_urls)
                
            else:
                LOG.warning('Don\'t know how to group %s renderer', task.__name__)

    @classmethod
    def render_result(cls, renderer, context, result, root=''):                
        # details pages do not need to be updated once written unless the
        # renderer specifies that an update is required
        path = cls.get_path(context, result, root)
        LOG.trace('Path for %s is %s', result.__class__.__name__, path)
        force_rerender = getattr(renderer, 'always_rerender', False)
        debug_cls = renderer.__class__ in DEBUG_CLASSES
        force_rerender = force_rerender or debug_cls # or result.stage_number in (9,)

        if force_rerender:
            LOG.trace('Forcing rerendering for %s', renderer.__class__.__name__)

        if os.path.exists(path) and not force_rerender:
            return

        # .. get the file object to which we'll render the result
        with cls.get_file(context, result, root) as fileobj:
            # .. and write the renderer's interpretation of this result to
            # the file object  
            try:
                LOG.trace('Writing %s output to %s', renderer.__class__.__name__,
                          path)
                fileobj.write(renderer.render(context, result))
            except:
                LOG.warning('Template generation failed for %s', cls.__name__)
                LOG.debug(mako.exceptions.text_error_template().render())
                fileobj.write(mako.exceptions.html_error_template().render())


def wrap_in_resultslist(task_result):
    l = basetask.ResultsList()
    l.append(task_result)
    l.timestamps = task_result.timestamps
    l.stage_number = task_result.stage_number
    l.inputs = task_result.inputs
    if hasattr(task_result, 'taskname'):
        l.taskname = task_result.taskname
    if hasattr(task_result, 'metadata'):
        l.metadata.update(task_result.metadata)

    # the newly-created ResultsList wrapper is missing a QA pool. However,
    # as there is only ever one task added to the list we can safely assume
    # that the pool for the wrapper should equal that of the child. The same
    # holds for logrecords. The task name is required as the plot level 
    # toggles work off the CASA task name.
    for attr in ['qa', 'pipeline_casa_task', 'logrecords']:
        if hasattr(task_result, 'qa'):
            setattr(l, attr, getattr(task_result, attr))

    return l


def group_into_sessions(context, task_results):
    """
    Return results grouped into lists by session. 
    """
    session_map = {ms.basename : ms.session 
                   for ms in context.observing_run.measurement_sets}
    
    def get_session(r):
        # return the session inputs argument if present, otherwise find
        # which session the measurement set is in
        if 'session' in r.inputs:
            return r.inputs['session']

        basename = os.path.basename(r.inputs['vis'])
        return session_map.get(basename, 'Shared')
    
    d = {}
    results_by_session = sorted(task_results, key=get_session)
    for k, g in itertools.groupby(results_by_session, get_session):
        l = basetask.ResultsList()
        l.extend(g)
        l.timestamps = task_results.timestamps
        l.stage_number = task_results.stage_number
        l.inputs = task_results.inputs
        if hasattr(task_results, 'taskname'):
            l.taskname = task_results.taskname
        d[k] = l

    return d


def group_into_measurement_sets(context, task_results):
    def get_vis(r):
        if type(r).__name__ in ('ImportDataResults', 'SDImportDataResults'):
            # in splitting by vis, there's only one MS in the mses array
            return r.mses[0].basename
        return os.path.basename(r.inputs['vis'])
    
    vises = [get_vis(r) for r in task_results]
    mses = [context.observing_run.get_ms(vis) for vis in vises]
    ms_names = [ms.basename for ms in mses]
    times = [utils.get_epoch_as_datetime(ms.start_time) for ms in mses]

    # sort MSes within session by execution time
    decorated = zip(times, ms_names, task_results)
    decorated.sort()
    
    d = collections.OrderedDict()
    for (_, name, task) in decorated:
        d[name] = wrap_in_resultslist(task)

    return d
    

def sort_by_time(mses):
    """
    Return measurement sets sorted by time order.
    """
    return sorted(mses, 
                  key=lambda ms: utils.get_epoch_as_datetime(ms.start_time))     


def get_rootdir(r):
    try:
        if type(r).__name__ in ('ImportDataResults', 'SDImportDataResults'):
            # in splitting by vis, there's only one MS in the mses array
            return r.mses[0].basename
        return os.path.basename(r.inputs['vis'])
    except:
        return 'shared'


def group_by_root(context, task_results):
    results_by_root = sorted(task_results, key=get_rootdir)

    d = collections.defaultdict(list)
    for k, g in itertools.groupby(results_by_root, get_rootdir):        
        l = basetask.ResultsList()
        l.extend(g)
        l.timestamps = task_results.timestamps
        l.stage_number = task_results.stage_number
        l.inputs = task_results.inputs
        if hasattr(task_results, 'taskname'):
            l.taskname = task_results.taskname
        d[k] = l

    return d


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
#                 T2_2_5Renderer,       # weather
                 T2_2_6Renderer,       # scans
                 T2_2_7Renderer,       # telescope pointing (single dish specific)
                 T2_3_1MRenderer,      # data set topic
                 T2_3_2MRenderer,      # calibration topic
                 T2_3_3MRenderer,      # flagging topic
        # disable unused line finding topic for July 2014 release
        # T2_3_4MRenderer,             # line finding topic
                 T2_3_5MRenderer,      # imaging topic
                 T2_3_6MRenderer,      # miscellaneous topic
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
        src = pkg_resources.resource_filename(resources.__name__, '')
        dst = outdir
        ignore_fn = shutil.ignore_patterns('*.zip', '*.py', '*.pyc', 'CVS*',
                                           '.svn')
        shutil.copytree(src, 
                        dst, 
                        symlinks=False, 
                        ignore=ignore_fn)

        # unzip fancybox to output directory
        infile = os.path.join(src, 'fancybox-2.1.5.zip')
        z = zipfile.ZipFile(infile, 'r')        
        z.extractall(outdir)

        distfile = os.path.join(src, 'bootstrap-3.3.5-dist.zip')
        WebLogGenerator.unpack_bootstrap(distfile, outdir)

    @staticmethod
    def unpack_bootstrap(distfile, outdir):
        zip_file = zipfile.ZipFile(distfile, 'r')
        for member in zip_file.namelist():
            filename = os.path.basename(member)
            # skip directories
            if not filename:
                continue
            
            # omit the first component ('dest') from the destination paths 
            # caution! this doesn't work if the path is absolute
            parts = member.split(os.path.sep)
            relocated = os.path.sep.join(parts[1:])
            
            # copy file
            source = zip_file.open(member)
            target = file(os.path.join(outdir, relocated), 'wb')
            with source, target:
                LOG.trace('Bootstrap copy: %s -> %s', source, target)
                shutil.copyfileobj(source, target)

    @staticmethod
    def render(context):
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


# adding classes to this tuple always rerenders their content, bypassing the
# cache or 'existing file' checks. This is useful for developing and debugging
# as you can just call WebLogGenerator.render(context) 
DEBUG_CLASSES = []

    
def get_mses_by_time(context):
    return sorted(context.observing_run.measurement_sets,
                  key=lambda ms: ms.start_time['m0']['value'])


def get_results_by_time(context, resultslist):
    # as this is a ResultsList with important properties attached, results
    # should be sorted in place.
    if hasattr(resultslist, 'sort'):
        if len(resultslist) is not 1:
            try:
                # sort the list of results by the MS start time
                resultslist.sort(key=lambda r: get_ms_start_time_for_result(context, r))
            except AttributeError:
                LOG.info('Could not time sort results for stage %s' % resultslist.stage_number)
    return resultslist


def get_ms_start_time_for_result(context, result):
    # single dish tasks do not attach Inputs to their component results, so
    # there's no reference or sort the results by.
    vis = result.inputs.get('vis', None)
    if vis is None:
        raise AttributeError
    return get_ms_attr_for_result(context, vis, lambda ms: ms.start_time['m0']['value'])


def get_ms_attr_for_result(context, vis, accessor):
    ms_basename = os.path.basename(vis)
    ms = context.observing_run.get_ms(ms_basename)
    return accessor(ms)
