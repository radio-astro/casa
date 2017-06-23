import os
import numpy
import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.displays.singledish.sparsemap as sparsemap
from pipeline.infrastructure.displays.singledish.utils import sd_polmap
from pipeline.domain import DataTable
from ..common import utils 
from pipeline.infrastructure.displays.singledish import atmutil

_LOG = infrastructure.get_logger(__name__)
LOG = utils.OnDemandStringParseLogger(_LOG)

class PlotterPool(object):
    def __init__(self):
        self.pool = {}
        self.figure_id = sparsemap.SparseMapAxesManager.MATPLOTLIB_FIGURE_ID()
    
    def create_plotter(self, num_ra, num_dec, num_plane, refpix, refval, increment):
#         key = (num_ra, num_dec)
#         if self.pool.has_key(key):
#             LOG.info('Reuse existing plotter: (nra, ndec) = {}', key)
#             plotter = self.pool[key]
#         else:
#             LOG.info('Create plotter for (nra, ndec) = {}', key)
#             fignums = pl.get_fignums()
#             while self.figure_id in fignums:
#                 self.figure_id += 1
#             plotter = sparsemap.SDSparseMapPlotter(nh=num_ra, nv=num_dec, 
#                                                    step=1, brightnessunit='Jy/beam',
#                                                    figure_id=self.figure_id)
#             self.pool[key] = plotter
#         plotter.setup_labels(refpix, refval, increment)
        plotter = sparsemap.SDSparseMapPlotter(nh=num_ra, nv=num_dec, 
                                               step=1, brightnessunit='Jy/beam',
                                               figure_id=self.figure_id)
        plotter.setup_labels(refpix, refval, increment)
        return plotter
    
    def done(self):
        for plotter in self.pool.values():
            plotter.done()
        
class PlotDataStorage(object):
    def __init__(self):
        self.map_data_storage = numpy.zeros((0), dtype=float)
        self.integrated_data_storage = numpy.zeros((0), dtype=float)
        self.map_mask_storage = numpy.zeros((0), dtype=bool)
        self.integrated_mask_storage = numpy.zeros((0), dtype=bool)
        self.map_data = self.map_data_storage
        self.integrated_data = self.integrated_data_storage
        self.map_mask = self.map_mask_storage
        self.integrated_mask = self.integrated_mask_storage

    def resize_storage(self, num_ra, num_dec, num_pol, num_chan):
        num_integrated = num_pol * num_chan
        num_map = num_ra * num_dec * num_integrated
        if len(self.map_data_storage) < num_map:
            self.map_data_storage = numpy.resize(self.map_data_storage, num_map)
        self.map_data = numpy.reshape(self.map_data_storage[:num_map], (num_ra, num_dec, num_pol, num_chan))
        
        if len(self.map_mask_storage) < num_map:
            self.map_mask_storage = numpy.resize(self.map_mask_storage, num_map)
        self.map_mask = numpy.reshape(self.map_mask_storage[:num_map], (num_ra, num_dec, num_pol, num_chan))
        
        if len(self.integrated_data_storage) < num_integrated:
            self.integrated_data_storage = numpy.resize(self.integrated_data_storage, num_integrated)
        self.integrated_data = numpy.reshape(self.integrated_data_storage[:num_integrated], (num_pol, num_chan))
            
