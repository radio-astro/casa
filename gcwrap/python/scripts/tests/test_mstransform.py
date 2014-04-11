import shutil
import unittest
import os
import numpy
import exceptions
from tasks import *
from taskinit import mstool, tbtool, msmdtool, aftool
from __main__ import default
import testhelper as th
from recipes.listshapes import listshapes
from parallel.parallel_task_helper import ParallelTaskHelper
from unittest.case import expectedFailure


# Define the root for the data files
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

aflocal = aftool()

def check_eq(val, expval, tol=None):
    """Checks that val matches expval within tol."""
#    print val
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

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):

    def copyfile(self,file):

        if os.path.exists(file):
           os.system('rm -rf '+ file)

        os.system('cp -RL '+datapath + file +' '+ file)

    def setUp_ngc5921(self):
        # data set with spw=0, 63 channels in LSRK
        self.vis = "ngc5921.ms"

        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_4ants(self):
        # data set with spw=0~15, 64 channels each in TOPO
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_jupiter(self):
        # data col, spw=0,1 1 channel each, TOPO, field=0~12, 93 scans
        self.vis = 'jupiter6cm.demo-thinned.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_g19(self):
        # data with spw=0~23 128 channel each in LSRK, field=0,1
        self.vis = 'g19_d2usb_targets_line-shortened-thinned.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_CAS_5076(self):

        self.vis = 'CAS-5076.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_almasim(self):

        self.vis = 'sim.alma.cycle0.compact.noisy.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_floatcol(self):
        datapath = os.environ.get('CASAPATH').split()[0] + \
                    "/data/regression/unittest/flagdata/"

        # 15 rows, 3 scans, 9 spw, mixed chans, XX,YY, FLOAT_DATA col
        self.vis = 'SDFloatColumn.ms'
        if os.path.exists(self.vis):
            self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_3c84(self):
        # MS is as follows (scan=1):
        #  SpwID   #Chans   Corrs
        #   0      256      RR
        #   0      256      LL
        #   1      128      RR  LL
        #   2      64       RR  RL  LR  LL

        self.vis = '3c84scan1.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_CAS_5013(self):

        self.vis = 'ALMA-data-mst-science-testing-CAS-5013-one-baseline-one-timestamp.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_CAS_4850(self):

        self.vis = 'CAS-4850-30s-limit-ALMA.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_CAS_4983(self):

        self.vis = 'CAS-4983.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)       
        
    def setUp_CAS_5172(self):

        self.vis = 'CAS-5172-phase-center.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)  

    def cleanup(self):
        os.system('rm -rf '+ self.vis)


class test_base_compare(test_base):

    def setUp(self):

        self.outvis = ''
        self.refvis = ''
        self.outvis_sorted = ''
        self.refvis_sorted = ''

        self.subtables=['/ANTENNA','/DATA_DESCRIPTION','/FEED','/FIELD','/FLAG_CMD',
                        '/POINTING','/POLARIZATION','/PROCESSOR','/STATE']
        self.sortorder=['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME']

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outvis)
        os.system('rm -rf '+ self.refvis)
        os.system('rm -rf '+ self.outvis_sorted)
        os.system('rm -rf '+ self.refvis_sorted)

    def sort(self):
        myms = mstool()

        myms.open(self.outvis)
        myms.sort(self.outvis_sorted,self.sortorder)
        myms.done()

        myms.open(self.refvis)
        myms.sort(self.refvis_sorted,self.sortorder)
        myms.done()

    def generate_tolerance_map(self):

        # Get column names
        mytb = tbtool()
        mytb.open(self.refvis)
        self.columns = mytb.colnames()
        mytb.close()

        # Define default tolerance
        self.mode={}
        self.tolerance={}
        for col in self.columns:
            self.mode[col] = "absolute"
            self.tolerance[col] = 1E-6

    def compare_subtables(self):
        for subtable in self.subtables:
            self.assertTrue(th.compTables(self.outvis_sorted+subtable,self.refvis_sorted+subtable, [],0.000001,"absolute"))

        # Special case for SOURCE which contains many un-defined columns
        # CAS-5172 (jagonzal): Commenting this out because cvel and mstransform produce different SORUCE subtable
        # For some reason cvel removes sources which are not present in any row of the main table even if the
        # user does not specify field selection
        #self.assertTrue(th.compTables(self.outvis_sorted+'/SOURCE',self.refvis_sorted+'/SOURCE', 
        #                              ['POSITION','TRANSITION','REST_FREQUENCY','SYSVEL','SOURCE_MODEL'],0.000001,"absolute"))

        # Special case for OBSERVATION which contains many un-defined columns
        self.assertTrue(th.compTables(self.outvis_sorted+'/OBSERVATION',self.refvis_sorted+'/OBSERVATION',
                                      ['LOG','SCHEDULE'],0.000001,"absolute"))

    def compare_main_table_columns(self,startrow = 0, nrow = -1, rowincr = 1):
        for col in self.columns:
            if col != "WEIGHT_SPECTRUM" and col != "SIGMA" and col != "FLAG_CATEGORY":
                    tmpcolumn = self.columns[:]
                    tmpcolumn.remove(col)
                    self.assertTrue(th.compTables(self.refvis_sorted,self.outvis_sorted,tmpcolumn,self.tolerance[col],self.mode[col],startrow,nrow,rowincr))

    def post_process(self,startrow = 0, nrow = -1, rowincr = 1):

        # Sort the output MSs so that they can be compared
        self.sort()

        # Compare results for subtables
        self.compare_subtables()

        # Compare columns from main table
        self.compare_main_table_columns(startrow,nrow,rowincr)


