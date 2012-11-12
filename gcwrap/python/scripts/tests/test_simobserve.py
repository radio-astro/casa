import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from simutil import *
import unittest

import numpy
import glob
# to rethrow exception 
import inspect
glb = sys._getframe(len(inspect.stack())-1).f_globals
if glb.has_key('__rethrow_casa_exceptions'):
    rethrow_org = glb['__rethrow_casa_exceptions']
else:
    rethrow_org = False

#
# Unit test of simobserve task.
# 
class simobserve_unittest_base:
    """
    Base class of simobserve unit test.
    The class defines common variables and test methods.
    """
    # Variables
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/simobserve/'
    thistask = "simobserve"
    imkeys=['max','mean','min','npts','rms','blc','blcf','trc','trcf','sigma','sum','sumsq']
    # relative and ablsolute tolerance
    # (atol=1.e8 effectively means to ignore absolute tolerance)
    rtol = 1.0e-5
    atol = 1.0e8 

    # Test methods
    def _check_file( self, name ):
        isthere = os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))

    def _get_imstats( self, name ):
            self._check_file(name)
            ia.open(name)
            stats = ia.statistics()
            ia.close()
            return stats

    def _get_msstats( self, name, column, compval):
        self._check_file(name)
        ms.open(name)
        stats = ms.statistics(column, compval)
        ms.close()
        return stats[column]

    def _check_imstats(self, name, ref):
        # ref: a dictionary of reference statistics or reference image name
        # name: the name of image to compare statistics
        if type(ref) == str:
            # get statistics of reference image
            ref = self._get_imstats(ref)
        # get statistics of  image
        stats = self._get_imstats(name)
        # define statistics to compare
        if hasattr(self,'imkeys'):
            compkeys = self.imkeys
        else:
            compkeys = ref.keys()
        for key in compkeys:
            message="image statistic '%s' does not match" % key
            if type(stats[key])==str:
                self.assertEqual(stats[key],ref[key],
                                 msg=message)
            else:
                ret=numpy.allclose(stats[key],ref[key],
                                   rtol=self.rtol,atol=self.atol)
                self.assertEqual(ret,True,msg=message)

    def _check_msstats(self,name,ref):
        # ref: a dictionary of reference statistics or reference MS name
        # name: the name of MS to compare statistics
        if type(ref) == str:
            # get statistics of reference MS
            ref=self._get_msstats(ref)
        stats=self._get_msstats(name)
        if hasattr(self,'mskeys'):
            compkeys = self.mskeys
        else:
            compkeys = ref.keys()
        # define statistics to compare
        for key in compkeys:
            message="MS statistic '%s' does not match" % key
            ret=numpy.allclose([stats[key]],[ref[key]],
                               rtol=self.rtol,atol=self.atol)
            self.assertEqual(ret,True,msg=message)

    # common helper methods
    def _copy_input(self,datanames=None):
        if not datanames:
            return
        if type(datanames) == str:
            datanames = [datanames]
        if len(datanames) > 0:
            for indata in datanames:
                if os.path.exists(indata): self._remove(indata)
                if os.path.exists(self.datapath+indata):
                    #print "copying", indata
                    self._copy(self.datapath+indata, indata)

    def _remove(self, path):
        if os.path.isdir(path):
            shutil.rmtree(path)
        else:
            os.remove(path)

    def _copy(self, src, dest):
        if os.path.isdir(src):
            shutil.copytree(src, dest)
        else:
            shutil.copy(src, dest)

    def _get_data_prefix(self,cfgname, project=""):
        if str.upper(cfgname[0:4]) == "ALMA":
            foo=cfgname.replace(';','_')
        else:
            foo = cfgname
            foo=foo.replace(".cfg","")
            sfoo=foo.split('/')
            if len(sfoo)>1: foo=sfoo[-1]

        return project+"."+foo

# ########################################################################
# #
# # Test skymodel only simulations
# #
# class simobserve_sky(simobserve_unittest_base,unittest.TestCase):
#     """
#     Test skymodel simulations
#     - Single step at a time
#     - All steps with 2D, 3D and 4D input skymodel
#     """
#     inmodel = ""
#     refmod = datapath+"" # reference modified skymodel
#     # reference simulated MS
#     refsdms = datapath+"" 
#     refintms = datapath+"" 
#     refnoisysdms = datapath+""
#     refnoisyintms = datapath+""
#     project = simobserve_unittest_base.thistask+"_sky"

#     # Reserved methods
#     def setUp(self):
#         if os.path.exists(self.infile):
#             shutil.rmtree(self.infile)
#         shutil.copytree(self.datapath+self.infile, self.infile)

#         default(simobserve)

#     def tearDown(self):
#         if (os.path.exists(self.infile)):
#             shutil.rmtree(self.infile)
#         shutil.rmtree(self.project)
        
#     # Tests of skymodel simulations
#     def testSky_skymodel(self):
#         """Test skymodel simulation: only modify model"""
#         skymodel = self.inmodel
#         # params
#         setpointings = False
#         ptgfile = self.ptg
#         obsmode = ""
#         thermalnoise = ""
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          setpointings=setpointings,ptgfile=ptgfile,
#                          obsmode=obsmode,thermalnoise=thermalnoise)

