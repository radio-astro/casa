# regridspw_test.py:  A test suite for testing the ms.regridspw() method
#
# Copyright (C) 2007
# Associated Universities, Inc. Washington DC, USA.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be addressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#   
# @author Dirk Petry
# @version 
#############################################################################
# regridspw unit test
import os;
import sys;
import math;

dochannelmode = true
dooutframes = true
dofreqmode = true
dovradmode = true
dovoptmode = true
dowavemode = true
dowildstuff = true

cenchanfreq = 0

def verifycolumn(colname,explength):
    a = tb.getcell(colname,0)
    datalength = len(a[0])
    if(datalength != explength):
        print "Unexpected length of array in column " + colname
        print "   expected ", explength, ", found ", datalength
        return 1
    print colname + ' OK'
    return 0

def verifycolumnb(colname,explength):
    a = tb.getcell(colname,0)
    datalength = len(a)
    if(datalength != explength):
        print "Unexpected length of array in column " + colname
        print "   expected ", explength, ", found ", datalength
        return 1
    print colname + ' OK'
    return 0

def verifySPWcolumn(colname,expval):
    try:
        tb.open('test.ms/SPECTRAL_WINDOW')
        a = math.floor(tb.getcell(colname,0))
        expval = math.floor(expval)
        tb.close()
        if(a != expval):
            print "Unexpected value of cell in column " + colname
            print "   expected ", expval, ", found ", a
            return 1
        print colname + ' OK'
        return 0
    except:
        tb.close()
        print "Error verifying ", colname, " in SPECTRAL_WINDOW"
        return 1

def initms(msname, nms, cenchan):
    origms = ['ngc4826.tutorial.ngc4826.ll.5.ms','mytest.ms']
    os.system('rm -rf '+msname+'; cp -R ' + origms[nms] + ' ' + msname)

    tb.open(msname+'/SPECTRAL_WINDOW')
    global cenchanfreq
    a = tb.getcell('CHAN_FREQ',0)
    cenchanfreq = math.floor(a[cenchan])
    tb.close()

    ms.open(msname, nomodify=false)
    print "Using ", origms[nms]
    return

def verifytabs(numchan):
    try:
        tb.open('test.ms')
        numerr = 0
        numerr += verifycolumn('DATA',numchan)
        numerr += verifycolumn('FLAG',numchan)
        tb.close()
    except:
        print "Error verifying MAIN"
        tb.close()
        numerr +=1
    try:
        tb.open('test.ms/SPECTRAL_WINDOW')
        numerr += verifycolumnb('CHAN_WIDTH',numchan)
        numerr += verifycolumnb('CHAN_FREQ',numchan)
        tb.close()
    except:
        print "Error verifying SPECTRAL_WINDOW"
        tb.close()
        numerr+=1
    return numerr

def testcenchanfreq(newcenchan):
    global cenchanfreq
    tb.open('test.ms/SPECTRAL_WINDOW')
    cf = math.floor((tb.getcell('CHAN_FREQ',0))[newcenchan])
    tb.close()
    if(cenchanfreq != cf):
        print "Center channel should have frequency ",  cenchanfreq, " but has ", cf
        return 1
    print "Central channel frequency OK ", cf, " Hz"
    return 0


if(not os.path.exists('ngc4826.tutorial.ngc4826.ll.5.ms')):
    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis='ngc4826.tutorial.ngc4826.ll.5.ms')
    
numerrors = 0

if dochannelmode:
    # test channel mode
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", chanwidth=2)'
    ms.regridspw(mode="chan", chanwidth=2)
    ms.close()
    numerrors += verifytabs(32)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",64*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=10, chanwidth=2)'
    ms.regridspw(mode="chan", center=10, bandwidth=10, chanwidth=2)
    ms.close()
    numerrors += verifytabs(5)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",10.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", start=5, bandwidth=10, chanwidth=2)'
    ms.regridspw(mode="chan", start=5, bandwidth=10, chanwidth=2)
    ms.close()
    numerrors += verifytabs(5)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",10.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=9, chanwidth=3)'
    ms.regridspw(mode="chan", center=10, bandwidth=9, chanwidth=3)
    ms.close()
    numerrors += verifytabs(3)
    numerrors += testcenchanfreq(1)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",9.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=10, chanwidth=4)'
    ms.regridspw(mode="chan", center=10, bandwidth=10, chanwidth=4)
    ms.close()
    numerrors += verifytabs(3)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",10.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=10, chanwidth=5)'
    ms.regridspw(mode="chan", center=10, bandwidth=10, chanwidth=5)
    ms.close()
    numerrors += verifytabs(3)
    numerrors += testcenchanfreq(1)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",10.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=9)'
    ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=9)
    ms.close()
    numerrors += verifytabs(3)
    numerrors += testcenchanfreq(1)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",11.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=11)'
    ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=11)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",11.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=61)'
    ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=61)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",11.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=64)'
    ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=64)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",11.*1562500.)
    initms('test.ms',0, 10)
    print 'ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=65)'
    ms.regridspw(mode="chan", center=10, bandwidth=11, chanwidth=65)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",11.*1562500.)

