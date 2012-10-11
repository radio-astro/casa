import os
import shutil
import time
import unittest
import testhelper as th
import partitionhelper as ph
from tasks import *
from taskinit import *
from __main__ import default
from parallel.parallel_task_helper import ParallelTaskHelper


''' Unit Tests for task partition'''

# jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
if os.environ.has_key('BYPASS_SEQUENTIAL_PROCESSING'):
    ParallelTaskHelper.bypassParallelProcessing(1)

    
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
        if os.path.lexists(fpath):        
            shutil.copytree(fpath, self.msfile, symlinks=True)
        else:
            self.fail('Data does not exist -> '+fpath)

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
        if os.path.lexists(fpath):        
            shutil.copytree(fpath, self.msfile, symlinks=True)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('partition')


class partition_test1(test_base):
    
    def setUp(self):
        self.setUp_ngc4826()

    def tearDown(self):
        shutil.rmtree(self.msfile, ignore_errors=True)        
        shutil.rmtree(self.mmsfile, ignore_errors=True)        
        shutil.rmtree(self.msfile+'.flagversions', ignore_errors=True)        
        shutil.rmtree(self.mmsfile+'.flagversions', ignore_errors=True)        

    def test_nomms(self):
        '''Partition: Create a normal MS with createmms=False'''
        partition(vis=self.msfile, outputvis=self.mmsfile, createmms=False)
        time.sleep(10)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Compare both tables. Ignore the DATA column and compare it in next line
        self.assertTrue(th.compTables(self.msfile, self.mmsfile, 
                                      ['FLAG_CATEGORY','FLAG','WEIGHT_SPECTRUM','DATA']))
        
        # Compare the DATA column
        self.assertTrue(th.compVarColTables(self.msfile,self.mmsfile,'DATA'))

    def test_default(self):
        '''Partition: create an MMS with default values'''
        partition(vis=self.msfile, outputvis=self.mmsfile)
        time.sleep(10)
        
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
        
#         TO DO: Compare both table using compTables when sorting in partition is fixed
        self.assertTrue(th.compTables(self.msfile, self.mmsfile, 
                                      ['FLAG','FLAG_CATEGORY','TIME_CENTROID',
                                       'WEIGHT_SPECTRUM','DATA']))

        # Compare the DATA column
#        self.assertTrue(th.compVarColTables(self.msfile,self.mmsfile,'DATA'))


    def test_scan_selection(self):
        '''Partition: create an MMS using scan selection'''
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='scan', scan='1,2,3,11')
        time.sleep(10)
        
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
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='spw', observation='2')
        time.sleep(10)
        
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
 
        # Compare spw IDs
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            ms_spw = ph.getSpwIds(self.msfile, s, selection=mysel)
            self.assertEqual(mms_spw, ms_spw, 'list of spws in scan=%s differs: '\
                             'mms_spw=%s <--> ms_spw=%s' %(s, mmsN, msN))


    def test_spw_selection(self):
        '''Partition: create an MMS separated by spws with spw=2,4 selection'''
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='spw', spw='2,4')
        time.sleep(10)
        
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
 
        # Compare spw IDs
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            ms_spw = ph.getSpwIds(self.msfile, s, selection=mysel)
            self.assertEqual(mms_spw, ms_spw, 'list of spws in scan=%s differs: '\
                             'mms_spw=%s <--> ms_spw=%s' %(s, mms_spw, ms_spw))

#    def addCleanup(self, function, *args, **kwargs):
    
class partition_test2(test_base):
    
    def setUp(self):
        self.setUp_fourants()

    def tearDown(self):
        shutil.rmtree(self.msfile, ignore_errors=True)        
        shutil.rmtree(self.mmsfile, ignore_errors=True)        
        shutil.rmtree(self.msfile+'.flagversions', ignore_errors=True)        
        shutil.rmtree(self.mmsfile+'.flagversions', ignore_errors=True)        

    # The followin test fails in the OSX platforms if the full MS is used to
    # create the MMS. It does not fail in partition, but in the ms.getscansummary()
    # methods. Check this soon
    def test_sepaxis(self):
        '''Partition: separationaxis=both'''        
        partition(vis=self.msfile, outputvis=self.mmsfile, spw='0~11',separationaxis='both')
#        partition(vis=self.msfile, outputvis=self.mmsfile,separationaxis='both')
        time.sleep(10)
        
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

        
    def test_all_columns(self):
        '''Partition: datacolumn=all'''
        partition(vis=self.msfile, outputvis=self.mmsfile, datacolumn='all')
        time.sleep(10)
        
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
                  spw='1~4,10,11')
        time.sleep(10)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
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

        # Compare spw IDs
        for s in slist:
            mms_spw = ph.getSpwIds(self.mmsfile, s)
            ms_spw = ph.getSpwIds(self.msfile, s, selection=mysel)
            self.assertEqual(mms_spw, ms_spw, 'list of spws in scan=%s differs: '\
                             'mms_spw=%s <--> ms_spw=%s' %(s, mms_spw, ms_spw))
 
        
    def test_numsubms(self):
        '''Partition: small numsubms value'''
        # There are 16 spws; we want only 6 sub-MSs.
        partition(vis=self.msfile, outputvis=self.mmsfile, separationaxis='spw',
                  numsubms=6)
        time.sleep(10)
        
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
        
    def test_flagversions(self):
        '''Partition: check that the .flagversions is created'''
                
        # Run partition and create the .flagversions
        partition(vis=self.msfile, outputvis=self.mmsfile, createmms=True)
        self.assertTrue(os.path.exists(self.mmsfile+'.flagversions'))
 
         # Check that the number of backups in MMS is correct
        aflocal = casac.agentflagger()
        aflocal.open(self.mmsfile)
        nv = aflocal.getflagversionlist()
        aflocal.done()
        self.assertEqual(len(nv), 3)
               
        # Run flagdata on MMS to check if it works well.
        flagdata(vis=self.mmsfile, mode='unflag', flagbackup=True)
        
        # Check that the number of backups in MMS is correct
        aflocal = casac.agentflagger()
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
        partition(vis=self.msfile, outputvis=self.mmsfile, createmms=True)
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
        
          
def suite():
    return [partition_test1, partition_test2]


















