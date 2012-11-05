import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

import numpy
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


# ###
# # Test single dish skymodel only simulations
# ###
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
    
#     def testSky_sdAtmNoise(self):
#         """Test skymodel simulation: only observation (SD)"""
#         skymodel = self.refmod
#         setpointings = False
#         obsmode = "sd"
#         thermalnoise = "tsys-atm"
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          setpointings=setpointings,ptgfile=ptgfile,
#                          obsmode=obsmode,thermalnoise=thermalnoise)
    
#     def testSky_sdManNoise(self):
#         """Test skymodel simulation: only observation (SD)"""
#         skymodel = self.refmod
#         setpointings = False
#         obsmode = "sd"
#         thermalnoise = "tsys-manual"
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
    
#     def testSky_intAtmNoise(self):
#         """Test skymodel simulation: only observation (SD)"""
#         skymodel = self.refmod
#         setpointings = False
#         obsmode = "int"
#         thermalnoise = "tsys-atm"
#         res = simobserve(project=self.project,skymodel=skymodel,
#                          setpointings=setpointings,ptgfile=ptgfile,
#                          obsmode=obsmode,thermalnoise=thermalnoise)
    
#     def testSky_intManNoise(self):
#         """Test skymodel simulation: only observation (INT)"""
#         skymodel = self.refmod
#         setpointings = False
#         obsmode = "int"
#         thermalnoise = "tsys-manual"
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

    
    

# ###
# # Test components list only simulations
# ###
# class simobserve_comp(simobserve_unittest_base,unittest.TestCase):
#     """
#     Test components list simulations
#     """

# ###
# # Test skymodel + components list simulations
# ###
# class simobserve_skycomp(simobserve_unittest_base,unittest.TestCase):
#     """
#     Test skymodel + components list simulations
#     """


###
# Tests on bad input parameter settings
###
class simobserve_badinputs(simobserve_unittest_base,unittest.TestCase):
    """
    Tests on bad input parameter setting
    """
    # global variables of the class
    inimage = "core3ps.skymodel"
    incomp = "core3ps.clist"
    ptg = "core3ps.ptg.txt"
    indata = [inimage,incomp,ptg]
    # Limit pointings to make elapse time shorter
    tottime = "250s"
    mapsize = ["40arcsec","40arcsec"]
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
        sdantlist = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
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
        sdantlist = self.sdantlist
        sdant = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
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
        sdantlist = self.sdantlist
        refdate = "05/21"
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
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
        sdantlist = self.sdantlist
        hourangle = self.badname
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             totaltime=self.tottime,mapsize=self.mapsize,
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
        sdantlist = self.sdantlist
        totaltime = self.badtime
        try:
            res = simobserve(project=self.project,skymodel=self.inimage,
                             mapsize=self.mapsize,
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
    return [simobserve_badinputs]
#     return [simobserve_badinputs,simobserve_sdsky,
#             simobserve_sdcomp,simobserve_sdcompsky]


########################################################################
# How to generate input data
# 
# 1. Generate a component list with
#    a Gaussian (20 arcsec) + 3 point-like sources
# 
#     cl.open()
#     cl.rename("core3ps.clist")
#     cl.addcomponent(flux=[20.,0.,0.,0.],fluxunit="Jy",polarization="stokes",\
#                     dir='J2000 19h00m00 -23d00m00',shape="gaussian",\
#                     majoraxis="20arcsec",minoraxis="20arcsec",\
#                     positionangle="0deg",freq="345.0GHz",\
#                     spectrumtype="constant")
#     ### - Point like sources
#     ### Note: the last 2 sources are dummy 0Jy sources to make
#     ###       compskymodel larger (80arcsec^2)
#     directions = ['J2000 18h59m59.698346 -22d59m48.749605',
#                   'J2000 19h00m1.102395 -22d59m44.919331',
#                   'J2000 18h59m59.062324 -23d00m13.627345',
#                   'J2000 19h00m2.666667 -23d00m40.000000',
#                   'J2000 18h59m57.333333 -22d59m20.000000']
#     fluxes = [[10.,0.,0.,0.],[5.,0.,0.,0.],[8.,0.,0.,0.],
#               [0.,0.,0.,0.],[0.,0.,0.,0.]]
#     for idx in range(len(directions)):
#         cl.addcomponent(flux=fluxes[idx],fluxunit="Jy",shape="gaussian",\
#                         majoraxis="0.5arcsec",minoraxis="0.5arcsec",\
#                         positionangle="0deg",\
#                         polarization="stokes",dir=directions[idx],\
#                         freq="345.0GHz",spectrumtype="constant")
#     cl.done()
#
# 2. Generate a CASA image of the components list using simobserve
#     simobserve(project="simCore3ps",complist='core3ps.clist',\
#                compwidth="2GHz",direction='J2000 19h00m00 -23d00m00',\
#                mapsize=['80arcsec','80arcsec'],totaltime='1300s')
#  simobserve generates a skymodel image,
#    "$project/$project.alma_out10.compskymodel" [1955x1955] pixels
#  This image is renamed as "core3ps.skymodel" and used in skymodel tests
#  
# 3. Remove dummy components from list
#
#       cl.open("core3ps.clist")
#       numcore = cl.length()
#       cl.remove([numcore-2, numcore-1])
#       cl.close()
# 4. Generate ptgfile
#     simobserve(project="simPtgCore3ps",complist='core3ps.clist',\
#                compwidth="2GHz",direction='J2000 19h00m00 -23d00m00',\
#                mapsize=['50arcsec','50arcsec'],totaltime='1300s',obsmode='')
#  simobserve generates a skymodel image,
#    "$project/$project.alma_out10.ptg.txt" with 43 pointings
#  This image is renamed as "core3ps.ptg.txt" and used in skymodel tests
########################################################################