#     def testSky_pointing(self):
#         """Test skymodel simulation: only setpointing"""
#         skymodel = self.refmod
#         setpointings = True
#         # params
#         obsmode = ""
#         thermalnoise = ""
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          setpointings=setpointings,ptgfile=ptgfile,
#                          obsmode=obsmode,thermalnoise=thermalnoise)
        
#     # Tests of Single dish simulations
#     def testSky_sdAll(self):
#         """Test skymodel simulation: single dish"""
#         skymodel = self.inmodel
#         obsmode = "sd"
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          obsmode=obsmode)
    
#     def testSky_sdObs(self):
#         """Test skymodel simulation: only observation (SD)"""
#         skymodel = self.refmod
#         setpointings = False
#         obsmode = "sd"
#         thermalnoise = ""
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          setpointings=setpointings,ptgfile=ptgfile,
#                          obsmode=obsmode,thermalnoise=thermalnoise)
    
#     # Tests of interferometer simulation
#     def testSky_intAll(self):
#         """Test skymodel simulation: interferometer"""
#         skymodel = self.inmodel
#         obsmode = "int"
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          obsmode=obsmode)

#     def testSky_intObs(self):
#         """Test skymodel simulation: only observation (INT)"""
#         skymodel = self.refmod
#         setpointings = False
#         obsmode = "int"
#         thermalnoise = ""
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          setpointings=setpointings,ptgfile=ptgfile,
#                          obsmode=obsmode,thermalnoise=thermalnoise)
    
#     def testSky_intLeak(self):
#         """Test skymodel simulation: only observation (INT)"""
#         skymodel = self.refmod
#         setpointings = False
#         obsmode = ""
#         thermalnoise = ""
#         leakage = 0.5
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          setpointings=setpointings,ptgfile=ptgfile,
#                          obsmode=obsmode,thermalnoise=thermalnoise,
#                          leakage=leakage)

    
    

# ########################################################################
# #
# # Test components list only simulations
# #
# class simobserve_comp(simobserve_unittest_base,unittest.TestCase):
#     """
#     Test components list simulations
#     """

# ########################################################################
# #
# # Test skymodel + components list simulations
# #
# class simobserve_skycomp(simobserve_unittest_base,unittest.TestCase):
#     """
#     Test skymodel + components list simulations
#     """