class test_Combspw1(test_base):
    ''' Tests for combinespws parameter'''

    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        os.system('rm -rf inpmms*.*ms combcvel*ms list.obs')

    def test_combspw1_1(self):
        '''mstransform: Combine four spws into one'''

        self.outputms = "combspw11.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=True, spw='0~3')
        self.assertTrue(os.path.exists(self.outputms))

        ret = th.verifyMS(self.outputms, 1, 256, 0)
        self.assertTrue(ret[0],ret[1])

        listobs(self.outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')

    def test_combspw1_2(self):
        '''mstransform: Combine some channels of two spws'''

        self.outputms = "combspw12.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=True, spw='0:60~63,1:60~63')
        self.assertTrue(os.path.exists(self.outputms))

        # The spws contain gaps, therefore the number of channels is bigger
        ret = th.verifyMS(self.outputms, 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

        # Compare with cvel results
        default(cvel)
        cvel(vis=self.vis, outputvis='combcvel12.ms', spw='0:60~63,1:60~63')
        ret = th.verifyMS('combcvel12.ms', 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw1_3(self):
        '''mstransform: Do not combine spws and create MMS with axis scan.'''
        self.setUp_jupiter()
        self.outputms = 'combspw13.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=False, spw='0,1',field = '12',
             datacolumn='DATA', createmms=True, separationaxis='scan')

        self.assertTrue(os.path.exists(self.outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=self.outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')

        ret = th.verifyMS(self.outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw1_4(self):
        '''mstransform: Combine some channels of two spws using MMS input'''
        # same test as test_combspw1_2
        mmsfile = "inpmms14.mms"
        # First create an MMS
        mstransform(vis=self.vis, outputvis=mmsfile, createmms=True)

        # Now do the same as in test_combspw1_2. Datacolumn moved to DATA
        self.outputms = "combspw14.ms"
        mstransform(vis=mmsfile, outputvis=self.outputms, combinespws=True, spw='0:60~63,1:60~63',
                    datacolumn='data')
        self.assertTrue(os.path.exists(self.outputms))

        # The spws contain gaps, therefore the number of channels is bigger
        ret = th.verifyMS(self.outputms, 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

        # Compare with cvel results
        default(cvel)
        cvel(vis=self.vis, outputvis='combcvel14.ms', spw='0:60~63,1:60~63')
        ret = th.verifyMS('combcvel14.ms', 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw1_5(self):
        '''mstransform: Combine four spws into one'''

        self.outputms = "combspw15.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=True, spw='2,5,8')
        self.assertTrue(os.path.exists(self.outputms))

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 1, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')

        # DDI subtable should have 4 rows with the proper indices
        mytb = tbtool()
        mytb.open(self.outputms + '/DATA_DESCRIPTION')
        spwCol = mytb.getcol('SPECTRAL_WINDOW_ID')
        mytb.close()
        nspw = spwCol.size
        check_eq(nspw, 1)
        check_eq(spwCol[0], 0)

    def test_combspw1_6(self):
        '''mstransform: Combine some channels of two spws'''

        self.outputms = "combspw16.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=True, spw='0~1:60~63')
        self.assertTrue(os.path.exists(self.outputms))

        # The spws contain gaps, therefore the number of channels is bigger
        ret = th.verifyMS(self.outputms, 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

        # Compare with cvel results
        default(cvel)
        cvel(vis=self.vis, outputvis='combcvel12.ms', spw='0~1:60~63')
        ret = th.verifyMS('combcvel12.ms', 1, 68, 0)
        self.assertTrue(ret[0],ret[1])


class test_Regridms1(test_base):
    '''Tests for regridms parameter using Four_ants_3C286.ms'''

    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        os.system('rm -rf testmms*ms list.obs')

    def test_regrid1_1(self):
        '''mstransform: Default of regridms parameters'''

        self.outputms = "reg11.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, regridms=True)
        self.assertTrue(os.path.exists(self.outputms))

        # The regriding should be the same as the input
        for i in range(16):
            ret = th.verifyMS(self.outputms, 16, 64, i)
            self.assertTrue(ret[0],ret[1])

        listobs(self.outputms)
        listobs(self.outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')

    def test_regrid1_2(self):
        '''mstransform: Default regridms with spw selection'''

        self.outputms = "reg12.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, regridms=True, spw='1,3,5,7')
        self.assertTrue(os.path.exists(self.outputms))

        # The output should be the same as the input
        for i in range(4):
            ret = th.verifyMS(self.outputms, 4, 64, i)
            self.assertTrue(ret[0],ret[1])

        listobs(self.outputms)

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 4, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r4'][0], 3,'Error re-indexing DATA_DESCRIPTION table')

    def test_regrid1_3(self):
        '''mstransform: Default regridms with spw selection using input MMS'''
        # same as test_regrid1_1
        mmsfile = 'testmms13.mms'
        # Create input MMS
        mstransform(vis=self.vis, outputvis=mmsfile, createmms=True, parallel=False,
                    separationaxis='scan')

        self.outputms = "reg13.ms"
        mstransform(vis=mmsfile, outputvis=self.outputms, regridms=True, spw='1,3,5,7',
                    datacolumn='DATA')
        self.assertTrue(os.path.exists(self.outputms))

        # The regriding should be the same as the input
        for i in range(4):
            ret = th.verifyMS(self.outputms, 4, 64, i)
            self.assertTrue(ret[0],ret[1])

        listobs(self.outputms)

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 4, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r4'][0], 3,'Error re-indexing DATA_DESCRIPTION table')


class test_Regridms3(test_base):
    '''Tests for regridms parameter using Jupiter MS'''

    def setUp(self):
        self.setUp_jupiter()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        os.system('rm -rf cvel31*.*ms')

    def test_regrid3_1(self):
        '''mstransform 12: Check that output columns are the same when using mstransform'''
        self.outputms = 'reg31.ms'

        mstransform(vis=self.vis, outputvis=self.outputms, field='6',
                    combinespws=True, regridms=True, datacolumn='data',
                    mode='frequency', nchan=2, start='4.8101 GHz', width='50 MHz',
                    outframe='')

        ret = th.verifyMS(self.outputms, 1, 2, 0)
        self.assertTrue(ret[0],ret[1])

        # Now run with cvel to compare the columns, CAS-4866
        outputms = 'cvel31.ms'
        cvel(vis=self.vis, outputvis=outputms, field='6',
            passall=False,mode='frequency',nchan=2,start='4.8101 GHz',
            width='50 MHz',outframe='')

        # Sort the output MSs so that they can be compared
        myms = mstool()

        myms.open('cvel31.ms')
        myms.sort('cvel31-sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()

        myms.open('reg31.ms')
        myms.sort('reg31-sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()

        self.assertTrue(th.compTables('cvel31-sorted.ms','reg31-sorted.ms', 'FLAG_CATEGORY',0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/ANTENNA','reg31-sorted.ms/ANTENNA', [],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/DATA_DESCRIPTION','reg31-sorted.ms/DATA_DESCRIPTION', [],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/FEED','reg31-sorted.ms/FEED', ['SPECTRAL_WINDOW_ID'],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/FIELD','reg31-sorted.ms/FIELD', [],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/FLAG_CMD','reg31-sorted.ms/FLAG_CMD', [],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/OBSERVATION','reg31-sorted.ms/OBSERVATION', ['LOG','SCHEDULE'],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/POINTING','reg31-sorted.ms/POINTING', [],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/POLARIZATION','reg31-sorted.ms/POLARIZATION', [],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/PROCESSOR','reg31-sorted.ms/PROCESSOR', [],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/SOURCE','reg31-sorted.ms/SOURCE', [],0.000001,"absolute"))
        self.assertTrue(th.compTables('cvel31-sorted.ms/STATE','reg31-sorted.ms/STATE', [],0.000001,"absolute"))

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 1, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')

# Uncomment after seg fault is fixed
    @unittest.skip('Skip until seg fault in InterpolateArray1D.tcc is fixed.')
    def test_regrid3_2(self):
        '''mstransform: Combine spw and regrid MS with two spws, select one field and 2 spws'''
        # cvel: test8
        self.outputms = "reg32a.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=True, regridms=True,
                    spw='0,1',field = '11',nchan=1, width=2, datacolumn='DATA')
        self.assertTrue(os.path.exists(self.outputms))

        ret = th.verifyMS(self.outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])

        # Now, do only the regridding and do not combine spws
        outputms = "reg32b.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True,
                    spw='0,1',field = '11',nchan=1, width=2, datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))

        ret = th.verifyMS(outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(outputms, 2, 1, 1)
        self.assertTrue(ret[0],ret[1])


class test_Hanning(test_base):
    '''Test for hanning transformation'''
    def setUp(self):
        self.setUp_ngc5921()

    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)

    def test_hanning1(self):
        '''mstransform: Apply Hanning smoothing in MS with 24 spws. Do not combine spws.'''
        self.setUp_g19()
        self.outputms = "hann1.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=False, hanning=True,
                    datacolumn='data')

        self.assertTrue(os.path.exists(self.outputms))
        ret = th.verifyMS(self.outputms, 24, 128, 0)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 24, 128, 2)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 24, 128, 15)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 24, 128, 18)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 24, 128, 23)
        self.assertTrue(ret[0],ret[1])

    def test_hanning2(self):
        '''mstransform: Apply Hanning smoothing and combine spw=1,2,3.'''
        self.setUp_g19()
        self.outputms = "hann2.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=True, hanning=True,
                    spw='1,2,3', datacolumn='data')

        self.assertTrue(os.path.exists(self.outputms))
        ret = th.verifyMS(self.outputms, 1, 1448, 0)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 1, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')

    def test_hanning3(self):
        '''mstransform: Hanning theoretical and calculated values should be the same'''
        # hanning: test4
        self.outputms = "hann3.ms"

        # The hanningsmooth task flags the first and last channels. Check it!
        # Before running the task
        flag_col = th.getVarCol(self.vis, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        # Get the DATA column before the transformation
        data_col = th.getVarCol(self.vis, 'DATA')

        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', hanning=True)

        # After running the task
        flag_col = th.getVarCol(self.outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

        corr_col = th.getVarCol(self.outputms, 'DATA')
        nrows = len(corr_col)

      # Loop over every 2nd row,pol and get the data for each channel
        max = 1e-05
        for i in range(1,nrows,2) :
            row = 'r%s'%i
            # polarization is 0-1
            for pol in range(0,2) :
                # array's channels is 0-63
                for chan in range(1,62) :
                    # channels must start from second and end before the last
                    data = data_col[row][pol][chan]
                    dataB = data_col[row][pol][chan-1]
                    dataA = data_col[row][pol][chan+1]

                    Smoothed = th.calculateHanning(dataB,data,dataA)
                    CorData = corr_col[row][pol][chan]

                    # Check the difference
                    self.assertTrue(abs(CorData-Smoothed) < max )

    def test_hanning4(self):
        '''mstransform: Flagging should be correct after hanning smoothing and frame transformation.'''
        # hanning: test8
#        clearcal(vis=self.vis)
        self.outputms = "hann4.ms"

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.vis, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        mstransform(vis=self.vis, outputvis=self.outputms, hanning=True, regridms=True,
                        outframe='cmb',datacolumn='data')

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][2] == [False])
        self.assertTrue(flag_col['r1'][0][60] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [True])
        self.assertTrue(flag_col['r1'][0][62] == [True])


class test_FreqAvg(test_base):
    '''Tests for frequency averaging'''
    def setUp(self):
        self.setUp_g19()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)

    def test_freqavg1(self):
        '''mstranform: Average 20 channels of one spw'''
        self.outputms = "favg1.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='2', chanaverage=True, chanbin=20)

        self.assertTrue(os.path.exists(self.outputms))
        ret = th.verifyMS(self.outputms, 1, 6, 0)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 1, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg2(self):
        '''mstranform: Select a few channels to average from one spw'''
        self.outputms = "favg2.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='2:10~20', chanaverage=True, chanbin=2)

        self.assertTrue(os.path.exists(self.outputms))
        ret = th.verifyMS(self.outputms, 1, 5, 0)
        self.assertTrue(ret[0],ret[1])

    def test_freqavg3(self):
        '''mstranform: Average all channels of one spw'''
        self.outputms = "favg3.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='23', chanaverage=True, chanbin=128)

        self.assertTrue(os.path.exists(self.outputms))
        ret = th.verifyMS(self.outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])

    def test_freqavg4(self):
        '''mstranform: Average using different bins for several spws'''
        self.outputms = "favg4.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='10,12,20', chanaverage=True,
                    chanbin=[128,4,10])

        self.assertTrue(os.path.exists(self.outputms))

        # Output should be:
        # spw=0 1 channel
        # spw=1 32 channels
        # spw=3 13 channels
        ret = th.verifyMS(self.outputms, 3, 1, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 32, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 12, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg5(self):
        '''mstranform: Different number of spws and chanbin. Expected error'''
        self.outputms = "favg5.ms"
        ret = mstransform(vis=self.vis, outputvis=self.outputms, spw='2,10', chanaverage=True,
                    chanbin=[10,20,4])

        self.assertFalse(ret)

    def test_freqavg6(self):
        '''mstranform: Average all channels of one spw, save as an MMS'''
        # same as test_freqavg3
        self.outputms = "favg6.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='23', chanaverage=True, chanbin=128,
                    createmms=True)

        self.assertTrue(os.path.exists(self.outputms))
        ret = th.verifyMS(self.outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])

    def test_freqavg7(self):
        '''mstranform: Average using different bins for several spws, output MMS'''
        # same as test_freqavg4
        self.outputms = "favg7.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='10,12,20', chanaverage=True,
                    chanbin=[128,4,10], createmms=True, separationaxis='scan')

        self.assertTrue(os.path.exists(self.outputms))

        # Output should be:
        # spw=0 1 channel
        # spw=1 32 channels
        # spw=3 13 channels
        ret = th.verifyMS(self.outputms, 3, 1, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 32, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 12, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg8(self):
        '''mstranform: Average using different bins for several spws, output MMS'''
        # same as test_freqavg4
        self.outputms = "favg8.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='10,12,20', chanaverage=True,
                    chanbin=[128,4,10], createmms=True, separationaxis='spw',numsubms=2)

        self.assertTrue(os.path.exists(self.outputms))

        # Output should be:
        # spw=0 1 channel
        # spw=1 32 channels
        # spw=3 13 channels
        ret = th.verifyMS(self.outputms, 3, 1, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 32, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 12, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_freqavg9(self):
        '''mstranform: Average using different bins and a channel selection, output MMS'''
        self.outputms = "favg9.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='2,12,10:1~10', chanaverage=True,
                    chanbin=[32,128,5], createmms=True, separationaxis='spw')

        self.assertTrue(os.path.exists(self.outputms))

        # Output should be:
        # spw=0 4 channels
        # spw=1 1 channel
        # spw=2 2 channels
        ret = th.verifyMS(self.outputms, 3, 4, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 1, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 2, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg10(self):
        '''mstranform: Average using different bins, channel selection, both axes, output MMS'''
        self.outputms = "favg10.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='2,12,10:1~10', chanaverage=True,
                    chanbin=[32,128,5], createmms=True, separationaxis='both')

        self.assertTrue(os.path.exists(self.outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=self.outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')

        # Output should be:
        # spw=0 4 channels
        # spw=1 1 channel
        # spw=2 2 channels
        ret = th.verifyMS(self.outputms, 3, 4, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 1, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 2, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')


class test_Shape(test_base):
    '''Test the tileshape parameter'''
    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)

    def test_shape1(self):
        '''mstransform: default tileshape'''
        self.outputms = "shape1.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, createmms=False, tileshape=[0])

        self.assertTrue(os.path.exists(self.outputms))

        # Get the tile shape in input
        tblocal = tbtool()
        tblocal.open(self.vis)
        inpdm = tblocal.getdminfo()
        tblocal.close()
        inptsh = th.getTileShape(inpdm)

        # Get the tile shape for the output
        tblocal.open(self.outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)

        # Compare both
        self.assertTrue((inptsh==outtsh).all(), 'Tile shapes are different')

    def test_shape2(self):
        '''mstransform: custom tileshape'''
        self.outputms = "shape2.ms"
        inptsh = [4,20,1024]
        mstransform(vis=self.vis, outputvis=self.outputms, createmms=False, tileshape=inptsh)

        self.assertTrue(os.path.exists(self.outputms))

        # Check the tile shape for the output
        tblocal = tbtool()
        tblocal.open(self.outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)

        self.assertTrue((inptsh==outtsh).all(), 'Tile shapes are different')

    def test_shape3(self):
        '''mstransform: DATA and FLAG tileshapes should be the same'''
        self.outputms = "shape3.ms"
        inptsh = [4,10,1024]
        mstransform(vis=self.vis, outputvis=self.outputms, createmms=False, tileshape=inptsh)

        self.assertTrue(os.path.exists(self.outputms))

        # Get the tile shape for the DATA output
        tblocal = tbtool()
        tblocal.open(self.outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)
        # And for the FLAG column
        flagtsh = th.getTileShape(outdm, 'FLAG')

        self.assertTrue((outtsh==flagtsh).all(), 'Tile shapes are different')


class test_Columns(test_base):
    '''Test different datacolumns'''

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        os.system('rm -rf ngc5921Jy.ms')
        
    def test_col1(self):
          """mstransform: try to make real a non-existing virtual MODEL column"""
          self.setUp_ngc5921()
          self.outputms = "col1.ms"
          mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='all', realmodelcol=True)

          self.assertTrue(os.path.exists(self.outputms))
          mcol = th.getColDesc(self.outputms, 'MODEL_DATA')
          mkeys = mcol.keys()
          self.assertTrue(mkeys.__len__()==0, 'Should not add MODEL_DATA column')
          
    def test_col2(self):
          """mstransform: make real a virtual MODEL column """
          self.setUp_ngc5921()
          self.outputms = "col2.ms"
          inpms = 'ngc5921Jy.ms'
          shutil.copytree(self.vis, inpms)

          # First, run setjy to create a virtual MODEl column (SOURCE_MODEL)
          setjy(vis=inpms,field='1331+305*',modimage='',standard='Perley-Taylor 99',
                scalebychan=False, usescratch=False)

          # Verify that the virtual column exists
          mcol = th.getColDesc(inpms+'/SOURCE', 'SOURCE_MODEL')
          mkeys = mcol.keys()
          self.assertTrue(mkeys.__len__() > 0, 'Should have a SOURCE_MODEL column')

          # Make the virtual column a real MODEL_DATA column
          mstransform(vis=inpms, outputvis=self.outputms, datacolumn='all', realmodelcol=True)

          self.assertTrue(os.path.exists(self.outputms))
          mcol = th.getColDesc(self.outputms, 'MODEL_DATA')
          mkeys = mcol.keys()
          self.assertTrue(mkeys.__len__() > 0, 'Should have a MODEL_DATA column')

    def test_col3(self):
        '''mstransform: split out the MODEL column'''
        self.setUp_4ants()
        self.outputms = 'col3.ms'
        mstransform(vis=self.vis, outputvis=self.outputms,field='1',spw='0:0~61',
                    datacolumn='model')

        # Compare with split. CAS-4940
        outputms = 'split3.ms'
        split(vis=self.vis, outputvis=outputms,field='1',spw='0:0~61',
                    datacolumn='model')

        th.compVarColTables('split3.ms','col3.ms','DATA')


class test_SeparateSPWs(test_base):
    '''Test the nspw parameter to separate spws'''
    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        os.system('rm -rf list.obs')

    def test_sep1(self):
        '''mstransform: separate one spw into 4, using default regrid parameters'''
        self.outputms = "separate1.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='2', regridms=True, nspw=4)
        self.assertTrue(os.path.exists(self.outputms))

        # Should create 4 spws with 16 channels each
        ret = th.verifyMS(self.outputms, 4, 16, 0)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 4, 16, 1)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 4, 16, 2)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 4, 16, 3)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 4, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r4'][0], 3,'Error re-indexing DATA_DESCRIPTION table')

    def test_sep2(self):
        '''mstransform: separate three spws into 2, using default regrid parameters'''
        self.outputms = "separate2.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='2,3,4', regridms=True, nspw=2)
        self.assertTrue(os.path.exists(self.outputms))

        # Should create 2 spws with ?96 channels each
        ret = th.verifyMS(self.outputms, 2, 96, 0)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 96, 1)
        self.assertTrue(ret[0],ret[1])

        listobs(self.outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')

    def test_sep3(self):
        '''mstransform: separate 16 spws into 4 with 10 channels each'''
        self.outputms = "separate3.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, regridms=True, nspw=4, nchan=10)
        self.assertTrue(os.path.exists(self.outputms))

        # Should create 4 spws with 10 channels each
        ret = th.verifyMS(self.outputms, 4, 10, 0)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 4, 10, 3)
        self.assertTrue(ret[0],ret[1])

    def test_CAS_5403_1(self):
        '''mstransform: separate spw 0 into 4 spws and check that DDI subtable is reindexed properly'''
        self.outputms = "test_5403_1.ms"
        mstransform(vis=self.vis, outputvis=self.outputms,regridms=True,spw='0',nspw=4)
        self.assertTrue(os.path.exists(self.outputms))

        # DDI subtable should have 4 rows with the proper index
        mytbSPW = tbtool()
        mytbSPW.open(self.outputms + '/DATA_DESCRIPTION')
        spwCol = mytbSPW.getcol('SPECTRAL_WINDOW_ID')
        mytbSPW.close()
        nspw = spwCol.size
        check_eq(nspw, 4)
        check_eq(spwCol[0], 0)
        check_eq(spwCol[1], 1)
        check_eq(spwCol[2], 2)
        check_eq(spwCol[3], 3)
        
        # Check some values from main table
        mytbMain = tbtool()
        mytbMain.open(self.outputms)
        data = mytbMain.getcol('DATA')
        check_eq(data.shape,(4,16,4296))
        mytbMain.close()

    def test_CAS_5403_2(self):
        '''mstransform: combine spw 0,1,2 into one spw and then break it doen in 4 spws.
                        and then check that DDI subtable is reindexed properly'''
        self.outputms = "test_5403_2.ms"
        mstransform(vis=self.vis, outputvis=self.outputms,regridms=True,spw='0,1,2',nspw=4)
        self.assertTrue(os.path.exists(self.outputms))

        # DDI subtable should have 4 rows with the proper index
        mytb = tbtool()
        mytb.open(self.outputms + '/DATA_DESCRIPTION')
        spwCol = mytb.getcol('SPECTRAL_WINDOW_ID')
        mytb.close()
        nspw = spwCol.size
        check_eq(nspw, 4)
        check_eq(spwCol[0], 0)
        check_eq(spwCol[1], 1)
        check_eq(spwCol[2], 2)
        check_eq(spwCol[3], 3)

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 4, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r4'][0], 3,'Error re-indexing DATA_DESCRIPTION table')
        
    def test_slicing_problem(self):
        '''mstransform: Separate SPWs after re-gridding one single SPW'''
        self.outputms = "test_slicing_problem.ms"
        mstransform(vis=self.vis, outputvis=self.outputms,regridms=True,nspw=3,nchan=10,spw='0:0~49')
        self.assertTrue(os.path.exists(self.outputms))
        
        mytb = tbtool()
        mytb.open(self.outputms + '/SPECTRAL_WINDOW')
        numChan = mytb.getcol('NUM_CHAN')      
        mytb.close()            
        check_eq(numChan[0], 10)
        check_eq(numChan[1], 10)
        check_eq(numChan[2], 10)        


