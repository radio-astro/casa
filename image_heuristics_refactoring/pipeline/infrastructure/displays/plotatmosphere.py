from __future__ import absolute_import
import os
import string

import pipeline.extern.adopted as adopted
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.displays.applycal as applycal
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.jobrequest as jobrequest
import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure.renderer.logger as logger
from . import common


LOG = infrastructure.get_logger(__name__)


class PlotAtmosphereLeaf(object):
    """
    Class to execute plotAtmosphere and return a plot wrapper.
    """

    def __init__(self, context, result, calto, yaxis, spw='', field='', intent='',
                 scan='', **plot_args):
        self._context = context
        self._result = result

        self._vis = calto.vis
        self._ms = context.observing_run.get_ms(self._vis)

        if yaxis not in ('transmission', 'transmissionPercent', 'opacity', 'tsky'):
            raise ValueError('Unsupported y axis: {!r}'.format(yaxis))
        self._xaxis = 'frequency'
        self._yaxis = yaxis

        self._spw = spw
        self._intent = intent

        if scan == '':
            # Convert field and intent to scan selection. We do that here
            # rather than relying on the plot routine to determine the scan as
            # we may have remapped the pipeline intents from their original
            # CASA intents.
            domain_fields = self._ms.get_fields(field)
            domain_spws = self._ms.get_spectral_windows(spw)
            scan_ids = [s.id for s in self._ms.get_scans(scan_intent=intent)
                        if s.fields.intersection(domain_fields)
                        and s.spws.intersection(domain_spws)]

            if not scan_ids:
                raise ValueError('No scan available for field {!r}, spw {!r}, '
                                 'intent {!r}'.format(field, spw, intent))

            # take the first scan from the selection.
            first_scan = min(scan_ids)
            scan = first_scan
        self._scan = str(scan)

        # use field name rather than ID where possible
        field_label = ''
        if field != '':
            matching = self._ms.get_fields(field)

            if len(matching) is 1:
                field_name = matching[0].name
                field_id = matching[0].id
                # in case the arg is an ID, check that returned field doesn't
                # contain a duplicate name
                matching_by_name = self._ms.get_fields(field_name)
                if len(matching_by_name) is 1:
                    # if the argument was the field ID but the field name is
                    # unique. Therefore, we can use the field name.
                    field = field_name
                    field_label = field_name
                    LOG.trace('One field found for search term %r and name '
                              'appears unique. Using %r as field identifier',
                              field, field_label)
                else:
                    # if the argument was the field ID but the field name is
                    # not unique, prepend the ID with the field name
                    field_label = '%s (field #%s)' % (field_name, field_id)
                    LOG.trace('One field found for search term %r but name '
                              'also refers to other fields. Using %r as field '
                              'identifier', field, field_label)
            else:
                LOG.trace('More than one field found for search term %r. '
                          'Using %r as field identifier', field, field)
                field_label = field

        self._field = str(field)
        self._field_label = str(field_label)

        # same procedure for receiver
        self._receiver = plot_args.get('receiver', '')
        if self._receiver:
            del plot_args['receiver']

        self._plot_args = plot_args
        self._plotfile = self._get_plotfile()

    def plot(self):
        task = self._get_plot_task()
        return [(task, self._get_plot_wrapper(task))]

    def _get_plotfile(self):
        mag = len(str(len(self._ms.scans)))
        scan = str(self._scan).zfill(mag)

        fileparts = {
            'vis': os.path.basename(self._vis),
            'x': self._xaxis,
            'y': self._yaxis,
            'field': '' if self._field_label == '' else '-%s' % filenamer.sanitize(self._field_label.replace(',', '_')),
            'scan': '-scan%s' % scan,
            'intent': '' if self._intent == '' else '%s-' % self._intent.replace(',', '_'),
        }

        if self._spw == '':
            fileparts['spw'] = ''
        else:
            # format spws for filename sorting
            spws = ['%0.2d' % int(spw) for spw in string.split(str(self._spw), ',')]
            fileparts['spw'] = 'spw%s-' % '_'.join(spws)

        png = '{vis}{field}{scan}-{spw}{intent}{y}_vs_{x}.png'.format(**fileparts)

        # Maximum filename size for Lustre filesystems is 255 bytes. Mosaics
        # can exceed this limit due to including the names of all the field.
        # Truncate over-long field components while keeping them unique by
        # replacing them with the hash of the component
        if len(png) > 251:  # 255 - '.png'
            png_hash = str(hash(png))
            LOG.info('Truncating plot filename to avoid filesystem limit.\n'
                     'Old: %s\nNew: %s', png, png_hash)
            png = '%s.png' % png_hash

        return os.path.join(self._context.report_dir,
                            'stage%s' % self._result.stage_number,
                            png)

    def _get_plot_wrapper(self, task):
        parameters = {'vis': os.path.basename(self._vis)}

        domain_spws = self._ms.get_spectral_windows(self._spw)
        parameters['receiver'] = sorted(set(spw.band for spw in domain_spws))

        for param, attr in [('spw', '_spw'), ('intent', '_intent'), ('scan', '_scan'), ('field', '_field_label')]:
            val = getattr(self, attr)
            if val != '':
                parameters[param] = val

        wrapper = logger.Plot(self._plotfile,
                              x_axis=self._xaxis,
                              y_axis=self._yaxis,
                              parameters=parameters,
                              command=str(task))

        return wrapper

    def _get_plot_task(self):
        task_args = {
            'vis': self._ms.name,
            'spw': str(self._spw),
            'scan': str(self._scan),
            'quantity': self._yaxis,
            'plotfile': self._plotfile
        }

        task_args.update(**self._plot_args)

        job = jobrequest.JobRequest(adopted.plotAtmosphere, **task_args)

        return job


class SpwComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each spw in the applied calibration.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None

    def __init__(self, context, result, calto, yaxis='', field='', intent='', scan='', **kwargs):
        ms = context.observing_run.get_ms(calto.vis)

        children = []
        for spw in ms.get_spectral_windows(calto.spw):
            if intent != '':
                wanted = set(intent.split(','))
                if spw.intents.isdisjoint(wanted):
                    continue

            leaf_obj = self.leaf_class(context, result, calto, spw=spw.id,
                                       field=field, intent=intent, yaxis=yaxis, scan=scan,
                                       **kwargs)
            children.append(leaf_obj)

        super(SpwComposite, self).__init__(children)


class PlotAtmosphereSpwComposite(SpwComposite):
    leaf_class = PlotAtmosphereLeaf

    def plot(self):
        jobs_and_wrappers = super(PlotAtmosphereSpwComposite, self).plot()

        jobs_and_callbacks = []
        for job, wrapper in jobs_and_wrappers:
            plotfile = job.kw['plotfile']

            # execute merged job if some of the output files are missing
            if not os.path.exists(plotfile):
                if mpihelpers.is_mpi_ready():
                    executable = mpihelpers.Tier0FunctionCall(job.fn, *job.args, **job.kw)
                    queued_job = mpihelpers.AsyncTask(executable)
                else:
                    queued_job = mpihelpers.SyncTask(job)

                # variables within functions and lambdas are late binding, so we
                # supply them as default arguments to get the values at function
                # definition time into the closure scope
                def callback(plotfile=plotfile, job=job):
                    if not os.path.exists(plotfile):
                        LOG.info('%s not found. plotAtmosphere did not '
                                 'generate any output for %s'.format(plotfile, job))

                jobs_and_callbacks.append((queued_job, callback))
            else:
                LOG.trace('Skipping unnecessary job: {!s}'.format(job))

        # now execute all the callbacks
        for (queued_job, callback) in jobs_and_callbacks:
            queued_job.get_result()
            callback()

        # at this point, the sequentially-named plots from the merged job have
        # been renamed match that of the unmerged job, so we can simply check
        # whether the plot (with the original filename) exists or not.
        wrappers = [w for _, w in jobs_and_wrappers]
        successful_wrappers = filter(lambda w: os.path.exists(w.abspath), wrappers)

        return successful_wrappers


class SpwSummaryChart(PlotAtmosphereSpwComposite):
    """
    Base class for executing plotatmosphere per spw
    """
    def __init__(self, context, result, yaxis, intent, **kwargs):
        (calto, intent) = applycal._get_summary_args(context, result, intent)
        LOG.info('%s plot: %s' % (yaxis, calto))

        if 'field' in kwargs:
            field = kwargs['field']
            del kwargs['field']
            LOG.debug('Override for %s plot: field=%s' % (yaxis, field))
        else:
            field = calto.field

        # request plots per spw
        super(SpwSummaryChart, self).__init__(
                context, result, calto, yaxis, intent=intent, field=field, **kwargs)


class TransmissionSummaryChart(SpwSummaryChart):
    """
    Create a transmission plot for each spw
    """
    def __init__(self, context, result, intent, **kwargs):
        plot_args = {
            'plotrange': None,
            'overlay': False,
            'showgrid': False,
            'verbose': False
        }
        plot_args.update(kwargs)

        super(TransmissionSummaryChart, self).__init__(
            context, result, yaxis='transmission', intent=intent, **plot_args)
