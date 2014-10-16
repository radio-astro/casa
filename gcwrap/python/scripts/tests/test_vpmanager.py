import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for the vpmanager tool. Tested methods:
        reset()
        summarizevps()
        getvp()
        numvps()
        setpbairy()
        setpbantresptable()
        createantresp()
        getrespimagename()
        setuserdefault()
        getuserdefault()
        saveastable()
        loadfromtable()
'''
class vpmanager_test(unittest.TestCase):
    
    # Input and output names
    res = None
    inputdir = 'mydir3'

    def setUp(self):
        self.res = None
    
    def tearDown(self):
        os.system('rm -rf ' + self.inputdir)

    def test0(self):
        '''Test 0: reset'''
        self.res = vp.reset()
        self.assertTrue(self.res)

    def test1(self):
        '''Test 1: summarizevps'''
        self.res = vp.summarizevps()
        self.assertTrue(self.res)

    def test2(self):
        '''Test 2: getvp for VLA'''
        vp.reset()
        myrec = vp.getvp(telescope='VLA',
                         obstime = '1999/07/24/10:00:00',
                         freq = 'TOPO 30GHz',
                         antennatype = '',
                         obsdirection = 'AZEL 30deg 60deg')
        
        self.assertTrue(myrec['commonpb']=='VLA')

    def test3(self):
        '''Test 3: getvp for ALMA'''
        vp.reset()
        vp.setpbantresptable(telescope='ALMA',
                             antresppath=casa['dirs']['data']+'/alma/responses/AntennaResponses-ALMA',
                             dopb=True)
        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'DV',
                         obsdirection = 'AZEL 30deg 60deg')
        
        self.assertTrue(myrec['name']=='IMAGE' and myrec['telescope']=='ALMA')

    def test4(self):
        '''Test 4: numvps for VLA'''
        vp.reset()
        myrval = vp.numvps(telescope='VLA',
                           obstime = '1999/07/24/10:00:00',
                           freq = 'TOPO 30GHz',
                           obsdirection = 'AZEL 30deg 60deg')
        
        self.assertTrue(myrval==1)

    def test5(self):
        '''Test 5: numvps for ALMA'''
        vp.reset()
        vp.setpbantresptable(telescope='ALMA',
                             antresppath=casa['dirs']['data']+'/alma/responses/AntennaResponses-ALMA',
                             dopb=True)
        myrval = vp.numvps(telescope='ALMA',
                           obstime = '2009/07/24/10:00:00',
                           freq = 'TOPO 100GHz',
                           obsdirection = 'AZEL 30deg 60deg')
        
        self.assertTrue(myrval==3)

    def test6(self):
        '''Test 6: numvps for ALMA with too high freq'''
        vp.reset()
        vp.setpbantresptable(telescope='ALMA',
                             antresppath=casa['dirs']['data']+'/alma/responses/AntennaResponses-ALMA',
                             dopb=True)
        myrval = vp.numvps(telescope='ALMA',
                           obstime = '2009/07/24/10:00:00',
                           freq = 'TOPO 1000GHz', # freq too high
                           obsdirection = 'AZEL 30deg 60deg')
        
        self.assertTrue(myrval==0)

    def test7(self):
        '''Test 7: setuserdefault for ALMA'''
        vp.reset()
        self.res = vp.setuserdefault(vplistnum=-1, telescope='ALMA')
        self.assertTrue(self.res)


    def test8(self):
        '''Test 8: define Airy beam for ALMA, then use it'''        
        vp.reset()
        vp.setpbairy(telescope='ALMA',
                     dishdiam=str(12./1.18)+'m',
                     blockagediam='0.75m',
                     maxrad='1.784deg',
                     reffreq='1.0GHz',
                     dopb=True)
        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = '',
                         obsdirection = 'AZEL 30deg 60deg')
        
        woanttypeok = (myrec['name']=='AIRY' and myrec['telescope']=='ALMA')

        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'DV', # should not matter since AIRY entry global
                         obsdirection = 'AZEL 30deg 60deg')
        
        withanttypeok = (myrec['name']=='AIRY' and myrec['telescope']=='ALMA')

        self.assertTrue(woanttypeok and withanttypeok)

    def test9(self):
        '''Test 9: define reference to antresp table for ALMA, then use it'''
        
        vp.reset()
        os.system('rm -rf BeamCalcTmpImage_*')
        vp.setpbantresptable(telescope='ALMA',
                             antresppath=casa['dirs']['data']+'/alma/responses/AntennaResponses-ALMA-RT',
                             dopb=True)
        myrec1 = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'DV',
                         obsdirection = 'AZEL 30deg 60deg')
        myrec2 = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'DA',
                         obsdirection = 'AZEL 30deg 60deg')
        myrec3 = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'PM',
                         obsdirection = 'AZEL 30deg 60deg')
        myrec4 = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'CM',
                         obsdirection = 'AZEL 30deg 60deg')
        
        self.assertTrue(myrec1['name']=='IMAGE' and myrec1['telescope']=='ALMA')
        self.assertTrue(myrec2['name']=='IMAGE' and myrec2['telescope']=='ALMA')
        self.assertTrue(myrec3['name']=='IMAGE' and myrec3['telescope']=='ALMA')
        self.assertTrue(myrec4['name']=='IMAGE' and myrec4['telescope']=='ALMA')


    def test10(self):
        '''Test 10: define Airy beams for ALMA antenna types, then use them'''
        
        vp.reset()
        vp.setpbairy(telescope='ALMA',
                     dishdiam='11m',
                     blockagediam='0.75m',
                     maxrad='1.784deg',
                     reffreq='1.0GHz',
                     dopb=True)
        myid1 = vp.getuserdefault('ALMA')
        
        vp.setpbairy(telescope='ALMA',
                     dishdiam='12m',
                     blockagediam='1.0m',
                     maxrad='1.784deg',
                     reffreq='1.0GHz',
                     dopb=True)

        myid2 = vp.getuserdefault('ALMA')

        vp.setpbairy(telescope='ALMA',
                     dishdiam='6m',
                     blockagediam='0.75m',
                     maxrad='3.5deg',
                     reffreq='1.0GHz',
                     dopb=True)

        myid3 = vp.getuserdefault('ALMA')


        vp.setuserdefault(myid1, 'ALMA', 'DV')
        vp.setuserdefault(myid1, 'ALMA', 'DA')
        vp.setuserdefault(myid2, 'ALMA', 'PM')
        vp.setuserdefault(myid3, 'ALMA', 'CM')
        
        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'DV',
                         obsdirection = 'AZEL 30deg 60deg')
        tdvok = (myrec['name']=='AIRY' and myrec['dishdiam']['value']==11)
        
        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'DA',
                         obsdirection = 'AZEL 30deg 60deg')
        tdaok = (myrec['name']=='AIRY' and myrec['dishdiam']['value']==11)
        
        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'PM',
                         obsdirection = 'AZEL 30deg 60deg')
        tpmok = (myrec['name']=='AIRY' and myrec['dishdiam']['value']==12)
        
        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'CM',
                         obsdirection = 'AZEL 30deg 60deg')
        tcmok = (myrec['name']=='AIRY' and myrec['dishdiam']['value']==6)

        myanttypes = vp.getanttypes('ALMA')

        tantok = (('' in myanttypes)
                  and ('CM' in myanttypes)
                  and ('DA' in myanttypes)
                  and ('DV' in myanttypes)
                  and ('PM' in myanttypes))

        vp.setuserdefault(-2, 'ALMA', '')

        myanttypes = vp.getanttypes('ALMA')

        tantok2 = ((not('' in myanttypes))
                  and ('CM' in myanttypes)
                  and ('DA' in myanttypes)
                  and ('DV' in myanttypes)
                  and ('PM' in myanttypes))

        self.assertTrue(tdvok and tdaok and tpmok and tcmok and tantok and tantok2)


    def test11(self):
        '''Test 11: getvp without observation parameters'''

        vp.reset()

        vp.setcannedpb(telescope="ALMA")

        myrec = vp.getvp('ALMA')
        
        woanttypeok = (myrec['name']=='COMMONPB' and myrec['telescope']=='ALMA')

        myrec = vp.getvp('ALMA', 'DV')
        
        withanttypeok = (myrec['name']=='COMMONPB' and myrec['telescope']=='ALMA')

        self.assertTrue(woanttypeok and withanttypeok)
        
        
    def test12(self):
        """Test 12: EXPECTED ERROR createantresp - no images"""
        os.system('mkdir '+self.inputdir)
        self.res = vp.createantresp(self.inputdir, "2011-02-02-12:00", ["band1","band2","band3"], ["83GHz","110GHz","230GHz"], ["110GHz","230GHz","350GHz"])
        self.assertFalse(self.res)

    def test13(self):
        '''Test 13: EXPECTED ERROR createantresp - two images have faulty band def'''
        os.system('mkdir '+self.inputdir)
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._80._100._110._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._110._200._230._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._230._300._350._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._80._100._110._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._110._200._230._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._230._300._350._GHz_ticra2007_EFP.im')
        self.res = vp.createantresp(self.inputdir, "2011-02-02-12:00", ["band1","band2","band3"], ["83GHz","110GHz","230GHz"], ["110GHz","230GHz","350GHz"])
        self.assertFalse(self.res)
        
    def test14(self):
        '''Test 14: createantresp - good input: six images, two antenna types'''
        os.system('mkdir '+self.inputdir)
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._80._100._110._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._110._200._230._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_DV__0._0._360._0._45._90._230._300._350._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._80._100._110._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._110._200._230._GHz_ticra2007_EFP.im')
        os.system('touch '+self.inputdir+'/ALMA_0_ME__0._0._360._0._45._90._230._300._350._GHz_ticra2007_EFP.im')
        self.res = vp.createantresp(self.inputdir, "2011-02-02-12:00", ["band1","band2","band3"], ["80GHz","110GHz","230GHz"], ["110GHz","230GHz","350GHz"])
        self.assertTrue(self.res)
        tb.open(self.inputdir+'/AntennaResponses')
        self.assertTrue(tb.nrows()==2)
        self.assertTrue(tb.ncols()==19)
        self.assertTrue(tb.getcell('NUM_SUBBANDS',0)==3)
        self.assertTrue(tb.getcell('NUM_SUBBANDS',1)==3)
        tb.close()

    def test15(self):
        '''Test 15: EXPECTED ERROR get image name from non-existant observatory'''
        self.res = vp.getrespimagename("ALMA2","2011/01/01/10:00","100GHz","AIF","DV","0deg","0deg","",0)
        self.assertFalse(self.res)

    def test16(self):
        '''Test 16: define Airy beam for ALMA, save it, load it, then use it'''        
        vp.reset()
        vp.setpbairy(telescope='ALMA',
                     dishdiam=str(12./1.18)+'m',
                     blockagediam='0.75m',
                     maxrad='1.784deg',
                     reffreq='1.0GHz',
                     dopb=True)

        shutil.rmtree('mydefs.tab', ignore_errors=True)
        vp.saveastable('mydefs.tab')
        vp.reset()
        vp.loadfromtable('mydefs.tab')
        
        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = '',
                         obsdirection = 'AZEL 30deg 60deg')
        
        woanttypeok = (myrec['name']=='AIRY' and myrec['telescope']=='ALMA')

        myrec = vp.getvp(telescope='ALMA',
                         obstime = '2009/07/24/10:00:00',
                         freq = 'TOPO 100GHz',
                         antennatype = 'DV', # should not matter since AIRY entry global
                         obsdirection = 'AZEL 30deg 60deg')
        
        withanttypeok = (myrec['name']=='AIRY' and myrec['telescope']=='ALMA')

        self.assertTrue(woanttypeok and withanttypeok)
    
##     def test17(self):
##         '''Test 17: get image name (fails if no resp. table path set in Observatories table))'''
##         vp.reset()
##         vp.setpbantresptable(telescope='ALMA',
##                              antresppath=casa['dirs']['data']+'/alma/responses/AntennaResponses-ALMA',
##                              dopb=True)
##         self.res = vp.getrespimagename("ALMA","2011/01/01/10:00","100GHz","INTERNAL","CM","0deg","0deg","",0)
##         self.assertTrue(self.res)


def suite():
    return [vpmanager_test]


