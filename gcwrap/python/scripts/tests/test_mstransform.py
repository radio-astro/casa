import shutil
import unittest
import os
import numpy
from tasks import *
from taskinit import mstool, tbtool
from __main__ import default
import testhelper as th
from recipes.listshapes import listshapes
from parallel.parallel_task_helper import ParallelTaskHelper

    
# Define the root for the data files
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

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
               
    def cleanup(self):
        os.system('rm -rf '+ self.vis)


class test_Combspw1(test_base):
    ''' Tests for combinespws'''
    
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf combspw*.*ms inpmms*.*ms combcvel*ms list.obs')
                
    def test_combspw1_1(self):
        '''mstransform: Combine four spws into one'''
        
        outputms = "combspw11.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0~3')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 256, 0)
        self.assertTrue(ret[0],ret[1])
        
        listobs(outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')
      
    def test_combspw1_2(self):
        '''mstransform: Combine some channels of two spws'''
        
        outputms = "combspw12.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0:60~63,1:60~63')
        self.assertTrue(os.path.exists(outputms))
        
        # The spws contain gaps, therefore the number of channels is bigger
        ret = th.verifyMS(outputms, 1, 68, 0)
        self.assertTrue(ret[0],ret[1])
        
        # Compare with cvel results
        default(cvel)
        cvel(vis=self.vis, outputvis='combcvel12.ms', spw='0:60~63,1:60~63')
        ret = th.verifyMS('combcvel12.ms', 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw1_3(self):
        '''mstransform: Do not combine spws and create MMS with axis scan.'''
        self.setUp_jupiter()
        outputms = 'combspw13.mms'
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, spw='0,1',field = '12',
             datacolumn='DATA', createmms=True, separationaxis='scan')
        
        self.assertTrue(os.path.exists(outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')
        
        ret = th.verifyMS(outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw1_4(self):
        '''mstransform: Combine some channels of two spws using MMS input'''
        # same test as test_combspw1_2
        mmsfile = "inpmms14.mms"
        # First create an MMS
        mstransform(vis=self.vis, outputvis=mmsfile, createmms=True)
        
        # Now do the same as in test_combspw1_2. Datacolumn moved to DATA
        outputms = "combspw14.ms"
        mstransform(vis=mmsfile, outputvis=outputms, combinespws=True, spw='0:60~63,1:60~63',
                    datacolumn='data')
        self.assertTrue(os.path.exists(outputms))
        
        # The spws contain gaps, therefore the number of channels is bigger
        ret = th.verifyMS(outputms, 1, 68, 0)
        self.assertTrue(ret[0],ret[1])
        
        # Compare with cvel results
        default(cvel)
        cvel(vis=self.vis, outputvis='combcvel14.ms', spw='0:60~63,1:60~63')
        ret = th.verifyMS('combcvel14.ms', 1, 68, 0)
        self.assertTrue(ret[0],ret[1])  
        
    def test_combspw1_5(self):
        '''mstransform: Combine four spws into one'''
        
        outputms = "combspw15.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='2,5,8')
        self.assertTrue(os.path.exists(outputms))
        
        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')        
        self.assertEqual(dd_col.keys().__len__(), 1, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')

        # DDI subtable should have 4 rows with the proper index      
        mytb = tbtool()
        mytb.open(outputms + '/DATA_DESCRIPTION')
        spwCol = mytb.getcol('SPECTRAL_WINDOW_ID')      
        mytb.close()          
        nspw = spwCol.size  
        check_eq(nspw, 1)   
        check_eq(spwCol[0], 0)
        

class test_Regridms1(test_base):
    '''Tests for regridms using Four_ants_3C286.ms'''
       
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf reg*.*ms testmms*ms list.obs')
        
    def test_regrid1_1(self):
        '''mstransform: Default of regridms parameters'''
        
        outputms = "reg11.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True)
        self.assertTrue(os.path.exists(outputms))
        
        # The regriding should be the same as the input
        for i in range(16):
            ret = th.verifyMS(outputms, 16, 64, i)
            self.assertTrue(ret[0],ret[1])

        listobs(outputms)    
        listobs(outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')
            
    def test_regrid1_2(self):
        '''mstransform: Default regridms with spw selection'''
        
        outputms = "reg12.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, spw='1,3,5,7')
        self.assertTrue(os.path.exists(outputms))
        
        # The output should be the same as the input
        for i in range(4):
            ret = th.verifyMS(outputms, 4, 64, i)
            self.assertTrue(ret[0],ret[1])
            
        listobs(outputms)    

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
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
        
        outputms = "reg13.ms"
        mstransform(vis=mmsfile, outputvis=outputms, regridms=True, spw='1,3,5,7',
                    datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        # The regriding should be the same as the input
        for i in range(4):
            ret = th.verifyMS(outputms, 4, 64, i)
            self.assertTrue(ret[0],ret[1])
            
        listobs(outputms)    

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 4, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r4'][0], 3,'Error re-indexing DATA_DESCRIPTION table')


class test_Regridms3(test_base):
    '''Tests for regridms using Jupiter MS'''
       
    def setUp(self):
        self.setUp_jupiter()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf reg31*.*ms cvel31.*ms')

    def test_regrid3_1(self):
        '''mstransform 12: Check that output columns are the same when using mstransform'''
        outputms = 'reg31.ms'
        
        mstransform(vis=self.vis, outputvis=outputms, field='6',
                    combinespws=True, regridms=True, datacolumn='data',
                    mode='frequency', nchan=2, start='4.8101 GHz', width='50 MHz',
                    outframe='')

        ret = th.verifyMS(outputms, 1, 2, 0)
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
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 1, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')

# Uncomment after seg fault is fixed
#    def test_regrid3_2(self):
#        '''mstransform: Combine spw and regrid MS with two spws, select one field and 2 spws'''
#        # cvel: test8
#        outputms = "reg32a.ms"
#        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, 
#                    spw='0,1',field = '11',nchan=1, width=2, datacolumn='DATA')
#        self.assertTrue(os.path.exists(outputms))
#        
#        ret = th.verifyMS(outputms, 1, 1, 0)
#        self.assertTrue(ret[0],ret[1])  
#        
#        # Now, do only the regridding and do not combine spws
#        outputms = "reg32b.ms"
#        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True, 
#                    spw='0,1',field = '11',nchan=1, width=2, datacolumn='DATA')
#        self.assertTrue(os.path.exists(outputms))
#        
#        ret = th.verifyMS(outputms, 2, 1, 0)
#        self.assertTrue(ret[0],ret[1])  
#        ret = th.verifyMS(outputms, 2, 1, 1)
#        self.assertTrue(ret[0],ret[1])  


class test_Hanning(test_base):
    '''Test for hanning transformation'''
    def setUp(self):
        self.setUp_ngc5921()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf hann*.*ms')

    def test_hanning1(self):
        '''mstransform: Apply Hanning smoothing in MS with 24 spws. Do not combine spws.'''
        self.setUp_g19()
        outputms = "hann1.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, hanning=True,
                    datacolumn='data')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 24, 128, 0)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 24, 128, 2)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 24, 128, 15)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 24, 128, 18)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 24, 128, 23)
        self.assertTrue(ret[0],ret[1])        

    def test_hanning2(self):
        '''mstransform: Apply Hanning smoothing and combine spw=1,2,3.'''
        self.setUp_g19()
        outputms = "hann2.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, hanning=True,
                    spw='1,2,3', datacolumn='data')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 1448, 0)
        self.assertTrue(ret[0],ret[1])        

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 1, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
            
    def test_hanning3(self):
        '''mstransform: Hanning theoretical and calculated values should be the same'''
        # hanning: test4
        outputms = "hann3.ms"
        
        # The hanningsmooth task flags the first and last channels. Check it!
        # Before running the task
        flag_col = th.getVarCol(self.vis, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])
        
        # Get the DATA column before the transformation
        data_col = th.getVarCol(self.vis, 'DATA')
        
        mstransform(vis=self.vis, outputvis=outputms, datacolumn='data', hanning=True)

        # After running the task
        flag_col = th.getVarCol(outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

        corr_col = th.getVarCol(outputms, 'DATA')
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
        outputms = "hann4.ms"
        
      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.vis, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        mstransform(vis=self.vis, outputvis=outputms, hanning=True, regridms=True,
                        outframe='cmb',datacolumn='data')

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(outputms, 'FLAG')
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
        os.system('rm -rf favg*.*ms')

    def test_freqavg1(self):
        '''mstranform: Average 20 channels of one spw'''
        outputms = "favg1.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='2', chanaverage=True, chanbin=20)
                            
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 6, 0)
        self.assertTrue(ret[0],ret[1])        

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 1, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        
    def test_freqavg2(self):
        '''mstranform: Select a few channels to average from one spw'''
        outputms = "favg2.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='2:10~20', chanaverage=True, chanbin=2)
                            
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 5, 0)
        self.assertTrue(ret[0],ret[1])        
        
    def test_freqavg3(self):
        '''mstranform: Average all channels of one spw'''
        outputms = "favg3.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='23', chanaverage=True, chanbin=128)
                            
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_freqavg4(self):
        '''mstranform: Average using different bins for several spws'''
        outputms = "favg4.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='10,12,20', chanaverage=True,
                    chanbin=[128,4,10])
                            
        self.assertTrue(os.path.exists(outputms))
        
        # Output should be:
        # spw=0 1 channel
        # spw=1 32 channels
        # spw=3 13 channels
        ret = th.verifyMS(outputms, 3, 1, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 32, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 12, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg5(self):
        '''mstranform: Different number of spws and chanbin. Expected error'''
        outputms = "favg5.ms"
        ret = mstransform(vis=self.vis, outputvis=outputms, spw='2,10', chanaverage=True,
                    chanbin=[10,20,4])
                            
        self.assertFalse(ret)

    def test_freqavg6(self):
        '''mstranform: Average all channels of one spw, save as an MMS'''
        # same as test_freqavg3
        outputms = "favg6.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='23', chanaverage=True, chanbin=128,
                    createmms=True)
                            
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_freqavg7(self):
        '''mstranform: Average using different bins for several spws, output MMS'''
        # same as test_freqavg4
        outputms = "favg7.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='10,12,20', chanaverage=True,
                    chanbin=[128,4,10], createmms=True, separationaxis='scan')
                            
        self.assertTrue(os.path.exists(outputms))
        
        # Output should be:
        # spw=0 1 channel
        # spw=1 32 channels
        # spw=3 13 channels
        ret = th.verifyMS(outputms, 3, 1, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 32, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 12, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg8(self):
        '''mstranform: Average using different bins for several spws, output MMS'''
        # same as test_freqavg4
        outputms = "favg8.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='10,12,20', chanaverage=True,
                    chanbin=[128,4,10], createmms=True, separationaxis='spw',numsubms=2)
                            
        self.assertTrue(os.path.exists(outputms))
        
        # Output should be:
        # spw=0 1 channel
        # spw=1 32 channels
        # spw=3 13 channels
        ret = th.verifyMS(outputms, 3, 1, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 32, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 12, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

    def test_freqavg9(self):
        '''mstranform: Average using different bins and a channel selection, output MMS'''
        outputms = "favg9.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='2,12,10:1~10', chanaverage=True,
                    chanbin=[32,128,5], createmms=True, separationaxis='spw')
                            
        self.assertTrue(os.path.exists(outputms))
        
        # Output should be:
        # spw=0 4 channels
        # spw=1 1 channel
        # spw=2 2 channels
        ret = th.verifyMS(outputms, 3, 4, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 1, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 2, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 3, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg10(self):
        '''mstranform: Average using different bins, channel selection, both axes, output MMS'''
        outputms = "favg10.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='2,12,10:1~10', chanaverage=True,
                    chanbin=[32,128,5], createmms=True, separationaxis='both')
                            
        self.assertTrue(os.path.exists(outputms))
        
        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')
        
        # Output should be:
        # spw=0 4 channels
        # spw=1 1 channel
        # spw=2 2 channels
        ret = th.verifyMS(outputms, 3, 4, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 1, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 3, 2, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
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
        os.system('rm -rf shape*.*ms')
         
    def test_shape1(self):
        '''mstransform: default tileshape'''
        outputms = "shape1.ms"
        mstransform(vis=self.vis, outputvis=outputms, createmms=False, tileshape=[0])
                            
        self.assertTrue(os.path.exists(outputms))
        
        # Get the tile shape in input
        tblocal = tbtool()
        tblocal.open(self.vis)
        inpdm = tblocal.getdminfo()
        tblocal.close()
        inptsh = th.getTileShape(inpdm)
        
        # Get the tile shape for the output
        tblocal.open(outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)
        
        # Compare both
        self.assertTrue((inptsh==outtsh).all(), 'Tile shapes are different')                

    def test_shape2(self):
        '''mstransform: custom tileshape'''
        outputms = "shape2.ms"
        inptsh = [4,20,1024]
        mstransform(vis=self.vis, outputvis=outputms, createmms=False, tileshape=inptsh)
                            
        self.assertTrue(os.path.exists(outputms))
                
        # Check the tile shape for the output
        tblocal = tbtool()
        tblocal.open(outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)
        
        self.assertTrue((inptsh==outtsh).all(), 'Tile shapes are different')
        
    def test_shape3(self):
        '''mstransform: DATA and FLAG tileshapes should be the same'''
        outputms = "shape3.ms"
        inptsh = [4,10,1024]
        mstransform(vis=self.vis, outputvis=outputms, createmms=False, tileshape=inptsh)
                            
        self.assertTrue(os.path.exists(outputms))
                
        # Get the tile shape for the DATA output
        tblocal = tbtool()
        tblocal.open(outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)        
        # And for the FLAG column
        flagtsh = th.getTileShape(outdm, 'FLAG')        

        self.assertTrue((outtsh==flagtsh).all(), 'Tile shapes are different')


class test_Columns(test_base):
    '''Test different datacolumns'''
    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
#        os.system('rm -rf '+ self.vis)
        os.system('rm -rf col*.*ms')
        
    # Disabled until CAS-5348 is fixed
    '''
    def test_col1(self):
        """mstransform: try to make real a non-existing virtual MODEL column"""
        self.setUp_ngc5921()
        outputms = "col1.ms"
        mstransform(vis=self.vis, outputvis=outputms, datacolumn='all', realmodelcol=True)
                            
        self.assertTrue(os.path.exists(outputms))
        mcol = th.getColDesc(outputms, 'MODEL_DATA')
        mkeys = mcol.keys()
        self.assertTrue(mkeys.__len__()==0, 'Should not add MODEL_DATA column')
    '''

    # Disabled until CAS-5348 is fixed
    '''
    # This should change. It should not add a real model column, only a virtual one.        
    def test_col2(self):
        """mstransform: make real a virtual MODEL column """
        self.setUp_ngc5921()
        outputms = "col2.ms"
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
        mstransform(vis=inpms, outputvis=outputms, datacolumn='all', realmodelcol=True)
                            
        self.assertTrue(os.path.exists(outputms))
        mcol = th.getColDesc(outputms, 'MODEL_DATA')
        mkeys = mcol.keys()
        self.assertTrue(mkeys.__len__() > 0, 'Should have a MODEL_DATA column')
    '''

    def test_col3(self):
        '''mstransform: split out the MODEL column'''
        outputms = 'col3.ms'
        mstransform(vis=self.vis, outputvis=outputms,field='1',spw='0:0~61', 
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
        os.system('rm -rf separate*.*ms list.obs')
        
    def test_sep1(self):
        '''mstransform: separate one spw into 4, using default regrid parameters'''
        outputms = "separate1.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='2', regridms=True, nspw=4)
        self.assertTrue(os.path.exists(outputms))

        # Should create 4 spws with 16 channels each
        ret = th.verifyMS(outputms, 4, 16, 0)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 4, 16, 1)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 4, 16, 2)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 4, 16, 3)
        self.assertTrue(ret[0],ret[1])       
        
        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 4, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r4'][0], 3,'Error re-indexing DATA_DESCRIPTION table')
        
    def test_sep2(self):
        '''mstransform: separate three spws into 2, using default regrid parameters'''
        outputms = "separate2.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='2,3,4', regridms=True, nspw=2)
        self.assertTrue(os.path.exists(outputms))

        # Should create 2 spws with ?96 channels each
        ret = th.verifyMS(outputms, 2, 96, 0)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 96, 1)
        self.assertTrue(ret[0],ret[1])        
        
        listobs(outputms, listfile='list.obs')
        self.assertTrue(os.path.exists('list.obs'), 'Probable error in sub-table re-indexing')
       
    def test_sep3(self):
        '''mstransform: separate 16 spws into 4 with 10 channels each'''
        outputms = "separate3.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, nspw=4, nchan=10)
        self.assertTrue(os.path.exists(outputms))

        # Should create 4 spws with 10 channels each
        ret = th.verifyMS(outputms, 4, 10, 0)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 4, 10, 3)
        self.assertTrue(ret[0],ret[1])        
        
    def test_CAS_5403_1(self):
        '''mstransform: separate spw 0 into 4 spws and check that DDI subtable is reindexed properly
                        and then check that DDI subtable is reindexed properly'''
        outputms = "test_5403_1.ms"
        mstransform(vis=self.vis, outputvis=outputms,regridms=True,spw='0',nspw=4)
        self.assertTrue(os.path.exists(outputms))

        # DDI subtable should have 4 rows with the proper index      
        mytb = tbtool()
        mytb.open(outputms + '/DATA_DESCRIPTION')
        spwCol = mytb.getcol('SPECTRAL_WINDOW_ID')      
        mytb.close()    
        nspw = spwCol.size  
        check_eq(nspw, 4)   
        check_eq(spwCol[0], 0)
        check_eq(spwCol[1], 1)
        check_eq(spwCol[2], 2)
        check_eq(spwCol[3], 3)
        
    def test_CAS_5403_2(self):
        '''mstransform: combine spw 0,1,2 into one spw and then break it doen in 4 spws. 
                        and then check that DDI subtable is reindexed properly'''
        outputms = "test_5403_2.ms"
        mstransform(vis=self.vis, outputvis=outputms,regridms=True,spw='0,1,2',nspw=4)
        self.assertTrue(os.path.exists(outputms))

        # DDI subtable should have 4 rows with the proper index      
        mytb = tbtool()
        mytb.open(outputms + '/DATA_DESCRIPTION')
        spwCol = mytb.getcol('SPECTRAL_WINDOW_ID')      
        mytb.close()            
        nspw = spwCol.size  
        check_eq(nspw, 4)   
        check_eq(spwCol[0], 0)
        check_eq(spwCol[1], 1)
        check_eq(spwCol[2], 2)
        check_eq(spwCol[3], 3)
   
        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 4, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r4'][0], 3,'Error re-indexing DATA_DESCRIPTION table')
         
