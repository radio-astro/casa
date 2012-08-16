import os
import shutil
import unittest
import testhelper as th
import partitionhelper as ph
from tasks import partition, listpartition
from taskinit import *
from __main__ import default


''' Unit Tests for task partition'''

    
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
            shutil.copytree(fpath, self.msfile)
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
            shutil.copytree(fpath, self.msfile)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('partition')


class partition_test1(test_base):
    
    def setUp(self):
        self.setUp_ngc4826()

    def tearDown(self):
        shutil.rmtree(self.msfile, ignore_errors=True)        
        shutil.rmtree(self.mmsfile, ignore_errors=True)        


    def test_nomms(self):
        '''Partition: Create a normal MS with createmms=False'''
        partition(vis=self.msfile, outputvis=self.mmsfile, createmms=False)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Compare both tables. Ignore the DATA column and compare it in next line
        self.assertTrue(th.compTables(self.msfile, self.mmsfile, 
                                      ['FLAG_CATEGORY','FLAG','WEIGHT_SPECTRUM','DATA']))
        
        # Compare the DATA column
        self.assertTrue(th.compVarColTables(self.msfile,self.mmsfile,'DATA'))

    def test_default(self):
        '''Partition: create an MMS with default values'''
        partition(vis=self.msfile, outputvis=self.mmsfile)
        
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
        
    def test_all_columns(self):
        '''Partition: datacolumn=all'''
        partition(vis=self.msfile, outputvis=self.mmsfile, datacolumn='all')
        
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
        
    
def suite():
    return [partition_test1, partition_test2]