class test_MMS(test_base):
    '''Several tests that create an MMS'''
    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)

    def test_mms1(self):
        '''mstransform: create MMS with spw separation and channel selections'''
        self.outputms = "testmms1.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw')

        self.assertTrue(os.path.exists(self.outputms))

        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(self.outputms, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_mms2(self):
        '''mstransform: create MMS with spw/scan separation and channel selections'''
        self.outputms = "testmms2.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='both')

        self.assertTrue(os.path.exists(self.outputms))

        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 2, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')

    def test_mms3(self):
        '''mstransform: create MMS with scan separation and channel selections'''
        self.outputms = "testmms3.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='scan')
        self.assertTrue(os.path.exists(self.outputms))

        # It should create 2 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_mms4(self):
        '''mstransform: verify spw sub-table consolidation'''
        self.outputms = "testmms4.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='3,5:10~20,7,9,11,13,15',createmms=True,
                    separationaxis='spw')
        self.assertTrue(os.path.exists(self.outputms))

        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(self.outputms, 7, 10, 1, ignoreflags=True)
        
    def test_CAS6206(self):
        '''mstransform: verify that all columns are re-indexed in SPW sub-table'''
        self.outputmms='test.mms'
        self.outputms='assoc.ms'
        self.setUp_CAS_5013()
        mstransform(vis=self.vis, outputvis=self.outputmms,createmms=True, datacolumn='corrected')
        
        # if SPW sub-table is not correct, the next step will fail
        self.assertTrue(mstransform(vis=self.outputmms, outputvis=self.outputms, hanning=True, datacolumn='data'))
        


