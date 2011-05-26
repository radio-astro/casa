##############################################################################
#                                                                            #
# Test Name:                                                                 #
#   coordsystest_regression.py                                               #
# Rationale for Inclusion:                                                   #
#   This is a Python translation of the Glish assay test coordsystest.g      #
#   It tests many coordsys tool methods                                      #
# Features Tested:                                                           #
#   coordsys, type, done, id, ncoordinates                                   #
#   coordinatetype, axiscoordinatetypes, summary                             #
#   is_coordsys, coordsystools                                               #
#   referencecode, setreferencecode                                          #
#   restfrequency, setrestfrequency                                          #
#   projection, setprojection                                                #
#   torecord, fromrecord, copy                                               #
#   setepoch, epoch, setobserver, observer                                   #
#   settelescope, telescope, setparentname, parentname                       #
#   setnames, names, setunits, units                                         #
#   setreferencepixel, referencepixel                                        #
#   setreferencevalue, referencevalue                                        #
#   setincrement, increment                                                  #
#   setlineartransform, lineartransform                                      #
#   setstokes, stokes                                                        #
#   findcoordinate, findaxis                                                 #
#   toworld, toworldmany, topixel, topixelmany                               #
#   naxes, axesmap                                                           #
#   reorder                                                                  #
#   frequencytovelocity, velocitytofrequency                                 #
#   setreferencelocation                                                     #
#   toabs, torel, toabsmany, torelmany                                       #
#   convert, convertmany                                                     #
#   setspectral                                                              #
#   settabular                                                               #
#   addcoordinate                                                            #
#   toworld, topixel with reference conversion                               #
#   setdirection                                                             #
#   replace                                                                  #
# Success/failure criteria:                                                  #
#    Internally tests each method for pass/fail.  Throws an uncaught         #
#    exception ending test when an unexpected failure occurs.                #
#    All tests pass if script runs to completion.                            #
#                                                                            #
##############################################################################
#                                                                            #
# Converted by RRusk 2007-11-08 from coordsystest.py                         #
#                                                                            #
##############################################################################

import time
import re
#
# coordsystest.py: test casapy coordsys tool
#

#
# Utils
#
def note(message, priority="INFO", origin="coordsystest", ntime=None, postcli='F'):
    #if not ntime:  #if (ntime==None):
    #    ntime=time.asctime()
    #print ntime, priority, origin, message
    if postcli: print message
    casalog.postLocally(message, priority, origin)
###
def info(message):
    #note(message,origin='coordsystest')
    print message
    casalog.postLocally(message, priority="INFO", origin="coordsystest")

def fail(message=""):
    casalog.postLocally(message, priority="SEVERE", origin='coordsystest')
    raise RuntimeError, message

###
def stop(message=""):
    note(message ,priority='SEVERE', origin='coordsystest')
    raise RuntimeError, message

###
def cleanup(dir):
    if (os.path.isdir(dir)):
        info("Cleaning up directory "+dir)
        def errFunc(raiser, problemPath, excInfo):
            #print raiser.__name__,'failed on',problemPath
            note(raiser.__name__+'failed on'+problemPath,"SEVERE")
            raise RuntimeError, "Cleanup of " + dir + " fails!"
        shutil.rmtree(dir,0,errFunc)
    return true

def all(x,y,tolerance=0):
    if len(x) != len(y):
        return false
    for i in range(len(x)):
        if not (abs(x[i]-y[i]) <= tolerance):
            print "x["+str(i)+"]=", x[i]
            print "y["+str(i)+"]=", y[i]
            return false
    return true

def alleq(x,y,tolerance=0):
    if x.size != y.size:
        print "x.size=", x.size
        print "y.size=", y.size
        return false
    if len(x.shape)==1:
        for i in range(len(x)):
            if not (abs(x[i]-y[i]) < tolerance):
                print "x[",i,"]=", x[i]
                print "y[",i,"]=", y[i]
                return false
    if len(x.shape)==2:
        for i in range(len(x)):
            for j in range(len(x[i])):
                if not (abs(x[i][j]-y[i][j]) < tolerance):
                    print "x[",i,"][",j,"]=", x[i][j]
                    print "y[",i,"][",j,"]=", y[i][j]
                    return false
    if len(x.shape)==3:
        for i in range(len(x)):
            for j in range(len(x[i])):
                for k in range(len(x[i][j])):
                    if not (abs(x[i][j][k]-y[i][j][k]) < tolerance):
                        print "x[",i,"][",j,"][",k,"]=", x[i][j][k]
                        print "y[",i,"][",j,"][",k,"]=", y[i][j][k]
                        return false
    if len(x.shape)==4:
        for i in range(len(x)):
            for j in range(len(x[i])):
                for k in range(len(x[i][j])):
                    for l in range(len(x[i][j][k])):
                        if not (abs(x[i][j][k][l]-y[i][j][k][l]) < tolerance):
                            print "x[",i,"][",j,"][",k,"][",l,"]=", x[i][j][k][l]
                            print "y[",i,"][",j,"][",k,"][",l,"]=", y[i][j][k][l]
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in alleq')
    return true


