import os
import sys
import shutil
import string
import listing as lt
import hashlib
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task listobs. It tests the following parameters:
    vis:        wrong and correct values
    selectdata: several data selection parameters
    verbose     true or false
    listfile:   save on a file
    
'''

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listobs/'

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/listobs/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR
    else:
        print 'WARN: directory '+DATADIR+' does not exist'

print 'listobs tests will use data from '+datapath         

# Reference files
reffile = datapath+'reflistobs'

# Input and output names
msfile1 = 'ngc5921_ut.ms'
msfile2 = 'uid___X02_X3d737_X1_01_small.ms'

def _sha1it(filename):
    blocksize = 65536
    hasher = hashlib.sha1()
    with open(filename, 'rb') as afile:
        buf = afile.read(blocksize)
        while len(buf) > 0:
            hasher.update(buf)
            buf = afile.read(blocksize)
    return hasher.hexdigest()

class listobs_test1(unittest.TestCase):

    def setUp(self):
        self.res = None
        if (not os.path.lexists(msfile1)):            
            shutil.copytree(datapath+msfile1, msfile1, symlinks=True)

        if (not os.path.lexists(msfile2)):            
            shutil.copytree(datapath+msfile2, msfile2, symlinks=True)
        
        default(listobs)
        
            
    def test1(self):
        '''Listobs 1: Input MS'''
        self.assertTrue(listobs(vis=msfile1, listunfl=True))

    def test2(self):
        '''Listobs 2: CSV-591. Check if long field names are fully displayed'''
        ms.open(msfile1)
        res = ms.summary(True, listunfl=True)
        ms.close()
        name = res['field_0']['name']
        self.assertFalse(name.__contains__('*'), "Field name contains a *")
        name = res['scan_7']['0']['FieldName']
        self.assertFalse(name.__contains__('*'), "Field name contains a *")
        
    def test3(self):
        '''Listobs 3: CAS-2751. Check that ALMA MS displays one row per scan'''
        ms.open(msfile2)
        res = ms.summary(True, listunfl=True)
        ms.close()
        # Begin and end times should be different
        btime = res['scan_1']['0']['BeginTime']
        etime = res['scan_1']['0']['EndTime']
        self.assertNotEqual(btime, etime, "Begin and End times of scan=1 should not be equal")
        
        # Only one row of scan=1 should be printed
        output = 'listobs4.txt'
        out = "newobs4.txt"
        reference = reffile+'4'
        diff = "difflistobs4"
        
        listobs(vis=msfile2, verbose=True, listfile=output, listunfl=True)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)    
        os.system("diff "+reference+" "+out+" > "+diff)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        'See the diff file %s.'%(out,reference, diff))
                    
    def test4(self):
        '''Listobs 4: Save on a file, verbose=False'''
        output = 'listobs5.txt'
        out = "newobs5.txt"
        reference = reffile+'5'
        diff1 = "diff1listobs5"
        diff2 = "diff2listobs5"
        
#        # Run it twice to check for the precision change
        self.res = listobs(vis=msfile1, verbose = False, listfile=output, listunfl=True)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)    
        os.system("diff "+reference+" "+out+" > "+diff1)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different in first run. %s != %s. '
                        'See the diff file %s.'%(out,reference, diff1))
        
        os.system('rm -rf '+output+ " "+out)
        self.res = listobs(vis=msfile1, verbose = False, listfile=output, listunfl=True)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)        
        os.system("diff "+reference+" "+out+" > "+diff2)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different in second run. %s != %s. '
                        'See the diff file %s.'%(out,reference,diff2))
        
    def test5(self):
        '''Listobs 5: Save on a file, verbose=True'''
        output = 'listobs6.txt'
        out = "newobs6.txt"
        diff = "difflistobs6"
        reference = reffile+'6'
        self.res = listobs(vis=msfile1, listfile=output, verbose = True, listunfl=True)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)        
        os.system("diff "+reference+" "+out+" > "+diff)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        'See the diff file %s.'%(out,reference,diff))
        

    def test6(self):
        '''Listobs 6: test scan selection parameters'''
        output = "listobs7.txt"
        out = "newobs7.txt"
        diff = "difflistobs7"
        reference = reffile+'7'
        self.res = listobs(vis=msfile1, scan='2', listfile=output, verbose=True, listunfl=True)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)        
        os.system("diff "+reference+" "+out+" > "+diff)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        'See the diff file %s.'%(out,reference,diff))

    def test7(self):
        '''Listobs 7: test antenna selection parameters'''
        output = "listobs8.txt"
        out = "newobs8.txt"
        diff = "difflistobs8"
        reference = reffile+'8'
        self.res = listobs(vis=msfile1, antenna='3&&4', listfile=output, verbose=True, listunfl=True)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)        
        os.system("diff "+reference+" "+out+" > "+diff)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        'See the diff file %s.'%(out,reference,diff))


    def test_overwrite(self):
        """Test overwrite parameter - CAS-5203"""
        listfile = "CAS-5203.log"
        self.assertTrue(
            listobs(vis=msfile1, listfile=listfile)
        )
        # test default value is overwrite=False
        self.assertFalse(
            listobs(vis=msfile1, listfile=listfile)
        )
        self.assertFalse(
            listobs(vis=msfile1, listfile=listfile, overwrite=False)
        )
        expec = _sha1it(listfile)
        self.assertTrue(
            listobs(vis=msfile1, listfile=listfile, overwrite=True)
        )
        got = _sha1it(listfile)
        self.assertTrue(got == expec)
        
class listobs_cleanup(unittest.TestCase):
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        shutil.rmtree(msfile1,ignore_errors=True)
        shutil.rmtree(msfile2,ignore_errors=True)
        os.system('rm -rf ' + 'listobs*.txt')
        
    def test_run(self):
        '''Listobs: Cleanup'''
        pass
        
def suite():
    return [listobs_test1,listobs_cleanup]
