import os
import numpy
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
#from tests.test_split import check_eq, SplitChecker
import unittest

'''
Unit tests for task uvcontsub.

Features tested:
  1. uvcontsub will use DATA if CORRECTED_DATA is absent.
  2. It gets the right answer for a known line + 0th order continuum.
  3. It gets the right answer for a known line + 4th order continuum.
  4. It gets the right answer for a known line + 0th order continuum,
     even when fitorder = 4.
'''
datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest'
uvcdatadir = 'uvcontsub' 

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    testmms = True
    DATADIR = str(os.environ.get('TEST_DATADIR'))
    if os.path.isdir(DATADIR):
        datapath = DATADIR
        
#Commented out for refactoring (eliminated test_split dependence)
#class UVContChecker(SplitChecker):
#    """
#    Base class for uvcontsub unit testing.
#    """
#    need_to_initialize = True
#    records = {}
#
#    def do_split(self, corrsel):
#        """
#        This is only called do_split because it comes from SplitChecker.
#        run_task (uvcontsub in this case) would have been a better name.
#        """
#        record = {}
#        try:
#            print "\nRunning uvcontsub"
#            uvran = uvcontsub(self.inpms, fitspw='0:0~5;18~23',
#                               fitorder=corrsel, want_cont=True,
#                               async=False)
#        except Exception, e:
#            print "Error running uvcontsub"
#            raise e
#        for spec in ('cont', 'contsub'):
#            specms = self.inpms + '.' + spec
#            tb.open(specms)
#            record[spec] = tb.getcell('DATA', 52)
#            tb.close()
#            shutil.rmtree(specms)
#        self.__class__.records[corrsel] = record
#        return uvran

class UVContsubUnitTestBase(unittest.TestCase):
    """
    uvcontsub unittest base class (refactored)
    """  
    def initialize(self,inpms):
        """
        initialize 
        """

        global testmms

        if testmms:
            print "Testing on MMSs."
        
        self.inpms = uvcdatadir+'/'+inpms
        if not os.path.exists(uvcdatadir):
            os.system('mkdir '+ uvcdatadir)

        if not os.path.exists(self.inpms):
            try:
                shutil.copytree(datapath + '/' + self.inpms, self.inpms)
            except Exception, e:
                raise Exception, "Missing input MS: " + datapath + '/' + self.inpms 


    def cleanup(self):
        """
        clean up the test dir
        """ 
        if os.path.exists(self.inpms):
            shutil.rmtree(self.inpms)


    def check_eq(self, val, expval, tol=None):
        """Checks that val matches expval within tol."""
	if type(val) == dict:
	    for k in val:
		check_eq(val[k], expval[k], tol)
	else:
	    try:
		if tol and hasattr(val, '__rsub__'):
		    are_eq = abs(val - expval) < tol
		else:
		    are_eq = val == expval
		if hasattr(are_eq, 'all'):
		    are_eq = are_eq.all()
		if not are_eq:
		    raise ValueError, '!='
	    except ValueError:
		errmsg = "%r != %r" % (val, expval)
		if (len(errmsg) > 66): # 66 = 78 - len('ValueError: ')
		    errmsg = "\n%r\n!=\n%r" % (val, expval)
		raise ValueError, errmsg
	    except Exception, e:
		print "Error comparing", val, "to", expval
		raise e


#class zeroth(UVContChecker):
class zeroth(UVContsubUnitTestBase):
    """Test zeroth order fit"""

    def setUp(self):
        self.initialize('known0.ms')

    def tearDown(self):
        self.cleanup()

    def test_zeroth(self):

        record = {}
        pnrows = {}
        try:
            print "\nRunning uvcontsub"
            uvran = uvcontsub(self.inpms, fitspw='0:0~5;18~23',
                               fitorder=0, want_cont=True,
                               async=False)
        except Exception, e:
            print "Error running uvcontsub"
            raise e


        for spec in ('cont', 'contsub'):
            specms = self.inpms + '.' + spec
            tb.open(specms)
            record[spec] = tb.getcell('DATA', 52)
            tb.close()
            tb.open(self.inpms+'.'+spec+'/POINTING')
            pnrows[spec] = tb.nrows()
            tb.close()
            shutil.rmtree(specms)
        #self.__class__.records[corrsel] = record
        #return uvran
        self.assertEqual(uvran,True)

        print "Continuum estimate in line-free region"
        self.check_eq(record['cont'][:,3],   # RR, LL
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)

        print "Continuum estimate in line region"
        self.check_eq(record['cont'][:,13],
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)

        print "Continuum-subtracted data in line-free region"
        self.check_eq(record['contsub'][:,21],   # RR, LL
                 numpy.array([ 0.+0.j,  0.+0.j]), 0.0001)

        print "Continuum-subtracted data in line region"
        self.check_eq(record['contsub'][:,9],   # RR, LL
                 numpy.array([87.+26.j, 31.+20.j]), 0.0001)

        print "Non-empty pointing table (for MMS case)"
        self.assertEqual(pnrows['cont'], 1)
        self.assertEqual(pnrows['contsub'], 1)
        
