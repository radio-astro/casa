import numpy
import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays.singledish.sparsemap as sparsemap

LOG = infrastructure.get_logger(__name__)

def analyze_plot_table(context, antid, spwid, polid, plot_table):
    datatable = context.observing_run.datatable_instance
    num_rows = len(plot_table) # num_plane * num_ra * num_dec
    num_dec = plot_table[-1][1] + 1
    num_ra = plot_table[-1][0] + 1
    num_plane = num_rows / (num_dec * num_ra)
    LOG.debug('num_ra=%s, num_dec=%s, num_plane=%s, num_rows=%s'%(num_ra,num_dec,num_plane,num_rows))
    each_grid = (range(i*num_plane, (i+1)*num_plane) for i in xrange(num_dec * num_ra))
    rowlist = [{} for i in xrange(num_dec * num_ra)]
    for row_index, each_plane in enumerate(each_grid):
        dataids = numpy.array([], dtype=int)
        for plot_table_rowid in each_plane:
            plot_table_row = plot_table[plot_table_rowid]
            LOG.debug('Process row %s: ra=%s, dec=%s'%(plot_table_rowid, plot_table_row[2], plot_table_row[3]))
            dataids = numpy.concatenate((dataids, plot_table_row[-1]))
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
        LOG.trace('RA %s DEC %s: dataids=%s'%(raid, decid, dataids))
        
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
            reference_data = context.observing_run[antid]
            beam_size = casatools.quanta.convert(reference_data.beam_size[spwid], outunit='deg')['value']
            increment_ra = beam_size / dec_corr
    if num_dec > 1:
        LOG.trace('num_dec > 1 (%s)'%(num_dec))
        increment_dec = plot_table[num_plane * num_ra][3] - plot_table[0][3]
    else:
        LOG.trace('num_dec is 1')
        dec = plot_table[0][3]
        dec_corr = numpy.cos(dec * casatools.quanta.constants('pi')['value'] / 180.0)
        LOG.trace('declination correction factor is %s'%(dec_corr))
        increment_dec = increment_ra * dec_corr
    increment_list = [-increment_ra, increment_dec]
    LOG.debug('refpix_list=%s'%(refpix_list))
    LOG.debug('refval_list=%s'%(refval_list))
    LOG.debug('increment_list=%s'%(increment_list))
    
    return num_ra, num_dec, num_plane, refpix_list, refval_list, increment_list, rowlist 

def create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment):
    plotter = sparsemap.SDSparseMapPlotter(nh=num_ra, nv=num_dec, step=1, brightnessunit='K')
    plotter.setup_labels(refpix, refval, increment)
    return plotter
    
def get_data(infile, datatable, num_ra, num_dec, num_chan, rowlist):
    integrated_data = numpy.zeros(num_chan, dtype=float)
    map_data = numpy.zeros((num_ra, num_dec, num_chan), dtype=float) + sparsemap.NoDataThreshold
    nrow = 0
    for d in rowlist:
        ix = num_ra - 1 - d['RAID']
        iy = d['DECID']
        idxs = d['IDS']
        if len(idxs) > 0:
            midx = d['MEDIAN_INDEX']
            median_row = datatable.tb1.getcell('ROW', idxs[midx])
            LOG.debug('median row for (%s,%s) is %s'%(ix, iy, median_row))
            nrow += len(idxs)
            with casatools.TableReader(infile) as tb:
                map_data[ix,iy,:] = tb.getcell('SPECTRA', median_row)
                for row in (datatable.tb1.getcell('ROW', i) for i in idxs):
                    integrated_data += tb.getcell('SPECTRA', row)
        else:
            LOG.debug('no data is available for (%s,%s)'%(ix,iy))
    integrated_data /= nrow
    LOG.trace('integrated_data=%s'%(integrated_data))
    LOG.trace('map_data.shape=%s'%(list(map_data.shape)))
    LOG.trace('map_data[0][0].shape=%s'%(map_data[0][0].shape))

    return integrated_data, map_data

def get_lines(datatable, num_ra, rowlist):
    lines_map = collections.defaultdict(dict)
    for d in rowlist:
        ix = num_ra - 1 - d['RAID']
        iy = d['DECID']
        ids = d['IDS']
        midx = d['MEDIAN_INDEX']
        if midx is not None:
            masklist = datatable.getcell('MASKLIST', ids[midx])
            lines_map[ix][iy] = None if len(masklist) == 0 else masklist
        else:
            lines_map[ix][iy] = None
    return lines_map

