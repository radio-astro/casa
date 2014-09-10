import os
import shutil
import time
import unittest
import testhelper as th
import partitionhelper as ph
from tasks import partition, flagdata, flagmanager, split, setjy, listpartition
from taskinit import msmdtool, mstool, aftool, tbtool
from __main__ import default
from parallel.parallel_task_helper import ParallelTaskHelper


''' Unit Tests for task partition'''

# jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
if os.environ.has_key('BYPASS_PARALLEL_PROCESSING'):
    ParallelTaskHelper.bypassParallelProcessing(1)
   
    
def compareSubTables(input,reference,order=None,excluded_cols=[]):
    
    tbinput = tbtool()
    tbinput.open(input)
    if order is not None:
        tbinput_sorted = tbinput.taql("SELECT * from " + input + " order by " + order)
    else:
        tbinput_sorted = tbinput
    
    tbreference = tbtool()
    tbreference.open(reference)
    if order is not None:
        tbreference_sorted = tbreference.taql("SELECT * from " + reference + " order by " + order)
    else:
        tbreference_sorted = tbreference
    
    columns = tbinput.colnames()
    for col in columns:
        if not col in excluded_cols:
            col_input = tbinput_sorted.getcol(col)
            col_reference = tbreference_sorted.getcol(col)
            if not (col_input == col_reference).all():
                tbinput.close()
                tbreference.close()
                return (False,col)
    
    tbinput.close()
    tbreference.close()
    return (True,"OK")
    

    
class test_base(unittest.TestCase):
    
    def setUp_ngc4826(self):
        
        datapath = os.environ.get('CASAPATH').split()[0] + \
                    "/data/regression/unittest/gaincal/"
                    
        # Input MS contain 4 obsID, 11 scans and 4 spws
        self.prefix = 'ngc4826'
        self.msfile = self.prefix + '.ms'  
        
        # Output files                      
        self.mmsfile = self.prefix + '.mms'
        
        fpath = os.path.join(datapath,self.msfile)
#         if os.path.lexists(fpath):        
#             shutil.copytree(fpath, self.msfile, symlinks=True)
#         else:
#             self.fail('Data does not exist -> '+fpath)
        if not os.path.exists(self.msfile):        
            shutil.copytree(fpath, self.msfile, symlinks=True)
        else:
            print 'MS is already around, no need to copy it.'

        default('partition')

    def setUp_fourants(self):
        
        datapath = os.environ.get('CASAPATH').split()[0] + \
                    "/data/regression/unittest/flagdata/"
                    
        # Input MS contain 2 scans and 16 spws. It has all data columns
        self.prefix = 'Four_ants_3C286'
        self.msfile = self.prefix + '.ms'  
        
        # Output files                      
        self.mmsfile = self.prefix + '.mms'
        
        fpath = os.path.join(datapath,self.msfile)
#         if os.path.lexists(fpath):        
#             shutil.copytree(fpath, self.msfile, symlinks=True)
#         else:
#             self.fail('Data does not exist -> '+fpath)
        if not os.path.exists(self.msfile):        
            shutil.copytree(fpath, self.msfile, symlinks=True)
        else:
            print 'MS is already around, no need to copy it.'

        default('partition')

    def setUp_floatcol(self):
        datapath = os.environ.get('CASAPATH').split()[0] + \
                    "/data/regression/unittest/flagdata/"
                    
        # 15 rows, 3 scans, 9 spw, mixed chans, XX,YY, FLOAT_DATA col
        self.prefix = "SDFloatColumn"
        self.msfile = 'SDFloatColumn.ms'
        
        # Output files                      
        self.mmsfile = self.prefix + '.mms'
        
        fpath = os.path.join(datapath,self.msfile)
        
        if not os.path.exists(self.msfile):        
            shutil.copytree(fpath, self.msfile, symlinks=True)
        else:
            print 'MS is already around, no need to copy it.'

        default('partition')
        
    def setUp_sub_tables_evla(self):
        
        # Define the root for the data files
        datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

        self.vis = 'test-subtables-evla.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default('partition')   
        
    def setUp_sub_tables_alma(self):
        
        # Define the root for the data files
        datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

        self.vis = 'test-subtables-alma.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default('partition')   

    def cleanup(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.vis +'.flagversions')
 
