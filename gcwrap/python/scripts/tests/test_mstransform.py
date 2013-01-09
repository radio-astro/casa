import shutil
import unittest
import os
import filecmp
from tasks import *
from taskinit import *
from __main__ import default

# Helper functions
# TODO: move them to testhelper.py later
def verify_ms(msname, expnumspws, expnumchan, inspw, expchanfreqs=[]):
    msg = ''
    tb.open(msname+'/SPECTRAL_WINDOW')
    nc = tb.getcell("NUM_CHAN", inspw)
    nr = tb.nrows()
    cf = tb.getcell("CHAN_FREQ", inspw)
    tb.close()
    tb.open(msname)
    dimdata = tb.getcell("FLAG", 0)[0].size
    tb.close()
    if not (nr==expnumspws):
        msg =  "Found "+str(nr)+", expected "+str(expnumspws)+" spectral windows in "+msname
        return [False,msg]
    if not (nc == expnumchan):
        msg = "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in spw "+str(inspw)+" in "+msname
        return [False,msg]
    if not (dimdata == expnumchan):
        msg = "Found "+ str(dimdata) +", expected "+str(expnumchan)+" channels in FLAG column in "+msname
        return [False,msg]

    if not (expchanfreqs==[]):
        print "Testing channel frequencies ..."
        print cf
        print expchanfreqs
        if not (expchanfreqs.size == expnumchan):
            msg =  "Internal error: array of expected channel freqs should have dimension ", expnumchan
            return [False,msg]
        df = (cf - expchanfreqs)/expchanfreqs
        if not (abs(df) < 1E-8).all:
            msg = "channel frequencies in spw "+str(inspw)+" differ from expected values by (relative error) "+str(df)
            return [False,msg]

    return [True,msg]

def test_eq(result, total, flagged):

    print "%s of %s data was flagged, expected %s of %s" % \
    (result['flagged'], result['total'], flagged, total)
    assert result['total'] == total, \
               "%s data in total; %s expected" % (result['total'], total)
    assert result['flagged'] == flagged, \
           "%s flags set; %s expected" % (result['flagged'], flagged)

def create_input(str_text, filename):
    '''Save the string in a text file'''
    
    inp = filename
    cmd = str_text
    
    # remove file first
    if os.path.exists(inp):
        os.system('rm -f '+ inp)
        
    # save to a file    
    with open(inp, 'w') as f:
        f.write(cmd)
        
    f.close()
    
    return

# Path for data
#datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/flagdata/"
#
## Pick up alternative data directory to run tests on MMSs
#testmms = False
#if os.environ.has_key('TEST_DATADIR'):   
#    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/flagdata/'
#    if os.path.isdir(DATADIR):
#        testmms = True
#        datapath = DATADIR

#print 'mstransform tests will use data from '+datapath         


# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):

    # TODO: create a directory under data unittest and link
    # the files there
    def setUp_cveltest(self):
        datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/fits-import-export/input/"
        self.vis = "test.ms"
#        if testmms:
#            self.vis = 'test.mms'

        if os.path.exists(self.vis):
           self.tearDown()
            
        os.system('cp -r '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_ngc5921(self):
        datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/flagdata/"
        self.vis = "ngc5921.ms"
#        if testmms:
#            self.vis = 'ngc5921.mms'

        if os.path.exists(self.vis):
            self.tearDown()
            
        os.system('cp -r '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def tearDown(self):
        os.system('rm -rf '+ self.vis)

    def setUp_data4tfcrop(self):
        datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/flagdata/"
        self.vis = "Four_ants_3C286.ms"
#        if testmms:
#            self.vis = 'Four_ants_3C286.mms'

        if os.path.exists(self.vis):
           self.tearDown()
            
        os.system('cp -r '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

# Add tests from test_cvel.py
# Look at tests in test_cvel-B.py
class test_cvel(test_base):
    '''Tests without any transformation applied'''
    def setUp(self):
        self.setUp_cveltest()
        
    def test_cveltest3(self):
        '''mstransform: Input and output vis set'''
        
        outfile = 'cveltest3.ms'
        rval = mstransform(vis=self.vis, outputvis=outfile, datacolumn='data')
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 64, 0)
        self.assertTrue(ret[0],ret[1])
        
             
class test_combspw(test_base):
    ''' Tests for combinespws'''
    
    def setUp(self):
        self.setUp_data4tfcrop()
                
    def test_combspw1(self):
        '''mstransform: Combine two spws'''
        
        outputms = "combspw1.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0,1')
        self.assertTrue(os.path.exists(outputms))
        
 
# Cleanup class 
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms*')
        os.system('rm -rf Four_ants_3C286.*ms*')

    def test_runTest(self):
        '''mstransform: Cleanup'''
        pass


def suite():
    return [test_combspw,
            test_cvel,
            cleanup]
