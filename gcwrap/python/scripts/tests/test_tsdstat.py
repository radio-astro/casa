import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing
import numpy
from numpy import array

import asap as sd
from tsdstat import tsdstat

class sdstat_unittest_base:
    """
    Base class for sdstat unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdstat/'
    taskname = "sdstat"
    outroot = taskname+'_test'
    outsuff = ".out"
    infile = 'OrionS_rawACSmod_calTPave.asap'

    ### helper functions for tests ###
    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertTrue(isthere,
                         msg='output file %s was not created because of the task failure'%(name))

    def _convert_masklist( self, chanlist, unit, filename, spw=0, restfreq = '' ):
        """
        Convert a masklist from channel unit to a specified unit
            chanlist : masklist in channel unit
            unit     : an output unit of masklist
            filename : a file name to get spectral coordinate from
            spw      : spectral window ID to get spectral coordinate
        """
        self._checkfile(filename)
        vallist = []
        scan = sd.scantable(filename, average=False)
        oldunit = scan.get_unit()
        self.assertTrue(spw in scan.getifnos(), "IF=%d does not exists in %s." % (spw, filename))
        scan.set_unit(unit)
        scan.set_selection(ifs=[spw])
        if restfreq != '':
            molids = scan._getmolidcol_list()
            scan.set_restfreqs(restfreq)
        chanval = scan._getabcissa(0)
        scan.set_unit(oldunit)
        if restfreq != '':
            scan._setmolidcol_list(molids)
            del molids
        del scan, oldunit
        for schan, echan in chanlist:
            vallist.append([chanval[schan],chanval[echan]])
        return vallist

    def _isInAllowedRange( self, testval, refval, allowdiff=1.e-5 ):
        """
        Check if a test value is within permissive relative difference from refval.
        Returns a boolean.
        testval & refval : two numerical values to compare
        allowdiff        : allowed relative difference to consider the two
                           values to be equal. (default 0.01)
        """
        denom = refval
        if refval == 0:
            if testval == 0:
                return True
            else:
                denom = testval
        rdiff = (testval-refval)/denom
        del denom,testval,refval
        return (abs(rdiff) <= allowdiff)

    def _to_list( self, input ):
        """
        Convert input to a list
        If input is None, this method simply returns None.
        """
        listtypes = (list, tuple, numpy.ndarray)
        if input == None:
            return None
        elif type(input) in listtypes:
            return list(input)
        else:
            return [input]

    def _get_elements( self, fulllist, elems=None ):
        """
        Returns a list of selected elements from an input list
            fulllist  : an input list
            elems     : a list of element IDs to return
        """
        self.assertTrue(type(fulllist) in (list,tuple,numpy.ndarray), \
                        "Input array should be either a list or tuple")
        if elems is not None:
            elems = self._to_list(elems)
        else:
            # No selection
            return fulllist
        # Empty elems=[]
        self.assertTrue(len(elems) > 0, "No index specified.")
        # Check range of elems
        if (max(elems) > len(fulllist) - 1) or (min(elems) < 0):
            raise Exception("Indices out of range: %s\nShould be in 0 - %d" % \
                            (str(elems), len(fulllist) - 1))
        sellist = []
        for id in elems:
            sellist.append(fulllist[id])
        return sellist

    def _compareStats( self, currstat, refstat, icomp=None, allowdiff=1.e-5, compstats=None ):
        # compare statistic values
        if compstats:
            compstats = self._to_list(compstats)
        else:
            compstats = refstat.keys()

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(refstat,dict),
                         msg="The referece statistics are not a dictionary")
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        for stat in compstats:
            self.assertTrue(refstat.has_key(stat),
                            msg = "'%s' is not defined in reference data" % stat)
            self.assertTrue(currstat.has_key(stat),
                            msg = "'%s' is not defined in the current run" % stat)
            refval = refstat[stat]
            currval = currstat[stat]
            # Quantum values
            if isinstance(refval,dict):
                if refval.has_key('unit') and currval.has_key('unit'):
                    self.assertEqual(refval['unit'],currval['unit'],\
                                     "The units differ in '%s' %s (current run), %s (reference)" % \
                                     (stat, currval['unit'], refval['unit']))
                    #print "Comparing unit of '%s': %s (current run), %s (reference)" % \
                    #      (stat,currval['unit'],refval['unit'])
                    refval = refval['value']
                    currval = currval['value']
                else:
                    raise Exception("Invalid quantum values. %s (current run) %s (reference)" %\
                                    (str(currval),str(refval)))
            refval = self._to_list(refval)
            currval = self._to_list(currval)
            if icomp is not None:
                refval = self._get_elements(refval,icomp)
            #print "Comparing '%s': %s (current run), %s (reference)" % \
            #      (stat,str(currval),str(refval))
            self.assertTrue(len(currval)==len(refval),"Number of elemnets differs.")
            for i in range(len(currval)):
                self.assertTrue(self._isInAllowedRange(currval[i], refval[i], allowdiff),
                                msg="'%s' of spectrum %d is different." % (stat, i))
        

    def _compareFiles( self, testfile, reffile ):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        self.assertTrue(os.path.exists(testfile),
                        msg=("Output file '%s' doesn't exist" % testfile))
        self.assertTrue(os.path.exists(reffile),
                        msg=("Reference file '%s' doesn't exist: " % reffile))
        self.assertTrue(listing.compare(testfile,reffile),
                        'New and reference files are different. %s != %s. '
                        %(testfile,reffile))
        


class sdstat_basicTest( sdstat_unittest_base, unittest.TestCase ):
    """
    Basic unit tests for task sdstat. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- default + valid input filename
    test02-03 --- available fluxunits
    test04-06 --- available specunits
    test07-09 --- masklist in various specunit
    test10    --- invertmask=True
    test11    --- format

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdcal)
      sdcal(infile='temp.asap',timeaverage=True,tweight='tintsys',
                polaverage=True,pweight='tsys',outfile=self.infile)
    """
    ### TODO:
    ### - need checking for flag application
    ### - comparison with simple spectral

    #compVstats = ['max','min','mean','sum','rms','median','stddev']
    # Line channels
    linechan0 = [[3999,4144]]
    linechan2 = [[2951,3088]]
    #### Reference data (from ASAP r2084 + CASA r14498, Mar. 31, 2011)
    # Reference statistic values (masklist=[], invertmask=False)
    ref_allK = {'rms': [4.1249432563781738, 4.0882534980773926, 4.4454779624938965, 4.1502971649169922],
                'min': [-6.5844717025756836, 0.6394113302230835, -92.782661437988281, 1.9847347736358643],
                'max': [12.278024673461914, 5.8824653625488281, 60.68634033203125, 5.2238655090332031],
                'median': [4.1043367385864258, 4.0837831497192383, 4.1507611274719238, 4.1468963623046875],
                'stddev': [0.2636776864528656, 0.1379869282245636, 1.5120075941085815, 0.1202411949634552],
                'sum': [33722.40234375, 33471.87109375, 34246.4453125, 33984.94140625],
                'mean': [4.1165041923522949, 4.0859217643737793, 4.1804742813110352, 4.148552417755127]}

    ref_allJy = {'rms': [3.2270569801330566, 3.1983535289764404, 3.4778206348419189, 3.24688720703125],
                 'min': [-5.1512141227722168, 0.50022917985916138, -72.586441040039062, 1.5527129173278809],
                 'max': [9.605438232421875, 4.602015495300293, 47.476600646972656, 4.0867743492126465],
                 'median': [3.2109360694885254, 3.1948564052581787, 3.2472553253173828, 3.2442317008972168],
                 'stddev': [0.20628249645233154, 0.10795092582702637, 1.1828852891921997, 0.094067998230457306],
                 'sum': [26381.958984375, 26186.046875, 26791.974609375, 26587.322265625],
                 'mean': [3.2204539775848389, 3.1965389251708984, 3.2705047130584717, 3.2455227375030518]}

    minmaxchan_all = {'max_abscissa': {'value': array([   21.,  8177.,  8186.,  8159.]), 'unit': 'channel'},
                      'min_abscissa': {'value': array([   18.,     0.,  8187.,     0.]), 'unit': 'channel'}}

    minmaxfreq_all = {'max_abscissa': {'value': array([ 45.46447881,  45.32569678,  44.09989566,  44.19101657]),
                                       'unit': 'GHz'},
                      'min_abscissa': {'value': array([ 45.4644605 ,  45.27578247,  44.09990176,  44.14121213]),
                                       'unit': 'GHz'}}
    minmaxvelo_all = {'max_abscissa': {'value': array([  169.89144245,  1084.50062449,  9162.84415004,  8562.33397937]),
                                       'unit': 'km/s'},
                      'min_abscissa': {'value': array([  170.01212783,  1413.44873913,  9162.80392158,  8890.55798174]),
                                       'unit': 'km/s'}}

    # Reference line statistic values (masklist=linechan2, invertmask=False)
    ref_line2 = {'rms': 5.0687642097473145, 'min': 3.9442729949951172,
                 'max': 6.1298322677612305, 'median': 4.9097409248352051,
                 'stddev': 0.57094955444335938, 'sum': 695.0701904296875,
                 'mean': 5.0367403030395508}
    minmaxchan_line2 = {'max_abscissa': {'value': 3048.0, 'unit': 'channel'}, 
                        'min_abscissa': {'value': 2951.0, 'unit': 'channel'}}

    # Reference baseline statistic values (masklist=linechan0, invertmask=True)
    ref_bl0 = {'rms': 4.1130456924438477, 'min': -6.5844717025756836,
               'max': 12.278024673461914, 'median': 4.1013059616088867,
               'stddev': 0.24747852981090546,'sum': 33033.6015625,
               'mean': 4.105593204498291}
    minmaxchan_bl0 = {'max_abscissa': {'value': 21.0, 'unit': 'channel'},
                      'min_abscissa': {'value': 18.0, 'unit': 'channel'}}

    ### Actual test scripts ###
    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdstat)

    def tearDown( self ):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test00( self ):
        """Test 0: Default parameters"""
        result = tsdstat()
        self.assertFalse(result)

    def test01( self ):
        """Test 1: Default parameters + valid input filename """
        tid="01"
        outfile = self.outroot+tid+self.outsuff

        currstat = tsdstat(infile=self.infile,outfile=outfile)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK)
        self._compareStats(currstat,self.minmaxchan_all)


    def test02( self ):
        """Test 2: fluxunit='K' """
        tid="02"
        fluxunit = 'K'
        # automatic conversion for GBT data
        telescopeparam = ""
        outfile = self.outroot+tid+self.outsuff

        currstat = tsdstat(infile=self.infile,fluxunit=fluxunit,telescopeparam=telescopeparam,outfile=outfile)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK)
        self._compareStats(currstat,self.minmaxchan_all)


    def test03( self ):
        """Test 3: fluxunit='Jy' """
        tid="03"
        fluxunit = 'Jy'
        # automatic conversion for GBT data
        telescopeparam = ""
        outfile = self.outroot+tid+self.outsuff

        currstat = tsdstat(infile=self.infile,fluxunit=fluxunit,telescopeparam=telescopeparam,outfile=outfile)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allJy)
        self._compareStats(currstat,self.minmaxchan_all)


    def test04( self ):
        """Test 4: specunit='channel' """
        tid="04"
        specunit = 'channel'
        outfile = self.outroot+tid+self.outsuff

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK)
        self._compareStats(currstat,self.minmaxchan_all)


    def test05( self ):
        """Test 5: specunit='GHz' """
        tid="05"
        specunit = 'GHz'
        outfile = self.outroot+tid+self.outsuff

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK)
        self._compareStats(currstat,self.minmaxfreq_all)


    def test06( self ):
        """Test 6: specunit='km/s' """
        tid="06"
        specunit = 'km/s'
        outfile = self.outroot+tid+self.outsuff

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK)
        self._compareStats(currstat,self.minmaxvelo_all)


    def test07( self ):
        """Test 7: maskllist (line) in specunit='channel' """
        tid="07"
        outfile = self.outroot+tid+self.outsuff
        iflist = [2]
        specunit = 'channel'

        masklist = self.linechan2

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_line2)
        self._compareStats(currstat,self.minmaxchan_line2)


    def test08( self ):
        """Test 8: maskllist (line) in specunit='GHz' """
        tid="08"
        outfile = self.outroot+tid+self.outsuff
        iflist = [2]
        specunit = 'GHz'

        masklist = self._convert_masklist(self.linechan2,specunit,self.infile,spw=iflist[0])
        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_line2)


    def test09( self ):
        """Test 9: maskllist (line) in specunit='km/s' """
        tid="09"
        outfile = self.outroot+tid+self.outsuff
        iflist = [2]
        specunit = 'km/s'

        masklist = self._convert_masklist(self.linechan2,specunit,self.infile,spw=iflist[0])
        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_line2)


    def test10( self ):
        """Test 10: invert = True"""
        tid="10"
        outfile = self.outroot+tid+self.outsuff
        iflist = [0]
        specunit = 'channel'

        masklist = self.linechan0
        invert = True

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist,invertmask=invert)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_bl0)
        self._compareStats(currstat,self.minmaxchan_bl0)

    def test11( self ):
        """Test1: specify format """
        tid="11"
        reference = self.datapath+"refstat"+tid
        
        specunit = 'channel'
        outfile = self.outroot+tid+self.outsuff
        format = '3.5f'

        currstat = tsdstat(infile=self.infile,specunit=specunit,format=format,outfile=outfile)
        # print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareFiles(outfile, reference)
        #self._compareStats(currstat,self.ref_allK)
        #self._compareStats(currstat,self.minmaxchan_all)

class sdstat_restfreqTest( sdstat_unittest_base, unittest.TestCase ):
    """
    Unit tests for task sdstat. Test variations of restfreq parameter.

    The list of tests:
    testRF01 - testRF02 --- a value (float, quantity w/ unit)
    testRF11 - testRF13 --- single element list (int, quantity w/o unit, dictionary)
    testRF21 - testRF23 --- single element list (float/int, quantity, dictionary)
    """
    iflist = [0,2]
    frf = [45.490e9, 44.075e9]
    irf = [45490000000, 44075000000]
    qurf = ['45490.MHz','44.075GHz']
    qrf = [str(frf[0]), str(irf[1])]
    drf = [{'name': "IF0 Rest", 'value': frf[0]}, \
           {'name': "IF2 Rest", 'value': qurf[1]}]
    badq = ['45490.km','44.075bad']
    
    #compVstats = ['max','min','mean','sum','rms','median','stddev']

    # Reference line statistic values (masklist=linechan2, invertmask=False)
    ref_allK02 = {'rms': [4.1249432563781738, 4.4454779624938965],
                'min': [-6.5844717025756836, -92.782661437988281],
                'max': [12.278024673461914, 60.68634033203125],
                'median': [4.1043367385864258, 4.1507611274719238],
                'stddev': [0.2636776864528656, 1.5120075941085815],
                'sum': [33722.40234375, 34246.4453125],
                'mean': [4.1165041923522949, 4.1804742813110352]}

    minmaxvrf0_all0 = {'max_abscissa': {'value': 168.19211024624954, 'unit': 'km/s'}, 
                        'min_abscissa': {'value': 168.3127963097034, 'unit': 'km/s'}}
    minmaxvrf2_all0 = {'max_abscissa': {'value': -9451.0554503663261, 'unit': 'km/s'}, 
                        'min_abscissa': {'value': -9450.9308897531319, 'unit': 'km/s'}}
    minmaxvrf2_all2 = {'max_abscissa': {'value': -169.33704197623328, 'unit': 'km/s'}, 
                        'min_abscissa': {'value': -169.37856218060918, 'unit': 'km/s'}}
    #minmaxchan_all = {'max_abscissa': {'value': array([   21.,  8186.]), 'unit': 'channel'},
    #                  'min_abscissa': {'value': array([   18.,  8187.]), 'unit': 'channel'}}    
    ### Actual test scripts ###
    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdstat)

    def tearDown( self ):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    # Tests
    def testRF01( self ):
        """Test RF01: restfreq (a float value)"""
        tid = "RF01"
        infile = self.infile
        outfile = self.outroot+tid+self.outsuff
        iflist = self.iflist
        specunit = 'km/s'
        restfreq = self.frf[1]
        
        print "Setting restfreq = %s" % (str(restfreq))
        currstat = tsdstat(infile=self.infile,outfile=outfile,iflist=iflist,\
                          specunit=specunit,restfreq=restfreq)

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK02)
        # Comparing min/max pos (Need to invert order of ref/test vals for icomp)
        self._compareStats(self.minmaxvrf2_all0,currstat,icomp=0,compstats=self.minmaxvrf2_all0.keys())
        self._compareStats(self.minmaxvrf2_all2,currstat,icomp=1,compstats=self.minmaxvrf2_all2.keys())

    def testRF02( self ):
        """Test RF02: restfreq (a quantity w/ unit)"""
        tid = "RF02"
        infile = self.infile
        outfile = self.outroot+tid+self.outsuff
        iflist = self.iflist
        specunit = 'km/s'
        restfreq = self.qurf[1]

        print "Setting restfreq = %s" % (str(restfreq))
        currstat = tsdstat(infile=self.infile,outfile=outfile,iflist=iflist,\
                          specunit=specunit,restfreq=restfreq)

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK02)
        # Comparing min/max pos (Need to invert order of ref/test vals for icomp)
        self._compareStats(self.minmaxvrf2_all0,currstat,icomp=0,compstats=self.minmaxvrf2_all0.keys())
        self._compareStats(self.minmaxvrf2_all2,currstat,icomp=1,compstats=self.minmaxvrf2_all2.keys())

    def testRF11( self ):
        """Test RF11: restfreq (single element list of int)"""
        tid = "RF11"
        infile = self.infile
        outfile = self.outroot+tid+self.outsuff
        iflist = self.iflist
        specunit = 'km/s'
        restfreq = [ self.irf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        currstat = tsdstat(infile=self.infile,outfile=outfile,iflist=iflist,\
                          specunit=specunit,restfreq=restfreq)

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK02)
        # Comparing min/max pos (Need to invert order of ref/test vals for icomp)
        self._compareStats(self.minmaxvrf2_all0,currstat,icomp=0,compstats=self.minmaxvrf2_all0.keys())
        self._compareStats(self.minmaxvrf2_all2,currstat,icomp=1,compstats=self.minmaxvrf2_all2.keys())

    def testRF12( self ):
        """Test RF12: restfreq (single element list of quantity w/o unit)"""
        tid = "RF12"
        infile = self.infile
        outfile = self.outroot+tid+self.outsuff
        iflist = self.iflist
        specunit = 'km/s'
        restfreq = [ self.qrf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        currstat = tsdstat(infile=self.infile,outfile=outfile,iflist=iflist,\
                          specunit=specunit,restfreq=restfreq)

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK02)
        # Comparing min/max pos (Need to invert order of ref/test vals for icomp)
        self._compareStats(self.minmaxvrf2_all0,currstat,icomp=0,compstats=self.minmaxvrf2_all0.keys())
        self._compareStats(self.minmaxvrf2_all2,currstat,icomp=1,compstats=self.minmaxvrf2_all2.keys())

    def testRF13( self ):
        """Test RF13: restfreq (single element list of dictionary)"""
        tid = "RF13"
        infile = self.infile
        outfile = self.outroot+tid+self.outsuff
        iflist = self.iflist
        specunit = 'km/s'
        restfreq = [ self.drf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        currstat = tsdstat(infile=self.infile,outfile=outfile,iflist=iflist,\
                          specunit=specunit,restfreq=restfreq)

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK02)
        # Comparing min/max pos (Need to invert order of ref/test vals for icomp)
        self._compareStats(self.minmaxvrf2_all0,currstat,icomp=0,compstats=self.minmaxvrf2_all0.keys())
        self._compareStats(self.minmaxvrf2_all2,currstat,icomp=1,compstats=self.minmaxvrf2_all2.keys())

    def testRF21( self ):
        """Test RF21: restfreq (a list of float & int)"""
        tid = "RF21"
        infile = self.infile
        outfile = self.outroot+tid+self.outsuff
        iflist = self.iflist
        specunit = 'km/s'
        restfreq = [ self.frf[0], self.irf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        currstat = tsdstat(infile=self.infile,outfile=outfile,iflist=iflist,\
                          specunit=specunit,restfreq=restfreq)

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK02)
        # Comparing min/max pos (Need to invert order of ref/test vals for icomp)
        self._compareStats(self.minmaxvrf0_all0,currstat,icomp=0,compstats=self.minmaxvrf2_all0.keys())
        self._compareStats(self.minmaxvrf2_all2,currstat,icomp=1,compstats=self.minmaxvrf2_all2.keys())

    def testRF22( self ):
        """Test RF22: restfreq (a list of quantity w/ and w/o unit)"""
        tid = "RF22"
        infile = self.infile
        outfile = self.outroot+tid+self.outsuff
        iflist = self.iflist
        specunit = 'km/s'
        restfreq = [ self.qurf[0], self.qrf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        currstat = tsdstat(infile=self.infile,outfile=outfile,iflist=iflist,\
                          specunit=specunit,restfreq=restfreq)

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK02)
        # Comparing min/max pos (Need to invert order of ref/test vals for icomp)
        self._compareStats(self.minmaxvrf0_all0,currstat,icomp=0,compstats=self.minmaxvrf2_all0.keys())
        self._compareStats(self.minmaxvrf2_all2,currstat,icomp=1,compstats=self.minmaxvrf2_all2.keys())

    def testRF23( self ):
        """Test RF23: restfreq (a list of dictionary)"""
        tid = "RF23"
        infile = self.infile
        outfile = self.outroot+tid+self.outsuff
        iflist = self.iflist
        specunit = 'km/s'
        restfreq = [ self.drf[1], self.drf[0] ]

        print "Setting restfreq = %s" % (str(restfreq))
        currstat = tsdstat(infile=self.infile,outfile=outfile,iflist=iflist,\
                          specunit=specunit,restfreq=restfreq)

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareStats(currstat,self.ref_allK02)
        # Comparing min/max pos (Need to invert order of ref/test vals for icomp)
        self._compareStats(self.minmaxvrf2_all0,currstat,icomp=0,compstats=self.minmaxvrf2_all0.keys())
        self._compareStats(self.minmaxvrf2_all2,currstat,icomp=1,compstats=self.minmaxvrf2_all2.keys())



        
class sdstat_storageTest( sdstat_unittest_base, unittest.TestCase ):
    """
    Unit tests for task sdstat. Test scantable sotrage and insitu
    parameters

    The list of tests:
    testMT  --- storage = 'memory', insitu = True
    testMF  --- storage = 'memory', insitu = False
    testDT  --- storage = 'disk', insitu = True
    testDF  --- storage = 'disk', insitu = False

    Note on handlings of disk storage:
       Task script restores unit and frame information.
    """
    linechan2 = [[2951,3088]]
    # Reference line statistic values (masklist=linechan2, invertmask=False)
    ref_line2 = {'rms': 5.0687642097473145, 'min': 3.9442729949951172,
                 'max': 6.1298322677612305, 'median': 4.9097409248352051,
                 'stddev': 0.57094955444335938, 'sum': 695.0701904296875,
                 'mean': 5.0367403030395508}
    minmaxchan_line2 = {'max_abscissa': {'value': 3048.0, 'unit': 'channel'}, 
                        'min_abscissa': {'value': 2951.0, 'unit': 'channel'}}
    minmaxvrf_line2 = {'max_abscissa': {'value': 43.993768228253309, 'unit': 'km/s'}, 
                        'min_abscissa': {'value': 48.021228055013935, 'unit': 'km/s'}}
    
    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        # back up the original settings
        self.storage = sd.rcParams['scantable.storage']
        self.insitu = sd.rcParams['insitu']

        default(tsdstat)

    def tearDown( self ):
        # restore settings
        sd.rcParams['scantable.storage'] = self.storage
        sd.rcParams['insitu'] = self.insitu
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def testMT( self ):
        """Storage Test MT: storage='memory' and insitu=T"""
        tid="MT"
        outfile = self.outroot+tid+self.outsuff
        iflist = [2]
        #specunit = 'GHz'
        specunit = 'km/s'
        restfreq = [44.075e9]

        sd.rcParams['scantable.storage'] = 'memory'
        initstat = tsdstat(infile=self.infile,specunit='',outfile=outfile+'.init',
                          iflist=iflist,masklist=self.linechan2)
        masklist = self._convert_masklist(self.linechan2,specunit,self.infile,spw=iflist[0],
                                          restfreq=restfreq[0])
        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist,restfreq=restfreq)
        # print "Statistics out of the current run:\n",currstat
        
        sd.rcParams['scantable.storage'] = 'memory'
        newinstat = tsdstat(infile=self.infile,specunit='',outfile=outfile+'.newin',
                          iflist=iflist,masklist=self.linechan2)

        # Test input data
        compstats = self.ref_line2.keys()
        print "Comparing INPUT statistics before/after calculations"
        self._compareStats(newinstat,initstat,compstats=compstats)
        print "Quantums before run"
        self._compareStats(initstat,self.minmaxchan_line2)
        print "Quantums after run"
        self._compareStats(newinstat,self.minmaxchan_line2)
        # Test output data
        print "Testing OUTPUT statistics"
        self._compareStats(currstat,self.ref_line2)
        print "Testing OUTPUT Quantums"
        self._compareStats(currstat,self.minmaxvrf_line2)

    def testMF( self ):
        """Storage Test MF: storage='memory' and insitu=F"""
        tid="MF"
        outfile = self.outroot+tid+self.outsuff
        iflist = [2]
        #specunit = 'GHz'
        specunit = 'km/s'
        restfreq = [44.075e9]

        sd.rcParams['scantable.storage'] = 'memory'
        initstat = tsdstat(infile=self.infile,specunit='',outfile=outfile+'.init',
                          iflist=iflist,masklist=self.linechan2)
        masklist = self._convert_masklist(self.linechan2,specunit,self.infile,spw=iflist[0],
                                          restfreq=restfreq[0])
        
        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist,restfreq=restfreq)
        # print "Statistics out of the current run:\n",currstat
        
        sd.rcParams['scantable.storage'] = 'memory'
        newinstat = tsdstat(infile=self.infile,specunit='',outfile=outfile+'.newin',
                          iflist=iflist,masklist=self.linechan2)

        # Test input data
        compstats = self.ref_line2.keys()
        print "Comparing INPUT statistics before/after calculations"
        self._compareStats(newinstat,initstat,compstats=compstats)
        print "Quantums before run"
        self._compareStats(initstat,self.minmaxchan_line2)
        print "Quantums after run"
        self._compareStats(newinstat,self.minmaxchan_line2)
        # Test output data
        print "Testing OUTPUT statistics"
        self._compareStats(currstat,self.ref_line2)
        print "Testing OUTPUT Quantums"
        self._compareStats(currstat,self.minmaxvrf_line2)

    def testDT( self ):
        """Storage Test DT: storage='disk' and insitu=T"""
        tid="DT"
        outfile = self.outroot+tid+self.outsuff
        iflist = [2]
        #specunit = 'GHz'
        specunit = 'km/s'
        restfreq = [44.075e9]

        sd.rcParams['scantable.storage'] = 'memory'
        initstat = tsdstat(infile=self.infile,specunit='',outfile=outfile+'.init',
                          iflist=iflist,masklist=self.linechan2)
        masklist = self._convert_masklist(self.linechan2,specunit,self.infile,spw=iflist[0],
                                          restfreq=restfreq[0])
        
        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist,restfreq=restfreq)
        # print "Statistics out of the current run:\n",currstat
        
        sd.rcParams['scantable.storage'] = 'memory'
        newinstat = tsdstat(infile=self.infile,specunit='',outfile=outfile+'.newin',
                          iflist=iflist,masklist=self.linechan2)

        # Test input data
        compstats = self.ref_line2.keys()
        print "Comparing INPUT statistics before/after calculations"
        self._compareStats(newinstat,initstat,compstats=compstats)
        print "Quantums before run"
        self._compareStats(initstat,self.minmaxchan_line2)
        print "Quantums after run"
        self._compareStats(newinstat,self.minmaxchan_line2)
        # Test output data
        print "Testing OUTPUT statistics"
        self._compareStats(currstat,self.ref_line2)
        print "Testing OUTPUT Quantums"
        self._compareStats(currstat,self.minmaxvrf_line2)

    def testDF( self ):
        """Storage Test DF: storage='disk' and insitu=F"""
        tid="DF"
        outfile = self.outroot+tid+self.outsuff
        iflist = [2]
        #specunit = 'GHz'
        specunit = 'km/s'
        restfreq = [44.075e9]

        sd.rcParams['scantable.storage'] = 'memory'
        initstat = tsdstat(infile=self.infile,specunit='',outfile=outfile+'.init',
                          iflist=iflist,masklist=self.linechan2)
        masklist = self._convert_masklist(self.linechan2,specunit,self.infile,spw=iflist[0],
                                          restfreq=restfreq[0])
        
        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))

        currstat = tsdstat(infile=self.infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist,restfreq=restfreq)
        # print "Statistics out of the current run:\n",currstat
        
        sd.rcParams['scantable.storage'] = 'memory'
        newinstat = tsdstat(infile=self.infile,specunit='',outfile=outfile+'.newin',
                          iflist=iflist,masklist=self.linechan2)

        # Test input data
        compstats = self.ref_line2.keys()
        print "Comparing INPUT statistics before/after calculations"
        self._compareStats(newinstat,initstat,compstats=compstats)
        print "Quantums before run"
        self._compareStats(initstat,self.minmaxchan_line2)
        print "Quantums after run"
        self._compareStats(newinstat,self.minmaxchan_line2)
        # Test output data
        print "Testing OUTPUT statistics"
        self._compareStats(currstat,self.ref_line2)
        print "Testing OUTPUT Quantums"
        self._compareStats(currstat,self.minmaxvrf_line2)

class sdstat_exceptions( sdstat_unittest_base, unittest.TestCase ):
    """
    Test the case when the task throws exception.
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdstat/'
    taskname = "sdstat"
    outroot = taskname+'_test'
    outsuff = ".out"
    infile = 'OrionS_rawACSmod_calTPave.asap'

    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdstat)

    def tearDown( self ):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def testNoData(self):
        try:
            res = tsdstat(infile=self.infile,iflist=99)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Selection contains no data. Not applying it.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

def suite():
    return [sdstat_basicTest, sdstat_restfreqTest, sdstat_storageTest,
            sdstat_exceptions]