########################################################################
#
# Test noise calculations
#
class simobserve_noise(simobserve_unittest_base,unittest.TestCase):
    """
    Test noise level of simulated MS
    """
    # global variables of the class
    inimage = "flatone.model"
    ptgfile = "flatone.single.ptg.txt"
    indata = [inimage, ptgfile]

    # standard parameter settings
    #project = simobserve_unittest_base.thistask+"_nz"
    project = "noise_sd"
    project_int = "noise_int"
    #indirection = 'J2000 19h00m00 -23d00m00'
    #incenter = "345GHz"
    #inwidth = "10MHz"
    tint = "4s"
    tottime = "1800s" # 30min
    mapsize = ["5arcsec","5arcsec"] # single pointing
    pointingspacing = "10arcsec"
    sdantlist = "aca.tp.cfg"
    antennalist = ""
    tau0 = 1.0
    pwv = 1.0
    graphics = 'file'

    skymodel = project + "/" + project + ".aca.tp.model"

    prevmsg = "The noise level differs from the previous value: %f (previous: %f)"
    anamsg = "The noise level differs more than 10%% from the analytic value: %f (analytic: %f)"

    # Reserved methods
    def setUp(self):
        # Add new line for better reading (these tests always print errors).
        print ""
        self._copy_input(self.indata)
        default(simobserve)

    def tearDown(self):
        if (os.path.exists(self.inimage)):
            shutil.rmtree(self.inimage)
        if os.path.exists(self.project):
            shutil.rmtree(self.project)

    #-----------------------------------------------------------------#
    # thermalnoise = "tsys-manual"
    def testNZ_intMan(self):
        """Test INT thermal noise (tsys-manual)"""
        project = self.project_int
        self._copy_input(project)
        skymodel = project+"/noise_int.aca_cycle1.model"
        antlist = "aca_cycle1.cfg"
        thermalnoise="tsys-manual"
        res = simobserve(project=project,skymodel=skymodel,
                         setpointings=False,integration=self.tint,
                         obsmode='',sdantlist="",antennalist=antlist,
                         thermalnoise=thermalnoise,tau0=self.tau0,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(project,antlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="manual",sd=False,aca7m=True)
        #print "MS noise:", msnoise
        #print "Analytic:", ananoise
        # Now compare the result
        refval = 9.78451847017  # testing only REAL part
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    def testNZ_sdMan(self):
        """Test SD thermal noise (tsys-manual): standard parameter set"""
        thermalnoise="tsys-manual"
        self._copy_input(self.project)
        res = simobserve(project=self.project,skymodel=self.skymodel,
                         setpointings = False,integration=self.tint,
                         obsmode="",sdantlist=self.sdantlist,antennalist="",
                         thermalnoise=thermalnoise,tau0=self.tau0,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="manual",sd=True)
        # Now compare the result
        refval = 4.91379000092
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    def testNZ_sdMan_tau(self):
        """Test SD thermal noise (tsys-manual): tau0=1.5"""
        thermalnoise="tsys-manual"
        tau0 = 1.5
        self._copy_input(self.project)
        res = simobserve(project=self.project,skymodel=self.skymodel,
                         setpointings = False,integration=self.tint,
                         obsmode="",sdantlist=self.sdantlist,antennalist="",
                         thermalnoise=thermalnoise,tau0=tau0,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="manual",sd=True,tau0=tau0)
        # Now compare the result
        refval = 9.27620818144
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))
        
    def testNZ_sdMan_dnu(self):
        """Test SD thermal noise (tsys-manual): inwidth='1MHz'"""
        thermalnoise="tsys-manual"
        inwidth = '1MHz'
        # need to recalculate skymodel and MS
        res = simobserve(project=self.project,skymodel=self.inimage,
                         inwidth=inwidth,setpointings=False,
                         ptgfile=self.ptgfile,integration=self.tint,
                         obsmode='sd',sdantlist=self.sdantlist,
                         antennalist=self.antennalist,totaltime=self.tottime,
                         thermalnoise=thermalnoise,tau0=self.tau0,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="manual",sd=True,dnu=inwidth)
        # Now compare the result
        refval = 15.5387677134
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    def testNZ_sdMan_tint(self):
        """Test SD thermal noise (tsys-manual): integration='2s'"""
        thermalnoise="tsys-manual"
        integration = '2s'
        totaltime = '900s'
        # need to recalculate MS
        res = simobserve(project=self.project,skymodel=self.inimage,
                         setpointings=False,ptgfile=self.ptgfile,
                         integration=integration,
                         obsmode='sd',sdantlist=self.sdantlist,
                         antennalist=self.antennalist,totaltime=totaltime,
                         thermalnoise=thermalnoise,tau0=self.tau0,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="manual",sd=True,integration=integration)
        # Now compare the result
        refval = 6.94461790663
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))
        
    def testNZ_sdMan_el(self):
        """Test SD thermal noise (tsys-manual): elevation = 60 deg"""
        thermalnoise="tsys-manual"
        indir = 'J2000 19h00m00 -53d00m00'
        # need to recalculate ptgs and MS
        res = simobserve(project=self.project,skymodel=self.inimage,
                         indirection=indir,setpointings=True,
                         integration=self.tint,mapsize=self.mapsize,
                         pointingspacing=self.pointingspacing,
                         obsmode='sd',sdantlist=self.sdantlist,
                         antennalist=self.antennalist,totaltime=self.tottime,
                         thermalnoise=thermalnoise,tau0=self.tau0,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="manual",sd=True,dir=-53.)
        print "MS noise:", msnoise
        print "Analytic:", ananoise
        # Now compare the result
        refval = 6.0450620991
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    #-----------------------------------------------------------------#
    # thermalnoise = "tsys-atm"
    def testNZ_intAtm(self):
        """Test INT thermal noise (tsys-atm): standard parameter set"""
        project = self.project_int
        self._copy_input(project)
        skymodel = project+"/noise_int.aca_cycle1.model"
        antlist = "aca_cycle1.cfg"
        thermalnoise="tsys-atm"
        res = simobserve(project=project,skymodel=skymodel,
                         setpointings=False,integration=self.tint,
                         obsmode='',sdantlist="",antennalist=antlist,
                         thermalnoise=thermalnoise,tau0=self.tau0,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(project,antlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="atm",sd=False,aca7m=True)
        # Now compare the result
        refval = 2.27105136133
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    def testNZ_sdAtm(self):
        """Test SD thermal noise (tsys-atm): standard parameter set"""
        thermalnoise="tsys-atm"
        self._copy_input(self.project)
        res = simobserve(project=self.project,skymodel=self.skymodel,
                         setpointings = False,integration=self.tint,
                         obsmode="",sdantlist=self.sdantlist,antennalist="",
                         thermalnoise=thermalnoise,user_pwv=self.pwv,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="atm",sd=True)
        # Now compare the result
        refval = 1.13985820952
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    def testNZ_sdAtm_pwv(self):
        """Test SD thermal noise (tsys-atm): pwv = 2.0"""
        thermalnoise="tsys-atm"
        pwv = 2.0
        self._copy_input(self.project)
        res = simobserve(project=self.project,skymodel=self.skymodel,
                         setpointings = False,integration=self.tint,
                         obsmode="",sdantlist=self.sdantlist,antennalist="",
                         thermalnoise=thermalnoise,user_pwv=pwv,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="atm",sd=True,pwv=pwv)
        # Now compare the result
        refval = 1.61886644931
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    def testNZ_sdAtm_dnu(self):
        """Test SD thermal noise (tsys-atm): inwidth='1MHz'"""
        thermalnoise="tsys-atm"
        inwidth = '1MHz'
        # need to recalculate skymodel and MS
        res = simobserve(project=self.project,skymodel=self.inimage,
                         inwidth=inwidth,setpointings=False,
                         ptgfile=self.ptgfile,integration=self.tint,
                         obsmode='sd',sdantlist=self.sdantlist,
                         antennalist=self.antennalist,totaltime=self.tottime,
                         thermalnoise=thermalnoise,user_pwv=self.pwv,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="atm",sd=True,dnu=inwidth)
        # Now compare the result
        refval = 3.60454794841
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    def testNZ_sdAtm_tint(self):
        """Test SD thermal noise (tsys-atm): integration = '2s'"""
        thermalnoise="tsys-atm"
        integration = '2s'
        totaltime = '900s'
        # need to recalculate MS
        res = simobserve(project=self.project,skymodel=self.inimage,
                         setpointings=False,ptgfile=self.ptgfile,
                         integration=integration,
                         obsmode='sd',sdantlist=self.sdantlist,
                         antennalist=self.antennalist,totaltime=totaltime,
                         thermalnoise=thermalnoise,user_pwv=self.pwv,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="atm",sd=True,integration=integration)
        # Now compare the result
        refval = 1.61165299786
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    def testNZ_sdAtm_el(self):
        """Test SD thermal noise (tsys-atm): elevation = 60 deg"""
        thermalnoise="tsys-atm"
        indir = 'J2000 19h00m00 -53d00m00'
        # need to recalculate ptgs and MSes
        res = simobserve(project=self.project,skymodel=self.inimage,
                         indirection=indir,setpointings=True,
                         integration=self.tint,mapsize=self.mapsize,
                         pointingspacing=self.pointingspacing,
                         obsmode='sd',sdantlist=self.sdantlist,
                         antennalist=self.antennalist,totaltime=self.tottime,
                         thermalnoise=thermalnoise,user_pwv=self.pwv,
                         graphics=self.graphics)
        self.assertTrue(res)
        # check for output file
        msdict = self._get_ms_names(self.project,self.sdantlist)
        if msdict is None:
            self.fail("Could not find output MSes")
        noisyms = msdict['noisy']
        origms = msdict['original']
        msnoise = self._get_noise(noisyms, origms)
        ananoise = self._calc_alma_noise(mode="atm",sd=True,dir=-53.)
        #print "MS noise:", msnoise
        #print "Analytic:", ananoise
        # Now compare the result
        refval = 1.22177450558
        self.assertTrue(abs((msnoise-refval)/refval) < 5.e-2,\
                        msg=self.prevmsg % (msnoise, refval))
        self.assertTrue(abs((msnoise-ananoise)/ananoise) < 1.e-1, \
                        msg=self.anamsg % (msnoise, ananoise))

    #-----------------------------------------------------------------#
    # Helper functions
    def _get_ms_names(self, project, antennalist):
        retDict = {"original": None, "noisy": None}
        prefix = project+"/"+self._get_data_prefix(antennalist, project)
        mslist = glob.glob(prefix+"*.noisier*.ms")
        if len(mslist) > 0:
            retDict['noisy'] = mslist[0]
        else:
            mslist = glob.glob(prefix+"*.noisy*.ms")
            if len(mslist) > 0:
                retDict['noisy'] = mslist[0]
            else:
                return None
        
        if os.path.exists(prefix+".ms"):
            retDict["original"] = prefix+".ms"
        elif os.path.exists(prefix+".sd.ms"):
            retDict["original"] = prefix+".sd.ms"
        else:
            return None

        return retDict

    def _get_noise(self, noisyms, origms):
        tb.open(noisyms)
        noisy_data = tb.getcol("DATA")
        tb.close()
        tb.open(origms)
        orig_data = tb.getcol("DATA")
        tb.close()
        diff_data = noisy_data - orig_data
        #if (diff_data.imag != 0).any():
        #    return [diff_data.real.std(),diff_data.imag.std()]
        #else:
        #    return [diff_data.real.std()]
        return diff_data.real.std()

    def _calc_alma_noise(self, mode="manual",sd=True,aca7m=False,\
                         freq="345GHz",dnu="10MHz",integration='4s',dir=-23.,\
                         pwv=1.0,tground=269.,tsky=263.,tau0=1.0):
        if sd:
            senscoeff = 1.0
        else:
            senscoeff = 1./numpy.sqrt(2)

        freq_ghz = qa.convert(freq,"GHz")['value']
        dnu_hz = qa.convert(dnu,"Hz")['value']
        tint_sec = qa.convert(integration,"s")['value']

        if aca7m:
            telescope = "ACA"
            diam = 7.
        else: #12m
            telescope = "ALMA"
            diam = 12.

        myutil = simutil()
        eta_phase, espill, eta_block, eta_taper, ecorr, trx \
                   = myutil.noisetemp(telescope=telescope,freq=freq)
        eant = eta_phase * espill * eta_block * eta_taper

        tcmb = 2.725
        # ALMA latitude
        antpos = -23.022886 # deg
        el = numpy.pi/2.- (dir-antpos)/180.*numpy.pi     # rad
        airmass = 1./numpy.sin(el)

        hn_k = 0.04799274551*freq_ghz

        if mode.find("manual") > -1:
            tau = tau0*airmass
            Rtcmb = 1./(numpy.exp(hn_k/tcmb)-1.)
            Rtatmos = 1./(numpy.exp(hn_k/tsky)-1.)
            Rtground = 1./(numpy.exp(hn_k/tground)-1.)
            R = Rtcmb*espill + \
                numpy.exp(tau) *( espill * (1.-numpy.exp(-tau)) * Rtatmos \
                               + (1.-espill) * Rtground + trx/hn_k)
        else: # atm
            tsky, tau0 = self._get_atmsky(tground, tcmb, freq, dnu,
                                         espill, pwv, airmass)
            tau = tau0*airmass
            R = numpy.exp(tau) * (1./(numpy.exp(hn_k/tsky)-1.) + trx/hn_k)

        amp = 8 * 1.38062e-16 * 1e23 * 1e-4 / (eant * ecorr * numpy.pi)
        tsys=hn_k*R
        factor = numpy.sqrt(senscoeff * amp / numpy.sqrt(dnu_hz * tint_sec))
        par = diam / factor / numpy.sqrt(tsys)

        return 1./par**2

    def _get_atmsky(self, tground, tcmb, freq, dnu, espill, pwv, airmass):
        freq_ghz = qa.convert(freq,"GHz")['value']
        hn_k = 0.04799274551*freq_ghz

        at.initAtmProfile(temperature=qa.quantity(tground))
        atmnchan = 10.
        fcntr = qa.quantity(freq)
        bw = qa.quantity(dnu)
        fres = qa.div(qa.quantity(dnu),atmnchan)
        at.initSpectralWindow(nbands=1,fCenter=fcntr,fWidth=bw,fRes=fres)
        at.setSkyBackgroundTemperature(qa.quantity(tcmb,"K"))
        at.setAirMass(airmass)
        at.setUserWH2O(qa.quantity(pwv,"mm"))
        rchan = at.getRefChan(spwid=0)
        ratio = at.getUserWH2O()["value"]/at.getGroundWH2O()["value"]

        #tsky = at.getTebbSky(nc=-1,spwid=0)
        dz = at.getProfile()[1]["value"]
        tz = at.getProfile()[2]["value"]
        radiance = 0.
        kv = 0.
        for iz in range(at.getNumLayers()):
            dtau0 = dz[iz] * (at.getAbsTotalDry(iz, rchan, 0)["value"] + \
                              at.getAbsTotalWet(iz, rchan, 0)["value"] * ratio)
            dmass = dtau0[0]*airmass
            radiance += (1./(numpy.exp(hn_k/tz[iz])-1.)) * numpy.exp(-kv*airmass) \
                        * (1. - numpy.exp(-dmass))
            kv += dtau0[0]

        R = espill * (radiance + (1./(numpy.exp(hn_k/tcmb)-1.))*numpy.exp(-kv*airmass))\
            + (1./(numpy.exp(hn_k/qa.quantity(tground)["value"]) - 1.)) * (1. - espill)
        tsky = hn_k / numpy.log(1. + (1. / R))
        tau0 = at.getDryOpacity(spwid=0) + \
               at.getWetOpacity(spwid=0)['value'][0]
        return tsky, tau0

    #def testNZ_genSd(self):
    #    self._generate_sd_input()
    def _generate_sd_input(self):
        project = self.project
        inimage = "flatoneimage.model"
        indirection = 'J2000 19h00m00 -23d00m00'
        incenter = "345GHz"
        inwidth = "10MHz"
        tint = "4s"
        tottime = "1800s" # 30min
        mapsize = ["5arcsec","5arcsec"] # single pointing
        pointingspacing = "10arcsec"
        sdantlist = "aca.tp.cfg"
        self._copy_input(inimage)
        simobserve(project=project,skymodel=inimage,indirection=indirection,
                   incenter=incenter,inwidth=inwidth,setpointings=True,
                   integration=tint,mapsize=mapsize,
                   pointingspacing=pointingspacing,
                   obsmode='sd',sdantlist=sdantlist,antennalist="",
                   totaltime=tottime,thermalnoise="",graphics="none")
        basename = project + "/" + self._get_data_prefix(sdantlist,project=project)
        shutil.move(basename+".skymodel", basename+".model")
        # only *.model, *.ms, and *.ptg.txt is necessary

    #def testNZ_genInt(self):
    #    self._generate_int_input()
    def _generate_int_input(self):
        project = self.project_int
        inimage = "flatoneimage.model"
        indirection = 'J2000 19h00m00 -23d00m00'
        incenter = "345GHz"
        inwidth = "10MHz"
        incell = "1arcsec"
        tint = "4s"
        tottime = "1800s" # 30min
        mapsize = ["5arcsec","5arcsec"] # single pointing
        pointingspacing = "10arcsec"
        antlist = "aca_cycle1.cfg"
        totaltime = "100s"
        self._copy_input(inimage)
        simobserve(project=project,skymodel=inimage,indirection=indirection,
                   incenter=incenter,inwidth=inwidth,incell=incell,
                   setpointings=True,integration=tint,mapsize=mapsize,
                   pointingspacing=pointingspacing,
                   obsmode='int',sdantlist="",antennalist=antlist,
                   totaltime=totaltime,thermalnoise="",graphics="none")
        basename = project + "/" + self._get_data_prefix(antlist,project=project)
        shutil.move(basename+".skymodel", basename+".model")
        # only *.model, *.ms, and *.ptg.txt is necessary


########################################################################
#
# Tests on bad input parameter settings
#
class simobserve_badinputs(simobserve_unittest_base,unittest.TestCase):
    """
    Tests on bad input parameter setting
    """
    # global variables of the class
    inimage = "core5ps.skymodel"
    incomp = "core5ps.clist"
    indata = [inimage,incomp]
    # Limit pointings to make elapse time shorter
    tottime = "1" #number of visit
    mapsize = ["5arcsec","5arcsec"] # single pointing
    sdmapsize = ["40arcsec","40arcsec"]
    sdantlist = "aca.tp.cfg"

    # bad parameter values
    badsize = "-60arcsec"
    badfreq = "-3GHz"
    baddir = "J3000 19h00m00 -23d00m00"
    badname = "badname"
    badtime = "-100s"
    badquant = "5bad"
    badnum = -1.
    project = simobserve_unittest_base.thistask+"_bad"

    failmsg = "The task must throw exception"
    errmsg = "Unexpected exception was thrown: %s"
    
    # Reserved methods of unit tests
    def setUp(self):
        if (os.path.exists(self.project)):
            shutil.rmtree(self.project)
        
        for data in self.indata:
            if os.path.exists(data):
                os.system("rm -rf %s" % data)
            os.system("cp -r %s %s" % (self.datapath+data, data))

        # task must rethrow exception 
        glb['__rethrow_casa_exceptions'] = True

        default(simobserve)
        # Add new line for better reading (these tests always print errors).
        print ""

    def tearDown(self):
        glb['__rethrow_casa_exceptions'] = rethrow_org
        for data in self.indata:
            if os.path.exists(data):
                os.system("rm -rf %s" % data)
        if (os.path.exists(self.project)):
            shutil.rmtree(self.project)

    # Tests on invalid parameter sets
    def test_default(self):
        """Test Default parameter set. Neigher skymodel nor complist"""
        try:
            res = simobserve()
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("No sky input found")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)


    def test_noProject(self):
        """Test no project name"""
        project = ''
        try:
            res = simobserve(project=project)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("No such file or directory: ''")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)
        

    def testBad_skymodel(self):
        """Test bad skymodel name"""
        skymodel=self.badname
        try:
            res = simobserve(project=self.project,skymodel=skymodel)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("No sky input found")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)

    def test_notImage(self):
        """Test non-image skymodel"""
        skymodel=self.incomp
        try:
            res = simobserve(project=self.project,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             skymodel=skymodel)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Image %s cannot be opened; its type is unknown" % skymodel)
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)
        
    def testBad_inbright(self):
        """Test bad inbright"""
        inbright=self.badquant
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             inbright=inbright)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("invalid literal for float(): %s" % inbright)
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)

    def testBad_indirection(self):
        """Test bad indirection ('J3000' is defaulted to 'J2000')"""
        indirection=self.baddir
        res = simobserve(project=self.project,skymodel=self.inimage,
                         totaltime=self.tottime,mapsize=self.mapsize,
                         indirection=indirection)
        self.assertTrue(res)
        # Need to compare MS with one generated with J2000

    def testBad_incell(self):
        """Test bad incell"""
        incell=self.badquant
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             incell=incell)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find('Error in QuantumHolder::fromString with input string "%s": Illegal input units or format' % incell)
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)

    def testBad_incenter(self):
        """Test bad incenter"""
        # Negaitve and non-frequency quantity are ignored
        incenter=self.badfreq

        res = simobserve(project=self.project,skymodel=self.inimage,
                         totaltime=self.tottime,mapsize=self.mapsize,
                         incenter=incenter)
        self.assertTrue(res)
        # Need to compare MS with one generated with J2000
        
    def testBad_inwidth(self):
        """Test bad inwidth"""
        # Negaitve and non-frequency quantity are ignored
        inwidth=self.badfreq

        res = simobserve(project=self.project,skymodel=self.inimage,
                         totaltime=self.tottime,mapsize=self.mapsize,
                         inwidth=inwidth)
        self.assertTrue(res)
        # Need to compare MS with one generated with J2000

    def testBad_complist(self):
        """Test bad complist name"""
        complist=self.badname
        try:
            res = simobserve(project=self.project,complist=complist,
                             totaltime=self.tottime,mapsize=self.mapsize)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("No sky input found")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)
        
    def test_notComp(self):
        """Test non-components list complist"""
        complist=self.inimage
        try:
            res = simobserve(project=self.project,complist=complist,
                             totaltime=self.tottime,mapsize=self.mapsize)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("%s is non existant or is not a componentlist table" % complist)
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)

    def testBad_compwidth(self):
        """Test bad compwidth"""
        # not frequency
        compwidth="2arcsec"
        try:
            res = simobserve(project=self.project,complist=self.incomp,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             compwidth=compwidth)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Quantum::operator- unequal units 'GHz, 'arcsec'")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)
        
    def testBad_ptgfile(self):
        """Test bad ptgfile name"""
        setpointings=False
        ptgfile = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             setpointings=setpointings,ptgfile=ptgfile)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Can't find pointing file")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)

    def test_notPtgfile(self):
        """Test nonconforming ptgfile"""
        # Generate bad file
        fname = self.project+".badptg.txt"
        f = open(fname,"w")
        f.write("#This is bad pointing file\nsome bad data written")
        f.close()
        del f
        
        setpointings=False
        ptgfile = fname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             setpointings=setpointings,ptgfile=ptgfile)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("No valid lines found in pointing file")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)

    def testBad_integration(self):
        """Test bad integration"""
        integration = self.badtime
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             integration=integration)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find('Failed AlwaysAssert qIntTime.getValue("s")>=0')
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_direction(self):
        """Test bad direction ('J3000' is defaulted to 'J2000')"""
        direction = self.baddir

        res = simobserve(project=self.project,skymodel=self.inimage,
                         totaltime=self.tottime,mapsize=self.mapsize,
                         direction=direction)
        self.assertTrue(res)
        # Need to compare MS with one generated with J2000

    def testBad_mapsize(self):
        """Test bad mapsize"""
        setpointings=True
        mapsize = [self.badquant, self.badquant]
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,
                             setpointings=setpointings,mapsize=mapsize)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("can't interpret '%s' as a CASA quantity" % self.badquant)
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_maptype(self):
        """Test bad maptype"""
        maptype = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             maptype=maptype)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        


    def testBad_spacing(self):
        """Test bad pointingspacing"""
        pointingspacing = self.badquant
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             pointingspacing=pointingspacing)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("can't interpret '%s' as a CASA quantity" % pointingspacing)
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        


    def testBad_obsmode(self):
        """Test bad obsmode"""
        obsmode = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             obsmode=obsmode)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_antennalist(self):
        """Test bad antennalist name"""
        antennalist = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             antennalist=antennalist)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Couldn't find antennalist")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_caldirection(self):
        """Test bad caldirection ('J3000' is defaulted to 'J2000')"""
        caldirection = self.baddir

        res = simobserve(project=self.project,skymodel=self.inimage,
                         totaltime=self.tottime,mapsize=self.mapsize,
                         caldirection=caldirection)
        self.assertTrue(res)
        # Need to compare MS with one generated with J2000


        # simobserve so far does not catches this
    def testBad_calflux(self):
        """Test bad calflux"""
        caldirection = "J2000 19h00m00 -23d00m50"
        calflux = self.badquant
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             caldirection=caldirection,calflux=calflux)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("can't interpret '%s' as a CASA quantity" % calflux)
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_sdantlist(self):
        """Test bad sdantlist name"""
        obsmode = "sd"
        mapsize = self.sdmapsize
        sdantlist = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=mapsize,
                             obsmode=obsmode,sdantlist=sdantlist)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Couldn't find antennalist")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    # simobserve automatically defaults a bad ID number to 0.
    # therefore testing non-numeric 'sdant' here
    def testBad_sdant(self):
        """Test bad sdant (non-numeric sdant)"""
        obsmode = "sd"
        mapsize = self.sdmapsize
        sdantlist = self.sdantlist
        sdant = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=mapsize,
                             obsmode=obsmode,sdantlist=sdantlist,
                             sdant=sdant)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_refdate(self):
        """Test bad refdate"""
        obsmode = "sd"
        mapsize = self.sdmapsize
        sdantlist = self.sdantlist
        refdate = "05/21"
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=mapsize,
                             obsmode=obsmode,sdantlist=sdantlist,
                             refdate=refdate)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Invalid reference date")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_hourangle(self):
        """Test bad hourangle"""
        obsmode = "sd"
        mapsize = self.sdmapsize
        sdantlist = self.sdantlist
        hourangle = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=mapsize,
                             obsmode=obsmode,sdantlist=sdantlist,
                             hourangle=hourangle)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find('Error in QuantumHolder::fromString with input string "%s": Illegal input units or format' % hourangle)
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    # casapy crashes for totaltime < 0
    def testBad_totaltime(self):
        """Test bad totaltime"""
        obsmode = "sd"
        mapsize = self.sdmapsize
        sdantlist = self.sdantlist
        totaltime = self.badtime
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             mapsize=mapsize,
                             obsmode=obsmode,sdantlist=sdantlist,
                             totaltime=totaltime)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Negative totaltime is not allowed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_noisetype(self):
        """Test bad thermalnoise type"""
        thermalnoise = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             thermalnoise=thermalnoise)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_pwv(self):
        """Test bad user_pwv"""
        thermalnoise = 'tsys-atm'
        user_pwv = self.badnum
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             thermalnoise=thermalnoise,user_pwv=user_pwv)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_Tground(self):
        """Test bad t_ground"""
        thermalnoise = 'tsys-atm'
        t_ground = self.badnum
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             thermalnoise=thermalnoise,t_ground=t_ground)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_Tsky(self):
        """Test bad t_sky"""
        thermalnoise = 'tsys-manual'
        t_sky = self.badnum
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             thermalnoise=thermalnoise,t_sky=t_sky)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_tau0(self):
        """Test bad tau0"""
        thermalnoise = 'tsys-manual'
        tau0 = self.badnum
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             thermalnoise=thermalnoise,tau0=tau0)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        

    def testBad_leakage(self):
        """Test bad leakage"""
        leakage = self.badnum
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             leakage=leakage)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        
    
    def testBad_graphics(self):
        """Test bad graphics selection"""
        graphics = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
                             graphics=graphics)
            self.fail(self.failmsg)
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            msg =  self.errmsg % str(e)
            self.assertNotEqual(pos,-1,msg=msg)        
        
    