class BaselineSubtractionPlotManager(object):
    @staticmethod
    def generate_plot_table(ms_id, antenna_id, spw_id, polarization_ids, grid_table):
        def _filter(msid, ant, spw, pols, table):
            for row in table:
                if row[0] == spw and row[1] in pols:
                    new_row_entry = row[2:6] + [numpy.array([r[3] for r in row[6] if r[-1] == msid and r[-2] == ant], dtype=int)]
                    yield new_row_entry
        new_table = list(_filter(ms_id, antenna_id, spw_id, polarization_ids, grid_table))
        return new_table
    
    def __init__(self, context, datatable):
        self.context = context
        self.datatable = datatable
        stage_number = self.context.task_counter
        self.stage_dir = os.path.join(self.context.report_dir,"stage%d" % stage_number)    
        if not os.path.exists(self.stage_dir):
            os.makedirs(self.stage_dir)

        self.pool = PlotterPool()
        self.prefit_storage = PlotDataStorage()
        self.postfit_storage = PlotDataStorage()
        
    def initialize(self, ms, blvis):
        self.ms = ms
        
        self.rowmap = utils.make_row_map(ms, blvis)
        self.prefit_data = ms.name
        self.postfit_data = blvis
        
        return True
    
    def finalize(self):
        self.pool.done()
        
    def resize_storage(self, num_ra, num_dec, num_pol, num_chan):
        self.prefit_storage.resize_storage(num_ra, num_dec, num_pol, num_chan)
        self.postfit_storage.resize_storage(num_ra, num_dec, num_pol, num_chan)
    
    def plot_spectra_with_fit(self, field_id, antenna_id, spw_id, 
                              grid_table=None, deviation_mask=None, channelmap_range=None,
                              showatm=True):
        if grid_table is None:
            return []
        
        # convert channelmap_range to plotter-aware format
        if channelmap_range is None:
            line_range = None
        else:
            line_range = [[r[0] - 0.5 * r[1], r[0] + 0.5 * r[1]] for r in channelmap_range if r[2] is True]
            if len(line_range) == 0:
                line_range = None
        
        self.field_id = field_id
        self.antenna_id = antenna_id
        self.spw_id = spw_id
        ms_id = self.context.observing_run.measurement_sets.index(self.ms)
        data_desc = self.ms.get_data_description(spw=spw_id)
        num_pol = data_desc.num_polarizations
        self.pol_list = numpy.arange(num_pol, dtype=int)
        self.plot_table = self.generate_plot_table(ms_id, antenna_id, spw_id, self.pol_list, grid_table)

        source_name = self.ms.fields[self.field_id].source.name.replace(' ', '_').replace('/','_')
        LOG.debug('Generating plots for source {} ant {} spw {}',
                  source_name, self.antenna_id, self.spw_id)
        
        outprefix_template = lambda x: 'spectral_plot_%s_subtraction_%s_%s_ant%s_spw%s'%(x,
                                                                                         '.'.join(self.ms.basename.split('.')[:-1]),
                                                                                         source_name,
                                                                                         self.antenna_id,
                                                                                         self.spw_id)
        prefit_prefix = os.path.join(self.stage_dir, outprefix_template('before'))
        postfit_prefix = os.path.join(self.stage_dir, outprefix_template('after'))
        LOG.debug('prefit_prefix=\'{}\'', os.path.basename(prefit_prefix))
        LOG.debug('postfit_prefix=\'{}\'', os.path.basename(postfit_prefix))
        
        if showatm is True:
            atm_freq, atm_transmission = atmutil.get_transmission(vis=self.ms.name, antenna_id=self.antenna_id,
                                                        spw_id=self.spw_id, doplot=False)
        else:
            atm_transmission = None
            atm_freq = None
        plot_list = self.plot_profile_map_with_fit(prefit_prefix, postfit_prefix, 
                                                   deviation_mask, line_range,
                                                   atm_transmission, atm_freq)
        ret = []
        for (plot_type, plots) in plot_list.iteritems():
            if plot_type == 'pre_fit':
                ptype = 'sd_sparse_map_before_subtraction'
                data = self.prefit_data
            else:
                ptype = 'sd_sparse_map_after_subtraction'
                data = self.postfit_data
            for (pol, figfile) in plots.iteritems():
                if os.path.exists(figfile):
                    parameters = {'intent': 'TARGET',
                                  'spw': self.spw_id,
                                  'pol': sd_polmap[pol],
                                  'ant': self.ms.antennas[self.antenna_id].name,
                                  'vis': self.ms.basename,
                                  'type': ptype,
                                  'file': data}
                    plot = logger.Plot(figfile,
                                       x_axis='Frequency',
                                       y_axis='Intensity',
                                       field=source_name,
                                       parameters=parameters)
                    ret.append(plot)
        return ret
    
    def plot_profile_map_with_fit(self, prefit_figfile_prefix, postfit_figfile_prefix, 
                                  deviation_mask, line_range, atm_transmission, atm_frequency):
        """
        plot_table format:
        [[0, 0, RA0, DEC0, [IDX00, IDX01, ...]],
         [0, 1, RA0, DEC1, [IDX10, IDX11, ...]],
         ...]
        """
        ms = self.ms
        antid = self.antenna_id
        spwid = self.spw_id
        plot_table = self.plot_table
        prefit_data = self.prefit_data
        postfit_data = self.postfit_data
        rowmap = self.rowmap
        
        dtrows = self.datatable.getcol('ROW')
    
        num_ra, num_dec, num_plane, refpix, refval, increment, rowlist = analyze_plot_table(self.datatable, dtrows, ms, antid, spwid, plot_table)
            
        plotter = self.pool.create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment)
        LOG.info('vis {} ant {} spw {} plotter figure id {} has {} axes', ms.basename, antid, spwid, plotter.axes.figure_id, len(plotter.axes.figure.axes))
        LOG.info('axes list: {}', [x.__hash__()  for x in plotter.axes.figure.axes])
        spw = ms.spectral_windows[spwid]
        nchan = spw.num_channels
        data_desc = ms.get_data_description(spw=spw)
        npol = data_desc.num_polarizations
        LOG.debug('nchan={}', nchan)
        
        self.resize_storage(num_ra, num_dec, npol, nchan) 
        
        frequency = numpy.fromiter((spw.channels.chan_freqs[i] * 1.0e-9 for i in xrange(nchan)), dtype=numpy.float64) # unit in GHz
        LOG.debug('frequency={}~{} (nchan={})',
                  frequency[0], frequency[-1], len(frequency))
    
        if rowmap is None:
            rowmap = utils.make_row_map(ms, postfit_data)
        postfit_integrated_data, postfit_map_data = get_data(postfit_data, dtrows, 
                                                             num_ra, num_dec, nchan, npol,
                                                             rowlist, rowmap=rowmap,
                                                             integrated_data_storage=self.postfit_storage.integrated_data,
                                                             map_data_storage=self.postfit_storage.map_data,
                                                             map_mask_storage=self.postfit_storage.map_mask)
        if line_range is not None:
            lines_map = get_lines(self.datatable, num_ra, rowlist)
        else:
            lines_map = None
    
        plot_list = {}
    
        # plot post-fit spectra
        plot_list['post_fit'] = {}
        plotter.setup_lines(line_range, lines_map)
        plotter.setup_reference_level(0.0)
        plotter.set_deviation_mask(deviation_mask)
        plotter.set_atm_transmission(atm_transmission, atm_frequency)
        plotter.set_global_scaling()
        for ipol in xrange(npol):
            postfit_figfile = postfit_figfile_prefix + '_pol%s.png'%(ipol)
            #LOG.info('#TIMING# Begin SDSparseMapPlotter.plot(postfit,pol%s)'%(ipol))
            plotter.plot(postfit_map_data[:,:,ipol,:], 
                         postfit_integrated_data[ipol], 
                         frequency, figfile=postfit_figfile)
            #LOG.info('#TIMING# End SDSparseMapPlotter.plot(postfit,pol%s)'%(ipol))
            if os.path.exists(postfit_figfile):
                plot_list['post_fit'][ipol] = postfit_figfile
    
        del postfit_integrated_data
        
        prefit_integrated_data, prefit_map_data = get_data(prefit_data, dtrows, 
                                                           num_ra, num_dec, 
                                                           nchan, npol, rowlist,
                                                           integrated_data_storage=self.prefit_storage.integrated_data,
                                                           map_data_storage=self.prefit_storage.map_data,
                                                           map_mask_storage=self.prefit_storage.map_mask)
        
        # fit_result shares its storage with postfit_map_data to reduce memory usage
        fit_result = postfit_map_data
        for x in xrange(num_ra):
            for y in xrange(num_dec):
                prefit = prefit_map_data[x][y]
                if not numpy.all(prefit == sparsemap.NoDataThreshold):
                    postfit = postfit_map_data[x][y]
                    fit_result[x,y] = prefit - postfit
                else:
                    fit_result[x,y,::] = sparsemap.NoDataThreshold
        
        
        # plot pre-fit spectra
        plot_list['pre_fit'] = {}
        plotter.setup_reference_level(None) 
        plotter.unset_global_scaling()
        for ipol in xrange(npol):
            prefit_figfile = prefit_figfile_prefix + '_pol%s.png'%(ipol)
            #LOG.info('#TIMING# Begin SDSparseMapPlotter.plot(prefit,pol%s)'%(ipol))
            plotter.plot(prefit_map_data[:,:,ipol,:], 
                         prefit_integrated_data[ipol], 
                         frequency, fit_result=fit_result[:,:,ipol,:], figfile=prefit_figfile)
            #LOG.info('#TIMING# End SDSparseMapPlotter.plot(prefit,pol%s)'%(ipol))
            if os.path.exists(prefit_figfile):
                plot_list['pre_fit'][ipol] = prefit_figfile
                        
        del prefit_integrated_data, prefit_map_data, postfit_map_data, fit_result
        
        plotter.done()
        
        return plot_list
        