if dooutframes:
    # test different outframes
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="chan", center=15, bandwidth=21, chanwidth=3)'
    ms.regridspw(outframe="LSRK", mode="chan", center=15, bandwidth=21, chanwidth=3)
    ms.close()
    numerrors += verifytabs(7)
    numerrors += testcenchanfreq(3)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="BARY",mode="chan", center=15, bandwidth=21, chanwidth=3)'
    ms.regridspw(outframe="BARY", mode="chan", center=15, bandwidth=21, chanwidth=3)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114976457450.0
    numerrors += testcenchanfreq(3)
    # try going back to LSRK
    os.system('rm -rf mytest.ms; cp -R test.ms mytest.ms')
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="chan")'
    ms.regridspw(outframe="LSRK", mode="chan")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK")'
    ms.regridspw(outframe="LSRK")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="wave")'
    ms.regridspw(outframe="LSRK", mode="wave")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="GALACTO",mode="chan", center=15, bandwidth=21, chanwidth=3)'
    ms.regridspw(outframe="GALACTO", mode="chan", center=15, bandwidth=21, chanwidth=3)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114979582198.0
    numerrors += testcenchanfreq(3)
    # try going back to LSRK
    os.system('rm -rf mytest.ms; cp -R test.ms mytest.ms')
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="chan")'
    ms.regridspw(outframe="LSRK", mode="chan")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK")'
    ms.regridspw(outframe="LSRK")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="wave")'
    ms.regridspw(outframe="LSRK", mode="wave")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)

    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LGROUP",mode="chan", center=15, bandwidth=21, chanwidth=3)'
    ms.regridspw(outframe="LGROUP", mode="chan", center=15, bandwidth=21, chanwidth=3)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 115000581271.0
    numerrors += testcenchanfreq(3)
    # try going back to LSRK
    os.system('rm -rf mytest.ms; cp -R test.ms mytest.ms')
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="chan")'
    ms.regridspw(outframe="LSRK", mode="chan")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK")'
    ms.regridspw(outframe="LSRK")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="wave")'
    ms.regridspw(outframe="LSRK", mode="wave")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)

    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="CMB",mode="chan", center=15, bandwidth=21, chanwidth=3)'
    ms.regridspw(outframe='CMB', mode="chan", center=15, bandwidth=21, chanwidth=3)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114865327257.0
    numerrors += testcenchanfreq(3)
    # try going back to LSRK
    os.system('rm -rf mytest.ms; cp -R test.ms mytest.ms')
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="chan")'
    ms.regridspw(outframe="LSRK", mode="chan")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK")'
    ms.regridspw(outframe="LSRK")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="wave")'
    ms.regridspw(outframe="LSRK", mode="wave")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRD",mode="chan", center=15, bandwidth=21, chanwidth=3)'
    ms.regridspw(outframe="LSRD", mode="chan", center=15, bandwidth=21, chanwidth=3)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973857743.0
    numerrors += testcenchanfreq(3)
    # try going back to LSRK
    os.system('rm -rf mytest.ms; cp -R test.ms mytest.ms')
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="chan")'
    ms.regridspw(outframe="LSRK", mode="chan")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK")'
    ms.regridspw(outframe="LSRK")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    print 'ms.regridspw(outframe="LSRK", mode="wave")'
    ms.regridspw(outframe="LSRK", mode="wave")
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    initms('test.ms', 1, 3)
    
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="GEO",mode="chan", center=15, bandwidth=21, chanwidth=3)'
    ms.regridspw(outframe="GEO", mode="chan", center=15, bandwidth=21, chanwidth=3)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114972784021.0
    numerrors += testcenchanfreq(3)
    initms('test.ms',0, 15)
    # Note: return to LSRK does not work

    print 'ms.regridspw(outframe="TOPO",mode="chan", center=15, bandwidth=21, chanwidth=3)'
    ms.regridspw(outframe="TOPO", mode="chan", center=15, bandwidth=21, chanwidth=3)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114972755932.0
    numerrors += testcenchanfreq(3)
    # Note: return to LSRK does not work


