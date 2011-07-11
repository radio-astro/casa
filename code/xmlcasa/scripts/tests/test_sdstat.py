import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
#
import listing
from numpy import array

asap_init()
import asap as sd
from sdstat import sdstat


def _get_chanval(filename,chanlist,unit,spw=0):
    vallist = []
    scan = sd.scantable(filename, average=False)
    scan.set_unit(unit)
    scan.set_selection(ifs=[spw])
    chanval = scan._getabcissa(0)
    del scan
    for schan, echan in chanlist:
        vallist.append([chanval[schan],chanval[echan]])
    return vallist

class sdstat_test(unittest.TestCase):
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
      default(sdaverage)
      sdaverage(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdaverage)
      sdaverage(infile='temp.asap',timeaverage=True,tweight='tintsys',
                polaverage=True,pweight='tsys',outfile=self.infile)
    """
    ### TODO:
    ### - need checking for flag application
    ### - comparison with simple spectral

    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
               '/data/regression/unittest/sdstat/'
    # Input and output names
    infile = 'OrionS_rawACSmod_calTPave.asap'
    outroot = 'sdstat_test'
    outsuff = ".out"
    #strefroot = datapath+'refstats'

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

    ### helper functions for tests ###
    def _compareVstats(self,currstat,refstat,icomp=None):
        ## icomp is a list of indices in reference dict to compare with testvals
        if icomp:
            # make sure icomp is a list if specified
            try: len(icomp)
            except: icomp = [icomp]

        akey = refstat.keys()[0]
        arefdata = refstat[akey]
        acurrdata = currstat[akey]
        # check if reference dictionary is a list or value
        reflist = False
        if isinstance(arefdata,list) or isinstance(arefdata,tuple):
            # reference dictionary stores a list to compare
            reflist = True

        # check if current value is a list or value
        if isinstance(acurrdata,list) or isinstance(acurrdata,tuple):
            # current statistics dictionary stores a list to compare
            if not reflist and len(acurrdata) > 1:
                del akey, arefdata, acurrdata, reflist
                raise Exception("Current data has more elements than indices set to compare")
            if not icomp:
                icomp = range(len(acurrdata))
            if reflist and len(acurrdata) > len(icomp):
                del akey, arefdata, acurrdata, reflist
                # make sure current data and icomp has the same number of elements if both are list
                raise Exception("Current data has more elements than indices set to compare")
            # The latter indices are ignored if you specified many 'icomp'
            icomp = icomp[:len(acurrdata)]
            currlist = True
        else:
            # current statistics dictionary stores a value to compare
            if reflist and not icomp:
                del akey, arefdata, acurrdata, reflist
                raise ValueError, "No index to compare specified"
            elif not reflist:
                # both are values: icomp is ignored
                icomp = [0]
            # The latter indices are ignored if you specified many 'icomp'
            icomp = [icomp[0]]
            currlist = False
        if reflist and (max(icomp) > len(arefdata)-1 or min(icomp) < 0):
            del akey, arefdata, acurrdata, reflist, currlist
            raise Exception("Indices of reference data out of range")
        del akey, arefdata, acurrdata
    
        # compare statistic values
        valstats = ['max','min','mean','sum','rms','median','stddev']
        allowdiff = 0.01
        for stat in valstats:
            for i in range(len(icomp)):
                if reflist:
                    refval = refstat[stat][icomp[i]]
                else:
                    refval = refstat[stat]
                if currlist:
                    currval = currstat[stat][i]
                else:
                    currval = currstat[stat] 
                self.assertTrue(self._isInAllowedRange(currval, refval, allowdiff),
                                msg="'%s' of spectrum %d are different." % (stat, i))
        del refstat, currstat, icomp, reflist, currlist, currval, refval, valstats, allowdiff

    def _isInAllowedRange(self, testval, refval, allowdiff):
        rdiff = (testval-refval)/refval
        del testval,refval
        return (rdiff <= allowdiff)

    def _compareQstats(self,currstat,refstat,icomp=None):
        ## icomp is a list of indices in reference dict to compare with testvals
        if icomp:
            # make sure icomp is a list if specified
            try: len(icomp)
            except: icomp = [icomp]

        akey = refstat.keys()[0]
        arefdata = refstat[akey]['value']
        acurrdata = currstat[akey]['value']
        # check if reference dictionary is a list or value
        # quanta values are an array or a value
        reflist = True
        try:
            len(arefdata)
        except:
            reflist = False

        # check if current value is a list or value
        try:
            # current statistics dictionary stores a list to compare
            if not reflist and len(acurrdata) > 1:
                del akey, arefdata, acurrdata, reflist
                raise Exception("Current data has more elements than indices set to compare")
            if not icomp:
                icomp = range(len(acurrdata))
            if reflist and len(acurrdata) > len(icomp):
                del akey, arefdata, acurrdata, reflist
                # make sure current data and icomp has the same number of elements if both are list
                raise Exception("Current data has more elements than indices set to compare")
            # The latter indices are ignored if you specified many 'icomp'
            icomp = icomp[:len(acurrdata)]
            currlist = True
        except:
            # current statistics dictionary stores a value to compare
            if reflist and not icomp:
                del akey, arefdata, acurrdata, reflist
                raise ValueError, "No index to compare specified"
            elif not reflist:
                # both are values
                icomp = [0]
            # The latter indices are ignored if you specified many 'icomp'
            icomp = [icomp[0]]
            currlist = False
        if reflist and (max(icomp) > len(arefdata)-1 or min(icomp) < 0):
            del akey, arefdata, acurrdata, reflist, currlist
            raise Exception("Indices of reference data out of range")
        del akey, arefdata, acurrdata

        for stat, dref in refstat.iteritems():
            self.assertEqual(dref['unit'],currstat[stat]['unit'],msg="Units of '%s' are different." % stat)
            for i in range(len(icomp)):
                if reflist:
                    refval = dref['value'][icomp[i]]
                else:
                    refval = dref['value']
                if currlist:
                    currval = currstat[stat]['value'][i]
                else:
                    currval = currstat[stat]['value']
                for i in range(len(icomp)):
                    self.assertAlmostEqual(currval,refval,places=6,\
                                           msg="'%s' position of spectrum %d changed " % (stat, i))
        del refstat, currstat, currval, refval, icomp, reflist, currlist, stat, dref

    def _compareFiles(self, testfile, reffile):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        self.assertTrue(os.path.exists(testfile),
                        msg=("Output file '%s' doesn't exist" % testfile))
        self.assertTrue(os.path.exists(reffile),
                        msg=("Reference file '%s' doesn't exist: " % reffile))
        self.assertTrue(listing.compare(testfile,reffile),
                        'New and reference files are different. %s != %s. '
                        %(testfile,reffile))
        

    ### Actual test scripts ###
    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdstat)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test00(self):
        """Test 0: Default parameters"""
        result = sdstat()
        self.assertFalse(result)

    def test01(self):
        """Test 1: Default parameters + valid input filename """
        self.tid="01"
        infile = self.infile
        outfile = self.outroot+self.tid+self.outsuff

        currstat = sdstat(infile=infile,outfile=outfile)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_allK)
        self._compareQstats(currstat,self.minmaxchan_all)


    def test02(self):
        """Test 2: fluxunit='K' """
        self.tid="02"
        infile = self.infile
        fluxunit = 'K'
        # automatic conversion for GBT data
        telescopeparm = ""
        outfile = self.outroot+self.tid+self.outsuff

        currstat = sdstat(infile=infile,fluxunit=fluxunit,telescopeparm=telescopeparm,outfile=outfile)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_allK)
        self._compareQstats(currstat,self.minmaxchan_all)


    def test03(self):
        """Test 3: fluxunit='Jy' """
        self.tid="03"
        infile = self.infile
        fluxunit = 'Jy'
        # automatic conversion for GBT data
        telescopeparm = ""
        outfile = self.outroot+self.tid+self.outsuff

        currstat = sdstat(infile=infile,fluxunit=fluxunit,telescopeparm=telescopeparm,outfile=outfile)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_allJy)
        self._compareQstats(currstat,self.minmaxchan_all)


    def test04(self):
        """Test 4: specunit='channel' """
        self.tid="04"
        infile = self.infile
        specunit = 'channel'
        outfile = self.outroot+self.tid+self.outsuff

        currstat = sdstat(infile=infile,specunit=specunit,outfile=outfile)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_allK)
        self._compareQstats(currstat,self.minmaxchan_all)


    def test05(self):
        """Test 5: specunit='GHz' """
        self.tid="05"
        infile = self.infile
        specunit = 'GHz'
        outfile = self.outroot+self.tid+self.outsuff

        currstat = sdstat(infile=infile,specunit=specunit,outfile=outfile)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_allK)
        self._compareQstats(currstat,self.minmaxfreq_all)


    def test06(self):
        """Test 6: specunit='km/s' """
        self.tid="06"
        infile = self.infile
        specunit = 'km/s'
        outfile = self.outroot+self.tid+self.outsuff

        currstat = sdstat(infile=infile,specunit=specunit,outfile=outfile)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_allK)
        self._compareQstats(currstat,self.minmaxvelo_all)


    def test07(self):
        """Test 7: maskllist (line) in specunit='channel' """
        self.tid="07"
        infile = self.infile
        outfile = self.outroot+self.tid+self.outsuff
        iflist = [2]
        specunit = 'channel'

        masklist = self.linechan2

        currstat = sdstat(infile=infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_line2)
        self._compareQstats(currstat,self.minmaxchan_line2)


    def test08(self):
        """Test 8: maskllist (line) in specunit='GHz' """
        self.tid="08"
        infile = self.infile
        outfile = self.outroot+self.tid+self.outsuff
        iflist = [2]
        specunit = 'GHz'

        masklist = _get_chanval(infile,self.linechan2,specunit,spw=iflist[0])
        currstat = sdstat(infile=infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_line2)


    def test09(self):
        """Test 9: maskllist (line) in specunit='km/s' """
        self.tid="09"
        infile = self.infile
        outfile = self.outroot+self.tid+self.outsuff
        iflist = [2]
        specunit = 'km/s'

        masklist = _get_chanval(infile,self.linechan2,specunit,spw=iflist[0])
        currstat = sdstat(infile=infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_line2)


    def test10(self):
        """Test 10: invert = True"""
        self.tid="07"
        infile = self.infile
        outfile = self.outroot+self.tid+self.outsuff
        iflist = [0]
        specunit = 'channel'

        masklist = self.linechan0
        invert = True

        currstat = sdstat(infile=infile,specunit=specunit,outfile=outfile,
                          iflist=iflist,masklist=masklist,invertmask=invert)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareVstats(currstat,self.ref_bl0)
        self._compareQstats(currstat,self.minmaxchan_bl0)

    def test11(self):
        """Test1: specify format """
        self.tid="11"
        reference = self.datapath+"refstat"+self.tid
        
        infile = self.infile
        specunit = 'channel'
        outfile = self.outroot+self.tid+self.outsuff
        format = '3.5f'

        currstat = sdstat(infile=infile,specunit=specunit,format=format,outfile=outfile)
        print "Statistics out of the current run:\n",currstat

        # Task sdstat returns a dictionary of statistic values
        self.assertTrue(isinstance(currstat,dict),
                         msg="The returned statistics are not a dictionary")
        self._compareFiles(outfile, reference)
        #self._compareVstats(currstat,self.ref_allK)
        #self._compareQstats(currstat,self.minmaxchan_all)



def suite():
    return [sdstat_test]
