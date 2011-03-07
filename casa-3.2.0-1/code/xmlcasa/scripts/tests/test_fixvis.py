import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task fixvis.

Features tested:                                                       
  1. Do converted directions in the FIELD table have the right shape? 

Note: The equinox_vis regression is a more general test of fixvis.
'''
datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/0420+417/'
inpms = '0420+417.ms'
outms = 'output.ms'

class fixvis_test1(unittest.TestCase):
    def setUp(self):
        res = None
        if not os.path.exists(inpms):
            shutil.copytree(datapath + inpms, inpms)
        default(fixvis)
        
    def tearDown(self):
        shutil.rmtree(inpms)
        shutil.rmtree(outms, ignore_errors=True)
        
    def test1(self):
        '''Do converted directions in the FIELD table have the right shape?'''
        refcode = 'J2000'
        self.res = fixvis(inpms, outms, refcode=refcode)
        tb.open(outms + '/FIELD')
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }

        def record_error(errmsg, retValue):
            """Helper function to print and update retValue on an error."""
            print "test_fixvis.test1: Error:", errmsg
            retValue['success'] = False
            retValue['error_msgs'] += errmsg + "\n"

        mscomponents = set(["table.dat",
                            "table.f0",
                            "table.f1",
                            "table.f2",
                            "table.f3",
                            "table.f4",
                            "table.f5",
                            "table.f6",
                            "table.f7",
                            "table.f8",
                            "table.f9",
                            "table.f10",
                            "table.f11",
                            "FIELD/table.dat",
                            "FIELD/table.f0"])
        for name in mscomponents:
            if not os.access(outms + "/" + name, os.F_OK):
                record_error(outms + "/" + name + " does not exist.", retValue)

        if retValue['success']:
            exp_npoly = 0
            try:
                tb.open(outms + '/FIELD')
                npoly = tb.getcell('NUM_POLY', 0)
                if npoly != exp_npoly:
                    record_error('FIELD/NUM_POLY[0], ' + str(npoly) + ' != expected '
                                 + str(exp_npoly), retValue)
                exp_shape = '[2, ' + str(npoly + 1) + ']'
                for dircol in ('PHASE_DIR', 'DELAY_DIR', 'REFERENCE_DIR'):
                    ref = tb.getcolkeywords(dircol)['MEASINFO']['Ref']
                    if ref != refcode:
                        record_error(dircol + "'s stated frame, " + ref
                                     + ', != expected ' + refcode, retValue)
                    dirshape = tb.getcolshapestring(dircol)
                    if dirshape != exp_shape:
                        record_error(dircol + "'s shape, " + dirshape
                                     + ', != expected ' + exp_shape)
            except:
                record_error('Error: Cannot get FIELD directions.', retValue)
            else:
                tb.close()
                
        self.assertTrue(retValue['success'])

    
def suite():
    return [fixvis_test1]        
        
    
