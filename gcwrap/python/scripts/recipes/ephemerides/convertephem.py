from taskinit import *
import glob

# Conversion of TOPO ephemerides to GEO
#
# Example:
#       import ephemeris.convertephem as ce
#       ce.convert2geo('titan.ms', 'Titan')
#
#   will find the ephemeris attached to field "Titan" in the MS "titan.ms"
#   and convert RA, Dec, and RadVel to the geocentric ref frame


def converttopoephem2geo(tablename='', outtablename='', overwrite=True):
    """
    converttopoephem2geo

    Convert the given topo ephemeris table to the geocentric ref frame.
    Converted are the RA, DEC, and RadVel columns only

    tablename -- name of the TOPO frame ephemeris table (in canonical CASA format)

    outtablename -- name of the output GEO frame ephemeris table

    """

    if type(tablename)!=str or tablename=='':
        casalog.post('Invalid parameter tablename', 'WARN')
        return False

    if type(outtablename)!=str or outtablename=='':
        casalog.post('Invalid parameter outtablename', 'WARN')
        return False

    #convert RA, DEC, and RadVel
    tbt = tbtool()
    met = metool()
    qat = qatool()

    tbt.open(tablename)
    ra = tbt.getcol('RA')
    dec = tbt.getcol('DEC')
    radvel = tbt.getcol('RadVel')
    mjd = tbt.getcol('MJD')
    geodist = tbt.getkeyword('GeoDist') # (km above reference ellipsoid)
    geolat = tbt.getkeyword('GeoLat') # (deg)
    geolong = tbt.getkeyword('GeoLong') # (deg)
    obsloc = tbt.getkeyword('obsloc')
    try:
        posref = tbt.getkeyword('posrefsys')
    except Exception, instance:
        casalog.post('Ephemeris does not have the posrefsys keyword. Assuming ICRF/J2000.0', 'WARN')
        posref = 'ICRF/J2000.0'
        
    tbt.close()

    if posref!='ICRF/J2000.0':
        casalog.post('Unsupported position reference system: '+posref+', need ICRF/J2000.0', 'WARN')
        return False

    if obsloc=='GEOCENTRIC':
        casalog.post('Obsloc is already GEOCENTRIC. Nothing to be done.', 'WARN')
        return True

    mepos = {'m0': {'value': geolong, 'unit': 'deg'},
             'm1': {'value': geolat, 'unit': 'deg'}, # latitude
             'm2': {'value': geodist, 'unit': 'km'}, # alt above ref ellipsoid
             'refer': 'WGS84',
             'type': 'position'}
    met.doframe(mepos)

    newra=[]
    newdec=[]
    newrho=[]
    newradvel=[]

    for i in range(0, len(mjd)):
        memjd={'m0': {'value': mjd[i]+100., 'unit': 'd'},
               'refer': 'UTC',
               'type': 'epoch'}
        met.doframe(memjd)

        olddir={'m0': {'value': ra[i], 'unit': 'deg'},
                'm1': {'value': dec[i], 'unit': 'deg'},
                'refer': 'TOPO',
                'type': 'direction'}
        met.doframe(olddir)
        newdir=met.measure(olddir, 'APP')

        newra.append(qat.convert(newdir['m0'],'deg')['value'])
        newdec.append(qat.convert(newdir['m1'],'deg')['value'])

        oldradvel={'m0': {'value': radvel[i], 'unit': 'AU/d'},
                   'refer': 'TOPO',
                   'type': 'radialvelocity'}

        newradvelme = met.measure(oldradvel, 'GEO')
        newradvel.append(qat.convert(newradvelme['m0'], 'AU/d')['value'])

    # create the converted table
    safetycopyname=tablename+'.orig'
    if overwrite:
        if outtablename==tablename:
            os.system('cp -R '+tablename+' '+safetycopyname)
        else:
            os.system('rm -rf '+outtablename)
            os.system('cp -R '+tablename+' '+outtablename)
    else:
        if os.path.exists(outtablename):
            casalog.post('Output table '+outtablename+' already exists.', 'WARN')
            return False
        os.system('cp -R '+tablename+' '+outtablename)
        
    try:
        tbt.open(outtablename, nomodify=False)
        tbt.putcol('RA', newra)
        tbt.putcol('DEC', newdec)
        tbt.putcol('RadVel', newradvel)
        tbt.putkeyword('GeoDist', 0.) 
        tbt.putkeyword('GeoLat', 0.)
        tbt.putkeyword('GeoLong', 0.) 
        tbt.putkeyword('obsloc', 'GEOCENTRIC')
        tbt.close()
    except Exception, instance:
        casalog.post('Conversion in situ was not possible. Restoring original ephemeris ...', 'INFO')
        if overwrite and outtablename==tablename:
            os.system('rm -rf '+tablename)
            os.system('mv '+safetycopyname+' '+tablename)
        casalog.post("*** Error \'%s\' " % (instance), 'SEVERE')
        return False
    

    if overwrite and outtablename==tablename:
        os.system('rm -rf original-'+tablename)

    return True

        
def convert2geo(vis='', field=''):

    """
    Convert the ephemeris attached to the given field of the given MS to GEO.
    Only RA, Dec, and RadVel are converted.
    If there is attached ephemeris or if the ephemeris is already in GEO, nothing is done.
    """

    casalog.origin('convertephem')

    mst = mstool()
    tbt = tbtool()

    rval = True

    try:
        fields = mst.msseltoindex(vis=vis,field=field)['field']
        numfields = 0 
        if(len(fields) == 0):
            casalog.post( "Field selection returned zero results.", 'WARN')
            return True

        theephstoconvert = []

        tbt.open(vis+'/FIELD')

        for fld in fields:
            if ('EPHEMERIS_ID' in tbt.colnames()):
                theid = tbt.getcell('EPHEMERIS_ID',fld)
                if theid>=0: # there is an ephemeris attached
                    ephemname = glob.glob(vis+'/FIELD/EPHEM'+str(theid)+'_*.tab')[0]
                    theephstoconvert.append(ephemname)
                    casalog.post('Found ephemeris '+ephemname+' for field '+str(fld), 'INFO')

        tbt.close()
        
        if len(theephstoconvert)==0:
            casalog.post('No ephemerides attached. Nothing to do.', 'WARN')
            return True
        else:
            for theeph in theephstoconvert:
                if converttopoephem2geo(theeph, theeph, overwrite=True):
                    casalog.post('Converted '+theeph+' to GEO.', 'INFO') 
                else:
                    casalog.post('Error converting '+theeph+' to GEO.', 'WARN')
                    rval = False

        return rval

    except Exception, instance:
        casalog.post("*** Error \'%s\' " % (instance), 'SEVERE')
        return False



