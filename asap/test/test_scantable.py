import unittest
import datetime
from asap import scantable, selector, rcParams, mask_not
rcParams["verbose"] = False

class ScantableTest(unittest.TestCase):
    def setUp(self):

        self.st = scantable("data/MOPS.rpf", average=True)
        restfreqs = [86.243]     # 13CO-1/0, SiO the two IF
        self.st.set_restfreqs(restfreqs,"GHz")

    def test_init(self):
        st = scantable("data/MOPS.rpf", average=False)
        self.assertEqual(st.ncycle(), 32)
        st = scantable("data/MOPS.rpf", average=True)
        self.assertEqual(st.ncycle(), 2)
        st = scantable("data/MOPS.rpf", unit="Jy")
        self.assertEqual(st.get_fluxunit(), "Jy")
        st = scantable("data/MOPS.rpf", unit="K")
        self.assertEqual(st.get_fluxunit(), "K")
        self.assertRaises(RuntimeError, scantable, "data/MOPS.rpf", unit="junk")
        st = scantable(["data/MOPS.rpf","data/MOPS.rpf"], average=False)
        self.assertEqual(st.nscan(), 4)

    def test_copy(self):
        st = self.st.copy()
        self.assertNotEqual(id(st), id(self.st))

    def test_drop_scan(self):
        st = self.st.drop_scan([1])
        self.assertEqual(st.nscan(), 1)

    def test_get_scan(self):
        st = self.st.get_scan([1])
        self.assertEqual(st.nscan(), 1)
        st = self.st.get_scan("Orion_SiO_R")
        self.assertEqual(st.get_sourcename()[-1], "Orion_SiO_R")
        self.assertEqual(st.nscan(), 1)

    def test_get_spectrum(self):
        spec = self.st.get_spectrum(0)
        self.assertAlmostEqual(max(spec), 215.279830933)

    def test_get_mask(self):
        spec = self.st.get_mask(0)
        self.assertEqual(len(spec), 4096)

    def test_set_spectrum(self):
        spec = [ 1.0 for i in range(self.st.nchan()) ]
        self.st.set_spectrum(spec, 0)
        spec1 = self.st.get_spectrum(0)
        self.assertAlmostEqual(max(spec1), 1.0)

    def test_selection(self):
        sel = selector()
        sel.set_polarisations("YY")
        self.st.set_selection(sel)
        self.assertEqual(self.st.getpolnos(), (1,))
        sel1 = self.st.get_selection()
        self.assertEqual(sel1.get_pols(), [1])
        self.st.set_selection(pols="XX")
        self.assertEqual(self.st.getpolnos(), (0,))

    def test_stats(self):
        stats = { 'min': 113.767166138,
                  'max':215.279830933, 'sumsq':128759200.0,
                  'sum':720262.375, 'mean':175.845306396,
                  'var':513.95324707, 'stddev':22.6705360413,
                  'avdev':16.3966751099, 'rms':177.300170898,
                  'median':182.891845703}
        for k,v in stats.iteritems():
            sval = self.st.stats(stat=k)
            self.assertAlmostEqual(sval[0], v)
        msk = self.st.create_mask([0,100], [3900,4096])
        self.assertAlmostEqual(self.st.stats("sum", msk)[0], 35216.87890625)

    def test_get_column_names(self):
        cnames = ['SCANNO', 'CYCLENO', 'BEAMNO', 'IFNO',
                  'POLNO', 'FREQ_ID', 'MOLECULE_ID', 'REFBEAMNO',
                  'TIME', 'INTERVAL', 'SRCNAME', 'SRCTYPE',
                  'FIELDNAME', 'SPECTRA', 'FLAGTRA', 'TSYS',
                  'DIRECTION', 'AZIMUTH', 'ELEVATION',
                  'OPACITY', 'TCAL_ID', 'FIT_ID',
                  'FOCUS_ID', 'WEATHER_ID', 'SRCVELOCITY',
                  'SRCPROPERMOTION', 'SRCDIRECTION',
                  'SCANRATE']
        self.assertEqual(self.st.get_column_names(), cnames)

    def test_get_tsys(self):
        self.assertAlmostEqual(self.st.get_tsys()[0], 175.830429077)

    def test_get_time(self):
        self.assertEqual(self.st.get_time(0), '2008/03/12/09:32:50')
        dt = datetime.datetime(2008,3,12,9,32,50)
        self.assertEqual(self.st.get_time(0, True), dt)

    def test_get_inttime(self):
        self.assertAlmostEqual(self.st.get_inttime()[0], 30.720016479)

    def test_get_sourcename(self):
        self.assertEqual(self.st.get_sourcename(0), 'Orion_SiO_R')
        self.assertEqual(self.st.get_sourcename()[:2], ['Orion_SiO_R', 'Orion_SiO'])

    def test_get_azimuth(self):
        self.assertAlmostEqual(self.st.get_azimuth()[0], 5.628767013)

    def test_get_elevation(self):
        self.assertAlmostEqual(self.st.get_elevation()[0], 1.01711678504)

    def test_get_parangle(self):
        self.assertAlmostEqual(self.st.get_parangle()[0], 2.5921990871)

    def test_get_direction(self):
        self.assertEqual(self.st.get_direction()[0], '05:35:14.5 -04.52.29.5')

    def test_get_directionval(self):
        dv = self.st.get_directionval()[0]
        self.assertAlmostEqual(dv[0], 1.4627692699)
        self.assertAlmostEqual(dv[1], -0.0850824415)

    def test_unit(self):
        self.st.set_unit('')
        self.st.set_unit('GHz')
        self.st.set_unit('km/s')
        self.assertRaises(RuntimeError, self.st.set_unit, 'junk')
        self.assertEquals(self.st.get_unit(), 'km/s')

    def test_average_pol(self):
        ap = self.st.average_pol()
        self.assertEqual(ap.npol(), 1)

    def test_drop_scan(self):
        s0 = self.st.drop_scan(1)
        self.assertEqual(s0.getscannos(), (0,))
        s1 = self.st.drop_scan([0])
        self.assertEqual(s1.getscannos(), (1,))

    def test_flag(self):
        q = self.st.auto_quotient()
        q.set_unit('km/s')
        q0 = q.copy()
        q1 = q.copy()
        msk = q0.create_mask([-10,20])
        q0.flag(mask=mask_not(msk))
        self.assertAlmostEqual(q0.stats(stat='max')[0], 95.62171936)
        q1.flag(mask=msk)
        self.assertAlmostEqual(q1.stats(stat='max')[0], 2.66563416)

if __name__ == '__main__':
    unittest.main()
