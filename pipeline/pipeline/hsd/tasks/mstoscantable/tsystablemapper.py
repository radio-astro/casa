import os
import shutil
import numpy

from taskinit import gentools

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

# generate column description, default is scalar column
def coldesc(colname, vtype, option=0, maxlen=0, ndim=-1, comment='', unit=None, measinfo=None):
    desc = __coldesc(vtype, option, maxlen, ndim, comment, unit, measinfo)
    return colname, desc

TABLE_DESC = dict([
    coldesc('SCANNO', 'uint', comment='Scan number'),
    coldesc('CYCLENO', 'uint', comment='Cycle number'),
    coldesc('IFNO', 'uint', comment='IF number'),
    coldesc('BEAMNO', 'uint', comment='Beam number'),
    coldesc('POLNO', 'uint', comment='Pol number'),
    coldesc('FREQ_ID', 'uint', comment='Key for FREQUENCIES table'),
    coldesc('TIME', 'double', comment='Time', unit='d', measinfo={'type': 'epoch', 'Ref': 'UTC'}),
    coldesc('TSYS', 'float', ndim=1, comment='Tsys'),
    coldesc('FLAGTRA', 'uchar', ndim=1, comment='Channel flag'),
    coldesc('ELEVATION', 'float', comment='Elevation')
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