class test_MMS(test_base):
    '''Several tests that create an MMS'''
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf testmms*.*ms')
    
    def test_mms1(self):
        '''mstransform: create MMS with spw separation and channel selections'''
        outputms = "testmms1.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw')
                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(outputms, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
                       
    def test_mms2(self):
        '''mstransform: create MMS with spw/scan separation and channel selections'''
        outputms = "testmms2.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='both')
                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

        # Verify that some sub-tables are properly re-indexed.
        dd_col = th.getVarCol(outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')            
        self.assertEqual(dd_col.keys().__len__(), 2, 'Wrong number of rows in DD table')    
        self.assertEqual(dd_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(dd_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')

    def test_mms3(self):
        '''mstransform: create MMS with scan separation and channel selections'''
        outputms = "testmms3.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='scan')                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 2 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])            
        
    def test_mms4(self):
        '''mstransform: verify spw sub-table consolidation'''
        outputms = "testmms4.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='3,5:10~20,7,9,11,13,15',createmms=True,
                    separationaxis='spw')                            
        self.assertTrue(os.path.exists(outputms))
        
        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(outputms, 7, 10, 1, ignoreflags=True)

            
class test_Parallel(test_base):
    '''Run some of the same tests in parallel'''
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf parallel*.*ms')
    
    def test_parallel1(self):
        '''mstransform: create MMS with spw separation and channel selections in parallel'''
        outputms = "parallel1.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw', parallel=False)
                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(outputms, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
                       
    def test_parallel2(self):
        '''mstransform: create MMS with spw/scan separation and channel selections in parallel'''
        outputms = "parallel2.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='both', parallel=False)
                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

    def test_parallel3(self):
        '''mstransform: create MMS with scan separation and channel selections in parallel'''
        outputms = "parallel3.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='scan', parallel=False)                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 2 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])            
        
    def test_parallel4(self):
        '''mstransform: verify spw sub-table consolidation in parallel'''
        outputms = "parallel4.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='3,5:10~20,7,9,11,13,15',createmms=True,
                    separationaxis='spw', parallel=False)                            
        self.assertTrue(os.path.exists(outputms))
        
        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(outputms, 7, 10, 1, ignoreflags=True)

    def test_parallel5(self):
        '''mstransform: Do not combine spws and create MMS with axis scan in parallel.'''
        self.setUp_jupiter()
        outputms = 'parallel5.mms'
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, spw='0,1',field = '12',
             datacolumn='DATA', createmms=True, separationaxis='scan', parallel=False)
        
        self.assertTrue(os.path.exists(outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')
        
        ret = th.verifyMS(outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])
        
        
class test_state(test_base):
    '''Test operation with state id'''
    def setUp(self):
        self.setUp_CAS_5076()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf test_state*.*ms')
        
    def test_CAS_5076(self):
        '''mstransform: select 2 scans and automatically re-index state sub-table'''
        outputms = "test_state.ms"
        mstransform(vis=self.vis, outputvis=outputms, scan='2,3', datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))        
        
        ''' this should not crash '''
        listobs(outputms)
        
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
        
     
   
#    def test_fillWeightSpectrumFromWeight(self):
#        '''mstransform: Fill output WEIGHT_SPECTRUM using WEIGHTS'''
#        
#        self.vis = 'combine-1-timestamp-2-SPW-no-WEIGHT_SPECTRUM-Same-Exposure.ms'
#        self.outvis = 'fillWeightSpectrumFromWeight.ms'
#        self.copyfile(self.vis)
#        
#        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn="DATA",combinespws=True,regridms=True,
#                    mode="frequency",nchan=50,start="2.20804e+10Hz",width="1.950702e+05Hz",nspw=1,
#                    interpolation="fftshift",phasecenter="J2000 12h01m53.13s -18d53m09.8s",
#                    outframe="CMB",veltype="radio")  
#        
#        mytb = tbtool()
#        mytb.open(self.outvis)
#        weightSpectrum = mytb.getcol('WEIGHT_SPECTRUM')      
#        mytb.close()
#        nchan = weightSpectrum.size  
#        check_eq(nchan, 50)   
#        check_eq(weightSpectrum[0][0][0], 58.9232, 0.0001)
#        check_eq(weightSpectrum[0][nchan-1][0], 31.4836, 0.0001)
        
        
#    def test_combineSPWAndChanAvgWithWeightSpectrum(self):
#        '''mstransform: Combine SPWs and channel average using WEIGHT_SPECTRUM'''
#        
#        self.vis = 'combine-1-timestamp-2-SPW-with-WEIGHT_SPECTRUM-Same-Exposure.ms'
#        self.outvis = 'test_combineSPWAndChanAvgWithWeightSpectrum.ms'
#        self.copyfile(self.vis)   
#        
#        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn="DATA",combinespws=True,regridms=True,
#                    mode="frequency",nchan=12,start="2.20804e+10Hz",width="7.802808e+05Hz",nspw=1,
#                    interpolation="fftshift",phasecenter="J2000 12h01m53.13s -18d53m09.8s",
#                    outframe="CMB",veltype="radio",chanaverage=True,chanbin=4,useweights='spectrum')
#        
#        mytb = tbtool()
#        mytb.open(self.outvis)
#        data = mytb.getcol('DATA')      
#        mytb.close()  
#        nchan = data.size   
#        check_eq(nchan, 12)
#        check_eq(data[0][0][0].real, 0.0628, 0.0001)
#        check_eq(data[0][nchan-1][0].imag, -0.2508, 0.0001)
        
        
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
        check_eq(data[0][0][0].real, 0.0628, 0.0001)
        check_eq(data[0][nchan-1][0].imag, -0.2508, 0.0001)
        
class test_channelAverageByDefault(test_base):
    
    def setUp(self):
        self.setUp_almasim()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outvis)
        
    def test_channelAverageByDefaultInVelocityMode(self):
        self.outvis = 'test_channelAverageByDefaultInVelocityMode.ms'
        
        mstransform(vis=self.vis,outputvis=self.outvis,regridms=True,combinespws=True,interpolation="linear",
                    mode="velocity",veltype="optical",width='30km/s',restfreq='230GHz')
        
        mytb = tbtool()
        mytb.open(self.outvis)
        data = mytb.getcol('DATA')      
        nchan = data.shape[1]
        mytb.close() 
        check_eq(nchan, 55)
        check_eq(data[0][0][0].real, 0.0323, 0.0001)
        check_eq(data[0][nchan-1][0].imag, 0.3296, 0.0001)
 
class test_float_column(test_base):
    def setUp(self):
        self.setUp_floatcol()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        
# The following seg faults at the moment. Uncomment in suite later
    def test_regrid_float(self):
        '''mstransform: change outframe of FLOAT_DATA MS'''
        self.outputms = 'floatcol1.mms'
        mstransform(vis=self.vis,outputvis=self.outputms,datacolumn='FLOAT_DATA',
                    regridms=True,outframe='LSRK',spw='1,2')
         
        mytb = tbtool()
        mytb.open(self.outputms+'/SPECTRAL_WINDOW')
        refnum = mytb.getcell('MEAS_FREQ_REF',0)
        mytb.close()
        self.assertEqual(refnum, 1)

class test_timeaverage(test_base):

    def setUp(self):
        self.setUp_4ants()
        self.subtables=['/ANTENNA','/DATA_DESCRIPTION','/FEED','/FIELD','/FLAG_CMD',
                        '/OBSERVATION','/POINTING','/POLARIZATION','/PROCESSOR','/STATE']
        self.sortorder=['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME']      
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outvis)
        os.system('rm -rf '+ self.refvis)
        #os.system('rm -rf '+ self.tmpvis)
        #os.system('rm -rf '+ self.outvis_sorted)
        #os.system('rm -rf '+ self.refvis_sorted)
        
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
            self.tolerance[col] = 0

        
    def compare_subtables(self):
        for subtable in self.subtables:
            self.assertTrue(th.compTables(self.outvis_sorted+subtable,self.refvis_sorted+subtable, [],0.000001,"absolute"))
            
        # Special case for SOURCE which contains many un-defined columns
        self.assertTrue(th.compTables(self.outvis_sorted+'/SOURCE',self.refvis_sorted+'/SOURCE', 
                                      ['POSITION','TRANSITION','REST_FREQUENCY','SYSVEL'],0.000001,"absolute"))     
        
    def compare_main_table_columns(self):
        for col in self.columns:
            if col != "WEIGHT_SPECTRUM" and col != "FLAG_CATEGORY":
                    tmpcolumn = self.columns[:]
                    tmpcolumn.remove(col)
                    self.assertTrue(th.compTables(self.refvis_sorted,self.outvis_sorted,tmpcolumn,self.tolerance[col],self.mode[col]))
    
        
    def post_process(self):
        
        # Sort the output MSs so that they can be compared
        self.sort()
        
        # Compare results for subtables
        self.compare_subtables()
        
        # Compare columns from main table
        self.compare_main_table_columns()
            
    def test_timeaverage_data(self):
        self.outvis = 'test_timeaverage_data-mst.ms'
        self.refvis = 'test_timeaverage_data-split.ms'
        self.outvis_sorted = 'test_timeaverage_data-mst-sorted.ms'
        self.refvis_sorted = 'test_timeaverage_data-split-sorted.ms'        
        
        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='DATA',timeaverage=True,timebin='30s')
        split(vis=self.vis,outputvis=self.refvis,datacolumn='DATA',timebin='30s')
        
        self.generate_tolerance_map()

        self.mode['UVW'] = "percentage"
        self.tolerance['UVW'] = 0.15/100.0    
        
        self.mode['FLAG'] = "absolute"
        self.tolerance['FLAG'] = 1
        
        self.mode['EXPOSURE'] = "absolute"
        self.tolerance['EXPOSURE'] = 1.001
        
        self.mode['INTERVAL'] = "absolute" 
        self.tolerance['INTERVAL'] = 1.0
           
        self.mode['TIME'] = "absolute"
        self.tolerance['TIME'] = 1.5
        
        self.mode['TIME_CENTROID'] = "absolute"
        self.tolerance['TIME_CENTROID'] = 1.075
        
        self.mode['DATA'] = "absolute"  
        self.tolerance['DATA'] = 1.1
        
        self.mode['WEIGHT'] = "percentage"
        self.tolerance['WEIGHT'] = 100
        
        self.mode['SIGMA'] = "absolute"
        self.tolerance['SIGMA'] = 31

        self.post_process()   
        
    def test_timeaverage_model(self):
        self.outvis = 'test_timeaverage_model-mst.ms'
        self.refvis = 'test_timeaverage_model-split.ms'
        self.outvis_sorted = 'test_timeaverage_model-mst-sorted.ms'
        self.refvis_sorted = 'test_timeaverage_model-split-sorted.ms'        
        
        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='MODEL',timeaverage=True,timebin='30s')
        split(vis=self.vis,outputvis=self.refvis,datacolumn='MODEL',timebin='30s')
        
        self.generate_tolerance_map()

        self.mode['UVW'] = "percentage"
        self.tolerance['UVW'] = 0.15/100.0    
        
        self.mode['FLAG'] = "absolute"
        self.tolerance['FLAG'] = 1
        
        self.mode['EXPOSURE'] = "absolute"
        self.tolerance['EXPOSURE'] = 1.001
        
        self.mode['INTERVAL'] = "absolute" 
        self.tolerance['INTERVAL'] = 1.0
           
        self.mode['TIME'] = "absolute"
        self.tolerance['TIME'] = 1.5
        
        self.mode['TIME_CENTROID'] = "absolute"
        self.tolerance['TIME_CENTROID'] = 1.075
        
        self.mode['DATA'] = "absolute"  
        self.tolerance['DATA'] = 1.1
        
        self.mode['WEIGHT'] = "percentage"
        self.tolerance['WEIGHT'] = 100
        
        self.mode['SIGMA'] = "absolute"
        self.tolerance['SIGMA'] = 31

        self.post_process()          
        
    def test_timeaverage_corrected(self):
        self.outvis = 'test_timeaverage_corrected-mst.ms'
        self.refvis = 'test_timeaverage_corrected-split.ms'
        self.outvis_sorted = 'test_timeaverage_corrected-mst-sorted.ms'
        self.refvis_sorted = 'test_timeaverage_corrected-split-sorted.ms'        
        
        mstransform(vis=self.vis,outputvis=self.outvis,datacolumn='CORRECTED',timeaverage=True,timebin='30s')
        split(vis=self.vis,outputvis=self.refvis,datacolumn='CORRECTED',timebin='30s')
        
        self.generate_tolerance_map()

        self.mode['UVW'] = "percentage"
        self.tolerance['UVW'] = 0.15/100.0    
        
        self.mode['FLAG'] = "absolute"
        self.tolerance['FLAG'] = 1
        
        self.mode['EXPOSURE'] = "absolute"
        self.tolerance['EXPOSURE'] = 1.001
        
        self.mode['INTERVAL'] = "absolute" 
        self.tolerance['INTERVAL'] = 1.0
           
        self.mode['TIME'] = "absolute"
        self.tolerance['TIME'] = 1.5
        
        self.mode['TIME_CENTROID'] = "absolute"
        self.tolerance['TIME_CENTROID'] = 1.075
        
        self.mode['DATA'] = "absolute"  
        self.tolerance['DATA'] = 10
        
        self.mode['WEIGHT'] = "percentage"
        self.tolerance['WEIGHT'] = 100
        
        self.mode['SIGMA'] = "absolute"
        self.tolerance['SIGMA'] = 31

        self.post_process()   
        
    def test_timeaverage_and_combine_spws(self):
        self.outvis = 'test_timeaverage_and_combine_spws_single_run.ms'
        self.tmpvis = 'test_timeaverage_and_combine_spws_1st_step.ms'
        self.refvis = 'test_timeaverage_and_combine_spws_2nd_step.ms'
        self.outvis_sorted = 'test_timeaverage_and_combine_spws_single_run_sorted.ms'
        self.refvis_sorted = 'test_timeaverage_and_combine_spws_2nd_step_sorted.ms'
        
        mstransform(vis=self.vis,outputvis=self.outvis,spw='9,10',antenna="0&&1",timerange='14:45:08.50~14:45:09.50',
                    datacolumn='DATA',combinespws=True,timeaverage=True,timebin='2s')
        mstransform(vis=self.vis,outputvis=self.tmpvis,spw='9,10',antenna="0&&1",timerange='14:45:08.50~14:45:09.50',
                    datacolumn='DATA',timeaverage=True,timebin='2s')
        mstransform(vis=self.tmpvis,outputvis=self.refvis,datacolumn='DATA',combinespws=True)
        
        self.generate_tolerance_map()
        
        self.mode['UVW'] = "absolute"
        self.tolerance['UVW'] = 0.000001
        
        self.mode['FLAG'] = "absolute"
        self.tolerance['FLAG'] = 1     
        
        self.mode['EXPOSURE'] = "absolute"
        self.tolerance['EXPOSURE'] = 10000    
        
        self.mode['WEIGHT'] = "absolute"
        self.tolerance['WEIGHT'] = 10000           
        
        self.mode['SIGMA'] = "absolute"
        self.tolerance['SIGMA'] = 10000          
        
        self.mode['TIME_CENTROID'] = "absolute"
        self.tolerance['TIME_CENTROID'] = 0.0001        
        
        self.mode['DATA'] = "absolute"
        self.tolerance['DATA'] = 2   

        self.post_process()         


# Cleanup class 
class Cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms* jupiter6cm.demo*')
        os.system('rm -rf Four_ants_3C286.*ms* g19_d2usb_targets*')
        os.system('rm -rf comb*.*ms* reg*.*ms hann*.*ms favg*.*ms')
        os.system('rm -rf testmms*.*ms parallel*.*ms')

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
#            test_float_column,
            Cleanup]