#@utils.profiler
def analyze_plot_table(datatable, dtrows, ms, antid, spwid, plot_table):
    #datatable = context.observing_run.datatable_instance
    num_rows = len(plot_table) # num_plane * num_ra * num_dec
    num_dec = plot_table[-1][1] + 1
    num_ra = plot_table[-1][0] + 1
    num_plane = num_rows / (num_dec * num_ra)
    LOG.debug('num_ra={}, num_dec={}, num_plane={}, num_rows={}',
              num_ra,num_dec,num_plane,num_rows)
    each_grid = (range(i*num_plane, (i+1)*num_plane) for i in xrange(num_dec * num_ra))
    rowlist = [{} for i in xrange(num_dec * num_ra)]
    for row_index, each_plane in enumerate(each_grid):
        def g():
            for plot_table_rowid in each_plane:
                plot_table_row = plot_table[plot_table_rowid]
                LOG.debug('Process row {}: ra={}, dec={}',
                          plot_table_rowid, plot_table_row[2],
                          plot_table_row[3])
                for i in plot_table_row[-1]:
                    # MS stores multiple polarization components in one cell 
                    # so it is not necessary to check polarization id
                    LOG.trace('Adding {} to dataids', i)
                    yield i
        dataids = numpy.fromiter(g(), dtype=int)
        if len(dataids) > 0:
            midx = median_index(dataids)
        else:
            midx = None
        raid = plot_table[each_plane[0]][0]
        decid = plot_table[each_plane[0]][1]
        ra = plot_table[each_plane[0]][2]
        dec = plot_table[each_plane[0]][3]
        rowlist[row_index].update(
                {"RAID": raid, "DECID": decid, "RA": ra, "DEC": dec,
                 "IDS": dataids, "MEDIAN_INDEX": midx})
        LOG.trace('RA {} DEC {}: dataids={}',
                  raid, decid, dataids)
        
    refpix_list = [0,0]
    refval_list = plot_table[num_ra * num_plane -1][2:4]
    if num_ra > 1:
        increment_ra = plot_table[num_plane][2] - plot_table[0][2]
    else:
        dec = plot_table[0][5]
        dec_corr = numpy.cos(dec * casatools.quanta.constants('pi')['value'] / 180.0)
        if num_dec > 1:
            increment_ra = plot_table[num_plane * num_ra][3] - plot_table[0][3] / dec_corr
        else:
            reference_data = ms
            beam_size = casatools.quanta.convert(reference_data.beam_sizes[antid][spwid], outunit='deg')['value']
            increment_ra = beam_size / dec_corr
    if num_dec > 1:
        LOG.trace('num_dec > 1 ({})', num_dec)
        increment_dec = plot_table[num_plane * num_ra][3] - plot_table[0][3]
    else:
        LOG.trace('num_dec is 1')
        dec = plot_table[0][3]
        dec_corr = numpy.cos(dec * casatools.quanta.constants('pi')['value'] / 180.0)
        LOG.trace('declination correction factor is {}', dec_corr)
        increment_dec = increment_ra * dec_corr
    increment_list = [-increment_ra, increment_dec]
    LOG.debug('refpix_list={}', refpix_list)
    LOG.debug('refval_list={}', refval_list)
    LOG.debug('increment_list={}', increment_list)
    
    return num_ra, num_dec, num_plane, refpix_list, refval_list, increment_list, rowlist 

