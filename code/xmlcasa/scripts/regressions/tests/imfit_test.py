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
# # This test was designed to run in the automated CASA test system.
# # This example shows who to run it manually from within casapy.
# import os
# sys.path.append( os.environ["CASAPATH"].split()[0]+'/code/xmlcasa/scripts/regressions/admin' )
# import publish_summary
# publish_summary.runTest( 'imfit_test' )
#
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
import sha
from __main__ import *


noisy_image = "gaussian_model_with_noise.im"
expected_model = "gaussian_model_with_noise_model.fits"
expected_residual = "gaussian_model_with_noise_resid.fits"
convolved_model = "gaussian_convolved.fits"
estimates_convolved = "estimates_convolved.txt"
two_gaussians_image = "two_gaussian_model.fits"
stokes_image = "imfit_stokes.fits"
two_gaussians_estimates = "estimates_2gauss.txt"
expected_new_estimates = "expected_new_estimates.txt"
passed = True
msgs = ''

def fit_using_full_image():
    success = True
    test = "fit_using_full_image: "
    global msgs
    def run_fitcomponents():
        ia.open(noisy_image)
        return ia.fitcomponents()
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
        res = code()
        clist = res['results']
        if (not res['converged']):
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
    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}
    

def fit_using_box():
    success = True
    global msgs
    for i in range(4):
        test = 'fit_using_box, loop #' + str(i) + ': '
        # the regions and box that should be used all define the same region
        # so that the results are the same for each loop (which makes the
        # code more compact)
        # i = 0: use box= keyword
        # i = 1: verify box is used when given both box and region
        # i = 2: use region record
        # i = 3: use named region (ie region record saved in image)
        if (i == 0):
            box = "130,89,170,129"
            region = ""
        elif (i == 1):
            box = "130,89,170,129"
            region = rg.box([0,0,0,0],[2,2,0,0])
        elif (i == 2):
            box = ''
            region = rg.box([130,89,0,0],[170,129,0,0])
        elif (i == 3):
            box = ''
            region = 'mybox'

        def run_fitcomponents():
            ia.open(noisy_image)
            return ia.fitcomponents(box=box, region=region)
        def run_imfit():
            default('imfit')
            return imfit(imagename=noisy_image, box=box, region=region)

        for j in [0, 1]:
            if (j == 0):
                code = run_fitcomponents
                method = test + "ia.fitcomponents: "
            else:
                code = run_imfit
                method = test + "imfit: "

            res = code()
            clist = res['results']
            if (not res['converged']):
                success = False
                msgs += method + "fit did not converge unexpectedly"
            epsilon = 1e-5
            # I flux test
            got = clist['component0']['flux']['value'][0]
            expected = 60319.8604
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "I flux density test failure, got " + str(got) \
                    + " expected " + str(expected) + "\n"
            # Q flux test
            got = clist['component0']['flux']['value'][1]
            expected = 0
            if (got != expected):
                success = False
                msgs += method + "Q flux density test failure, got " + str(got) \
                    + " expected " + str(expected) + "\n"
            # RA test
            got = clist['component0']['shape']['direction']['m0']['value']
            expected = 0.00021337
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "RA test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Dec test
            got = clist['component0']['shape']['direction']['m1']['value']
            expected = 1.935906e-05
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "Dec test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Major axis test
            got = clist['component0']['shape']['majoraxis']['value']
            expected = 23.545212
            epsilon = 1e-6
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Major axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Minor axis test
            got = clist['component0']['shape']['minoraxis']['value']
            expected = 18.86450
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Minor axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Position angle test
            got = clist['component0']['shape']['positionangle']['value']
            expected = 119.81296
            epsilon = 1e-5
            if (not near_abs(got, expected, epsilon)):
                success = False
                msgs += method + "Position angle test failure, got " + str(got) \
                    + " expected " + str(expected) + "\n"
    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}
    
