##########################################################################
# imfit_test.py
#
# Copyright (C) 2008, 2009
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Dave Mehringer
# </author>
#
# <summary>
# Test suite for the CASA imfit Task
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="imfit.py:description">imfit</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# imfit_test stands for imfit test
# </etymology>
#
# <synopsis>
# imfit_test.py is a Python script that tests the correctness
# of the ia.fitcomponents tool method and the imfit task in CASA.
# </synopsis> 
#
# <example>
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_imfit[test1,test2,...]
# </example>
#
# <motivation>
# To provide a test standard to the imfit task to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import os
import casac
from tasks import *
from taskinit import *
import hashlib
import shutil
from __main__ import *
import unittest


noisy_image = "gaussian_model_with_noise.im"
noisy_image_xx = "gaussian_model_with_noise_xx.im"
expected_model = "gaussian_model_with_noise_model.fits"
expected_residual = "gaussian_model_with_noise_resid.fits"
convolved_model = "gaussian_convolved.fits"
estimates_convolved = "estimates_convolved.txt"
two_gaussians_image = "two_gaussian_model.fits"
stokes_image = "imfit_stokes.fits"
two_gaussians_estimates = "estimates_2gauss.txt"
expected_new_estimates = "expected_new_estimates.txt"
gauss_no_pol = "gauss_no_pol.fits"
jyperbeamkms = "jyperbeamkmpersec.fits";
masked_image = 'myout.im'
multiplane_image = "gauss_multiplane.fits"
multibeam_image = "gauss_multibeam.im"
two_gauss_multiplane_estimates="estimates_2gauss_multiplane.txt"
msgs = ''
twogim = "2g.im"
twogest = "2g_estimates.txt"
circular = "circular_gaussian.im"
kimage = "bunitk.im"
decon_im = "decon_test.im"

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imfit/'

# are the two specified numeric values relatively close to each other? 
def near (first, second, epsilon):
    if first == 0 and second == 0:
        return True
    denom = first
    if first == 0:
        denom = second
    return (abs((first - second)/denom) <= abs(epsilon))

def near_abs(first, second, epsilon):
    return abs(first - second) <= epsilon
# Do the got and expected images match?
# @param got The name of the test image
# @param expected The name of the expected image
# @param difference The name of the difference image to write
def check_image(got, expected):
    myia = iatool()
    if not myia.open(got):
        casalog.post("Cannot find image " + got, 'SEVERE')
        return False
    gotunit = myia.brightnessunit()
    gotpix = myia.getchunk()
    myia.open(expected)
    expunit = myia.brightnessunit()
    if gotunit != expunit:
        casalog.post(
            "Units differ: got '" + gotunit
            + "' expected '" + expunit + "'",
            'SEVERE'
        )
        myia.done()
        return False
    exppix = myia.getchunk()
    myia.done()
    return (gotpix - exppix == 0).all()

# count the number of lines in the specified file in which the spcified string occurs
def count_matches(filename, match_string):
    count = 0
    for line in open(filename):
        if (match_string in line):
            count += 1
    return count

