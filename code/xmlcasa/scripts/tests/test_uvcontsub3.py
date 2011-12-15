import os
import numpy
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from tests.test_split import check_eq, SplitChecker
import unittest

'''
Unit tests for task uvcontsub3.

Features tested:
  1. uvcontsub3 will use DATA if CORRECTED_DATA is absent.
  2. It gets the right answer for a known line + 0th order continuum.
  3. It gets the right answer for a known line + 4th order continuum.
  4. It gets the right answer for a known line + 0th order continuum,
     even when fitorder = 4.
'''

uvcdatadir = 'unittest/uvcontsub/'  # in SplitChecker.datapath/

class UVContChecker(SplitChecker):
    """
    Base class for uvcontsub3 unit testing.
    """
    need_to_initialize = True
    records = {}

    def do_split(self, corrsel):
        """
        This is only called do_split because it comes from SplitChecker.
        run_task (uvcontsub3 in this case) would have been a better name.
        """
        record = {}
        try:
            print "\nRunning uvcontsub3"
            uvran = uvcontsub3(self.inpms, fitspw='0:0~5;18~23',
                               fitorder=corrsel, async=False)
        except Exception, e:
            print "Error running uvcontsub3"
            raise e
        specms = self.inpms + '.contsub'
        tb.open(specms)
        record['contsub'] = tb.getcell('DATA', 52)
        tb.close()
        shutil.rmtree(specms)
        self.__class__.records[corrsel] = record
        return uvran

class zeroth(UVContChecker):
    inpms = uvcdatadir + 'known0.ms' 
    corrsels = [0]                    # fitorder, not corr selection.

    def test_line_cont(self):
        """Continuum-subtracted data in line-free region"""
        check_eq(self.records[0]['contsub'][:,21],   # RR, LL
                 numpy.array([ 0.+0.j,  0.+0.j]), 0.0001)
    def test_line_line(self):
        """Continuum-subtracted data in line region"""
        check_eq(self.records[0]['contsub'][:,9],   # RR, LL
                 numpy.array([87.+26.j, 31.+20.j]), 0.0001)
        