def test_nonconvergence():
    test = "test_nonconvergence: "
    success = True
    global msgs

    box = '0,0,20,20'
    def run_fitcomponents():
        ia.open(noisy_image)
        return ia.fitcomponents(box=box)
    def run_imfit():
        default('imfit')
        return imfit(imagename=noisy_image, box=box)

    for i in [0, 1]:
        if (i == 0):
            code = run_fitcomponents
            method = test + "ia.fitcomponents: "
        else:
            code = run_imfit
            method = test + "imfit: "
        res = code()
        if (res['converged']):
            success = False
            msgs += method + "fit unexpectedly converged\n"

    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}

def fit_using_range():
    success = True
    global msgs
    for i in range(3):
        test = 'fit_using_range, loop #' + str(i) + ': '
        # the ranges and mask defined all define the same pixels to be used
        # so that the results are the same for each loop (which makes the
        # code more compact)
        # i = 0: use mask keyword
        # i = 1: use includepix keyword
        # i = 2: use excludepix keyword
        if (i == 0):
            mask = noisy_image + ">40"
            includepix = []
            excludepix = []
        elif (i == 1):
            mask = ''
            includepix = [40,400]
            excludepix = []
        elif (i == 2):
            mask = ''
            includepix = []
            excludepix = [-10,40]

        def run_fitcomponents():
            ia.open(noisy_image)
            return ia.fitcomponents(mask=mask, includepix=includepix, excludepix=excludepix)
        def run_imfit():
            default('imfit')
            return imfit(imagename=noisy_image, mask=mask, includepix=includepix, excludepix=excludepix)

        for j in [0, 1]:
            if (j == 0):
                code = run_fitcomponents
                method = test + "ia.fitcomponents: "
            else:
                code = run_imfit
                method = test + "imfit: "
            res = code()

            clist = res['results']
            if (not res['converged']):
                success = False
                msgs += method + "fit did not converge unexpectedly"
            epsilon = 1e-5
            # I flux test
            got = clist['component0']['flux']['value'][0]
            expected = 60354.3232
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "I flux density test failure, got " + str(got) \
                    + " expected " + str(expected) + "\n"
            # Q flux test
            got = clist['component0']['flux']['value'][1]
            expected = 0
            if (got != expected):
                success = False
                msgs += method + "Q flux density test failure, got " + str(got) \
                    + " expected " + str(expected) + "\n"
            # RA test
            got = clist['component0']['shape']['direction']['m0']['value']
            expected = 0.000213391
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "RA test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Dec test
            got = clist['component0']['shape']['direction']['m1']['value']
            expected = 1.93449e-05
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Dec test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Major axis test
            got = clist['component0']['shape']['majoraxis']['value']
            expected = 23.541712
            epsilon = 1e-7
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Major axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Minor axis test
            got = clist['component0']['shape']['minoraxis']['value']
            expected = 18.882029
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Minor axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
            # Position angle test
            got = clist['component0']['shape']['positionangle']['value']
            expected = 119.769648
            if (not near(got, expected, epsilon)):
                success = False
                msgs += method + "Position angle test failure, got " + str(got) + \
                    " expected " + str(expected) + "\n"

    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}

# Do the got and expected images match?
# @param got The name of the test image
# @param expected The name of the expected image
# @param difference The name of the difference image to write
def check_image(got, expected, difference):
    ia.open(got);
    expr = "\"" + got + "\" - \"" + expected + "\"";
    ia.imagecalc(difference, expr, True);
    ia.open(difference);
    stats = ia.statistics()
    return stats['sumsq'] == 0

# Test writing of residual and model images 
def residual_and_model_test():
    test = "residual_and_model_test: "
    success = True
    global msgs
    residual = 'residual.im'
    model = 'model.im'
    box="100,100,200,200"
    for i in [0, 1]:
        residual = "residual.im" + str(i)
        model = "model.im" + str(i)
        def run_fitcomponents():
            ia.open(noisy_image)
            return ia.fitcomponents(box=box, residual=residual, model=model)
        def run_imfit():
            default('imfit')
            return imfit(imagename=noisy_image, box=box, residual=residual, model=model)

        if (i == 0):
            code = run_fitcomponents
            method = test + "ia.fitcomponents: "
        else:
            code = run_imfit
            method = test + "imfit: "
        res = code()
        clist = res['results']

        if (not res['converged']):
            success = False
            msgs += method + "fit did not converge unexpectedly"
        if (not check_image(residual, expected_residual, 'residualDifference.im')):
            success = False
            msgs += method + "Did not get expected residual image\n"
        if (not check_image(model, expected_model, 'modelDifference.im')):
            success = False
            msgs += method + "Did not get expected model image\n"

    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}        