if dofreqmode:
    # test freq mode
    initms('test.ms',0, 15)
    print 'ms.regridspw(mode="freq", chanwidth=2*1562500.)'
    ms.regridspw(mode="freq", chanwidth=2*1562500.)
    ms.close()
    numerrors += verifytabs(32)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",64*1562500.)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114973628113.0, bandwidth=21.*1562500., chanwidth=3.*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114973628113.0, bandwidth=21.*1562500., chanwidth=3.*1562500.)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",21.*1562500.)    
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=10*1562500., chanwidth=2*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=10*1562500., chanwidth=2*1562500.)
    ms.close()
    numerrors += verifytabs(5)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",10.*1562500.)    
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=9*1562500., chanwidth=3*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=9*1562500., chanwidth=3*1562500.)
    ms.close()
    numerrors += verifytabs(3)
    numerrors += testcenchanfreq(1)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",9.*1562500.)    
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=10*1562500., chanwidth=4*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=10*1562500., chanwidth=4*1562500.)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=10*1562500., chanwidth=5*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=10*1562500., chanwidth=5*1562500.)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=9*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=9*1562500.)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=11*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=11*1562500.)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=61*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=61*1562500.)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=64*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=64*1562500.)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 10)
    print 'ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=65*1562500.)'
    ms.regridspw(outframe="LSRK",mode="freq", center=114965815613.0, bandwidth=11*1562500., chanwidth=65*1562500.)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)

if dovradmode:
    # test vrad mode
    spofli = 299792458.
    rest = 114973628113.0
    cw = abs(spofli * (1. - (rest+1562500.)/rest))
    print "Choose rest freq rest = ", rest
    print "Original Channel width cw =", cw

    initms('test.ms',0, 15)
    print 'ms.regridspw(mode="vrad", chanwidth=2*cw, restfreq=rest)'
    ms.regridspw(mode="vrad", chanwidth=2*cw, restfreq=rest)
    ms.close()
    numerrors += verifytabs(32)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",64*1562500.)
    
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=21.*cw, chanwidth=3.*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=21.*cw, chanwidth=3.*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",21.*1562500.)

    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=10*cw, chanwidth=2*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=10*cw, chanwidth=2*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(5)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",10.*1562500.)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=9*cw, chanwidth=3*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=9*cw, chanwidth=3*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(3)
    numerrors += testcenchanfreq(1)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",9.*1562500.)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=10*cw, chanwidth=4*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=10*cw, chanwidth=4*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=10*cw, chanwidth=5*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=10*cw, chanwidth=5*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=9*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=9*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=11*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=11*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=61*cw,restfreq=rest)' 
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=61*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=64*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=64*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=65*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vrad", center=0., bandwidth=11*cw, chanwidth=65*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)

if dovoptmode:
    # test vopt mode
    spofli = 299792458.
    rest = 114973628113.0
    cw = abs(spofli * (rest /(rest+1562500.) - 1.))
    print "Choose rest freq rest = ", rest
    print "Original Channel width cw =", cw

    initms('test.ms',0, 15)
    print 'ms.regridspw(mode="vopt", chanwidth=2*cw, restfreq=rest)'
    ms.regridspw(mode="vopt", chanwidth=2*cw, restfreq=rest)
    ms.close()
    numerrors += verifytabs(32)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",99999997.0)
    
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=21.*cw, chanwidth=3.*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=21.*cw, chanwidth=3.*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628113.0
    numerrors += testcenchanfreq(3)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",32812150.0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=10*cw, chanwidth=2*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=10*cw, chanwidth=2*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(5)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=9*cw, chanwidth=3*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=9*cw, chanwidth=3*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(3)
    numerrors += testcenchanfreq(1)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=10*cw, chanwidth=4*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=10*cw, chanwidth=4*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=10*cw, chanwidth=5*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=10*cw, chanwidth=5*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=9*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=9*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=11*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=11*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=61*cw,restfreq=rest)' 
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=61*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=64*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=64*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=65*cw,restfreq=rest)'
    ms.regridspw(outframe="LSRK",mode="vopt", center=0., bandwidth=11*cw, chanwidth=65*cw,restfreq=rest)
    ms.close()
    numerrors += verifytabs(1)
    numerrors += testcenchanfreq(0)

