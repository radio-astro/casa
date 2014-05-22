import contextlib
import time

from taskinit import gentools
#import mtpy

@contextlib.contextmanager
def opentable(vis):
    print 'open table %s'%(vis)
    tb = gentools(['tb'])[0]
    tb.open(vis, nomodify=False)
    yield tb
    print 'closing table %s'%(vis)
    tb.close()

def readchunk(table, criteria, nrecord):
    tb = table.query(criteria)
    nrow = tb.nrows()
    rownumbers = tb.rownumbers()
    tb.close()
    nchunk = nrow / nrecord 
    for ichunk in xrange(nchunk):
        start = ichunk * nrecord
        end = start + nrecord
        chunk =  _readchunk(table, rownumbers[start:end])
        #print 'readchunk:',chunk
        yield chunk

    # residuals
    residual = nrow % nrecord
    if residual > 0:
        start = nrow - residual
        end = nrow
        chunk = _readchunk(table, rownumbers[start:end])
        #print 'readchunk:',chunk
        yield chunk
        
def _readchunk(table, rows):
    print '_readchunk: reading rows %s...'%(rows)
    return tuple((_readrow(table, irow) for irow in rows))

def _readrow(table, row):
    get = lambda col: table.getcell(col, row)
    return (row, get('FLOAT_DATA'), get('FLAG'), get('TIME_CENTROID'))
            
def reducechunk(chunk):
    return tuple((reducerecord(record) for record in chunk))

def reducerecord(record):
    print 'reducing row %s'%(record[0])
    data, flag, stats = reducedata(record[0], record[1], record[2], record[3])
    return (record[0], data, flag, record[3], stats)

def reducerecord2(record):
    data, mask = tosakura(record[1], record[2])
    data, mask = calibratedata(data, mask, record[3])
    mask = masknanorinf(data, mask)
    mask = maskedge(data, mask)
    data, mask = baselinedata(data, mask)
    mask = clipdata(data, mask)
    data = smoothdata(data, mask)
    stats = calcstats(data, mask)
    data, flag = tocasa(data, mask)
    yield (record[0], data, flag, record[3], stats)
    
def writechunk(table, results):
    put = lambda row, col, val: table.putcell(col, row, val)
    for record in results:
        row = int(record[0])
        data = record[1]
        flag = record[2]
        print 'writing result to table %s at row %s...'%(table.name(), row)
        put(row, 'FLOAT_DATA', data)
        put(row, 'FLAG', flag)

###
def reducedata(row, data, flag, timestamp):
    data[:] = float(row)
    print 'reducing row %s...'%(row)
    #mtpy.wait_for(5, 'row%s'%(row))
    print 'done reducing row %s...'%(row)
    return data, flag, {'statistics': data.real.mean()}