# #@utils.profiler
# def create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment):
#     plotter = sparsemap.SDSparseMapPlotter(nh=num_ra, nv=num_dec, step=1, brightnessunit='Jy/beam')
#     plotter.setup_labels(refpix, refval, increment)
#     return plotter
#     

#@utils.profiler
def get_data(infile, dtrows, num_ra, num_dec, num_chan, num_pol, rowlist, rowmap=None,
             integrated_data_storage=None, integrated_mask_storage=None, 
             map_data_storage=None, map_mask_storage=None):
    # default rowmap is EchoDictionary
    if rowmap is None:
        rowmap = utils.EchoDictionary()

    integrated_shape = (num_pol, num_chan)
    map_shape = (num_ra, num_dec, num_pol, num_chan)
    if integrated_data_storage is not None:
        assert integrated_data_storage.shape == integrated_shape
        assert integrated_data_storage.dtype == float
        integrated_data = integrated_data_storage
        integrated_data[:] = 0.0
    else:
        integrated_data = numpy.zeros((num_pol, num_chan), dtype=float)

    num_accumulated = numpy.zeros((num_pol, num_chan), dtype=int)

    if map_data_storage is not None:
        assert map_data_storage.shape == map_shape
        assert map_data_storage.dtype == float
        map_data = map_data_storage
        map_data[:] = sparsemap.NoDataThreshold
    else:
        map_data = numpy.zeros((num_ra, num_dec, num_pol, num_chan), dtype=float) + sparsemap.NoDataThreshold
    if map_mask_storage is not None:
        assert map_mask_storage.shape == map_shape
        assert map_mask_storage.dtype == bool
        map_mask = map_mask_storage
        map_mask[:] = False
    else:
        map_mask = numpy.zeros((num_ra, num_dec, num_pol, num_chan), dtype=bool)
    nrow = 0
    
    # column name for spectral data
    with casatools.TableReader(infile) as tb:
        colnames = ['CORRECTED_DATA', 'DATA', 'FLOAT_DATA']
        colname = None
        for name in colnames:
            if name in tb.colnames():
                colname = name
                break
    assert colname is not None
        
    with casatools.TableReader(infile) as tb:
        for d in rowlist:
            ix = num_ra - 1 - d['RAID']
            iy = d['DECID']
            idxs = d['IDS']
            if len(idxs) > 0:
                midx = d['MEDIAN_INDEX']
                median_row = dtrows[idxs[midx]]
                mapped_row = rowmap[median_row]
                LOG.debug('median row for ({},{}) is {} (mapped to {})',
                          ix, iy, median_row, mapped_row)
                nrow += len(idxs)
                this_data = tb.getcell(colname, mapped_row)
                this_mask = tb.getcell('FLAG', mapped_row)
                map_data[ix,iy] = this_data.real
                map_mask[ix,iy] = this_mask
                # to access MS rows in sorted order (avoid jumping distant row, accessing back and forth)
                rows = dtrows[idxs].copy()
                rows.sort()
                for row in rows:
                    mapped_row = rowmap[row]
                    LOG.debug('row {}: mapped_row {}', row, mapped_row)
                    this_data = tb.getcell(colname, mapped_row)
                    this_mask = tb.getcell('FLAG', mapped_row)
                    LOG.trace('this_mask.shape={}', this_mask.shape)
                    LOG.trace('all(this_mask==True) = {}',
                              numpy.all(this_mask==True))
                    binary_mask = numpy.asarray(numpy.logical_not(this_mask), dtype=int)
                    integrated_data += this_data.real * binary_mask 
                    num_accumulated += binary_mask 
            else:
                LOG.debug('no data is available for ({},{})', ix,iy)
    integrated_data_masked = numpy.ma.masked_array(integrated_data, num_accumulated == 0)
    integrated_data_masked /= num_accumulated
    map_data_masked = numpy.ma.masked_array(map_data, map_mask)
    LOG.trace('integrated_data={}', integrated_data)
    LOG.trace('num_accumulated={}', num_accumulated)
    LOG.trace('map_data.shape={}', map_data.shape)

    return integrated_data_masked, map_data_masked

