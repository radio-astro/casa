import shutil
import unittest
import os
import filecmp
import numpy
from tasks import *
from taskinit import *
from __main__ import default
"""
Unit tests for task setjy.

Features tested:
  1. Does setjy(modimage=modelimu, fluxdensity=0) NOT scale the model image's
     flux density?
  2. Does setjy(modimage=modelimu) scale the model image's flux density?
  3. Solar system (Uranus) flux density calibration.
"""

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/'

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/setjy/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR

print 'setjy tests will use data from '+datapath

if os.environ.has_key('BYPASS_PARALLEL_PROCESSING'):
    ParallelTaskHelper.bypassParallelProcessing(1)


class SetjyUnitTestBase(unittest.TestCase):

    def setUpMS(self,MS):

        self.inpms = MS

        # Create working area
        setjydatapath = 'unittest/setjy/'
        # 2015-02-02 TT: this seems unnessary directory layer...
        #if not os.path.exists(setjydatapath):
        #    print "\nCreate working area..."
        #    os.system('mkdir -p '+setjydatapath)

        # Create a new fresh copy of the MS
        print "\nCreate a new local copy of the MS..."
        #print " setjydatapath=",setjydatapath, " inpms=",self.inpms
        if testmms:
            os.system('cp -rH ' + datapath + self.inpms + ' '+self.inpms)
        else:
            os.system('cp -rf ' + os.environ.get('CASAPATH').split()[0] + "/data/regression/" + setjydatapath + self.inpms + ' ' + self.inpms)

    def resetMS(self):

        if os.path.exists(self.inpms):
            print "\nRemoving a local copy of MS from the previous test..."
            #ret = os.system('rm -rf unittest/setjy/*')
            shutil.rmtree(self.inpms)

    def get_last_history_line(self,vis, origin='setjy::imager::setjy()',
                              nback=0, maxnback=20, hint=''):
        """
        Finding the right history line is a bit tricky...it helps to filter
        by origin and read from the back to remain unaffected by changes
        elsewhere.
        
        This reads up to maxnback lines with origin origin until it finds one
        including hint in the message, going backwards from nback lines from the
        end.
        
        Returns 'JUNK' on failure.
        """
        retline = 'JUNK'
        try:
            tblocal = tbtool()
            tblocal.open(vis + '/HISTORY')
            st = tblocal.query('ORIGIN == pattern("%s*")' % origin, columns='MESSAGE')
            nstrows = st.nrows()
            startrow = st.nrows() - 1 - nback
            # don't go back more than selected rows
            if maxnback > nstrows:
                maxnback = nstrows - 1
            stoprow = startrow - maxnback
            for linenum in xrange(startrow, stoprow - 1, -1):
                curline = st.getcell('MESSAGE', linenum)
                #if hint in curline:
                if curline.find(hint)!=-1:
                    retline = curline
                    break
            st.close()
            tblocal.close()
        except Exception, e:
            print "\nError getting last history line"
            tblocal.close()
            raise e

        return retline

    def check_history(self,histline, items):
        isok = True
        #print "\nhistline=",histline, " items=",items
        for item in items:
            if item not in histline:
                isok = False
                break
        if not isok:
            errmsg = "%s not found in %s.\n" % (items, histline)
            errmsg += "It could be that a change to HISTORY caused the wrong line to be selected."
            raise AssertionError, errmsg
        return isok

    def check_eq(self,val, expval, tol=None):
        """Checks that val matches expval within tol."""
        if type(val) == dict:
            for k in val:
                check_eq(val[k], expval[k], tol)
        else:
            try:
                if tol and hasattr(val, '__rsub__'):
                    are_eq = abs(val - expval) < tol
                else:
                    are_eq = val == expval
                if hasattr(are_eq, 'all'):
                    are_eq = are_eq.all()
                if not are_eq:
                    raise ValueError, '!='
            except ValueError:
                errmsg = "%r != %r" % (val, expval)
                if (len(errmsg) > 66): # 66 = 78 - len('ValueError: ')
                    errmsg = "\n%r\n!=\n%r" % (val, expval)
                raise ValueError, errmsg
            except Exception, e:
                print "Error comparing", val, "to", expval
                raise e


