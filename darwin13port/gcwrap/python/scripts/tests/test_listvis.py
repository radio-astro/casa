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

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listvis/'

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/listvis/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR
    else:
        print 'WARN: directory '+DATADIR+' does not exist'

print 'listvis tests will use data from '+datapath         

# Reference files
reffile = datapath+'reflistvis'

# Input and output names
msfile1 = 'ngc5921_ut.ms'
msfile2 = 'OrionS_rawACSmod'
if testmms:
    msfile2 = 'OrionS_rawACSmod.mms'
    
msfile3 = 'as1039q4_ut.ms'
out = 'listvis'

class listvis_test1(unittest.TestCase):

    def setUp(self):
        fpath = os.path.join(datapath, msfile1)
        os.symlink(fpath, msfile1)       

        fpath = os.path.join(datapath, msfile2)
        os.symlink(fpath, msfile2)       

        fpath = os.path.join(datapath, msfile3)
        os.symlink(fpath, msfile3)       

        default(listvis)
        
    
    def tearDown(self):
        if os.path.lexists(msfile1):
            os.unlink(msfile1)
        if os.path.lexists(msfile2):
            os.unlink(msfile2)
        if os.path.lexists(msfile3):
            os.unlink(msfile3)       
            
        os.system('rm -rf ' + out+'*')
        os.system('rm -rf ' + 'compare*')
        

    def test1(self):
        '''Listvis 1: Data column'''
        output = out+'1'
        comp = 'compare.1'
        reference = reffile+'1'
        listvis(vis=msfile1,datacolumn='data',listfile=output,
                field='2',spw='0:4~5',selectdata=True,antenna='8&9')

        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different. %s != %s. '
                        'See the diff file'%(output,reference))
    def test2(self):
        '''Listvis 2: Data column with different selections'''
        output = out+'2'
        comp = 'compare.2'
        reference = reffile+'2'        
        listvis(vis=msfile1,field='0',spw='0:1~2',selectdata=True, antenna='2&11',
                listfile=output)
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different. %s != %s. '
                        'See the diff file'%(output,reference))
        
    def test3(self):
        '''Listvis 3: Float data column (CAS-2138)'''
        output = out+'3'
        comp = 'compare.3'
        reference = reffile+'3'
        listvis(vis=msfile2,datacolumn='float_data',listfile=output,
                spw='0:2001~2003')
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different. %s != %s. '
                        'See the diff file.'%(output,reference))
        

    def test4(self):
        '''Listvis 4: Data with auto-correlation'''
        output = out+'4'
        comp = 'compare.4'
        reference = reffile+'4'
        listvis(vis=msfile1,datacolumn='data',listfile=output,
                spw='0:1',field='1',selectdata=True,antenna='2&&2')
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different. %s != %s. '
                        'See the diff file.'%(output,reference))

    def test5(self):
        '''Listvis 5: MS with blanked scan (CAS-2112)'''
        output = out+'5'
        comp = 'compare.5'
        reference = reffile+'5'
        listvis(vis=msfile3,datacolumn='data',listfile=output,spw='0:1~2',
                selectdata=True,antenna='1',scan='1')
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different. %s != %s. '
                        'See the diff file.'%(output,reference))

    def test6(self):
        '''Listvis 6: MS with good scan (CAS-2112)'''
        output = out+'6'
        comp = 'compare.6'
        reference = reffile+'6'
        listvis(vis=msfile3,spw='4:1~2',selectdata=True,antenna='8',scan='3',listfile=output)
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different. %s != %s. '
                        'See the diff file.'%(output,reference))
        
        
def suite():
    return [listvis_test1]