class imfit_test(unittest.TestCase):
    
    def setUp(self):
        
        for f in [
            noisy_image, noisy_image_xx, expected_model, expected_residual, convolved_model,
            estimates_convolved, two_gaussians_image, two_gaussians_estimates,
            expected_new_estimates, stokes_image, gauss_no_pol, jyperbeamkms,
            masked_image, multiplane_image, multibeam_image, two_gauss_multiplane_estimates,
            twogim, twogest
        ] :
            if not os.path.exists(f):
                if (os.path.isdir(datapath + f)):
                    shutil.copytree(datapath + f, f)
                if (os.path.isfile(datapath + f)):
                    shutil.copy(datapath + f, f)

    def tearDown(self):
        for f in [
            # removing this image with rmtree() etc fails on mac
            # noisy_image,
            noisy_image_xx, expected_model, expected_residual, convolved_model,
            estimates_convolved, two_gaussians_estimates,
            expected_new_estimates, stokes_image, gauss_no_pol, jyperbeamkms,
            masked_image, multiplane_image, multibeam_image, two_gauss_multiplane_estimates
        ] :
            if (os.path.isdir(f)):
                os.system("rm -rf " + f)
                #shutil.rmtree(f)
            if (os.path.isfile(f)):
                os.remove(f)
        self.assertTrue(len(tb.showcache()) == 0)

    def test_fit_using_full_image(self):
        '''Imfit: Fit using full image'''
        test = "fit_using_full_image: "
        def run_fitcomponents():
            myia = iatool()
            myia.open(noisy_image)
            res = myia.fitcomponents()
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=noisy_image)
    
        for i in [0 ,1]:
            if (i == 0):
                code = run_fitcomponents
                method = test + "ia.fitcomponents: "
            else:
                code = run_imfit
                method += test + "imfit: "
            self._check_results(code())
            
    def _check_results(self, res):
            success = True
            global msgs
            clist = res['results']
            if (not res['converged'][0]):
                success = False
                msgs += method + "fit did not converge unexpectedly"
            epsilon = 1e-5
            # I flux test
            got = clist['component0']['flux']['value'][0]
            expected = 60291.7956
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "I flux density test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Q flux test
            got = clist['component0']['flux']['value'][1]
            expected = 0
            if (got != expected):
                success = False
                msgs += method + "Q flux density test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # RA test
            got = clist['component0']['shape']['direction']['m0']['value']
            expected = 0.00021339
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "RA test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Dec test
            got = clist['component0']['shape']['direction']['m1']['value']
            expected = 1.935825e-5 
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "Dec test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Major axis test
            got = clist['component0']['shape']['majoraxis']['value']
            expected = 23.530022 
            epsilon = 1e-6
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Major axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Minor axis test
            got = clist['component0']['shape']['minoraxis']['value']
            expected = 18.862125  
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Minor axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Position angle test
            got = clist['component0']['shape']['positionangle']['value']
            expected = 119.88185
            epsilon = 1e-5 
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "Position angle test failure, got " + str(got) + " expected " + str(expected) + "\n"

            self.assertTrue(success,msgs)
        
    
    def test_fit_using_box(self):
        '''Imfit: Fit using box'''
        for i in range(3):
            test = 'fit_using_box, loop #' + str(i) + ': '
            # the regions and box that should be used all define the same region
            # so that the results are the same for each loop (which makes the
            # code more compact)
            # i = 0: use box= keyword
            # i = 1: use region record
            # i = 2: use named region (ie region record saved in image)
            if (i == 0):
                box = "130,89,170,129"
                region = ""
            elif (i == 1):
                box = ''
                region = rg.box([130,89,0,0],[170,129,0,0])
            elif (i == 2):
                box = ''
                region = 'mybox'
    
            def run_fitcomponents():
                myia = iatool()
                myia.open(noisy_image)
                res = myia.fitcomponents(box=box, region=region)
                myia.close()
                return res
            def run_imfit():
                default('imfit')
                return imfit(imagename=noisy_image, box=box, region=region)
            for code in [run_fitcomponents, run_imfit]:
                self._check_box_results(code())
                
    def _check_box_results(self, res):
        epsilon = 1e-5
        clist = res['results']
        self.assertTrue(
            res['converged'][0],
            "fit did not converge unexpectedly"
        )
        # I flux test
        got = clist['component0']['flux']['value'][0]
        expected = 60319.8604
        self.assertTrue(
            near(got, expected, epsilon),
            "I flux density test failure, got " + str(got)
            + " expected " + str(expected)
        )
        # Q flux test
        got = clist['component0']['flux']['value'][1]
        expected = 0
        self.assertTrue(
            got == expected,
            "Q flux density test failure, got " + str(got)
            + " expected " + str(expected)
        )
        # RA test
        got = clist['component0']['shape']['direction']['m0']['value']
        expected = 0.00021337
        self.assertTrue(
            near_abs(got, expected, epsilon),
            "RA test failure, got " + str(got) + " expected " + str(expected)
        )
        # Dec test
        got = clist['component0']['shape']['direction']['m1']['value']
        expected = 1.935906e-05
        self.assertTrue(
            near_abs(got, expected, epsilon),
            "Dec test failure, got " + str(got) + " expected " + str(expected)
        )
        # Major axis test
        got = clist['component0']['shape']['majoraxis']['value']
        expected = 23.545212
        epsilon = 1e-6
        self.assertTrue(
            near(got, expected, epsilon),
            "Major axis test failure, got " + str(got) + " expected " + str(expected)
        )
        got = clist['component0']['shape']['majoraxiserror']['value']
        expected = 0.01776
        epsilon = 1e-3
        self.assertTrue(
            near(got, expected, epsilon),
            "Major axis error test failure, got " + str(got) + " expected " + str(expected)
        )
        self.assertTrue(
            clist['component0']['shape']['majoraxis']['unit']
            == clist['component0']['shape']['majoraxiserror']['unit'],
            "Major axis and major axis error units are different\n"
        )
        # Minor axis test
        got = clist['component0']['shape']['minoraxis']['value']
        expected = 18.86450
        epsilon = 1e-6
        self.assertTrue(
            near(got, expected, epsilon),
            "Minor axis test failure, got " + str(got) + " expected " + str(expected)
        )
        got = clist['component0']['shape']['minoraxiserror']['value']
        expected = 0.01423
        epsilon = 1e-3
        self.assertTrue(
            near(got, expected, epsilon),
            "Minor axis error test failure, got " + str(got) + " expected " + str(expected)
        )
        self.assertTrue(
            clist['component0']['shape']['minoraxis']['unit']
            == clist['component0']['shape']['minoraxiserror']['unit'],
            "Minor axis and minor axis error units are different"
        )
        # Position angle test
        got = clist['component0']['shape']['positionangle']['value']
        expected = 119.81296
        epsilon = 1e-5
        self.assertTrue(
            near_abs(got, expected, epsilon),
            "Position angle test failure, got " + str(got)
            + " expected " + str(expected)
        )
        got = clist['component0']['shape']['positionangleerror']['value']
        expected = 0.1367
        epsilon = 1e-3
        self.assertTrue(
            near(got, expected, epsilon),
            "Position angle error test failure, got " + str(got) + " expected " + str(expected)
        )
        self.assertTrue(
            clist['component0']['shape']['positionangle']['unit']
            == clist['component0']['shape']['positionangleerror']['unit'],
            "Position angle and position angle error units are different"
        )

    def test_nonconvergence(self):
        '''Imfit: Test non-convergence'''
        test = "test_nonconvergence: "
        success = True
        global msgs
    
        box = '0,0,20,20'
        def run_fitcomponents():
            myia = iatool()
            myia.open(noisy_image)
            res = myia.fitcomponents(box=box)
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=noisy_image, box=box)
    
        for code in [run_fitcomponents, run_imfit]:
            res = code()
            if (res['converged'][0]):
                success = False
                msgs += method + "fit unexpectedly converged\n"
    
        self.assertTrue(success,msgs)
    
    def test_fit_using_range(self):
        '''Imfit: Fit using range'''
        success = True
        global msgs
        for i in range(4):
            test = 'fit_using_range, loop #' + str(i) + ': '
            # the ranges and mask defined all define the same pixels to be used
            # so that the results are the same for each loop (which makes the
            # code more compact)
            # i = 0: use mask keyword
            # i = 1: use includepix keyword
            # i = 2: use excludepix keyword
            # i = 3 use masked image
            if (i == 0):
                mask = noisy_image + ">40"
                includepix = []
                excludepix = []
                pixelmask = ""
            elif (i == 1):
                mask = ''
                includepix = [40,400]
                excludepix = []
                pixelmask = ""
            elif (i == 2):
                mask = ''
                includepix = []
                excludepix = [-10,40]
                pixelmask = ""
            elif (i == 3):
                mask = ''
                includepix = []
                excludepix = []
                pixelmask = "mymask"
    
            def run_fitcomponents():
                myia = iatool()
                myia.open(masked_image)
                myia.maskhandler("set", pixelmask)
                res = myia.fitcomponents(mask=mask, includepix=includepix, excludepix=excludepix)
                myia.close()
                return res
            def run_imfit():
                default('imfit')
                return imfit(imagename=masked_image, mask=mask, includepix=includepix, excludepix=excludepix)
    
            for code in [run_fitcomponents, run_imfit]:
                res = code()
                clist = res['results']
                if (not res['converged'][0]):
                    success = False
                    msgs += method + "fit did not converge unexpectedly"
                epsilon = 1e-5
                # I flux test
                got = clist['component0']['flux']['value'][0]
                expected = 60354.3232
                if (not near(got, expected, epsilon)):
                    success = False
                    msgs += test + "I flux density test failure, got " + str(got) \
                        + " expected " + str(expected) + "\n"
                # Q flux test
                got = clist['component0']['flux']['value'][1]
                expected = 0
                if (got != expected):
                    success = False
                    msgs += test + "Q flux density test failure, got " + str(got) \
                        + " expected " + str(expected) + "\n"
                # RA test
                got = clist['component0']['shape']['direction']['m0']['value']
                expected = 0.000213391
                if (not near(got, expected, epsilon)):
                    success = False
                    msgs += test + "RA test failure, got " + str(got) + " expected " + str(expected) + "\n"
                # Dec test
                got = clist['component0']['shape']['direction']['m1']['value']
                expected = 1.93449e-05
                if (not near(got, expected, epsilon)):
                    success = False
                    msgs += test + "Dec test failure, got " + str(got) + " expected " + str(expected) + "\n"
                # Major axis test
                got = clist['component0']['shape']['majoraxis']['value']
                expected = 23.541712
                epsilon = 1e-7
                if (not near(got, expected, epsilon)):
                    success = False
                    msgs += test + "Major axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
                # Minor axis test
                got = clist['component0']['shape']['minoraxis']['value']
                expected = 18.882029
                if (not near(got, expected, epsilon)):
                    success = False
                    msgs += test + "Minor axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
                # Position angle test
                got = clist['component0']['shape']['positionangle']['value']
                expected = 119.769648
                if (not near(got, expected, epsilon)):
                    success = False
                    msgs += test + "Position angle test failure, got " + str(got) + \
                        " expected " + str(expected) + "\n"
    
        self.assertTrue(success,msgs)
    
    
    # Test writing of residual and model images 
    def test_residual_and_model(self):
        '''Imfit: Test residual and model'''        
        box="100,100,200,200"
        def run_fitcomponents(model, residual):
            myia = iatool()
            myia.open(noisy_image)
            res = myia.fitcomponents(
                box=box, residual=residual, model=model
            )
            myia.done()
            return res
        def run_imfit(model, residual):
            default('imfit')
            return imfit(
                imagename=noisy_image, box=box, residual=residual,
                model=model
            )
        for code in [run_fitcomponents, run_imfit]:
            model = 'model_' + str(code) + '.im'
            residual = 'resid_' + str(code) + '.im'

            res = code(model, residual)
            clist = res['results']
    
            self.assertTrue(res['converged'][0])
            self.assertTrue(check_image(residual, expected_residual))
            self.assertTrue(check_image(model, expected_model))
                
                    
    # Test using initial estimates and fixed parameters
    def test_fit_using_estimates(self):
        '''Imfit: Test using estimates'''
        test = 'fit_using_estimates: '
        global msgs
        box = "121,84,178,135"
        def run_fitcomponents():
            myia = iatool()
            myia.open(convolved_model)
            res = myia.fitcomponents(estimates=estimates_convolved, box=box)
            print "** image " + convolved_model
            print "*** estimates " + estimates_convolved
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(
                imagename=convolved_model, box=box,
                estimates=estimates_convolved
            )
        for code in [run_fitcomponents, run_imfit]:
            res = code()
    
            clist = res['results']
            self.assertTrue(
                res['converged'][0],
                test + "fit did not converge unexpectedly"
            )
            epsilon = 1e-5
            # I flux test
            got = clist['component0']['flux']['value'][0]
            expected = 60300
            self.assertTrue(
                near(got, expected, 0.01),
                test + "I flux density test failure, got " + str(got) + " expected " + str(expected)
            )                                                                  
            # Q flux test
            got = clist['component0']['flux']['value'][1]
            expected = 0
            self.assertTrue(
                got == expected,
                test + "Q flux density test failure, got " + str(got) + " expected " + str(expected)
            )
            # RA test
            shape = clist['component0']['shape']
            got = shape['direction']['m0']['value']
            expected =  0.00021329
            self.assertTrue(near(got, expected, 0.01),
                test + "RA test failure, got " + str(got) + " expected " + str(expected)
            )
            # Dec test
            got = shape['direction']['m1']['value']
            expected = 1.93925e-5
            self.assertTrue(
                near(got, expected, 0.01),
                test + "Dec test failure, got " + str(got) + " expected " + str(expected)
            )
            # Major axis test
            got = shape['majoraxis']['value']
            expected =  28.2615
            epsilon = 0.01
            self.assertTrue(
                near(got, expected, epsilon),
                test+ "Major axis test failure, got " + str(got) + " expected " + str(expected)
            )
            # Minor axis test
            got = shape['minoraxis']['value']
            expected = 25.55
            self.assertTrue(
                near(got, expected, epsilon),
                test + "Minor axis test failure, got " + str(got) + " expected " + str(expected)
            )
            # Position angle test
            got = shape['positionangle']['value']
            expected = 126.868
            self.assertTrue(
                near(got, expected, epsilon),
                test + "Position angle test failure, got " + str(got) + " expected " + str(expected)
            )

    def test_position_errors(self):
        '''Imfit: Test position errors'''
        success = True
        test = 'test_position_errors: '
        box = "122, 85, 177, 138"
        def run_fitcomponents():
            myia = iatool()
            myia.open(convolved_model)
            res = myia.fitcomponents(box=box)
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=convolved_model, box=box)
    
        for code in [run_fitcomponents, run_imfit]:
            res = code()
            clist = res['results']
            error = clist['component0']['shape']['direction']['error']

            self.assertTrue(res['converged'][0])
            got = error['latitude']['value']
            print "*** got ", got
            self.assertTrue(abs(got) < 1e-6)
    
            got = error['longitude']['value']
            self.assertTrue(abs(got) < 1e-6)

        self.assertTrue(success,msgs)
    
    # test writing, appending, and overwriting log files
    def test_logfile(self):
        '''Imfit: Test logfile'''
        for i in [0, 1]:
            logfile = os.getcwd() + "/imfit.log" + str(i)
            box = "21, 159, 93, 237, 126, 89, 169, 131"
            if (i == 0):
                def run_fitcomponents(append=None):
                    myia = iatool()
                    myia.open(two_gaussians_image)
                    if (append == None):
                        res = myia.fitcomponents(box=box, estimates=two_gaussians_estimates, logfile=logfile)
                    else:
                        res = myia.fitcomponents(box=box, estimates=two_gaussians_estimates, logfile=logfile, append=append)
                    myia.done()
                    return res
                
                code = run_fitcomponents
            else:
                def run_imfit(append=None):
                    default('imfit')
                    if (append == None):
                        return imfit(imagename=two_gaussians_image, box=box, estimates=two_gaussians_estimates, logfile=logfile)
                    else:
                        return imfit(
                            imagename=two_gaussians_image, box=box, estimates=two_gaussians_estimates,
                            logfile=logfile, append=append
                        )
                code = run_imfit
            res = code()
            self.assertTrue(os.path.exists(logfile), "logfile was not written")       
       
            self.assertTrue(
                count_matches(logfile, "****** Fit performed") == 1,
                "unexpected logfile"
            )
            #default, append
            res = code()
            self.assertTrue(
                os.path.exists(logfile), "logfile was not written"
            )
            self.assertTrue(
                count_matches(logfile, "****** Fit performed") == 2,
                "logfile not appended"
            )
            # explicit append
            res = code(True)
            self.assertTrue(
                os.path.exists(logfile), "logfile was not written"
            )
            self.assertTrue(
                count_matches(logfile, "****** Fit performed") == 3,
                "logfile not appended"
            )
            # overwrite
            res = code(False)
            self.assertTrue(
                os.path.exists(logfile), "logfile was not written"
            )
            self.assertTrue(
                count_matches(logfile, "****** Fit performed") == 1,
                "logfile not overwritten"
            )
    
    # Test writing of a new estimates file
    def test_newestimates(self):
        '''Imfit: Test new estimates'''
        success = True
        test = 'test_newestimates: '
        global msgs
        box = "20, 157, 93, 233, 119, 87, 178, 133"
        for i in [0, 1]:
            newestimates = "newestimates" + str(i) + ".txt"
            if (i == 0):
                def run_fitcomponents():
                    myia = iatool()
                    myia.open(two_gaussians_image)
                    res = myia.fitcomponents(box=box, estimates=two_gaussians_estimates, newestimates=newestimates)
                    return res
                code = run_fitcomponents
                method = test + "ia.fitcomponents: "
            else:
                def run_imfit():
                    default('imfit')
                    return imfit(
                        imagename=two_gaussians_image, box=box, estimates=two_gaussians_estimates,
                        newestimates=newestimates
                    )
                code = run_imfit
                method = test + "imfit: "
            res = code()
    
            if (not os.path.exists(newestimates)):
                success = False
                msgs += method + "new estimates file was not written\n"
                return {'success' : success, 'error_msgs' : msgs}        
     
            expected_sha = hashlib.sha512(open(expected_new_estimates, 'r').read()).hexdigest()
    
            got_sha = hashlib.sha512(open(newestimates, 'r').read()).hexdigest()
            if (got_sha != expected_sha):
                success = False
                msgs += method + "new estimates file differs from expected\n"
        
        self.assertTrue(success,msgs)
        
    ## Test imfit on various polarization planes
    def test_polarization_image(self):
        '''Imfit: Test polarization image'''
        success = True
        test = 'test_polarization_image: '
        global msgs
        def run_fitcomponents(stokes):
            myia = iatool()
            myia.open(stokes_image)
            res = myia.fitcomponents(stokes=stokes)
            return res
        def run_imfit(stokes):
            default('imfit')
            return imfit(imagename=stokes_image, stokes=stokes)
    
        stokes = ['I','Q','U','V']
        expectedFlux = [133.60641, 400.81921, 375.76801, -1157.92212]
        expectedRA = [1.2479113396, 1.2479113694, 1.2478908580, 1.2478908284]
        expectedDec = [0.782579122, 0.782593666, 0.782593687, 0.782579143]
        expectedMaj = [7.992524398, 11.988806751, 8.991589959, 12.987878913]
        expectedMin = [5.994405977, 5.994395540, 4.995338093, 7.992524265]
        expectedPA = [40.083248, 160.083213, 50.082442, 135.08243]
    
        for i in [0 ,1]:
            for j in range(len(stokes)):
                if (i == 0):
                    code = run_fitcomponents
                    method = test + "ia.fitcomponents: "
                else:
                    code = run_imfit
                    method += test + "imfit: "
                res = code(stokes[j])
    
                clist = res['results']
                if (not res['converged'][0]):
                    success = False
                    msgs += method + "fit did not converge unexpectedly for stokes " + stokes[j]
                got = clist['component0']['flux']['value'][j]
    
                # flux density test
                if (not near(got, expectedFlux[j], 1e-5)):
                    success = False
                    msgs += method + " " + str(stokes) + " flux density test failure, got " + str(got) + " expected " + str(expectedFlux[j]) + "\n"
    
                # RA test
                got = clist['component0']['shape']['direction']['m0']['value']
                if (not near_abs(got, expectedRA[j], 1e-8)):
                    success = False
                    msgs += method + "stokes " + stokes[j] + " RA test failure, got " + str(got) + " expected " + str(expectedRA[j]) + "\n"
    
                # Dec test
                got = clist['component0']['shape']['direction']['m1']['value']
                if (not near_abs(got, expectedDec[j], 1e-8)):
                    success = False
                    msgs += method + "stokes " + stokes[j] + " Dec test failure, got " + str(got) + " expected " + str(expectedDec[j]) + "\n"
    
                # Major axis test
                got = clist['component0']['shape']['majoraxis']['value']
                if (not near(got, expectedMaj[j], 1e-7)):
                    success = False
                    msgs += method + "stokes " + stokes[j] + " Major axis test failure, got " + str(got) + " expected " + str(expectedMaj[j]) + "\n"
            
                # Minor axis test
                got = clist['component0']['shape']['minoraxis']['value']
                if (not near(got, expectedMin[j], 1e-7)):
                    success = False
                    msgs += method + "stokes " + stokes[j] + " Minor axis test failure, got " + str(got) + " expected " + str(expectedMin[j]) + "\n"
    
                # Position angle test
                got = clist['component0']['shape']['positionangle']['value']
                if (not near_abs(got, expectedPA[j], 1e-5)):
                    success = False
                    msgs += method + "stokes " + stokes[j] + " Position angle test failure, got " + str(got) + " expected " + str(expectedPA[j]) + "\n"
        
        self.assertTrue(success,msgs)         
    
    def test_CAS_2318(self):
        "Verification of CAS-2318 fix"
        success = True
        test = 'test_CAS_2318: '
        global msgs
        def run_fitcomponents():
            myia = iatool()
            myia.open(gauss_no_pol)
            res = myia.fitcomponents()
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=gauss_no_pol)
        
        for code in [run_fitcomponents, run_imfit]:

            # Just the fact that an exception isn't thrown verifies the fix
            clist = code()['results']
            got = clist['component0']['flux']['value'][0]
            expected = 394312.65593496
            self.assertTrue(near(got, expected, 1e-5))

    def test_CAS_1233(self):
        box = "124, 88, 173, 134"
        def run_fitcomponents():
            myia = iatool()
            myia.open(jyperbeamkms)
            res = myia.fitcomponents(box=box)
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=jyperbeamkms, box=box)
    
        for i in [0 ,1]:
            if (i == 0):
                code = run_fitcomponents
            else:
                code = run_imfit
            res = code()
            clist = res['results']
            self.assertTrue(
                res['converged'][0],
                "fit did not converge unexpectedly"
            )
            epsilon = 1e-5
            # I flux test
            self.assertTrue(clist['component0']['flux']['unit'] == 'Jy.km/s')

            got = clist['component0']['flux']['value'][0]
            expected = 60318.6
            self.assertTrue(
                near(got, expected, epsilon),
                "I flux density test failure, got " + str(got) + " expected " + str(expected)
            )

            # RA test
            got = clist['component0']['shape']['direction']['m0']['value']
            expected = 0.000213318
            self.assertTrue(
                near_abs(got, expected, epsilon),
                "RA test failure, got " + str(got) + " expected " + str(expected)
            )
            # Dec test
            got = clist['component0']['shape']['direction']['m1']['value']
            expected = 1.939254e-5
            self.assertTrue(
                near_abs(got, expected, epsilon),
                "Dec test failure, got " + str(got) + " expected " + str(expected)
            )
            # Major axis test
            got = clist['component0']['shape']['majoraxis']['value']
            expected =  26.50461508
            epsilon = 1e-6
            self.assertTrue(
                near(got, expected, epsilon),
                "Major axis test failure, got " + str(got) + " expected " + str(expected)
            )
            # Minor axis test
            got = clist['component0']['shape']['minoraxis']['value']
            expected = 23.99821851
            self.assertTrue(
                near(got, expected, epsilon),
                "Minor axis test failure, got " + str(got) + " expected " + str(expected)
            )
            # Position angle test
            got = clist['component0']['shape']['positionangle']['value']
            expected = 126.3211060
            epsilon = 1e-5
            self.assertTrue(
                near_abs(got, expected, epsilon),
                "Position angle test failure, got " + str(got) + " expected " + str(expected)
            )

    def test_CAS_2633(self):
        """bug fix: imfit always returns 1000 MHz as image frequency"""
        method = "test_CAS_2633"
        test = "test_CAS_2633"
        box = "120, 90, 175, 130"
        def run_fitcomponents():
            myia = iatool()
            myia.open(jyperbeamkms)
            res = myia.fitcomponents(box=box)
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=jyperbeamkms, box=box)
    
        for i in [0 ,1]:
            if (i == 0):
                code = run_fitcomponents
                method = test + "ia.fitcomponents: "
            else:
                code = run_imfit
                method += test + "imfit: "
            res = code()
            clist = res['results']
            self.assertTrue(res['converged'][0])
            epsilon = 1e-7
            # ref freq test
            got = clist['component0']['spectrum']['frequency']['m0']['value']
            expected = 1.415
            self.assertTrue(near(got, expected, epsilon))

    def test_CAS_2595(self):
        """ Test CAS-2595 feature addition: write component list table"""
        method = "test_CAS_2595"
        test = "test_CAS_2595"
        mycl = cltool()
        complist = "mycomplist_CAS-2595.tbl"
        def run_fitcomponents(imagename, estimates, overwrite, box=""):
            myia = iatool()
            myia.open(imagename)
            res = myia.fitcomponents(
                complist=complist, estimates=estimates,
                box=box, overwrite=overwrite
            )
            myia.done()
            return res
        def run_imfit(imagename, estimates, overwrite, box=""):
            default('imfit')
            return imfit(
                imagename=imagename, estimates=estimates, box=box,
                complist=complist, overwrite=overwrite
            )
        for code in (run_fitcomponents, run_imfit):
            res = code(noisy_image, "", False, "130,92,169,130")
            mycl.open(complist)
            self.assertTrue(
                mycl.length() == 1,
                str(code) + " didn't get 1 component as expected from table"
            )
            mycl.done()
            # don't overwrite existing comp list
            res = code(
                two_gaussians_image, two_gaussians_estimates, False,
                "31, 172, 89, 232, 128, 91, 171, 134"
            )
            mycl.open(complist)
            self.assertTrue(
                mycl.length() == 1,
                str(code) + " didn't get 1 component as expected from table"
            )
            mycl.done()
            # now overwrite existing comp list
            res = code(
                two_gaussians_image, two_gaussians_estimates, True,
                "31, 172, 89, 232, 128, 91, 171, 134"
            )
            mycl.open(complist)
            self.assertTrue(
                mycl.length() == 2,
                str(code) + " didn't get 2 component as expected from table"
            )
            
            mycl.done()
            shutil.rmtree(complist)
 
    def test_CAS_2999(self):
        """Test multiplane fitting"""
        
        method = "test_CAS_2999"
        test = "test_CAS_2999"
        imagename = multiplane_image
        complist = "mycomplist_CAS-2999.tbl"
        estimates = two_gauss_multiplane_estimates
        chans = "0~3"
        resid = "residualImage_multi"
        model = "modelImage_multi"
        mask = "gauss_multiplane.fits<15";
        def run_fitcomponents():
            myia = iatool()
            myia.open(imagename)
            res = myia.fitcomponents(
                chans=chans, mask=mask, complist=complist,
                estimates=estimates, overwrite=True,
                model=model, residual=resid
            )
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(
                imagename=imagename, chans=chans,
                mask=mask, complist=complist, estimates=estimates,
                overwrite=True, model=model, residual=resid
            )
        mycl = cltool()
        for code in (run_fitcomponents, run_imfit):
            res = code()
            mycl.open(complist)
            self.assertTrue(
                mycl.length() == 8,
                str(code) + " didn't get 8 component as expected from table"
            )
            mycl.done()
            self.assertTrue(
                res['converged'].size == 4,
                "Size of converged array is not 4"
            )
            self.assertTrue(
                all(res['converged']),
                "One or more of the converged elements are False"
            )

    def test_zero_level(self):
        """Test zero level fitting"""
        
        mycl = cltool()
        myia = iatool()

        def run_fitcomponents(imagename):
            myia = iatool()
            myia.open(imagename)
            res = myia.fitcomponents(
                box="130,89,170,129", dooff=True, offset=0.0
            )
            myia.done()
            return res
        def run_imfit(imagename):
            default('imfit')
            return imfit(
                imagename=imagename, 
                box="130,89,170,129", dooff=True, offset=0.0
            )
        
        j = 0
        for code in (run_fitcomponents, run_imfit):
            for i in range(3):
                if i == 0:
                    off = -10
                if i == 1:
                    off = 0
                if i == 2:
                    off = 5
                myia.open(noisy_image)
                myshape = myia.shape()
                csys = myia.coordsys().torecord()
                myia.done()
                myia.fromshape(
                    "tmp" + str(i) + "_" + str(j) + ".im",
                    myshape, csys
                )
                myia.calc(noisy_image + "+" + str(off))
                imagename = "xx" + str(i) + "_" + str(j) + ".im"
                subim = myia.subimage(imagename)
                subim.done()
                myia.done()
                            
                res = code(imagename)
                mycl.fromrecord(res["results"])
                got = mycl.getfluxvalue(0)[0]
                expected = 60498.5586
                epsilon = 1e-5
                self.assertTrue(near(got, expected, epsilon))
                got = mycl.getfluxvalue(0)[1]
                self.assertTrue(got == 0)
                got = mycl.getrefdir(0)["m0"]["value"]
                expected = 0.000213372126
                epsilon = 1e-5
                self.assertTrue(near(got, expected, epsilon))
                got = mycl.getrefdir(0)["m1"]["value"]
                expected = 1.93581236e-05
                epsilon = 1e-5
                self.assertTrue(near(got, expected, epsilon))
                shape = mycl.getshape(0)
                got = shape["majoraxis"]["value"]
                expected = 23.5743464
                epsilon = 1e-5
                self.assertTrue(near(got, expected, epsilon))
                got = shape["minoraxis"]["value"]
                expected = 18.8905131
                epsilon = 1e-5
                self.assertTrue(near(got, expected, epsilon))
                got = shape["positionangle"]["value"]
                expected = 119.818744
                epsilon = 1e-5
                self.assertTrue(near(got, expected, epsilon))
                mycl.done()
                
                got = res["zerooff"]["value"][0]
                expected = off - 0.102277
                self.assertTrue(near(got, expected, epsilon))
                self.assertTrue(res['zerooff']['unit'] == "Jy/pixel")

                mycl.done()
                
                j = j + 1

    def test_fix_zero_level(self):
        """Test fixing zero level offset"""
        
        method = "test_fix_zero_level"
        test = method
        mycl = cltool()
        myia = iatool()
        offset = -0.102277
        imagename = noisy_image

        def run_fitcomponents(imagename):
            myia = iatool()
            myia.open(imagename)
            res = myia.fitcomponents(
                box="130,89,170,129", dooff=True,
                offset=offset, fixoffset=True
            )
            myia.done()
            return res
        j = 0
        def run_imfit(imagename):
            default('imfit')
            return imfit(
                imagename=imagename, 
                box="130,89,170,129", dooff=True,
                offset=offset, fixoffset=True
            )
        for code in (run_fitcomponents, run_imfit):                    
            res = code(imagename)
            mycl.fromrecord(res["results"])
            got = mycl.getfluxvalue(0)[0]
            expected = 60498.5586
            epsilon = 1e-5
            print "***got " + str(got)
            self.assertTrue(near(got, expected, epsilon))
            got = mycl.getfluxvalue(0)[1]
            self.assertTrue(got == 0)
            got = mycl.getrefdir(0)["m0"]["value"]
            expected = 0.000213372126
            epsilon = 1e-5
            self.assertTrue(near(got, expected, epsilon))
            got = mycl.getrefdir(0)["m1"]["value"]
            expected = 1.93581236e-05
            epsilon = 1e-5
            self.assertTrue(near(got, expected, epsilon))
            shape = mycl.getshape(0)
            got = shape["majoraxis"]["value"]
            expected = 23.5743464
            epsilon = 1e-5
            self.assertTrue(near(got, expected, epsilon))
            got = shape["minoraxis"]["value"]
            expected = 18.8905131
            epsilon = 1e-5
            self.assertTrue(near(got, expected, epsilon))
            got = shape["positionangle"]["value"]
            expected = 119.818744
            epsilon = 1e-5
            self.assertTrue(near(got, expected, epsilon))
            mycl.done()
                
            got = res["zerooff"]["value"][0]
            expected = offset
            self.assertTrue(near(got, expected, epsilon))
            
            got = res["zeroofferr"]["value"][0]
            expected = 0
            self.assertTrue(near(got, expected, epsilon))
            mycl.done()
                
            j = j + 1

    def test_stretch(self):
        """imfit : test mask stretch"""
        imagename = multiplane_image
        yy = iatool()
        yy.open(imagename)
        mycsys = yy.coordsys().torecord()
        yy.done()
        mymask = "maskim"
        yy.fromshape(mymask, [70, 70, 1])
        yy.setcoordsys(mycsys)
        yy.addnoise()
        yy.done()
        yy.open(imagename)
        zz = yy.fitcomponents(
            mask=mymask + ">-100",
            stretch=False
        )
        self.assertTrue(zz['results']['nelements'] == 0)
        zz = imfit(
            imagename, mask=mymask + ">-100",
            stretch=False
        )
        self.assertTrue(zz['results']['nelements'] == 0)

        zz = yy.fitcomponents(
            mask=mymask + ">-100",
            stretch=True
        )
        self.assertTrue(zz['results']['nelements'] == 4)

        yy.done()
        zz = imfit(
            imagename, mask=mymask + ">-100",
            stretch=True
        )
        self.assertTrue(zz['results']['nelements'] == 4)

    def test_non_zero_channel(self):
        """imfit: test fitting for channel number other than zero (CAS-3676)"""
        imagename = multiplane_image
        chans = "1~3"
        box = "8, 10, 69, 69"
        def run_fitcomponents():
            myia = iatool()
            myia.open(imagename)
            res = myia.fitcomponents(
                box=box, chans=chans, mask="", complist="",
                estimates="", overwrite=True,
                model="", residual=""
            )
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(
                imagename=imagename, box=box, chans=chans,
                mask="", complist="", estimates="",
                overwrite=True, model="", residual=""
            )
        mycl = cltool()
        epsilon = 1e-5
        for code in (run_fitcomponents, run_imfit):
            res = code()
            mycl.fromrecord(res["results"])
            self.assertTrue(
                near(mycl.getfluxvalue(0)[0], 840.364455394, epsilon),
                str(code) + " didn't get right flux for comp 0"
            )
            self.assertTrue(
                near(mycl.getfluxvalue(1)[0], 1145.9896413, epsilon),
                str(code) + " didn't get right flux for comp 1"
            )
            print "*** got ", mycl.getfluxvalue(2)[0]

            self.assertTrue(
                near(mycl.getfluxvalue(2)[0], 3143.99613138, epsilon),
                str(code) + " didn't get right flux for comp 2"
            )
            mycl.done()
            self.assertTrue(
                res['converged'].size == 3,
                "Size of converged array is not 3"
            )
            self.assertTrue(
                all(res['converged']),
                "One or more of the converged elements are False"
            )

    def test_xx_fit(self):
        '''Imfit: Fit using pol xx'''
        success = True
        test = "fit_xx: "
        global msgs
        def run_fitcomponents():
            myia = iatool()
            myia.open(noisy_image_xx)
            res = myia.fitcomponents()
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=noisy_image_xx)
    
        for i in [0 ,1]:
            if (i == 0):
                code = run_fitcomponents
                method = test + "ia.fitcomponents: "
            else:
                code = run_imfit
                method += test + "imfit: "
            res = code()
            clist = res['results']
            if (not res['converged'][0]):
                success = False
                msgs += method + "fit did not converge unexpectedly"
            epsilon = 1e-5
            # I flux test
            got = clist['component0']['flux']['value'][0]
            expected = 60291.7956
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "I flux density test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Q flux test
            got = clist['component0']['flux']['value'][1]
            expected = 0
            if (got != expected):
                success = False
                msgs += method + "Q flux density test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # RA test
            got = clist['component0']['shape']['direction']['m0']['value']
            expected = 0.00021339
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "RA test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Dec test
            got = clist['component0']['shape']['direction']['m1']['value']
            expected = 1.935825e-5 
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "Dec test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Major axis test
            got = clist['component0']['shape']['majoraxis']['value']
            expected = 23.530022 
            epsilon = 1e-6
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Major axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Minor axis test
            got = clist['component0']['shape']['minoraxis']['value']
            expected = 18.862125  
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Minor axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Position angle test
            got = clist['component0']['shape']['positionangle']['value']
            expected = 119.88185
            epsilon = 1e-5 
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "Position angle test failure, got " + str(got) + " expected " + str(expected) + "\n"

        self.assertTrue(success,msgs)
        
    def test_multibeam(self):
        myia = iatool()
        myia.open(multibeam_image)
        # just confirm it finishes successfully
        res = myia.fitcomponents()
        self.assertTrue(res["converged"].all())
        
    def test_strange_units(self):
        '''Imfit: Test strange units'''
        myia = iatool()
        test = "test_strange_units: "
        myia.open(noisy_image)
        box = "130,89,170,129"
        outname = "bad_units.im"
        subim = myia.subimage(outname)
        myia.done()
        unit = "erg"
        subim.setbrightnessunit(unit)
        self.assertTrue(subim.brightnessunit() == unit)
        subim.done()
        def run_fitcomponents():
            myia.open(outname)
            res = myia.fitcomponents(box=box)
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=outname, box=box)
    
        for i in [0 ,1]:
            if (i == 0):
                code = run_fitcomponents
                method = test + "ia.fitcomponents: "
            else:
                code = run_imfit
                method += test + "imfit: "
            self._check_box_results(code())
            
    def test_multiple_boxes(self):
        """Test support for multiple boxes (CAS-4978)"""
        myia = iatool()
        myia.open(twogim)
        # just that it runs successfully is test enough
        myia.fitcomponents(box="37,43,59,56,143,142,157,159", estimates=twogest)
        myia.done()
        imfit(imagename=twogim, box="37,43,59,56,143,142,157,159", estimates=twogest)
        
    def test_region_selection(self):
        """Test region selection raised in CAS-5093"""
        # from George's tests
        imagename = os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imfit/CAS-5093.im'
        myia = iatool()
        residual = 'framework.resid.tmp'
        myia.open(imagename)
        shape=myia.shape()
        plane = 23
        blc = [174, 164, 0, plane]
        trc = [213, 206, 0, plane]
        reg=rg.box(blc=blc, trc=trc)
        a = myia.fitcomponents(region=reg, residual=residual)
        self.assertTrue(a['converged'])
        a = myia.fitcomponents(box="174, 164, 213, 206", chans="23", residual=residual)
        self.assertTrue(a['converged'])
        myia.done()
        
    def test_circular_gaussian(self):
        """Test convolved circular gaussian with noise doesn't throw exception (CAS-5211)""" 
        myia = iatool()
        myia.open(datapath + "circular_gaussian.im")
        self.assertTrue(myia.fitcomponents())
        myia.done()
        
    def test_k_image(self):
        """Test brightness units = K, CAS-5711"""
        myia = iatool()
        myia.open(datapath + kimage)
        # we modify the image, so want to leave the original intact
        sub = myia.subimage()
        myia.done()
        zz = sub.fitcomponents()
        flux = zz['results']['component0']['flux']
        self.assertTrue(near(flux['value'][0], 0.00028916, 1e-4))
        print "*** xx " + str(flux['error'][0])
        self.assertTrue(near(flux['error'][0], 2.22688e-8, 1e-4))
        self.assertTrue(flux['unit'] == "K.rad.rad")
        
        sub.setbrightnessunit("Jy/beam")
        zz = sub.fitcomponents()
        flux = zz['results']['component0']['flux']
        self.assertTrue(near(flux['value'][0], 60318.42427068, 1e-4))
        self.assertTrue(near(flux['error'][0], 4.6, 1e-1))
        self.assertTrue(flux['unit'] == "Jy")
        
        sub.setrestoringbeam(remove=T)
        sub.setbrightnessunit("Jy/pixel")
        zz = sub.fitcomponents()
        flux = zz['results']['component0']['flux']
        self.assertTrue(near(flux['value'][0], 12302316.98919902, 1e-4))
        self.assertTrue(near(flux['error'][0], 55, 1e-1))
        self.assertTrue(flux['unit'] == "Jy")
        
        sub.setbrightnessunit("K")
        zz = sub.fitcomponents()
        flux = zz['results']['component0']['flux']
        self.assertTrue(near(flux['value'][0],  0.00028916, 1e-4))
        print "got ", flux['error'][0]
        self.assertTrue(near(flux['error'][0], 1.3e-9, 1e-1))
        self.assertTrue(flux['unit'] == "K.rad.rad")
        
        sub.done()

        myia.done()
        
    def test_rms(self):
        '''Test rms parameter'''
        box = "130,89,170,129"
        def run_fitcomponents():
            myia = iatool()
            myia.open(noisy_image)
            res = myia.fitcomponents(box=box, rms=rms)
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(imagename=noisy_image, box=box, rms=rms)
        mycl = cltool()
        for i in [0 ,1]:
            for rms in [5, "5Jy/pixel"]:
                if (i == 0):
                    code = run_fitcomponents
                else:
                    code = run_imfit
                zz = code()
                self._check_results2(zz)
                mycl.fromrecord(zz['results'])
                got = mycl.getfluxerror(0)[0]
                print "*** got ", got
                self.assertTrue(abs(got - 224) < 1)
            
    def _check_results2(self, res):
        clist = res['results']
        self.assertTrue(
            res['converged'][0], "fit did not converge"
        )
        epsilon = 1e-5
        # I flux test
        got = clist['component0']['flux']['value'][0]
        expected = 60319.8603529
        self.assertTrue(
            near(got, expected, epsilon),
            "I flux density test failure, got " + str(got) + " expected " + str(expected)
        )
        # Q flux test
        got = clist['component0']['flux']['value'][1]
        expected = 0
        self.assertTrue(
            got == expected,
            "Q flux density test failure, got " + str(got) + " expected " + str(expected)
        )
        # RA test
        got = clist['component0']['shape']['direction']['m0']['value']
        expected = 0.00021339
        self.assertTrue(
            near_abs(got, expected, epsilon),
            "RA test failure, got " + str(got) + " expected " + str(expected)
        )
        # Dec test
        got = clist['component0']['shape']['direction']['m1']['value']
        expected = 1.935825e-5 
        self.assertTrue(
            near_abs(got, expected, epsilon),
            "Dec test failure, got " + str(got) + " expected " + str(expected)
        )
        # Major axis test
        got = clist['component0']['shape']['majoraxis']['value']
        expected = 23.545212402
        epsilon = 1e-6
        self.assertTrue(
            near(got, expected, epsilon),
            "Major axis test failure, got " + str(got) + " expected " + str(expected)
        )
        # Minor axis test
        got = clist['component0']['shape']['minoraxis']['value']
        expected =   18.864504714
        self.assertTrue(
            near(got, expected, epsilon),
            "Minor axis test failure, got " + str(got) + " expected " + str(expected)
        )
        # Position angle test
        got = clist['component0']['shape']['positionangle']['value']
        expected = 119.812966882
        epsilon = 1e-5 
        self.assertTrue(
            near_abs(got, expected, epsilon),
            "Position angle test failure, got " + str(got) + " expected " + str(expected)
        )
            
    def test_deconvolved_dictionary(self):
        """Test deconvolved dictionary"""
        def _comp_lists(zz):
            decon = cltool()
            decon.fromrecord(zz['deconvolved'])
            con = cltool()
            con.fromrecord(zz['results'])
            self.assertTrue((decon.getfluxvalue(0) == con.getfluxvalue(0)).all())
            self.assertTrue((decon.getfluxerror(0) == con.getfluxerror(0)).all())
            self.assertTrue((decon.getfluxunit(0) == con.getfluxunit(0)))
            self.assertTrue((decon.getrefdir(0) == con.getrefdir(0)))
            self.assertTrue((decon.getspectrum(0) == con.getspectrum(0)))
            self.assertFalse((decon.getshape(0) == con.getshape(0)))
            return [decon, con]
            
        shutil.copytree(datapath + decon_im, decon_im)
        myia = iatool()
        myia.open(decon_im)
        myia.setrestoringbeam("3arcmin", "3arcmin", "0deg")
        zz = imfit(imagename=decon_im)
        [decon, con] = _comp_lists(zz)
        dshape = decon.getshape(0)
        cshape = con.getshape(0)
        self.assertTrue(abs(dshape['majoraxis']['value'] - 230) < 1)
        self.assertTrue(abs(dshape['minoraxis']['value'] - 141) < 1)
        self.assertTrue(abs(cshape['majoraxis']['value'] - 292) < 1)
        self.assertTrue(abs(cshape['minoraxis']['value'] - 229) < 1)
        comp = zz['deconvolved']['component0']
        self.assertTrue(abs(comp['peak']['value'] - 2.19935) < 1e-5)
        self.assertTrue(abs(comp['peak']['error'] - 0.278872) < 1e-5)
        self.assertTrue(abs(comp['sum']['value'] - 10.93169) < 1e-5)
        beaminfo = comp['beam']
        self.assertTrue(beaminfo['beamarcsec']['major']['value'] == 180)
        self.assertTrue(beaminfo['beamarcsec']['minor']['value'] == 180)
        self.assertTrue(abs(beaminfo['beampixels'] - 10.197810) < 1e-5)
        self.assertTrue(abs(beaminfo['beamster'] - 8.62897407e-7) < 1e-15)
        
        self.assertTrue(comp['spectrum']['channel'] == 0)
        self.assertFalse(zz['results']['component0']['ispoint'])
        self.assertFalse(zz['deconvolved']['component0']['ispoint'])
        myia.setrestoringbeam("4arcmin", "4arcmin", "0deg")
        zz = imfit(imagename=decon_im)
        [decon, con] = _comp_lists(zz)
        self.assertTrue(zz['results']['component0']['ispoint'])
        self.assertTrue(zz['deconvolved']['component0']['ispoint'])
        dshape = decon.getshape(0)
        cshape = con.getshape(0)
        major = dshape['majoraxis']['value']
        minor = dshape['minoraxis']['value']
        self.assertTrue(major < 1e-59 and major > 0)
        self.assertTrue(minor < 1e-59 and minor > 0)
        self.assertTrue(abs(cshape['majoraxis']['value'] - 292) < 1)
        self.assertTrue(abs(cshape['minoraxis']['value'] - 229) < 1)
        myia.setrestoringbeam("5arcmin", "5arcmin", "0deg")
        zz = imfit(imagename=decon_im)
        self.assertTrue(zz['results']['component0']['ispoint'])
        self.assertTrue(zz['deconvolved']['component0']['ispoint'])

        [decon, con] = _comp_lists(zz)
        dshape = decon.getshape(0)
        cshape = con.getshape(0)
        self.assertTrue(
            decon.torecord()['component0']['shape']['type'] == 'Point'
        )
        self.assertTrue(abs(cshape['majoraxis']['value'] - 292) < 1)
        self.assertTrue(abs(cshape['minoraxis']['value'] - 229) < 1)
        
        decon.done()
        con.done()
        myia.done()
        
    def test_uncertainties(self):
        """Test uncertainties, CAS-3476"""
        imagename = datapath + "uncertainties_fixture.im"
        myia = iatool()
        mycl = cltool()
        
        def run_fitcomponents():
            myia = iatool()
            myia.open(imagename)
            res = myia.fitcomponents(
                chans=chans, rms=rms, noisefwhm=noisefwhm
            )
            myia.done()
            return res
        def run_imfit():
            default('imfit')
            return imfit(
                imagename=imagename, chans=chans,
                rms=rms, noisefwhm=noisefwhm
            )
        def frac(val, err):
            f = qa.div(err, val)
            f = qa.convert(f, "")
            return qa.getvalue(f)
        
        # first channel has gaussian elongated along x axis
        # second channel has gaussian elongated along y axis
        # third channel has gaussian at PA = 60 (150 degrees wrt positive x pixel axis)
        
        # uncorrelated noise, sqrt(2)/rho = 0.01329
        noisefwhm = "-1arcmin"
        rms = 0.1
        # expfrac = sqrt(2)/rho
        expfrac = 0.01329
        for chans in range(3):
            for code in [run_fitcomponents, run_imfit]:
                res = code()
                mycl.fromrecord(res['results'])
                got = mycl.getfluxerror(0)[0]
                self.assertTrue(near(got, 0.2214, 1e-3))
                shape = mycl.getshape(0)
                mj = qa.quantity(shape['majoraxis'])
                mjerr = qa.quantity(shape['majoraxiserror'])
                f = frac(mj, mjerr)
                self.assertTrue(near(f, expfrac, 1e-3))
                mn = qa.quantity(shape['minoraxis'])
                mnerr = qa.quantity(shape['minoraxiserror'])
                f = frac(mn, mnerr)
                self.assertTrue(near(f, expfrac, 1e-3))
                paerr = qa.quantity(shape['positionangleerror'])
                paerr = qa.convert(paerr,"rad")
                paerr = qa.getvalue(paerr)
                self.assertTrue(near(paerr, 0.012526, 1e-3))
                direrr = res['results']['component0']['shape']['direction']['error']
                longerr = qa.convert(direrr['longitude'], "arcsec")
                laterr = qa.convert(direrr['latitude'], "arcsec")
                if chans == 0:
                    self.assertTrue(near(qa.getvalue(longerr), 6.77, 1e-3))
                    self.assertTrue(near(qa.getvalue(laterr), 3.39, 1e-2))
                    got = qa.quantity(
                        res['deconvolved']['component0']['shape']['majoraxis']
                    )
                    # arcsec
                    self.assertTrue(near(qa.getvalue(got), 1181, 1e-3))
                    
                    got = qa.quantity(
                        res['deconvolved']['component0']['shape']['majoraxiserror']
                    )
                    self.assertTrue(near(qa.getvalue(got), 16.33, 1e-3))
                    got = qa.quantity(
                        res['deconvolved']['component0']['shape']['minoraxis']
                    )
                    self.assertTrue(near(qa.getvalue(got), 561.1, 1e-3))
                    got = qa.quantity(
                        res['deconvolved']['component0']['shape']['minoraxiserror']
                    )
                    self.assertTrue(near(qa.getvalue(got), 8.821, 1e-3))
                    got = qa.quantity(
                        res['deconvolved']['component0']['shape']['positionangle']
                    )
                    self.assertTrue(near(qa.getvalue(got), 90.67, 1e-3))
                    got = qa.quantity(
                        res['deconvolved']['component0']['shape']['positionangleerror']
                    )
                    self.assertTrue(near(qa.getvalue(got), 0.7479, 1e-3))
                if chans == 1:
                    self.assertTrue(near(qa.getvalue(longerr), 3.39, 1e-2))
                    self.assertTrue(near(qa.getvalue(laterr), 6.77, 1e-3))
                if chans == 2:
                    self.assertTrue(near(qa.getvalue(longerr), 6.10, 1e-2))
                    self.assertTrue(near(qa.getvalue(laterr), 4.48, 1e-3))
                    
                noisefwhm = "-1arcmin"        
        
        # correlated noise
        # sqrt(2)/rho(1.5, 1.5) = 0.069498
        # sqrt(2)/rho(2.5, 0.5) = 0.073398
        # sqrt(2)/rho(0.5, 2.5) = 0.065805
        
        noisefwhm = "4arcmin"
        for chans in range(3):
            for code in [run_fitcomponents, run_imfit]:
                res = code()
                mycl.fromrecord(res['results'])
                got = mycl.getfluxerror(0)[0]
                self.assertTrue(near(got, 1.248, 1e-3))
                shape = mycl.getshape(0)
                mj = qa.quantity(shape['majoraxis'])
                mjerr = qa.quantity(shape['majoraxiserror'])
                f = frac(mj, mjerr)
                self.assertTrue(near(f, 0.073398, 1e-3))
                mn = qa.quantity(shape['minoraxis'])
                mnerr = qa.quantity(shape['minoraxiserror'])
                f = frac(mn, mnerr)
                self.assertTrue(near(f, 0.065805, 1e-3))
                paerr = qa.quantity(shape['positionangleerror'])
                paerr = qa.convert(paerr,"rad")
                paerr = qa.getvalue(paerr)
                self.assertTrue(near(paerr, 0.06204, 1e-3))
                direrr = res['results']['component0']['shape']['direction']['error']
                longerr = qa.convert(direrr['longitude'], "arcsec")
                laterr = qa.convert(direrr['latitude'], "arcsec")
                if chans == 0:
                    self.assertTrue(near(qa.getvalue(longerr), 37.403, 1e-3))
                    self.assertTrue(near(qa.getvalue(laterr), 16.766, 1e-3))
                if chans == 1:
                    self.assertTrue(near(qa.getvalue(longerr), 16.766, 1e-3))
                    self.assertTrue(near(qa.getvalue(laterr), 37.403, 1e-3))
                if chans == 2:
                    self.assertTrue(near(qa.getvalue(longerr), 33.46, 1e-3))
                    self.assertTrue(near(qa.getvalue(laterr), 23.68, 1e-3))

def suite():
    return [imfit_test]
