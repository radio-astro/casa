import os
import shutil
import unittest
import testhelper as th
import partitionhelper as ph
from tasks import partition, listpartition
from taskinit import *
from __main__ import default

''' Unit Test for task partition'''

    
    
# Path for data
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/gaincal/"

print 'partition tests will use data from '+datapath         

class partition_test(unittest.TestCase):
    
    def setUp(self):
        # Input files
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

    def tearDown(self):
        shutil.rmtree(self.msfile, ignore_errors=True)        
        shutil.rmtree(self.mmsfile, ignore_errors=True)        


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
#            print '%s <--> %s'%(mmsN,msN)
            self.assertEqual(mmsN, msN, 'Nrows in scan=%s differs: mms_nrows=%s <--> ms_nrows=%s'
                             %(s, mmsN, msN))
 
        # Add comparison for spw Ids
        

        
#         TO DO: Compare both table using compTables when sorting in partition is fixed
#        self.assertTrue(th.compTables(self.msfile, self.mmsfile, 
#                                      ['FLAG','FLAG_CATEGORY','TIME_CENTROID','DATA']))

        # Compare the DATA column
#        self.assertTrue(th.compVarColTables(self.msfile,self.mmsfile,'DATA'))

    def test_nomms(self):
        '''Partition: Craete a normal MS with createmms=False'''
        partition(vis=self.msfile, outputvis=self.mmsfile, createmms=False)
        
        self.assertTrue(os.path.exists(self.mmsfile), 'MMS was not created for this test')
        
        # Compare both tables. Ignore the DATA column and compare it in next line
        self.assertTrue(th.compTables(self.msfile, self.mmsfile, 
                                      ['FLAG_CATEGORY','FLAG','DATA']))
        
        # Compare the DATA column
        self.assertTrue(th.compVarColTables(self.msfile,self.mmsfile,'DATA'))

def suite():
    return [partition_test]