class test_SingleObservation(SetjyUnitTestBase):
    """Test single observation MS"""

    def setUp(self):
        # Replaced with a realistic ALMA data 
        #  - use modified version of CASAGuide's TWHya data (X3c1_wvrtsys.ms with only first 4scans)
        #    4 spws (wvr spw is already split out) 
        #self.setUpMS("unittest/setjy/2528.ms")         # Uranus
        #self.setUpMS("2528.ms")         # Uranus
        # Use TWHYA data (Titan)
        prefix = 'twhya_setjy' 
        self.ismms = False
        if testmms:
            msname=prefix+'.mms'
            self.ismms = True
        else:
            msname=prefix+'.ms'
        self.setUpMS(msname)        

    def tearDown(self):
        self.resetMS()

    def test1_SingleObservationOldModel(self):
        """ Test vs an MS with a single observation using the Butler-JPL-Horizons 2010 model"""

        os.system("mv " + self.inpms + " " + self.inpms + ".test1")
        self.inpms += ".test1"
        record = {}

        tblocal = tbtool()
        tblocal.open(self.inpms)
        cols = tblocal.colnames()
        tblocal.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + self.inpms + " already has a MODEL_DATA col" + str(cols)

        try:
            #print "\nRunning setjy(field='Uranus')."
            print "\nRunning setjy(field='Titan')."
            #sjran = setjy(vis=self.inpms, field='Uranus', spw='', modimage='',
            sjran = setjy(vis=self.inpms, field='Titan', spw='', modimage='',
                          scalebychan=False, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2010', usescratch=True)
            #print "sjran=",sjran
        except Exception, e:
            print "\nError running setjy(field='Uranus')"
            raise e

        try:
            tblocal.open(self.inpms)
            cols = tblocal.colnames()
            if 'MODEL_DATA' not in cols:
                #raise AssertionError, "setjy(field='Uranus') did not add a MODEL_DATA column"
                raise AssertionError, "setjy(field='Titan') did not add a MODEL_DATA column"
            else:
                #record['wvr'] = tblocal.getcell('MODEL_DATA', 0)
                #record['auto3'] = tblocal.getcell('MODEL_DATA', 10)
                #record['long3'] = tblocal.getcell('MODEL_DATA', 11)
                #record['auto4'] = tblocal.getcell('MODEL_DATA', 2)
                #record['med4'] = tblocal.getcell('MODEL_DATA', 4)
                #record['long4'] = tblocal.getcell('MODEL_DATA', 3)
                # for Titan
                # mms - sorted by spw so row no. of a specific data will be different from 
                # normal ms case...
                # and can be different depending on how the MMS is partitioned.
                # so use taql to find the appropriate row
                if self.ismms:
                    # MMS data storage layout changed???
                    #record['auto2'] = tblocal.getcell('MODEL_DATA', 1892)
                    #record['long2'] = tblocal.getcell('MODEL_DATA', 1930)
                    #record['auto3'] = tblocal.getcell('MODEL_DATA', 2838)
                    #record['med3'] = tblocal.getcell('MODEL_DATA', 2854)
                    #record['long3'] = tblocal.getcell('MODEL_DATA', 2868)
                    ####
                    querystr = 'FIELD_ID==1'
                    auto2query = querystr+' AND DATA_DESC_ID==2 AND ANTENNA1==0 AND ANTENNA2==0 AND TIME<2011/04/22/00:07:03' 
                    subt = tblocal.query(auto2query)
                    record['auto2'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    long2query = querystr+' AND DATA_DESC_ID==2 AND ANTENNA1==5 AND ANTENNA2==7 AND TIME<2011/04/22/00:07:03' 
                    subt = tblocal.query(long2query)
                    record['long2'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    auto3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==3 AND ANTENNA2==3 AND TIME<2011/04/22/00:07:03' 
                    subt = tblocal.query(auto3query)
                    record['auto3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    med3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==1 AND ANTENNA2==4 AND TIME<2011/04/22/00:07:03' 
                    subt = tblocal.query(med3query)
                    record['med3'] = subt.getcell('MODEL_DATA', 0)
                    long3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==3 AND ANTENNA2==7 AND TIME<2011/04/22/00:07:03' 
                    subt = tblocal.query(long3query)
                    record['long3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                else:
		    record['auto2'] = tblocal.getcell('MODEL_DATA', 270)
		    record['long2'] = tblocal.getcell('MODEL_DATA', 310)
		    record['auto3'] = tblocal.getcell('MODEL_DATA', 408)
		    record['med3'] = tblocal.getcell('MODEL_DATA', 424)
		    record['long3'] = tblocal.getcell('MODEL_DATA', 438)
                tblocal.close()
                #record['history'] = self.get_last_history_line(self.inpms, origin='setjy::imager::setjy()', hint='Uranus')
                #record['history'] = self.get_last_history_line(self.inpms, origin='imager::setjy()', hint='Uranus')
                # Exclude the test for history for MMS case for now...Currently get_last_history_line only look at history table 
                # in the parent MS of MMS but actually setjy run with MMS update the history info in individual SUBMSes.
                if not self.ismms: record['history'] = self.get_last_history_line(self.inpms, origin='imager::setjy()', hint='Titan')
                self.result = record
        except AssertionError, e:
            print "\nError accesing MODEL_DATA"
            tblocal.close()
            raise e

        #"""Flux density in HISTORY (Uranus)?"""
        """Flux density in HISTORY (Titan)?"""
        #self.check_history(self.result['history'], ["Uranus", "V=0] Jy"])
        #print "history =",self.result['history']
        if not self.ismms: self.check_history(self.result['history'], ["Titan", "V=0] Jy"])
        """Returned fluxes """
        self.assertTrue(sjran.has_key('1')) 
        #self.check_eq(sjran['0']['1']['fluxd'][0],65.23839313,0.0001)
        self.check_eq(sjran['1']['1']['fluxd'][0],3.33542042,0.0001)

        #"""WVR spw"""
        #self.check_eq(self.result['wvr'], numpy.array([[26.40653229+0.j,26.40653229+0.j]]),0.0001)

        """Zero spacing of spw 2"""
        #self.check_eq(self.result['auto3'], numpy.array([[65.80638885+0.j],[65.80638885+0.j]]),0.0001)
        self.check_eq(self.result['auto2'][0][0], (3.12483382+0.j),0.0001)
        """Long spacing of spw 3"""
        #self.check_eq(self.result['long3'], numpy.array([[4.76111794+0.j],[4.76111794+0.j]]),0.0001)
        self.check_eq(self.result['long2'][0][0],(2.84687614 +5.76921887e-12j),0.0001)
        """Zero spacing of spw 3"""
        #self.check_eq(self.result['auto4'], numpy.array([[69.33396912+0.j],[69.33396912+0.j]]),0.0001)
        self.check_eq(self.result['auto3'][0][0], (3.08946729+0.j),0.0001)
        """Medium spacing of spw 3"""
        #self.check_eq(self.result['med4'], numpy.array([[38.01076126+0.j],[38.01076126+0.j]]),0.0001)
        self.check_eq(self.result['med3'][0][0],(3.05192709 -1.08149264e-12j) ,0.0001)
        """Long spacing of spw 3"""
        #self.check_eq(self.result['long4'], numpy.array([[2.83933783+0.j],[2.83933783+0.j]]),0.0001)
        self.check_eq(self.result['long3'][0][0], (2.62474346 +6.37270531e-12j),0.0001)
        
        return sjran

    def test2_SingleObservationScaleByChan(self):
        """ Test vs an MS with a single observation using the Butler-JPL-Horizons 2010 model and scalying by channel"""

        os.system("mv " + self.inpms + " " + self.inpms + ".test2")
        self.inpms += ".test2"
        record = {}

        tblocal = tbtool()
        tblocal.open(self.inpms)
        cols = tblocal.colnames()
        tblocal.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + self.inpms + " already has a MODEL_DATA col" + str(cols)

        try:
            #print "\nRunning setjy(field='Uranus')."
            print "\nRunning setjy(field='Titan')."
            #sjran = setjy(vis=self.inpms, field='Uranus', spw='', modimage='',
            sjran = setjy(vis=self.inpms, field='Titan', spw='', modimage='',
                          scalebychan=True, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2010', usescratch=True)
        except Exception, e:
            #print "\nError running setjy(field='Uranus')"
            print "\nError running setjy(field='Titan')"
            raise e
        try:
            tblocal.open(self.inpms)
            cols = tblocal.colnames()
 
            if 'MODEL_DATA' not in cols:
                raise AssertionError, "setjy(field='Uranus') did not add a MODEL_DATA column"
            else:
                #record['wvr'] = tblocal.getcell('MODEL_DATA', 0)
                #record['auto1'] = tblocal.getcell('MODEL_DATA', 18)
                #record['long1'] = tblocal.getcell('MODEL_DATA', 19)
                #record['auto4'] = tblocal.getcell('MODEL_DATA', 2)
                #record['long4'] = tblocal.getcell('MODEL_DATA', 3)
                # Titan
                if self.ismms:
		    #record['auto0'] = tblocal.getcell('MODEL_DATA', 45)
		    #record['long0'] = tblocal.getcell('MODEL_DATA', 78)
		    #record['auto3'] = tblocal.getcell('MODEL_DATA', 2835)
		    #record['long3'] = tblocal.getcell('MODEL_DATA', 2868)
                    querystr = 'FIELD_ID==1'
                    auto0query = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==0 AND ANTENNA2==0 AND TIME/(24*3600) IN [{MJD(2011/04/22/00:07:03),MJD(2011/04/22/00:07:13)}]'
                    subt = tblocal.query(auto0query)
                    record['auto0'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    long0query = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==3 AND ANTENNA2==7 AND TIME/(24*3600) IN [{MJD(2011/04/22/00:07:03),MJD(2011/04/22/00:07:13)}]'
                    subt = tblocal.query(long0query)
		    record['long0'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    auto3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==0 AND ANTENNA2==0 AND TIME < 2011/04/22/00:07:03'
                    subt = tblocal.query(auto3query)
		    record['auto3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    long3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==3 AND ANTENNA2==7 AND TIME < 2011/04/22/00:07:03'
                    subt = tblocal.query(long3query)
		    record['long3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                else:
		    record['auto0'] = tblocal.getcell('MODEL_DATA', 45)
		    record['long0'] = tblocal.getcell('MODEL_DATA', 78)
		    record['auto3'] = tblocal.getcell('MODEL_DATA', 405)
		    record['long3'] = tblocal.getcell('MODEL_DATA', 438)
                tblocal.close()
            #    record['history'] = self.get_last_history_line(self.inpms, origin='setjy::imager::setjy()', hint="V=0] Jy")
                #record['history'] = self.get_last_history_line(self.inpms, origin='imager::setjy()', hint="V=0] Jy")
                if not self.ismms: record['history'] = self.get_last_history_line(self.inpms, origin='imager::setjy()', hint="V=0] Jy")
                self.result = record
        except AssertionError, e:
            print "\nError accesing MODEL_DATA"
            tblocal.close()
            raise e

        """Flux density in HISTORY (scalebychan)?"""
        #self.check_history(self.result['history'], ["Uranus", "V=0] Jy"])
        if not self.ismms: self.check_history(self.result['history'], ["Titan", "V=0] Jy"])

        #"""WVR spw with scalebychan"""
        #self.check_eq(self.result['wvr'], numpy.array([[25.93320656+0.j,
        #                                                26.88228607+0.j]]),
        #         0.003)

        """Zero spacing of spw 1 with scalebychan"""
        # 8 (decreasing freq!) chans, XX & YY.
        #self.check_eq(self.result['auto1'],
        #         numpy.array([[65.49415588+0.j, 65.42105865+0.j,
        #                       65.34798431+0.j, 65.27491760+0.j,
        #                       65.20187378+0.j, 65.12883759+0.j,
        #                       65.05581665+0.j, 64.98281097+0.j],
        #                      [65.49415588+0.j, 65.42105865+0.j,
        #                       65.34798431+0.j, 65.27491760+0.j,
        #                       65.20187378+0.j, 65.12883759+0.j,
        #                       65.05581665+0.j, 64.98281097+0.j]]),0.0001)
        # Titan ------------
        # check spw0, YY chan 0, 1920, 3839
        self.check_eq(self.result['auto0'][1][0], 3.30965233+0.j, 0.0001)
        self.check_eq(self.result['auto0'][1][1920], 3.31375313+0j, 0.0001)
        self.check_eq(self.result['auto0'][1][3839], 3.31785417+0j, 0.0001)

        """Long spacing of spw 1 with scalebychan"""
        #self.check_eq(self.result['long1'],
        #         numpy.array([[4.92902184+0.j, 4.96826363+0.j,
        #                       5.00747252+0.j, 5.04664850+0.j,
        #                       5.08579159+0.j, 5.12490082+0.j,
        #                       5.16397619+0.j, 5.20301771+0.j],
        #                      [4.92902184+0.j, 4.96826363+0.j,
        #                       5.00747252+0.j, 5.04664850+0.j,
        #                       5.08579159+0.j, 5.12490082+0.j,
        #                       5.16397619+0.j, 5.20301771+0.j]]),0.0001)
        # Titan
        self.check_eq(self.result['long0'][1][0],(2.77658414+6.98719121e-12j),0.0001)
        self.check_eq(self.result['long0'][1][1920],(2.77936244+6.99878090e-12j),0.0001)
        self.check_eq(self.result['long0'][1][3839],(2.78213906+7.01037362e-12j),0.0001)

        # spw 4 only has 1 chan, so it should be the same as without scalebychan.
        #"""Zero spacing of spw 4 with scalebychan"""
        #self.check_eq(self.result['auto4'], numpy.array([[69.33396912+0.j],[69.33396912+0.j]]),0.0001)
        """Zero spacing of spw 3 with scalebychan"""
        self.check_eq(self.result['auto3'][1][0], (3.0934467+0j),0.0001)
        self.check_eq(self.result['auto3'][1][1920], (3.08946729+0j),0.0001)
        self.check_eq(self.result['auto3'][1][3839], (3.08549213+0j),0.0001)

        #"""Long spacing of spw 4 with scalebychan"""
        #self.check_eq(self.result['long4'], numpy.array([[2.83933783+0.j],[2.83933783+0.j]]),0.0001)

        """Long spacing of spw 3 with scalebychan"""
        self.check_eq(self.result['long3'][1][0],(2.62812424+6.38091359e-12j) ,0.0001)
        self.check_eq(self.result['long3'][1][1920],(2.62534332+6.36981873e-12j) ,0.0001)
        self.check_eq(self.result['long3'][1][3839],(2.62256360+6.35873776e-12j) ,0.0001)

        return sjran

    def test3_SingleObservationNewModel(self):
        """ Test vs an MS with one single observation using the Butler-JPL-Horizons 2012 model"""

        # print out some values for debugging
        debug=False

        os.system("mv " + self.inpms + " " + self.inpms + ".test3")
        self.inpms += ".test3"
        record = {}

        tblocal = tbtool()
        tblocal.open(self.inpms)
        cols = tblocal.colnames()
        tblocal.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + self.inpms + " already has a MODEL_DATA col" + str(cols)

        try:
            #print "\nRunning setjy(field='Uranus')."
            print "\nRunning setjy(field='Titan')."
            #sjran = setjy(vis=self.inpms, field='Uranus', spw='', modimage='',
            sjran = setjy(vis=self.inpms, field='Titan', spw='', modimage='',
                          scalebychan=False, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2012', usescratch=True)
        except Exception, e:
            print "\nError running setjy(field='Uranus')"
            raise e
        try:
            tblocal.open(self.inpms)
            cols = tblocal.colnames()
            if 'MODEL_DATA' not in cols:
                #raise AssertionError, "setjy(field='Uranus') did not add a MODEL_DATA column"
                raise AssertionError, "setjy(field='Titan') did not add a MODEL_DATA column"
            else:
                #record['wvr'] = tblocal.getcell('MODEL_DATA', 0)
                #record['auto3'] = tblocal.getcell('MODEL_DATA', 10)
                #record['long3'] = tblocal.getcell('MODEL_DATA', 11)
                #record['auto4'] = tblocal.getcell('MODEL_DATA', 2)
                #record['med4'] = tblocal.getcell('MODEL_DATA', 4)
                #record['long4'] = tblocal.getcell('MODEL_DATA', 3)
                #Titan
                if self.ismms:
                    #record['auto2'] = tblocal.getcell('MODEL_DATA', 1892)
                    #record['long2'] = tblocal.getcell('MODEL_DATA', 1930)
                    #record['auto3'] = tblocal.getcell('MODEL_DATA', 2838)
                    #record['med3'] = tblocal.getcell('MODEL_DATA', 2854)
                    #record['long3'] = tblocal.getcell('MODEL_DATA', 2868)
                    querystr = 'FIELD_ID==1'
                    auto2query = querystr+' AND DATA_DESC_ID==2 AND ANTENNA1==0 AND ANTENNA2==0 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(auto2query)
                    record['auto2'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    long2query = querystr+' AND DATA_DESC_ID==2 AND ANTENNA1==5 AND ANTENNA2==7 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(long2query)
                    record['long2'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    auto3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==3 AND ANTENNA2==3 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(auto3query)
                    record['auto3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    med3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==1 AND ANTENNA2==4 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(med3query)
                    record['med3'] = subt.getcell('MODEL_DATA', 0)
                    long3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==3 AND ANTENNA2==7 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(long3query)
                    record['long3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()

                else:
                    record['auto2'] = tblocal.getcell('MODEL_DATA', 270)
                    record['long2'] = tblocal.getcell('MODEL_DATA', 310)
                    record['auto3'] = tblocal.getcell('MODEL_DATA', 408)
                    record['med3'] = tblocal.getcell('MODEL_DATA', 424)
                    record['long3'] = tblocal.getcell('MODEL_DATA', 438)
                tblocal.close()
                #record['history'] = self.get_last_history_line(self.inpms, origin='setjy', hint='Uranus')
                if not self.ismms: record['history'] = self.get_last_history_line(self.inpms, origin='setjy', hint='Titan')
                self.result = record
        except AssertionError, e:
            print "\nError accesing MODEL_DATA"
            tblocal.close()
            raise e

        if debug:
          print "self.result['history']=",self.result['history']
          print "self.result['auto0']=",self.result['auto0']
          print "self.result['auto3']=",self.result['auto3']

        #"""Flux density in HISTORY (Uranus)?"""
        #self.check_history(self.result['history'], ["Uranus:", "V=0.0] Jy"])
        """Flux density in HISTORY (Titan)?"""
        if not self.ismms: self.check_history(self.result['history'], ["Titan:", "V=0.0] Jy"])

        """Returned fluxes """
        self.assertTrue(sjran.has_key('1')) 
        self.check_eq(sjran['1']['1']['fluxd'][0],3.27488661,0.0001)

        #"""WVR spw"""
        #self.check_eq(self.result['wvr'], numpy.array([[ 25.33798409+0.j,25.33798409+0.j]]),0.0001)
        # new value after code and ephemeris data update 2012-10-03
        #self.check_eq(self.result['wvr'], numpy.array([[ 25.33490372+0.j, 25.33490372+0.j]]),0.0001)

        #"""Zero spacing of spw 3"""
	#self.check_eq(self.result['auto3'], numpy.array([[ 66.72530365+0.j],[ 66.72530365+0.j]]),0.0001)
        # new value after code and ephemeris data update 2012-10-03
	#self.check_eq(self.result['auto3'], numpy.array([[ 66.71941376+0.j], [ 66.71941376+0.j]]),0.0001)
        #"""Zero spacing of spw 4"""
        #self.check_eq(self.result['auto4'], numpy.array([[ 70.40153503+0.j],[ 70.40153503+0.j]]),0.0001)
        # new value after code and ephemeris data update 2012-10-03
        #self.check_eq(self.result['auto4'], numpy.array([[ 70.39561462+0.j], [ 70.39561462+0.j]]), 0.0001)
        """Zero spacing of spw 2"""
        self.check_eq(self.result['auto2'][0][0], (6.69543791+0.j),0.0001)

        """Long spacing of spw 2"""
        self.check_eq(self.result['long2'][0][0],(6.09987020 +2.47228783e-11j),0.0001)

        """Zero spacing of spw 3"""
        self.check_eq(self.result['auto3'][0][0], (3.13487768+0.j),0.0001)

        """Medium spacing of spw 3"""
        self.check_eq(self.result['med3'][0][0],(3.09678578 -2.19477778e-12j) ,0.0001)

        """Long spacing of spw 3"""
        self.check_eq(self.result['long3'][0][0], (2.66332293 +1.29327478e-11j),0.0001)
        return sjran

    def test4_SingleObservationSelectByIntent(self):
        """ Test vs an MS with one single observation using the Butler-JPL-Horizons 2010 model with the selection by intent"""

        os.system("mv " + self.inpms + " " + self.inpms + ".test4")
        self.inpms += ".test4"
        record = {}

        tblocal = tbtool()
        tblocal.open(self.inpms)
        cols = tblocal.colnames()
        tblocal.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + self.inpms + " already has a MODEL_DATA col" + str(cols)

        try:
            #print "\nRunning setjy(field='Uranus')."
            print "\nRunning setjy(field='Titan')."
            sjran = setjy(vis=self.inpms, field='', spw='', modimage='',
                          selectdata=True, intent="*AMPLI*",
                          scalebychan=True, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2010', usescratch=True)
        except Exception, e:
            #print "\nError running setjy(field='Uranus')"
            print "\nError running setjy(field='Titan')"
            raise e
        try:
            tblocal.open(self.inpms)
            cols = tblocal.colnames()
            if 'MODEL_DATA' not in cols:
                #raise AssertionError, "setjy(field='Uranus') did not add a MODEL_DATA column"
                raise AssertionError, "setjy(field='Titan') did not add a MODEL_DATA column"
            else:
                #record['wvr'] = tblocal.getcell('MODEL_DATA', 0)
                #record['auto1'] = tblocal.getcell('MODEL_DATA', 18)
                #record['long1'] = tblocal.getcell('MODEL_DATA', 19)
                #record['auto4'] = tblocal.getcell('MODEL_DATA', 2)
                #record['long4'] = tblocal.getcell('MODEL_DATA', 3)
                #Titan
                if self.ismms:
                    #record['auto0'] = tblocal.getcell('MODEL_DATA', 45)
                    #record['long0'] = tblocal.getcell('MODEL_DATA', 78)
		    #record['auto3'] = tblocal.getcell('MODEL_DATA', 2835)
		    #record['long3'] = tblocal.getcell('MODEL_DATA', 2868)
                    querystr = 'FIELD_ID==1'
                    auto0query = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==0 AND ANTENNA2==0 AND TIME/(24*3600) IN [{MJD(2011/04/22/00:07:03),MJD(2011/04/22/00:07:13)}]'
                    subt = tblocal.query(auto0query)
                    record['auto0'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    long0query = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==3 AND ANTENNA2==7 AND TIME/(24*3600) IN [{MJD(2011/04/22/00:07:03),MJD(2011/04/22/00:07:13)}]'
                    subt = tblocal.query(long0query)
                    record['long0'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    auto3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==0 AND ANTENNA2==0 AND TIME < 2011/04/22/00:07:03'
                    subt = tblocal.query(auto3query)
                    record['auto3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    long3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==3 AND ANTENNA2==7 AND TIME < 2011/04/22/00:07:03'
                    subt = tblocal.query(long3query)
                    record['long3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()

                else:
                    record['auto0'] = tblocal.getcell('MODEL_DATA', 45)
                    record['long0'] = tblocal.getcell('MODEL_DATA', 78)
                    record['auto3'] = tblocal.getcell('MODEL_DATA', 405)
                    record['long3'] = tblocal.getcell('MODEL_DATA', 438)
                tblocal.close()
                #record['history'] = self.get_last_history_line(self.inpms, origin='setjy::imager::setjy()', hint="V=0] Jy")
                if not self.ismms: record['history'] = self.get_last_history_line(self.inpms, origin='imager::setjy()', hint="V=0] Jy")
                self.result = record
        except AssertionError, e:
            print "\nError accesing MODEL_DATA"
            tblocal.close()
            raise e

        """Flux density in HISTORY (selectbyIntent)?"""
        #self.check_history(self.result['history'], ["Uranus", "V=0] Jy"])
        if not self.ismms: self.check_history(self.result['history'], ["Titan", "V=0] Jy"])

        #"""WVR spw with selectbyIntent"""
        #self.check_eq(self.result['wvr'], numpy.array([[25.93320656+0.j,
        #                                                26.88228607+0.j]]),
        #         0.003)

        #"""Zero spacing of spw 1 with scalebychan"""
        # 8 (decreasing freq!) chans, XX & YY.
        #self.check_eq(self.result['auto1'],
        #         numpy.array([[65.49415588+0.j, 65.42105865+0.j,
        #                       65.34798431+0.j, 65.27491760+0.j,
        #                       65.20187378+0.j, 65.12883759+0.j,
        #                       65.05581665+0.j, 64.98281097+0.j],
        #                      [65.49415588+0.j, 65.42105865+0.j,
        #                       65.34798431+0.j, 65.27491760+0.j,
        #                       65.20187378+0.j, 65.12883759+0.j,
        #                       65.05581665+0.j, 64.98281097+0.j]]),0.0001)

        #"""Long spacing of spw 1 with scalebychan"""
        #self.check_eq(self.result['long1'],
        #         numpy.array([[4.92902184+0.j, 4.96826363+0.j,
        #                       5.00747252+0.j, 5.04664850+0.j,
        #                       5.08579159+0.j, 5.12490082+0.j,
        #                       5.16397619+0.j, 5.20301771+0.j],
        #                      [4.92902184+0.j, 4.96826363+0.j,
        #                       5.00747252+0.j, 5.04664850+0.j,
        #                       5.08579159+0.j, 5.12490082+0.j,
        #                       5.16397619+0.j, 5.20301771+0.j]]),0.0001)

        # spw 4 only has 1 chan, so it should be the same as without scalebychan.
        #"""Zero spacing of spw 4 with scalebychan"""
        #self.check_eq(self.result['auto4'], numpy.array([[69.33396912+0.j],[69.33396912+0.j]]),0.0001)
        #"""Long spacing of spw 4 with scalebychan"""
        #self.check_eq(self.result['long4'], numpy.array([[2.83933783+0.j],[2.83933783+0.j]]),0.0001)

        """Zero spacing of spw 3 with scalebychan, selectbyintent"""
        self.check_eq(self.result['auto3'][1][0], (3.0934467+0j),0.0001)
        self.check_eq(self.result['auto3'][1][1920], (3.08946729+0j),0.0001)
        self.check_eq(self.result['auto3'][1][3839], (3.08549213+0j),0.0001)

        return sjran

    def test5_SingleObservationSelectByIntentNewModel(self):
        """ Test vs an MS with one single observation using the Butler-JPL-Horizons 2012 model with the selection by intent"""

        # print out some values for debugging
        debug=False

        os.system("mv " + self.inpms + " " + self.inpms + ".test5")
        self.inpms += ".test5"
        record = {}

        tblocal = tbtool()
        tblocal.open(self.inpms)
        cols = tblocal.colnames()
        tblocal.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + self.inpms + " already has a MODEL_DATA col" + str(cols)

        try:
            #print "\nRunning setjy(field='Uranus')."
            print "\nRunning setjy(field='Titan')."
            sjran = setjy(vis=self.inpms, field='', spw='', modimage='',
                          selectdata=True, intent="*AMPLI*",
                          scalebychan=False, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2012', usescratch=True)
        except Exception, e:
            #print "\nError running setjy(field='Uranus')"
            print "\nError running setjy(field='Titan')"
            raise e
        try:
            tblocal.open(self.inpms)
            cols = tblocal.colnames()
            if 'MODEL_DATA' not in cols:
                #raise AssertionError, "setjy(field='Uranus') did not add a MODEL_DATA column"
                raise AssertionError, "setjy(field='Titan') did not add a MODEL_DATA column"
            else:
                #record['wvr'] = tblocal.getcell('MODEL_DATA', 0)
                #record['auto3'] = tblocal.getcell('MODEL_DATA', 10)
                #record['long3'] = tblocal.getcell('MODEL_DATA', 11)
                #record['auto4'] = tblocal.getcell('MODEL_DATA', 2)
                #record['med4'] = tblocal.getcell('MODEL_DATA', 4)
                #record['long4'] = tblocal.getcell('MODEL_DATA', 3)
                # Titan
                if self.ismms:
                    # row numbers for specific data changed...
                    #record['auto2'] = tblocal.getcell('MODEL_DATA', 1892)
                    #record['long2'] = tblocal.getcell('MODEL_DATA', 1930)
                    #record['auto3'] = tblocal.getcell('MODEL_DATA', 2838)
                    #record['med3'] = tblocal.getcell('MODEL_DATA', 2854)
                    #record['long3'] = tblocal.getcell('MODEL_DATA', 2868)
                    querystr = 'FIELD_ID==1'
                    auto2query = querystr+' AND DATA_DESC_ID==2 AND ANTENNA1==0 AND ANTENNA2==0 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(auto2query)
                    record['auto2'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    long2query = querystr+' AND DATA_DESC_ID==2 AND ANTENNA1==5 AND ANTENNA2==7 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(long2query)
                    record['long2'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    auto3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==3 AND ANTENNA2==3 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(auto3query)
                    record['auto3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    med3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==1 AND ANTENNA2==4 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(med3query)
                    record['med3'] = subt.getcell('MODEL_DATA', 0)
                    long3query = querystr+' AND DATA_DESC_ID==3 AND ANTENNA1==3 AND ANTENNA2==7 AND TIME<2011/04/22/00:07:03'
                    subt = tblocal.query(long3query)
                    record['long3'] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                else:
		    record['auto2'] = tblocal.getcell('MODEL_DATA', 270)
		    record['long2'] = tblocal.getcell('MODEL_DATA', 310)
		    record['auto3'] = tblocal.getcell('MODEL_DATA', 408)
		    record['med3'] = tblocal.getcell('MODEL_DATA', 424)
		    record['long3'] = tblocal.getcell('MODEL_DATA', 438)
                tblocal.close()
                #record['history'] = self.get_last_history_line(self.inpms, origin='setjy', hint='Uranus')
                if not self.ismms: record['history'] = self.get_last_history_line(self.inpms, origin='setjy', hint='Titan')
                self.result = record
        except AssertionError, e:
            print "\nError accesing MODEL_DATA"
            tblocal.close()
            raise e

        if debug:
          if not self.ismms: print "self.result['history']=",self.result['history']
          print "self.result['auto0']=",self.result['auto0']
          print "self.result['auto3']=",self.result['auto3']

        #"""Flux density in HISTORY (Uranus)?"""
        #self.check_history(self.result['history'], ["Uranus:", "V=0.0] Jy"])
        #"""WVR spw"""
        #self.check_eq(self.result['wvr'], numpy.array([[ 25.33798409+0.j,25.33798409+0.j]]),0.0001)
        # new value after code and ephemeris data update 2012-10-03
        #self.check_eq(self.result['wvr'], numpy.array([[ 25.33490372+0.j, 25.33490372+0.j]]),0.0001)
        #"""Zero spacing of spw 3"""
	#self.check_eq(self.result['auto3'], numpy.array([[ 66.72530365+0.j],[ 66.72530365+0.j]]),0.0001)
        # new value after code and ephemeris data update 2012-10-03
	#self.check_eq(self.result['auto3'], numpy.array([[ 66.71941376+0.j], [ 66.71941376+0.j]]),0.0001)
        #"""Zero spacing of spw 4"""
        #self.check_eq(self.result['auto4'], numpy.array([[ 70.40153503+0.j],[ 70.40153503+0.j]]),0.0001)
        # new value after code and ephemeris data update 2012-10-03
        #self.check_eq(self.result['auto4'], numpy.array([[ 70.39561462+0.j], [ 70.39561462+0.j]]), 0.0001)
        #Titan
        """Zero spacing of spw 2"""
        self.check_eq(self.result['auto2'][0][0], (6.69543791+0.j),0.0001)

        """Long spacing of spw 2"""
        self.check_eq(self.result['long2'][0][0],(6.09987020 +2.47228783e-11j),0.0001)

        """Zero spacing of spw 3"""
        self.check_eq(self.result['auto3'][0][0], (3.13487768+0.j),0.0001)

        """Medium spacing of spw 3"""
        self.check_eq(self.result['med3'][0][0],(3.09678578 -2.19477778e-12j) ,0.0001)

        """Long spacing of spw 3"""
        self.check_eq(self.result['long3'][0][0], (2.66332293 +1.29327478e-11j),0.0001)

        return sjran

class test_MultipleObservations(SetjyUnitTestBase):
    """Test multiple observation MS (CAS-3320)"""

    def setUp(self):
        prefix = 'multiobs' 
        self.ismms = False
        if testmms:
            msname = prefix+'.mms'
            self.ismms = True
        else:
            msname=prefix+'.ms'
        #self.setUpMS("unittest/setjy/multiobs.ms")         # Titan
        self.setUpMS(msname)         # Titan

    def tearDown(self):
        self.resetMS()

    def test1_MultipleObservationOldModel(self):
        """ Test vs an MS with multiple observations using the Butler-JPL-Horizons 2010 model"""

        os.system("mv " + self.inpms + " " + self.inpms + ".test1")
        self.inpms += ".test1"
        record = {}

        tblocal = tbtool()
        tblocal.open(self.inpms)
        cols = tblocal.colnames()
        tblocal.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + self.inpms + " already has a MODEL_DATA col" + str(cols)

        try:
            print "\nRunning setjy(field='Titan')."
            sjran = setjy(self.inpms, field='Titan', spw='',
                          selectdata=True, observation=1, 
                          modimage='',
                          scalebychan=False, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2010', usescratch=True)
        except Exception, e:
            print "\nError running setjy(field='Titan')"
            raise e

        try:
            tblocal.open(self.inpms)
            cols = tblocal.colnames()
            if 'MODEL_DATA' not in cols:
                raise AssertionError, "setjy(field='Titan') did not add a MODEL_DATA column"
            else:
                if self.ismms:
                    # MMS data row layout changed 
		    #record[0] = tblocal.getcell('MODEL_DATA', 0)[0, 0]
		    #record[1] = tblocal.getcell('MODEL_DATA', 386)[0]
		    #record[2] = tblocal.getcell('MODEL_DATA', 544)[0, 0]
                    querystr = 'STATE_ID==0'
                    query0 = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==0 AND ANTENNA2==1 AND FIELD_ID==0'
                    subt = tblocal.query(query0)
                    record[0] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    query1 = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==6 AND ANTENNA2==8 AND FIELD_ID==1'
                    subt = tblocal.query(query1)
                    record[1] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    query1 = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==2 AND ANTENNA2==7 AND FIELD_ID==2'
                    subt = tblocal.query(query1)
                    record[2] = subt.getcell('MODEL_DATA', 0)

                else:
		    record[0] = tblocal.getcell('MODEL_DATA', 0)[0, 0]
		    record[1] = tblocal.getcell('MODEL_DATA', 666)[0]
		    record[2] = tblocal.getcell('MODEL_DATA', 950)[0, 0]
                tblocal.close()
                self.result = record
        except AssertionError, e:
            print "\nError accesing MODEL_DATA"
            tblocal.close()
            raise e

        """Was obsID 0 left alone?"""
        self.check_eq(self.result[0], 1.0+0.0j, 0.003)
        """Was obsID 1 set?"""
        self.check_eq(self.result[1],
                 numpy.array([1.40439999+0.j, 1.40436542+0.j,
                              1.40433097+0.j, 1.40429640+0.j]), 0.003)
        """Was obsID 2 left alone?"""
        self.check_eq(self.result[2], 1.0+0.0j, 0.003)
        # TODO:use record to check values in MODEL_DATA

    def test2_MultipleObservationNewModel(self):
        """ Test vs an MS with multiple observations using the Butler-JPL-Horizons 2012 model"""

        os.system("mv " + self.inpms + " " + self.inpms + ".test2")
        self.inpms += ".test2"
        record = {}

        tblocal = tbtool()
        tblocal.open(self.inpms)
        cols = tblocal.colnames()
        tblocal.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + self.inpms + " already has a MODEL_DATA col" + str(cols)

        try:
            print "\nRunning setjy(field='Titan')."
            sjran = setjy(self.inpms, field='Titan', spw='',
                          selectdata=True, observation=1, 
                          modimage='',
                          scalebychan=False, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2012', usescratch=True)
        except Exception, e:
            print "\nError running setjy(field='Uranus')"
            raise e

        try:
            tblocal.open(self.inpms)
            cols = tblocal.colnames()
            if 'MODEL_DATA' not in cols:
                raise AssertionError, "setjy(field='Titan') did not add a MODEL_DATA column"
            else:
                if self.ismms:
		    #record[0] = tblocal.getcell('MODEL_DATA', 0)[0, 0]
		    #record[1] = tblocal.getcell('MODEL_DATA', 979)[0]
		    #record[2] = tblocal.getcell('MODEL_DATA', 544)[0, 0]
                    querystr = 'STATE_ID==0'
                    query0 = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==0 AND ANTENNA2==1 AND FIELD_ID==0'
                    subt = tblocal.query(query0)
                    record[0] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    query1 = querystr+' AND DATA_DESC_ID==1 AND ANTENNA1==0 AND ANTENNA2==5 AND FIELD_ID==1'
                    subt = tblocal.query(query1)
                    record[1] = subt.getcell('MODEL_DATA', 0)
                    subt.close()
                    query1 = querystr+' AND DATA_DESC_ID==0 AND ANTENNA1==2 AND ANTENNA2==7 AND FIELD_ID==2'
                    subt = tblocal.query(query1)
                    record[2] = subt.getcell('MODEL_DATA', 0)
                else:
		    record[0] = tblocal.getcell('MODEL_DATA', 0)[0, 0]
		    record[1] = tblocal.getcell('MODEL_DATA', 671)[0]
		    record[2] = tblocal.getcell('MODEL_DATA', 950)[0, 0]
                tblocal.close()
                self.result = record
        except AssertionError, e:
            print "\nError accesing MODEL_DATA"
            tblocal.close()
            raise e

        """Was obsID 0 left alone?"""
        self.check_eq(self.result[0], 1.0+0.0j, 0.003)
        """Was obsID 1 set?"""
        self.check_eq(self.result[1],
	#        numpy.array([ 1.21551239-0.33617234j,  1.19003308-0.41755155j,
	#                      1.15911222-0.49702403j,  1.12289071-0.57422638j]),
	        numpy.array([ 1.26114714+0.j,  1.26116526+0.j, 1.26118350+0.j,  1.26120162+0.j]),
			      0.003)
        """Was obsID 2 left alone?"""
        self.check_eq(self.result[2], 1.0+0.0j, 0.003)
        # TODO:use record to check values in MODEL_DATA


class test_ModImage(SetjyUnitTestBase):

    def setUp(self):
        #self.inpuvf = datapath + '/ATST2/NGC1333/N1333_1.UVFITS'
        #self.inpms = 'unittest/setjy/n1333_1.ms'
        prefix = 'n1333_1' 
        self.ismms = False
        if testmms:
            msname=prefix+'.mms'
            self.ismms = True
        else:
            msname=prefix+'.ms'
        #self.setUpMS("unittest/setjy/multiobs.ms")         # Titan
        self.setUpMS(msname)         # Titan
        #self.inpms = 'n1333_1.ms'
        self.field = '0542+498_1'
        self.modelim = '3C147_U.im'
        self.result = {}
        #if not os.path.exists(self.inpuvf):
        #    raise EnvironmentError, "Missing input UVFITS file: " + datapath + self.inpuvf

        #try:
            #if not os.path.exists('unittest/setjy'):
            #    print "\nCreate working area..."
            #    os.system('mkdir -p unittest/setjy')
        #    print "Importing", self.inpuvf, "to an MS."
        #    importuvfits(fitsfile=self.inpuvf, vis=self.inpms,antnamescheme="new")
        #except Exception, e:
        #    print "importuvfits error:"
        #    raise e

    def tearDown(self):
        self.resetMS()
        
    
    def test1_UBandModelwithQBandMS(self):
        """ Test U-Band model with Q-Band data to see impact of flux density scale """

        # The MS is in Q band, so deliberately choose the U band model so that the structure
        # is not too far off, but whether or not its flux density is scaled makes a difference.

        print "Running multiple setjy with different parameters..."
        for use_oldstandard in [False, True]:
        # for debugging ...
        #for use_oldstandard in [True]:
            selStandard = ("Perley-Taylor 99" if use_oldstandard else "Perley-Butler 2010")
            print "!!!!! Run with standard=\"%s\" !!!!!" % selStandard
            self.result[use_oldstandard] = self.run_setjy(use_oldstandard)

        
        print "!!!! Run with standard=\"manual\", fluxdensity !!!!!"
        self.result['fluxdens'] = self.run_setjy(False, 1234.0)
        print "!!!! Run with standard=\"manual\", fluxdensity and spix !!!!!"
        self.result['spix'] = self.run_setjy(False,1234.0 * (43.42064/35.0)**0.7,-0.7,"35.0GHz")

        # check on HISTORY sub-table entries - does not check for values
        """Flux density in HISTORY (old standard)?"""
        #no scaling
        #self.check_history(self.result[True]['history'],["Scaling spw 1's model image to I ="])
        if not self.ismms: self.check_history(self.result[True]['history'],["fld ind 12) spw 1  [I="])
        """Flux density in HISTORY (new default standard)?"""
        if not self.ismms: self.check_history(self.result[False]['history'],["Scaling spw(s) [0, 1]'s model image to I ="])
        #"""Flux density in HISTORY (fluxdensity)?""" <= no flux density is written in HISTORY, just input flux dens.
        #self.check_history(self.result['fluxdens']['history'],["Scaling spw 1's model image to I ="])
        """Flux density in HISTORY (spix)?"""
        #self.check_history(self.result['spix']['history'],["Scaling spw 1's model image to I ="])
        if not self.ismms: self.check_history(self.result['spix']['history'],["Flux density as a function of frequency"])

        # computed flux check
        # -different standards
        """ Returned flux density (using old standard) """
        # fieldid = 12
        self.assertTrue(self.result[True]['setjyran'].has_key('12'))
        self.check_eq(self.result[True]['setjyran']['12']['1']['fluxd'][0],0.91134687,0.0001)
        """ Returned flux density (default standard=Perley-Butler 2010) """
        self.assertTrue(self.result[False]['setjyran'].has_key('12'))
        #self.check_eq(self.result[False]['setjyran']['12']['1']['fluxd'][0],0.0,0.0001)
        # Updated value for the updated run_setjy 2014-05-01 TT
        self.check_eq(self.result[False]['setjyran']['12']['1']['fluxd'][0],1.0510757,0.0001)
        #
        # -manual mode (fluxdensity specification)
        """ Returned flux density (with input fluxdensity) """
        self.assertTrue(self.result['fluxdens']['setjyran'].has_key('12'))
        self.check_eq(self.result['fluxdens']['setjyran']['12']['1']['fluxd'][0],1234.0,0.0001)
        """ Returned flux density (with input fluxdensity and spix) """
        self.assertTrue(self.result['spix']['setjyran'].has_key('12'))
        #self.check_eq(self.result['spix']['setjyran']['12']['1']['fluxd'][0],1233.91240671,0.0001)
        # Updated value for the updated run_setjy 2014-05-01 TT
        self.check_eq(self.result['spix']['setjyran']['12']['1']['fluxd'][0],1234.0328507,0.0001)
        #
        # -for standard='Perley-Butler 2010, with model image
        """modimage != '' and fluxdensity == 0 -> no scaling?"""
        #self.check_eq(self.result[False]['short'], 2.712631, 0.05)
        # Updated value for the updated run_setjy 2014-05-01 TT
        self.check_eq(self.result[False]['short'], 1.0508747, 0.05)
        #self.check_eq(self.result[False]['long'],  2.4080808, 0.05)
        # Updated value for the updated run_setjy 2014-05-01 TT
        self.check_eq(self.result[False]['long'],  0.9328917, 0.05)
        #
        # -for standard='Perley-Taylor 99' (no model specification is allowed)
        """Perley-Taylor 99 standard?"""
        self.check_eq(self.result[True]['short'], 0.911185, 0.025)
        #self.check_eq(self.result[True]['long'],  0.808885, 0.025)
        # Updated value for the updated run_setjy 2014-05-01 TT
        self.check_eq(self.result[True]['long'],  0.9114067, 0.025)
        #"""modimage != '' and fluxdensity > 0""" this is no longer supported in the task
        """fluxdensity > 0"""  # should be = input fluxdensity for model vis
        self.check_eq(self.result['fluxdens']['short'], 1234.0, 0.05)
        self.check_eq(self.result['fluxdens']['long'],  1234.0, 0.05)
        #"""modimage != '', fluxdensity > 0, and spix = -0.7""" with modimage no longer supproted
        """fluxdensity > 0, and spix = -0.7"""
        #self.check_eq(self.result['spix']['short'], 1233.7, 0.5)
        #self.check_eq(self.result['spix']['long'],  1095.2, 0.5)
        self.check_eq(self.result['spix']['short'], 1234.0, 0.5)
        self.check_eq(self.result['spix']['long'],  1234.0, 0.5)

        return True

    def run_setjy(self, use_oldstandard, fluxdens=0, spix=0, reffreq="1GHz"):
        record = {'setjyran': False}
        try:
            if use_oldstandard:
                record['setjyran'] = setjy(vis=self.inpms, field=self.field,
                                           #modimage=self.modelim,
                                           scalebychan=False,
                                           standard='Perley-Taylor 99',
                                           usescratch=True
                                           )

                if not self.ismms: record['history'] = self.get_last_history_line(self.inpms,
                                                           #origin='imager::setjy()::',
                                                           origin='imager::setjy()',
                                                           #hint='model image to I')
                                                           hint='fld ind 12) spw 1  [I=')
            else:
                if fluxdens==0:
                    # use default standard with model
                    record['setjyran'] = setjy(vis=self.inpms, field=self.field,
                                               model=self.modelim,
                                               scalebychan=False,
                                               standard='Perley-Butler 2010',
                                               spix=spix, reffreq=reffreq,
                                               usescratch=True
                                               )

                    if not self.ismms: record['history'] = self.get_last_history_line(self.inpms,
                                                           origin='imager::setjy()',
                                                           hint='model image to I')
                else:
                    record['setjyran'] = setjy(vis=self.inpms, field=self.field,
                                               #model=self.modelim,
                                               #scalebychan=False,
                                               scalebychan=True,
                                               standard='manual',
                                               fluxdensity=fluxdens,
                                               spix=spix, reffreq=reffreq,
                                               usescratch=True
                                               )

                    if spix!=0.0 and not self.ismms:
                        record['history'] = self.get_last_history_line(self.inpms,
                                                           origin='imager::setjy()',
                                                           hint='Flux density as a function of frequency')


            ms.open(self.inpms)
            record['short'] = ms.statistics(column='MODEL',
                                            complex_value='amp',
                                            field='0542+498_1',
                                            baseline='2&9',
                                            time='2003/05/02/19:53:30.0',
                                            correlation='rr')['MODEL']['mean']
            record['long']  = ms.statistics(column='MODEL',
                                            complex_value='amp',
                                            field='0542+498_1',
                                            baseline='21&24',
                                            time='2003/05/02/19:53:30.0',
                                            correlation='ll')['MODEL']['mean']
            ms.close()
        except Exception, e:
            print "Error from setjy or ms.statistics()"
            raise e

        return record
    
class test_inputs(SetjyUnitTestBase):
    """Test input parameter checking"""
    def setUp(self):
        #self.setUpMS("unittest/setjy/2528.ms")         # Uranus
        #self.setUpMS("2528.ms")         # Uranus
        self.setUpMS("multiobs.ms")

    def tearDown(self):
        self.resetMS()

    def test_vischeck(self):
        """ Test input vis check"""
        self.inpms='wrong.ms'
        if os.path.exists(self.inpms):
            shutil.rmtree(self.inpms) 


        # test by temporarily setting __rethrow_casa_exceptions
        sjran=None
        try:
            myf = sys._getframe(len(inspect.stack()) - 1).f_globals
            original_rethrow_setting=myf.get('__rethrow_casa_exceptions',False)
            myf['__rethrow_casa_exceptions']=True
            print "\nRunning setjy with a non-existant vis"
            sjran = setjy(vis=self.inpms,listmodels=False)
        except Exception, setjyUTerr:
            msg = setjyUTerr.message
            self.assertNotEqual(msg.find("%s does not exist" % self.inpms), -1,
                                'wrong type of exception is thrown')
        finally:
            # put back original rethrow setting
            myf['__rethrow_casa_exceptions']=original_rethrow_setting
        self.assertEqual(sjran,None,"Failed to raise exception.") 
     
    def test_listmodels(self):
        """ Test listmodels mode """
        self.inpms=''
        print "\nRunning setjy in listmodels mode ...."
        sjran = setjy(vis=self.inpms,listmodels=True)
        self.assertTrue(sjran)


class test_conesearch(SetjyUnitTestBase):
    """Test search for field match by position (standard='Perley-Butler 2013')"""

    def setUp(self):
        prefix = 'n1333_nonstdcalname' 
        if testmms:
            msname=prefix+'.mms'
        else:
            msname=prefix+'.ms'
        #self.setUpMS('unittest/setjy/n1333_nonstdcalname.ms')
        self.setUpMS(msname)
        self.field = 'myfcalsrc'
        self.modelim = '3C147_U.im'
        self.result = {}

    def tearDown(self):
        self.resetMS()
 
    def test_searchByPosition(self): 
        sjran = setjy(vis=self.inpms, 
                      field=self.field,
                      modimage=self.modelim,
                      scalebychan=False,
                      #standard='Perley-Taylor 99',
                      standard='Perley-Butler 2013',
                      usescratch=True
                      )
        ret = True
        if type(sjran)!=dict:
            ret = False
        else: 
            outfldid = ""
            for ky in sjran.keys():
                if sjran[ky].has_key('fieldName') and sjran[ky]['fieldName']==self.field:
                    outfldid = ky
                    break 
            ret = len(outfldid)
            if not ret:
                print "FAIL: missing field = %s in the returned dictionary" % self.field 
        self.check_eq(sjran['12']['1']['fluxd'][0],0.99125797,0.0001)
        self.assertTrue(ret)

class test_fluxscaleStandard(SetjyUnitTestBase):
    """Test standard="fluxscale" mode"""

    def setUp(self):
        prefix = 'ngc5921' 
        if testmms:
            msname=prefix+'.mms'
        else:
            msname=prefix+'.ms'
        self.setUpMS(msname) 
        self.field = 'myfcalsrc'
        self.modelim = '3C147_U.im'
        self.result = {}

    def tearDown(self):
        self.resetMS()

    def test_fluxscaleStandard1(self):
        """ Test for accepting input fluxscale dictionary """
        import numpy as np
        fluxscaledict=\
        {'1': {'0': {'fluxd': np.array([ 2.48362403,  0.        ,  0.        ,  0.        ]),
             'fluxdErr': np.array([ 0.00215907,  0.        ,  0.        ,  0.        ]),
             'numSol': np.array([ 54.,   0.,   0.,   0.])},
       'fieldName': '1445+09900002_0',
       'fitFluxd': 0.0,
       'fitFluxdErr': 0.0,
       'fitRefFreq': 0.0,
       'spidx': np.array([ 0.,  0.,  0.]),
       'spidxerr': np.array([ 0.,  0.,  0.])},
       'freq':np. array([  1.41266507e+09]),
       'spwID': np.array([0], dtype=np.int32),
       'spwName': np.array(['none'], dtype='|S5')}

        sjran = setjy(vis=self.inpms,
                      standard='fluxscale',
                      fluxdict=fluxscaledict,
                      usescratch=False
                      )
        ret = True
        if type(sjran)!=dict:
            ret = False
        else:
            outfldid = ""
            for ky in sjran.keys():
                if sjran[ky].has_key('fieldName') and sjran[ky]['fieldName']==fluxscaledict['1']['fieldName']:
                    outfldid = ky
                    break
            ret = len(outfldid)
            if not ret:
                print "FAIL: missing field = %s in the returned dictionary" % self.field
        self.check_eq(sjran['1']['0']['fluxd'][0],2.48362403,0.0001)
        self.assertTrue(ret)

class test_setpol(SetjyUnitTestBase):
    """Test multi-term spix and polarization parameter setting"""

    def setUp(self):
        prefix = '3c391calonly'
        if testmms:
            msfile = prefix + '.mms'
        else:
            msfile = prefix + '.ms'
        #self.setUpMS('unittest/setjy/3c391calonly.ms')
        self.setUpMS(msfile)
        self.result = {}

    def tearDown(self):
        self.resetMS()

    def test_setpol1(self):
        """ Test for multi-term spix (alpha and beta) """

        sjran = setjy(vis=self.inpms,
                      standard='manual',
                      field = 'J1331+3030',
                      fluxdensity = [7.81694, 0.355789, 0.79909, 0],
                      spix = [-0.62,-0.1], 
                      reffreq='4536.0MHz',
                      usescratch=True)
        ret = True
        if type(sjran)!=dict:
            ret = False
        #else:
        #    print sjran 
        #print "fluxdic=",sjran 
 
        self.check_eq(sjran['0']['0']['fluxd'][0],7.81694, 0.0001)
        self.assertTrue(ret)

        # expected flux
        #fref = 4.536e9
        #logflx = log10(7.81694) + (-0.62)*log10(f/fref) + (-0.1)*log10(f/fref)
        # fmin at last chan (Freq=4662000000.0Hz)
        fexpmin = 7.68502
        ms.open(self.inpms)
        retrec = ms.statistics(field='0', baseline='1&2', correlation='rr', column='model', complex_value='amp')
        ms.close()
        self.check_eq(retrec['MODEL']['min'],fexpmin,0.0001)

    def test_setpol2(self):
        """ Test for constant polindex and polangle with I flux density  """

        sjran = setjy(vis=self.inpms,
                      standard='manual',
                      field = 'J1331+3030',
                      fluxdensity = [7.81694, 0, 0, 0],
                      spix = [-0.62],
                      reffreq='4536.0MHz',
                      usescratch=True)
        ret = True
        if type(sjran)!=dict:
            ret = False
        #else:
        #    print sjran

        self.check_eq(sjran['0']['0']['fluxd'][0],7.81694, 0.0001)
        self.assertTrue(ret)

        # expected flux
        #fref = 4.536e9
        #logflx = log10(7.81694) + (-0.62)*log10(f/fref) + (-0.1)*log10(f/fref)
        # fmin at last chan (Freq=4662000000.0Hz)
        fexpmin = 7.68527
        ms.open(self.inpms)
        retrec = ms.statistics(field='0', baseline='1&2', correlation='rr', column='model', complex_value='amp')
        #retrec2 = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='phase')
        ms.close()
        self.check_eq(retrec['MODEL']['min'],fexpmin,0.0001)

    def test_setpol3(self):
        """ Test for frequency-dependent polindex (2 terms)   """
        # the constant terms (polindex[0] and polangle[0] is ignored..
        pang = 0.5*66.*numpy.pi/180
        sjran = setjy(vis=self.inpms,
                      standard='manual',
                      field = 'J1331+3030',
                      fluxdensity = [7.81694, 0.355789, 0.79909, 0],
                      spix = [-0.62],
                      polindex=[0,-0.5],
                      polangle=[pang],
                      reffreq='4536.0MHz',
                      usescratch=True)
        ret = True
        if type(sjran)!=dict:
            ret = False
        #else:
        #    print sjran

        self.check_eq(sjran['0']['0']['fluxd'][0],7.81694, 0.0001)
        self.assertTrue(ret)

        # expected flux
        #fref = 4.536e9
        # I flux
        #logflx = log10(7.81694) + (-0.62)*log10(f/fref) + (-0.1)*log10(f/fref)
        # min fluxes  at last chan (Freq=4662000000.0Hz)
        ifexpmin = 7.68527
        ms.open(self.inpms)
        retrecI = ms.statistics(field='0', baseline='1&2', correlation='rr', column='model', complex_value='amp')
        # Q flux
        # polindex0 = 0.11190024, polindex = polindex0 - 0.5*(f-fref)/fref  (f-fref)/fref = 0.027778
        # => poindex_min = 0.09801124, with ifexpmin + pang constant => Qmin = 0.306371465
        # Umin = sqrt(I^2*polindex - Q^2)
        qfexpmin = 0.306371 
        ufexpmin = 0.688121784
        retrecQ = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='real') 
        retrecU = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='imaginary') 
        #print "retrecQ=",retrecQ['MODEL']['min']
        #print "retrecU=",retrecU['MODEL']['min']
        ms.close()
        self.check_eq(retrecI['MODEL']['min'],ifexpmin,0.0001)
        self.check_eq(retrecQ['MODEL']['min'],qfexpmin,0.0001)
        self.check_eq(retrecU['MODEL']['min'],ufexpmin,0.0001)

    def test_setpol4(self):
        """ Test for frequency-dependent polangle (2 terms)   """
        # the constant terms (polindex[0] and polangle[0] is ignored..
        pang = 0.5*66.*numpy.pi/180
        sjran = setjy(vis=self.inpms,
                      standard='manual',
                      field = 'J1331+3030',
                      fluxdensity = [7.81694, 0.355789, 0.79909, 0],
                      spix = [-0.62],
                      polindex=[0],
                      polangle=[pang,-0.5],
                      reffreq='4536.0MHz',
                      usescratch=True)
        ret = True
        if type(sjran)!=dict:
            ret = False
        #else:
        #    print sjran

        self.check_eq(sjran['0']['0']['fluxd'][0],7.81694, 0.0001)
        self.assertTrue(ret)

        # expected flux
        #fref = 4.536e9
        # I flux
        #logflx = log10(7.81694) + (-0.62)*log10(f/fref) + (-0.1)*log10(f/fref)
        # min fluxes  at last chan (Freq=4662000000.0Hz)
        ifexpmin = 7.68527
        ms.open(self.inpms)
        retrecI = ms.statistics(field='0', baseline='1&2', correlation='rr', column='model', complex_value='amp')
        # U flux
        # polindex0 = 0.11190024, 
        # polangle0 = 0.5759586531581288, polangle = polangle0 - 0.5*(f-fref)/fref  (f-fref)/fref = 0.027778
        # => poangle_min = 0.562069653158, with ifexpmin + polindex constant => Qmax = 0.37147241999237574 
        # Umin = sqrt(I^2*polindex^2 - Q^2)
        qfexpmax = 0.371472 
        ufexpmin = 0.775616 
        retrecQ = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='real')
        retrecU = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='imaginary')
        #print "retrecQ=",retrecQ['MODEL']['min']
        #print "retrecU=",retrecU['MODEL']['min']
        ms.close()
        self.check_eq(retrecI['MODEL']['min'],ifexpmin,0.0001)
        self.check_eq(retrecQ['MODEL']['max'],qfexpmax,0.0001)
        self.check_eq(retrecU['MODEL']['min'],ufexpmin,0.0001)

    def testr5(self):
        """ Test for rotation measure (with constant polindex and polangle) """
        # the constant terms (polindex[0] and polangle[0] is ignored..
        pang = 0.5*66.*numpy.pi/180
        sjran = setjy(vis=self.inpms,
                      standard='manual',
                      field = 'J1331+3030',
                      fluxdensity = [7.81694, 0.355789, 0.79909, 0],
                      spix = [-0.62],
                      polindex=[0],
                      polangle=[pang],
                      rotmeas=10.0,
                      reffreq='4536.0MHz',
                      usescratch=True)
        ret = True
        if type(sjran)!=dict:
            ret = False
        #else:
        #    print sjran

        self.check_eq(sjran['0']['0']['fluxd'][0],7.81694, 0.0001)
        self.assertTrue(ret)

        # expected flux
        #fref = 4.536e9
        # I flux
        #logflx = log10(7.81694) + (-0.62)*log10(f/fref)
        # min fluxes  at last chan (Freq=4662000000.0Hz)
        ifexpmin = 7.68527
        ms.open(self.inpms)
        retrecI = ms.statistics(field='0', baseline='1&2', correlation='rr', column='model', complex_value='amp')
        # U flux
        # polindex = 0.11190024,
        # polangle = 0.57595865
        # rotmeas = 10.0 => angle = 2*rotmeas*c^2*(fref^2-f^2)/ (f^2*f0^2) 
        qfexpend = 0.353443
        ufexpend = 0.783996
        retrecQ = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='real')
        retrecU = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='imaginary')
        #print "retrecQ=",retrecQ['MODEL']['min']
        #print "retrecU=",retrecU['MODEL']['min']
        ms.close()
        self.check_eq(retrecI['MODEL']['min'],ifexpmin,0.0001)
        self.check_eq(retrecQ['MODEL']['min'],qfexpend,0.0001)
        self.check_eq(retrecU['MODEL']['min'],ufexpend,0.0001)

    def testr6(self):
        """ Test for spectral index with curvature and frequnecy-dependent polindex and polangle with rotmeas """
        # the constant terms (polindex[0] and polangle[0] is ignored..
        pang = 0.5*66.*numpy.pi/180
        sjran = setjy(vis=self.inpms,
                      standard='manual',
                      field = 'J1331+3030',
                      fluxdensity = [7.81694, 0.355789, 0.79909, 0],
                      spix = [-0.62, -0.1],
                      polindex=[0, -0.5, 0.1],
                      polangle=[pang, -0.5],
                      rotmeas=10.0,
                      reffreq='4536.0MHz',
                      usescratch=True)
        ret = True
        if type(sjran)!=dict:
            ret = False
        #else:
        #    print sjran

        self.check_eq(sjran['0']['0']['fluxd'][0],7.81694, 0.0001)
        self.assertTrue(ret)

        # expected flux
        #fref = 4.536e9
        # I flux
        #logflx = log10(7.81694) + (-0.62)*log10(f/fref) + (-0.1)*log10(f/fref)^2
        # min fluxes  at last chan (Freq=4662000000.0Hz)
        ifexpmin = 7.6850217
        ms.open(self.inpms)
        retrecI = ms.statistics(field='0', baseline='1&2', correlation='rr', column='model', complex_value='amp')
        # U flux - based on python script calculation
        # polindex0 = 0.11190024, polindex= polindex0 +(-0.5)*(f-fref)/fref +(-0.1)*((f-fref)/fref)^2
        # polindex(f=fmax) = 
        # polangle0 = 0.57595865
        # rotmeas = 10.0 => angle = 2*rotmeas*c^2*(fref^2-f^2)/ (f^2*f0^2)
        qfexpmin = 0.328774
        ufexpmin = 0.678335
        anglemin = 1.119481
        retrecQ = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='real')
        retrecU = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='imaginary')
        retrecAngle = ms.statistics(field='0', baseline='1&2', correlation='rl', column='model', complex_value='phase')
        #print "retrecQ=",retrecQ['MODEL']['min']
        #print "retrecU=",retrecU['MODEL']['min']
        ms.close()
        self.check_eq(retrecI['MODEL']['min'],ifexpmin,0.0001)
        self.check_eq(retrecQ['MODEL']['min'],qfexpmin,0.0001)
        self.check_eq(retrecU['MODEL']['min'],ufexpmin,0.0001)
        self.check_eq(retrecAngle['MODEL']['min'],anglemin,0.0001)

def suite():
    return [test_SingleObservation,test_MultipleObservations,test_ModImage, test_inputs, test_conesearch, test_fluxscaleStandard, test_setpol]