# Test using initial estimates and fixed parameters
def fit_using_estimates():
    success = True
    test = 'fit_using_estimates: '
    global msgs

    def run_fitcomponents():
        ia.open(convolved_model)
        return ia.fitcomponents(estimates=estimates_convolved)
    def run_imfit():
        default('imfit')
        return imfit(imagename=convolved_model, estimates=estimates_convolved)

    for i in [0, 1]:
        if (i == 0):
            code = run_fitcomponents
            method = test + "ia.fitcomponents: "
        else:
            code = run_imfit
            method = test + "imfit: "
        res = code()

        clist = res['results']
        if (not res['converged']):
            success = False
            msgs += method + "fit did not converge unexpectedly"
        epsilon = 1e-5
        # I flux test
        got = clist['component0']['flux']['value'][0]
        expected = 60082.6
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
        expected = 0.000213318
        if (not near(got, expected, epsilon)):
            success = False
            msgs += method + "RA test failure, got " + str(got) + " expected " + str(expected) + "\n"
        # Dec test
        got = clist['component0']['shape']['direction']['m1']['value']
        expected = 1.939254e-5 
        if (not near(got, expected, epsilon)):
            success = False
            msgs += method + "Dec test failure, got " + str(got) + " expected " + str(expected) + "\n"
        # Major axis test
        got = clist['component0']['shape']['majoraxis']['value']
        expected = 28.21859344 
        epsilon = 1e-7
        if (not near(got, expected, epsilon)):
            success = False
            msgs += method + "Major axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
        # Minor axis test
        got = clist['component0']['shape']['minoraxis']['value']
        expected = 25.55011520
        if (not near(got, expected, epsilon)):
            success = False
            msgs += method + "Minor axis test failure, got " + str(got) + " expected " + str(expected) + "\n"
        # Position angle test
        got = clist['component0']['shape']['positionangle']['value']
        expected = 126.3211050
        if (not near(got, expected, epsilon)):
            success = False
            msgs += method + "Position angle test failure, got " + str(got) + " expected " + str(expected) + "\n"

    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}
   
# are the two specified numeric values relatively close to each other? 
def near (first, second, epsilon):
    return (abs((first - second)/first) <= abs(epsilon))

def near_abs(first, second, epsilon):
    return abs(first - second) <= epsilon

# test writing, appending, and overwriting log files
def test_logfile():
    success = True
    test = "test_logfile: "
    global msgs

    for i in [0, 1]:
        logfile = os.getcwd() + "/imfit.log" + str(i)
        if (i == 0):
            def run_fitcomponents(append=None):
                ia.open(two_gaussians_image)
                if (append == None):
                    return ia.fitcomponents(estimates=two_gaussians_estimates, logfile=logfile)
                else:
                    return ia.fitcomponents(estimates=two_gaussians_estimates, logfile=logfile, append=append)
            code = run_fitcomponents
            method = test + "ia.fitcomponents: "
        else:
            def run_imfit(append=None):
                default('imfit')
                if (append == None):
                    return imfit(imagename=two_gaussians_image, estimates=two_gaussians_estimates, logfile=logfile)
                else:
                    return imfit(
                        imagename=two_gaussians_image, estimates=two_gaussians_estimates,
                        logfile=logfile, append=append
                    )
            code = run_imfit
            method = test + "imfit: "
        res = code()
        if (not os.path.exists(logfile)):
            passed = False
            msgs += method + "logfile was not written\n"
            return {'success' : passed, 'error_msgs' : msgs}        
   
        if ( count_matches(logfile, "****** Fit performed") != 1):
            success = False
            msgs += method + "unexpected logfile\n"
        #default, append
        res = code()
        if (not os.path.exists(logfile)):
            passed = False
            msgs += method + "logfile was not written\n"
            return {'success' : passed, 'error_msgs' : msgs}        
   
        if ( count_matches(logfile, "****** Fit performed") != 2):
            success = False
            msgs += method + "logfile not appended\n"
    
        # explicit append
        res = code(True)
        if (not os.path.exists(logfile)):
            passed = False
            msgs += method + "logfile was not written\n"
            return {'success' : passed, 'error_msgs' : msgs}        
   
        if ( count_matches(logfile, "****** Fit performed") != 3):
            success = False
            msgs += method + "logfile not appended\n"
        # overwrite
        res = code(False)
        if (not os.path.exists(logfile)):
            passed = False
            msgs += method + "logfile was not written\n"
            return {'success' : passed, 'error_msgs' : msgs}        
   
        if ( count_matches(logfile, "****** Fit performed") != 1):
            success = False
            msgs += method + "logfile not overwritten\n"

    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}