class partition_test1(test_base):
    
    def setUp(self):
        self.setUp_ngc4826()

    def tearDown(self):
#        shutil.rmtree(self.msfile, ignore_errors=True)        
        shutil.rmtree(self.mmsfile, ignore_errors=True)        
#        shutil.rmtree(self.msfile+'.flagversions', ignore_errors=True)        
        shutil.rmtree(self.mmsfile+'.flagversions', ignore_errors=True)        
        os.system("rm -rf *ms_sorted*")    

    def test_nomms(self):
        '''Partition: Create a normal MS with createmms=False'''
        partition(vis=self.msfile, outputvis=self.mmsfile, createmms=False)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Sort the output MSs so that they can be compared
        myms = mstool()
        
        myms.open(self.msfile)
        myms.sort('ms_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()
        
        myms.open(self.mmsfile)
        myms.sort('mms_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()

        # Compare both tables. Ignore the DATA column and compare it in next line
        self.assertTrue(th.compTables('ms_sorted.ms','mms_sorted.ms', 
                                      ['FLAG_CATEGORY','FLAG','WEIGHT_SPECTRUM','DATA']))
        
        # Compare the DATA column
        self.assertTrue(th.compVarColTables('ms_sorted.ms','mms_sorted.ms','DATA'))


    def test_default_scan(self):
        '''Partition: create an MMS with default values and axis=scan'''
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='scan')
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)
        
        # Compare nrows of all scans
        slist = ph.getMMSScans(thisdict)
        
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows(self.msfile, s)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))
 
        # Compare spw IDs
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            ms_spw = ph.getSpwIds(self.msfile, s)
            self.assertEqual(mms_spw, ms_spw, 'list of spws in scan=%s differs: '\
                             'mms_spw=%s <--> ms_spw=%s' %(s, mmsN, msN))

        # Sort the output MSs so that they can be compared
        myms = mstool()
        
        myms.open(self.msfile)
        myms.sort('ms_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()
        
        myms.open(self.mmsfile)
        myms.sort('mms_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()

        self.assertTrue(th.compTables('ms_sorted.ms', 'mms_sorted.ms', 
                                      ['FLAG','FLAG_CATEGORY','TIME_CENTROID',
                                       'WEIGHT_SPECTRUM','DATA']))
        
        # Compare the DATA column
        self.assertTrue(th.compVarColTables('ms_sorted.ms','mms_sorted.ms','DATA'))

    def test_scan_selection(self):
        '''Partition: create an MMS using scan selection'''
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='scan', scan='1,2,3,11',
                  flagbackup=False, disableparallel=True)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)
        
        # Compare nrows of all scans in selection
        slist = ph.getMMSScans(thisdict)       
        self.assertEqual(slist.__len__(), 4)
        
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows(self.msfile, s)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))
 
        # Compare spw IDs
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            ms_spw = ph.getSpwIds(self.msfile, s)
            self.assertEqual(mms_spw, ms_spw, 'list of spws in scan=%s differs: '\
                             'mms_spw=%s <--> ms_spw=%s' %(s, mmsN, msN))

    def test_spw_separation(self):
        '''Partition: create an MMS separated by spws with observation selection'''
        # NOTE: ms.getscansummary() used in ph.getScanNrows does not honour several observation
        #       IDs, therefore I need to selection by obs id in partition
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='spw', observation='2',
                  flagbackup=False, disableparallel=True)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)

        # Dictionary with selection to compare with original MS
        mysel = {'observation':'2'}
        
        # Compare nrows of all scans in MMS and MS
        slist = ph.getMMSScans(thisdict)
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows(self.msfile, s, selection=mysel)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))
 
        # spwids are re-indexed. The expected IDs are:
        # ms_spw = 2 --> mms_spw = 0
        # ms_spw = 3 --> mms_spw = 1, etc.
        # Check that MMS spw IDs have been re-indexed properly
        indexed_ids = range(4)
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            self.assertEqual(mms_spw, indexed_ids, 'spw IDs were not properly re-indexed')


    def test_spw_selection(self):
        '''Partition: create an MMS separated by spws with spw=2,4 selection'''
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='spw', spw='2,4',
                  flagbackup=False, disableparallel=True)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)
        
        # Dictionary with selection to compare with original MS
        mysel = {'spw':'2,4'}
        
        # Compare nrows of all scans in selection
        slist = ph.getMMSScans(thisdict)
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows(self.msfile, s, selection=mysel)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))
 
        # spwids are re-indexed. The expected IDs are:
        # ms_spw = 2 --> mms_spw = 0
        # ms_spw = 4 --> mms_spw = 1
        # Check that MMS spw IDs have been re-indexed properly
        indexed_ids = range(2)
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            self.assertEqual(mms_spw, indexed_ids, 'spw IDs were not properly re-indexed')

    def test_model_keys(self):
        '''partition: CAS-4398, handle the MODEL keywords correctly'''
        
        print '*** Check that MODEL_DATA is not present in MS first'
        mytb = tbtool()
        try:
            mytb.open(self.msfile+'/MODEL_DATA')
        except Exception, instance:
            print '*** Expected exception. \"%s\"'%instance
            mytb.close()
        
        inpms = 'ngc4826Jy.ms'
        shutil.copytree(self.msfile, inpms)
        
        # First, run setjy to create the SOURCE_MODEL column
        setjy(vis=inpms,field='0',fluxdensity=[23.0,0.,0.,0.],standard='manual', scalebychan=False)
        
        # Now create an MMS from it. The SOURCE_MODEL column should be there too
        partition(vis=inpms,outputvis=self.mmsfile, observation='1',spw='1',
                  scan='1,2,3', disableparallel=True, flagbackup=False)  

        print 'Check the SOURCE_MODEL columns....'
        mytb = tbtool()
        mytb.open(inpms+'/SOURCE')
        msdict = mytb.getcell('SOURCE_MODEL', 0)
        mytb.close()
        
        mytb.open(self.mmsfile+'/SOURCE')
        mmsdict = mytb.getcell('SOURCE_MODEL', 0)
        mytb.close()

        self.assertEqual(set(msdict['cl_0'].keys()),set(mmsdict['cl_0'].keys()))
        
        # Check that the real MODEL_DATA column is not created in MMS
        try:
            mytb.open(self.mmsfile+'/MODEL_DATA')
        except Exception, instance:
            print '*** Expected exception. \"%s\"'%instance
            mytb.close()
        

    
