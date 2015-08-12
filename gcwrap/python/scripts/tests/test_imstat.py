import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import math
import numpy
import numbers

#run using
# `which casapy` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py --mem test_imstat
#

'''
Unit tests for task imstat.
'''
class imstat_test(unittest.TestCase):
    
    # Input and output names
    moment = 'moment_map.im'
    s150 = '150arcsec_pix.im'
    s15 = '15arcsec_pix.im'
    s0_015 = '0.015arcsec_pix.im'
    s0_0015 = '0.0015arcsec_pix.im'
    s0_00015 = '0.00015arcsec_pix.im'
    linear_coords = 'linearCoords.fits'
    fourdim = '4dim.im'
    kimage = "ktest.im"
    res = None

    def _compare(self, resold, resnew, helpstr):
        mytype = type(resold)
        self.assertTrue(mytype == type(resnew), helpstr + ": types differ")
        if mytype == dict:
            for k in resold.keys():
                self._compare(resold[k], resnew[k], helpstr)
        elif mytype == numpy.ndarray:
            oldarray = resold.ravel()
            newarray = resnew.ravel()
            self.assertTrue(
                len(oldarray) == len(newarray),
                helpstr + ": array lengths not equal"
            )
            for i in range(len(oldarray)):
                self._compare(oldarray[i], newarray[i], helpstr)
        elif mytype == str:
            self.assertTrue(
                resold == resnew,
                helpstr + ": string inequality, old = " + resold + ", new = " + resnew
            )
        elif isinstance(resold, numbers.Integral) or mytype == numpy.int32:
            self.assertTrue(
                resold == resnew,
                helpstr + ": integral inequality, old = " + str(resold) + ", new = " + str(resnew)
            )
        elif isinstance(resold, numbers.Real):
            self.assertTrue(
                resold == resnew
                or abs(resnew/resold - 1) < 1e-6,
                helpstr + "float inequality: old = " + str(resold)
                + ", new = " + str(resnew)
            )
        else:
            self.assertTrue(False, "Unhandled type " + str(mytype))

    def setUp(self):
        self.res = None
        self._myia = iatool()
        default(clean)
        self.datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/imstat/'
    
    def tearDown(self):
        self._myia.done()
        for dir in [
            self.moment, self.s150, self.s15, self.s0_015, self.s0_0015,
            self.s0_00015, self.linear_coords, self.fourdim
        ]:
            if os.path.isfile(dir):
                os.remove(dir)
            elif (os.path.exists(dir)):
                shutil.rmtree(dir)
        self.assertTrue(len(tb.showcache()) == 0)

    def test001(self):
        """Test 1: verify moment maps can have flux densities computed in statistics"""
        shutil.copytree(self.datapath+self.moment, self.moment)
        stats = imstat(imagename=self.moment)
        mean = stats['mean']
        npts = stats['npts']

        _myia = iatool()
        _myia.open(self.moment)
        summary = _myia.summary()
        _myia.close()
        rainc = qa.abs(qa.quantity(summary['incr'][0],'rad'))
        rainc = qa.convert(rainc,'arcsec')
        decinc = qa.abs(qa.quantity(summary['incr'][1],'rad'))
        decinc = qa.convert(decinc,'arcsec')
        beam = summary['restoringbeam']
        major = beam['major']
        minor = beam['minor']
        pixperbeam = qa.div(qa.mul(major,minor),(qa.mul(rainc,decinc)))['value']*(math.pi/(4*math.log(2)))
        got = stats['flux'][0]
        expected = (mean*npts/pixperbeam)[0]
        self.assertTrue(abs(got - expected) < 1e-11)
 
    def test002(self):
        """ Test 2: test position format for 150 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s150, self.s150)
        _myia = iatool()
        _myia.open(self.s150)
        stats = _myia.statistics()
        _myia.close()
        self.assertTrue(stats['blcf'] == '15:43:21.873, -00.17.47.274, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:40.165, +05.11.29.923, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:43:25.618, +04.22.40.617, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:00:27.115, +10.20.37.699, I, 1.41332e+09Hz')

    def test003(self):
        """ Test 3: test position format for 15 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s15, self.s15)
        _myia = iatool()
        _myia.open(self.s15)
        print "*** before "
        stats = _myia.statistics()
        print "*** after"
        _myia.close()
        self.assertTrue(stats['blcf'] == '15:24:08.404, +04.31.59.181, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:04.016, +05.04.44.999, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:24:08.491, +04.59.59.208, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:19:52.390, +05.35.44.246, I, 1.41332e+09Hz')

    def test004(self):
        """ Test 4: test position format for 0.015 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s0_015, self.s0_015)
        _myia = iatool()
        _myia.open(self.s0_015)
        stats = _myia.statistics()
        _myia.close() 
        self.assertTrue(stats['blcf'] == '15:22:00.1285, +05.03.58.0800, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:00.0040, +05.04.00.0450, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:22:00.1285, +05.03.59.7600, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:21:59.8725, +05.04.01.9050, I, 1.41332e+09Hz')

    def test005(self):
        """ Test 5: test position format for 0.0015 arcsec pixel image is correct """
        print "*** test 5"
        _myia = iatool()
        shutil.copytree(self.datapath+self.s0_0015, self.s0_0015)
        _myia.open(self.s0_0015)
        stats = _myia.statistics()
        _myia.close()
        print "*** blcf xxx " + str(stats['blcf'])
        self.assertTrue(stats['blcf'] == '15:22:00.01285, +05.03.59.80800, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:00.00040, +05.04.00.00450, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:22:00.01285, +05.03.59.97600, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:21:59.98725, +05.04.00.19050, I, 1.41332e+09Hz')

    def test006(self):
        """ Test 6: test position format for 0.00015 arcsec pixel image is correct """
        shutil.copytree(self.datapath+self.s0_00015, self.s0_00015)
        _myia = iatool()
        _myia.open(self.s0_00015)
        stats = _myia.statistics()
        _myia.close()
        self.assertTrue(stats['blcf'] == '15:22:00.001285, +05.03.59.980800, I, 1.41332e+09Hz') 
        self.assertTrue(stats['maxposf'] == '15:22:00.000040, +05.04.00.000450, I, 1.41332e+09Hz')
        self.assertTrue(stats['minposf'] == '15:22:00.001285, +05.03.59.997600, I, 1.41332e+09Hz')
        self.assertTrue(stats['trcf'] == '15:21:59.998725, +05.04.00.019050, I, 1.41332e+09Hz')

    def test007(self):
        """ Test 7: test that box parameter can have spaces, CAS-2050 """
        shutil.copytree(self.datapath+self.s0_00015, self.s0_00015)
        box = '0, 0,  1 ,   1'
        stats = imstat(imagename=self.s0_00015, box=box)
        self.assertTrue(stats['npts'] == 4) 
        
    def test008(self):
        """ Test 8: verify fix for CAS-2195"""
        def test_statistics(image):
            _myia = iatool()
            _myia.open(myim)
            stats = _myia.statistics()
            _myia.done()
            return stats
        
        def test_imstat(image):
            return imstat(image)
            
        myim = self.linear_coords
        shutil.copy(self.datapath + myim, myim)
        expected_max = [3, 10]
        expected_min = [4, 0]
        for code in [test_statistics, test_imstat]:
            stats = code(myim)
            self.assertTrue((stats['maxpos'] == expected_max).all())
            self.assertTrue((stats['minpos'] == expected_min).all())
            
    def test009(self):
        """ Test 9: choose axes works"""
        def test_statistics(image, axes):
            _myia = iatool()
            _myia.open(myim)
            stats = _myia.statistics(axes=axes)
            _myia.done()
            return stats
        
        def test_imstat(image, axes):
            return imstat(image, axes=axes)
            
        myim = self.fourdim
        shutil.copytree(self.datapath + myim, myim)
        axes = [-1, [0, 1, 2], [0, 1], 3]
        expected_mean = [
                [59.5], [ 57.5,  58.5,  59.5,  60.5,  61.5],
                [
                    [50., 51., 52., 53., 54.],
                    [55., 56., 57., 58., 59.],
                    [60., 61., 62., 63., 64.],
                    [65., 66., 67., 68., 69.]
                ],
                [
                    [
                        [2., 7., 12., 17.],
                        [22., 27., 32., 37.],
                        [42., 47., 52., 57.]
                    ],
                    [
                        [62., 67., 72., 77.],
                        [ 82.,  87.,  92., 97.],
                        [ 102., 107., 112., 117.]
                    ]
                ]
            ]
        expected_sumsq = [
                [568820], [ 108100.,  110884.,  113716.,  116596.,  119524.],
                [
                    [ 22000., 22606., 23224., 23854., 24496.],
                    [ 25150., 25816., 26494., 27184., 27886.],
                    [ 28600., 29326., 30064., 30814., 31576.],
                    [ 32350., 33136., 33934., 34744., 35566.]
                ],
                [
                    [
                        [ 3.00000000e+01, 2.55000000e+02, 7.30000000e+02, 1.45500000e+03],
                        [ 2.43000000e+03, 3.65500000e+03, 5.13000000e+03, 6.85500000e+03],
                        [  8.83000000e+03,   1.10550000e+04,   1.35300000e+04, 1.62550000e+04]
                    ],
                    [
                        [  1.92300000e+04,   2.24550000e+04,   2.59300000e+04, 2.96550000e+04],
                        [  3.36300000e+04,   3.78550000e+04,   4.23300000e+04, 4.70550000e+04],
                        [  5.20300000e+04,   5.72550000e+04,   6.27300000e+04, 6.84550000e+04]
                    ]
                ]
            ]
        for i in range(len(axes)):
            for code in [test_statistics, test_imstat]:
                stats = code(myim, axes[i])
                self.assertTrue((stats['mean'] == expected_mean[i]).all())
                self.assertTrue((stats['sumsq'] == expected_sumsq[i]).all())
            
            """
    def test_robust(self):
        *"" Confirm robust parameter*""
        def test_statistics(image, robust):
            _myia = iatool()
            _myia.open(myim)
            stats = _myia.statistics(robust=robust)
            _myia.done()
            return stats
        def test_imstat(image, robust):
            return imstat(image, robust=robust)
        myim = self.fourdim
        shutil.copytree(self.datapath + myim, myim)
        for robust in [True, False]:
            for code in [test_statistics, test_imstat]:
                stats = code(myim, robust)
                self.assertTrue(stats.has_key('median') == robust)
                self.assertTrue(stats.has_key('medabsdevmed') == robust)
                self.assertTrue(stats.has_key('quartile') == robust)
                
                """
                
    def test_stretch(self):
        """ ia.statistics(): Test stretch parameter"""
        yy = iatool()
        mymask = "maskim"
        yy.fromshape(mymask, [200, 200, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [200,200,1,20]
        imagename = "tmp.im"
        yy.fromshape(imagename, shape)
        yy.addnoise()
        self.assertRaises(
            Exception,
            yy.statistics,
            mask=mymask + ">0", stretch=False
        )
        zz = yy.statistics(
            mask=mymask + ">0", stretch=True
        )
        self.assertTrue(zz and type(zz) == type({}))
        yy.done()
        
        zz = imstat(
            imagename=imagename, mask=mymask + ">0", stretch=False
        )
        self.assertFalse(zz)

        zz = imstat(
            imagename=imagename, mask=mymask + ">0", stretch=True
        )
        self.assertTrue(type(zz) == type({}) and (not zz == {}))
        yy.done()
   
    def test010(self):
        """test logfile """
        def test_statistics(image, axes, logfile, append):
            _myia = iatool()
            _myia.open(image)
            stats = _myia.statistics(
                axes=axes, logfile=logfile, append=append, list=True, verbose=True
            )
            _myia.done()
            return stats
        
        def test_imstat(image, axes, logfile, append):
            return imstat(image, axes=axes, logfile=logfile, append=append, verbose=True)
            
        logfile = "imstat.log"
        i = 1
        myim = self.fourdim
        shutil.copytree(self.datapath + myim, myim)
        for code in [test_statistics, test_imstat]:
            append = False
            if i == 2:
                append = True
            stats = code(myim, [0], logfile, append)
            size = os.path.getsize(logfile)
            print "size " + str(size)
            # appending, second time through size should double
            self.assertTrue(size > 1.2e4*i and size < 1.3e4*i )
            i = i+1


    def test011(self):
        """ test multiple region support"""
        shape = [10, 10, 10]
        myia = self._myia
        myia.fromshape("test011.im", shape)
        box = "0, 0, 2, 2, 4, 4, 6, 6"
        chans = "0~4, 6, >8"
        reg = rg.frombcs(
            myia.coordsys().torecord(), shape,
            box=box, chans=chans
        )
        bb = myia.statistics(region=reg)
        self.assertTrue(bb["npts"][0] == 126)
        bb = imstat(imagename=myia.name(), chans=chans, box=box)
        self.assertTrue(bb["npts"][0] == 126)
        
        rfilename = "myreg.reg"
        rg.tofile(rfilename, reg)
        bb = myia.statistics(region=rfilename)
        self.assertTrue(bb["npts"][0] == 126)
            
    def test012(self):
        """ Test multi beam support"""
        myia = self._myia
        shape = [15, 20, 4, 10]
        myia.fromshape("", shape)
        xx = numpy.array(range(shape[0]*shape[1]))
        xx.resize(shape[0], shape[1])
        myia.putchunk(xx, replicate=True)
        myia.setbrightnessunit("Jy/pixel")
        def check(myia, axes, exp):
            for i in range(shape[2]):
                for j in range(shape[3]):
                    got = myia.statistics(
                        axes=axes,
                        region=rg.box(
                            blc = [0, 0, i, j],
                            trc=[shape[0]-1, shape[1]-1, i, j]
                        )
                    )
                    self.assertTrue(len(got.keys()) == len(exp.keys()))
                    for k in got.keys():
                        if (type(got[k]) == type(got["rms"])):
                            if (k != "blc" and k != "trc"):
                                self.assertTrue((got[k] == exp[k][0][0]).all())
                            
        axes = [0, 1]
        exp = myia.statistics(axes=axes)
        self.assertFalse(exp.has_key("flux"))
        check(myia, axes, exp)
        myia.setbrightnessunit("Jy/beam")
        self.assertTrue(myia.brightnessunit() == "Jy/beam")
        major = qa.quantity("3arcmin")
        minor = qa.quantity("2.5arcmin")
        pa= qa.quantity("20deg")
        myia.setrestoringbeam(
            major=major, minor=minor, pa=pa
        )
        self.assertTrue(myia.brightnessunit() == "Jy/beam")
        exp = myia.statistics(axes=axes)
        self.assertTrue(exp.has_key("flux"))
        check(myia, axes, exp)
        myia.setrestoringbeam(remove=True)
        self.assertTrue(
            myia.setrestoringbeam(
                major=major, minor=minor, pa=pa,
                channel=0, polarization=0
            )
        )
        exp = myia.statistics(axes=axes)
        self.assertTrue(exp.has_key("flux"))
        check(myia, axes, exp)
        nmajor = qa.add(major, qa.quantity("0.1arcmin"))
        self.assertTrue(
            myia.setrestoringbeam(
                major=nmajor, minor=minor, pa=pa,
                channel=1, polarization=1
            )
        )
        exp = myia.statistics(axes=axes)
        self.assertTrue(exp.has_key("flux"))
        self.assertTrue(
            abs(1 - qa.getvalue(nmajor)*exp["flux"][1][1]/(qa.getvalue(major)*exp["flux"][0][0]))
            < 1e-7
        )        
        
    def test_CAS4545(self):
        """verify CAS-4545 fix: full support for >2Gpixel images"""
        myia = self._myia
        myia.fromshape("", [1291, 1290, 1290])
        stats = myia.statistics()
        self.assertTrue(stats['npts'][0] == myia.shape().prod())
        
    def test_kflux(self):
        """Test flux determination for image with units of K - CAS-5779"""
        myia = self._myia
        myia.open(self.datapath + self.kimage)
        res = myia.statistics()
        self.assertTrue(abs(res['flux']/65265.98528085 - 1) < 1e-9)
        
    def test_ranges(self):
        """test specifying ranges works correctly for median, medabsdevmed, and iqr"""
        data = []
        for i in range(1000):
            if i % 2 == 0:
                datum = i
            else:
                datum = -(i*i)
            data.append(datum)
        data = numpy.reshape(data, [10, 10, 10])
        myia = self._myia
        myia.fromarray("", data)
        stats = myia.statistics(robust=True)
        self.assertTrue(stats['median'][0] == -0.5)
        self.assertTrue(stats['medabsdevmed'][0] == 967.5)
        self.assertTrue(stats['quartile'][0] == 251499)
        stats = myia.statistics(robust=True, includepix=[0.1, 1001] )
        self.assertTrue(stats['median'][0] == 500)
        self.assertTrue(stats['medabsdevmed'][0] == 250)
        self.assertTrue(stats['quartile'][0] == 500)
        stats = myia.statistics(robust=True, excludepix=[0.1, 1001] )
        self.assertTrue(stats['median'][0] == -249001)
        self.assertTrue(stats['medabsdevmed'][0] == 216240)
        self.assertTrue(stats['quartile'][0] == 499000)

        myia.done()
        
    def test_hingesfences(self):
        """Test hinges-fences algorithm"""
        data = range(100)
        myia = self._myia
        imagename = "hftest.im"
        myia.fromarray(imagename, data)
        classic = myia.statistics(robust=True, algorithm="cl")
        for i in range(2):
            if i == 0:
                hfall = myia.statistics(robust=True, algorithm="h")
                hf0 = myia.statistics(robust=True, algorithm="h", fence=0)
            else:
                hfall = imstat(imagename=imagename, algorithm="h")
                hf0 = imstat(imagename=imagename, algorithm="h", fence=0)
            for k in classic.keys():
                if type(classic[k]) == numpy.ndarray:
                    self.assertTrue((hfall[k] == classic[k]).all())
                else:
                    self.assertTrue(hfall[k] == classic[k])
            self.assertTrue(hf0['npts'][0] == 51)
            self.assertTrue(hf0['mean'][0] == 49)
            self.assertTrue(hf0['q1'][0] == 36)
    
    def test_fithalf(self):
        """Test fit to half algorithm"""
        data = numpy.array(range(100))
        data = data*data
        myia = self._myia
        imagename = "fhtest.im"
        myia.fromarray(imagename, data)
        myia.done()
        for i in range(2):
            for center in ["mean", "median", "zero"]:
                for lside in [True, False]:
                    if i == 0:
                        myia.open(imagename)
                        res = myia.statistics(
                            robust=True, algorithm="f",
                            center=center, lside=lside
                        )
                        myia.done()
                    else:
                        res = imstat(
                            imagename=imagename, algorithm="f",
                            center=center, lside=lside
                        )
                    if (lside):
                        if (center == "mean"):
                            self.assertTrue(res['npts'][0] == 116)
                            self.assertTrue(res['mean'][0] == 3283.5)
                            self.assertTrue(res['median'][0] == 3283.5)
                            self.assertTrue(res['q1'][0] == 784.0)
                        elif (center == "median"):
                            self.assertTrue(res['npts'][0] == 100)
                            self.assertTrue(res['mean'][0] == 2450.5)
                            self.assertTrue(res['median'][0] == 2450.5)
                            self.assertTrue(res['q1'][0] == 576.0)
                        elif (center == "zero"):
                            self.assertTrue(res['npts'][0] == 2)
                            self.assertTrue(res['mean'][0] == 0)
                            self.assertTrue(res['median'][0] == 0)
                            self.assertTrue(res['q1'][0] == 0)
                    else:
                        if (center == "mean"):
                            self.assertTrue(res['npts'][0] == 84)
                            self.assertTrue(res['mean'][0] == 3283.5)
                            self.assertTrue(res['median'][0] == 3283.5)
                            self.assertTrue(res['q1'][0] == 326.0)
                        elif (center == "median"):
                            self.assertTrue(res['npts'][0] == 100)
                            self.assertTrue(res['mean'][0] == 2450.5)
                            self.assertTrue(res['median'][0] == 2450.5)
                            self.assertTrue(res['q1'][0] == -724.0)
                        elif (center == "zero"):
                            self.assertTrue(res['npts'][0] == 200)
                            self.assertTrue(res['mean'][0] == 0)
                            self.assertTrue(res['median'][0] == 0)
                            self.assertTrue(res['q1'][0] == -2500.0)
    
    def test_chauvenet(self):
        """Test Chauvenet's criterion algorithm"""
        data = [
                -2.61279178e+00,  -2.59342551e+00,  -2.16943479e+00,
                -2.13970494e+00,  -1.91509378e+00,  -1.91133809e+00,
                -1.84780550e+00,  -1.67959487e+00,  -1.55754685e+00,
                -1.49124575e+00,  -1.47779667e+00,  -1.38040781e+00,
                -1.37083769e+00,  -1.34913635e+00,  -1.29416192e+00,
                -1.10022914e+00,  -1.07126451e+00,  -1.05194223e+00,
                -1.03733921e+00,  -1.02524054e+00,  -9.84085381e-01,
                -9.46198046e-01,  -9.23078358e-01,  -9.21401978e-01,
                -8.76483500e-01,  -8.60657215e-01,  -8.26754928e-01,
                -7.59524405e-01,  -7.36167967e-01,  -6.76235080e-01,
                -6.72010839e-01,  -6.33015037e-01,  -5.91541886e-01,
                -5.87743282e-01,  -5.28600693e-01,  -5.03111005e-01,
                -4.84272331e-01,  -3.87220532e-01,  -3.62094551e-01,
                -3.12986404e-01,  -3.01742464e-01,  -2.86407530e-01,
                -2.77583510e-01,  -2.37437248e-01,  -2.37364024e-01,
                -2.35247806e-01,  -2.11185545e-01,  -1.92734912e-01,
                -1.87121660e-01,  -1.77792773e-01,  -1.69995695e-01,
                -1.45033970e-01,  -1.16942599e-01,  -6.27262741e-02,
                -3.45510058e-02,  -3.06752156e-02,  -1.79617219e-02,
                -1.14524942e-02,  -3.16955987e-03,   7.29589257e-04,
                1.24999344e-01,   2.12515876e-01,   2.50957519e-01,
                2.79240131e-01,   2.81288683e-01,   3.05763662e-01,
                3.11809599e-01,   3.40768367e-01,   3.51874888e-01,
                3.91162097e-01,   4.58450705e-01,   4.82642174e-01,
                4.96854514e-01,   7.20111370e-01,   7.22756803e-01,
                7.25001752e-01,   8.35289240e-01,   8.46509099e-01,
                8.93022776e-01,   9.00427580e-01,   9.17734325e-01,
                9.18030262e-01,   1.04210591e+00,   1.05506992e+00,
                1.09472048e+00,   1.15250385e+00,   1.16275501e+00,
                1.21244884e+00,   1.22725236e+00,   1.31463480e+00,
                1.33273876e+00,   1.57637489e+00,   1.58221984e+00,
                1.65665936e+00,   1.80032420e+00,   1.91410339e+00,
                2.02669597e+00,   2.08605909e+00,   2.09777880e+00,
                2.21240473e+00,
                3.5, 4, 5, 6, 7, 8, 1000000
            ]
        myia = self._myia
        imagename = "chauvtest.im"
        myia.fromarray(imagename, data)
        myia.done()
        for i in [0, 1]:
            for zscore in [3.5, -1]:
                for maxiter in [0, 1, -1]:
                    if i == 0:
                        myia.open(imagename)
                        stats = myia.statistics(algorithm="ch", zscore=zscore, maxiter=maxiter)
                        myia.done()
                    else:
                        stats = imstat(imagename=imagename, algorithm="ch", zscore=zscore, maxiter=maxiter)
                    if zscore == 3.5:
                        if maxiter == 0:
                            enpts = 106
                            emax = 8
                        elif maxiter == 1:
                            enpts = 104
                            emax = 6
                        elif maxiter == -1:
                            enpts = 102
                            emax = 4
                    elif zscore == -1:
                        if maxiter == 0:
                            enpts = 106
                            emax = 8
                        elif maxiter == 1:
                            enpts = 103
                            emax = 5
                        elif maxiter == -1:
                            enpts = 100
                            emax = data[99]
                    self.assertTrue(stats['npts'][0] == enpts)
                    self.assertTrue(abs(stats['max'][0] - emax) < 1e-6)
    
    def test_CAS7472(self):
        """Verify stats of sub regions of temp images produce correct results when using originial (Kileen pointer) method"""
        myia = iatool()
        myia.fromshape("", [100,100])
        myia.addnoise()
        bb = myia.getchunk()
        bb[50,50] = 200
        bb[51,51] = -200
        myia.putchunk(bb)
        reg = rg.box([25,25],[75,75])
        resnew = myia.statistics(region=reg, clmethod="framework")
        resold = myia.statistics(region=reg, clmethod="tiled")
        myia.done()
        self.assertTrue(resnew['max'][0] == 200)
        self.assertTrue(resnew['min'][0] == -200)
        self.assertTrue(resold['max'][0] == 200)
        self.assertTrue(resold['min'][0] == -200)
        self._compare(resold, resnew, "first")
    
        myia.fromshape("", [100,100, 100])
        myia.addnoise()
        reg = rg.box([25,25,25],[75,75,25])
        for axes in [[], [0], [1], [2], [0,1], [0,2], [1,2], [0,1,2]]:
            resnew = myia.statistics(axes=axes, region=reg, clmethod="framework")
            resold = myia.statistics(axes=axes, region=reg, clmethod="tiled")
            self._compare(resold, resnew, "axes=" + str(axes))
        myia.done()
        
    def test_CAS7697(self):
        """verify fix to CAS-7697, min/max should be zero in masked plane"""
        myia = iatool()
        imagename = "CAS7697.im"
        myia.fromshape(imagename, [100,100, 10])
        myia.addnoise()
        bb = myia.getchunk()
        bb[:,:,9] = 40
        myia.putchunk(bb)
        myia.calcmask(imagename + "<30")
        for clmethod in ["framework", "tiled"]:
            stats = myia.statistics(axes=[0,1], clmethod=clmethod)
            self.assertTrue(stats['max'][9] == 0)
            self.assertTrue(stats['min'][9] == 0)
        myia.done()
def suite():
    return [imstat_test]
