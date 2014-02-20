from __future__ import absolute_import

import os
import time
import abc
import numpy
import math
import string
import pylab as pl

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from .utils import RADEClabel, RArotation, DECrotation, DDMMSSs, HHMMSSss
from .common import DPISummary, DPIDetail, SingleDishDisplayInputs, ShowPlot, draw_beam, LightSpeed
LOG = infrastructure.get_logger(__name__)

class ClusterValidationAxesManager(object):
    def __init__(self, ncluster, nh, nv, aspect_ratio,
                 xformatter, yformatter, xlocator, ylocator,
                 xrotation, yrotation, ticksize):
        self.ncluster = ncluster
        self.nh = nh
        self.nv = nv
        self.aspect_ratio = aspect_ratio
        self.xformatter = xformatter
        self.yformatter = yformatter
        self.xlocator = xlocator
        self.ylocator = ylocator
        self.xrotation = xrotation
        self.yrotation = yrotation
        self.ticksize = ticksize

        self._legend = None
        self._axes = None

    @property
    def axes_legend(self):
        if self._legend is None:
            self._legend = pl.axes([0.0, 0.85, 1.0, 0.15])
            self._legend.set_axis_off()

        return self._legend

        
    @property
    def axes_list(self):
        if self._axes is None:
            self._axes = list(self.__axes_list())

        return self._axes

    def __axes_list(self):
        for icluster in xrange(self.ncluster):
            x = icluster % self.nh
            y = int(icluster / self.nh)
            x1 = 1.0 / float(self.nh)
            if x == 0:
                x0 = x1 * (x + 0.1)
            else:
                #x0 = x1 * (x + 0.15)
                x0 = x1 * (x + 0.1)
            x1 *= 0.8
            y1 = 0.8 / float(self.nv)
            y0 = y1 * (self.nv - y - 1 + 0.3)
            if self.nv > 2:
                y1 *= 0.5
            else:
                y1 *= 0.6

            axes = pl.axes([x0, y0, x1, y1])
            # 2008/9/20 DEC Effect
            axes.set_aspect(self.aspect_ratio)
            #axes.set_aspect('equal')
            pl.xlabel('RA', size=self.ticksize)
            pl.ylabel('Dec', size=self.ticksize)
            axes.xaxis.set_major_formatter(self.xformatter)
            axes.yaxis.set_major_formatter(self.yformatter)
            axes.xaxis.set_major_locator(self.xlocator)
            axes.yaxis.set_major_locator(self.ylocator)
            xlabels = axes.get_xticklabels()
            pl.setp(xlabels, 'rotation', self.xrotation, fontsize=self.ticksize)
            ylabels = axes.get_yticklabels()
            pl.setp(ylabels, 'rotation', self.yrotation, fontsize=self.ticksize)
            pl.xticks(size=self.ticksize)
            pl.yticks(size=self.ticksize)

            
            yield axes
        

class ClusterDisplay(object):
    Inputs = SingleDishDisplayInputs

    def __init__(self, inputs):
        self.inputs = inputs

    @property
    def context(self):
        return self.inputs.context

    def __baselined(self):
        for group in self.inputs.result.outcome['baselined']:
            if group.has_key('clusters') and group.has_key('lines'):
                yield group
            
    def plot(self):
        plot_list = []

        stage_dir = os.path.join(self.context.report_dir,
                                 'stage%d'%(self.inputs.result.stage_number))
        start_time = time.time()
        for group in self.__baselined():
            cluster = group['clusters']
            if not cluster.has_key('cluster_score'):
                # it should be empty cluster (no detection) so skip this cycle
                continue
            spw = group['spw']
            group_id = group['group_id']
            iteration = group['iteration']
            t0 = time.time()
            plot_score = ClusterScoreDisplay(group_id, iteration, cluster, spw, stage_dir)
            plot_list.extend(plot_score.plot())
            t1 = time.time()
            plot_property = ClusterPropertyDisplay(group_id, iteration, cluster, spw, stage_dir)
            plot_list.extend(plot_property.plot())
            antenna = group['index'][0]
            lines = group['lines']
            t2 = time.time()
            plot_validation = ClusterValidationDisplay(self.context, group_id, iteration, cluster, spw, antenna, lines, stage_dir)
            plot_list.extend(plot_validation.plot())
            t3 = time.time()

            LOG.debug('PROFILE: ClusterScoreDisplay elapsed time is %s sec'%(t1-t0))
            LOG.debug('PROFILE: ClusterPropertyDisplay elapsed time is %s sec'%(t2-t1))
            LOG.debug('PROFILE: ClusterValidationDisplay elapsed time is %s sec'%(t3-t2))

        end_time = time.time()
        LOG.debug('PROFILE: plot elapsed time is %s sec'%(end_time-start_time))
        
        return plot_list

