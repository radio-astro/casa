import casa as c
import numpy as np
def cl2skycat(componentlist='', skycat=''):
    """
    Converts a componentlist dictionary or componnent list file on disk
    to  a skycatalog to overlay on image in the viewer
    
    """
    qa=c.quanta
    cl=c.casac.componentlist()
    if(type(componentlist)==str):
        cl.open(componentlist)
    elif(type(componentlist)==dict):
        cl.purge()
        cl.fromrecord(componentlist)
    if(cl.length()==0):
            print "no components found"
            return

    des={}
    des['Type']={'valueType':'string'}
    des['Long']={'valueType':'double'}
    des['Lat']={'valueType':'double'}
    des['COMP_ID']={'valueType':'string'}
    des['RA']={'valueType':'string'}
    des['DEC']={'valueType':'string'}
    des['FluxValue']={'valueType':'double'}
    c.table.create(tablename=skycat, tabledesc=des, nrow=cl.length())
    eltype=[]
    nam=[]
    RA=[]
    DEC=[]
    lati=np.zeros((cl.length(),))
    longi=np.zeros((cl.length(),))
    fluxval=np.zeros((cl.length(),))
    for k in range(cl.length()):
        longi[k]=qa.convert(cl.getrefdir(k)['m0'],'deg')['value']
        lati[k]=qa.convert(cl.getrefdir(k)['m1'],'deg')['value']
        fluxval[k]=cl.getfluxvalue(k)[0]
        RA.append(qa.time(cl.getrefdir(k)['m0'], prec=10))
        DEC.append(qa.angle(cl.getrefdir(k)['m1'], prec=10))
        eltype.append(cl.getrefdir(k)['refer'])
        nam.append(str(k))
    c.table.putcol('Type', eltype)
    c.table.putcol('RA', RA)
    c.table.putcol('DEC', DEC)
    c.table.putcol('COMP_ID', nam)
    c.table.putcol('Long', longi)
    c.table.putcol('Lat', lati)
    c.table.putcol('FluxValue', fluxval)
    c.table.putcolkeyword(columnname='Long', keyword='UNIT', value='deg')
    c.table.putcolkeyword(columnname='Lat', keyword='UNIT', value='deg')                     
    c.table.putinfo({'type':'Skycatalog'})
    c.table.done()