# Test writing of a new estimates file
def test_newestimates():
    success = True
    test = 'test_newestimates: '
    global msgs

    for i in [0, 1]:
        newestimates = "newestimates" + str(i) + ".txt"
        if (i == 0):
            def run_fitcomponents():
                ia.open(two_gaussians_image)
                return ia.fitcomponents(estimates=two_gaussians_estimates, newestimates=newestimates)
            code = run_fitcomponents
            method = test + "ia.fitcomponents: "
        else:
            def run_imfit():
                default('imfit')
                return imfit(
                    imagename=two_gaussians_image, estimates=two_gaussians_estimates, newestimates=newestimates
                )
            code = run_imfit
            method = test + "imfit: "
        res = code()

        if (not os.path.exists(newestimates)):
            passed = False
            msgs += method + "new estimates file was not written\n"
            return {'success' : passed, 'error_msgs' : msgs}        
 
        expected_sha = sha.sha(open(expected_new_estimates, 'r').read()).hexdigest()

        got_sha = sha.sha(open(newestimates, 'r').read()).hexdigest()
        if (got_sha != expected_sha):
            success = False
            msgs += method + "new estimates file differs from expected\n"
    
    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}
    
## Test imfit on various polarization planes
def test_polarization_image():
    success = True
    test = 'test_polarization_image: '
    global msgs
    def run_fitcomponents(stokes):
        ia.open(stokes_image)
        return ia.fitcomponents(stokes=stokes)
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
            if (not res['converged']):
                success = False
                msgs += method + "fit did not converge unexpectedly for stokes " + stokes[j]
            got = clist['component0']['flux']['value'][j]

            # flux density test
            if (not near(got, expectedFlux[j], 1e-5)):
                success = False
                msgs += method + " " + stokes + " flux density test failure, got " + str(got) + " expected " + str(expectedFlux[j]) + "\n"

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
    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}
 
   
# count the number of lines in the specified file in which the spcified string occurs
def count_matches(filename, match_string):
    count = 0
    for line in open(filename):
        if (match_string in line):
            count += 1
    return count

####################################################################
# Methods for the automated CASA testing
####################################################################

def description():
    return " Tests the correctness of the ia.fitcomponents method in CASA"

def data():
    # The data is in <casa data regression dir>/ngc5921redux/reference/ngc5921_regresion
    #src_file='/export/chipmunk1/CASA_TEST/TESTDATA/images/ALPHA/ngc5921.im'

    return [
        noisy_image, expected_model, expected_residual,
        convolved_model, estimates_convolved,
        two_gaussians_image, two_gaussians_estimates,
        expected_new_estimates, stokes_image
    ]


def doCopy():
    return [0, 0, 0, 0, 0, 0, 0, 0, 0]

def run():
    testResults=[]
    testResults.append(fit_using_full_image())
    testResults.append(fit_using_box())
    testResults.append(fit_using_range())
    testResults.append(residual_and_model_test())
    testResults.append(fit_using_estimates())
    testResults.append(test_nonconvergence())
    testResults.append(test_logfile())
    testResults.append(test_newestimates())   
    testResults.append(test_polarization_image())   
 
    print "PASSED: ", passed
    print "*** ERROR MES: ", msgs
    if ( not passed ):
        casalog.post( msgs, 'EXCEPTION' )
        raise Exception, 'imfit test has failed!\n'+ msgs

    return []