class test_Parallel(test_base):
    '''Run some of the same tests in parallel'''
    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)

    def test_parallel1(self):
        '''mstransform: create MMS with spw separation and channel selections in parallel'''
        self.outputms = "parallel1.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw', parallel=False)

        self.assertTrue(os.path.exists(self.outputms))

        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(self.outputms, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_parallel2(self):
        '''mstransform: create MMS with spw/scan separation and channel selections in parallel'''
        self.outputms = "parallel2.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='both', parallel=False)

        self.assertTrue(os.path.exists(self.outputms))

        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_parallel3(self):
        '''mstransform: create MMS with scan separation and channel selections in parallel'''
        self.outputms = "parallel3.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='scan', parallel=False)
        self.assertTrue(os.path.exists(self.outputms))

        # It should create 2 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_parallel4(self):
        '''mstransform: verify spw sub-table consolidation in parallel'''
        self.outputms = "parallel4.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='3,5:10~20,7,9,11,13,15',createmms=True,
                    separationaxis='spw', parallel=False)
        self.assertTrue(os.path.exists(self.outputms))

        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(self.outputms, 7, 10, 1, ignoreflags=True)

    def test_parallel5(self):
        '''mstransform: Do not combine spws and create MMS with axis scan in parallel.'''
        self.setUp_jupiter()
        self.outputms = 'parallel5.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=False, spw='0,1',field = '12',
             datacolumn='DATA', createmms=True, separationaxis='scan', parallel=False)

        self.assertTrue(os.path.exists(self.outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=self.outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')

        ret = th.verifyMS(self.outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])


class test_state(test_base):
    '''Test operation with state id'''
    def setUp(self):
        self.setUp_CAS_5076()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        
    def test_select_by_scan_intent_and_reindex_state_accordingly(self):
        '''mstransform: select a scan intent and re-index state sub-table'''
        self.outputms = "test_select_by_scan_intent_and_reindex_state_accordingly.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, intent='OBSERVE_TARGET*', datacolumn='DATA')
        self.assertTrue(os.path.exists(self.outputms))
        
        # Check that state sub-table has been re-indexed
        mytb = tbtool()
        mytb.open(self.outputms + '/STATE')
        scan_intent = mytb.getcol('OBS_MODE')
        mytb.close()
        n_subscans = scan_intent.size
        check_eq(n_subscans, 12)
        
        # listobs checks that re-indexing is consistent
        listobs(self.outputms)

    def test_select_by_scan_but_not_implicit_state_reindex(self):
        '''mstransform: select 2 scans and do not automatically re-index state sub-table'''
        self.outputms = "test_select_by_scan_but_not_implicit_state_reindex.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, scan='2,3', datacolumn='DATA')
        self.assertTrue(os.path.exists(self.outputms))
        
        # Check that state sub-table has not been re-indexed
        mytb = tbtool()
        mytb.open(self.outputms + '/STATE')
        scan_intent = mytb.getcol('OBS_MODE')
        mytb.close()
        n_subscans = scan_intent.size
        check_eq(n_subscans, 30)        

        # listobs checks that re-indexing is consistent
        listobs(self.outputms)


class test_WeightSpectrum(test_base):
    '''Test usage of WEIGHT_SPECTRUM to channel average and combine SPWs with different exposure'''

    def setUp(self):
        default(mstransform)

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outvis)

    def test_combineSPWDiffExpWithWeightSpectrum(self):
        '''mstransform: Combine SPWs with different exposure using WEIGHT_SPECTRUM'''

        self.vis = 'combine-1-timestamp-2-SPW-with-WEIGHT_SPECTRUM-Different-Exposure.ms'
        self.outvis = 'combineSPWDiffExpWithWeightSpectrum.ms'
        self.copyfile(self.vis)

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn="DATA",combinespws=True,regridms=True,
                    mode="frequency",nchan=50,start="2.20804e+10Hz",width="1.950702e+05Hz",nspw=1,
                    interpolation="fftshift",phasecenter="J2000 12h01m53.13s -18d53m09.8s",
                    outframe="CMB",veltype="radio")

        mytb = tbtool()
        mytb.open(self.outvis)
        data = mytb.getcol('DATA')
        exposure = mytb.getcol('EXPOSURE')
        mytb.close()
        nchan = data.size
        check_eq(nchan, 50)
        check_eq(data[0][0][0].real, 0.0950, 0.0001)
        check_eq(data[0][nchan-1][0].imag, 0.0610, 0.0001)
        check_eq(exposure[0], 7.5, 0.0001)

    def test_combineSPWDiffExpWithWeightSpectrumFilledFromWeight(self):
        '''mstransform: Combine SPWs with different exposure using WEIGHT_SPECTRUM filled from WEIGHT'''

        self.vis = 'combine-1-timestamp-2-SPW-no-WEIGHT_SPECTRUM-Different-Exposure.ms'
        self.outvis = 'combineSPWDiffExpWithWeightSpectrumFilledFromWeight.ms'
        self.copyfile(self.vis)

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn="DATA",combinespws=True,regridms=True,
                    mode="frequency",nchan=50,start="2.20804e+10Hz",width="1.950702e+05Hz",nspw=1,
                    interpolation="fftshift",phasecenter="J2000 12h01m53.13s -18d53m09.8s",
                    outframe="CMB",veltype="radio",usewtspectrum=True)

        mytb = tbtool()
        mytb.open(self.outvis)
        data = mytb.getcol('DATA')
        exposure = mytb.getcol('EXPOSURE')
        mytb.close()
        nchan = data.size
        check_eq(nchan, 50)
        check_eq(data[0][0][0].real, 0.0950, 0.0001)
        check_eq(data[0][nchan-1][0].imag, 0.0610, 0.0001)
        check_eq(exposure[0], 7.5, 0.0001)

    def test_fillWeightSpectrumFromWeight(self):
        '''mstransform: Fill output WEIGHT_SPECTRUM using WEIGHTS'''

        self.vis = 'combine-1-timestamp-2-SPW-no-WEIGHT_SPECTRUM-Same-Exposure.ms'
        self.outvis = 'fillWeightSpectrumFromWeight.ms'
        self.copyfile(self.vis)

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn="DATA",combinespws=True,regridms=True,
                    mode="frequency",nchan=50,start="2.20804e+10Hz",width="1.950702e+05Hz",nspw=1,
                    interpolation="fftshift",phasecenter="J2000 12h01m53.13s -18d53m09.8s",
                    outframe="CMB",veltype="radio",usewtspectrum=True)

        mytb = tbtool()
        mytb.open(self.outvis)
        weightSpectrum = mytb.getcol('WEIGHT_SPECTRUM')
        mytb.close()
        nchan = weightSpectrum.size
        check_eq(nchan, 50)
        check_eq(weightSpectrum[0][0][0], 1.9007, 0.0001)
        check_eq(weightSpectrum[0][nchan-1][0], 1.0156, 0.0001)

    def test_combineSPWAndChanAvgWithWeightSpectrum(self):
        '''mstransform: Combine SPWs and channel average using WEIGHT_SPECTRUM'''

        self.vis = 'combine-1-timestamp-2-SPW-with-WEIGHT_SPECTRUM-Same-Exposure.ms'
        self.outvis = 'test_combineSPWAndChanAvgWithWeightSpectrum.ms'
        self.copyfile(self.vis)

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn="DATA",combinespws=True,regridms=True,
                    mode="frequency",nchan=12,start="2.20804e+10Hz",width="7.802808e+05Hz",nspw=1,
                    interpolation="fftshift",phasecenter="J2000 12h01m53.13s -18d53m09.8s",
                    outframe="CMB",veltype="radio",chanaverage=True,chanbin=4,useweights='spectrum')

        mytb = tbtool()
        mytb.open(self.outvis)
        data = mytb.getcol('DATA')
        mytb.close()
        nchan = data.size
        check_eq(nchan, 12)
        check_eq(data[0][0][0].real, 0.0893, 0.0001)
        check_eq(data[0][nchan-1][0].imag, -0.2390, 0.0001)

    def test_combineSPWDiffExpAndChanAvgWithWeightSpectrum(self):
        '''mstransform: Combine SPWs with different exposure and channel average using WEIGHT_SPECTRUM'''

        self.vis = 'combine-1-timestamp-2-SPW-with-WEIGHT_SPECTRUM-Different-Exposure.ms'
        self.outvis = 'test_combineSPWDiffExpAndChanAvgWithWeightSpectrum.ms'
        self.copyfile(self.vis)

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn="DATA",combinespws=True,regridms=True,
                    mode="frequency",nchan=12,start="2.20804e+10Hz",width="7.802808e+05Hz",nspw=1,
                    interpolation="fftshift",phasecenter="J2000 12h01m53.13s -18d53m09.8s",
                    outframe="CMB",veltype="radio",chanaverage=True,chanbin=4,useweights='spectrum')

        mytb = tbtool()
        mytb.open(self.outvis)
        data = mytb.getcol('DATA')
        mytb.close()
        nchan = data.size
        check_eq(nchan, 12)
        check_eq(data[0][0][0].real, 0.0893, 0.0001)
        check_eq(data[0][nchan-1][0].imag, -0.2390, 0.0001)

