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
msfile3 = 'as1039q4_ut.ms'
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

        if (not os.path.exists(msfile3)):            
            shutil.copytree(datapath+msfile3, msfile3)

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
#        listvis(vis=msfile1,datacolumn='data',listfile=output,
#                field='2',spw='0:4~5',selectdata=True,antenna='1&2')
        self.assertTrue(lt.runTests(output,reference,'1.000',comp),
                        'New and reference files are different. %s != %s. '
                        'See the diff file'%(output,reference))
    def test2(self):
        '''Listvis 1: Data column with different selections'''
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
        
        
class listvis_test2(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        shutil.rmtree(msfile1,ignore_errors=True)
        shutil.rmtree(msfile2,ignore_errors=True)
        shutil.rmtree(msfile3,ignore_errors=True)
        os.system('rm -rf ' + out+'*')
        
    def test1a(self):
        '''Listvis: Cleanup'''
        pass
        
def suite():
    return [listvis_test1,listvis_test2]