class ClusterDisplayWorker(object):
    __metaclass__ = abc.ABCMeta
    
    MATPLOTLIB_FIGURE_ID = 8907
    
    def __init__(self, group_id, iteration, cluster, spw, stage_dir):
        self.group_id = group_id
        self.iteration = iteration
        self.cluster = cluster
        self.spw = spw
        self.stage_dir = stage_dir

    def plot(self):
        if ShowPlot:
            pl.ion()
        else:
            pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        if ShowPlot:
            pl.ioff()

        pl.cla()
        pl.clf()

        return list(self._plot())

    def _create_plot(self, plotfile, type, x_axis, y_axis):
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = self.spw
        parameters['pol'] = 0
        parameters['ant'] = 'all'
        parameters['type'] = type
        plot_obj = logger.Plot(plotfile,
                               x_axis=x_axis,
                               y_axis=y_axis,
                               field='',
                               parameters=parameters)
        return plot_obj

    @abc.abstractmethod
    def _plot(self):
        raise NotImplementedError
        
class ClusterScoreDisplay(ClusterDisplayWorker):
    def _plot(self):
        ncluster, score = self.cluster['cluster_score']
        pl.plot(ncluster, score, 'bx', markersize=10)
        [xmin, xmax, ymin, ymax] = pl.axis()
        pl.xlabel('Number of Clusters', fontsize=11)
        pl.ylabel('Score (Lower is better)', fontsize=11)
        pl.title('Score are plotted versus number of the cluster', fontsize=11)
        pl.axis([0, xmax+1, ymin, ymax])
        
        if ShowPlot:
            pl.draw()

        plotfile = os.path.join(self.stage_dir,
                                'cluster_score_group%s_spw%s_iter%s.png'%(self.group_id,self.spw,self.iteration))
        pl.savefig(plotfile, format='png', dpi=DPIDetail)
        plot = self._create_plot(plotfile, 'cluster_score',
                                 'Number of Clusters', 'Score')
        yield plot
        
class ClusterPropertyDisplay(ClusterDisplayWorker):
    def _plot(self):
        lines = self.cluster['detected_lines']
        properties = self.cluster['cluster_property']
        scaling = self.cluster['cluster_scale']

        sorted_properties = sorted(properties)
        width = lines[:,0]
        center = lines[:,1]
        pl.plot(center, width, 'bs', markersize=1)
        [xmin, xmax, ymin, ymax] = pl.axis()
        axes = pl.gcf().gca()
        cluster_id = 0
        for [cx, cy, dummy, r] in sorted_properties:
            radius = r * scaling
            aspect = 1.0 / scaling
            x_base = cx
            y_base = cy * scaling
            draw_beam(axes, radius, aspect, x_base, y_base, offset=0)
            pl.text(x_base, y_base, str(cluster_id), fontsize=10, color='red')
            cluster_id += 1
        pl.xlabel('Line Center (Channel)', fontsize=11)
        pl.ylabel('Line Width (Channel)', fontsize=11)
        pl.axis([xmin-1, xmax+1, 0, ymax+1])
        pl.title('Clusters in the line Center-Width space\n\nRed Oval(s) shows each clustering region. Size of the oval represents cluster radius', fontsize=11)

        if ShowPlot:
            pl.draw()

        plotfile = os.path.join(self.stage_dir,
                                'cluster_property_group%s_spw%s_iter%s.png'%(self.group_id,self.spw,self.iteration))
        pl.savefig(plotfile, format='png', dpi=DPISummary)
        plot = self._create_plot(plotfile, 'clustering_property',
                                 'Line Center', 'Line Width')
        yield plot
        
