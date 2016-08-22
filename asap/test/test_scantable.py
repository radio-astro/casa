import sys
import os
import shutil
import datetime
from nose.tools import *
import asap
from asap import scantable, selector, mask_not
from asap.logging import asaplog
# no need for log messages
asaplog.disable()

def tempdir_setup():
    os.makedirs("test_temp")

def tempdir_teardown():
    shutil.rmtree("test_temp", True)

class TestScantable(object):
    def setup(self):
        pth = os.path.dirname(__file__)
        s = scantable(os.path.join(pth, "data", "MOPS.rpf"), average=True)
        sel = selector()
        # make sure this order is always correct - in can be random
        sel.set_order(["SCANNO", "POLNO"])
        s.set_selection(sel)
        self.st = s.copy()
        restfreqs = [86.243]     # 13CO-1/0, SiO the two IF
        self.st.set_restfreqs(restfreqs,"GHz")

    def test_init(self):
        fname = os.path.join(os.path.dirname(__file__), "data", "MOPS.rpf")
        st = scantable(fname, average=False)
        assert_equal(st.ncycle(), 32)
        st = scantable(fname, average=True)
        assert_equal(st.ncycle(), 2)
        st = scantable(fname, unit="Jy")
        assert_equal(st.get_fluxunit(), "Jy")
        st = scantable(fname, unit="K")
        assert_equal(st.get_fluxunit(), "K")
        assert_raises(RuntimeError, scantable, fname, unit="junk")
        st = scantable([fname,fname], average=False)
        assert_equal(st.nscan(), 4)

    def test_copy(self):
        st = self.st.copy()
        assert_not_equal(id(st), id(self.st))

    def test_drop_scan(self):
        st = self.st.drop_scan([1])
        assert_equal(st.nscan(), 1)

    def test_get_scan(self):
        st = self.st.get_scan([1])
        assert_equal(st.nscan(), 1)
        st = self.st.get_scan("Orion_SiO_R")
        assert_equal(st.get_sourcename()[-1], "Orion_SiO_R")
        assert_equal(st.nscan(), 1)

    def test_get_spectrum(self):
        spec = self.st.get_spectrum(0)
        assert_almost_equal(max(spec), 215.279830933)

    def test_get_mask(self):
        spec = self.st.get_mask(0)
        assert_equal(len(spec), 4096)

    def test_set_spectrum(self):
        spec = [ 1.0 for i in range(self.st.nchan()) ]
        self.st.set_spectrum(spec, 0)
        spec1 = self.st.get_spectrum(0)
        assert_almost_equal(max(spec1), 1.0)

    def test_selection(self):
        sel = selector()
        sel.set_polarisations("YY")
        self.st.set_selection(sel)
        assert_equal(self.st.getpolnos(), [1])
        sel1 = self.st.get_selection()
        assert_equal(sel1.get_pols(), [1])
        self.st.set_selection(pols="XX")
        assert_equal(self.st.getpolnos(), [0])


    def stats(self, key, value, mask=False):
        msk = None
        if mask:
            msk = self.st.create_mask([0,100], [3900,4096])
        sval = self.st.stats(stat=key, mask=msk)
        assert_almost_equal(sval[0], value)

    def test_masked_stats(self):
        stats = { 'min': 113.767166138,
                  'max':128.21571350, 'sumsq':4180516.75,
                  'sum':35216.87890625, 'mean':118.5753479,
                  'var':15.75608253, 'stddev':3.9693932533,
                  'avdev':3.395271778, 'rms':118.6415405,
                  'median':117.5024261}
        for k,v in stats.items():
            yield self.stats, k, v, True

    def test_stats(self):
        stats = { 'min': 113.767166138,
                  'max':215.279830933, 'sumsq':128759200.0,
                  'sum':720262.375, 'mean':175.845306396,
                  'var':513.95324707, 'stddev':22.6705360413,
                  'avdev':16.3966751099, 'rms':177.300170898,
                  'median':182.891845703}
        for k,v in stats.items():
            yield self.stats, k, v

    def test_get_column_names(self):
        cnames = ['SCANNO', 'CYCLENO', 'BEAMNO', 'IFNO',
                  'POLNO', 'FREQ_ID', 'MOLECULE_ID', 'REFBEAMNO', 'FLAGROW',
                  'TIME', 'INTERVAL', 'SRCNAME', 'SRCTYPE',
                  'FIELDNAME', 'SPECTRA', 'FLAGTRA', 'TSYS',
                  'DIRECTION', 'AZIMUTH', 'ELEVATION',
                  'OPACITY', 'TCAL_ID', 'FIT_ID',
                  'FOCUS_ID', 'WEATHER_ID', 'SRCVELOCITY',
                  'SRCPROPERMOTION', 'SRCDIRECTION',
                  'SCANRATE']
        assert_equal(self.st.get_column_names(), cnames)

    def test_get_tsys(self):
        assert_almost_equal(self.st.get_tsys()[0], 175.830429077)

    def test_set_tsys(self):
        s = self.st.copy()
        newval = 100.0
        s.set_tsys(newval, 0)
        assert_almost_equal(s.get_tsys()[0], newval)
        s2 = self.st.copy()
        s2.set_tsys(newval)
        out = s2.get_tsys()
        for i in xrange(len(out)):
            assert_almost_equal(out[i], newval)

    def test_get_time(self):
        assert_equal(self.st.get_time(0), '2008/03/12/09:32:50')
        dt = datetime.datetime(2008,3,12,9,32,50)
        assert_equal(self.st.get_time(0, True), dt)

    def test_get_inttime(self):
        assert_almost_equal(self.st.get_inttime()[0], 30.720016479)

    def test_get_sourcename(self):
        assert_equal(self.st.get_sourcename(0), 'Orion_SiO_R')
        assert_equal(self.st.get_sourcename(),
                         ['Orion_SiO_R', 'Orion_SiO_R',
                          'Orion_SiO', 'Orion_SiO'])

    def test_set_sourcename(self):
        s = self.st.copy()
        newname = "TEST"
        s.set_sourcename(newname)
        assert_equal(s.get_sourcename(), [newname]*4)

    def test_get_azimuth(self):
        assert_almost_equal(self.st.get_azimuth()[0], 5.628767013)

    def test_get_elevation(self):
        assert_almost_equal(self.st.get_elevation()[0], 1.01711678504)

    def test_get_parangle(self):
        assert_almost_equal(self.st.get_parangle()[0], 2.5921990871)

    def test_get_direction(self):
        assert_equal(self.st.get_direction()[0], 'J2000 05:35:14.5 -04.52.29.5')

    def test_get_directionval(self):
        dv = self.st.get_directionval()[0]
        assert_almost_equal(dv[0], 1.4627692699)
        assert_almost_equal(dv[1], -0.0850824415)

    def test_unit(self):
        self.st.set_unit('')
        self.st.set_unit('GHz')
        self.st.set_unit('km/s')
        assert_raises(RuntimeError, self.st.set_unit, 'junk')
        assert_equals(self.st.get_unit(), 'km/s')

    def test_average_pol(self):
        ap = self.st.average_pol()
        assert_equal(ap.npol(), 1)

    def test_drop_scan(self):
        s0 = self.st.drop_scan(1)
        assert_equal(s0.getscannos(), [0])
        s1 = self.st.drop_scan([0])
        assert_equal(s1.getscannos(), [1])

    def test_flag(self):
        q = self.st.auto_quotient()
        q.set_unit('km/s')
        q0 = q.copy()
        q1 = q.copy()
        msk = q0.create_mask([-10,20])
        q0.flag(mask=mask_not(msk))
        q1.flag(mask=msk)
        assert_almost_equal(q0.stats(stat='max')[0], 95.62171936)
        assert_almost_equal(q1.stats(stat='max')[0], 2.66563416)


    def test_average_time_weight(self):
        weights = {'none': 236.61423,
                   'var' : 232.98752,
                   'tsys' : 236.37482,
                   'tint' : 236.61423,
                   'tintsys' : 236.37482,
                   'median' : 236.61423,
                   }

        for k,v in weights.items():
            yield self.av_weight, k, v

    def av_weight(self, weight, result):
        out = self.st.average_time(weight=weight)
        assert_almost_equals(max(out.get_spectrum(0)), result, 5)

    @with_setup(tempdir_setup, tempdir_teardown)
    def test_save(self):
        fname = os.path.join("test_temp", 'scantable_test.%s')
        formats = [(fname % 'sdfits', 'SDFITS', True),
                   (fname % 'ms', 'MS2', True),
                   (fname % 'class.fits', 'CLASS', False),
                   (fname % 'fits', 'FITS', False),
                   (fname % 'txt', 'ASCII', False),
                   ]
        for fmt in formats:
            yield self.save, fmt

    def save(self, args):
        fname = args[0]
        self.st.save(fname, args[1], True)
        # do some verification args[2] == True
        if args[-1]:
            s = scantable(fname)
            ds = self.st - s
            assert_equals(self.st.getpolnos(), s.getpolnos())
            assert_equals(self.st.getscannos(), s.getscannos())
            assert_equals(self.st.getifnos(), s.getifnos())
            assert_equals(self.st.getbeamnos(), s.getbeamnos())
            # see if the residual spectra are ~ 0.0
            for spec in ds:
                assert_almost_equals(sum(spec)/len(spec), 0.0, 5)

    def test_auto_poly_baseline(self):
        q = self.st.auto_quotient()
        b = q.auto_poly_baseline(insitu=False)
        res_rms = (q-b).stats('rms')
        assert_almost_equals(res_rms[0], 0.38370, 5)
        assert_almost_equals(res_rms[1], 0.38780, 5)        
        

    def test_poly_baseline(self):
        q = self.st.auto_quotient()
        msk = q.create_mask([0.0, 1471.0], [1745.0, 4095.0])
        b = q.poly_baseline(order=0, mask=msk,insitu=False)
        res_rms = (q-b).stats('rms')
        assert_almost_equals(res_rms[0], 0.38346, 5)
        assert_almost_equals(res_rms[1], 0.38780, 5)        


    def test_reshape(self):
        cp = self.st.copy()
        n = cp.nchan()
        rs = cp.reshape(10,-10,False)
        assert_equals(n-20, rs.nchan())
        assert_equals(cp.nchan(), n)
        rs = cp.reshape(10,n-11,False)
        assert_equals(n-20, rs.nchan())
        cp.reshape(10,-10,True)
        assert_equals(n-20, cp.nchan())