def coordsystest():
    def test1():
        info('');
        info('');
        info('');
        info('Test 1 - coordsys, type, done, id, ncoordinates');
        info('       - coordinatetype, axiscoordinatetypes, summary');
        info('       - is_coordsys, coordsystools');
        #
        info('');
        info ('Testing coordsys constructor');
        info('');
        mycs = cs.newcoordsys()
        if not mycs:
            return stop('coordsys constructor 1 failed');
        #if not cs.iscoordsys(mycs): fail('is_coordsys 1 failed')
        #
        if mycs.ncoordinates()!=0: fail('ncoordinates 1 failed')
        if mycs.type()!='coordsys': fail('type 1 failed')
        #
        #id = mycs.id()
        #if not id: fail('id 1 failed')
        #ok = id.has_key('sequence') and id.has_key('pid')
        #ok = ok and id.has_key('time')
        #ok = ok and id.has_key('host')
        #ok = ok and id.has_key('agentid')
        #if not ok: fail(' id record has wrong fields')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T);
        if not mycs: fail('coordsys constructor 2 failed')
        #if not cs.iscoordsys(mycs): fail('is_coordsys 2 failed')
        if mycs.ncoordinates()!=1: fail('ncoordinates 2 failed')
        if mycs.coordinatetype(0)!='Direction': fail('coordinatetype 1 failed')
        t1 = mycs.axiscoordinatetypes(T);
        t2 = mycs.axiscoordinatetypes(F);
        ok = t1[0]=='Direction' and t1[1]=='Direction'
        ok = ok and t2[0]=='Direction'
        ok = ok and t2[1]=='Direction';
        if not ok: fail('axiscoordinatetypes 1 failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(spectral=T)
        if not mycs: fail('coordsys constructor 3 failed')
        #if not cs.iscoordsys(mycs): fail('is_coordsys 3 failed')
        if mycs.ncoordinates()!=1: fail('ncoordinates 3 failed')
        if mycs.coordinatetype(0)!='Spectral': fail('coordinatetype 2 failed')
        t1 = mycs.axiscoordinatetypes(T)
        t2 = mycs.axiscoordinatetypes(F)
        ok = (t1=='Spectral') and (t2=='Spectral')
        if not ok: fail('axiscoordinatetypes 2 failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(stokes="I Q U V")
        if not mycs: fail('coordsys constructor 4 failed')
        #if not cs.iscoordsys(mycs): fail('is_coordsys 4 failed')
        if mycs.ncoordinates()!=1: fail('ncoordinates 4 failed')
        if mycs.coordinatetype(0)!='Stokes': fail('coordinatetype 3 failed')
        t1 = mycs.axiscoordinatetypes(T)
        t2 = mycs.axiscoordinatetypes(F)
        ok = t1=='Stokes' and t2=='Stokes'
        if not ok: fail('axiscoordinatetypes 3 failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(linear=3)
        if not mycs: fail('coordsys constructor 5 failed')
        #if not cs.iscoordsys(mycs): fail('is_coordsys 5 failed;'
        if mycs.ncoordinates()!=1: fail('ncoordinates 5 failed')
        if mycs.coordinatetype(0)!='Linear': fail('coordinatetype 4 failed')
        t1 = mycs.axiscoordinatetypes(T)
        t2 = mycs.axiscoordinatetypes(F)
        ok = t1[0]=='Linear' and t2[0]=='Linear'
        if not ok: fail('axiscoordinatetypes 4 failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(tabular=T)
        if not mycs: fail('coordsys constructor 6 failed')
        #if not cs.iscoordsys(mycs): fail('is_coordsys 6 failed')
        if mycs.ncoordinates()!=1: fail('ncoordinates 6 failed')
        if mycs.coordinatetype(0)!='Tabular': fail('coordinatetype 6 failed')
        t1 = mycs.axiscoordinatetypes(T)
        t2 = mycs.axiscoordinatetypes(F)
        ok = t1=='Tabular' and t2=='Tabular'
        if not ok: fail('axiscoordinatetypes 5 failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, spectral=T, stokes="I Q U V",
                              linear=1, tabular=T)
        if not mycs: fail('coordsys constructor 7 failed')
        #if not cs.iscoordsys(mycs): fail('is_coordsys 7 failed')
        if mycs.ncoordinates()!=5: fail('ncoordinates 7 failed')
        ok = mycs.coordinatetype(0)=='Direction'
        ok = ok and mycs.coordinatetype(1)=='Stokes'
        ok = ok and mycs.coordinatetype(2)=='Spectral'
        ok = ok and mycs.coordinatetype(3)=='Linear'
        ok = ok and mycs.coordinatetype(4)=='Tabular'
        if not ok: fail('coordinatetype 5 failed')
        t = mycs.coordinatetype()
        if not t: fail()
        if (t[0]!='Direction' or t[1]!='Stokes' or
            t[2]!='Spectral' or t[3]!='Linear' or
            t[4]!='Tabular'): fail('coordinatetype 6 failed')
        #
        t1 = mycs.axiscoordinatetypes(T)
        t2 = mycs.axiscoordinatetypes(F)
        ok = t1[0]=='Direction' and t1[1]=='Direction'
        ok = ok and (t1[2]=='Stokes' and t1[3]=='Spectral')
        ok = ok and (t1[4]=='Linear' and t1[5]=='Tabular')
        ok = ok and (t2[0]=='Direction' and t2[1]=='Direction')
        ok = ok and (t2[2]=='Stokes' and t2[3]=='Spectral')
        ok = ok and (t2[4]=='Linear' and t2[5]=='Tabular')
        if not ok: fail('axiscoordinatetypes 7 failed')
        #
        if not mycs.summary(): fail()
        if not mycs.summary(doppler='optical'): fail()
        if not mycs.summary(doppler='radio'): fail()
        #
        if not mycs.done(): fail()
        #
        cs1 = cs.newcoordsys()
        cs2 = cs.newcoordsys()
        #l = coordsystools()
        #ok = length(l)==2 and  l[0]=='cs1' and l[1]=='cs2'
        #if not ok:
        #    fail('coordsystools failed')
        #if not cs1.done(): fail()
        #if not cs2.done(): fail()
        ###
        return T

    def test2():
        info('')
        info('')
        info('')
        info('Test 2 - referencecode, setreferencecode')
        info('         restfrequency, setrestfrequency')
        info('         projection, setprojection')
        #
        # Frequency. Does not test reference value conversion
        # is correct
        #
        info('')
        info ('Testing referencecode')
        info('')
        mycs = cs.newcoordsys(direction=T, spectral=T)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        d = me.frequency('LSRK')
        if not d: fail()
        list = me.listcodes(d)
        if not list: fail()
        #
        for i in list['normal']:
            if (i!='REST'):
                ok = mycs.setreferencecode(type='spectral', value=i, adjust=T)
                if not ok: fail()
                if (mycs.referencecode(type='spectral')!=i):
                    fail('failed to recover spectral reference code '+i)
        #
        # Direction. Does not test reference value conversion
        # is correct
        #
        d = me.direction('J2000')
        if not d: fail()
        list = me.listcodes(d)
        if not list: fail()
        #
        for i in list['normal']:
            if not re.search("AZEL",i, re.IGNORECASE):
                ok = mycs.setreferencecode(type='direction', value=i, adjust=F)
                if not ok: fail()
                if (mycs.referencecode(type='direction')!=i):
                    fail('failed to recover direction reference code '+i)
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, spectral=T, linear=1)
        if not mycs: fail('coordsys constructor 2 failed')
        ok = mycs.setreferencecode(type='direction', value='B1950')
        if not ok: fail()
        ok = mycs.setreferencecode(type='spectral', value='BARY')
        if not ok: fail()
        #
        c = mycs.referencecode()
        ok = len(c)==3 and c[0]=='B1950' and c[1]=='BARY' and c[2]==''
        if not ok: fail('referencecode 3 failed')
        #
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setreferencecode(value='doggies')
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('setreferencecode unexpectedly did not fail')
        #
        # projection
        #
        info('')
        info ('Testing projection')
        info('')
        ok = mycs.setprojection('SIN', [1.0,2.0])
        if not ok: fail()
        p = mycs.projection()
        if not p: fail()
        ok = p['type']=='SIN' and len(p['parameters'])==2
        ok = ok and p['parameters'][0]==1.0
        ok = ok and p['parameters'][1]==2.0
        if not ok: fail('setprojection/projection 1 reflection failed')
        #
        p = mycs.projection('all')['types']
        if len(p)!=27: fail('projection 1 failed')
        for i in p:
            n = mycs.projection(i)
            if not n: fail()
        #
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setprojection('fish')
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            ok = false
        if ok: fail('setprojection 1 unexpectedly did not fail')
        #
        # restfrequency
        #
        info('')
        info ('Testing restfrequency')
        info('')
        rf1 = qa.quantity('1.2GHz')
        if not rf1: fail()
        ok = mycs.setrestfrequency(rf1)
        if not ok: fail()
        rf2 = mycs.restfrequency()
        if not rf2: fail()
        rf2 = qa.convert(rf2,rf1['unit'])
        ok = abs(qa.getvalue(rf1)-qa.getvalue(rf2))<1.0e-6
        ok = ok and qa.getunit(rf1)==qa.getunit(rf2)
        if not ok: fail('setrestfrequency/restfrequency 1 reflection failed')
        #
        unit = qa.getunit(mycs.restfrequency())
        if not unit: fail()
        rf1 = 2.0
        ok = mycs.setrestfrequency(rf1)
        if not ok: fail()
        rf2 = mycs.restfrequency()
        if not rf2: fail()
        rf1 = qa.unit(rf1, unit)
        if not rf1: fail()
        ok = abs(qa.getvalue(rf1)-qa.getvalue(rf2))<1.0e-6
        ok = ok and qa.getunit(rf1)==qa.getunit(rf2)
        if not ok: fail('setrestfrequency/restfrequency 2 reflection failed')
        #
        rf1 = qa.quantity([1e9, 2e9], 'Hz')
        # Select second freq
        ok = mycs.setrestfrequency(value=rf1, which=1, append=F);
        rf2 = qa.convert(mycs.restfrequency(),qa.getunit(rf1))
        v1 = qa.getvalue(rf1)
        v2 = qa.getvalue(rf2)
        ok = abs(v1[0]-v2[1])<1e-6
        ok = ok and abs(v1[1]-v2[0])<1e-6
        ok = ok and qa.getunit(rf1)==qa.getunit(rf2)
        if not ok: fail('setrestfrequency/restfrequency 3 reflection failed')
        #
        rf1 = qa.quantity('1kg')
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setrestfrequency(rf1)
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            ok = false
        if ok: fail('setrestfrequency 3 unexpectedly did not fail')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, spectral=F)
        try:
            note('Expect SEVERE error and Exception here')
            rf = mycs.restfrequency()
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            rf = false
        if rf: fail('restfrequency unexpectedly did not fail')
        #
        rf1 = qa.quantity('1GHz')
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setrestfrequency(rf1)
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            ok = false
        if ok: fail('setrestfrequency 4 unexpectedly did not fail')
        #
        if not mycs.done(): fail()
        #
        return T

    def test3():
        info('')
        info('')
        info('')
        info('Test 3 - torecord, fromrecord, copy')
        #
        mycs = cs.newcoordsys(direction=T, spectral=T, stokes="I Q U V",
                              linear=3)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        r = mycs.torecord()
        if not r: fail('torecord 1 failed')
        ok = r.has_key('direction0') and r.has_key('stokes1')
        ok = ok and r.has_key('spectral2')
        ok = ok and r.has_key('linear3')
        if not ok: fail('torecord did not produce valid record')
        #
        cs2 = cs.newcoordsys(direction=F, spectral=F, stokes="", linear=0)
        if not cs2: fail('coordsys constructor 2 failed')
        ok = cs2.fromrecord(r)
        if not ok: fail('fromrecord 1 failed')
        #if not is_coordsys(cs2)) {
        # fail('fromrecord 1 did not produce a coordsys tool'
        #
        if not mycs.done(): fail()
        if not cs2.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 3 failed')
        cs2 = mycs.copy()
        if not cs2: fail()
        ok = mycs.done()
        if (ok==false and cs2==false): fail('copy was a reference !')
        if not cs2.done(): fail;
        ###
        return T

    def test4():
        info('')
        info('')
        info('')
        info('Test 4 - setepoch, epoch, setobserver, observer')
        info('         settelescope, telescope, setparentname, parentname')
        #
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        # Epoch
        #
        info('')
        info ('Testing epoch')
        info('')
        epoch1 = me.epoch('UTC', 'today')
        ok = mycs.setepoch(epoch1)
        if not ok: fail()
        epoch2 = mycs.epoch()
        #
        ok = abs(me.getvalue(epoch1)['m0']['value']-me.getvalue(epoch2)['m0']['value'])<1.0e-6
        ok = ok and me.getvalue(epoch1)['m0']['unit'] == me.getvalue(epoch2)['m0']['unit']
        ok = ok and me.gettype(epoch1) == me.gettype(epoch2)
        ok = ok and me.getref(epoch1) == me.getref(epoch2)
        if not ok:
            fail('setepoch/epoch reflection failed')
        #
        # Observer
        #
        info('')
        info ('Testing observer')
        info('')
        obs1 = 'Biggles'
        ok = mycs.setobserver(obs1)
        if not ok: fail()
        obs2 = mycs.observer()
        #
        ok = obs1==obs2
        if not ok:
            fail('setobserver/observer reflection failed')
        #
        # Telescope
        #
        info('')
        info ('Testing telescope')
        info('')
        tel1 = 'VLA'
        ok = mycs.settelescope(tel1)
        if not ok: fail()
        tel2 = mycs.telescope()
        #
        ok = tel1==tel2
        if not ok:
            fail('settelescope/telescope reflection failed')
        pos = me.observatory(mycs.telescope(T))
        if not me.ismeasure(pos): fail('telescope 1 failed')
        #
        # Parent name
        #
        info('')
        info ('Testing parentname')
        info('')
        pn1 = 'Biggles.image'
        ok = mycs.setparentname(pn1)
        if not ok: fail()
        pn2 = mycs.parentname()
        #
        ok = pn1==pn2
        if not ok:
            fail('setparentname/parentname reflection failed')
        #
        if not mycs.done(): fail()
        #
        ###
        return T

    def test5():
        info('')
        info('')
        info('')
        info('Test 5 - setnames, names, setunits, units')
        info('         setreferencepixel, referencepixel')
        info('         setreferencevalue, referencevalue')
        info('         setincrement, increment')
        info('         setlineartransform, lineartransform')
        info('         setstokes, stokes')
        #
        mycs = cs.newcoordsys(direction=T, spectral=T)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        # Names
        #
        info('')
        info ('Testing names')
        info('')
        val1 = "a b c"
        ok = mycs.setnames(value=val1)
        if not ok: fail()
        val2 = mycs.names()
        #
        ok = val1.split()==val2
        if not ok: fail('setnames/names reflection 1 failed')
        #
        val2 = mycs.names('spec')
        ok = val2[0]==val1.split()[2]
        if not ok: fail('names 1 failed')
        #
        val1 = 'fish'
        ok = mycs.setnames(type='spec', value=val1)
        if not ok: fail()
        val2 = mycs.names('spec')
        ok = val2==val1
        if not ok: fail('setnames/names reflection 2 failed')
        if not mycs.done(): fail()
        #
        # Units
        #
        info('')
        info ('Testing units')
        info('')
        mycs = cs.newcoordsys(direction=T, spectral=T)
        if not mycs: fail('coordsys constructor 2 failed')
        val1 = "deg rad GHz"
        ok = mycs.setunits(value=val1)
        if not ok: fail()
        val2 = mycs.units()
        #
        ok = val1.split()==val2
        if not ok: fail('setunits/units 1 reflection failed')
        #
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setunits(value="Hz Hz Hz")
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            ok = false
        if ok: fail('setunits 2 unexpectedly did not fail')
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setunits(value="m")
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            ok = false
        if ok: fail('setunits 3 unexpectedly did not fail')
        #
        val1 = "deg rad GHz"
        ok = mycs.setunits(value=val1)
        if not ok: fail()
        val2 = mycs.units('spec')
        ok = val2==val1.split()[2]
        if not ok: fail('units 1 failed')
        #
        val1 = 'kHz'
        ok = mycs.setunits(type='spec', value=val1)
        if not ok: fail()
        val2 = mycs.units('spec')
        ok = val2[0]==val1[0]
        if not ok: fail('setunits/units reflection 2 failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, linear=2)
        if not mycs: fail('coordsys constructor 2b failed')
        val1 = "Hz kHz"
        ok = mycs.setunits(type='linear', value=val1, overwrite=T)
        if not ok: fail()
        val2 = mycs.units()
        ok = val1.split()[0]==val2[2] and val1.split()[1]==val2[3]
        if not ok: fail('setunits overwrite test failed')
        if not mycs.done(): fail;
        #
        # Reference pixel
        #
        info('')
        info ('Testing referencepixel')
        info('')
        mycs = cs.newcoordsys(direction=T, spectral=T)
        if not mycs: fail('coordsys constructor 3 failed')
        val1 = [0,1,2]
        ok = mycs.setreferencepixel(value=val1)
        if not ok: fail()
        val2 = mycs.referencepixel()['numeric']
        #
        ok = abs(val1[0]-val2[0])<1.0e-6
        ok = ok and abs(val1[1]-val2[1])<1.0e-6
        ok = ok and abs(val1[2]-val2[2])<1.0e-6
        if not ok:
            fail('setreferencepixel/referencepixel reflection failed')
        #
        val2 = mycs.referencepixel('dir')['numeric']
        if len(val2)!=2: fail()
        ok = abs(val2[0]-val1[0])<1.0e-6 and abs(val2[1]-val1[1])<1.0e-6
        if not ok: fail('referencepixel 1 failed')
        val2 = mycs.referencepixel('spec')['numeric']
        if len(val2)!=1: fail()
        ok = abs(val2-val1[2])<1.0e-6
        if not ok: fail('referencepixel 2 failed')
        #
        val1 = [0,0]
        ok = mycs.setreferencepixel(type='dir', value=val1)
        val2 = mycs.referencepixel('dir')['numeric']
        if len(val2)!=2: fail()
        ok = abs(val2[0]-val1[0])<1e-6 and abs(val2[1]-val1[1])<1e-6
        if not ok: fail('setreferencepixel 3 failed')
        #
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setreferencepixel (type='lin', value=[0,0])
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            ok = false
        if ok: fail('setreferencepixel 1 unexpectedly did not fail')
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.referencepixel('lin')
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            ok = false
        if ok: fail('setreferencepixel 2 unexpectedly did not fail')
        if not mycs.done(): fail()
        #
        # linear transform
        #
        info('')
        info ('Testing lineartransform')
        info('')
        mycs = cs.newcoordsys(direction=T, spectral=T, stokes='IQ', tabular=T, linear=3)
        if not mycs: fail('coordsys constructor 3b failed')
        #
        val1 = ia.makearray(0,[2,2])
        val1[0,0] = 2.0
        val1[1,1] = 3.0
        type = 'direction'
        ok = mycs.setlineartransform(value=val1, type=type)
        if not ok: fail()
        val2 = mycs.lineartransform(type=type)
        #
        ok = alleq(val1,val2,1.0e-6)
        if not ok:
            fail('direction setlineartransform/lineartransform reflection failed')
        ##
        val1 = ia.makearray(2,[1,1])
        type = 'spectral'
        ok = mycs.setlineartransform(value=val1, type=type)
        if not ok: fail()
        val2 = mycs.lineartransform(type=type)
        #
        ok = alleq(val1,val2,1.0e-6)
        if not ok:
            fail('spectral setlineartransform/lineartransform reflection failed')
        ##
        val1 = ia.makearray(2,[1,1])
        type = 'stokes'
        ok = mycs.setlineartransform(value=val1, type=type)
        if not ok: fail()
        val2 = mycs.lineartransform(type=type);  # Does not set ; returns T
        #
        ok = alleq(val1,val2,1.0e-6)
        if ok:
            fail('stokes setlineartransform/lineartransform reflection failed')
        ##
        val1 = ia.makearray(4,[1,1])
        type = 'tabular'
        ok = mycs.setlineartransform(value=val1, type=type)
        if not ok: fail()
        val2 = mycs.lineartransform(type=type)
        #
        ok = alleq(val1,val2,1.0e-6)
        if not ok:
            fail('tabular setlineartransform/lineartransform reflection failed')
        ##
        val1 = ia.makearray(0,[3,3])
        val1[0,0] = 2.0
        val1[1,1] = 3.0;
        val1[2,2] = 4.0
        type = 'linear'
        ok = mycs.setlineartransform(value=val1, type=type)
        if not ok: fail()
        val2 = mycs.lineartransform(type=type)
        #
        ok = alleq(val1,val2,1.0e-6)
        if not ok:
            fail('direction setlineartransform/lineartransform reflection failed')
        if not mycs.done(): fail()
        #
        # Reference value
        #
        info('')
        info ('Testing referencevalue')
        info('')
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 4 failed')
        ok = mycs.setunits(value="rad rad")
        if not ok: fail()
        val1 = mycs.referencevalue(format='q')
        val1['quantity']['*1'] = qa.quantity('0.01rad')
        val1['quantity']['*2'] = qa.quantity('-0.01rad')
        ok = mycs.setreferencevalue(value=val1)
        if not ok: fail()
        val2 = mycs.referencevalue(format='q')
        if not val2: fail()
        #
        ok = abs(val1['quantity']['*1']['value']-val2['quantity']['*1']['value'])<1e-6
        ok = ok and abs(val1['quantity']['*2']['value']-val2['quantity']['*2']['value'])<1e-6
        ok = ok and val1['quantity']['*1']['unit']==val2['quantity']['*1']['unit']
        ok = ok and val1['quantity']['*2']['unit']==val2['quantity']['*2']['unit']
        if not ok:
            fail('setreferencevalue/referencevalue 1 reflection failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 5 failed')
        units = mycs.units()
        if not units: fail()
        val1 = [1.0,2.0]
        ok = mycs.setreferencevalue(value=val1)
        if not ok: fail()
        val2 = mycs.referencevalue(format='q')
        if not val2: fail()
        ok = abs(val1[0]-val2['quantity']['*1']['value'])<1e-6
        ok = ok and abs(val1[1]-val2['quantity']['*2']['value'])<1e-6
        ok = ok and units[0]==val2['quantity']['*1']['unit']
        ok = ok and units[1]==val2['quantity']['*2']['unit']
        if not ok: fail('setreferencevalue/referencevalue 2 reflection failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(spectral=T)
        if not mycs: fail('coordsys constructor 6 failed')
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setreferencevalue (value='i like doggies')
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('setreferencevalue unexpectedly did not fail')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, spectral=T)
        if not mycs: fail('coordsys constructor 7 failed')
        val1 = mycs.referencevalue(format='q')
        if not val1: fail()
        val2 = mycs.referencevalue(type='spec', format='q')
        if not val2: fail()
        ok = abs(val1['quantity']['*3']['value']-val2['quantity']['*1']['value'])<1e-6 and val1['quantity']['*3']['unit']==val2['quantity']['*1']['unit']
        if not ok: fail('referencevalue 1 failed')
        #
        val1 = [-10]
        ok = mycs.setreferencevalue(type='spec', value=val1)
        if not ok: fail()
        val2 = mycs.referencevalue(type='spec', format='n')
        if not val2: fail()
        ok = abs(val1[0]-val2['numeric'][0])<1e-6
        if not ok: fail('setreferencevalue 1 failed')
        #
        val1 = mycs.referencevalue(format='n')
        if not val1: fail()
        val2 = mycs.referencevalue(type='spec', format='n')
        if not val2: fail()
        ok = abs(val1['numeric'][2]-val2['numeric'][0])<1e-6
        if not ok: fail('referencevalue 2 failed')
        if not mycs.done(): fail()
        #
        # increment
        #
        info('')
        info ('Testing increment')
        info('')
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 7 failed')
        ok = mycs.setunits(value="rad rad")
        if not ok: fail()
        val1 = mycs.increment(format='q')
        val1['quantity']['*1'] = qa.quantity('0.01rad')
        val1['quantity']['*2'] = qa.quantity('-0.01rad')
        ok = mycs.setincrement(value=val1)
        if not ok: fail()
        val2 = mycs.increment(format='q')
        if not val2: fail()
        #
        ok = abs(val1['quantity']['*1']['value']-val2['quantity']['*1']['value'])<1e-6
        ok = ok and abs(val1['quantity']['*2']['value']-val2['quantity']['*2']['value'])<1e-6
        ok = ok and val1['quantity']['*1']['unit']==val2['quantity']['*1']['unit']
        ok = ok and val1['quantity']['*2']['unit']==val2['quantity']['*2']['unit']
        if not ok:
            fail('setincrement/increment 1 reflection failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 8 failed')
        units = mycs.units()
        if not units: fail()
        val1 = [1.0,2.0]
        ok = mycs.setincrement(value=val1)
        if not ok: fail()
        val2 = mycs.increment(format='q')
        if not val2: fail()
        ok = abs(val1[0]-val2['quantity']['*1']['value'])<1e-6
        ok = ok and abs(val1[1]-val2['quantity']['*2']['value'])<1e-6
        ok = ok and units[0]==val2['quantity']['*1']['unit']
        ok = ok and units[1]==val2['quantity']['*2']['unit']
        if not ok:
            fail('setincrement/increment 2 reflection failed')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(spectral=T)
        if not mycs: fail('coordsys constructor 9 failed')
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setincrement(value='i like doggies')
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('setincrement 1 unexpectedly did not fail')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, spectral=T)
        if not mycs: fail('coordsys constructor 10 failed')
        val1 = [1.0, 2.0, 3.0]
        ok = mycs.setincrement(value=val1)
        if not ok: fail()
        val2 = mycs.increment(format='n', type='dir')
        ok = abs(val2['numeric'][0]-val1[0])<1e-6 and abs(val2['numeric'][1]-val1[1])<1e-6
        if not ok: fail('setincrement/increment 3 reflection failed')
        val2 = mycs.increment(type='spe',format='n')
        ok = abs(val2['numeric'][0]-val1[2])<1e-6
        if not ok: fail('setincrement/increment 4 reflection failed')
        try:
            note('Expect SEVERE error and Exception here')
            val2 = mycs.increment(type='lin', format='q')
        except Exception, e:
            note('Caught expected Exception')
            val2 = false
        if val2: fail('increment 2 unexpectedly did not fail')
        #
        val1 = [-10]
        ok = mycs.setincrement(type='spec', value=val1)
        if not ok: fail()
        val2 = mycs.increment(type='spec', format='n')
        if not val2: fail()
        ok = abs(val1[0]-val2['numeric'][0])<1e-6
        if not ok: fail('setincrement/increment 5 reflection failed')
        if not mycs.done(): fail;
        #
        # Stokes
        #
        mycs = cs.newcoordsys(stokes="I RL")
        if not mycs: fail()
        stokes = mycs.stokes()
        if not stokes: fail()
        if (stokes[0]!='I' and stokes[1]!='RL'):
            fail('stokes 1  recovered wrong values')
        ok = mycs.setstokes("XX V")
        if not ok: fail()
        stokes = mycs.stokes()
        if not stokes: fail()
        if (stokes[0]!='XX' and stokes[1]!='V'):
            fail('stokes 2 recovered wrong values')
        if not mycs.done(): fail;
        #
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail()
        try:
            stokes = true
            note('Expect SEVERE error and Exception here')
            stokes = mycs.stokes()
        except Exception, e:
            note('Caught expected Exception')
            stokes = false
        if stokes: fail('stokes 2 unexpectedly did not fail')
        try:
            ok = true
            note('Expect SEVERE error and Exception here')
            ok = mycs.setstokes("I V")
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('setstokes 2 unexpectedly did not fail')
        #
        if not mycs.done(): fail;
        #
        ###
        return T

    def test6():
        info('')
        info('')
        info('')
        info('Test 6 - findcoordinate, findaxis')
        #
        mycs = cs.newcoordsys()
        if not mycs: fail('coordsys constructor 1 failed')

        # findcoordinate
        #local pa, wa
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.findcoordinate('fish', 1)
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            return stop('findcoordinate 1 unexpectedly did not fail')
        ok = mycs.findcoordinate('dir', 20)
        if ok['return']:
            return stop('findcoordinate 2 unexpectedly did not fail')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, stokes="I V", spectral=T, linear=2)
        if not mycs: fail('coordsys constructor 1 failed')
        ok = mycs.findcoordinate('dir',0)
        if not ok or not ok['return']:
            return stop('findcoordinate 3 failed')
        pa = ok['pixel']
        wa = ok['world']
        if not (pa[0] == 0 and pa[1] == 1):
            return stop('find 3 pixel axes are wrong')
        if not (wa[0] == 0 and wa[1] == 1):
            return stop('find 3 world axes are wrong')
        #
        ok = mycs.findcoordinate('stokes',0)
        if not ok or not ok['return']:
            return stop('findcoordinate 4 failed')
        pa=ok['pixel']
        wa=ok['world']
        if not pa==2:
            return stop('findcoordinate 4 pixel axes are wrong')
        if not wa==2:
            return stop('findcoordinate 4 world axes are wrong')
        #
        ok = mycs.findcoordinate('spectral',0)
        if not ok or not ok['return']:
            return stop('findcoordinate 5 failed')
        pa=ok['pixel']
        wa=ok['world']
        if not pa==3:
            return stop('findcoordinate 5 pixel axes are wrong')
        if not wa==3:
            return stop('findcoordinate 5 world axes are wrong')
        #
        #
        ok = mycs.findcoordinate('linear',0)
        if not ok or not ok['return']:
            return stop('findcoordinate 6 failed')
        pa=ok['pixel']
        wa=ok['world']
        if not (pa[0]==4 and pa[1]==5):
            return stop('findcoordinate 6 pixel axes are wrong')
        if not (wa[0]==4 and wa[1]==5):
            return stop('findcoordinate 6 world axes are wrong')
        #
        if not mycs.done(): fail()
        #
        # findaxis
        #
        mycs = cs.newcoordsys(direction=T, linear=2)
        #local coord, axisincoord
        #
        ok = mycs.findaxis(T, 0)
        if not ok: fail()
        coord=ok['coordinate']
        axisincoord=ok['axisincoordinate']
        if (coord!=0 or axisincoord!=0):
            return stop('findaxis 0 values are wrong')
        #
        ok = mycs.findaxis(T, 1)
        if not ok: fail()
        coord=ok['coordinate']
        axisincoord=ok['axisincoordinate']
        if (coord!=0 or axisincoord!=1):
            return stop('findaxis 1 values are wrong')
        #
        ok = mycs.findaxis(T, 2)
        if not ok: fail()
        coord=ok['coordinate']
        axisincoord=ok['axisincoordinate']
        if (coord!=1 or axisincoord!=0):
            return stop('findaxis 2 values are wrong')
        #
        ok = mycs.findaxis(T, 3)
        if not ok: fail()
        coord=ok['coordinate']
        axisincoord=ok['axisincoordinate']
        if (coord!=1 or axisincoord!=1):
            return stop('findaxis 3 values are wrong')
        #
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.findaxis(T, 4)
        except Exception, e:
            note('Caught expect Exception:' + str(e))
            ok = false
        if ok:
            return stop('findaxis 4 unexpectedly found the axis')
        #
        if not mycs.done(): fail()

        ###
        return T
 
    def test7():
        info('')
        info('')
        info('')
        info('Test 7 - toworld, toworldmany, topixel, topixelmany')
        #
        mycs = cs.newcoordsys(direction=T, spectral=T, stokes="I V", linear=2)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        info('')
        info('Testing toworld')
        info('')
        #
        rp = mycs.referencepixel()['numeric']
        if len(rp)!=6: fail()
        rv = mycs.referencevalue(format='n')
        if not rv: fail()
        #d = abs(mycs.toworld(value=rp, format='n') - rv)
        #if not all(d<1e-6): fail('toworld 1 gives wrong values')
        if not alleq(mycs.toworld(value=rp, format='n')['numeric'],
                     rv['numeric'],tolerance=1e-6):
            fail('toworld 1 gives wrong values')
        #
        d = mycs.toworld(value=list(rp), format='q')
        u = mycs.units()
        if not u: fail()
        #
        if len(d['quantity'])!=len(rv['numeric']):
            fail('toworld 2 gives wrong number of quantities')
        for i in range(len(d['quantity'])):
            if abs(d['quantity']['*'+str(i+1)]['value']-rv['numeric'][i])>1e-6:
                fail('toworld 2 gives wrong values')
            if d['quantity']['*'+str(i+1)]['unit'] != u[i]:
                fail('toworld 2 gives wrong units')
        #
        q = mycs.toworld(value=rp, format='q')
        if not q: fail()
        m = mycs.toworld(value=rp, format='m')
        if not m: fail()
        m = m['measure']
        ok = m.has_key('direction') and m.has_key('spectral')
        ok = ok and m['spectral'].has_key('frequency')
        ok = ok and m['spectral'].has_key('opticalvelocity')
        ok = ok and m['spectral'].has_key('radiovelocity')
        ok = ok and m['spectral'].has_key('betavelocity')
        ok = ok and m.has_key('stokes')
        ok = ok and m.has_key('linear')
        if not ok: fail('toworld 3 gives wrong fields')
        d = m['direction']
        f = m['spectral']['frequency']
        l = m['linear']
        s = m['stokes']
        #
        v = me.getvalue(d)
        q['quantity']['*1'] = qa.convert(q['quantity']['*1'], v['m0']['unit'])
        q['quantity']['*2'] = qa.convert(q['quantity']['*2'], v['m1']['unit'])
        ok = abs(v['m0']['value']-q['quantity']['*1']['value'])<1e-6
        ok = ok and abs(v['m1']['value']-q['quantity']['*2']['value'])<1e-6
        ok = ok and v['m0']['unit']==q['quantity']['*1']['unit']
        ok = ok and v['m1']['unit']==q['quantity']['*2']['unit']
        if not ok: fail('toworld 3 gives wrong direction values')
        #
        v = me.getvalue(f)
        q['quantity']['*4'] = qa.convert(q['quantity']['*4'], v['m0']['unit'])
        ok = abs(v['m0']['value']-q['quantity']['*4']['value'])<1e-6
        ok = ok and v['m0']['unit']==q['quantity']['*4']['unit']
        if not ok: fail('toworld 3 gives wrong frequency values')
        #
        q['quantity']['*5'] = qa.convert(q['quantity']['*5'], l['*1']['unit'])
        q['quantity']['*6'] = qa.convert(q['quantity']['*6'], l['*2']['unit'])
        ok = abs(l['*1']['value']-q['quantity']['*5']['value'])<1e-6
        ok = ok and abs(l['*2']['value']-q['quantity']['*6']['value'])<1e-6
        ok = ok and l['*1']['unit']==q['quantity']['*5']['unit']
        ok = ok and l['*2']['unit']==q['quantity']['*6']['unit']
        if not ok: fail('toworld 3 gives wrong linear values')
        #
        # toworldmany - any is as good as any other
        #
        p = mycs.referencepixel()['numeric']
        w = mycs.referencevalue()
        rIn = ia.makearray(0, [len(p), 10])
        for i in range(10):
            for j in range(len(p)):
                rIn[j,i] = p[j]
        rOut = mycs.toworldmany(rIn)
        if len(rOut['numeric'])!=len(rIn): fail()
        for i in range(10):
            for j in range(len(p)):
                if not (rOut['numeric'][j,i] - w['numeric'][j]) < 1e-6:
                    fail('toworldmany 1 gives wrong values')
        #
        # topixel
        #
        info('')
        info('Testing topixel')
        info('')
        #
        tol = 1.0e-6
        rp = mycs.referencepixel()['numeric']
        if len(rp)!=6: fail()
        rv = mycs.referencevalue(format='n')
        if not rv: fail()
        p = mycs.topixel(value=rv)['numeric']
        if len(p)!=6: fail()
        if not all(p, rp, tol):
            fail('topixel 1 gives wrong values')
        #
        for format in ["n","q","m","s","nqms"]:
            for i in range(len(rp)): p[i] = rp[i]+1
            w = mycs.toworld(value=p, format=format)
            if not w: fail()
            #
            p2 = mycs.topixel(value=w)['numeric']
            if len(p2)!=6: fail()
            if not all(p, p2, tol):
                s = 'toworld/topixel reflection failed for format "'+format+'"'
                fail(s)
            #
            # topixelmany - any is as good as any other
            #
            n = 10
            p = mycs.referencepixel()['numeric']
            w = mycs.toworld(p, 'n')
            w = w['numeric']
            rIn = ia.makearray(0, [len(w), n])
            for i in range(n):
                for j in range(len(w)):
                    rIn[j,i] = w[j]
            r2 = mycs.topixelmany(rIn)
            if len(r2['numeric'])!=len(rIn): fail()
            for i in range(n):
                for j in range(len(w)):
                    if not abs((p[j]-r2['numeric'][j,i]) < 1e-6):
                        fail('topixelmany 1 gives wrong values')
            #
            if not mycs.done(): fail()
            ###
            return T

    def test8():
        info('')
        info('')
        info('')
        info('Test 8 - naxes, axesmap')
        #
        info('')
        info('Testing naxes')
        info('')
        mycs = cs.newcoordsys()
        if not mycs: fail('coordsys constructor 1 failed')
        n = mycs.naxes()
        if not n==0: fail('naxes 1 failed')
        if mycs.naxes()!=0: fail('naxes 1 gave wrong result')
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, spectral=T, stokes="I V", linear=2)
        if not mycs: fail('coordsys constructor 2 failed')
        n = mycs.naxes()
        if not n: fail('naxes 2 failed')
        if mycs.naxes()!=6: fail('naxes 2 gave wrong result')
        #
        info('')
        info('Testing axesmap')
        info('')
        #
        # Since I have no way to reorder the world and pixel axes
        # from Glish, all I can do is check the maps are the
        # same presently
        #
        toworld = mycs.axesmap(toworld=T)
        if not toworld: fail()
        topixel = mycs.axesmap(toworld=F)
        if not topixel: fail()
        #
        idx = range(0,len(mycs.referencepixel()['numeric']))
        if not all(toworld,idx): fail('toworld map is wrong')
        if not all(topixel,idx): fail('topixel map is wrong')
        #
        if not mycs.done(): fail()
        ###
        return T

    def test9():
        info('')
        info('')
        info('')
        info('Test 9 - reorder')
        #
        info('')
        info('Testing reorder')
        info('')
        mycs = cs.newcoordsys(direction=T, spectral=T, stokes='I V', linear=1)
        if not mycs: fail('coordsys constructor 1 failed')
        #order = [4,3,2,1]
        order = [3,2,1, 0]
        ok = mycs.reorder(order)
        if not ok: fail('reorder 1 failed')
        ok = mycs.coordinatetype(0)=='Linear'
        ok = ok and mycs.coordinatetype(1)=='Spectral'
        ok = ok and mycs.coordinatetype(2)=='Stokes'
        ok = ok and mycs.coordinatetype(3)=='Direction'
        if not ok: fail('reorder reordered incorrectly')
        #
        try:
            note("Expect SEVERE error and Exception here")
            ok = mycs.reorder([1,2])
        except Exception, e:
            note("Caught expected Exception")
            ok = false
        if ok: fail('reorder 2 unexpectedly did not fail')
        #
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.reorder([1,2,3,10])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('reorder 3 unexpectedly did not fail')
        #
        if not mycs.done(): fail()
        ###
        return T

    def test10():
        info('')
        info('')
        info('')
        info('Test 10 - frequencytovelocity, velocitytofrequency')
        #
        mycs = cs.newcoordsys(spectral=T)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        info('')
        info('Testing frequencytovelocity')
        info('')

        # Set rest freq to reference freq
        rv = mycs.referencevalue(format='n')
        if not rv: fail()
        restFreq = rv['numeric'][0]
        ok = mycs.setrestfrequency(restFreq)
        if not ok: fail()

        # Find radio velocity increment
        df = mycs.increment(format='n')
        c = qa.constants('c')['value']                          #m/s
        drv = -c * df['numeric'] / rv['numeric'][0] / 1000.0    #km/s
        #
        freq = rv['numeric'][0]
        freqUnit = mycs.units();
        vel = mycs.frequencytovelocity(value=freq, frequnit=freqUnit,
                                       doppler='radio', velunit='km/s')
        if (abs(vel) > 1e-6):
            fail('frequencytovelocity 1 got wrong values')
        freq2 = mycs.velocitytofrequency(value=vel, frequnit=freqUnit,
                                         doppler='optical', velunit='km/s')
        if (abs(freq2-freq) > 1e-6):
            fail('velocitytofrequency 1 got wrong values')
        ##
            vel = mycs.frequencytovelocity(value=freq, frequnit=freqUnit,
                                           doppler='optical', velunit='km/s')
        if (abs(vel) > 1e-6):
            fail('frequencytovelocity 2 got wrong values')
        #
        freq2 = mycs.velocitytofrequency(value=vel, frequnit=freqUnit,
                                         doppler='optical', velunit='km/s')
        if (abs(freq2-freq) > 1e-6):
            fail('velocitytofrequency 2 got wrong values')
        ##
        rp = mycs.referencepixel()['numeric']
        if rp!=0.0: fail()
        freq = mycs.toworld (value=rp+1, format='n')
        vel = mycs.frequencytovelocity(value=list(freq['numeric']),
                                       frequnit=freqUnit,
                                       doppler='radio', velunit='m/s')
        d = abs(vel - (1000.0*drv))
        if (d > 1e-6):
            fail('frequencytovelocity 3 got wrong values')
        freq2 = mycs.velocitytofrequency(value=vel, frequnit=freqUnit,
                                         doppler='radio', velunit='m/s')
        if (abs(freq2-freq['numeric']) > 1e-6):
            fail('velocitytofrequency 3 got wrong values')
        ##
        freq = [rv['numeric'][0], freq['numeric'][0]]
        vel = mycs.frequencytovelocity(value=freq, frequnit=freqUnit,
                                       doppler='radio', velunit='m/s')
        if (len(vel)!=2):
            fail('frequencytovelocity 4 returned wrong length vector')
        d1 = abs(vel[0] - 0.0)
        d2 = abs(vel[1] - (1000.0*drv))
        if (d1>1e-6 or d2>1e-6):
            fail('frequencytovelocity 4 got wrong values')
        freq2 = mycs.velocitytofrequency(value=vel, frequnit=freqUnit,
                                         doppler='radio', velunit='m/s')
        d1 = abs(freq[0] - freq2[0])
        d2 = abs(freq[1] - freq2[1])
        if (d1>1e-6 or d2>1e-6):
            fail('velocitytofrequency 4 got wrong values')

        # Forced errors
        try:
            note('Expect SEVERE error and Exception here')
            vel = true
            vel = mycs.frequencytovelocity(value=rv['numeric'][0],
                                           frequnit='Jy',
                                           doppler='radio', velunit='km/s')
        except Exception, e:
            note('Caught expected Exception')
            vel = false
        if vel: fail('frequencytovelocity 5 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            freq = true
            freq = mycs.velocitytofrequency(value=rv['numeric'][0],
                                            frequnit='Jy',
                                            doppler='radio', velunit='km/s')
        except Exception, e:
            note('Caught expected Exception')
            freq = false
        if freq: fail('velocitytofrequency 5 unexpectedly did not fail')
        ##
        try:
            note('Expect SEVERE error and Exception here')
            vel = true
            vel = mycs.frequencytovelocity(value=rv['numeric'][0],
                                           frequnit='GHz',
                                           doppler='radio', velunit='doggies')
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            vel = false
        if vel: fail('frequencytovelocity 6 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            freq = true
            freq = mycs.velocitytofrequency(value=rv['numeric'][0],
                                            frequnit='GHz',
                                            doppler='radio', velunit='doggies')
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            freq = false
        if freq:
            fail('velocitytofrequency 6 unexpectedly did not fail')
        #
        if not mycs.done(): fail()
        #
        mycs = cs.newcoordsys(direction=T, spectral=F)
        if not mycs: fail('coordsys constructor 2 failed')
        try:
            note('Expect SEVERE error and Exception here')
            vel = true
            vel = mycs.frequencytovelocity(value=[1.0], frequnit='Hz',
                                           doppler='radio', velunit='km/s')
        except Exception, e:
            note('Caught expected Exception')
            vel = false
        if vel:
            fail('frequencytovelocity 7 unexpectedly did not fail')
        if not mycs.done(): fail()
        ###
        return T

    def test11():
        info('')
        info('')
        info('')
        info('Test 11 - setreferencelocation')
        #
        mycs = cs.newcoordsys(linear=2, spectral=T)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        p = [1.0, 1.0, 1.0]
        ok = mycs.setreferencepixel(value=p)
        if not ok: fail()
        rp = mycs.referencepixel()['numeric']
        if len(rp)!=3: fail()
        if not all(rp,p,1e-6): fail('setreferencepixel/referencepixel reflection failed')
        #
        shp = [101,101,10]
        inc = mycs.increment(format='n')['numeric']
        if len(inc)!=3: fail()
        w = mycs.toworld([1,1,1], 'n')['numeric']
        if len(w)!=3: fail()
        w += inc
        p = [51,51,5]  #p = ((shp-1)/2.0) + 1
        #
        ok = mycs.setreferencelocation (pixel=p, world=w, mask=[T,T,T])
        if not ok: fail()
        #
        rp = mycs.referencepixel()['numeric']
        if len(rp)!=3: fail()
        rv = mycs.referencevalue(format='n')['numeric']
        if len(rv)!=3: fail()
        #
        ok = abs(rv[0]-w[0])<1e-6 and abs(rv[1]-w[1])<1e-6
        ok = ok and abs(rv[2]-w[2])<1e-6
        if not ok: fail('setreferencelocation recovered wrong reference value')
        #
        ok = abs(rp[0]-p[0])<1e-6 and abs(rp[1]-p[1])<1e-6
        ok = ok and abs(rp[2]-p[2])<1e-6
        if not ok: fail('setreferencelocation recovered wrong reference pixel')
        #
        if not mycs.done(): fail;
        #
        ###
        return T

    def test12():
        info('')
        info('')
        info('')
        info('Test 12 - toabs, torel, toabsmany, torelmany')
        #
        mycs = cs.newcoordsys(direction=T, spectral=T, stokes="I V LL", linear=2)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        info('')
        info('Testing torel/toabs on pixel coordinates')
        info('')
        #
        p = mycs.referencepixel()
        pn = p['numeric']  # pointer to numeric part of p
        for i in range(len(pn)): pn[i]=pn[i]+1
        if len(pn)!=6: fail()
        p2 = mycs.torel(p)
        if not p2: fail()
        p3 = mycs.toabs(p2)['numeric']
        if len(p3)==0: fail()
        d = abs(p3 - pn)
        for i in range(len(d)):
            if d[i]>1e-6 : fail('torel/toabs pixel reflection test 1 failed')
        #
        info('')
        info('Testing torel/toabs on world coordinates')
        info('')
        #
        p = mycs.referencepixel()['numeric']
        for i in range(len(p)): p[i] += 1
        if len(p)!=6: fail()
        #
        for f in ["n","q","s"]:
            w = mycs.toworld(p, format=f)
            if not w: fail()
            #
            w2 = mycs.torel(w)
            if not w2: fail()
            w3 = mycs.toabs(w2)
            if not w3: fail()
            #
            p2 = mycs.topixel(w3)['numeric']
            if len(p2)!=6: fail()
            if not all(p2,p,1e-6):
                s = 'torel/toabs world reflection test 1 failed for format "'+f+'"'
                fail(s)
        #
        p = mycs.referencepixel()
        if not p: fail()
        try:
            note("Expect SEVERE error and Exception here")
            p2 = mycs.toabs(p)
        except Exception, e:
            note("Caught expected Exception")
            p2 = false
        if p2: fail('toabs 1 unexpectedly did not fail')
        #
        p2 = mycs.torel(p)
        if not p2: fail()
        try:
            note('Expect SEVERE error and Exception here')
            p3 = mycs.torel(p2)
        except Exception, e:
            note('Caught expected Exception')
            p3 = false
        if p3: fail('torel 1 unexpectedly did not fail')
        #
        w = mycs.referencevalue()
        if not w: fail()
        try:
            note('Expect SEVERE error and Exception here')
            w2 = mycs.toabs(w)
        except:
            note('Caught expected exception')
            w2 = false
        if w2: fail('toabs 2 unexpectedly did not fail')
        #
        w2 = mycs.torel(w)
        if not w2: fail()
        try:
            note('Expect SEVERE error and Exception here')
            w3 = mycs.torel(w2)
        except Exception, e:
            note('Caught expected Exception')
            w3 = false
        if w3: fail('torel 2 unexpectedly did not fail')
        #
        # toabsmany, torelmany
        #
        info('')
        info('Testing toabsmany/torelmany')
        info('')
        p = mycs.referencepixel()['numeric']
        w = mycs.toworld(p, 'n')['numeric']
        n = 5
        pp = ia.makearray(0.0, [len(p), n])
        ww = ia.makearray(0.0, [len(w), n])
        for i in range(n):
            for j in range(len(p)):
                pp[j,i] = p[i]
            for j in range(len(w)):
                ww[j,i] = w[i]
        #
        relpix = mycs.torelmany(pp, F)
        if len(relpix['numeric'])!=len(p): fail()
        abspix = mycs.toabsmany(relpix, F)
        if len(abspix['numeric'])!=len(relpix['numeric']): fail()
        #
        relworld = mycs.torelmany(ww, T)
        if len(relworld['numeric'])!=len(w): fail()
        absworld = mycs.toabsmany(relworld, T)
        if len(absworld['numeric'])!=len(relworld['numeric']): fail()
        #
        for i in range(n):
            for j in range(len(p)):
                if not abs(p[j]-abspix['numeric'][j,i])<1e-6:
                    fail('toabsmany/torelmany gives wrong values for pixels')
            #
            for j in range(len(w)):
                #if not (abs(w[j])-absworld[j,i])<1e-6:
                if not alleq(w[j],absworld['numeric'][j,i],1e-6):
                    fail('toabsmany/torelmany gives wrong values for world')
            #
        if not mycs.done(): fail()
        ###
        return T
 

    def test13():
        info('')
        info('')
        info('Test 13 - convert, convertmany')
        #
        mycs = cs.newcoordsys(direction=T, spectral=T, stokes="I V LL",
                              linear=2)
        if not mycs: fail('coordsys constructor 1 failed')
        tol = 1.0e-6
        n = mycs.naxes()

        ####################
        # abs pix to abs pix
        absin = n*[T]
        unitsin = n*['pix']
        coordin = mycs.referencepixel()['numeric'] # Make sure in range of stokes
        for i in range(len(coordin)):
            coordin[i] += 2
        absout = n*[T]
        unitsout = n*['pix']
        dopplerin = 'radio'
        dopplerout = 'radio'
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        if not all(p,coordin,tol): fail('convert 1 gives wrong values')

        # abs pix to rel pix
        absout = n * [F]
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.torel(coordin, F)['numeric']
        if len(p2)!=n: fail()
        if not all(p,p2,tol):
            fail('convert 2 gives wrong values')

        # rel pix to abs pix
        absin = n * [F]
        rp = mycs.referencepixel()['numeric']
        for i in range(len(rp)): rp[i] += 2
        coordin = list(mycs.torel(rp, F)['numeric'])
        if len(coordin)!=n: fail()
        absout = n * [T]
        #
        p = mycs.convert(coordin, absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.referencepixel()['numeric']
        if len(p2)!=n: fail()
        for i in range(len(p2)): p2[i] += 2
        if not all(p,p2,tol): fail('convert 3 gives wrong values')

        #######################
        # abs pix to abs world
        absin = n * [T]
        coordin = mycs.referencepixel()['numeric'] + 2
        if len(coordin)!=n: fail()
        unitsin = n * ['pix']
        absout = n * [T]
        unitsout = mycs.units()
        if not unitsout: fail()
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.referencepixel()['numeric'] + 2
        if len(p2)!=n: fail()
        w = mycs.toworld(p2)['numeric']
        if len(w)!=n: fail()
        if not all(p,w,tol): fail('convert 4 gives wrong values')

        # abs pix to rel world
        absin = n * [T]
        coordin = list(mycs.referencepixel()['numeric']+2)
        if len(coordin)!=n: fail()
        unitsin = n * ['pix']
        absout = n * [F]
        unitsout = mycs.units()
        if not unitsout: fail()
        #
        p = mycs.convert(coordin, absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.referencepixel()['numeric'] + 2
        if len(p2)!=n: fail()
        w = mycs.torel(mycs.toworld(p2),T)['numeric']
        if not all(p,w,tol): fail('convert 5 gives wrong values')
        
        # rel pix to abs world
        absin = n * [F]
        coordin = mycs.torel(mycs.referencepixel()['numeric']+2,F)
        if not coordin: fail()
        unitsin = n * ['pix']
        absout = n * [T]
        unitsout = mycs.units()
        if not unitsout: fail()
        #
        p = mycs.convert(list(coordin['numeric']), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.referencepixel()['numeric']+2
        if len(p2)!=n: fail()
        w = mycs.toworld(p2)['numeric']
        if len(w)!=n: fail()
        if not all(p,w,tol): fail('convert 6 gives wrong values')

        # rel pix to rel world
        absin = n * [F]
        coordin = mycs.torel(mycs.referencepixel()['numeric']+2,F)
        if not coordin: fail()
        unitsin = n * ['pix']
        absout = n * [F]
        unitsout = mycs.units()
        #
        p = mycs.convert(list(coordin['numeric']), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.referencepixel()['numeric']+2
        if len(p2)!=n: fail()
        w = mycs.torel(mycs.toworld(p2),T)['numeric']
        if len(w)!=6: fail()
        if not all(p,w,tol): fail('convert 7 gives wrong values')

        #######################
        # abs world to abs pix
        absin = n * [T]
        coordin = mycs.toworld(mycs.referencepixel()['numeric']+2)
        if not coordin: fail()
        unitsin = mycs.units()
        if not unitsout: fail()
        absout = n * [T]
        unitsout = n * ['pix']
        #
        p = mycs.convert(list(coordin['numeric']), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.referencepixel()['numeric'] + 2
        if len(p2)!=n: fail()
        if not all(p,p2,tol): fail('convert 8 gives wrong values')

        # abs world to rel pix
        absin = n * [T]
        coordin = mycs.toworld(mycs.referencepixel()['numeric']+2)
        if not coordin: fail()
        unitsin = mycs.units()
        if not unitsin: fail()
        unitsout = n * ['pix']
        absout = n * [F]
        #
        p = mycs.convert(list(coordin['numeric']), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.torel(mycs.referencepixel()['numeric']+2,F)['numeric']
        if len(p2)!=n: fail()
        if not all(p,p2,tol): fail('convert 9 gives wrong values')

        # rel world to abs pix
        absin = n * [F]
        coordin = mycs.torel(mycs.toworld(mycs.referencepixel()['numeric']+2),T)
        if not coordin: fail()
        unitsin = mycs.units()
        if not unitsin: fail()
        absout = n * [T]
        unitsout = n * ['pix']
        #
        p = mycs.convert(list(coordin['numeric']), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.referencepixel()['numeric']+2
        if len(p2)!=n: fail()
        if not all(p,p2,tol): fail('convert 10 gives wrong values')

        # rel world to rel pix
        absin = n * [F]
        coordin = mycs.torel(mycs.toworld(mycs.referencepixel()['numeric']+2),T)
        if not coordin: fail()
        unitsin = mycs.units()
        absout = n * [F]
        unitsout = n * ['pix']
        #
        p = mycs.convert(list(coordin['numeric']), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.torel(mycs.referencepixel()['numeric']+2,F)['numeric']
        if len(p2)!=n: fail()
        if not all(p, p2, tol): fail('convert 11 gives wrong values')

        # velocity
        ok = mycs.findcoordinate('spectral')
        if not ok['return']: fail()
        pa = ok['pixel']
        wa = ok['world']
        #
        sAxis = pa
        dopplerin = 'radio'
        dopplerout = 'optical'
        vRefIn = mycs.frequencytovelocity(
            value=mycs.referencevalue()['numeric'][sAxis],
            doppler=dopplerin,
            velunit='km/s')
        if not vRefIn: fail()
        vRefOut = mycs.frequencytovelocity(
            value=mycs.referencevalue()['numeric'][sAxis],
            doppler=dopplerout,
            velunit='km/s')
        if not vRefOut: fail()

        # absvel to absvel
        absin = n * [T]
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        coordin = mycs.toworld(p)
        if not coordin: fail()
        unitsin = mycs.units()
        if not unitsin: fail()
        absout = n * [T]
        unitsout = mycs.units()
        if not unitsout: fail()
        #
        w = coordin['numeric'][sAxis]
        vIn = mycs.frequencytovelocity(value=w, doppler=dopplerin,
                                       velunit='km/s')
        if not vIn: fail()
        vOut = mycs.frequencytovelocity(value=w, doppler=dopplerout,
                                        velunit='km/s')
        if not vOut: fail()
        #
        coordin['numeric'][sAxis] = vIn
        unitsin[sAxis] = 'km/s'
        unitsout[sAxis] = 'km/s'
        #
        p = mycs.convert(list(coordin['numeric']), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        if not abs(p[sAxis]-vOut)<tol: fail('convert 12 gives wrong values')

        # absvel to relvel
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        coordin = mycs.toworld(p)['numeric']
        if len(coordin)!=n: fail()
        #
        w = coordin[sAxis]
        vIn = mycs.frequencytovelocity(value=w, doppler=dopplerin,
                                       velunit='km/s')
        if not vIn: fail()
        vOut = mycs.frequencytovelocity(value=w, doppler=dopplerout,
                                        velunit='km/s')
        if not vOut: fail()
        vOut -= vRefOut
        #
        coordin[sAxis] = vIn
        absin[sAxis] = T
        absout[sAxis] = F
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        d = abs(p[sAxis]-vOut)
        if not d<tol: fail('convert 13 gives wrong values')

        # absvel to absworld
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        coordin = mycs.toworld(p)['numeric']
        if len(coordin)!=n: fail()
        #
        w = coordin[sAxis]
        vIn = mycs.frequencytovelocity(value=w, doppler=dopplerin,
                                       velunit='km/s')
        if not vIn: fail()
        #
        coordin[sAxis] = vIn
        absin[sAxis] = T
        absout[sAxis] = T
        unitsout = mycs.units()
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        w = mycs.toworld(mycs.referencepixel()['numeric']+2)['numeric']
        if len(w)!=n: fail()
        d = abs(p[sAxis]-w[sAxis])
        if not d<tol: fail('convert 14 gives wrong values')

        # absvel to relworld
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        coordin = mycs.toworld(p)['numeric']
        if len(coordin)!=n: fail()
        #
        w = coordin[sAxis]
        vIn = mycs.frequencytovelocity(value=w, doppler=dopplerin,
                                       velunit='km/s')
        if not vIn: fail()
        #
        coordin[sAxis] = vIn
        absin[sAxis] = T
        absout[sAxis] = F
        unitsout = mycs.units()
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        w = mycs.torel(mycs.toworld(mycs.referencepixel()['numeric']+2),T)['numeric']
        if len(w)!=n: fail()
        d = abs(p[sAxis]-w[sAxis])
        if not d<tol: fail('convert 15 gives wrong values')

        # absvel to abspix
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        coordin = mycs.toworld(p)['numeric']
        if len(coordin)!=n: fail()
        #
        w = coordin[sAxis]
        vIn = mycs.frequencytovelocity(value=w, doppler=dopplerin,
                                    velunit='km/s')
        if not vIn: fail()
        #
        coordin[sAxis] = vIn
        absin[sAxis] = T
        absout[sAxis] = T
        unitsout[sAxis] = 'pix'
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2 = mycs.referencepixel()['numeric']+2
        if len(p2)!=n: fail()
        d = abs(p[sAxis]-p2[sAxis])
        if not d<tol: fail('convert 16 gives wrong values')

        # absvel to relpix
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        coordin = mycs.toworld(p)['numeric']
        if len(coordin)!=n: fail()
        #
        w = coordin[sAxis]
        vIn = mycs.frequencytovelocity(value=w, doppler=dopplerin,
                                       velunit='km/s')
        if not vIn: fail()
        #
        coordin[sAxis] = vIn
        absin[sAxis] = T
        absout[sAxis] = F
        unitsout[sAxis] = 'pix'
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        p2  = mycs.torel(mycs.referencepixel()['numeric']+2,F)['numeric']
        if len(p2)!=n: fail()
        d = abs(p[sAxis]-p2[sAxis])
        if not (d<tol): fail('convert 17 gives wrong values')

        # relvel to absvel
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        coordin = mycs.toworld(p)['numeric']
        if len(coordin)!=n: fail()
        #
        w = coordin[sAxis]
        vIn = mycs.frequencytovelocity(value=w, doppler=dopplerin,
                                       velunit='km/s')
        if not vIn: fail()
        vIn -= vRefIn
        vOut = mycs.frequencytovelocity(value=w, doppler=dopplerout,
                                        velunit='km/s')
        if not vOut: fail()
        #
        coordin[sAxis] = vIn
        absin[sAxis] = F
        absout[sAxis] = T
        unitsin[sAxis] = 'km/s'
        unitsout[sAxis] = 'km/s'
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        d = abs(p[sAxis]-vOut)
        if not (d<tol): fail('convert 18 gives wrong values')

        # absworld to absvel
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        coordin = mycs.toworld(p)['numeric']
        if len(coordin)!=n: fail()
        vOut = mycs.frequencytovelocity(value=coordin[sAxis],
                                        doppler=dopplerout,
                                        velunit='km/s')
        if not vOut: fail()
        #
        absin[sAxis] = T
        absout[sAxis] = T
        unitsin = mycs.units()
        unitsout[sAxis] = 'km/s'
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        d = abs(p[sAxis]-vOut)
        if not (d<tol): fail('convert 19 gives wrong values')

        # relworld to absvel
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        w = mycs.toworld(p)['numeric']
        if len(w)!=n: fail()
        coordin = mycs.torel(w,T)['numeric']
        if len(coordin)!=n: fail()
        vOut = mycs.frequencytovelocity(value=w[sAxis],
                                        doppler=dopplerout,
                                        velunit='km/s')
        if not vOut: fail()
        #
        absin[sAxis] = F
        absout[sAxis] = T
        unitsin = mycs.units()
        unitsout[sAxis] = 'km/s'
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        d = abs(p[sAxis]-vOut)
        if not (d<tol): fail('convert 20 gives wrong values')

        # abspix to absvel
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        w = mycs.toworld(p)['numeric']
        if len(w)!=n: fail()
        vOut = mycs.frequencytovelocity(value=w[sAxis],
                                        doppler=dopplerout,
                                        velunit='km/s')
        if not vOut: fail()
        #
        coordin = w
        coordin[sAxis] = p[sAxis]
        absin[sAxis] = T
        absout[sAxis] = T
        unitsin[sAxis] = 'pix'
        unitsout[sAxis] = 'km/s'
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        d = abs(p[sAxis]-vOut)
        if not (d<tol): fail('convert 21 gives wrong values')

        # relpix to absvel
        p = mycs.referencepixel()['numeric'] + 2
        if len(p)!=n: fail()
        w = mycs.toworld(p)['numeric']
        if len(w)!=n: fail()
        vOut = mycs.frequencytovelocity(value=w[sAxis],
                                        doppler=dopplerout,
                                        velunit='km/s')
        if not vOut: fail()
        #
        p = mycs.torel(p,F)['numeric']
        coordin[sAxis] = p[sAxis]
        absin[sAxis] = F
        absout[sAxis] = T
        unitsin[sAxis] = 'pix'
        unitsout[sAxis] = 'km/s'
        #
        p = mycs.convert(list(coordin), absin, dopplerin, unitsin,
                         absout, dopplerout, unitsout)
        if not p: fail()
        #
        d = abs(p[sAxis]-vOut)
        if not (d<tol): fail('convert 22 gives wrong values')
        mycs.done()

        # mixed
        mycs = cs.newcoordsys(direction=T, spectral=T, linear=1)
        if not mycs: fail()
        absPix = mycs.referencepixel()
        absPix['numeric'] += 4
        if len(absPix['numeric'])!=4: fail()
        relPix = mycs.torel(absPix, F)
        if len(relPix['numeric'])!=4: fail()
        absWorld = mycs.toworld(absPix)
        if not absWorld: fail()
        relWorld = mycs.torel(absWorld, T)
        if not relWorld: fail()
        n = mycs.naxes();

        # convertmany.  any test is as good as any other
        coordin = mycs.referencepixel()['numeric']
        if len(coordin)!=n: fail()
        absin = n * [T]
        unitsin = n * ['pix']
        dopplerin = 'radio'
        absout = n * [T]
        unitsout = mycs.units()
        if not unitsout: fail()
        dopplerout = 'radio'
        #
        coordout = mycs.convert(list(coordin), absin, dopplerin,
                                unitsin, absout, dopplerout, unitsout)
        if not coordout: fail()
        #
        rIn = ia.makearray(0, [len(coordin), 10])
        for i in range(10):
            for j in range(len(coordin)):
                rIn[j,i] = coordin[j]
        rOut = mycs.convertmany (rIn, absin, dopplerin, unitsin,
                                 absout, dopplerout, unitsout)
        if len(rOut)!=n: fail()
        for i in range(10):
            for j in range(len(coordin)):
                d = rOut[j,i] - coordout[j]
                if not (d<tol): fail('convertmany gives wrong values')
        #
        return T

    def test14():
        info('')
        info('')
        info('')
        info('Test 14 - setspectral')
        #
        info('')
        info('Testing setspectral')
        info('')
        #
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 1 failed')
        try:
            note("Expect SEVERE error and Exception here")
            ok = mycs.setspectral(refcode='lsrk')
        except Exception, e:
            note("Caught expected Exception")
            ok = false
        if ok:
            fail ('setspectral 1 unexpectedly did not fail')
        mycs.done()
        #
        mycs = cs.newcoordsys(spectral=T)
        #
        rc = 'LSRK'
        ok = mycs.setspectral(refcode=rc)
        if not ok: fail()
        if mycs.referencecode('spectral') != rc:
            fail('setspectral/reference code test fails')
        #
        rf = qa.quantity('1.0GHz')
        ok = mycs.setspectral(restfreq=rf)
        if not ok: fail()
        rf2 = mycs.restfrequency()
        if not rf2: fail()
        rf3 = qa.convert(rf2, 'GHz')
        if not rf3: fail()
        if (qa.getvalue(rf3) != 1.0):
            fail('setspectral/restfrequency test fails')
        #
        fd = [1, 1.5, 2, 2.5, 3]
        fq = qa.quantity(fd, 'GHz')
        ok = mycs.setspectral(frequencies=fq)
        if not ok: fail()
        #
        doppler = 'optical'
        vunit = 'km/s'
        vd = mycs.frequencytovelocity(fd, 'GHz', doppler, vunit)
        vq = qa.quantity(vd, vunit)
        ok = mycs.setspectral(velocities=vq, doppler=doppler)
        if not ok: fail()
        #
        fd2 = mycs.velocitytofrequency(vd, 'GHz', doppler, vunit)
        if not all(fd2,fd,1e-6):
            fail('setspectral/freq/vel consistency test failed')
        #
        if not mycs.done(): fail()

    def test15():
        info('')
        info('')
        info('')
        info('Test 15 - settabular')
        #
        info('')
        info('Testing settabular')
        info('')
        #
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 1 failed')
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.settabular(pixel=[1,2], world=[1,2])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            fail ('settabular 1 unexpectedley did not fail')
        mycs.done()
        #
        mycs = cs.newcoordsys(tabular=T)
        #
        p = [0, 1, 2, 3, 4]
        w = [10, 20, 30, 40, 50]
        ok = mycs.settabular(pixel=p, world=w)
        if not ok: fail()
        #
        rv = mycs.referencevalue()['numeric']
        if not rv: fail()
        if (rv[0] != w[0]):  fail('settabular test 1 failed (refval)')
        #
        rp = mycs.referencepixel()['numeric']
        if rp!=0.0: fail()
        if (rp != p[0]):  fail('settabular test 1 failed (refpix)')
        #
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.settabular(pixel=[0,1,2], world=[10,20])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('settabular test 2 unexpectedly did not fail')
        #
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.settabular(pixel=[0,1], world=[1,10,20])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('settabular test 3 unexpectedly did not fail')
        #
        ok = mycs.settabular(pixel=[0,1,2], world=[1,10,20])
        if not ok: fail('settabular test 4 failed')
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.settabular(pixel=[0,1,2,3])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('settabular test 5 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.settabular(world=[0,1,2,3])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok: fail('settabular test 6 unexpectedly did not fail')
        #
        if not mycs.done(): fail()

    def test16():
        info('')
        info('')
        info('')
        info('Test 16 - addcoordinate')
        #
        info('')
        info('Testing addcoordinate')
        info('')
        #
        mycs = cs.newcoordsys()
        if not mycs: fail('coordsys constructor 1 failed')
        #
        ok = mycs.addcoordinate(direction=T, spectral=T, linear=2, tabular=T, stokes="I V")
        if not ok:
            fail ('addcoordinate failed')
        #
        n = mycs.ncoordinates()
        if (n != 5):
            fail ('addcoordinate gave wrong number of coordinates')

        # We don't know what order they will be in. This is annoying.
        types = mycs.coordinatetype()
        hasDir = F
        hasSpec = F
        hasLin = F
        hasTab = F
        hasStokes = F
        for i in range(n):
            if (types[i]=='Direction'):
                hasDir = T
            else:
                if (types[i]=='Spectral'):
                    hasSpec = T
                else:
                    if (types[i]=='Linear'):
                        hasLin = T
                    else:
                        if (types[i]=='Tabular'):
                            hasTab = T
                        else:
                            if (types[i]=='Stokes'):
                                hasStokes = T
        #
        ok = hasDir and hasSpec and hasLin and hasTab and hasStokes
        if not ok:
            fail('addcoordinate did not add correct types')
        #
        mycs.done()

    def test17():
        info('')
        info('')
        info('')
        info('Test 17 - toworld, topixel with reference conversion')
        #
        mycs = cs.newcoordsys(direction=T, spectral=T)
        if not mycs: fail()
        #
        v = mycs.units()
        v[0] = 'rad'
        v[1] = 'rad'
        v[2] = 'Hz'
        ok = mycs.setunits(v)
        if not ok: fail()
        #
        mycs.setrestfrequency(1.420405752E9)
        #
        ok = mycs.setreferencecode(value='J2000', type='direction', adjust=F)
        if not ok: fail;
        ok = mycs.setreferencecode(value='LSRK', type='spectral', adjust=F)
        if not ok: fail;
        #
        v = mycs.referencevalue()
        v[0] = 0.0
        v[1] = -0.5
        v[2] = 1.4e9
        ok = mycs.setreferencevalue(v)
        if not ok: fail()
        #
        v = list(mycs.referencepixel()['numeric'])
        v[0] = 101
        v[1] = 121
        v[2] = 10.5
        ok = mycs.setreferencepixel(v)
        if not ok: fail()
        #
        v = mycs.increment()
        v[0] = -1.0e-6
        v[1] =  2.0e-6
        v[2] =  4.0e6
        ok = mycs.setincrement(v)
        if not ok: fail()
        #
        v = mycs.units()
        v[0] = 'deg'
        v[1] = 'deg'
        ok = mycs.setunits(v)
        if not ok: fail()

        ok = mycs.setconversiontype(direction='GALACTIC', spectral='BARY')
        if not ok: fail()
        #local d,s
        d = mycs.conversiontype(type='direction')
        if not d: fail()
        s = mycs.conversiontype(type='spectral')
        if not s: fail()
        if (d != 'GALACTIC' or s != 'BARY'):
            fail('setconversiontype consistency test failed')
        #
        p = mycs.referencepixel()['numeric']
        for i in range(len(p)): p[i] += 10.0
        if len(p)!=3: fail()
        w = mycs.toworld(value=p, format='n')
        if not w: fail()
        p2 = mycs.topixel(value=w)['numeric']
        if len(p2)!=3: fail()
        #
        # Need to look into why i need such a large tolerance
        #
        tol = 1e-3
        if not all(p2,p,tol): fail('failed consistency test 1')
        ###
        return T

    def test18():
        info('')
        info('')
        info('')
        info('Test 18 - setdirection')
        #
        info('')
        info('Testing setdirection')
        info('')
        #
        mycs = cs.newcoordsys(direction=T)
        if not mycs: fail('coordsys constructor 1 failed')

        # Test 1
        refcode = 'GALACTIC'
        proj = 'CAR'
        projpar = []
        refpix = list(mycs.referencepixel()['numeric'])
        for i in range(len(refpix)): refpix[i] *= 1.1
        refval = mycs.referencevalue(format='n')['numeric']
        for i in range(len(refval)): refval[i] *= 1.1
        xform = ia.makearray(0.0, [2, 2]);
        xform[0,0] = 1.0
        xform[1,1] = 1.0
        ok = mycs.setdirection(refcode=refcode,
                               proj=proj, projpar=projpar,
                               refpix=refpix, refval=refval,
                               xform=xform)
        if not ok: fail()
        #
        if (proj != mycs.projection()['type']):
            fail('Projection was not updated')
        if (len(projpar) > 0):
            if (projpar != mycs.projection()['parameters']):
                fail('Projection parameters were not updated')
        if not all(refpix, mycs.referencepixel()['numeric'], 1e-6):
            fail('Reference pixel was not updated')
        if not all(refval, mycs.referencevalue(format='n')['numeric'], 1e-6):
            fail('Reference value was not updated')

        # Test 2
        refcode = 'J2000'
        proj = 'SIN'
        projpar = [0,0]
        refval = "20.0deg -33deg"
        refval2 = [20,-33]
        ok = mycs.setdirection(refcode=refcode,
                               proj=proj, projpar=projpar,
                               refval=refval)
        if not ok: fail()
        #
        if (proj != mycs.projection()['type']):
            fail('Projection was not updated')
        if (len(projpar) > 0):
            if not all(projpar,mycs.projection()['parameters'],1e-6):
                fail('Projection parameters were not updated')
        ok = mycs.setunits (value="deg deg")
        if not all (refval2, mycs.referencevalue(format='n')['numeric'], 1e-6):
            fail('Reference value was not updated')
        #
        if not mycs.done(): fail()


    def test19():
        info('')
        info('')
        info('')
        info('Test 19 - replace')
        #
        info('')
        info('Testing replace')
        info('')
        #
        mycs = cs.newcoordsys(direction=T, linear=1)
        if not mycs: fail('coordsys constructor 1 failed')
        #
        cs2 = cs.newcoordsys(linear=1)
        if not cs2: fail('coordsys constructor 2 failed')
        #
        try:
            note('Expect SEVERE error and Exception here')
            ok = mycs.replace(cs2.torecord(), whichin=0, whichout=0)
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            fail('replace 1 unexpectedly did not fail')
        ok = cs2.done()
        #
        cs2 = cs.newcoordsys(spectral=T)
        ok = mycs.replace(cs2.torecord(), whichin=0, whichout=1)
        if not ok: fail()
        if mycs.coordinatetype(1) != 'Spectral':
            fail('Replace 1 did not set correct coordinate type')
        #
        ok = cs2.done()
        ok = mycs.done()


    test1()
    test2()
    test3()
    test4()
    test5()
    test6()
    test7()
    test8()
    test9()
    test10()
    test11()
    test12() #check toabsmany/torelmany
    test13()
    test14()
    test15()
    test16()
    test17()
    test18()
    test19()
    

Benchmarking = True
if Benchmarking:
    startTime = time.time()
    regstate = False
    for i in range(100):
        coordsystest()
    endTime = time.time()
    regstate = True
else:
    coordsystest()

#exit()
