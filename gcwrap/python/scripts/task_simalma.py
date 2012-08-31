from taskinit import *
from simutil import *
import os
import re
import glob
import numpy

def simalma(
    project=None,
    skymodel=None, inbright=None, indirection=None, incell=None, 
    incenter=None, inwidth=None,
    complist=None, compwidth=None,
    ########
    setpointings=None,
    ptgfile=None,
    integration=None, direction=None, mapsize=None, 
    #maptype=None,
    #pointingspacing=None,
    #caldirection=None, calflux=None, 
    #observe=None, 
    #refdate=None,
    antennalist=None,
    hourangle=None, 
    totaltime=None,
    #sdantlist=None, sdant=None,
    ###
    acaratio = None,
    acaconfig = None,
    ###
    #thermalnoise=None,
    #user_pwv=None, t_ground=None, t_sky=None, tau0=None,
    pwv=None,
    #seed=None,
    #leakage=None,
    image=None,
    #vis=None, modelimage=None,
    imsize=None, imdirection=None,cell=None,
    niter=None, threshold=None,
    #weighting=None, mask=None, outertaper=None, stokes=None,     
    #analyze=None, 
    #showarray=None, showuv=None, showpsf=None, showmodel=None, 
    #showconvolved=None, showclean=None, showresidual=None, showdifference=None, 
    #showfidelity=None,
    graphics=None,
    verbose=None, 
    overwrite=None,
    async=False):

    try:

        casalog.origin('simalma')
        if verbose: casalog.filter(level="DEBUG2")
    
        a = inspect.stack()
        stacklevel = 0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel = k
        myf = sys._getframe(stacklevel).f_globals
        
        # Parameter verifications
        # antennalist should be one of ALMA config.
        if antennalist.upper().find("ALMA") < 0:
            raise ValueError, "ALMA configuration should be selected in antennalist."
        #if not os.path.exists(project):
        #    os.mkdir(project)

        # create the utility object:
        myutil = simutil(direction)
        if verbose: myutil.verbose = True
        msg = myutil.msg
        msg("##### simalma currently does not invoke actual simulation ###", priority="warn")

        # Fixed parameters
        nyquist = 0.48113 ## Nyquist
        maptype_bl = 'ALMA'
        maptype_tp = 'square'
        pbgridratio_tp = 0.36
        refdate = '2013/05/21'
        caldirection = ""
        calflux = "0Jy"
        tpantid = 0.
        t_ground = 270.
        if pwv > 0:
            thermalnoise = "tsys-atm"
        else:
            thermalnoise = ""
        #seed = 11111
        leakage = 0.
        weighting = "briggs"

        # Operation flags
        addnoise = (thermalnoise != '')
        rectpoints = setpointings and \
                     ((direction == "") or \
                      myutil.isdirection(direction,halt=False))
        # test for cycle 1
        q = re.compile('.*CYCLE.?1.?;(.*)')
        isC1 = q.match(antennalist.upper())

        # antennalist of ACA and TP
        antlist_tp = "aca.tp.cfg"
        if isC1:
            antlist_aca = "aca_cycle1.cfg"
        else:
            antlist_aca = "aca.i.cfg"
        # Resolve prefixes of simulation data
        pref_bl = get_data_prefix(antennalist, project)
        pref_aca = get_data_prefix(antlist_aca, project)
        pref_tp = get_data_prefix(antlist_tp, project)
        ptgfile_bl = project+"/"+pref_bl+".ptg.txt"

        # Either skymodel or complist should exists
        if type(skymodel) == type([]):
            skymodel = skymodel[0]
        skymodel = skymodel.replace('$project',pref_bl)
    
        if type(complist) == type([]):
            complist = complist[0]

        if((not os.path.exists(skymodel)) and (not os.path.exists(complist))):
            raise ValueError, "No sky input found.  At least one of skymodel or complist must be set."

        ###########################
        # Get model_size and model_center
        msg("Step 0: Preporcessing is to be done here.", priority="warn")
        
        ############################################################
        # ALMA-BL simulation
        # simalma always does model generation, pointing calc. and observation
        # BL mapsize should be 1 PB smaller than skymodel when using ACA
        calcsize = (isinstance(mapsize,list) and len(mapsize[0]) == 0) or \
                   len(mapsize) == 0
        #if acaratio > 0 and rectpoints and calcsize:
        #    mapx = qa.sub(model_size[0], PB12)
        #    mapy = qa.sub(model_size[1], PB12)
        #    mapsize_bl = [qa.tos(mapx), qa.tos(mapy)]
        #else:
        #    mapsize_bl = mapsize
        mapsize_bl = mapsize

        step = 1
        msg("Step %d: BL simobserve is to be done here." % step, priority="warn")

        if acaratio > 0:
            msg("#### ACA is requested #####", priority="warn")
            ########################################################
            # ACA-7m simulation
            tottime_aca = qa.tos(qa.mul(totaltime, acaratio))
            # Same pointings as BL
            ptgfile_aca = ptgfile_bl
            step += 1
            msg("Step %d: ACA simobserve is to be done here." % step, priority="warn")

            ########################################################
            # ACA-TP  simulation
            step += 1
            msg("Step %d: TP simobserve is to be done here." % step, priority="warn")
            

        ################################################################
        # Imaging
        if image:
            msg("#### imaging is requested #####", priority="warn")
            modelimage = ""
            if acaratio > 0:
                ########################################################
                # Image ACA-7m + ACA-TP
                # resolve modelimage name for BL imaging
                step += 1
                msg("Step %d: ACA + TP simanalyze is to be done here." % step, priority="warn")
                
                modelimage = pref_aca+".image"
            ############################################################
            # Image ALMA-BL
            step += 1
            msg("Step %d: BL + ACA image simanalyze is to be done here." % step, priority="warn")

    except TypeError, e:
        finalize_tools()
        msg("task_simalma -- TypeError: %s" % e,priority="error")
        return False
    except ValueError, e:
        finalize_tools()
        #print "task_simobserve -- OptionError: ", e
        msg("task_simalma -- OptionError: %s" % e,priority="error")
        return False
    except Exception, instance:
        finalize_tools()
        #print '***Error***',instance
        msg("task_simalma -- Exception: %s" % instance,priority="error")
        return False
    return True

def finalize_tools():
    if ia.isopen(): ia.close()
    sm.close()
    #cl.close()   # crashes casa

def get_data_prefix(cfgname, project=""):
    if str.upper(cfgname[0:4]) == "ALMA":
        foo=cfgname.replace(';','_')
    else:
        foo = cfgname
        foo=foo.replace(".cfg","")
        sfoo=foo.split('/')
        if len(sfoo)>1: foo=sfoo[-1]
    return project+"."+foo