class test_channelAverageByDefault(test_base_compare):

    def setUp(self):
        super(test_channelAverageByDefault,self).setUp()
        self.setUp_almasim()
        self.outvis = 'test_channelAverageByDefaultInVelocityMode-mst.ms'
        self.refvis = 'test_channelAverageByDefaultInVelocityMode-cvel.ms'
        self.outvis_sorted = 'test_channelAverageByDefaultInVelocityMode-mst-sorted.ms'
        self.refvis_sorted = 'test_channelAverageByDefaultInVelocityMode-cvel-sorted.ms'
        os.system('rm -rf test_channelAverageByDefaultInVelocityMode*')

    def tearDown(self):
        super(test_channelAverageByDefault,self).tearDown()

    def test_channelAverageByDefaultInVelocityMode(self):
        self.outvis = 'test_channelAverageByDefaultInVelocityMode.ms'

        mstransform(vis=self.vis,outputvis=self.outvis,regridms=True,combinespws=True,interpolation="linear",
                    mode="velocity",veltype="optical",width='30km/s',restfreq='230GHz',datacolumn='ALL')
        cvel(vis=self.vis,outputvis=self.refvis,interpolation="linear",mode="velocity",veltype="optical",width='30km/s',restfreq='230GHz')

        self.generate_tolerance_map()
        self.post_process()


