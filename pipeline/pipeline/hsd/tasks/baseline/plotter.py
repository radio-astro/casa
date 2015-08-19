import numpy
import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays.singledish.sparsemap as sparsemap

LOG = infrastructure.get_logger(__name__)

def analyze_grid_table(datatable, antid, spwid, polid, grid_table):
    num_rows = len(grid_table) # num_plane * num_ra * num_dec
    num_dec = grid_table[-1][3] + 1
    num_ra = grid_table[-1][2] + 1
    num_plane = num_rows / (num_dec * num_ra)
    LOG.debug('num_ra=%s, num_dec=%s, num_plane=%s, num_rows=%s'%(num_ra,num_dec,num_plane,num_rows))
    each_grid = numpy.arange(num_rows, dtype=int).reshape((num_dec*num_ra,num_plane))
    LOG.trace('each_grid=%s'%(each_grid))
    get_index = lambda x: (x[0], x[3], x[4])
    rowlist = []
    for each_plane in each_grid:
        datarows = []
        dataids = []
        for grid_table_rowid in each_plane:
            grid_table_row = grid_table[grid_table_rowid]
            thisspw = grid_table_row[0]
            LOG.debug('Process row %s: ra=%s, dec=%s'%(grid_table_rowid, grid_table_row[4], grid_table_row[5]))
            for grid_data in grid_table_row[-1]:
                row, idx, thisant = get_index(grid_data)
                thispol = datatable.tb1.getcell('POL', idx)
                LOG.debug('ROW=%s, IDX=%s, ANT=%s, SPW=%s, POL=%s'%(row, idx, thisant, thisspw, thispol))
                if thisant == antid and thisspw == spwid and thispol == polid:
                    LOG.debug('Adding data to datarows')
                    datarows.append(row)
                    dataids.append(idx)
        if len(datarows) > 0:
            midx = median_index(datarows)
        else:
            midx = None
        raid = grid_table[each_plane[0]][2]
        decid = grid_table[each_plane[0]][3]
        ra = grid_table[each_plane[0]][4]
        dec = grid_table[each_plane[0]][5]
        rowlist.append({"RAID": raid, "DECID": decid, "RA": ra, "DEC": dec,
                        "ROWS": datarows, "IDS": dataids, "MEDIAN_INDEX": midx})
        LOG.trace('RA %s DEC %s: datarows=%s'%(raid, decid, datarows))
        
    refpix_list = [0,0]
    refval_list = grid_table[num_ra * num_plane -1][4:6]
    increment_ra = grid_table[num_plane][4] - grid_table[0][4]
    increment_dec = grid_table[num_plane * num_ra][5] - grid_table[0][5]
    increment_list = [-increment_ra, increment_dec]
    LOG.debug('refpix_list=%s'%(refpix_list))
    LOG.debug('refval_list=%s'%(refval_list))
    LOG.debug('increment_list=%s'%(increment_list))
    
    return num_ra, num_dec, num_plane, refpix_list, refval_list, increment_list, rowlist 

def create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment):
    plotter = sparsemap.SDSparseMapPlotter(nh=num_ra, nv=num_dec, step=1, brightnessunit='K')
    plotter.setup_labels(refpix, refval, increment)
    return plotter
    
def get_data(infile, num_ra, num_dec, num_chan, rowlist):
    integrated_data = numpy.zeros(num_chan, dtype=float)
    map_data = numpy.zeros((num_ra, num_dec, num_chan), dtype=float) + sparsemap.NoDataThreshold
    nrow = 0
    for d in rowlist:
        ix = num_ra - 1 - d['RAID']
        iy = d['DECID']
        rows = d['ROWS']
        if len(rows) > 0:
            midx = d['MEDIAN_INDEX']
            median_row = rows[midx]
            LOG.debug('median row for (%s,%s) is %s (median %s)'%(ix, iy, median_row, numpy.median(rows)))
            nrow += len(rows)
            with casatools.TableReader(infile) as tb:
                map_data[ix,iy,:] = tb.getcell('SPECTRA', median_row)
                for row in rows:
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

def plot_profile_map(context, antid, spwid, polid, grid_table, infile, outfile, line_range):
    datatable = context.observing_run.datatable_instance

    num_ra, num_dec, num_plane, refpix, refval, increment, rowlist = analyze_grid_table(datatable, antid, spwid, polid, grid_table)
        
    plotter = create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment)
    
    nchan = context.observing_run[antid].spectral_window[spwid].num_channels
    LOG.debug('nchan=%s'%(nchan))
    
    integrated_data, map_data= get_data(infile, num_ra, num_dec, nchan, rowlist)
    lines_map = get_lines(datatable, num_ra, rowlist)

    spw = context.observing_run[antid].spectral_window[spwid]
    frequency = numpy.array([spw.refval + (i - spw.refpix) * spw.increment for i in xrange(nchan)]) * 1.0e-9    
    LOG.debug('frequency=%s~%s (nchan=%s)'%(frequency[0], frequency[-1], len(frequency)))
    plotter.setup_lines(line_range, lines_map)
    plotter.setup_reference_level(0.0)
    plotter.plot(map_data, integrated_data, frequency, figfile=outfile)
    plotter.done()
    return integrated_data, map_data

def plot_profile_map_with_fit(context, antid, spwid, polid, grid_table, prefit_data, postfit_data, prefit_figfile, postfit_figfile, line_range):
    datatable = context.observing_run.datatable_instance

    num_ra, num_dec, num_plane, refpix, refval, increment, rowlist = analyze_grid_table(datatable, antid, spwid, polid, grid_table)
        
    plotter = create_plotter(num_ra, num_dec, num_plane, refpix, refval, increment)
    
    nchan = context.observing_run[antid].spectral_window[spwid].num_channels
    LOG.debug('nchan=%s'%(nchan))
    
    prefit_integrated_data, prefit_map_data = get_data(prefit_data, num_ra, num_dec, nchan, rowlist)
    postfit_integrated_data, postfit_map_data = get_data(postfit_data, num_ra, num_dec, nchan, rowlist)
    lines_map = get_lines(datatable, num_ra, rowlist)
    
    fit_result = numpy.zeros(prefit_map_data.shape, dtype=float) + sparsemap.NoDataThreshold
    for x in xrange(num_ra):
        for y in xrange(num_dec):
            prefit = prefit_map_data[x][y]
            if not all(prefit == sparsemap.NoDataThreshold):
                postfit = postfit_map_data[x][y]
                fit_result[x][y] = prefit - postfit
    
    spw = context.observing_run[antid].spectral_window[spwid]
    frequency = numpy.array([spw.refval + (i - spw.refpix) * spw.increment for i in xrange(nchan)]) * 1.0e-9    
    LOG.debug('frequency=%s~%s (nchan=%s)'%(frequency[0], frequency[-1], len(frequency)))
    
    plotter.setup_lines(line_range, lines_map)
    
    # plot pre-fit spectra
    plotter.setup_reference_level(None)
    plotter.plot(prefit_map_data, prefit_integrated_data, frequency, fit_result=fit_result, figfile=prefit_figfile)
    
    # plot post-fit spectra
    plotter.setup_reference_level(0.0)
    plotter.plot(postfit_map_data, postfit_integrated_data, frequency, figfile=postfit_figfile)
        
    plotter.done()
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
