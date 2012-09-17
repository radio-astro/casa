import os
import sys
import shutil
import string
from __main__ import default
import partitionhelper as ph
from tasks import listpartition
import unittest

'''
Unit tests for task listpartition. It tests the following parameters:
    vis:        wrong and correct values
    createdict  true or false
    listfile:   save to a file
    
    These tests use auxiliary functions defined in partitionhelper.py
    
'''


# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):

    def setUp_MSdata(self):
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
            pass
        else:
            print "Linking to data..."
            os.system('ln -s ' + os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/partition/" + self.vis + ' ' + self.vis)
            
        default(listpartition)

    def setUp_MMSdata1(self):
        self.vis = 'pFourantsSpw.mms'
        self.visdata = self.vis+"/SUBMSS/"

        if os.path.exists(self.vis):
            pass
        else:
            print "Linking to data..."
            os.system('ln -s ' + os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/partition/" + self.vis + ' ' + self.vis)

        default(listpartition)

    def setUp_MMSdata2(self):
        self.vis = 'pFourantsScan.mms'
        self.visdata = self.vis+"/SUBMSS/"

        if os.path.exists(self.vis):
            pass
        else:
            print "Linking to data..."
            os.system('ln -s ' + os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/partition/" + self.vis + ' ' + self.vis)

        default(listpartition)

    def setUp_MMSdata3(self):
        self.vis = 'pFourantsMix.mms'
        self.visdata = self.vis+"/SUBMSS/"

        if os.path.exists(self.vis):
            pass
        else:
            print "Linking to data..."
            os.system('ln -s ' + os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/partition/" + self.vis + ' ' + self.vis)

        default(listpartition)


class test_MS(test_base):

    def setUp(self):
        self.setUp_MSdata()
             
        
    def testMS1(self):
        '''listpartition MS1: Input MS'''
        res = listpartition(vis=self.vis)
        self.assertEqual(res, {}, "It should return an empty dictionary")
                    
    def testMS2(self):
        '''listpartition MS2: Save to a file'''
        output = 'listpartitionms.txt'
        if os.path.exists(output):
            os.system('rm -rf '+output)
            
        listpartition(vis=self.vis, listfile=output)
        self.assertTrue(os.path.exists(output), 'Output file does not exist')
        
                
    def testMS3(self):
        '''listpartition MS3: Create an output dictionary'''

        resdict = listpartition(vis=self.vis, createdict=True)
        
        self.assertEqual(resdict[0]['MS'], self.vis)


class test_MMS_spw(test_base):

    def setUp(self):
        self.setUp_MMSdata1()
             
        
    def testspw1(self):
        '''listpartition MMS spw1: Input MMS'''
        res = listpartition(vis=self.vis)
        self.assertEqual(res, {}, "It should return an empty dictionary")

                    
    def testspw2(self):
        '''listpartition MMS spw2: Save to a file'''
        output = 'listpartitionspw.txt'
        if os.path.exists(output):
            os.system('rm -rf '+output)
            
        listpartition(vis=self.vis, listfile=output)
        self.assertTrue(os.path.exists(output), 'Output file %s does not exist'%output)
        
        # Check the number of lines in the output file
        ff = open(output,'r')
        nlines = len(ff.readlines())
        ff.close()
        self.assertEqual(nlines, 33, 'Wrong number of lines in output')
        
                
    def testspw3(self):
        '''listpartition MMS spw3: Create an output dictionary'''

        resdict = listpartition(vis=self.vis, createdict=True)
        nkeys = resdict.keys().__len__()
        self.assertEqual(nkeys, 16)
                
        # Check all scans in all sub-MSs
        for k in resdict.keys():
            subms = resdict[k]['MS']
            MS = self.visdata+'/'+subms
            scans = resdict[k]['scanId'].keys()
            for s in scans:
                nr = resdict[k]['scanId'][s]['nrows']
                refN = ph.getScanNrows(MS, s)
                self.assertEqual(nr, refN, '%s, scan=%s, nrows=%s do not match reference nrows=%s'\
                                 %(MS, s, nr, refN))
                        
class test_MMS_scan(test_base):

    def setUp(self):
        self.setUp_MMSdata2()
             
        
    def testscan1(self):
        '''listpartition MMS scan1: Input MMS'''
        res = listpartition(vis=self.vis)
        self.assertEqual(res, {}, "It should return an empty dictionary")

                    
    def testscan2(self):
        '''listpartition MMS scan2: Save to a file'''
        output = 'listpartitionscan1.txt'
        if os.path.exists(output):
            os.system('rm -rf '+output)
            
        listpartition(vis=self.vis, listfile=output)
        self.assertTrue(os.path.exists(output), 'Output file %s does not exist'%output)
        
        # Check the number of lines in the output file
        ff = open(output,'r')
        nlines = len(ff.readlines())
        ff.close()
        self.assertEqual(nlines, 3, 'Wrong number of lines in output')
        
                
    def testscan3(self):
        '''listpartition MMS scan3: Create an output dictionary'''

        resdict = listpartition(vis=self.vis, createdict=True)
        nkeys = resdict.keys().__len__()
        self.assertEqual(nkeys, 2)
                
        # Check all scans in all sub-MSs
        for k in resdict.keys():
            subms = resdict[k]['MS']
            MS = self.visdata+'/'+subms
            scans = resdict[k]['scanId'].keys()
            for s in scans:
                nr = resdict[k]['scanId'][s]['nrows']
                refN = ph.getScanNrows(MS, s)
                self.assertEqual(nr, refN, '%s, scan=%s, nrows=%s do not match reference nrows=%s'\
                                 %(MS, s, nr, refN))
            
    def testscan4(self):
        '''listpartition MMS scan4: check the sizes of the sub-MSs'''
        
        output = 'listpartitionscan2.txt'
        if os.path.exists(output):
            os.system('rm -rf '+output)
            
        listpartition(vis=self.vis, listfile=output)
        self.assertTrue(os.path.exists(output))

        # Compare the sizes of the sub-MSs with the output of du -hs
        ff = open(output,'r')
        mslist = ff.readlines()
        i = 0
        for l in mslist:
            if i == 0:
                i += 1
                continue
            
            ll = l.rstrip()
            rear = ll.rpartition(' ')
            front = ll.partition(' ')
            
            # Now get the du -hs for the same sub-MS
            # Step into the data directory
            dusize = ph.getDiskUsage(self.visdata+front[0])

            # Compare both
            self.assertEqual(dusize, rear[2], '%s is not equal to %s for %s'%(dusize,rear[2],front[0]))
            
        ff.close()        

class test_MMS_mix(test_base):

    def setUp(self):
        self.setUp_MMSdata3()
             
        
    def testmix1(self):
        '''listpartition MMS mix1: Input MMS'''
        res = listpartition(vis=self.vis)
        self.assertEqual(res, {}, "It should return an empty dictionary")

                    
    def testmix2(self):
        '''listpartition MMS mix2: Save to a file'''
        output = 'listpartitionmix1.txt'
        if os.path.exists(output):
            os.system('rm -rf '+output)
            
        listpartition(vis=self.vis, listfile=output)
        self.assertTrue(os.path.exists(output), 'Output file %s does not exist'%output)
        
        # Check the number of lines in the output file
        ff = open(output,'r')
        nlines = len(ff.readlines())
        ff.close()
        self.assertEqual(nlines, 33, 'Wrong number of lines in output')
        
                
    def testmix3(self):
        '''listpartition MMS mix3: Create an output dictionary'''

        resdict = listpartition(vis=self.vis, createdict=True)
        nkeys = resdict.keys().__len__()
        self.assertEqual(nkeys, 32)
                
        # Check all scans in all sub-MSs
        for k in resdict.keys():
            subms = resdict[k]['MS']
            MS = self.visdata+'/'+subms
            scans = resdict[k]['scanId'].keys()
            for s in scans:
                nr = resdict[k]['scanId'][s]['nrows']
                refN = ph.getScanNrows(MS, s)
                self.assertEqual(nr, refN, '%s, scan=%s, nrows=%s do not match reference nrows=%s'\
                                 %(MS, s, nr, refN))
                        
    def testmix4(self):
        '''listpartition MMS mix4: check the sizes of the sub-MSs'''
        
        output = 'listpartitionmix2.txt'
        if os.path.exists(output):
            os.system('rm -rf '+output)
            
        listpartition(vis=self.vis, listfile=output)
        self.assertTrue(os.path.exists(output))

        # Compare the sizes of the sub-MSs with the output of du -hs
        ff = open(output,'r')
        mslist = ff.readlines()
        i = 0
        for l in mslist:
            if i == 0:
                i += 1
                continue
            
            ll = l.rstrip()
            rear = ll.rpartition(' ')
            front = ll.partition(' ')
            
            # Now get the du -hs for the same sub-MS
            # Step into the data directory
            dusize = ph.getDiskUsage(self.visdata+front[0])

            # Compare both
            self.assertEqual(dusize, rear[2], '%s is not equal to %s for %s'%(dusize,rear[2],front[0]))
            
        ff.close()        

class listpartition_cleanup(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        
        os.system('rm -rf Four_ants_3C286.ms')
        os.system('rm -rf pFourants*.mms')
        os.system('rm -rf ' + 'listpartition*.txt')
        
    def test_run(self):
        '''listpartition: Cleanup'''
        pass
        
def suite():
    return [test_MS,
            test_MMS_spw,
            test_MMS_scan,
            test_MMS_mix,
            listpartition_cleanup]
    
    
    