class partition_test2(test_base):
    
    def setUp(self):
        self.setUp_fourants()

    def tearDown(self):
#        shutil.rmtree(self.msfile, ignore_errors=True)        
        shutil.rmtree(self.mmsfile, ignore_errors=True)        
#        shutil.rmtree(self.msfile+'.flagversions', ignore_errors=True)        
        shutil.rmtree(self.mmsfile+'.flagversions', ignore_errors=True) 
        os.system("rm -rf split30.ms* *ms_sorted*")       

    def test_default(self):
        '''Partition: create an MMS with default values in parallel'''
        
        # First split off one scan to run the test faster
        split(vis=self.msfile, outputvis='split30.ms', datacolumn='DATA', scan='30')
        msfile = 'split30.ms'

        partition(vis=msfile, outputvis=self.mmsfile)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Gather several metadata information
        # for the MS
        mdlocal1 = msmdtool()
        mdlocal1.open(msfile)
        ms_rows = mdlocal1.nrows()
        ms_nscans = mdlocal1.nscans()
        ms_nspws = mdlocal1.nspw()
        ms_scans = mdlocal1.scannumbers()
        mdlocal1.close()        
          
        # for the MMS
        mdlocal2 = msmdtool()
        mdlocal2.open(self.mmsfile)
        mms_rows = mdlocal2.nrows()
        mms_nscans = mdlocal2.nscans()
        mms_nspws = mdlocal2.nspw()
        mms_scans = mdlocal2.scannumbers()
        mdlocal2.close()        
          
        # Compare the number of rows
        self.assertEqual(ms_rows, mms_rows, 'Compare total number of rows in MS and MMS')
        self.assertEqual(ms_nscans, mms_nscans, 'Compare number of scans')
        self.assertEqual(ms_nspws, mms_nspws, 'Compare number of spws')
          
        # Compare the scans
        self.assertEqual(ms_scans.all(), mms_scans.all(), 'Compare all scan IDs')
  
        try:
            mdlocal1.open(msfile)
            mdlocal2.open(self.mmsfile)
          
            # Compare the spws
            for i in ms_scans:                
                msi = mdlocal1.spwsforscan(i)
                mmsi = mdlocal2.spwsforscan(i)
                self.assertEqual(msi.all(), mmsi.all(), 'Compare spw Ids for a scan')
        finally:          
            mdlocal1.close()
            mdlocal2.close()               

        # Sort the output MSs so that they can be compared
        myms = mstool()
        
        myms.open(msfile)
        myms.sort('ms_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()
        
        myms.open(self.mmsfile)
        myms.sort('mms_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()

        self.assertTrue(th.compTables('ms_sorted.ms', 'mms_sorted.ms', 
                                      ['FLAG','FLAG_CATEGORY','TIME_CENTROID',
                                       'WEIGHT_SPECTRUM','DATA']))

        # Compare the DATA column
        self.assertTrue(th.compVarColTables('ms_sorted.ms', 'mms_sorted.ms','DATA'))
        
        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.mmsfile)
        self.assertEqual(sepaxis, 'scan,spw', 'Partition did not write AxisType correctly in MMS')
        
    def test_default_sequential(self):
        '''Partition: create an MMS with default values in sequential'''
        partition(vis=self.msfile, outputvis=self.mmsfile, disableparallel=True)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)
        
        # Get scans of MMS
        slist = ph.getMMSScans(thisdict)
         
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows(self.msfile, s)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))
 
        # Compare spw IDs of MS and MMS
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            ms_spw = ph.getSpwIds(self.msfile, s)
            self.assertEqual(mms_spw, ms_spw, 'list of spws in scan=%s differs: '\
                             'mms_spw=%s <--> ms_spw=%s' %(s, mmsN, msN))
            
        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.mmsfile)
        self.assertEqual(sepaxis, 'scan,spw', 'Partition did not write AxisType correctly in MMS')


        # Sort the output MSs so that they can be compared