class test_float_column(test_base):
    def setUp(self):
        self.setUp_floatcol()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)

    def test_regrid_float(self):
        '''mstransform: change outframe of FLOAT_DATA MS'''
        self.outputms = 'floatcol1.mms'
        # jagonzal: Let's use spw 0, because the rest have negative chan widths
        mstransform(vis=self.vis,outputvis=self.outputms,datacolumn='FLOAT_DATA',
                    regridms=True,outframe='LSRK',spw='0')

	print "Check column and keywords"
        mytb = tbtool()
        mytb.open(self.outputms+'/SPECTRAL_WINDOW')
        refnum = mytb.getcell('MEAS_FREQ_REF',0)
        mytb.close()
        self.assertEqual(refnum, 1)
        
        # CAS-5900. Check the keywords in FLOAT_DATA are the same
        mytb.open(self.outputms)
        okeys = mytb.getcolkeywords('FLOAT_DATA')
        mytb.close()
        mytb.open(self.vis)
        ikeys = mytb.getcolkeywords('FLOAT_DATA')
        mytb.close()
        self.assertDictEqual(ikeys, okeys, 'Keywords from FLOAT_DATA are different')
        
        
class test_timeaverage(test_base_compare):

    def setUp(self):
        super(test_timeaverage,self).setUp()
        self.setUp_4ants()
        self.outvis = 'test_timeaverage-mst.ms'
        self.refvis = 'test_timeaverage-split.ms'
        self.outvis_sorted = 'test_timeaverage-mst-sorted.ms'
        self.refvis_sorted = 'test_timeaverage-split-sorted.ms'
        os.system('rm -rf test_timeaverage*')
#        flagdata(vis=self.vis,mode='unflag', flagbackup=False)
        self.unflag_ms()

    def tearDown(self):
        super(test_timeaverage,self).tearDown()

    def unflag_ms(self):
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.init()
        aflocal.run(writeflags=True)
        aflocal.done()

    def test_timeaverage_data(self):

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='DATA',timeaverage=True,timebin='30s')
        split(vis=self.vis,outputvis=self.refvis,datacolumn='DATA',timebin='30s')

        self.generate_tolerance_map()

        self.mode['UVW'] = "percentage"
        self.tolerance['UVW'] = 15.0/100

        self.mode['EXPOSURE'] = "percentage"
        self.tolerance['EXPOSURE'] = 3.58E-5/100

        self.mode['TIME_CENTROID'] = "absolute"
        self.tolerance['TIME_CENTROID'] = 2.77E-4

        self.mode['DATA'] = "percentage"
        self.tolerance['DATA'] = 2.27E-5/100

        self.mode['WEIGHT'] = "percentage"
        self.tolerance['WEIGHT'] = 3.15E-3/100

        self.mode['SIGMA'] = "percentage"
        self.tolerance['SIGMA'] = 35

        self.post_process()

    def test_timeaverage_model(self):

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='MODEL',timeaverage=True,timebin='30s')
        split(vis=self.vis,outputvis=self.refvis,datacolumn='MODEL',timebin='30s')

        self.generate_tolerance_map()

        self.mode['UVW'] = "percentage"
        self.tolerance['UVW'] = 4.08E-3/100

        self.mode['EXPOSURE'] = "percentage"
        self.tolerance['EXPOSURE'] = 3.58E-5/100

        self.mode['TIME_CENTROID'] = "absolute"
        self.tolerance['TIME_CENTROID'] = 2.77E-4

        self.mode['DATA'] = "percentage"
        self.tolerance['DATA'] = 1.20E-5/100

        self.mode['WEIGHT'] = "percentage"
        self.tolerance['WEIGHT'] = 3.15E-3/100

        self.mode['SIGMA'] = "percentage"
        self.tolerance['SIGMA'] = 35

        self.post_process()

    def test_timeaverage_corrected(self):

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='CORRECTED',timeaverage=True,timebin='30s')
        split(vis=self.vis,outputvis=self.refvis,datacolumn='CORRECTED',timebin='30s')

        self.generate_tolerance_map()

        self.mode['UVW'] = "percentage"
        self.tolerance['UVW'] = 4.08E-3/100

        self.mode['EXPOSURE'] = "percentage"
        self.tolerance['EXPOSURE'] = 3.58E-5/100

        self.mode['TIME_CENTROID'] = "absolute"
        self.tolerance['TIME_CENTROID'] = 2.77E-4

        self.mode['DATA'] = "percentage"
        self.tolerance['DATA'] = 2.34E-5/100

        self.mode['WEIGHT'] = "percentage"
        self.tolerance['WEIGHT'] = 3.15E-3/100

        self.mode['SIGMA'] = "percentage"
        self.tolerance['SIGMA'] = 35

        self.post_process()

    def test_timeaverage_baseline_dependent(self):

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='DATA',timeaverage=True,timebin='10s',maxuvwdistance=1E5)
        mstransform(vis=self.vis,outputvis=self.refvis,datacolumn='DATA',timeaverage=True,timebin='10s')

        self.generate_tolerance_map()

        self.post_process()

class test_timeaverage_limits(test_base):

    def setUp(self):
        self.setUp_CAS_4850()
        self.outvis = 'test_timeaverage_limits.ms'

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outvis)

    def test_CAS_4850(self):

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='DATA',timeaverage=True,timebin='40s')

        mytb = tbtool()
        mytb.open(self.outvis)
        interval = mytb.getcol('INTERVAL')
        print interval[0]
        mytb.close()
        check_eq(interval[0] >= 40.0,True)


