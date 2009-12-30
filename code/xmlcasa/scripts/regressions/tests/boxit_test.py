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
# Test suite for the CASA boxit Task
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="boxit.py:description">imfit</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# boxit_test stands for boxit test
# </etymology>
#
# <synopsis>
# boxit_test.py is a Python script that tests the correctness
# boxit task in CASA.
# </synopsis> 
#
# <example>
# # This test was designed to run in the automated CASA test system.
# # This example shows who to run it manually from within casapy.
# import os
# sys.path.append( os.environ["CASAPATH"].split()[0]+'/code/xmlcasa/scripts/regressions/admin' )
# import publish_summary
# publish_summary.runTest( 'boxit_test' )
#
# </example>
#
# <motivation>
# boxit needs a regression script. Since I didn't write boxit, I'm assuming
# it works properly and am basing the tests on that assumption.
# </motivation>
#

###########################################################################
import os
import casac
from tasks import *
from taskinit import *
import sha
from __main__ import *

imagename = 'R1046_boxit' 
passed = True
msgs = ''
exp_basic_rgn = "boxit_expected_basic.rgn"
exp_basic_mask = "boxit_expected_basic.mask"

exp_minsize_rgn = "boxit_expected_minsize.rgn"
exp_minsize_mask = "boxit_expected_minsize.mask"

exp_diag_rgn = "boxit_expected_diag.rgn"
exp_diag_mask = "boxit_expected_diag.mask"

exp_boxstretch_rgn = "boxit_expected_boxstretch.rgn"
exp_boxstretch_mask = "boxit_expected_boxstretch.mask"

def basic_test():
    success = True
    test = "Basic boxit test to ensure correct region file is being written"
    global msgs, imagename
    regionfile = 'boxit_basic.reg'
    mask = 'boxit_basic.mask'
    boxit(imagename=imagename, threshold=.5, regionfile=regionfile, maskname=mask)
    if (not compare_region(exp_basic_rgn, regionfile)):
        success = False
        msgs += test + ": region file not correctly written" 
    if (not compare_mask(exp_basic_mask, mask, 'basic_mask_diff')):
        success = False
        msgs += test + ": mask  not correctly written"
    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}
    
def minsize_test():
    success = True
    test = "Test of non-default minsize parameter"
    global msgs, imagename
    regionfile = "boxit_minsize.rgn"
    mask = "boxit_minsize.mask"
    boxit(imagename=imagename, threshold=.5, regionfile=regionfile, maskname=mask, minsize=10)
    if (not compare_region(exp_minsize_rgn, regionfile)):
        success = False
        msgs += test + ": did not write expected region file"
    if (not compare_mask(exp_minsize_mask, mask, 'minsize_mask_diff')):
        success = False
        msgs += test + ": mask  not correctly written"
    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}
    
def diag_test():
    success = True
    test = "Test of non-default diag parameter"
    global msgs, imagename
    regionfile = 'boxit_diag.rgn'
    mask = 'boxit_diag.mask'
    boxit(imagename=imagename, threshold=.5, regionfile=regionfile, maskname=mask, minsize=10, diag=True)
    if (not compare_region(exp_diag_rgn, regionfile)):
        success = False
        msgs += test + ": did not write expected region file"
    if (not compare_mask(exp_diag_mask, mask, 'diag_mask_diff')):
        success = False
        msgs += test + ": mask  not correctly written"
    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}

def boxstretch_test():
    success = True
    test = "Test of non-default boxstretch parameter"
    global msgs, imagename
    regionfile = "boxit_boxtretch.rgn"
    mask = "boxit_boxtretch.mask"
    boxit(imagename=imagename, threshold=.5, regionfile=regionfile, maskname=mask, boxstretch=5)
    if (not compare_region(exp_boxstretch_rgn, regionfile)):
        success = False
        msgs += test + ": did not write expected region file"
    if (not compare_mask(exp_boxstretch_mask, mask, 'boxstretch_mask_diff')):
        success = False
        msgs += test + ": mask  not correctly written"
    global passed
    passed &= success
    return {'success' : success, 'error_msgs' : msgs}


def compare_region(expected_region_file, got_region_file):
    # sha1 test for now, not great since regions can be permuted, but works for now
    f = open(expected_region_file, 'r')
    expected_sha1 = sha.sha(f.read()).hexdigest()
    f.close()
    f = open(got_region_file, 'r')
    got_sha1 = sha.sha(f.read()).hexdigest()
    f.close()
    return expected_sha1 == got_sha1

def compare_mask(expected_mask, got_mask, difference_image):
    if (not immath(imagename=[expected_mask, got_mask], mode='evalexpr', expr='IM0-IM1', outfile=difference_image)):
        return False
    if not ia.open(difference_image):
        return False
    pixels = ia.getchunk()
    ia.done()
    return not pixels.any() 

####################################################################
# Methods for the automated CASA testing
####################################################################

def description():
    return "Tests the correctness of the boxit task"

def data():
    # The data is in <casa data regression dir>/ngc5921redux/reference/ngc5921_regresion
    #src_file='/export/chipmunk1/CASA_TEST/TESTDATA/images/ALPHA/ngc5921.im'

    return [
        imagename, exp_basic_rgn, exp_basic_mask, exp_minsize_rgn, exp_minsize_mask,
        exp_diag_rgn, exp_diag_mask, exp_boxstretch_rgn, exp_boxstretch_mask 
    ]

def doCopy():
    return [0, 0, 0, 0, 0, 0, 0, 0, 0]

def run():
    testResults=[]
    testResults.append(basic_test())
    testResults.append(minsize_test())
    testResults.append(diag_test())
    testResults.append(boxstretch_test())

    print "PASSED: ", passed
    print "*** ERROR MES: ", msgs
    if ( not passed ):
        casalog.post( msgs, 'EXCEPTION' )
        raise Exception, 'boxit test has failed!\n'+ msgs

    return []

