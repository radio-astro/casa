import casa as c
import numpy as np
import re;
def field2skycat(msname='', skycat='',fieldpattern=''):
    """
    Converts field table in ms to skycatalog
    allow overlay on image in the viewer
    """
    enumTypes =['J2000','JMEAN','JTRUE','APP','B1950',
                'B1950_VLA','BMEAN','BTRUE','GALACTIC','HADEC','AZEL','AZELSW',
                'AZELGEO','AZELSWGEO','JNAT','ECLIPTIC','MECLIPTIC','TECLIPTIC','SUPERGAL',
                'ITRF','TOPO','ICRS']

    qa=c.quanta
    c.table.open(msname+'/FIELD')
    dir=c.table.getcol('PHASE_DIR')
    nam=c.table.getcol('NAME')
    nfield=c.table.nrows()
    n=0;
    for k in range(nfield):
        if (re.match(fieldpattern,nam[k]) != None):
            n=n+1

    eltype=[]
    if(c.table.getcolkeyword('PHASE_DIR', 'MEASINFO').has_key('VarRefCol')):
        typeid=c.table.getcol(c.table.getcolkeyword('PHASE_DIR', 'MEASINFO')['VarRefCol'])
        for k in range(nfield):
            if (re.match(fieldpattern,nam[k]) != None):
                eltype.append(enumTypes[typeid[k]])
    else:
        eltype=[c.table.getcolkeyword('PHASE_DIR', 'MEASINFO')['Ref']]*nfield
    unitra=c.table.getcolkeyword('PHASE_DIR', 'QuantumUnits')[0]
    unitdec=c.table.getcolkeyword('PHASE_DIR', 'QuantumUnits')[1]
    c.table.done()
    des={}
    des['Type']={'valueType':'string'}
    des['Long']={'valueType':'double'}
    des['Lat']={'valueType':'double'}
    des['FIELD_NAME']={'valueType':'string'}
    des['FIELD_ID']={'valueType':'string'}
    des['RA']={'valueType':'string'}
    des['DEC']={'valueType':'string'}


    c.table.create(tablename=skycat, tabledesc=des, nrow=n);
    c.table.putcol('Type', eltype)
    lati=np.zeros((n,))
    longi=np.zeros((n,))
    RA=[]
    DEC=[]
    fid=[]
    fieldname=[]
    n=0;
    for k in range(nfield):
        if (re.match(fieldpattern,nam[k]) != None):
            longi[n]=qa.convert(qa.quantity(dir[0,0,k], unitra),'deg')['value']
            lati[n]=qa.convert(qa.quantity(dir[1,0,k], unitdec), 'deg')['value']
            RA.append(qa.time(qa.quantity(dir[0,0,k], unitra), prec=10))
            DEC.append(qa.angle(qa.quantity(dir[1,0,0], unitdec), prec=10))
            fid.append(str(k))
            fieldname.append(nam[k]);
            n=n+1;
    c.table.putcol('RA', RA)
    c.table.putcol('DEC', DEC)
#    c.table.putcol('FIELD_NAME', nam)
    c.table.putcol('FIELD_NAME', fieldname);
    c.table.putcol('FIELD_ID', fid)
    c.table.putcol('Long', longi)
    c.table.putcol('Lat', lati)
    c.table.putcolkeyword(columnname='Long', keyword='UNIT', value='deg')
    c.table.putcolkeyword(columnname='Lat', keyword='UNIT', value='deg')                     
    c.table.putinfo({'type':'Skycatalog'})
    c.table.done()
