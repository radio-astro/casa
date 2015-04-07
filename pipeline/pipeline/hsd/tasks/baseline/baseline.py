from __future__ import absolute_import

import os
import shutil
import glob
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure.displays.singledish.utils import sd_polmap

from .. import common
from . import maskline
#from .fitting import FittingFactory
from . import fitting
from . import plotter

LOG = infrastructure.get_logger(__name__)

class SDBaselineInputs(common.SingleDishInputs):
    """
    Inputs for baseline subtraction
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, infiles=None, spw=None, pol=None,
                 linewindow=None, edge=None, broadline=None, fitorder=None,
                 fitfunc=None):
        self._init_properties(vars())
        for key in ['spw', 'pol']:
            val = getattr(self, key)
            if val is None or (val[0] == '[' and val[-1] == ']'):
                self._to_list([key])
        #self._to_list(['infiles', 'iflist', 'pollist', 'edge', 'linewindow'])
        self._to_list(['infiles', 'edge', 'linewindow'])
        self._to_bool('broadline')
        self._to_numeric('fitorder')
        if isinstance(self.fitorder, float):
            self.fitorder = int(self.fitorder)

    @property
    def antennalist(self):
        if type(self.infiles) == list:
            antennas = [self.context.observing_run.get_scantable(f).antenna.name 
                        for f in self.infiles]
            return list(set(antennas))
        else:
            return [self.context.observing_run.get_scantable(self.infiles).antenna.name]

##     def get_iflist(self, index):
##         spw = '' if self.spw is None else self.spw
##         return self._get_arg(spw, index)

##     def get_pollist(self, index):
##         pol = '' if self.pol is None else self.pol
##         return self._get_arg(pol, index)

##     def _get_arg(self, arg_list, index):
##         sel = self._to_casa_arg(arg_list, index)
##         return common.selection_to_list(sel)
            

class SDBaselineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDBaselineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDBaselineResults, self).merge_with_context(context)

        # increment iteration counter
        # register detected lines to reduction group member
        reduction_group = context.observing_run.reduction_group
        for b in self.outcome['baselined']:
            group_id = b['group_id']
            spw = b['spw']
            antenna = b['index']
            pols = b['pols']
            lines = b['lines']
            channelmap_range = b['channelmap_range']
            group_desc = reduction_group[group_id]
            for (ant,spw,pol) in zip(antenna,spw,pols):
                group_desc.iter_countup(ant, spw, pol)
                group_desc.add_linelist(lines, ant, spw, pol,
                                        channelmap_range=channelmap_range)
                st = context.observing_run[ant]
                st.work_data = st.baselined_name

    def _outcome_name(self):
        return ['%s: %s (spw=%s, pol=%s)'%(idx, name, b['spw'], b['pols'])
                for b in self.outcome['baselined']
                for (idx,name) in zip(b['index'], b['name'])]

class SDBaseline(common.SingleDishTaskTemplate):
    Inputs = SDBaselineInputs

    @common.datatable_setter
    def prepare(self):
        inputs = self.inputs
        context = inputs.context
        datatable = context.observing_run.datatable_instance
        reduction_group = context.observing_run.reduction_group
        infiles = inputs.infiles
        args = inputs.to_casa_args()
        st_names = context.observing_run.st_names
        file_index = [st_names.index(infile) for infile in infiles]

        window = [] if inputs.linewindow is None else inputs.linewindow
        edge = (0,0) if inputs.edge is None else inputs.edge
        broadline = False if inputs.broadline is None else inputs.broadline
        fitorder = 'automatic' if inputs.fitorder is None or inputs.fitorder < 0 else inputs.fitorder
        fitfunc = 'spline' if inputs.fitfunc is None else inputs.fitfunc
        
        dummy_suffix = "_temp"
        # Clear-up old temporary scantables (but they really shouldn't exist)
        self._clearup_dummy()
        
        baselined = []

        ifnos = numpy.array(datatable.getcol('IF'))
        polnos = numpy.array(datatable.getcol('POL'))
        srctypes = numpy.array(datatable.getcol('SRCTYPE'))
        antennas = numpy.array(datatable.getcol('ANTENNA'))

        # generate storage for baselined data
        self._generate_storage_for_baselined(context, reduction_group)

        # mkdir stage_dir if it doesn't exist
        stage_number = context.task_counter
        stage_dir = os.path.join(context.report_dir,"stage%d" % stage_number)
        if not os.path.exists(stage_dir):
            os.makedirs(stage_dir)

        # loop over reduction group
        #files = set()
        files_temp = {}
        plot_list = []
        for (group_id,group_desc) in reduction_group.items():
            LOG.debug('Processing Reduction Group %s'%(group_id))
            LOG.debug('Group Summary:')
            for m in group_desc:
                LOG.debug('\tAntenna %s Spw %s Pol %s'%(m.antenna, m.spw, m.pols))
            # assume all members have same spw and pollist
            first_member = group_desc[0]
            pols_list = list(common.pol_filter(group_desc, inputs.get_pollist))
            LOG.debug('pols_list=%s'%(pols_list))
            iteration = first_member.iteration[0]

            # reference data is first scantable 
            st = context.observing_run[first_member.antenna]

            # skip channel averaged spw
            nchan = group_desc.nchan
            if nchan == 1:
                LOG.info('Skip channel averaged spw %s.'%(first_member.spw))
                continue

                
            srctype = st.calibration_strategy['srctype']

            LOG.debug('spw=\'%s\''%(args['spw']))
            LOG.debug('antenna_list=%s'%(file_index))
            member_list = list(common.get_valid_members(group_desc, file_index, args['spw']))
            # skip this group if valid member list is empty
            if len(member_list) == 0:
                LOG.info('Skip reduction group %d'%(group_id))
                continue

            member_list.sort()
            antenna_list = [group_desc[i].antenna for i in member_list]
            spwid_list = [group_desc[i].spw for i in member_list]
            pols_tmp = [None if pols_list[i] == group_desc[i].pols else pols_list[i] \
                        for i in member_list]
            LOG.debug('pols_tmp=%s'%(pols_tmp))
            pols_list = [pols_list[i] for i in member_list]
            LOG.debug('pols_list=%s'%(pols_list))
            
            LOG.debug('Members to be processed:')
            for i in xrange(len(member_list)):
                LOG.debug('\tAntenna %s Spw %s Pol %s'%(antenna_list[i], spwid_list[i], pols_list[i]))

            index_list = list(common.get_index_list(datatable, antenna_list, spwid_list, pols_tmp, srctype))
            index_list.sort()

            LOG.debug('index_list=%s'%(index_list))
            if len(index_list) == 0:
                LOG.info('Skip reduction group %s'%(group_id))
                continue
            
            maskline_inputs = maskline.MaskLine.Inputs(context, antenna_list, spwid_list, iteration, 
                                                       index_list, window, edge, broadline)
            maskline_task = maskline.MaskLine(maskline_inputs)
            maskline_result = self._executor.execute(maskline_task, merge=True)
            detected_lines = maskline_result.outcome['detected_lines']
            channelmap_range = maskline_result.outcome['channelmap_range']
            cluster_info = maskline_result.outcome['cluster_info']
            grid_table = maskline_result.outcome['grid_table']

            # fit order determination and fitting
            fitter_cls = fitting.FittingFactory.get_fitting_class(fitfunc)

            # loop over file
            for (ant,spwid,pols) in zip(antenna_list, spwid_list, pols_list):
                if len(pols) == 0:
                    LOG.info('Skip Antenna %s Spw %s (polarization selection is null)'%(ant, spwid))
                    continue
                
                LOG.debug('Performing spectral baseline subtraction for Antenna %s Spw %s Pols %s'%(ant, spwid, pols))
                
                _iteration = group_desc.get_iteration(ant, spwid)
                outfile = self._get_dummy_name(context, ant)
                LOG.debug('pols=%s'%(pols))
                fitter_inputs = fitter_cls.Inputs(context, ant, spwid, pols, _iteration, 
                                                  fitorder, edge, outfile)
                fitter = fitter_cls(fitter_inputs)
                fitter_result = self._executor.execute(fitter, merge=True)
                # store temporal scantable name
                if not files_temp.has_key(ant):
                    files_temp[ant] = fitter_result.outcome.pop('outtable')
                    
                # generate plot for weblog
                # prefix for spectral plot before baseline subtraction
                st = context.observing_run[ant]
                # TODO: use proper source name when we can handle multiple source 
                source_name = ''
                for (source_id,source) in st.source.items():
                    if 'TARGET' in source.intents:
                        source_name = source.name.replace(' ', '_').replace('/','_')
                prefix = 'spectral_plot_before_subtraction_%s_%s_ant%s_spw%s'%('.'.join(st.basename.split('.')[:-1]),source_name,ant,spwid)
                plot_list.extend(self.plot_spectra(source_name, ant, spwid, pols, grid_table, 
                                                   context.observing_run[ant].name, stage_dir, prefix, channelmap_range))
                prefix = prefix.replace('before', 'after')
                plot_list.extend(self.plot_spectra(source_name, ant, spwid, pols, grid_table, outfile, stage_dir, prefix, channelmap_range))
                
            name_list = [context.observing_run[f].baselined_name
                         for f in antenna_list]
            baselined.append({'group_id': group_id, 'iteration': iteration,
                              'name': name_list, 'index': antenna_list,
                              'spw': spwid_list, 'pols': pols_list,
                              'lines': detected_lines,
                              'channelmap_range': channelmap_range,
                              'clusters': cluster_info})
            LOG.debug('cluster_info=%s'%(cluster_info))

        # replace working scantable with temporal baselined scantable
        for file_idx, temp_name in files_temp.items():
            blname = context.observing_run[file_idx].baselined_name
            if os.path.exists(blname) and os.path.exists(temp_name):
                shutil.rmtree(blname)
            shutil.move(temp_name, blname)

        outcome = {'baselined': baselined,
                   'edge': edge,
                   'plots': plot_list}
        results = SDBaselineResults(task=self.__class__,
                                    success=True,
                                    outcome=outcome)
                
#         #stage_number is taken care of by basetask.result_finaliser
#         if self.inputs.context.subtask_counter is 0: 
#             results.stage_number = self.inputs.context.task_counter - 1
#         else:
#             results.stage_number = self.inputs.context.task_counter 
                
        return results

    def analyse(self, result):
        return result

    def _generate_storage_for_baselined(self, context, reduction_group):
        for antenna in xrange(len(context.observing_run)):
#             reference = context.observing_run[antenna].name
            reference = context.observing_run[antenna].baseline_source
#             storage = context.observing_run[antenna].baselined_name
            storage = self._get_dummy_name(context, antenna)
            if not os.path.exists(storage):
                # generate
                self._generate_storage_from_reference(storage, reference)
            iter_counter_list = []
            for (id, desc) in reduction_group.items():
                for member in desc:
                    if member.antenna == antenna:
                        iter_counter_list.extend(member.iteration)
            LOG.debug('iter_counter_list=%s'%(iter_counter_list))
            if all(numpy.array(iter_counter_list) == 0):
                # generate
                self._generate_storage_from_reference(storage, reference)

    def _generate_storage_from_reference(self, storage, reference):
        LOG.debug('generating %s from %s'%(os.path.basename(storage), os.path.basename(reference)))
        with casatools.TableReader(reference) as tb:
            copied = tb.copy(storage, deep=True, returnobject=True)
            copied.close()

    @property
    def _dummy_suffix(self):
        return "_temp"
    
    def _get_dummy_name(self, context, idx):
        """Generate temporal scantable name."""
        return context.observing_run[idx].name + self._dummy_suffix
        
    def _clearup_dummy(self):
        remove_list = glob.glob("*"+self._dummy_suffix)
        for dummy in remove_list:
            LOG.debug("Removing old temprary file '%s'" % dummy)
            shutil.rmtree(dummy)
        del remove_list

    def plot_spectra(self, source, ant, spwid, pols, grid_table, infile, outdir, outprefix, channelmap_range):
        #plot_list = []
        st = self.inputs.context.observing_run[ant]
        line_range = [[r[0] - 0.5 * r[1], r[0] + 0.5 * r[1]] for r in channelmap_range if r[2] is True]
        if len(line_range) == 0:
            line_range = None
        for pol in pols:
            outfile = os.path.join(outdir, outprefix+'_pol%s.png'%(pol))
            status = plotter.plot_profile_map(self.inputs.context, ant, spwid, pol, grid_table, infile, outfile, line_range)
            if status and os.path.exists(outfile):
                #plot_list.append(outfile)
                if outprefix.find('spectral_plot_before_subtraction') == -1:
                    plottype = 'sd_sparse_map_after_subtraction'
                else:
                    plottype = 'sd_sparse_map_before_subtraction'
                parameters = {'intent': 'TARGET',
                              'spw': spwid,
                              'pol': sd_polmap[pol],
                              'ant': st.antenna.name,
                              'vis': st.ms.basename,
                              'type': plottype,
                              'file': infile}
                plot = logger.Plot(outfile,
                                   x_axis='Frequency',
                                   y_axis='Intensity',
                                   field=source,
                                   parameters=parameters)
                #plot_list.append(plot)
                yield plot
        #return plot_list