def get_lines(datatable, num_ra, rowlist):
    lines_map = collections.defaultdict(dict)
    #with casatools.TableReader(rwtablename) as tb:
    for d in rowlist:
        ix = num_ra - 1 - d['RAID']
        iy = d['DECID']
        ids = d['IDS']
        midx = d['MEDIAN_INDEX']
        if midx is not None:
            masklist = datatable.getcell('MASKLIST', ids[midx])
            lines_map[ix][iy] = None if (len(masklist) == 0 or numpy.all(masklist == -1))else masklist
        else:
            lines_map[ix][iy] = None
    return lines_map

# @utils.profiler
# def plot_profile_map_with_fit(context, ms, antid, spwid, plot_table, prefit_data, postfit_data, prefit_figfile_prefix, postfit_figfile_prefix, deviation_mask, line_range,
#                               rowmap=None):
#     """
#     plot_table format:
#     [[0, 0, RA0, DEC0, [IDX00, IDX01, ...]],
#      [0, 1, RA0, DEC1, [IDX10, IDX11, ...]],
#      ...]
#     """
#     #datatable = DataTable(context.observing_run.ms_datatable_name)
#     rotablename = DataTable.get_rotable_name(context.observing_run.ms_datatable_name)
#     rwtablename = DataTable.get_rwtable_name(context.observing_run.ms_datatable_name)
#     with casatools.TableReader(rotablename) as tb:
#         dtrows = tb.getcol('ROW')
# 
#     num_ra, num_dec, num_plane, refpix, refval, increment, rowlist = analyze_plot_table(context, dtrows, ms, antid, spwid, plot_table)
#         
#     plotter = create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment)
#     
#     spw = ms.spectral_windows[spwid]
#     nchan = spw.num_channels
#     data_desc = ms.get_data_description(spw=spw)
#     npol = data_desc.num_polarizations
#     LOG.debug('nchan={}', nchan)
#     
#     frequency = numpy.fromiter((spw.channels.chan_freqs[i] * 1.0e-9 for i in xrange(nchan)), dtype=numpy.float64) # unit in GHz
#     LOG.debug('frequency={}~{} (nchan={})',
#               frequency[0], frequency[-1], len(frequency))
# 
#     if rowmap is None:
#         rowmap = utils.make_row_map(ms, postfit_data)
#     postfit_integrated_data, postfit_map_data = get_data(postfit_data, dtrows, 
#                                                          num_ra, num_dec, nchan, npol,
#                                                          rowlist, rowmap=rowmap)
#     lines_map = get_lines(rwtablename, num_ra, rowlist)
# 
#     plot_list = {}
# 
#     # plot post-fit spectra
#     plot_list['post_fit'] = {}
#     plotter.setup_lines(line_range, lines_map)
#     plotter.setup_reference_level(0.0)
#     plotter.set_deviation_mask(deviation_mask)
#     plotter.set_global_scaling()
#     for ipol in xrange(npol):
#         postfit_figfile = postfit_figfile_prefix + '_pol%s.png'%(ipol)
#         LOG.info('#TIMING# Begin SDSparseMapPlotter.plot(postfit,pol%s)'%(ipol))
#         plotter.plot(postfit_map_data[:,:,ipol,:], 
#                      postfit_integrated_data[ipol], 
#                      frequency, figfile=postfit_figfile)
#         LOG.info('#TIMING# End SDSparseMapPlotter.plot(postfit,pol%s)'%(ipol))
#         if os.path.exists(postfit_figfile):
#             plot_list['post_fit'][ipol] = postfit_figfile
# 
#     del postfit_integrated_data
#     
#     prefit_integrated_data, prefit_map_data = get_data(prefit_data, dtrows, 
#                                                        num_ra, num_dec, 
#                                                        nchan, npol, rowlist)
#     
#     # fit_result shares its storage with postfit_map_data to reduce memory usage
#     fit_result = postfit_map_data
#     for x in xrange(num_ra):
#         for y in xrange(num_dec):
#             prefit = prefit_map_data[x][y]
#             if not numpy.all(prefit == sparsemap.NoDataThreshold):
#                 postfit = postfit_map_data[x][y]
#                 fit_result[x,y] = prefit - postfit
#             else:
#                 fit_result[x,y,::] = sparsemap.NoDataThreshold
#     
#     
#     # plot pre-fit spectra
#     plot_list['pre_fit'] = {}
#     plotter.setup_reference_level(None) 
#     plotter.unset_global_scaling()
#     for ipol in xrange(npol):
#         prefit_figfile = prefit_figfile_prefix + '_pol%s.png'%(ipol)
#         LOG.info('#TIMING# Begin SDSparseMapPlotter.plot(prefit,pol%s)'%(ipol))
#         plotter.plot(prefit_map_data[:,:,ipol,:], 
#                      prefit_integrated_data[ipol], 
#                      frequency, fit_result=fit_result[:,:,ipol,:], figfile=prefit_figfile)
#         LOG.info('#TIMING# End SDSparseMapPlotter.plot(prefit,pol%s)'%(ipol))
#         if os.path.exists(prefit_figfile):
#             plot_list['pre_fit'][ipol] = prefit_figfile
#             
#     plotter.done()
#     
#     del prefit_integrated_data, prefit_map_data, postfit_map_data, fit_result
#     
#     return plot_list

def median_index(arr):
    if not numpy.iterable(arr) or len(arr) == 0:
        return numpy.nan
    else:
        sorted_index = numpy.argsort(arr)
        if len(arr) < 3:
            return sorted_index[0]
        else:
            return sorted_index[len(arr) / 2]