#         myms = mstool()
#         
#         myms.open(self.msfile)
#         myms.sort('ms_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
#         myms.done()
#         
#         myms.open(self.mmsfile)
#         myms.sort('mms_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
#         myms.done()
#         
# #         TO DO: Compare both table using compTables when sorting in partition is fixed
#         self.assertTrue(th.compTables('ms_sorted.ms', 'mms_sorted.ms', 
#                                       ['FLAG','FLAG_CATEGORY','TIME_CENTROID',
#                                        'WEIGHT_SPECTRUM','MODEL_DATA','DATA',
#                                        'CORRECTED_DATA']))


    # The following test fails in the OSX platforms if the full MS is used to
    # create the MMS. It does not fail in partition, but in the ms.getscansummary()
    # methods. Check this soon
    def test_sepaxis(self):
        '''Partition: separationaxis=auto'''        
        partition(vis=self.msfile, outputvis=self.mmsfile, spw='0~11',separationaxis='auto',
                  flagbackup=False, disableparallel=True)
#        partition(vis=self.msfile, outputvis=self.mmsfile,separationaxis='auto')
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')

        # Dictionary with selection to compare with original MS
        mysel = {'spw':'0~11'}
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)
        
        # Compare nrows of all scans in selection
        slist = ph.getMMSScans(thisdict)
        self.assertEqual(slist.__len__(), 2)
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows(self.msfile, s, selection=mysel)
#            msN = ph.getScanNrows(self.msfile, s)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))

        # Compare spw IDs
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            ms_spw = ph.getSpwIds(self.msfile, s, selection=mysel)
#            ms_spw = ph.getSpwIds(self.msfile, s)
            self.assertEqual(mms_spw, ms_spw, 'list of spws in scan=%s differs: '\
                             'mms_spw=%s <--> ms_spw=%s' %(s, mms_spw, ms_spw))

        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.mmsfile)
        self.assertEqual(sepaxis, 'scan,spw', 'Partition did not write AxisType correctly in MMS')
        
    def test_all_columns(self):
        '''Partition: datacolumn=all'''
        partition(vis=self.msfile, outputvis=self.mmsfile, datacolumn='all',
                  flagbackup=False, disableparallel=True)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)
        
        # Compare nrows of all scans in selection
        slist = ph.getMMSScans(thisdict)
        self.assertEqual(slist.__len__(), 2)
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows(self.msfile, s)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))

        # Compare spw IDs
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            ms_spw = ph.getSpwIds(self.msfile, s)
            self.assertEqual(mms_spw, ms_spw, 'list of spws in scan=%s differs: '\
                             'mms_spw=%s <--> ms_spw=%s' %(s, mms_spw, ms_spw))

    def test_scan_spw(self):
        '''Partition: separationaxis=scan with spw selection'''
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='scan',
                  spw='1~4,10,11', flagbackup=False)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        self.assertTrue(os.path.exists(self.msfile),'Make sure the input MS is not deleted inside the task')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)

        # Dictionary with selection to compare with original MS
        mysel = {'spw':'1~4,10,11'}
        
        # Compare nrows of all scans in selection
        slist = ph.getMMSScans(thisdict)
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows(self.msfile, s, selection=mysel)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))

        # The comparison should be
        # ms_spw = 1 --> mms_spw = 0
        # ms_spw = 2 --> mms_spw = 1, etc.
        # Check that MMS spw IDs have been re-indexed properly
        indexed_ids = range(6)
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            self.assertEqual(mms_spw, indexed_ids, 'spw IDs were not properly re-indexed')
            
        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.mmsfile)
        self.assertEqual(sepaxis, 'scan', 'Partition did not write AxisType correctly in MMS')
 
    def test_numsubms(self):
        '''Partition: small numsubms value'''
        # There are 16 spws; we want only 6 sub-MSs.
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='spw',
                  numsubms=6, flagbackup=False, disableparallel=True)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=self.mmsfile, createdict=True)
        
        # Check the number of sub-MSs
        mmslist = []
        klist = thisdict.keys()
        for kk in klist:
            mmslist.append(thisdict[kk]['MS'])
        
        nsubms = mmslist.__len__()
        self.assertEqual(nsubms, 6, 'There should be only 6 sub-MSs')
        
        # Check that spw list is the same in MS and MMS
        spwlist = ph.getMMSSpwIds(thisdict)        

        # Reference list of scans in MS
        slist = ph.getScanList(self.msfile)
        setlist = set([])
        for s in slist:
            ms_spw = ph.getSpwIds(self.msfile, s)
            for a in ms_spw:
                setlist.add(a)

        self.assertEqual(list(setlist), spwlist)

        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.mmsfile)
        self.assertEqual(sepaxis, 'spw', 'Partition did not write AxisType correctly in MMS')
        
    def test_flagversions(self):
        '''Partition: check that the .flagversions is created'''
                
        # Run partition and create the .flagversions
        partition(vis=self.msfile, outputvis=self.mmsfile, createmms=True,
                  disableparallel=True)
        self.assertTrue(os.path.exists(self.mmsfile+'.flagversions'))
 
         # Check that the number of backups in MMS is correct
        aflocal = aftool()
        aflocal.open(self.mmsfile)
        nv = aflocal.getflagversionlist()
        aflocal.done()
        self.assertEqual(len(nv), 3)
               
        # Run flagdata on MMS to check if it works well.
        flagdata(vis=self.mmsfile, mode='unflag', flagbackup=True)
        
        # Check that the number of backups in MMS is correct
        aflocal = aftool()
        aflocal.open(self.mmsfile)
        nvref = aflocal.getflagversionlist()
        aflocal.done()
        self.assertEqual(len(nvref), 4)
        
    def test_flagsrestore(self):
        '''Partition: check that we can restore the flags'''
        # Delete any flagversions
        if os.path.exists(self.msfile+'.flagversions'):
            shutil.rmtree(self.msfile+'.flagversions')
            
        # Unflag the MS
        flagdata(vis=self.msfile, mode='unflag', flagbackup=False)
        
        # Run partition and create the .flagversions
        partition(vis=self.msfile, outputvis=self.mmsfile, createmms=True,
                  disableparallel=True)
        self.assertTrue(os.path.exists(self.mmsfile+'.flagversions'))
        
        # Flag spw=9 and then spw=7 in the MMS
        flagdata(vis=self.mmsfile, mode='manual', spw='9', flagbackup=True)
        flagdata(vis=self.mmsfile, mode='manual', spw='7', flagbackup=True)
        
        # There should be flags in spw=7 and 9 in MMS
        res = flagdata(vis=self.mmsfile, mode='summary')
        self.assertEqual(res['flagged'],549888)
        
        # Restore the original flags (there should be none)
        flagmanager(vis=self.mmsfile, mode='restore', versionname='partition_1')
        res = flagdata(vis=self.mmsfile, mode='summary')
        self.assertEqual(res['flagged'],0)
        
    def test_channels1(self):
        '''partition: create MMS with spw separation and channel selections'''
        partition(vis=self.msfile, outputvis=self.mmsfile, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw', disableparallel=True, flagbackup=False)
                            
        self.assertTrue(os.path.exists(self.mmsfile))
        
        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(self.mmsfile, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

    def test_channels2(self):
        '''partition: create MMS with spw/scan separation and channel selections'''
        partition(vis=self.msfile, outputvis=self.mmsfile, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='auto', disableparallel=True, flagbackup=False)
                            
        self.assertTrue(os.path.exists(self.mmsfile))
        
        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.mmsfile, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(self.mmsfile, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

    def test_channels3(self):
        '''partition: verify spw sub-table consolidation'''
        partition(vis=self.msfile, outputvis=self.mmsfile, spw='3,5:10~19,7,9,11,13,15',
                    createmms=True,separationaxis='spw', flagbackup=False)       
                             
        self.assertTrue(os.path.exists(self.mmsfile))
        
        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(self.mmsfile, 7, 10, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.mmsfile)
        self.assertEqual(sepaxis, 'spw', 'Partition did not write AxisType correctly in MMS')

class partition_float(test_base):
    def setUp(self):
        self.setUp_floatcol()

    def tearDown(self):
        shutil.rmtree(self.mmsfile, ignore_errors=True)        
        shutil.rmtree(self.mmsfile+'.flagversions', ignore_errors=True) 

    def test_split_float(self):
        '''partition: split an MS with FLOAT_DATA'''
        partition(vis=self.msfile, outputvis=self.mmsfile,spw='1,3,5',
                  createmms=False,datacolumn='FLOAT_DATA', disableparallel=True,flagbackup=False)
        
        ret = th.verifyMS(self.mmsfile, 3, 512, 0, [], ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.mmsfile, 3, 512, 1, [], ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.mmsfile, 3, 1024, 2, [], ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_mms_float(self):
        '''partition: '''
        partition(vis=self.msfile, outputvis=self.mmsfile,spw='1,3,5',
                  datacolumn='FLOAT_DATA', disableparallel=True,flagbackup=False)
        
        ret = th.verifyMS(self.mmsfile, 3, 512, 0, [], ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.mmsfile, 3, 512, 1, [], ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.mmsfile, 3, 1024, 2, [], ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        
class test_partiton_subtables_evla(test_base):
    '''Test effect of SPW of partition on EVLA sub-tables'''
    
    def setUp(self):
        
        self.vis = ""
        self.outputms = ""
                
        self.setUp_sub_tables_evla()
        
    def tearDown(self):
        os.system("rm -rf " + self.vis)
        os.system("rm -rf " + self.outputms)
    
    def test_merge_subtables_after_partiton_evla(self):
        '''mstransform: Check that sub-tables are properly merged after partiton'''
        
        self.outputms = "parted.ms"
        
        partition(self.vis, outputvis=self.outputms,separationaxis='spw',numsubms=4)
        
        subtable = "/FEED"
        sort_order = "SPECTRAL_WINDOW_ID, ANTENNA_ID, FEED_ID, TIME"
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,sort_order)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")
        
        subtable = "/CALDEVICE"
        sort_order = "SPECTRAL_WINDOW_ID, ANTENNA_ID, FEED_ID, TIME"
        excluded_cols = ['CAL_LOAD_NAMES','NOISE_CAL','CAL_EFF','TEMPERATURE_LOAD']
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,sort_order,excluded_cols)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")       
        
        subtable = "/SYSPOWER"
        sort_order = "SPECTRAL_WINDOW_ID, ANTENNA_ID, FEED_ID, TIME"
        excluded_cols = ['SWITCHED_DIFF','SWITCHED_SUM','REQUANTIZER_GAIN','TANT_SPECTRUM']
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,sort_order,excluded_cols)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")   
        
        subtable = "/SOURCE"
        sort_order = "SPECTRAL_WINDOW_ID, TIME, SOURCE_ID"
        excluded_cols = ['POSITION','TRANSITION','REST_FREQUENCY','SYSVEL']        
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,sort_order,excluded_cols)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")         
        
        subtable = "/SPECTRAL_WINDOW"
        excluded_cols = ['ASSOC_SPW_ID','ASSOC_NATURE']
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,None,excluded_cols)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")   
        