#class fourth(UVContChecker):
class fourth(UVContsubUnitTestBase):

    def setUp(self):
        self.initialize('known4.ms')

    def tearDown(self):
        self.cleanup()
 
    def test_fourth(self):
 
        infitorder=4                    # fitorder
        record = {}
        try:
            print "\nRunning uvcontsub"
            uvran = uvcontsub(self.inpms, fitspw='0:0~5;18~23',
                               fitorder=infitorder, want_cont=True,
                               async=False)
        except Exception, e:
            print "Error running uvcontsub"
            raise e


        for spec in ('cont', 'contsub'):
            specms = self.inpms + '.' + spec
            tb.open(specms)
            record[spec] = tb.getcell('DATA', 52)
            tb.close()
            shutil.rmtree(specms)
        #self.__class__.records[corrsel] = record
        #return uvran
        self.assertEqual(uvran,True)

        print "Continuum estimate"
        self.check_eq(record['cont'],   # [[RR], [LL]]
                 numpy.array([[20.00000-10.j,      12.50660-10.00000j,
                                7.10324-10.00000j,  3.35941-10.j,
                                0.89944-10.j,      -0.59741-10.j,
                               -1.39700-10.j,      -1.71032-10.j,
                               -1.69345-10.j,      -1.44760-10.j,
                               -1.01907-10.j,      -0.39929-10.j,
                                0.47521-10.j,       1.72278-10.j,
                                3.51665-10.j,       6.08496-10.j,
                                9.71073-10.j,      14.73187-10.j,
                               21.54116-10.j,      30.58629-10.j,
                               42.36984-10.j,      57.44926-10.00000j,
                               76.43690-10.00000j,100.00000-10.j],
                             [-10.00000+5.j,       -6.25330+5.j,
                               -3.55162+5.00000j,  -1.67970+5.00000j,
                               -0.44972+5.00000j,   0.29870+5.j,
                                0.69850+5.j,        0.85516+5.j,
                                0.84673+5.j,        0.72380+5.j,
                                0.50953+5.j,        0.19964+5.j,
                               -0.23760+5.j,       -0.86139+5.j,
                               -1.75832+5.j,       -3.04248+5.j,
                               -4.85537+5.j,       -7.36593+5.j,
                              -10.77058+5.j,      -15.29315+5.00000j,
                              -21.18492+5.00000j, -28.72463+5.00000j,
                              -38.21845+5.j,      -50.00000+5.j]]),
                 0.0001)

        print "Continuum-subtracted data"
        self.check_eq(record['contsub'],   # [[RR], [LL]]
                 numpy.array([[0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                              30.86956-0.10000j,   34.34782-0.10000j,
                              37.82608-0.10000j,   41.30434-0.10000j,
                              44.78260-0.10000j,   48.26086-0.10000j,
                              48.26086-0.10000j,   44.78260-0.10000j,
                              41.30434-0.10000j,   37.82608-0.10000j,
                              34.34782-0.10000j,   30.86956-0.10000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j],
                              [0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                             -30.86956+0.10000j,  -34.34782+0.10000j,
                             -37.82608+0.10000j,  -41.30434+0.10000j,
                             -44.78260+0.10000j,  -48.26086+0.10000j,
                             -48.26086+0.10000j,  -44.78260+0.10000j,
                             -41.30434+0.10000j,  -37.82608+0.10000j,
                             -34.34782+0.10000j,  -30.86956+0.10000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j]]),
                 0.0001)

