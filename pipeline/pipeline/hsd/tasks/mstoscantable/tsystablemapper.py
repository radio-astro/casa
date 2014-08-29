import os
import shutil
import numpy

from taskinit import gentools

#tb = gentools(['tb'])[0]

def __coldesc( vtype, option, maxlen,
             ndim, comment, unit=None, measinfo=None ):
    d={'dataManagerGroup': 'StandardStMan',
       'dataManagerType': 'StandardStMan'}
    d['valueType'] = vtype
    d['option'] = option
    if ndim > 0:
        d['ndim'] = ndim
    d['maxlen'] = maxlen
    d['comment'] = comment
    if unit is not None:
        d['keywords'] = {'QuantumUnits': [unit] }
    if measinfo is not None:
        if not d.has_key('keywords'):
            d['keywords'] = {}
        d['keywords']['MEASINFO'] = measinfo
    return d

def tbobj():
    return gentools(['tb'])[0]

def coldesc(colname, vtype, option, maxlen, ndim, comment, unit=None, measinfo=None):
    desc = __coldesc(vtype, option, maxlen, ndim, comment, unit, measinfo)
    return colname, desc

TABLE_DESC = dict([
    coldesc('SCANNO', 'uint', 0, 0, -1, 'Scan number'),
    coldesc('CYCLENO', 'uint', 0, 0, -1, 'Cycle number'),
    coldesc('IFNO', 'uint', 0, 0, -1, 'IF number'),
    coldesc('BEAMNO', 'uint', 0, 0, -1, 'Beam number'),
    coldesc('POLNO', 'uint', 0, 0, -1, 'Pol number'),
    coldesc('FREQ_ID', 'uint', 0, 0, -1, 'Key for FREQUENCIES table'),
    coldesc('TIME', 'double', 0, 0, -1, 'Time', unit='d', measinfo={'type': 'epoch', 'Ref': 'UTC'}),
    coldesc('TSYS', 'float', 0, 0, 1, 'Tsys'),
    coldesc('FLAGTRA', 'uchar', 0, 0, 1, 'Channel flag'),
    coldesc('ELEVATION', 'float', 0, 0, -1, 'Elevation')
])

TABLE_KEYWORD = {'VERSION': 1,
                 'ScantableName': '{name}',
                 'ApplyType': 'CALTSYS',
                 'FREQUENCIES': 'Table: {name}'}

def map(prefix, caltable, reftable):
    # initial check
    check(caltable)
    
    tb = tbobj()
    antenna = antennanames(caltable)
    names = {}
    for (antenna_id, antenna_name) in enumerate(antenna):
        name = '.'.join([prefix, antenna_name, 'tsyscal.tbl'])
        names[antenna_name] = name
        ret = tb.create(name, TABLE_DESC, memtype='plain', nrow=0)
        try:
            fill(tb, caltable, antenna_id)
            keywords = TABLE_KEYWORD.copy()
            if reftable is not None:
                src = os.path.join(reftable, 'FREQUENCIES')
                dst = os.path.join(name, 'FREQUENCIES')
                if os.path.exists(src):
                    if os.path.exists(dst):
                        shutil.rmtree(dst)
                    shutil.copytree(src, dst)
            scntable = '.'.join([prefix, antenna_name, 'asap'])
            keywords['ScantableName'] = os.path.abspath(scntable)
            keywords['FREQUENCIES'] = 'Table: %s'%(os.path.abspath(dst))
            putkeyword(tb, keywords)
        finally:
            tb.close()
    return names

def check(caltable):
    # Make sure caltable type is B TSYS
    tb = tbobj()
    tb.open(caltable)
    viscal = tb.getkeyword('VisCal')
    tb.close()

    assert viscal == 'B TSYS'

def putkeyword(table, keywords):
    for (k,v) in keywords.items():
        table.putkeyword(k, v)
    
def fill(table, caltable, antenna_id):
    tb = tbobj()
    tb.open(caltable)
    try:
        tsel = tb.query('ANTENNA1==%s'%(antenna_id))
        rows = tsel.rownumbers()
        tsel.close()
        for row in rows:
            t = tb.getcell('TIME', row)
            spw = tb.getcell('SPECTRAL_WINDOW_ID', row)
            scan = tb.getcell('SCAN_NUMBER', row)
            tsys = tb.getcell('FPARAM', row)
            flag = tb.getcell('FLAG', row)
            npol = tsys.shape[0]
            flagtra = flag * 128
            for ipol in xrange(npol):
                idx = table.nrows()
                table.addrows()
                table.putcell('TIME', idx, t / 86400.0)
                table.putcell('IFNO', idx, spw)
                table.putcell('FREQ_ID', idx, spw)
                table.putcell('SCANNO', idx, scan)
                table.putcell('CYCLENO', idx, 0)
                table.putcell('POLNO', idx, ipol)
                table.putcell('TSYS', idx, tsys[ipol])
                table.putcell('FLAGTRA', idx, flagtra[ipol])
                table.putcell('ELEVATION', idx, 0.0)
    finally:
        tb.close()

def antennaname(name):
    tb = tbobj()
    tb.open(name)
    antenna = tb.getkeyword('AntennaName')
    tb.close()
    site, ant = antenna.split('//')
    ant, station = ant.split('@')
    return ant

def antennanames(caltable):
    tb = tbobj()
    tb.open(os.path.join(caltable, 'ANTENNA'))
    names = tb.getcol('NAME')
    tb.close()
    return names
