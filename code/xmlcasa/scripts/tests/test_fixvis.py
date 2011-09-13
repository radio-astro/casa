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
  2. Does the phase center shifting result in the expected shifts?

Note: The equinox_vis regression is a more general test of fixvis.
'''
datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/0420+417/'
inpms = '0420+417.ms'
outms = 'output.ms'
datapath2 = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/fixvis/'
inpms2 = 'twocenteredpointsources.ms'
outms2 = 'testx.ms'

class fixvis_test1(unittest.TestCase):
    def setUp(self):
        res = None
        if not os.path.exists(inpms):
            shutil.copytree(datapath + inpms, inpms)
        if not os.path.exists(inpms2):
            shutil.copytree(datapath2 + inpms2, inpms2)
        default(fixvis)
        shutil.rmtree(outms, ignore_errors=True)
        shutil.rmtree(outms2, ignore_errors=True)
        
    def tearDown(self):
        shutil.rmtree(inpms)
        shutil.rmtree(inpms2)
        shutil.rmtree(outms, ignore_errors=True)
        shutil.rmtree(outms2, ignore_errors=True)
        os.system('rm -rf test[yz]*')

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
        
    def test2(self):
        '''Apply trivial phase center shift, i.e. none.'''
        refcode = 'J2000'
        shutil.rmtree(outms2, ignore_errors=True)

        mystats = ''
        try:
            self.res = fixvis(inpms2, outms2, field='0', refcode=refcode,
                              phasecenter='J2000 18h00m02.3092s -29d59m29.9987s')
            retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
            os.system('rm -rf testy*')
            clean(vis=outms2, imagename='testy', field='0', niter=10, threshold='0.1mJy',
                  psfmode='clark', imagermode='csclean', ftmachine='ft', mask=True,
                  imsize=[128, 128], cell=['0.100000080arcsec', '0.100000080arcsec'],
                  weighting='natural', uvtaper=True, outertaper=[''],innertaper=[])
            mystats = imstat('testy.image')
        except:
            print "*** Unexpected error ***"
            
        self.assertTrue(mystats['maxposf'] == '18:00:02.309, -29.59.29.999, I, 2.26e+11Hz' and
                        (mystats['maxpos'] == [64, 64, 0, 0]).all())

    def test3(self):
        '''Apply positive phase center shift along DEC.'''
        refcode = 'J2000'
        shutil.rmtree(outms2, ignore_errors=True)
        mystats = ''
        try:
            self.res = fixvis(inpms2, outms2, field='0', refcode=refcode,
                              phasecenter='J2000 18h00m02.3092s -29d59m26.9987s')
            retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
            os.system('rm -rf testy*')
            clean(vis=outms2, imagename='testy', field='0', niter=10, threshold='0.1mJy',
                  psfmode='clark', imagermode='csclean', ftmachine='ft', mask=True,
                  imsize=[128, 128], cell=['0.100000080arcsec', '0.100000080arcsec'],
                  phasecenter='J2000 18:00:02.30921 -029.59.26.998716',
                  weighting='natural', uvtaper=True,
                  outertaper=[''], innertaper=[])

            mystats = imstat('testy.image')
        except:
            print "*** Unexpected error ***"
            
        self.assertTrue(mystats['maxposf'] == '18:00:02.309, -29.59.29.999, I, 2.26e+11Hz' and
                        (mystats['maxpos'] == [64,34,0,0]).all())

    def test4(self):
        '''Apply negative phase center shift along DEC using offset syntax.'''
        refcode = 'J2000'
        shutil.rmtree(outms2, ignore_errors=True)
        mystats = ''
        try:
            self.res = fixvis(inpms2, outms2, field='0', refcode=refcode, phasecenter='0h -0d0m3s')
            retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
            os.system('rm -rf testy*')
            clean(vis=outms2,imagename='testy',field='0',niter=10,threshold='0.1mJy',psfmode='clark',imagermode='csclean',
                  ftmachine='ft',mask=True,imsize=[128, 128],cell=['0.100000080arcsec', '0.100000080arcsec'],
                  weighting='natural',uvtaper=True,
                  outertaper=[''],innertaper=[])

            mystats = imstat('testy.image')
        except:
            print "*** Unexpected error ***"
            
        self.assertTrue(mystats['maxposf']=='18:00:02.309, -29.59.29.999, I, 2.26e+11Hz' and
                        (mystats['maxpos']==[64,94,0,0]).all())

    def test5(self):
        '''Apply positive phase center shift along RA.'''
        refcode = 'J2000'
        shutil.rmtree(outms2, ignore_errors=True)
        mystats = ''
        try:
            self.res = fixvis(inpms2, outms2, field='0', refcode=refcode,
                              phasecenter='J2000 18h00m02.5401s -29d59m29.9987s')
            self.assertTrue(self.res)
            retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
            os.system('rm -rf testy*')
            clean(vis=outms2, imagename='testy', field='0', niter=10, threshold='0.1mJy',
                  psfmode='clark', imagermode='csclean', ftmachine='ft', mask=True,
                  imsize=[128, 128], cell=['0.100000080arcsec', '0.100000080arcsec'],
                  weighting='natural',uvtaper=True, outertaper=[''], innertaper=[])

            mystats = imstat('testy.image')
        except:
            print "*** Unexpected error ***"
            
        self.assertTrue(mystats['maxposf']=='18:00:02.309, -29.59.29.999, I, 2.26e+11Hz' and
                        (mystats['maxpos']==[94,64,0,0]).all())

    def test6(self):
        '''Apply negative phase shift along RA using offset syntax (offset is an angle).'''
        refcode = 'J2000'
        shutil.rmtree(outms2, ignore_errors=True)
        mystats = ''
        try:
            self.res = fixvis(inpms2, outms2, field='0', refcode=refcode, phasecenter='-0d0m3s 0deg')
            self.assertTrue(self.res)
            retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
            os.system('rm -rf testy*')
            clean(vis=outms2,imagename='testy',field='0',niter=10,threshold='0.1mJy',psfmode='clark',imagermode='csclean',
                  ftmachine='ft',mask=True,imsize=[128, 128],cell=['0.100000080arcsec', '0.100000080arcsec'],
                  weighting='natural',uvtaper=True,
                  outertaper=[''],innertaper=[])

            mystats = imstat('testy.image')
        except:
            print "*** Unexpected error ***"
            
        self.assertTrue(mystats['maxposf']=='18:00:02.309, -29.59.29.999, I, 2.26e+11Hz' and
                        (mystats['maxpos']==[34,64,0,0]).all())

    def test7(self):
        '''Apply negative phase shift along RA in field 1 (using offset syntax, offset is a time), no shift in field 0.'''
        refcode = 'J2000'
        shutil.rmtree(outms2, ignore_errors=True)
        os.system('cp -R ' + inpms2 + ' ' + outms2)

        mystats0 = ''
        mystats1 = ''
        try:
            x = qa.div(qa.quantity(-3./3600., 'deg'),
                       qa.cos(qa.quantity(31.,'deg')))['value']*24./360.*3600. # (seconds)
            phc = str(x) + 's 0deg'
            self.res = fixvis(vis=outms2, outputvis=outms2, field='1', refcode=refcode,
                              phasecenter=phc)
            self.assertTrue(self.res)
            retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
            os.system('rm -rf testy* testz*')
            clean(vis=outms2, imagename='testy', field='0', niter=10, threshold='0.1mJy',
                  psfmode='clark', imagermode='csclean', ftmachine='ft', mask=True,
                  imsize=[128, 128], cell=['0.100000080arcsec', '0.100000080arcsec'],
                  weighting='natural',uvtaper=True, outertaper=[''],innertaper=[])
            clean(vis=outms2, imagename='testz', field='1', niter=10, threshold='0.1mJy',
                  psfmode='clark',imagermode='csclean',
                  ftmachine='ft', mask=True, imsize=[128, 128],
                  cell=['0.100000080arcsec', '0.100000080arcsec'],
                  weighting='natural', uvtaper=True,
                  outertaper=[''], innertaper=[])

            mystats0 = imstat('testy.image')
            mystats1 = imstat('testz.image')
        except:
            print "*** Unexpected error ***"

        self.assertTrue(mystats0['maxposf']=='18:00:02.309, -29.59.29.999, I, 2.26e+11Hz' and
                        (mystats0['maxpos']==[64,64,0,0]).all() and
                        mystats1['maxposf']=='18:00:02.333, -30.59.29.999, I, 2.26e+11Hz' and
                        (mystats1['maxpos']==[34,64,0,0]).all())

    
def suite():
    return [fixvis_test1]        
        
    
