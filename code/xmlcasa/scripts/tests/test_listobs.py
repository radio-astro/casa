import os
import sys
import shutil
import string
import listing as lt
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task listobs. It tests the following parameters:
    vis:        wrong and correct values
    verbose     true or false
    listfile:   save on a file
    
'''

# Reference files
refpath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listobs/'
reffile = refpath+'reflistobs'

# Input and output names
msfile1 = 'ngc5921_ut.ms'

class listobs_test1(unittest.TestCase):

    def setUp(self):
        self.res = None
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listobs/'
        if (not os.path.exists(msfile1)):            
            shutil.copytree(datapath+msfile1, msfile1)
        
        default(listobs)
        
    
    def tearDown(self):
        pass        

    def test1(self):
        '''Listobs 1: Default values'''
        self.res = listobs()
        self.assertFalse(self.res,'Default parameters should return False')
        
    def test2(self):
        '''Listobs 2: Input MS'''
        self.res = listobs(vis=msfile1)
        self.assertEqual(self.res, None, "Return value should be None")

    def test3(self):
        '''Listobs 3: CSV-591. Check if long field names are fully displayed'''
        # NOTE: it needs the ability to save on a file first, in order to use listobs.
        ms.open(msfile1)
        res = ms.summary(True)
        ms.close()
        name = res['header']['field_0']['name']
        self.assertFalse(name.__contains__('*'), "Field name contains a *")
        name = res['header']['scan_7']['0']['FieldName']
        self.assertFalse(name.__contains__('*'), "Field name contains a *")
                    
#    def test3(self):
#        '''Listobs 3: Save on a file, verbose=False'''
#        output = 'listobs3.txt'
#        out = "newobs3.txt"
#        reference = reffile+'3'
#        diff1 = "diff1listobs3"
#        diff2 = "diff2listobs3"
#        
#        # Run it twice to check for the precision change
#        self.res = listobs(vis=msfile1, verbose = False, listfile=output)
#        # Remove the name of the MS from output before comparison
#        os.system("sed '1,3d' "+ output+ ' > '+ out)    
#        os.system("diff "+reference+" "+out+" > "+diff1)    
#        self.assertTrue(lt.compare(out,reference),
#                        'New and reference files are different in first run. %s != %s. '
#                        'See the diff file %s.'%(out,reference, diff1))
#        
#        os.system('rm -rf '+output+ " "+out)
#        self.res = listobs(vis=msfile1, verbose = False, listfile=output)
#        # Remove the name of the MS from output before comparison
#        os.system("sed '1,3d' "+ output+ ' > '+ out)        
#        os.system("diff "+reference+" "+out+" > "+diff2)    
#        self.assertTrue(lt.compare(out,reference),
#                        'New and reference files are different in second run. %s != %s. '
#                        'See the diff file %s.'%(out,reference,diff2))
        
#    def test4(self):
#        '''Listobs 4: Save on a file, verbose=True'''
#        output = 'listobs4.txt'
#        comp = 'compare.4'
#        reference = reffile+'4'
#        self.res = listobs(vis=msfile1, listfile=output, verbose = True)
#        self.assertTrue(lt.compare(output,reference),
#                        'New and reference files are different. %s != %s. '
#                        'See the diff file.'%(output,reference))
        

class listobs_cleanup(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        shutil.rmtree(msfile1,ignore_errors=True)
        os.system('rm -rf ' + 'listobs*.txt')
        
    def test_run(self):
        '''Listobs: Cleanup'''
        pass
        
def suite():
    return [listobs_test1,listobs_cleanup]
