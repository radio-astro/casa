import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import math
import numpy

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
        myia.fromshape("", [1290, 1290, 1290])
        stats = myia.statistics()
        self.assertTrue(stats['npts'][0] == myia.shape().prod())
        
    def test_kflux(self):
        """Test flux determination for image with units of K - CAS-5779"""
        myia = self._myia
        myia.open(self.datapath + self.kimage)
        res = myia.statistics()
        self.assertTrue(abs(res['flux']/65265.98528085 - 1) < 1e-9)
        
 
def suite():
    return [imstat_test]