class test_partiton_subtables_alma(test_base):
    '''Test effect of SPW of partition on ALMA sub-tables'''
    
    def setUp(self):
        
        self.vis = ""
        self.outputms = ""
                
        self.setUp_sub_tables_alma()
        
    def tearDown(self):
        os.system("rm -rf " + self.vis)
        os.system("rm -rf " + self.outputms)
    
    def test_merge_subtables_after_partiton_alma(self):
        '''mstransform: Check that sub-tables are properly merged after partiton'''
        
        self.outputms = "parted.ms"
        
        partition(self.vis, outputvis=self.outputms,separationaxis='spw',numsubms=4,flagbackup=False)
        
        subtable = "/FEED"
        sort_order = "SPECTRAL_WINDOW_ID, ANTENNA_ID, FEED_ID, TIME"
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,sort_order)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")
        
        subtable = "/CALDEVICE"
        sort_order = "SPECTRAL_WINDOW_ID, ANTENNA_ID, FEED_ID, TIME"
        excluded_cols = ['CAL_LOAD_NAMES','NOISE_CAL','CAL_EFF','TEMPERATURE_LOAD']
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,sort_order,excluded_cols)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")       
        
        subtable = "/SYSCAL"
        sort_order = "SPECTRAL_WINDOW_ID, ANTENNA_ID, FEED_ID, TIME"
        excluded_cols = ['TCAL_SPECTRUM','TRX_SPECTRUM','TSKY_SPECTRUM','TSYS_SPECTRUM','TANT_SPECTRUM','TANT_TSYS_SPECTRUM']
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,sort_order,excluded_cols)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")   
        
        subtable = "/SOURCE"
        sort_order = "SPECTRAL_WINDOW_ID, TIME, SOURCE_ID"
        excluded_cols = ['POSITION','TRANSITION','REST_FREQUENCY','SYSVEL']        
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,sort_order,excluded_cols)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")         
        
        subtable = "/SPECTRAL_WINDOW"
        excluded_cols = ['ASSOC_SPW_ID','ASSOC_NATURE']
        res = compareSubTables(self.outputms+subtable,self.vis+subtable,None,excluded_cols)
        self.assertTrue(res[0],"Error comparing " + res[1] + " column from " + subtable + " sub-table")           
        

# Cleanup class 
class partition_cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc4826*.*ms* Four_ants_3C286.*ms*')

    def test_runTest(self):
        '''partition: Cleanup'''
        print 'Cleaning up after test_partition'
          
def suite():
    return [partition_test1, 
            partition_test2, 
            partition_float,
            test_partiton_subtables_evla,
            test_partiton_subtables_alma,
            partition_cleanup]


















