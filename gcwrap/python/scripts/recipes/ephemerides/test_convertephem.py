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

    tablename -- name of the ephemeris table (in canonical CASA format)

    outtablename -- name of the reference ephemeris table

    """

    print toldeg

    if type(tablename)!=str or tablename=='':
        casalog.post('Invalid parameter tablename', 'WARN')
        return False

    if type(outtablename)!=str or outtablename=='':
        casalog.post('Invalid parameter outtablename', 'WARN')
        return False

    #compare RA, DEC, and RadVel
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

    for i in xrange(len(ra)):
        if ora[i] < 0:
            ora[i] += 360.
        rvq = qa.quantity(radvel[i],'AU/d')
        radvel[i] = qa.convert(rvq, 'km/s')['value']
        rvq = qa.quantity(oradvel[i],'AU/d')
        oradvel[i] = qa.convert(rvq, 'km/s')['value']

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
        print ra[i],", ", ora[i],", ", dra[i],", ", dec[i],", ", odec[i],", ", ddec[i],", ", radvel[i],", ", oradvel[i],", ", dradvel[i]

    a = np.array(dra)
    b = np.array(ddec)
    c = np.array(dradvel)

    print "Absolute differences mean and max"
    print "RA (deg)", a.mean(), " ", a.max()
    print "Dec (deg) ", b.mean(), " ", b.max()
    print "RA (arcsec)", a.mean()*3600., " ", a.max()*3600.
    print "Dec (arcsec) ", b.mean()*3600., " ", b.max()*3600.

    print
    print "RadVel (km/s)", c.mean(), " ", c.max()

    ra = np.array(rdra)
    rb = np.array(rddec)
    rc = np.array(rdradvel)

    print
    print "Relative differences mean and max (arcsec and km/s)"
    print "RA ", ra.mean()*3600., " ", ra.max()*3600.
    print "Dec ", rb.mean()*3600., " ", rb.max()*3600.
    print "RadVel ", rc.mean(), " ", rc.max()

    rval = True
    if (a.mean()>toldeg):
        print "RA  not within ", toldeg*3600., " arcsec of expectation."
        rval = False
    if (b.mean()>toldeg):
        print "Dec not within ", toldeg*3600., " arcsec of expectation."
        rval = False
    if (c.mean()>tolradvelkms):
        print "radvel not within ", tolradvelkms, " km/s of expectation."
        rval = False

    return rval


def test_convertephem():
    """
    Verify whether convertephem works as expected.
    """

    inputtab = 'Titan-ALMA.tab'
    reftab = 'Titan-geocentric.tab'

    if not os.path.exists(inputtab):
        rval = os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/fixplanets/'+inputtab+' .')
        if rval!=0:
            raise Exception, "Error copying input data"
    if not os.path.exists(reftab):
        rval = os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/fixplanets/'+reftab+' .')
        if rval!=0:
            raise Exception, "Error copying reference data"

    outtab = 'geo-titan.tab'

    import recipes.ephemerides.convertephem as ce

    ce.converttopoephem2geo(inputtab, outtab)

    return compare(outtab, reftab, 0.6/3600., 1E-4) # tolerances in deg and km/s resp.