#class combspw(UVContChecker):
class combspw(UVContsubUnitTestBase):

    def setUp(self):
        self.initialize('combspw.ms')
    
    def tearDown(self):
        self.cleanup()


    def test_combspw(self):
        fitorders = [0, 1]                    # fitorder, not corr selection.
        record = {}
        for infitorder in fitorders:
            record[infitorder]={}
	    try:
		print "\nRunning uvcontsub"
		uvran = uvcontsub(self.inpms, fitspw='1~10:5~122,15~22:5~122',
				   spw='6~14', combine='spw',
				   fitorder=infitorder, want_cont=False,
				   async=False)
	    except Exception, e:
		print "Error running uvcontsub"
		raise e

	    specms = self.inpms + '.contsub'
	    tb.open(specms)
	    record[infitorder]['contsub'] = tb.getcell('DATA', 52)[0][73]
	    tb.close()
	    shutil.rmtree(specms)
	    #self.__class__.records[corrsel] = record
	    #return uvran
	    self.assertEqual(uvran,True)

        print "combspw fitorder=0 line estimate"
        self.check_eq(record[0]['contsub'], -6.2324+17.9865j, 0.001)

        print "combspw fitorder=1 line estimate"
        self.check_eq(record[1]['contsub'], -6.2533+17.6584j, 0.001)
    

class excludechans(UVContsubUnitTestBase):
    """Test excludechans """
    # test excludechans=True, otherwise the result should be identical
    # with zeroth test.
    def setUp(self):
        self.initialize('known0.ms')

    def tearDown(self):
        self.cleanup()

    def test_excludechans(self):

        record = {}
        pnrows = {}
        try:
            print "\nRunning uvcontsub"
           
            uvran = uvcontsub(self.inpms, fitspw='0:6~17', #'0:0~5;18~23'
                               excludechans=True, fitorder=0, 
                               want_cont=True, async=False)
        except Exception, e:
            print "Error running uvcontsub"
            raise e


        for spec in ('cont', 'contsub'):
            specms = self.inpms + '.' + spec
            tb.open(specms)
            record[spec] = tb.getcell('DATA', 52)
            tb.close()
            tb.open(self.inpms+'.'+spec+'/POINTING')
            pnrows[spec] = tb.nrows()
            tb.close()
            shutil.rmtree(specms)
        #self.__class__.records[corrsel] = record
        #return uvran
        self.assertEqual(uvran,True)

        print "Continuum estimate in line-free region"
        self.check_eq(record['cont'][:,3],   # RR, LL
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)

        print "Continuum estimate in line region"
        self.check_eq(record['cont'][:,13],
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)

        print "Continuum-subtracted data in line-free region"
        self.check_eq(record['contsub'][:,21],   # RR, LL
                 numpy.array([ 0.+0.j,  0.+0.j]), 0.0001)

        print "Continuum-subtracted data in line region"
        self.check_eq(record['contsub'][:,9],   # RR, LL
                 numpy.array([87.+26.j, 31.+20.j]), 0.0001)

        print "Non-empty pointing table (for MMS case)"
        self.assertEqual(pnrows['cont'], 1)
        self.assertEqual(pnrows['contsub'], 1)

class freqrangeselection(UVContsubUnitTestBase):
    """Test frequency range fitspw """
    # test excludechans=True, otherwise the result should be identical
    # with zeroth test.
    def setUp(self):
        self.initialize('known0.ms')

    def tearDown(self):
        self.cleanup()

    def test_freqrangeselection(self):

        record = {}
        pnrows = {}
        try:
            print "\nRunning uvcontsub"

            uvran = uvcontsub(self.inpms, 
                               fitspw='*:1412665073.7687755~1412787144.0812755Hz;1413104526.8937755~1413226597.2062755Hz',
                               fitorder=0,
                               want_cont=True, async=False)
        except Exception, e:
            print "Error running uvcontsub"
            raise e


        for spec in ('cont', 'contsub'):
            specms = self.inpms + '.' + spec
            tb.open(specms)
            record[spec] = tb.getcell('DATA', 52)
            tb.close()
            tb.open(self.inpms+'.'+spec+'/POINTING')
            pnrows[spec] = tb.nrows()
            tb.close()
            shutil.rmtree(specms)
        #self.__class__.records[corrsel] = record
        #return uvran
        self.assertEqual(uvran,True)

        print "Continuum estimate in line-free region"
        self.check_eq(record['cont'][:,3],   # RR, LL
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)

        print "Continuum estimate in line region"
        self.check_eq(record['cont'][:,13],
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)

        print "Continuum-subtracted data in line-free region"
        self.check_eq(record['contsub'][:,21],   # RR, LL
                 numpy.array([ 0.+0.j,  0.+0.j]), 0.0001)

        print "Continuum-subtracted data in line region"
        self.check_eq(record['contsub'][:,9],   # RR, LL
                 numpy.array([87.+26.j, 31.+20.j]), 0.0001)

        print "Non-empty pointing table (for MMS case)"
        self.assertEqual(pnrows['cont'], 1)
        self.assertEqual(pnrows['contsub'], 1)


    
def suite():
    return [zeroth, fourth, combspw, excludechans, freqrangeselection]