if dowavemode:
    # test wave mode
    spofli = 299792458.
    regcentf = 114973628113.0
    regcentl = spofli/regcentf
    cw = regcentl - spofli/(regcentf+1562500.)
    print "Original Channel width cw =", cw

    initms('test.ms',0, 15)
    print 'ms.regridspw(mode="wave", chanwidth=2*cw)'
    ms.regridspw(mode="wave", chanwidth=2*cw)
    ms.close()
    numerrors += verifytabs(32)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",99999997.0)
    
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=21.*cw, chanwidth=3.*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=21.*cw, chanwidth=3.*cw)
    ms.close()
    numerrors += verifytabs(7)
    cenchanfreq = 114973628112.0
    numerrors += testcenchanfreq(3)
    numerrors += verifySPWcolumn("TOTAL_BANDWIDTH",32812150.0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=10*cw, chanwidth=2*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=10*cw, chanwidth=2*cw)
    ms.close()
    numerrors += verifytabs(5)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=9*cw, chanwidth=3*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=9*cw, chanwidth=3*cw)
    ms.close()
    numerrors += verifytabs(3)
    cenchanfreq = 114973628112.0
    numerrors += testcenchanfreq(1)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=10*cw, chanwidth=4*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=10*cw, chanwidth=4*cw)
    ms.close()
    numerrors += verifytabs(1)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=10*cw, chanwidth=5*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=10*cw, chanwidth=5*cw)
    ms.close()
    numerrors += verifytabs(1)
    cenchanfreq = 114973628112.0
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=9*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=9*cw)
    ms.close()
    numerrors += verifytabs(1)
    cenchanfreq = 114973628112.0
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=11*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=11*cw)
    ms.close()
    numerrors += verifytabs(1)
    cenchanfreq = 114973628112.0
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=61*cw)' 
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=61*cw)
    ms.close()
    numerrors += verifytabs(1)
    cenchanfreq = 114973628112.0
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=64*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=64*cw)
    ms.close()
    numerrors += verifytabs(1)
    cenchanfreq = 114973628112.0
    numerrors += testcenchanfreq(0)
    initms('test.ms',0, 15)
    print 'ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=65*cw)'
    ms.regridspw(outframe="LSRK",mode="wave", center=regcentl, bandwidth=11*cw, chanwidth=65*cw)
    ms.close()
    numerrors += verifytabs(1)
    cenchanfreq = 114973628112.0
    numerrors += testcenchanfreq(0)

if dowildstuff:
    # try to break regridspw
    try:
        initms('test.ms',0, 0)
        print 'ms.regridspw()'
        ms.regridspw()
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1
    try:
        initms('test.ms',0, 0)
        print 'ms.regridspw(outframe="VARY")'
        ms.regridspw(outframe='VARY')
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1
    try:
        initms('test.ms',0, 0)
        print 'ms.regridspw(mode="xxx")'
        ms.regridspw(mode='xxx')
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1
    try:
        initms('test.ms',0, 0)
        print 'ms.regridspw(outframe="bary", mode="freq", chanwidth=200000., interpolation="guess")'
        ms.regridspw(outframe="bary", mode="freq", chanwidth=200000., interpolation="guess")
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1
    try:
        initms('test.ms',0, 0)
        print 'ms.regridspw(outframe="bary", mode="wave", center=1)'
        ms.regridspw(outframe='bary', mode="wave", center=1)
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1
    try:
        initms('test.ms',0, 0)
        print 'ms.regridspw(outframe="bary", mode="vopt", bandwidth=1E16)'
        ms.regridspw(outframe='bary', mode="vopt", bandwidth=1E16)
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1
    try:
        initms('test.ms',0, 0)
        print 'ms.regridspw(outframe="bary", mode="vrad", restfreq=1E9, chanwidth=1E9)'
        ms.regridspw(outframe='bary', mode="vrad", restfreq=1E9, chanwidth=1E9)
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1
    try:
        initms('test.ms',0, 10)
        print 'ms.regridspw(mode="chan", center=10, start=2, bandwidth=10, chanwidth=2)'
        ms.regridspw(mode="chan", center=10, start=2, bandwidth=10, chanwidth=2)
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1
    try:
        initms('test.ms',0, 10)
        print 'ms.regridspw(mode="freq", center=1E9, start=1E9, bandwidth=5E9)'
        ms.regridspw(mode="freq", center=1E9, start=1E9, bandwidth=5E9)
        ms.close()
    except:
        ms.close()
        print "Bad input not properly intercepted."
        numerrors += 1

print "-- end regridspw test ---------------------"
print numerrors, " errors total."
    
