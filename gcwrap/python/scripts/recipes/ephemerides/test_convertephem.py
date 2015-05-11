# unit test for convertephem recipe
# DP, May 2015
#
# based on a Mars ephemeris obtained from JPL Horizons once in TOPO (for ALMA)
# and once in GEO. The test converts the TOPO ephemeris into GEO and compares
# to the result from JPL Horizons. The agreement is mediocre:
# RA differs on average by 1.7 arcsec, Dec by 0.4 arcsec, the radial velocity by 0.52 km/s
# Maximum difference is not much larger: 3.1 arcsec in RA, 1.0 arcsec in Dec, 0.63 km/s in Radvel 
#
# This is at least partially explained by the more accurate model behind the JPL ephemerides.
#
# usage:
#   execfile('test_convertephem.py')
#   test_convertephem()
# should return True.

import numpy as np

def compare(tablename='', outtablename='', toldeg=2.7778e-05, tolradvelkms=0.6):
    """
    compare

    tablename -- name of the TOPO frame ephemeris table (in canonical CASA format)

    outtablename -- name of the GEO frame ephemeris table

    """

    print toldeg

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
    tbt.close()
    tbt.open(outtablename)
    ora = tbt.getcol('RA')
    odec = tbt.getcol('DEC')
    oradvel = tbt.getcol('RadVel')
    tbt.close()


    dra = []
    ddec = []
    dradvel = []
    rdra = []
    rddec = []
    rdradvel = []

    for i in range(0, len(ra)):
        dra.append(abs(ra[i]-ora[i])*np.cos(odec[i]/180.*pi))
        ddec.append(abs(dec[i]-odec[i]))
        dradvel.append(abs(radvel[i]-oradvel[i]))
        rdra.append(abs((ra[i]-ora[i])/ra[i]))
        rddec.append(abs((dec[i]-odec[i])/dec[i]))
        rdradvel.append(abs((radvel[i]-oradvel[i])/radvel[i]))

    a = np.array(dra)
    b = np.array(ddec)
    c = np.array(dradvel)

    print "Absolute differences mean and max"
    print "RA (deg)", a.mean(), " ", a.max()
    print "Dec (deg) ", b.mean(), " ", b.max()
    print "RA (arcsec)", a.mean()*3600., " ", a.max()*3600.
    print "Dec (arcsec) ", b.mean()*3600., " ", b.max()*3600.


    print
    print "RadVel (AU/d)", c.mean(), " ", c.max()
    radvel_auperd = qa.quantity(c.mean(),'AU/d')
    radvel_kmpers = qa.convert(radvel_auperd, 'km/s')['value']
    print "RadVel (km/s) mean deviation = ", radvel_kmpers

    ra = np.array(rdra)
    rb = np.array(rddec)
    rc = np.array(rdradvel)

    print
    print "Relative differences mean and max"
    print "RA ", ra.mean(), " ", ra.max()
    print "Dec ", rb.mean(), " ", rb.max()
    print "RadVel ", rc.mean(), " ", rc.max()


    if (a.mean()<toldeg) and (b.mean()<toldeg) and (radvel_kmpers<tolradvelkms):
        print "RA and Dec within ", toldeg, " deg = ", toldeg*3600., " arcsec, radvel within ", tolradvelkms, " km/s of expectation."
        return True
    else:
        return False


def test_convertephem():
    """
    Verify whether convertephem works as expected.
    """

    inputtab = 'fewdays-topo.txt.tab'
    reftab = 'fewdays-geo.txt.tab'

    if not os.path.exists(inputtab):
        rval = os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/fixplanets/'+inputtab+' .')
        if rval!=0:
            raise Exception, "Error copying input data"
    if not os.path.exists(reftab):
        rval = os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/fixplanets/'+reftab+' .')
        if rval!=0:
            raise Exception, "Error copying reference data"

    outtab = 'geo-fewdays-topo.txt.tab'

    import recipes.ephemerides.convertephem as ce

    ce.converttopoephem2geo(inputtab, outtab)

    return compare(outtab, reftab, 1.7/3600., 0.6)