class ClusterValidationDisplay(ClusterDisplayWorker):
    Description = {
        'detection': 'Clustering Analysis at Detection stage\n\nYellow Square: Single spectrum is detected in the grid\nCyan Square: More than one spectra are detected in the grid\n',
        'validation': 'Clustering Analysis at Validation stage\n\nValidation by the rate (Number of clustering member [Nmember] v.s. Number of total spectra belong to the Grid [Nspectra])\n Blue Square: Validated: Nmember > ${valid} x Nspectra\nCyan Square: Marginally validated: Nmember > ${marginal} x Nspectra\nYellow Square: Questionable: Nmember > ${questionable} x Nspectrum\n',
        'smoothing': 'Clustering Analysis at Smoothing stage\n\nBlue Square: Passed continuity check\nCyan Square: Border\nYellow Square: Questionable\n',
        'final': 'Clustering Analysis at Final stage\n\nGreen Square: Final Grid where the line protection channels are calculated and applied to the baseline subtraction\nBlue Square: Final Grid where the calculated line protection channels are applied to the baseline subtraction\n\nIsolated Grids are eliminated.\n'
    }

    def __init__(self, context, group_id, iteration, cluster, spw, antenna, lines, stage_dir):
        super(ClusterValidationDisplay, self).__init__(group_id, iteration, cluster, spw, stage_dir)
        self.context = context
        self.antenna = antenna
        self.lines = lines

    def _plot(self):
        pl.clf()
        
        marks = ['gs', 'bs', 'cs', 'ys']

        num_cluster = len(self.lines)
        num_panel_h = int(math.sqrt(num_cluster - 0.1)) + 1
        num_panel_v = num_panel_h
        ra0 = self.cluster['grid']['ra_min']
        dec0 = self.cluster['grid']['dec_min']
        scale_ra = self.cluster['grid']['grid_ra']
        scale_dec = self.cluster['grid']['grid_dec']

        # 2008/9/20 DEC Effect
        aspect_ratio = 1.0 / math.cos(dec0 / 180.0 * 3.141592653)


        # common message for legends
        scale_msg = self.__scale_msg(scale_ra, scale_dec, aspect_ratio)

        # Plotting routine
        nx = len(self.cluster['cluster_flag'][0])
        ny = len(self.cluster['cluster_flag'][0][0])
        xmin = ra0
        xmax = nx * scale_ra + xmin
        ymin = dec0
        ymax = ny * scale_dec + ymin

        marker_size = int(300.0 / (max(nx, ny * 1.414) * num_panel_h) + 1.0)
        tick_size = int(6 + (1 / num_panel_h) * 2)

        span = max(xmax - xmin, ymax - ymin)
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        axes_manager = ClusterValidationAxesManager(num_cluster,
                                                    num_panel_h,
                                                    num_panel_v,
                                                    aspect_ratio,
                                                    RAformatter,
                                                    DECformatter,
                                                    RAlocator,
                                                    DEClocator,
                                                    RArotation,
                                                    DECrotation,
                                                    tick_size)
        axes_list = axes_manager.axes_list
        axes_legend = axes_manager.axes_legend
        
        for (mode,data,threshold,description) in self.__stages():
            plot_objects = []
            
            for icluster in xrange(num_cluster):
                pl.gcf().sca(axes_list[icluster])
                
                xdata = []
                ydata = []
                for i in xrange(len(threshold)):
                    xdata.append([])
                    ydata.append([])
                for ix in xrange(nx):
                    for iy in xrange(ny):
                        for i in xrange(len(threshold)):
                            if data[icluster][ix][iy] == len(threshold) - i:
                                xdata[i].append(xmin + (0.5 + ix) * scale_ra)
                                ydata[i].append(ymin + (0.5 + iy) * scale_dec)
                                break

                # Convert Channel to Frequency and Velocity
                #ichan = self.lines[icluster][0] + 0.5
                (frequency, width) = self.__line_property(icluster)
                pl.title('Cluster%s: Center=%.4f GHz Width=%.1f km/s'%(icluster,frequency,width), fontsize=tick_size+1)
                if self.lines[icluster][2] != False:
                    for i in xrange(len(threshold)):
                        plot_objects.extend(
                            pl.plot(xdata[i], ydata[i], marks[4 - len(threshold) + i], markersize=marker_size)
                        )

                if self.lines[icluster][2] != True:
                    if num_panel_h > 2:
                        _tick_size = tick_size
                    else:
                        _tick_size = tick_size + 1
                    plot_objects.append(
                        pl.text(0.5 * (xmin + xmax), 0.5 * (ymin + ymax),
                                'INVALID CLUSTER',
                                horizontalalignment='center',
                                verticalalignment='center',
                                size=_tick_size)
                        )
                pl.axis([xmax, xmin, ymin, ymax])

                # Legends
                pl.gcf().sca(axes_legend)
                msg = description + scale_msg
                plot_objects.append(
                    pl.text(0.5, 0.5, msg, horizontalalignment='center', verticalalignment='center', size=8)
                    )

            if ShowPlot:
                pl.draw()

            plotfile = os.path.join(self.stage_dir,
                                    'cluster_group_%s_spw%s_iter%s_%s.png'%(self.group_id,self.spw,self.iteration,mode))
            pl.savefig(plotfile, format='png', dpi=DPISummary)

            for obj in plot_objects:
                obj.remove()
            
            plot = self._create_plot(plotfile, 'clustering_%s'%(mode),
                                     'R.A.', 'Dec.')
            yield plot

    def __stages(self):
        digits = {'detection': 1, 'validation': 10,
                  'smoothing': 100, 'final': 1000}
        for key in ['detection', 'validation', 'smoothing', 'final']:
            if self.cluster.has_key('cluster_flag'):
                # Pick up target digit
                _data = self.cluster['cluster_flag']
                _digit = digits[key]
                data = (_data / _digit) % 10
                LOG.debug('data=%s'%(data))
                threshold = self.cluster[key+'_threshold']
                desc = self.Description[key]
                if key == 'validation':
                    template = string.Template(desc)
                    valid = '%.1f'%(threshold[0])
                    marginal = '%.1f'%(threshold[1])
                    questionable = '%.1f'%(threshold[2])
                    desc = template.safe_substitute(valid=valid,
                                                    marginal=marginal,
                                                    questionable=questionable)
                yield (key,data,threshold,desc)

    def __line_property(self, icluster):
        spectral_window = self.context.observing_run[self.antenna].spectral_window[self.spw]
        refval = spectral_window.refval
        refpix = spectral_window.refpix
        increment = spectral_window.increment
        if len(spectral_window.rest_frequencies) > 0:
            rest_frequency = spectral_window.rest_frequencies[0]
        else:
            rest_frequency = refval
        
        # line property in channel
        line_center = self.lines[icluster][0] 
        line_width = self.lines[icluster][1]

        center_frequency = refval + (line_center - refpix) * increment
        width_in_frequency = abs(line_width * increment)

        center_frequency *= 1.0e-9 # Hz -> GHz
        width_in_velocity = width_in_frequency / rest_frequency * LightSpeed

        return (center_frequency, width_in_velocity)

    def __scale_msg(self, scale_ra, scale_dec, aspect_ratio):
        if scale_ra >= 1.0:
            unit = 'degree'
            scale_factor = 1.0
        elif scale_ra * 60.0 >= 1.0:
            unit = 'arcmin'
            scale_factor = 60.0
        else:
            unit = 'arcsec'
            scale_factor = 3600.0
        ra_text = scale_ra / aspect_ratio * scale_factor
        dec_text = scale_dec * scale_factor

        return 'Scale of the Square (Grid): %.1f x %.1f (%s)'%(ra_text, dec_text, unit)