def plot_profile_map(context, antid, spwid, polid, plot_table, infile, outfile, line_range):
    datatable = context.observing_run.datatable_instance

    num_ra, num_dec, num_plane, refpix, refval, increment, rowlist = analyze_plot_table(context, antid, spwid, polid, plot_table)
        
    plotter = create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment)
    
    nchan = context.observing_run[antid].spectral_window[spwid].num_channels
    LOG.debug('nchan=%s'%(nchan))
    
    integrated_data, map_data= get_data(infile, datatable, num_ra, num_dec, nchan, rowlist)
    lines_map = get_lines(datatable, num_ra, rowlist)

    spw = context.observing_run[antid].spectral_window[spwid]
    frequency = numpy.array([spw.refval + (i - spw.refpix) * spw.increment for i in xrange(nchan)]) * 1.0e-9    
    LOG.debug('frequency=%s~%s (nchan=%s)'%(frequency[0], frequency[-1], len(frequency)))
    plotter.setup_lines(line_range, lines_map)
    plotter.setup_reference_level(0.0)
    plotter.plot(map_data, integrated_data, frequency, figfile=outfile)
    plotter.done()
    return integrated_data, map_data

def plot_profile_map_with_fit(context, antid, spwid, polid, plot_table, prefit_data, postfit_data, prefit_figfile, postfit_figfile, line_range):
    """
    plot_table format:
    [[0, 0, RA0, DEC0, [IDX00, IDX01, ...]],
     [0, 1, RA0, DEC1, [IDX10, IDX11, ...]],
     ...]
    """
    datatable = context.observing_run.datatable_instance

    num_ra, num_dec, num_plane, refpix, refval, increment, rowlist = analyze_plot_table(context, antid, spwid, polid, plot_table)
        
    plotter = create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment)
    
    nchan = context.observing_run[antid].spectral_window[spwid].num_channels
    LOG.debug('nchan=%s'%(nchan))
    
    spw = context.observing_run[antid].spectral_window[spwid]
    frequency = numpy.array([spw.refval + (i - spw.refpix) * spw.increment for i in xrange(nchan)]) * 1.0e-9    
    LOG.debug('frequency=%s~%s (nchan=%s)'%(frequency[0], frequency[-1], len(frequency)))

    postfit_integrated_data, postfit_map_data = get_data(postfit_data, datatable, num_ra, num_dec, nchan, rowlist)
    lines_map = get_lines(datatable, num_ra, rowlist)

    # plot post-fit spectra
    plotter.setup_reference_level(0.0)
    #plotter.set_global_scaling()
    plotter.plot(postfit_map_data, postfit_integrated_data, frequency, figfile=postfit_figfile)

    del postfit_integrated_data
    
    prefit_integrated_data, prefit_map_data = get_data(prefit_data, datatable, num_ra, num_dec, nchan, rowlist)
    
    # fit_result shares its storage with postfit_map_data to reduce memory usage
    fit_result = postfit_map_data
    for x in xrange(num_ra):
        for y in xrange(num_dec):
            prefit = prefit_map_data[x][y]
            if not all(prefit == sparsemap.NoDataThreshold):
                postfit = postfit_map_data[x][y]
                fit_result[x][y] = prefit - postfit
            else:
                fit_result[x][y][:] = sparsemap.NoDataThreshold
    
    
    plotter.setup_lines(line_range, lines_map)
    
    # plot pre-fit spectra
    plotter.setup_reference_level(None)
    #plotter.unset_global_scaling()
    plotter.plot(prefit_map_data, prefit_integrated_data, frequency, fit_result=fit_result, figfile=prefit_figfile)
            
    plotter.done()
    
    del prefit_integrated_data, prefit_map_data, postfit_map_data, fit_result
    
    return True

def median_index(arr):
    if not numpy.iterable(arr) or len(arr) == 0:
        return numpy.nan
    else:
        sorted_index = numpy.argsort(arr)
        if len(arr) < 3:
            return sorted_index[0]
        else:
            return sorted_index[len(arr) / 2]
