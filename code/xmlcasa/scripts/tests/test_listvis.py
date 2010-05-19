import os
import sys
import shutil
import listing as lt
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task listvis. It tests the following parameters:
    vis:           wrong and correct values
    datacolumn     default and non-default values
    field:         wrong field type; non-default value
    spw:           wrong value; non-default value
    selectdata:    True; subparameters:
                     antenna:   non-default values
    
'''

# Reference files
refpath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listvis/'
reffile = refpath+'reflistvis'

# Input and output names
msfile1 = 'ngc5921_ut.ms'
msfile2 = 'OrionS_rawACSmod'
res = None
out = 'listvis'

class listvis_test1(unittest.TestCase):


    def setUp(self):
        self.res = None
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listvis/'
        if (not os.path.exists(msfile1)):            
            shutil.copytree(datapath+msfile1, msfile1)
        
        if (not os.path.exists(msfile2)):            
            shutil.copytree(datapath+msfile2, msfile2)

        default(listvis)
        
    
    def tearDown(self):
        pass        

    def test1(self):
        '''Listvis 1: Data column'''
        output = out+'1'
        comp = 'compare.1'
        reference = reffile+'1'
        listvis(vis=msfile1,datacolumn='data',listfile=output,
                field='2',spw='0:4~5',selectdata=True,antenna='8&9')
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different.\n %s != %s.'
                        'See the comparison %s'%(output,reference,comp))
        
    def test2(self):
        '''Listvis 2: Float data column'''
        output = out+'2'
        comp = 'compare.2'
        reference = reffile+'2'
        listvis(vis=msfile2,datacolumn='float_data',listfile=output,
                spw='0:2001~2003')
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different\n. %s != %s.'
                        'See the comparison %s'%(output,reference,comp))
        

    def test3(self):
        '''Listvis 3: Data with auto-correlation'''
        output = out+'3'
        comp = 'compare.3'
        reference = reffile+'3'
        listvis(vis=msfile1,datacolumn='data',listfile=output,
                spw='0:1',field='1',selectdata=True,antenna='2&&2')
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different\n. %s != %s.'
                        'See the comparison %s'%(output,reference,comp))
        
class listvis_test2(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        shutil.rmtree(msfile1,ignore_errors=True)
        shutil.rmtree(msfile2,ignore_errors=True)
        os.system('rm -rf ' + out+'*')
        
    def test1a(self):
        '''Listvis: Cleanup'''
        pass
        
def suite():
    return [listvis_test1,listvis_test2]