class fourth(UVContChecker):
    inpms = uvcdatadir + 'known4.ms'
    corrsels = [4]                    # fitorder, not corr selection.

    def test_line(self):
        """Continuum-subtracted data"""
        check_eq(self.records[4]['contsub'],   # [[RR], [LL]]
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

class combspw(UVContChecker):
    inpms = uvcdatadir + 'combspw.ms'
    corrsels = [0, 1]                    # fitorder, not corr selection.

    def do_split(self, fitord):
        """
        This is only called do_split because it comes from SplitChecker.
        run_task (uvcontsub3 in this case) would have been a better name.
        """
        record = {}
        try:
            print "\nRunning uvcontsub3"
            uvran = uvcontsub3(self.inpms, fitspw='1~10:5~122,15~22:5~122',
                               spw='6~14', combine='spw',
                               fitorder=fitord, async=False)
        except Exception, e:
            print "Error running uvcontsub3"
            raise e
        specms = self.inpms + '.contsub'
        tb.open(specms)
        
        # Same data, different row number as compared to uvcontsub or the input MS.
        record['contsub'] = tb.getcell('DATA', 31)[0][73]
        
        tb.close()
        shutil.rmtree(specms)
        self.__class__.records[fitord] = record
        return uvran

    def test_comb0(self):
        """combspw fitorder=0 line estimate"""
        check_eq(self.records[0]['contsub'], -6.2324+17.9865j, 0.001)

    def test_comb1(self):
        """combspw fitorder=1 line estimate"""
        check_eq(self.records[1]['contsub'], -6.2533+17.6584j, 0.001)
        
class knowncombspw(UVContChecker):
    inpms = uvcdatadir + 'knowncombspw.ms'
    corrsels = [1]                    # fitorder, not corr selection.

    def do_split(self, fitord):
        """
        This is only called do_split because it comes from SplitChecker.
        run_task (uvcontsub3 in this case) would have been a better name.
        """
        record = {}
        try:
            print "\nRunning uvcontsub3"
            uvran = uvcontsub3(self.inpms, fitspw='0,1:0~15,3:23~31,4',
                               spw='1~3', combine='spw',
                               fitorder=fitord, async=False)
        except Exception, e:
            print "Error running uvcontsub3"
            raise e
        specms = self.inpms + '.contsub'
        tb.open(specms)
        
        # Same data, different row number as compared to uvcontsub or the input MS.
        
        record['blue'] = tb.getcell('DATA',  7)[0]
        record['line'] = tb.getcell('DATA', 37)[0]
        record['red']  = tb.getcell('DATA', 61)[0]
        
        tb.close()
        shutil.rmtree(specms)
        self.__class__.records[fitord] = record
        return uvran

    def test_blue(self):
        """The blueward side"""
        check_eq(self.records[1]['blue'],
                 numpy.array([ -3.03268433e-04 +1.00890160e-01j,
                               -3.02314758e-04 +9.34381485e-02j,
                               -3.00884247e-04 +8.59861374e-02j,
                               -2.99692154e-04 +7.85341263e-02j,
                               -2.98261642e-04 +7.10821152e-02j,
                               -2.96354294e-04 +6.36301041e-02j,
                               -2.92539597e-04 +5.61790466e-02j,
                               -2.84433365e-04 +4.87308502e-02j,
                               -2.66551971e-04 +4.12874222e-02j,
                               -2.25067139e-04 +3.38554382e-02j,
                               -1.28984451e-04 +2.64511108e-02j,
                                8.96453857e-05 +1.91068649e-02j,
                                5.75304031e-04 +1.18970871e-02j,
                                1.62601471e-03 +4.96959686e-03j,
                                3.83996964e-03 -1.37615204e-03j,
                                8.37826729e-03 -6.55937195e-03j,
                                1.74312592e-02 -9.48524475e-03j,
                                3.49960327e-02 -8.15582275e-03j,
                                6.81409836e-02 +9.63211060e-04j,
                                1.28964663e-01 +2.39229202e-02j,
                                2.37491846e-01 +7.07340240e-02j,
                                4.25746441e-01 +1.57408714e-01j,
                                7.43155241e-01 +3.08660507e-01j,
                                1.26323771e+00 +5.61248779e-01j,
                                2.09118795e+00 +9.67771530e-01j,
                                3.37145138e+00 +1.60044956e+00j,
                                5.29375315e+00 +2.55414772e+00j,
                                8.09537888e+00 +3.94750881e+00j,
                                1.20570116e+01 +5.92087221e+00j,
                                1.74892941e+01 +8.62956047e+00j,
                                2.47078934e+01 +1.22314072e+01j,
                                3.39962006e+01 +1.68681087e+01j]), 0.001)

    def test_line(self):
        """The known line"""
        check_eq(self.records[1]['line'],
                 numpy.array([   1.53886628 +0.8961916j ,    2.52163553 +1.38012409j,
                                 4.02423143 +2.12396812j,    6.25472546 +3.23176193j,
                                 9.46806622 +4.83097982j,   13.95866108 +7.06882477j,
                                20.04272079+10.10340309j,   28.02855110+14.08886528j,
                                38.17472839+19.15450096j,   50.63872910+25.37904739j,
                                65.42163086+32.76304626j,   82.31739807+41.20347595j,
                               100.87736511+50.47600555j,  120.40031433+60.23003006j,
                               139.95661926+70.00072479j,  158.44956970+79.23975372j,
                               174.71115112+87.36308289j,  187.62126160+93.8106842j ,
                               196.23449707+98.10986328j,  199.89437866+99.93234253j,
                               198.31597900+99.13568878j,  191.62258911+95.78153992j,
                               180.32984924+90.12772369j,  165.28004456+82.59536743j,
                               147.53840637+73.71709442j,  128.26898193+64.07492828j,
                               108.61006165+54.23802185j,   89.56748199+44.70927429j,
                                71.93868256+35.88742447j,   56.27381897+28.04753876j,
                                42.87281799+21.33958817j,   31.81188011+15.80166435j]),
                 0.001)

    def test_red(self):
        """The redward side"""
        check_eq(self.records[1]['red'],
                 numpy.array([  1.23712059e+02 +6.19947433e+01j,
                                1.04113014e+02 +5.21877708e+01j,
                                8.53355255e+01 +4.27915688e+01j,
                                6.81218643e+01 +3.41772842e+01j,
                                5.29632835e+01 +2.65905476e+01j,
                                4.01046829e+01 +2.01537933e+01j,
                                2.95765038e+01 +1.48822517e+01j,
                                2.12436962e+01 +1.07083941e+01j,
                                1.48608809e+01 +7.50953341e+00j,
                                1.01248760e+01 +5.13407898e+00j,
                                6.71838522e+00 +3.42338037e+00j,
                                4.34179068e+00 +2.22763014e+00j,
                                2.73274589e+00 +1.41565514e+00j,
                                1.67514348e+00 +8.79401207e-01j,
                                1.00004458e+00 +5.34399033e-01j,
                                5.81416249e-01 +3.17632675e-01j,
                                3.29175472e-01 +1.84060097e-01j,
                                1.81462049e-01 +1.02749825e-01j,
                                9.73758698e-02 +5.32541275e-02j,
                                5.08389473e-02 +2.25324631e-02j,
                                2.57951021e-02 +2.55870819e-03j,
                                1.26885176e-02 -1.14479065e-02j,
                                6.01756573e-03 -2.22358704e-02j,
                                2.71499157e-03 -3.13396454e-02j,
                                1.12497807e-03 -3.95870209e-02j,
                                3.80396843e-04 -4.74128723e-02j,
                                4.14252281e-05 -5.50346375e-02j,
                               -1.08182430e-04 -6.25619888e-02j,
                               -1.72197819e-04 -7.00464249e-02j,
                               -1.98423862e-04 -7.75127411e-02j,
                               -2.08497047e-04 -8.49704742e-02j,
                               -2.11834908e-04 -9.24243927e-02j]), 0.001)
        
def suite():
    return [zeroth, fourth, combspw, knowncombspw]