def suite():
    return [simobserve_noise, simobserve_badinputs]
#     return [simobserve_sdsky, simobserve_sdcomp, simobserve_sdcompsky,
#             simobserve_noise,simobserve_badinputs]


########################################################################
# How to generate input data
# 
# 1. Generate a component list with
#    a Gaussian (20 arcsec) + 5 point-like sources
# 
#     cl.open()
#     cl.rename("core5ps.clist")
#     cl.addcomponent(flux=[2.,0.,0.,0.],fluxunit="Jy",polarization="stokes",\
#                     dir='J2000 19h00m00 -23d00m00',shape="gaussian",\
#                     majoraxis="20arcsec",minoraxis="20arcsec",\
#                     positionangle="0deg",freq="345.0GHz",\
#                     spectrumtype="constant")
#     ### - Point like sources
#     ### Note: the last 2 sources are dummy 0Jy sources to make
#     ###       compskymodel larger (80arcsec^2)
#     directions = ['J2000 18h59m59.379275 -23d00m10.374590',
#                   'J2000 19h00m0.063002 -22d59m48.826670',
#                   'J2000 19h00m0.386715 -23d00m2.973792',
#                   'J2000 19h00m0.711507 -22d59m57.302818',
#                   'J2000 18h59m59.918234 -22d59m57.740534',
#                   'J2000 19h00m2.666667 -23d00m40.000000',
#                   'J2000 18h59m57.333333 -22d59m20.000000']
#     fluxes = [[0.004347,0.,0.,0.],[0.003808,0.,0.,0.],[0.004571,0.,0.,0.],
#               [0.008541,0.,0.,0.],[0.005158,0.,0.,0.],[0.,0.,0.,0.],[0.,0.,0.,0.]]
#     for idx in range(len(directions)):
#         cl.addcomponent(flux=fluxes[idx],fluxunit="Jy",shape="gaussian",\
#                         majoraxis="0.5arcsec",minoraxis="0.5arcsec",\
#                         positionangle="0deg",\
#                         polarization="stokes",dir=directions[idx],\
#                         freq="345.0GHz",spectrumtype="constant")
#     cl.done()
#
# 2. Generate a CASA image of the components list using simobserve
#     simobserve(project="simCore5ps",complist='core5ps.clist',\
#                compwidth="10MHz",direction='J2000 19h00m00 -23d00m00',\
#                obsmode='int',mapsize=['80arcsec','80arcsec'],totaltime='1')
#  simobserve generates a skymodel image,
#    "$project/$project.alma_out10.compskymodel" [1873,1873] pixels
#  This image is renamed as "core5ps.skymodel" and used in skymodel tests
#  
# 3. Remove dummy components from list
#
#       cl.open("core5ps.clist")
#       numcore = cl.length()
#       cl.remove([numcore-2, numcore-1])
#       cl.close()
# 4. Generate ptgfiles
#    [I] TP pointing
#     simobserve(project="simSDPtgCore5ps",complist='core5ps.clist',\
#                compwidth="10MHz",direction='J2000 19h00m00 -23d00m00',\
#                maptype='square',mapsize=['40arcsec','40arcsec'],\
#                totaltime='1',obsmode='',sdantlist='aca.tp.cfg',antennalist='')
#     simobserve generates a skymodel image,
#       "$project/$project.alma_out10.ptg.txt" with 16 pointings
#     This image is renamed as "core5ps.sd40asec.ptg.txt" and used in skymodel tests
#    [II] INT pointing
#     simobserve(project="simINTPtgCore5ps",complist='core5ps.clist',\
#                compwidth="10MHz",direction='J2000 19h00m00 -23d00m00',\
#                maptype='ALMA',mapsize=['5arcsec','5arcsec'],totaltime='1',\
#                obsmode='',sdantlist='',antennalist='alma.out01.cfg')
#     simobserve generates a skymodel image,
#       "$project/$project.alma_out10.ptg.txt" with single pointing
#     This image is renamed as "core5ps.single.ptg.txt" and used in skymodel tests
########################################################################
