import numpy
import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays.singledish.sparsemap as sparsemap

LOG = infrastructure.get_logger(__name__)

def plot_profile_map(context, antid, spwid, polid, grid_table, infile, outfile, line_range):
    datatable = context.observing_run.datatable_instance
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
        raid = grid_table[each_plane[0]][2]
        decid = grid_table[each_plane[0]][3]
        ra = grid_table[each_plane[0]][4]
        dec = grid_table[each_plane[0]][5]
        rowlist.append({"RAID": raid, "DECID": decid, "RA": ra, "DEC": dec,
                        "ROWS": datarows, "IDS": dataids})
        LOG.trace('RA %s DEC %s: datarows=%s'%(raid, decid, datarows))
        
    plotter = sparsemap.SDSparseMapPlotter(nh=num_ra, nv=num_dec, step=1, brightnessunit='K')
    refpix_list = [0,0]
    refval_list = grid_table[0][4:6]
    increment_ra = grid_table[num_plane][4] - grid_table[0][4]
    increment_dec = grid_table[num_plane * num_ra][5] - grid_table[0][5]
    increment_list = [increment_ra, increment_dec]
    LOG.debug('refpix_list=%s'%(refpix_list))
    LOG.debug('refval_list=%s'%(refval_list))
    LOG.debug('increment_list=%s'%(increment_list))
    plotter.setup_labels(refpix_list, refval_list, increment_list)
    
    #nchan = datatable.tb1.getcell('NCHAN', rowlist[0]['ROWS'][0])
    nchan = context.observing_run[antid].spectral_window[spwid].num_channels
    LOG.debug('nchan=%s'%(nchan))
    
    integrated_data = numpy.zeros(nchan, dtype=float)
    map_data = numpy.zeros((num_ra, num_dec, nchan), dtype=float) + sparsemap.NoDataThreshold
    lines_map = collections.defaultdict(dict)
    nrow = 0
    for d in rowlist:
        # reverse the data in horizontal (RA) direction
        ix = num_ra - 1 - d['RAID']
        iy = d['DECID']
        rows = d['ROWS']
        ids = d['IDS']
        if len(rows) > 0:
            midx = median_index(rows)
            median_row = rows[midx]
            LOG.debug('median row for (%s,%s) is %s (median %s)'%(ix, iy, median_row, numpy.median(rows)))
            nrow += len(rows)
            with casatools.TableReader(infile) as tb:
                map_data[ix,iy,:] = tb.getcell('SPECTRA', median_row)
                masklist = context.observing_run.datatable_instance.getcell('MASKLIST', ids[midx])
                lines_map[ix][iy] = None if len(masklist) == 0 else masklist
                for row in rows:
                    integrated_data += tb.getcell('SPECTRA', row)
        else:
            LOG.debug('no data is available for (%s,%s)'%(ix,iy))
    integrated_data /= nrow
    LOG.trace('integrated_data=%s'%(integrated_data))
    LOG.trace('map_data.shape=%s'%(list(map_data.shape)))
    LOG.trace('map_data[0][0].shape=%s'%(map_data[0][0].shape))
    spw = context.observing_run[antid].spectral_window[spwid]
    frequency = numpy.array([spw.refval + (i - spw.refpix) * spw.increment for i in xrange(nchan)]) * 1.0e-9    
    LOG.debug('frequency=%s~%s (nchan=%s)'%(frequency[0], frequency[-1], len(frequency)))
    plotter.setup_lines(line_range, lines_map)
    plotter.setup_reference_level(0.0)
    plotter.plot(map_data, integrated_data, frequency, outfile)
    return integrated_data, map_data

def median_index(arr):
    if not numpy.iterable(arr) or len(arr) == 0:
        return numpy.nan
    else:
        sorted_index = numpy.argsort(arr)
        if len(arr) < 3:
            return sorted_index[0]
        else:
            return sorted_index[len(arr) / 2]