class test_multiple_transformations(test_base_compare):

    def setUp(self):
        super(test_multiple_transformations,self).setUp()
        self.setUp_4ants()
        self.outvis = 'test_timeaverage_and_combine_spws_single_run.ms'
        self.tmpvis = 'test_timeaverage_and_combine_spws_1st_step.ms'
        self.refvis = 'test_timeaverage_and_combine_spws_2nd_step.ms'
        self.outvis_sorted = 'test_timeaverage_and_combine_spws_single_run_sorted.ms'
        self.refvis_sorted = 'test_timeaverage_and_combine_spws_2nd_step_sorted.ms'
        os.system('rm -rf test_timeaverage_and_combine_spws*')

    def tearDown(self):
        super(test_multiple_transformations,self).tearDown()

    def test_timeaverage_2x_and_combine_two_spws_one_baseline_one_timestep(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',antenna="0&&1", timerange='14:45:08.50~14:45:9.50',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='2s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',antenna="0&&1", timerange='14:45:08.50~14:45:9.50',
                    datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_2x_and_combine_two_spws_one_baseline_two_timesteps(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',antenna="0&&1", timerange='14:45:08.50~14:45:11.50',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='2s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',antenna="0&&1", timerange='14:45:08.50~14:45:11.50',
                    datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_2x_and_combine_two_spws_two_baselines_one_timestep(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',antenna="0&&1~2", timerange='14:45:08.50~14:45:9.50',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='2s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',antenna="0&&1~2", timerange='14:45:08.50~14:45:9.50',
                    datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_2x_and_combine_two_spws_two_baselines_two_timesteps(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',antenna="0&&1~2", timerange='14:45:08.50~14:45:11.50',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='2s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',antenna="0&&1~2", timerange='14:45:08.50~14:45:11.50',
                    datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_2x_and_combine_two_spws_two_baselines(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',antenna="0&&1~2",
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='2s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',antenna="0&&1~2",
                    datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_30x_and_combine_two_spws_two_baselines(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',antenna="0&&1~2",
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='30s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',antenna="0&&1~2",
                    datacolumn='DATA',timeaverage=True,timebin='30s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_2x_and_combine_two_spws_four_baselines(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='2s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',
                    datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_30x_and_combine_two_spws_four_baselines(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='30s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',
                    datacolumn='DATA',timeaverage=True,timebin='30s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_2x_and_combine_seven_spws_four_baselines(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='8~15',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='2s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='8~15',
                    datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_30x_and_combine_seven_spws_four_baselines(self):

        mstransform(vis=self.vis,outputvis=self.outvis,spw='8~15',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='30s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='8~15',
                    datacolumn='DATA',timeaverage=True,timebin='30s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)

        self.generate_tolerance_map()

        self.post_process()

    def test_timeaverage_and_channel_average(self):

        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='DATA',timeaverage=True,timebin='2s',chanaverage=True,chanbin=2)
        mstransform(vis=self.vis,outputvis=self.tmpvis,datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',chanaverage=True,chanbin=2)

        self.generate_tolerance_map()

        self.post_process()


class test_regridms_single_spw(test_base_compare):
    '''Tests for regridms w/o combining SPWS'''

    def setUp(self):
        super(test_regridms_single_spw,self).setUp()
        self.setUp_CAS_5013()
        self.outvis = 'test_regridms_single_spw_mst.ms'
        self.refvis = 'test_regridms_single_spw_cvel.ms'
        self.outvis_sorted = 'test_regridms_single_spw_mst_sorted.ms'
        self.refvis_sorted = 'test_regridms_single_spw_cvel_sorted.ms'
        os.system('rm -rf test_regridms_single_sp*')

    def tearDown(self):
        super(test_regridms_single_spw,self).tearDown()

    def test_regrid_only_LSRK(self):
        '''mstransform: Change ref. frame to LSRK'''

        mstransform(vis=self.vis,outputvis=self.outvis,regridms=True,datacolumn='ALL',
                    field='Vy_CMa',spw='3',mode='frequency',nchan=3830,start='310427.353MHz',width='-244.149kHz',outframe='lsrk')
        cvel(vis=self.vis,outputvis=self.refvis,
             field='Vy_CMa',spw='3',mode='frequency',nchan=3830,start='310427.353MHz',width='-244.149kHz',outframe='lsrk')

        self.generate_tolerance_map()

        self.post_process()
        
class test_regridms_multiple_spws(test_base_compare):
    '''Tests for regridms combining SPWS'''
       
    def setUp(self):
        super(test_regridms_multiple_spws,self).setUp()
        self.setUp_CAS_5172()
        self.outvis = 'test_regridms_multiple_spw_mst.ms'
        self.refvis = 'test_regridms_multiple_spw_cvel.ms'
        self.outvis_sorted = 'test_regridms_multiple_spw_mst_sorted.ms'
        self.refvis_sorted = 'test_regridms_multiple_spw_cvel_sorted.ms'
        os.system('rm -rf test_regridms_multiple_spw*')        
        
    def tearDown(self):
        super(test_regridms_multiple_spws,self).tearDown()
        
    def test_combine_regrid_fftshift(self):
        '''mstransform: Combine 2 SPWs and change ref. frame to LSRK using fftshift''' 
        
        cvel(vis = self.vis, outputvis = self.refvis ,mode = 'velocity',nchan = 10,start = '-50km/s',width = '5km/s',
             interpolation = 'fftshift',restfreq = '36.39232GHz',outframe = 'LSRK',veltype = 'radio')
        
        mstransform(vis = self.vis, outputvis = self.outvis, datacolumn='all',combinespws = True, regridms = True, 
                    mode = 'velocity', nchan = 10, start = '-50km/s', width = '5km/s', interpolation = 'fftshift', 
                    restfreq = '36.39232GHz', outframe = 'LSRK', veltype = 'radio')

        self.generate_tolerance_map()
        
        self.mode['WEIGHT'] = "absolute"
        self.tolerance['WEIGHT'] = 1

        self.post_process()          
        
class test_regridms_spw_with_different_number_of_channels(test_base):
    '''Tests for regridms w/o combining SPWS'''
       
    def setUp(self):
        self.setUp_CAS_4983()
        self.outvis = 'test_regridms_spw_with_different_number_of_channels.ms'
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outvis)
        
    def test_regridms_spw_with_different_number_of_channels_separately(self):
        '''mstransform: Regrid SPWs separately, applying pre-channel averaging to only some of them''' 
        
        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='data',field='J0102-7546',regridms=True,
                    mode='frequency',width='29297.28kHz',outframe='lsrk',veltype='radio')  
        
        # DDI subtable should have 4 rows with the proper indices     
        mytb = tbtool()
        mytb.open(self.outvis + '/SPECTRAL_WINDOW')
        numChan = mytb.getcol('NUM_CHAN')      
        mytb.close()          
        check_eq(numChan[0], 32)
        check_eq(numChan[1], 2)
        check_eq(numChan[2], 68)
        check_eq(numChan[3], 2)        


class test_spw_poln(test_base):
    '''tests for spw with different correlation shapes'''

    def setUp(self):
        self.setUp_3c84()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        os.system('rm -rf list.obs')

    def test_corr_selection(self):
        '''mstransform: verify correct re-indexing of sub-tables'''
        self.outputms = '3cLL.ms'

        # It will select spws 1,2,3, polids=1,2,3 but each with 1 NUM_CORR only
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', correlation='LL')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'polarization':'LL'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()

        self.assertEqual(inp_nrow, out_nrow)

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')

        pol_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'POLARIZATION_ID')
        self.assertEqual(pol_col['r1'][0], 1,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r2'][0], 2,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r3'][0], 3,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')

        # Verify that POLARIZATION table is not re-sized.
        corr_col = th.getVarCol(self.outputms+'/POLARIZATION', 'NUM_CORR')
        self.assertEqual(corr_col.keys().__len__(), 4, 'Wrong number of rows in POLARIZATION table')

        listobs(self.outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')

    def test_repeated_spwid(self):
        '''mstransform: split one spw ID mapping to two DDI'''
        # MS looks like this:
        # SpwID  #Chans  Corrs        DDID
        # 0       256    RR           0
        # 0       256    LL           1
        # 1       128    RR,LL        2
        # 2       64     RR,RL,LR,LL  3

        self.outputms = '3cspw0.ms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'spw':'0'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()

        self.assertEqual(inp_nrow, out_nrow)

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 2, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')

        pol_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'POLARIZATION_ID')
        self.assertEqual(pol_col['r1'][0], 0,'Error re-indexing POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r2'][0], 1,'Error re-indexing POLARIZATION_ID of DATA_DESCRIPTION table')


    def test_spwid_poln_LL(self):
        '''mstransform: split one spw ID, polarization LL from RR,LL'''
        # MS looks like this:
        # SpwID  #Chans  Corrs        DDID
        # 0       256    RR           0
        # 0       256    LL           1
        # 1       128    RR,LL        2
        # 2       64     RR,RL,LR,LL  3

        self.outputms = '3cspw0LL.ms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0',
                    correlation='LL')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'spw':'0','polarization':'LL'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()

        self.assertEqual(inp_nrow, out_nrow)

    def test_spwids_poln_RR(self):
        '''mstransform: split two spw IDs, polarization RR from RR,LL'''
        # MS looks like this:
        # SpwID  #Chans  Corrs        DDID
        # 0       256    RR           0
        # 0       256    LL           1
        # 1       128    RR,LL        2
        # 2       64     RR,RL,LR,LL  3

        self.outputms = '3cspw01RR.ms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0,1',
                    correlation='RR')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'spw':'0,1','polarization':'RR'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()

        self.assertEqual(inp_nrow, out_nrow)

    def test_spw_selection(self):
        '''mstransform: split two spws with different polarization shapes'''
        self.outputms = '3cspw12.ms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='1,2')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'spw':'1,2'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()
        self.assertEqual(inp_nrow, out_nrow)

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 2, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')

        pol_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'POLARIZATION_ID')
        self.assertEqual(pol_col['r1'][0], 2,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r2'][0], 3,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')

        # Verify that POLARIZATION table is not re-sized.
        corr_col = th.getVarCol(self.outputms+'/POLARIZATION', 'NUM_CORR')
        self.assertEqual(corr_col.keys().__len__(), 4, 'Wrong number of rows in POLARIZATION table')

    def test_regrid_spw_with_diff_pol_shape(self):
        '''mstransform: regrid spw 0 that has repeated SPW ID'''
        self.outputms = '3cLSRKspw0.ms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0',
                    regridms=True,outframe='LSRK')

        ret = th.verifyMS(self.outputms, 1, 256, 0)
        self.assertTrue(ret[0],ret[1])

        mytb = tbtool()
        mytb.open(self.outputms+'/SPECTRAL_WINDOW')
        refnum = mytb.getcell('MEAS_FREQ_REF',0)
        mytb.close()
        self.assertEqual(refnum, 1)

        listobs(self.outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')

        # Verify the metadata information
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        dds = msmdt.datadescids()
        msmdt.done()
        self.assertEqual(dds.__len__(),2,'Wrong number of rows in DD table')

    def test_chanavg_spw_with_diff_pol_shape(self):
        '''mstransform: channel average spw 0 that has repeated SPW ID'''
        self.outputms = '3cChAvespw0.ms'
        # Create only one output channel
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0',
                    chanaverage=True,chanbin=256)

        # verify the metadata of the output
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        nchan = msmdt.nchan(0) # 1
        nrow = msmdt.nrows() # 2600
        dds = msmdt.datadescids() # 2
        meanfreq = msmdt.meanfreq(0) # 4968996093.75
        chanfreq = msmdt.chanfreqs(0) # [4.96899609e+09]
        chanwidth = msmdt.chanwidths(spw=0, unit='kHz') # 2000
        msmdt.done()

        self.assertEqual(dds.__len__(),2,'Wrong number of rows in DD table')
        self.assertEqual(nchan, 1)
        self.assertEqual(nrow, 2600,'Wrong number of rows in DD table')
        self.assertEqual(meanfreq, 4968996093.75)
        self.assertEqual(chanwidth, 2000)
        self.assertAlmostEqual(meanfreq, chanfreq, 1)

        listobs(self.outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')

    def test_mms_spw_selection(self):
        '''mstransform: Create MMS and select two spws with different polarization shapes'''
        self.outputms = '3cspw12.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='1,2',
                    createmms=True, separationaxis='spw')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'spw':'1,2'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()
        self.assertEqual(inp_nrow, out_nrow)

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 2, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')

        pol_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'POLARIZATION_ID')
        self.assertEqual(pol_col['r1'][0], 2,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r2'][0], 3,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')

        # Verify that POLARIZATION table is not re-sized.
        corr_col = th.getVarCol(self.outputms+'/POLARIZATION', 'NUM_CORR')
        self.assertEqual(corr_col.keys().__len__(), 4, 'Wrong number of rows in POLARIZATION table')

    def test_mms_spw_selection2(self):
        '''mstransform: Create MMS and select two spws with different polarization shapes'''
        self.outputms = '3cspw01.mms'
        # spw=0 contains two DD in DATA_DESCRIPTION table
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0,1',
                    createmms=True, separationaxis='spw')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'spw':'0,1'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()
        self.assertEqual(inp_nrow, out_nrow)

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')

        pol_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'POLARIZATION_ID')
        self.assertEqual(pol_col['r1'][0], 0,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r2'][0], 1,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r3'][0], 2,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')

        # Verify that POLARIZATION table is not re-sized.
        corr_col = th.getVarCol(self.outputms+'/POLARIZATION', 'NUM_CORR')
        self.assertEqual(corr_col.keys().__len__(), 4, 'Wrong number of rows in POLARIZATION table')

    def test_mms_spw_selection3(self):
        '''mstransform: Create MMS and select three spws with numsubms=2'''
        self.outputms = '3cspw012.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0,1,2',
                    createmms=True, separationaxis='spw', numsubms=2)

        # Verify the input versus the output
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        out_dds = msmdt.datadescids()
        out_nrow = msmdt.nrows()
        msmdt.done()

        self.assertTrue(out_nrow,5200)
        ref = [0,1,2,3]
        for i in out_dds:
            self.assertEqual(out_dds[i], ref[i])

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 4, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r4'][0], 2,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        
        in_feed_col = th.getVarCol(self.vis+'/FEED', 'SPECTRAL_WINDOW_ID')
        out_feed_col = th.getVarCol(self.outputms+'/FEED', 'SPECTRAL_WINDOW_ID')
        
        self.assertDictEqual(in_feed_col, out_feed_col, 'FEED table differs in input and output')

    def test_mms_scan_spw_partition(self):
        '''mstransform: Create MMS and part by scan/spw'''
        self.outputms = '3cscanspw02.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0,2',
                    createmms=True)

        # Verify the input versus the output
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        out_dds = msmdt.datadescids()
        msmdt.done()

        ref = [0,1,2]
        for i in out_dds:
            self.assertEqual(out_dds[i], ref[i])

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        dd_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')

    def test_mms_XXYY_selection(self):
        '''mstransform: correlation='RR,LL' should select and re-index properly'''
        self.outputms = '3cRRLL.mms'
        # spw 0 should not be processed. The selection should happen before the MMS work
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', correlation='RR,LL',
                    createmms=True)
        
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        out_dds = msmdt.datadescids()
        msmdt.done()
        
        ref = [0,1]
        for i in out_dds:
            self.assertEqual(out_dds[i], ref[i])
        
        pol_col = th.getVarCol(self.outputms+'/POLARIZATION','NUM_CORR')
        self.assertEqual(pol_col['r1'][0], 0,'Error in NUM_CORR of POLARIZATION table')
        self.assertEqual(pol_col['r2'][0], 0,'Error in NUM_CORR of POLARIZATION table')
        self.assertEqual(pol_col['r3'][0], 2,'Error in NUM_CORR of POLARIZATION table')
        self.assertEqual(pol_col['r4'][0], 2,'Error in NUM_CORR of POLARIZATION table')

        # Verify that POLARIZATION table is not re-sized.
        corr_col = th.getVarCol(self.outputms+'/POLARIZATION', 'NUM_CORR')
        self.assertEqual(corr_col.keys().__len__(), 4, 'Wrong number of rows in POLARIZATION table')

class testFlags(test_base):
    '''Test the keepflags parameter'''
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
    
    def test_split_keepflags_false(self):
        '''mstransform: split them and do not keep flags in output MS'''
        self.outputms = 'donotkeepflags.ms'
        
        # Unflag and flag spw=4
        flagdata(self.vis, flagbackup=False, mode='list', inpfile=["mode='unflag'","spw='4'"])
        
        # Split scan=31 out
        mstransform(self.vis, outputvis=self.outputms, datacolumn='corrected', scan='31', keepflags=False)
        
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        spws = msmdt.spwsforscan(31)
        msmdt.close()
        self.assertEqual(spws.size, 15)
        
    def test_select_dropped_spw(self):
        '''mstransform: keepflags=False and select flagged spw. Expect error.'''        
        self.outputms = 'donotkeepflags_spw15.ms'
        
        # Unflag and flag spw=15
        flagdata(self.vis, flagbackup=False, mode='list', inpfile=["mode='unflag'","spw='15'"])
    
        try:
            mstransform(self.vis, outputvis=self.outputms, datacolumn='data', spw='>14', keepflags=False)
        except exceptions.RuntimeError, instance:
            print 'Expected Error: %s'%instance
        
        print 'Expected Error!'
        

# Cleanup class
class Cleanup(test_base):

    def tearDown(self):
        os.system('rm -rf ngc5921.*ms* jupiter6cm.demo*')
        os.system('rm -rf Four_ants_3C286.*ms* g19_d2usb_targets*')
        os.system('rm -rf comb*.*ms* reg*.*ms hann*.*ms favg*.*ms')
        os.system('rm -rf split*.*ms')
        os.system('rm -rf 3c84scan1*ms* test.mms')
        os.system('rm -rf donotkeepflags*')

    def test_runTest(self):
        '''mstransform: Cleanup'''
        pass


def suite():
    return [
            test_Combspw1,
            test_Regridms1,
            test_Regridms3,
            test_Hanning,
            test_FreqAvg,
            test_Shape,
            test_Columns,
            test_SeparateSPWs,
            test_MMS,
            test_Parallel,
            test_state,
            test_WeightSpectrum,
            test_channelAverageByDefault,
            test_timeaverage,
            test_timeaverage_limits,
            test_multiple_transformations,
            test_regridms_single_spw,
            test_regridms_multiple_spws,
            test_float_column,
            test_spw_poln,
            test_regridms_spw_with_different_number_of_channels,
            testFlags,
            Cleanup]
