import os
import shutil
import re
import glob
import pdb

from taskinit import *
from simutil import *
from simobserve import simobserve
from simanalyze import simanalyze
from feather import feather
from concat import concat
from imregrid import imregrid
from immath import immath
from sdimaging import sdimaging

def simalma(
    project=None,
    dryrun=None,
    skymodel=None, inbright=None, indirection=None, incell=None, 
    incenter=None, inwidth=None,
    complist=None, compwidth=None,
    ########
    setpointings=None,
    ptgfile=None,
    integration=None, direction=None, mapsize=None, 
    antennalist=None,
    hourangle=None, 
    totaltime=None,
    ###
    tpnant = None,
    tptime = None,
    ###
    pwv=None,
    image=None,
    imsize=None, imdirection=None,cell=None,
    niter=None, threshold=None,
    graphics=None,
    verbose=None, 
    overwrite=None,
    async=False):

    # Collect a list of parameter values to save inputs
    in_params =  locals()

    #-------------------------
    # Create the utility object
    myutil = simutil(direction)
    if verbose: myutil.verbose = True
    msg = myutil.msg
    msg2 = myutil.msg2
    from simutil import is_array_type


    try:

        ###########################
        # preliminaries
        ###########################
        
        # Predefined parameters 
        pbcoeff = 1.13 ##  PB defined as pbcoeff*lambda/d
        nyquist = 0.5/1.13 ## Nyquist spacing = PB*nyquist
        maptype_int = 'ALMA'
        maptype_tp = 'square'

        # time ratios for 12extended, 12compact,7m, and TP:
        default_timeratio = [1,0.5,4,8]

        # pbgridratio_tp = 0.25  # ->  would be gridratio_tp = 1/3.4
        # the grid spacing is defined in terms of lambda/d times these factors
        # ALMA OT uses lambda/d/sqrt(3)
        gridratio_int = 1./pl.sqrt(3) # 0.5 is nyquist 
        gridratio_tp  = 1./2.5 

        # number of 12m primary beams to pad the total power image during
        # the gridding stage (i.e. even larger pad than the padding 
        # added for the observation). 
        tppad_npb = 2. 
        
        # weight of 7m data relative to 12m data
        weightratio_7_12 = 0.34


        # the scale factor to correct expected Gauss PB size to empirical simPB
        simpb_factor = 0.96
        caldirection = ""
        calflux = "0Jy"
        tpantid = 0
        t_ground = 270.
        if pwv > 0:
            thermalnoise = "tsys-atm"
        else:
            thermalnoise = ""
        leakage = 0.
        weighting = "briggs"

        antlist_tp_default="aca.tp.cfg"

        #----------------------------------------
        # Save outputs in a directory called "project"
        fileroot = project
        # simalma is not supposed to run multiple times.
        if os.path.exists(fileroot):
            infomsg = "Project directory, '%s', already exists." % fileroot
            if overwrite:
                casalog.post(infomsg)
                casalog.post("Removing old project directory '%s'" % fileroot)
                shutil.rmtree(fileroot)
            else:
                raise Exception, infomsg

        if not os.path.exists(fileroot):
            os.mkdir(fileroot)

        concatname=fileroot+"/"+project+".concat"

        #-------------------------
        # set up reporting to file, terminal, logger
        casalog.origin('simalma')
        if verbose:
            casalog.filter(level="DEBUG2")
            v_priority = "WARN"
            # print report messages to the terminal and logger
            report_out = ['t','l','f'] 
        else:
            v_priority = "INFO"
            report_out = ['f'] # print report just to the file

        myutil.msg2_out=report_out

        simobserr = "simalma caught an exception in task simobserve"
        simanaerr = "simalma caught an exception in task simanalyze"

        # open report file
        myutil.reportfile=fileroot+"/"+project+".simalma.report.txt"
        myutil.openreport()



        #----------------------------------------
        # Get globals to call saveinputs()
        a = inspect.stack()
        stacklevel = 0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel = k
        myf = sys._getframe(stacklevel).f_globals

        # Save input parameters of simalma
        saveinputs = myf['saveinputs']
        saveinputs('simalma',fileroot+"/"+project+".simalma.last",
                   myparams=in_params)



        # filename parsing of cfg file here so that the project filenames 
        # can contain the cfg
        repodir = os.getenv("CASAPATH").split(' ')[0] + "/data/alma/simmos/"




        #--------------------------
        # format mapsize
        if not is_array_type(mapsize):
            mapsize = [mapsize, mapsize]
        elif len(mapsize) < 2:
            mapsize = [mapsize[0], mapsize[0]]

        #---------------------------
        # Operation flags
        addnoise = (thermalnoise != '')
        # Rectangle setup mode
        multiptg = (not setpointings) \
                   or (is_array_type(direction) and len(direction) > 1)
        rectmode = (not multiptg)

        # Use full model image as a mapsize = ["", ""]
        fullsize = (len(mapsize[0]) == 0) or (len(mapsize[1]) == 0)
















        ###########################
        # analyze input sky model
        ###########################

        #----------------------------
        # Either skymodel or complist should exist
        if is_array_type(skymodel):
            if len(skymodel)>1:
                msg("You have given more than one skymodel - simalma will only use the first one, "+skymodel[0],priority="WARNING")
            skymodel = skymodel[0]
        skymodel = skymodel.replace('$project',project)

        if is_array_type(complist):
            if len(complist)>1:
                msg("You have given more than one componentlist - simalma will only use the first one, "+componentlist[0],priority="WARNING")
            complist = complist[0]

        if((not os.path.exists(skymodel)) and (not os.path.exists(complist))):
            raise ValueError, "No sky input found.  At least one of skymodel or complist must be set."


        #-------------------------
        # Get model_size and model_center
        # TODO: check if outmodel==inmodel works (just collect info)
        msg2("----------------------------------------------------")
        if os.path.exists(skymodel):
            components_only=False
            outmodel = fileroot+"/"+project+"temp.skymodel"
            model_vals = myutil.modifymodel(skymodel, outmodel, inbright,
                                            indirection, incell, incenter,
                                            inwidth, -1, False)
            shutil.rmtree(outmodel)
            model_refdir =  model_vals[0]
            model_cell = model_vals[1]
            model_size = model_vals[2]
            model_nchan = model_vals[3]
            model_center = model_vals[4]
            del model_vals
            msg2("You will be simulating from sky model image "+skymodel)
            msg2("  pixel(cell) size = "+qa.tos(model_cell[0]))
            msg2("  model_refdir size = "+qa.tos(model_size[0]))
            # XXX TODO add more 

        else:
            # XXX TODO make sure components AND image work here
            components_only=True
            compdirs = []
            cl.open(complist)
            for i in range(cl.length()):
                compdirs.append(myutil.dir_m2s(cl.getrefdir(i)))

            model_refdir, coffs = myutil.average_direction(compdirs)
            model_center = cl.getspectrum(0)['frequency']['m0']
            cmax = 0.0014 # ~5 arcsec
            for i in range(coffs.shape[1]):
                xc = pl.absolute(coffs[0,i])  # offsets in deg
                yc = pl.absolute(coffs[1,i])
                cmax = max(cmax, xc)
                cmax = max(cmax, yc)
            model_size = ["%fdeg" % (2*cmax), "%fdeg" % (2*cmax)]
            cl.done()
            model_cell = None
            model_nchan = 1
            del compdirs, coffs, xc, yc, cmax



        #-----------------------------------
        # determine mapsize - copied code from simobserve
        # if the user has not input a map size, then use model_size
        if len(mapsize) == 0:
            mapsize = model_size
            if verbose: msg("setting map size to "+str(model_size))
        else:
             if type(mapsize) == type([]):
                 if len(mapsize[0]) == 0:
                     mapsize = model_size
                     if verbose: msg("setting map size to "+str(model_size))

        if components_only:
            # if map is greater tham model (defined from components) 
            # then use mapsize as modelsize
            if type(mapsize) == type([]):
                map_asec = qa.convert(mapsize[0],"arcsec")['value']
            else:
                map_asec = qa.convert(mapsize,"arcsec")['value']
            if type(model_size) == type([]):
                mod_asec = qa.convert(model_size[0],"arcsec")['value']
            else:
                mod_asec = qa.convert(model_size,"arcsec")['value']
            if map_asec>mod_asec:
                model_size=["%farcsec" % map_asec,"%farcsec" % map_asec]











        ###########################
        # Calculate 12-m PB and grid spacing for int and tp
        ###########################

        Dant = 12.
        wave_length = 0.2997924/ qa.convert(qa.quantity(model_center),'GHz')['value']
        # (wave length)/D_ant in arcsec
        lambda_D = wave_length / Dant * 3600. * 180 / pl.pi
        PB12 = qa.quantity(lambda_D*pbcoeff, "arcsec")
        # Correction factor for PB in simulation
        # (PSF of simulated image is somehow smaller than PB12)
        PB12sim = qa.mul(PB12, simpb_factor)

        msg("PB size: %s" % (qa.tos(PB12)), origin="simalma", priority='DEBUG2')

        # Pointing spacing of observations - define in terms of lambda/d
        # to avoid ambiguities in primary beam definition. 
        # it would be best for simobserve to accept the shorthand "LD"
        # but until it does that, we'll divide by pbcoeff and use "PB":
        # this is a fragile solution since it depends on pbcoeff being
        # the same in simalma and simobserve.
        pointingspacing = str(gridratio_int/pbcoeff)+"PB"
        ptgspacing_tp = qa.tos(qa.mul(PB12sim, (gridratio_tp/pbcoeff))) 










        ###########################
        # analyze antennalist(s) 
        ###########################

        if type(antennalist)==type(" "):
            antennalist=[antennalist]

        # number of arrays/configurations to run:
        nconfigs=len(antennalist)
        msg2("",out=['f']) # blank line in file
        msg2("----------------------------------------------------")
        msg2("You are requesting %i configurations:" % nconfigs,origin="simalma.report")
        msg2("",out=['f']) # blank line in file
        
        configtypes=[] # ALMA, ACA, or ALMASD (latter is special case)
        cycles=[] 
        resols=[]
        tp_inconfiglist=False

         # check filename consistency, get resolution, etc        
        for configfile in antennalist:

            isalma=0

            # antennalist should contain ALMA or ACA
            if configfile.find(";")>=0:
                telescopename="BYRES"
                configparms=configfile.split(";")
                res=configparms[-1]
                res_arcsec=-1
                if myutil.isquantity(res):
                    if qa.compare(res,"arcsec"):
                        res_arcsec=qa.convert(res,"arcsec")['value']
                        
                if res_arcsec>0:
                    resols.append(res_arcsec)
                else:
                    msg2("simalma cannot interpret the antennalist entry "+configfile+" as desired array and resolution e.g. ALMA;0.5arcsec",priority="ERROR",out=['l','t','f'])

            else:
                # we can only verify explicit files right now, not 
                # configs specified as "ALMA;0.5arcsec"
                #
                configfile_short = (configfile.split("/"))[-1]
                # Search order is fileroot/ -> specified path -> repository
                if len(configfile) > 0:
                    if os.path.exists(fileroot+"/"+configfile):
                        configfile = fileroot + "/" + configfile
                    elif not os.path.exists(configfile) and \
                            os.path.exists(repodir+configfile):
                        configfile = repodir + configfile
                   # Now make sure the configfile exists
                    if not os.path.exists(configfile):
                        msg2("Couldn't find configfile: %s" % configfile, priority="error")
                    stnx, stny, stnz, stnd, padnames, nant, telescopename = myutil.readantenna(configfile)
                    if telescopename=="ALMASD":
                        resols.append(qa.convert(PB12,'arcsec')['value'])
                    else:
                        psfsize = myutil.approxBeam(configfile,qa.convert(qa.quantity(model_center),'GHz')['value'])
                        resols.append(psfsize) # FWHM in arcsec

            q = re.compile('CYCLE\d?\d')
            isCycle = q.search(configfile_short.upper())
            if isCycle:
                whatCycle = isCycle.group()[-1] # will break if cycle>9
                cycles.append(whatCycle)
            else:
                cycles.append(-1) # -1 is unknown; defaults to full ALMA


            if configfile_short.upper().find("ALMA") >= 0:
                if telescopename=="ALMA" or telescopename=="BYRES":
                    configtypes.append("ALMA")
                    isalma=isalma+1
                else:
                    msg2("Inconsistent antennalist entry "+configfile_short+" has ALMA in the name but not set as the observatory",priority="error")
            if configfile_short.upper().find("ACA") >= 0:
                if telescopename=="ACA" or telescopename=="BYRES":
                    configtypes.append("ACA")
                    isalma=isalma+1
                elif telescopename=="ALMASD" or telescopename=="BYRES":
                    tp_inconfigfile=True
                    configtypes.append("ALMASD")
                    isalma=isalma+1
                else:
                    msg2("Inconsistent antennalist entry "+configfile_short+" has ACA in the name but the observatory is not ACA or ALMASD",priority="error")
                #if configfile.upper().find("CYCLE0")


            if isalma==0:
                s="simalma can't accept antennalist entry "+configfile_short+" because it is neither ALMA nor ACA (in the name and the observatory in the file)"
                msg2(s,origin="simalma.report",priority="error")
#                raise ValueError, s  # not ness - msg2 raises the exception
            if isalma==2:
                s="simalma doesn't understand your antennalist entry "+configfile_short
                msg2(s,origin="simalma.report",priority="error")
#                raise ValueError,s
            

        #-----------------------------
        # total power parameter:
        tptime_min=0.
        if myutil.isquantity(tptime):
            if qa.compare(tptime,'s'):
                tptime_min=qa.convert(tptime,'min')['value']


        #-----------------------------
        # is there a mix of cycle0,1,2,etc,full?
        whatCycle=cycles[0]
        cyclesInconsistent=False
        for i in range(nconfigs):
            if cycles[i]!=whatCycle:
                cyclesInconsistent=True


        #-----------------------------
        # exposure time parameter totaltime
        totaltime_min=[]
        if len(totaltime)==1:
#        if type(totaltime)==type(" "):
            # scalar input - use defaults
            totaltime_min0=0.
            myutil.isquantity(totaltime[0])
            if qa.compare(totaltime[0],'s'):
                totaltime_min0=qa.convert(totaltime[0],'min')['value']
            else:
                raise ValueError,"Can't convert totaltime parameter "+totaltime[0]+" to minutes"
            totaltime_min=pl.zeros(nconfigs)
            # sort by res'l - TP could still be on here
            resols=pl.array(resols)
            intorder=resols.argsort()
            # assume the scalar user input refers to the highest res 12m
            totaltime_min[intorder[0]]=totaltime_min0
            for j in intorder[1:]:
                if configtypes[j]=='ALMA':
                    # lower res 12m
                    totaltime_min[j]=totaltime_min0*default_timeratio[1]
                elif configtypes[j]=='ACA':
                    # 7m
                    totaltime_min[j]=totaltime_min0*default_timeratio[2]
                elif configtypes[j]=='ALMASD':
                    # tp
                    totaltime_min[j]=totaltime_min0*default_timeratio[3]
                else:
                    raise Exception,"configuration types = "+str(configtypes)
        else:
            for time in totaltime:
                myutil.isquantity(time)
                if qa.compare(time,'s'):
                    time_min=qa.convert(time,'min')['value']
                else:
                    raise ValueError,"Can't convert totaltime parameter "+time+" to minutes"
                totaltime_min.append(time_min)
                
        if len(totaltime_min)!=len(antennalist):
            raise Exception,"totaltime must either be the same length vector as antennalist or a scalar"



        #-----------------------------
        # print out what's requested.  
        antlist_tp=None
        for i in range(nconfigs):
            configfile=antennalist[i]
            msg2("  "+configfile+":")

            if configtypes[i]=="ALMA":                
                msg2("    12m ALMA array")
            elif configtypes[i]=="ACA":
                msg2("    7m ACA array")                
            elif configtypes[i]=="ALMASD":
                msg2("   12m total power observation")
                if tpnant>0:
                    msg2("   This antennalist entry will be ignored in favor of the tpnant requesting %d total power antennas" % tpnant)
                    antlist_tp=antlist_tp_default
                    if tptime_min>0:
                        msg2("    The total map integration time will be %s minutes as per the tptime parameter." % tptime_min)
                    else:
                        tptime_min=totaltime_min[i]
                        msg2("    The total map integration time will be %s minutes as per the totaltime parameter." % tptime_min)
                else:
                    # Note: assume the user won't put in >1 TP antlist.
                    msg2("   Note: it is preferred to specify total power with the tpnant,tptime parameters so that one can also specify the number of antennas.")
                    msg2("   Specifying the total power array as one of the antenna configurations will only allow you to use a single antenna for the simulation")
                    msg2("   simalma will proceed with a single total power antenna observing for %d minutes." % totaltime_min[i])
                    tpnant=1
                    tptime_min=totaltime_min[i]
                    antlist_tp=configfile                

            # print cycle and warn if mixed
            if cycles[i]>'-1':
                msg2("    Cycle "+cycles[i]+" configuration")
            else:
                msg2("    Full ALMA configuration")
            if cyclesInconsistent:
                msg2("    WARNING: Your choices of configurations mix different cycles and/or Full ALMA.  Assuming you know what you want.")
                

            # print resolution for INT, and warn if model_cell too large
            if configtypes[i]!='ALMASD':
                msg2("    approximate synthesized beam FWHM = %f arcsec" % resols[i])
                if is_array_type(model_cell):
                    cell_asec=qa.convert(model_cell[0],'arcsec')['value']
                else:
                    cell_asec=qa.convert(model_cell,'arcsec')['value']    
                if cell_asec > 0.2*resols[i]:
                    if cell_asec >= resols[i]:
                        # XXX if not dryrun raise exception here
                        msg2("   ERROR: your sky model cell %f arcsec is too large to simulate this beam.  Simulation will fail" % cell_asec)
                    else:
                        msg2("   WARNING: your sky model cell %f arcsec does not sample this beam well. Simulation may be unreliable or clean may fail." % cell_asec)

                msg2("    Observation time = %f min" % totaltime_min[i])
                if totaltime_min[i]>360:
                    msg2("    WARNING: this is longer than 6hr - simalma won't (yet) split the observation into multiple nights, so your results may be unrealistic")


        if not tp_inconfiglist and tpnant>0:
            msg2("")
            if tptime_min>0:
                msg2("You are also requesting Total Power observations:")
                msg2("    %d antennas for %f minutes" % (tpnant,tptime_min))
            else:
                msg2("You have requested %d total power antennas (tpnant), but no finite integration (tptime) -- check your inputs; no Total Power will be simulated.")



        # remove tp from configlist
        antennalist=pl.array(antennalist)
        configtypes=pl.array(configtypes)
        totaltime_min=pl.array(totaltime_min)
        resols=pl.array(resols)            
        z=pl.where(configtypes!='ALMASD')[0]
        antennalist=antennalist[z]
        configtypes=configtypes[z]
        totaltime_min=totaltime_min[z]
        resols=resols[z]

                                 

# TODO check model_size against mapsize  - separately after this?




                             
                             








#        ###########################
#        # Resolve prefixes of simulation data (as defined in 
#        # simobserve and simanalyze)
#        
#        pref_bl = get_data_prefix(antennalist, project)
#        pref_aca = get_data_prefix(antlist_aca, project)
        # Resolve output names (as defined in simobserve and simanalyze)
#        if addnoise:
#            msname_bl = pref_bl+".noisy.ms"
#            msname_aca = pref_aca+".noisy.ms"
#            msname_tp = pref_tp+".noisy.sd.ms"
#            #imagename_aca = pref_aca+".noisy.image"
#        else:
#            msname_bl = pref_bl+".ms"
#            msname_aca = pref_aca+".ms"
#            msname_tp = pref_tp+".sd.ms"
#            #imagename_aca = pref_aca+".image"
#        
#        imagename_tp = project+".sd.image"
#        imagename_int = project+".concat.image"
#        msname_concat = project+".concat.ms"
#
        combimage = project+".feather.image"
        simana_file = project+".simanalyze.last"




        ############################################################
        # run simobserve 
        msg2("")
        msg2("----------------------------------------------------")
        msg2("simalma calls simobserve to simulate each component:")

        # sort by res'l - save the ptgfile from the max res'l config 
        # for use in TP pointing definition.
        resols=pl.array(resols)
        intorder=resols.argsort()
        pref_bl=get_data_prefix(antennalist[intorder[0]],project)
        ptgfile_bl=fileroot+"/"+pref_bl+".ptg.txt"

        step = 1

        for i in range(nconfigs):
            if configtypes[i]=="ALMA":                
                s="12m ALMA array"
            elif configtypes[i]=='ACA':
                s="7m ACA array"
            else:
                s="12m total power map"
                
            if configtypes[i]!='ALMASD':

                msg2("",out=['f'])
                if dryrun:
                    outv=['f','l']
                else:
                    outv=['f','l','t']
                msg2("="*60, origin="simalma", priority="warn",out=outv)
                msg2((" Step %d: simulating " % step)+s, origin="simalma", priority="warn",out=outv)
                msg2("="*60, origin="simalma", priority="warn",out=outv)

                # filename prefixes
                pref = get_data_prefix(antennalist[i], project)

                obsmode_int = 'int'
                ptgfile_int = fileroot+"/"+pref+".ptg.txt"

                task_param = {}
                task_param['project'] = project
                task_param['skymodel'] = skymodel
                task_param['inbright'] = inbright
                task_param['indirection'] = indirection
                task_param['incell'] = incell
                task_param['incenter'] = incenter
                task_param['inwidth'] = inwidth
                task_param['complist'] = complist
                task_param['compwidth'] = compwidth
                task_param['setpointings'] = setpointings
                task_param['ptgfile'] = ptgfile
                task_param['integration'] = integration
                task_param['direction'] = direction
                task_param['mapsize'] = mapsize
                task_param['maptype'] = maptype_int
                # this is approriate for 7m or 12m since its in terms of PB
                task_param['pointingspacing'] = pointingspacing
                task_param['caldirection'] = caldirection
                task_param['calflux'] = calflux
                task_param['obsmode'] = obsmode_int
                task_param['hourangle'] = hourangle
                task_param['totaltime'] = "%fmin" % totaltime_min[i]
                task_param['antennalist'] = antennalist[i]
                task_param['sdantlist'] = ""
                task_param['sdant'] = 0
                task_param['thermalnoise'] = thermalnoise
                task_param['user_pwv'] = pwv
                task_param['t_ground'] = t_ground
                task_param['leakage'] = leakage
                task_param['graphics'] = graphics
                task_param['verbose'] = verbose
                task_param['overwrite'] = overwrite

                msg2("Executing: "+get_taskstr('simobserve', task_param), origin="simalma", priority=v_priority,out=outv)
                if not dryrun:
                    try:
                        simobserve(**task_param)
                        del task_param
                    except:
                        raise Exception, simobserr
                    finally:
                        casalog.origin('simalma')
            
                qimgsize_tp = None
                step += 1

        if tpnant>0 and tptime_min<=0:
            raise Exception,"You requested total power (tpnant=%d) but did not specify a valid nonzero tptime" % tpnant


        mslist_tp = []
        if tptime_min>0:
            ########################################################
            # ACA-TP  simulation - always do this last
            obsmode_sd = "sd"

            msg2("="*60, origin="simalma", priority="warn",out=outv)
            msg2((" Step %d: simulating Total Power" % step), origin="simalma", priority="warn",out=outv)
            msg2("="*60, origin="simalma", priority="warn",out=outv)

            if antlist_tp is None:
                antlist_tp = antlist_tp_default

            pref_tp = get_data_prefix(antlist_tp, project)
            if addnoise:
                msname_tp = pref_tp+".noisy.sd.ms"
            else:
                msname_tp = pref_tp+".sd.ms"
                
        
            ###########################
            # Resolve pointing directions of ACA-TP.
            #
            # Pointing directions of TP simulation is defined as follows:
            #
            # [I] if ALMA-12m maps a rectangle region (rectmode=T),
            # TP maps slightly larger region than ALMA-12m by adding 1 PB to
            # mapsize (pointing extent of ALMA-12m).
            #
            # [II] if a list of pointing deirections are specified for the 
            # ALMA-12m observation (multiptg=T), TP pointings are defined as
            # rectangle areas of 2PB x 2PB centered at each pointing direction
            # of ALMA-12m. However, in some cases, it is more efficient to
            # just map a rectangle region that covers all ALMA-12m pointings.
            # In such case, ACA-TP maps a rectangle region whose extent is 2PB
            # larger than the extent of all ALMA-12m pointings.
            if rectmode:
                # Add 1PB to mapsize
                if fullsize:
                    mapx = qa.add(PB12,model_size[0])   # in the unit same as PB
                    mapy = qa.add(PB12,model_size[1])   # in the unit same as PB
                    mapsize_tp = [qa.tos(mapx), qa.tos(mapy)]
                    msg("The full skymodel is being mapped by ALMA 12-m and ACA 7-m arrays. The total power antenna observes 1PB larger extent.", origin="simalma", priority='warn')
                else:
                    # mapsize is defined. Add 1 PB to mapsize.
                    mapx = qa.add(qa.quantity(mapsize[0]), PB12)
                    mapy = qa.add(qa.quantity(mapsize[1]), PB12)
                    mapsize_tp = [qa.tos(mapx), qa.tos(mapy)]
                    msg2("Only part of the skymodel is being mapped by ALMA 12-m and ACA 7-m arrays. The total power antenna observes 1PB larger extent.", origin="simalma", priority='warn')
            else:
                # multi-pointing mode
                npts, pointings, time = myutil.read_pointings(ptgfile_bl)
                center, offsets = myutil.average_direction(pointings)
                del time
                qx = qa.quantity(max(offsets[0])-min(offsets[0]),"deg")
                qy = qa.quantity(max(offsets[1])-min(offsets[1]),"deg")
                # map extent to cover all pointings + 2PB 
                mapx = qa.add(qa.mul(PB12,2.),qx)   # in the unit same as PB
                mapy = qa.add(qa.mul(PB12,2.),qy)   # in the unit same as PB
                mapsize_tp = [qa.tos(mapx), qa.tos(mapy)]
                # number of pointings to map vicinity of each pointings
                qptgspc_tp = qa.quantity(ptgspacing_tp)
                dirs_multi_tp = myutil.calc_pointings2(qptgspc_tp,
                                                       qa.tos(qa.mul(PB12,2.)),
                                                       "square", pointings[0])
                npts_multi = npts * len(dirs_multi_tp)
        
                msg("Number of pointings to map vicinity of each direction = %d" % npts_multi, origin="simalma", priority="DEBUG2")
                del qptgspc_tp, dirs_multi_tp
        
            # save imsize for TP image generation
#            qimgsize_tp = [mapx, mapy]
            # for imaging later, we need to pad even more!
            mapx_im = qa.add(mapx, qa.mul(PB12,(2*tppad_npb)))
            mapy_im = qa.add(mapy, qa.mul(PB12,(2*tppad_npb)))
            qimgsize_tp = [mapx_im, mapy_im]

        
            qptgspc_tp = qa.quantity(ptgspacing_tp)
            pbunit = PB12['unit']
            # number of pointings to map pointing region
            # TODO: use calc pointings for consistent calculation
            npts_rect = int(qa.convert(mapx, pbunit)['value'] \
                            / qa.convert(qptgspc_tp, pbunit)['value']) \
                        * int(qa.convert(mapy, pbunit)['value'] \
                              / qa.convert(qptgspc_tp, pbunit)['value'])
            msg("Number of pointings to map a rect region = %d" % npts_rect, origin="simalma", priority="DEBUG2")
        
            if rectmode:
                dir_tp = direction
                npts_tp = npts_rect
                msg("Rectangle mode: The total power antenna observes 1PB larger region compared to ALMA 12-m and ACA 7-m arrays", origin="simalma", priority='warn')
            else:
                if npts_multi < npts_rect:
                    # Map 2PB x 2PB extent centered at each pointing direction
                    # need to get a list of pointings
                    dir_tp = []
                    locsize = qa.mul(2, PB12)
                    for dir in pointings:
                        dir_tp += myutil.calc_pointings2(qa.tos(qptgspc_tp),
                                                         qa.tos(locsize),
                                                         "square", dir)
        
                    mapsize_tp = ["", ""]
                    #npts_tp = npts_multi
                    npts_tp = len(dir_tp)
                    msg("Multi-pointing mode: The total power antenna observes +-1PB of each point", origin="simalma", priority='warn')
                else:
                    # Map a region that covers all directions
                    dir_tp = center
                    npts_tp = npts_rect
                    msg("Multi-pointing mode: The total power antenna maps a region that covers all pointings", origin="simalma", priority='warn')
                    msg("- Center of poinings: %s" % center, origin="simalma", priority='warn')
                    msg("- Map size: [%s, %s]" % (mapsize_tp[0], mapsize_tp[1]), origin="simalma", priority='warn')
        
        
            # Scale integration time of TP (assure >= 1 visit per direction)
            tottime_tp = "%dmin" % tptime_min
            integration_tp = integration
            ndump = int(qa.convert(tottime_tp, 's')['value']
                       / qa.convert(integration, 's')['value'])
            msg("Max number of dump in %s (integration %s): %d" % \
                (tottime_tp, integration, ndump), origin="simalma", \
                priority="DEBUG2")
        
            if ndump < npts_tp:
                t_scale = float(ndump)/float(npts_tp)
                integration_tp = qa.tos(qa.mul(integration, t_scale))
                msg("Integration time is scaled to cover all pointings in observation time.", origin="simalma", priority='warn')
                msg("- Scaled total power integration time: %s" % integration_tp, origin="simalma", priority='warn')
                ## Sometimes necessary to avoid the effect of round-off error
                #iunit = qa.quantity(integration_tp)['unit']
                #intsec = qa.convert(integration_tp,"s")
                #totsec = intsec['value']*npts_tp#+0.000000001)
                ##tottime_tp = qa.tos(qa.convert(qa.quantity(totsec, "s"), iunit))
                #tottime_tp = qa.tos(qa.quantity(totsec, "s"))
        
            task_param = {}
            task_param['project'] = project
            task_param['skymodel'] = skymodel
            task_param['inbright'] = inbright
            task_param['indirection'] = indirection
            task_param['incell'] = incell
            task_param['incenter'] = incenter
            task_param['inwidth'] = inwidth
            task_param['complist'] = complist
            task_param['compwidth'] = compwidth
            task_param['setpointings'] = True
            task_param['ptgfile'] = '$project.ptg.txt'
            task_param['integration'] = integration_tp
            task_param['direction'] = dir_tp
            task_param['mapsize'] = mapsize_tp
            task_param['maptype'] = maptype_tp
            task_param['pointingspacing'] = ptgspacing_tp
            task_param['caldirection'] = caldirection
            task_param['calflux'] = calflux
            task_param['obsmode'] = obsmode_sd
            task_param['hourangle'] = hourangle
            task_param['totaltime'] = "%dmin" % tptime_min
            task_param['antennalist'] = ""
            task_param['sdantlist'] = antlist_tp
            task_param['sdant'] = tpantid
            task_param['thermalnoise'] = thermalnoise
            task_param['user_pwv'] = pwv
            task_param['t_ground'] = t_ground
            task_param['leakage'] = leakage
            task_param['graphics'] = graphics
            task_param['verbose'] = verbose
            task_param['overwrite'] = overwrite
        
            if dryrun:
                outv=['f']
            else:
                outv=['l','t','f']
            msg2("Executing: "+get_taskstr('simobserve', task_param), origin="simalma", priority=v_priority,out=outv)
            if not dryrun:
                try:
                    simobserve(**task_param)
                    del task_param
                except:
                    raise Exception, simobserr
                finally:
                    casalog.origin('simalma')
            











        ################################################################
        # Imaging
        if image:
            modelimage = ""
            imagename_tp = project+".sd.image"
            if tptime_min > 0:
                ########################################################
                # Image ACA-TP
                step += 1
                msg2("="*60, origin="simalma", priority="warn")
                msg2(" Step %d: generating a total power image. " % step, origin="simalma", priority="warn")
                msg2("="*60, origin="simalma", priority="warn")

                if os.path.exists(fileroot+"/"+msname_tp):
                    vis_tp = msname_tp
                else:
                    msg2("Total power MS '%s' is not found" \
                        % msname_tp, origin="simalma", priority="error")

                # Define imsize to cover TP map region
                msg2("Defining image size to cover map region of total power simulation", origin="simalma", priority=v_priority)
                msg2("- The total power map size: [%s, %s]" % \
                    (qa.tos(qimgsize_tp[0]), qa.tos(qimgsize_tp[1])), \
                    origin="simalma", priority=v_priority)
                if cell != '':
                   # user-defined cell size
                   msg2("- The user defined cell size: %s" % cell, \
                       origin="simalma", priority=v_priority)                   
                   cell_tp = [qa.tos(cell), qa.tos(cell)]
                else:
                   if model_cell == None:
                       # components only simulation
                       compmodel = fileroot+"/"+pref_bl+".compskymodel"
                       msg2("getting the cell size of input compskymodel", \
                           origin="simalma", priority=v_priority)
                       if not os.path.exists(compmodel):
                           msg2("Could not find the skymodel, '%s'" % \
                               compmodel, priority='error')
                       # modifymodel just collects info if outmodel==inmodel
                       model_vals = myutil.modifymodel(compmodel,compmodel,
                                                       "","","","","",-1,
                                                       flatimage=False)
                       model_cell = model_vals[1]
                       model_size = model_vals[2]

                   # skymodel (+ components list) simulation
#                   msg2("- The cell size of input skymodel: [%s, %s]" % \
#                       (qa.tos(model_cell[0]), qa.tos(model_cell[1])), \
#                       origin="simalma", priority=v_priority)
                   cell_tp = [qa.tos(model_cell[0]), qa.tos(model_cell[1])]

                 #####################################################

                imsize_tp = calc_imsize(mapsize=qimgsize_tp, cell=cell_tp)

                msg2("---> The number of pixels needed to cover the map region: [%d, %d]" % \
                    (imsize_tp[0], imsize_tp[1]), \
                    origin="simalma", priority=v_priority)

                msg("Compare with BL imsize and adopt the larger one", \
                    origin="simalma", priority=v_priority)
                # Compare with imsize of BL (note: imsize is an intArray)
                if is_array_type(imsize) and imsize[0] > 0:
                    # User has defined imsize
                    if len(imsize) > 1:
                        imsize_bl = imsize[0:2]
                    else:
                        imsize_bl = [imsize[0], imsize[0]]
                    msg2("---> BL imsize (user defined): [%d, %d]" % \
                        (imsize_bl[0], imsize_bl[1]), \
                        origin="simalma", priority=v_priority)
                else:
                    # the same as input model (calculate from model_size)
                    msg2("estimating imsize of BL from input sky model.", \
                        origin="simalma", priority=v_priority)
                    imsize_bl = calc_imsize(mapsize=model_size, cell=cell_tp)
                    msg2("---> Estimated BL imsize (sky model): [%d, %d]" % \
                        (imsize_bl[0], imsize_bl[1]), \
                        origin="simalma", priority=v_priority)


                imsize_tp = [max(imsize_tp[0], imsize_bl[0]), \
                             max(imsize_tp[1], imsize_bl[1])]

                msg2("The image pixel size of TP: [%d, %d]" % \
                    (imsize_tp[0], imsize_tp[1]), \
                    origin="simalma", priority=v_priority)

                # Generate TP image
                msg2("Generating TP image using 'GJinc' kernel.",\
                    origin="simalma", priority=v_priority)
                gfac = 2.52       # b in Mangum et al. (2007)
                jfac = 1.55       # c in Mangum et al. (2007)
                convfac = 1.8     # The conversion factor to get HWHM of kernel roughly equal to qhwhm
                kernelfac = 0.7   # ratio of (kernel HWHM)/(TP pointingspacing)
                #qfwhm = PB12      # FWHM of GJinc kernel.
                #gwidth = qa.tos(qa.mul(gfac/3.,qfwhm))
                #jwidth = qa.tos(qa.mul(jfac/3.,qfwhm))
                qhwhm = qa.mul(qptgspc_tp, kernelfac)  # hwhm of GJinc kernel
                gwidth = qa.tos(qa.mul(qhwhm, convfac))
                jwidth = qa.tos(qa.mul(jfac/gfac/pl.log(2.),gwidth))
                #print("Kernel parameter: [qhwhm, gwidth, jwidth] = [%s, %s, %s]" % (qa.tos(qhwhm), gwidth, jwidth))
                # Parameters for sdimaging
                task_param = {}
#                 task_param['infile'] = fileroot+"/"+vis_tp
                task_param['infiles'] = fileroot+"/"+vis_tp
                task_param['gridfunction'] = 'gjinc'
                task_param['gwidth'] = gwidth
                task_param['jwidth'] = jwidth
                task_param['outfile'] = fileroot+"/"+imagename_tp
                task_param['imsize'] = imsize_tp
                # sdimaging doesn't actually take a quantity,
                #cell_arcmin=qa.convert(cell_tp[0],'arcmin')['value']
                #task_param['cell'] = cell_arcmin
                task_param['cell'] = cell_tp
                task_param['phasecenter'] = model_refdir
                task_param['dochannelmap'] = True
                task_param['nchan'] = model_nchan
                saveinputs('sdimaging',
                           fileroot+"/"+project+".sd.sdimaging.last",
                           myparams=task_param)
                msg2("Having set up the gridding parameters, the sdimaging task is called to actually creage the image:")
                msg2("   "+get_taskstr('sdimaging', task_param), origin="simalma", priority=v_priority)

                if not dryrun:
                    sdimaging(**task_param)
                del task_param
                # TODO: scale TP image
                
                # Set restoring beam
                # TODO: set proper beam size
                #bmsize = qa.quantity(PB12)
                pbunit = PB12sim['unit']
                simpb_val = PB12sim['value']
                # the acutal HWHM is 3.5% smaller
                kernel_val = qa.convert(qhwhm, pbunit)['value']*0.965 
                bmsize = qa.quantity(pl.sqrt(simpb_val**2+4.*kernel_val**2), pbunit)
                msg2("Setting estimated restoring beam to TP image: %s" % qa.tos(bmsize),\
                         origin="simalma", priority=v_priority)
                #print "- SimPB = %f%s" % (simpb_val, pbunit)
                #print "- image kernel = %f%s" % (kernel_val, pbunit)

                if not dryrun:
                    ia.open(fileroot+"/"+imagename_tp)
                    ia.setrestoringbeam(major=bmsize, minor=bmsize,
                                    pa=qa.quantity("0.0deg"))
                    ia.close()
                
#                 ##### Generate TP image using BOX kernel
#                 msg("Generating TP image using 'BOX' kernel.",\
#                     origin="simalma", priority=v_priority)
#                 im.open(fileroot+"/"+vis_tp)
#                 im.selectvis(nchan=model_nchan,start=0,step=1,spw=0)
#                 ### TODO: should set proper phasecenter based on imdirection!!!
#                 im.defineimage(mode='channel',nx=imsize_tp[0],ny=imsize_tp[1],cellx=imgcell[0],celly=imgcell[1],phasecenter=model_refdir,nchan=model_nchan,start=0,step=1,spw=0)
#                 im.setoptions(ftmachine='sd',gridfunction='box')
#                 im.makeimage(type='singledish',image=fileroot+"/"+imagename_tp)
#                 im.close()
#                 # set restoring beam
#                 ia.open(fileroot+"/"+imagename_tp)
#                 ia.setrestoringbeam(major=PB12,minor=PB12,pa=qa.quantity("0.0deg"))
#                 ia.close()
#                 #####################################################



                # Analyze TP image
                tpskymodel=fileroot+"/"+pref_tp+".skymodel"

                msg2("Analyzing TP image.", origin="simalma", priority=v_priority)
                vis_tp = fileroot+"/"+vis_tp

                task_param = {}
                task_param['project'] = project
                task_param['image'] = False
                task_param['imagename'] = fileroot+"/"+imagename_tp
                task_param['skymodel'] = tpskymodel
                task_param['analyze'] = True
                task_param['showuv'] = False
                task_param['showpsf'] = False
                task_param['showconvolved'] = True
                task_param['graphics'] = graphics
                task_param['verbose'] = verbose
                task_param['overwrite'] = overwrite

                msg2("Executing: "+get_taskstr('simanalyze', task_param), origin="simalma", priority=v_priority)

                if not dryrun:
                    try:
                        simanalyze(**task_param)
                        del task_param
                    except:
                        raise Exception, simanaerr
                    finally:
                        casalog.origin('simalma')

                # Back up simanalyze.last file
                if os.path.exists(fileroot+"/"+simana_file):
                    simana_new = imagename_tp.replace(".image",".simanalyze.last")
                    msg2("Back up input parameter set to '%s'" % simana_new, \
                        origin="simalma", priority=v_priority)
                    shutil.move(fileroot+"/"+simana_file, fileroot+"/"+simana_new)

                if not os.path.exists(fileroot+"/"+imagename_tp):
                    msg2("TP image '%s' is not found" \
                        % imagename_tp, origin="simalma", priority="error")
                #modelimage = imagename_aca






            ############################################################
            # Image each int array separately
            nconfig=len(antennalist)

            for i in range(nconfig):
                step += 1
                msg2("="*60, origin="simalma", priority="warn")
                msg2((" Step %d: imaging and analyzing " % step)+antennalist[i], origin="simalma", priority="warn")
                msg("="*60, origin="simalma", priority="warn")

                pref=get_data_prefix(antennalist[i], project)
                if addnoise:
                    msname = pref+".noisy.ms"
                    imagename_int=pref+".noisy.ms"
                else:
                    msname= pref+".ms"
                    imagename_int=pref+".ms"
                    
                if os.path.exists(fileroot+"/"+msname):
                    vis_int = msname
                else:
                    msg("Could not find MS to image, '%s'" \
                            % msname, origin="simalma", priority="error")

# i think simanalye is fixed 20130826
#            # TMP fix: get correct skymodel file so that simanalyze picks it
#            if acaratio > 0:
#                if os.path.exists(tpskymodel):
#                    shutil.move(tpskymodel,tpskymodel+".save")
#                else:
#                    msg("TP skymodel '%s' is not found" \
#                        % tpskymodel, origin="simalma", priority="error")
#
#                if os.path.exists(acaskymodel+".save"):
#                    shutil.move(acaskymodel+".save",acaskymodel)
#                else:
#                    msg("ACA skymodel '%s' is not found" \
#                        % acaskymodel+".save", origin="simalma", priority="error")

                task_param = {}
                task_param['project'] = project
                task_param['image'] = image
                task_param['vis'] = vis_int
                task_param['modelimage'] = ""
                task_param['cell'] = cell
                task_param['imsize'] = imsize
                task_param['imdirection'] = imdirection
                task_param['niter'] = niter
                task_param['threshold'] = threshold
                task_param['weighting'] = weighting
                task_param['mask'] = []
                task_param['outertaper'] = []
                task_param['stokes'] = 'I'
                task_param['analyze'] = True
                task_param['graphics'] = graphics
                task_param['verbose'] = verbose
                task_param['overwrite'] = overwrite

                msg2("Executing: "+get_taskstr('simanalyze', task_param), origin="simalma", priority=v_priority)

                if not dryrun:
                    try:
                        simanalyze(**task_param)
                        del task_param
                    except:
                        raise Exception, simanaerr
                    finally:
                        casalog.origin('simalma')



            if nconfig>1:
                ############################################################
                # concat
                step += 1
                msg2("="*60, origin="simalma", priority="warn")
                msg2(" Step %d: concatenating interferometric visibilities." % step, origin="simalma", priority="warn")
                msg2("="*60, origin="simalma", priority="warn")
                
                weights=pl.zeros(nconfig)+1
                z=pl.where(configtypes=='ACA')[0]
                if len(z)>0:
                    weights[z]=weightratio_7_12

                mslist=[]
                for i in range(nconfig):
                    pref=get_data_prefix(antennalist[i],project)
                    if addnoise:
                        msname = fileroot + "/" + pref+".noisy.ms"
                    else:
                        msname = fileroot + "/" + pref+".ms"
                    mslist.append(msname)

                msg2("Will execute: concat(vis="+str(mslist)+",concatvis="+concatname+".ms,visweightscale="+str(weights))
                if not dryrun:
                    try:
                        concat(vis=mslist,concatvis=concatname+".ms",visweightscale=weights)
                    except:
                        raise Exception, simanaerr
                    finally:
                        casalog.origin('simalma')




            ############################################################
            # Image ALMA-BL + ACA-7m                
                step += 1
                msg2("="*60, origin="simalma", priority="warn")
                msg2((" Step %d: imaging and analyzing " % step)+concatname+".ms", origin="simalma", priority="warn")
                msg("="*60, origin="simalma", priority="warn")

                    
                if os.path.exists(fileroot+"/"+concatname+".ms"):
                        vis_int = fileroot+"/"+concatname+".ms"
                elif os.path.exists(concatname+".ms"):
                    vis_int = concatname+".ms"
                else: msg("Could not find MS to image, "+concatname+".ms", origin="simalma", priority="error")

                task_param = {}
                task_param['project'] = project
                task_param['image'] = image
                task_param['vis'] = vis_int
                task_param['modelimage'] = ""
                task_param['cell'] = cell
                task_param['imsize'] = imsize
                task_param['imdirection'] = imdirection
                task_param['niter'] = niter
                task_param['threshold'] = threshold
                task_param['weighting'] = weighting
                task_param['mask'] = []
                task_param['outertaper'] = []
                task_param['stokes'] = 'I'
                task_param['analyze'] = True
                task_param['graphics'] = graphics
                task_param['verbose'] = verbose
                task_param['overwrite'] = overwrite

                msg2("Executing: "+get_taskstr('simanalyze', task_param), origin="simalma", priority=v_priority)
                imagename_int=os.path.basename(concatname.rstrip("/"))+".image"

                if not dryrun:
                    try:
                        simanalyze(**task_param)
                        del task_param
                    except:
                        raise Exception, simanaerr
                    finally:
                        casalog.origin('simalma')








            if tptime_min > 0:
                ########################################################
                # Combine TP + INT image
                step += 1
                msg("="*60, origin="simalma", priority="warn")
                msg(" Step %d: combining a total power and synthesis image. " % step, origin="simalma", priority="warn")
                msg("="*60, origin="simalma", priority="warn")
                if os.path.exists(fileroot+"/"+imagename_int):
                    highimage0 = fileroot+"/"+imagename_int
                else:
                    msg("The synthesized image '%s' is not found" \
                        % imagename_int, origin="simalma", priority="error")
                if not os.path.exists(fileroot+"/"+imagename_tp):
                    msg("ACA is requested but total power image '%s' is not found" \
                        % imagename_tp, origin="simalma", priority="error")
                #lowimage = fileroot+"/"+imagename_tp

                # Need to manipulate TP image here
                outimage0 = fileroot+"/" + combimage+"0"
                outimage = fileroot+"/" + combimage
                pbcov = highimage0.rstrip("image") + "flux.pbcoverage"
                regridimg = fileroot + "/" + imagename_tp + ".regrid"
                scaledimg = fileroot + "/" + imagename_tp + ".pbscaled"
                lowimage = scaledimg

                # regrid TP image
                inttemplate = imregrid(imagename = highimage0, template='get')
                imregrid(imagename = fileroot+"/"+imagename_tp,
                         template = inttemplate, output = regridimg)
                # multiply SD image with INT PB coverage
                if not os.path.exists(pbcov):
                    msg("The flux image '%s' is not found" \
                        % pbcov, origin="simalma", priority="error")
                immath(imagename=[regridimg, pbcov],
                       expr='IM1*IM0',outfile=scaledimg)
                # restore TP beam and brightness unit
                ia.open(fileroot+"/"+imagename_tp)
                beam_tp = ia.restoringbeam()
                bunit_tp = ia.brightnessunit()
                ia.close()
                ia.open(scaledimg)
                ia.setrestoringbeam(beam=beam_tp)
                ia.setbrightnessunit(bunit_tp)
                ia.close()

                # de-pbcor the INT image
                highimage = fileroot+"/"+imagename_int+".pbscaled"
                #immath(imagename=[highimage0, pbcov],
                #       expr='IM1/IM0',outfile=highimage)        
                immath(imagename=[highimage0, pbcov],
                       expr='IM1*IM0',outfile=highimage)
                # restore INT beam and brightness unit
                ia.open(highimage0)
                beam_int = ia.restoringbeam()
                bunit_int = ia.brightnessunit()
                ia.close()
                ia.open(highimage)
                ia.setrestoringbeam(beam=beam_int)
                ia.setbrightnessunit(bunit_int)
                ia.close()

                # Feathering
                task_param = {}
                task_param['imagename'] = outimage0
                task_param['highres'] = highimage
                task_param['lowres'] = lowimage

                msg("Executing: "+get_taskstr('feather', task_param), origin="simalma", priority=v_priority)
                try:
                    saveinputs('feather',
                               fileroot+"/"+project+".feather.last",
                               myparams=task_param)
                    feather(**task_param)
                    del task_param

                    # This seems not necessary anymore.
                    ## transfer mask - feather should really do this
                    #ia.open(outimage0)
                    #ia.maskhandler('copy',[highimage+":mask0",'mask0'])
                    #ia.maskhandler('set','mask0')
                    #ia.done()
                except:
                    raise Exception, "simalma caught an exception in task feather"
                finally:
                    shutil.rmtree(regridimg)
                    #shutil.rmtree(scaledimg)
                    casalog.origin('simalma')

                # re-pbcor the result
                immath(imagename=[outimage0, pbcov],
                       expr='IM0/IM1',outfile=outimage)


                ########################################################
                # Generate Summary Plot
                grscreen = False
                grfile = False
                if graphics == "both":
                    grscreen = True
                    grfile = True
                if graphics == "screen":
                    grscreen = True
                if graphics == "file":
                    grfile = True
                if grscreen or grfile:
                    if grfile:
                        file = fileroot + "/" + project + ".combine.png"
                    else:
                        file = ""

                    # check for image pathes
                    if os.path.exists(skymodel):
                        flatsky = pref_bl + ".skymodel.flat"
                    else:
                        flatsky = pref_bl + ".compskymodel.flat"
                    if not os.path.exists(fileroot+"/"+flatsky):
                        raise Exception, "Coud not find a skymodel image '%s'" % flatsky

                    if not os.path.exists(fileroot+"/"+combimage):
                        raise Exception, "Coud not find the combined image '%s'" % combimage

                    if not os.path.exists(fileroot+"/"+imagename_int):
                        raise Exception, "Coud not find the synthesized image '%s'" % imagename_int

                    if not os.path.exists(fileroot+"/"+imagename_tp):
                        raise Exception, "Coud not find the total power image '%s'" % (imagename_tp)
                    # Now the actual plotting
                    disprange = None
                    myutil.newfig(multi=[2,2,1],show=grscreen)
                    # skymodel
                    #discard = myutil.statim(fileroot+"/"+flatsky,disprange=disprange)

                    #disprange = []
                    # generate flat synthesized (7m+12m) image
                    flatint = fileroot + "/" + imagename_int + ".flat"
                    myutil.flatimage(fileroot+"/"+imagename_int,verbose=verbose)
                    if not os.path.exists(flatint):
                        raise Exception, "Failed to generate '%s'" % (flatint)

                    # generate convolved sky model image
                    myutil.convimage(fileroot+"/"+flatsky, flatint)
                    discard = myutil.statim(fileroot+"/"+flatsky+".regrid.conv",disprange=disprange)
                    shutil.rmtree(fileroot+"/"+flatsky+".regrid")
                    shutil.rmtree(fileroot+"/"+flatsky+".regrid.conv")
                    
                    # total power image
                    flattp = fileroot + "/" + imagename_tp + ".flat"
                    myutil.flatimage(fileroot+"/"+imagename_tp,verbose=verbose)
                    #flattp = scaledimg + ".flat"
                    #myutil.flatimage(scaledimg,verbose=verbose)
                    if not os.path.exists(flattp):
                        raise Exception, "Failed to generate '%s'" % (flattp)
                    myutil.nextfig()
                    discard = myutil.statim(flattp,disprange=disprange)
                    shutil.rmtree(flattp)

                    #disprange = []
                    # display flat synthesized (7m+12m) image
                    myutil.nextfig()
                    discard = myutil.statim(flatint,disprange=disprange)
                    shutil.rmtree(flatint)

                    # combined image
                    flatcomb = fileroot + "/" + combimage + ".flat"
                    myutil.flatimage(fileroot+"/"+combimage,verbose=verbose)
                    if not os.path.exists(flatcomb):
                        raise Exception, "Failed to generate '%s'" % (flatcomb)
                    myutil.nextfig()
                    discard = myutil.statim(flatcomb,disprange=disprange)
                    myutil.endfig(show=grscreen,filename=file)
                    shutil.rmtree(flatcomb)

        if myutil.isreport():
            myutil.closereport()
        finalize_tools()



    except TypeError, e:
        finalize_tools()
        if myutil.isreport():
            myutil.closereport()
        casalog.post("simalma -- TypeError: %s" % str(e), priority="ERROR")
        raise TypeError, e
        return False
    except ValueError, e:
        finalize_tools()
        if myutil.isreport():
            myutil.closereport()
        casalog.post("simalma -- OptionError: %s" % str(e), priority="ERROR")
        raise ValueError, e
        return False
    except Exception, instance:
        finalize_tools()
        if myutil.isreport():
            myutil.closereport()
        casalog.post("simalma -- Exception: %s" % str(instance),
                     priority="ERROR")
        raise Exception, instance
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

def calc_imsize(mapsize=None, cell=None):
    if mapsize == None:
        raise ValueError, "mapsize is not defined"
    if cell == None:
        raise ValueError, "cell is not defined"
    # get a list of cell size
    if is_array_type(cell):
        if len(cell) < 2:
            cell = [cell[0], cell[0]]
    else:
        cell = [cell, cell]

    for qval in cell:
        if not qa.compare(qval, "deg"):
            raise TypeError, "cell should be an angular size"

    qcellx = qa.quantity(cell[0])
    qcelly = qa.quantity(cell[1])

    # get a list of map size
    if is_array_type(mapsize):
        if len(mapsize) < 2:
            mapsize = [mapsize[0], mapsize[0]]
    else:
        mapsize = [mapsize, mapsize]

    for qval in mapsize:
        if not qa.compare(qval, "deg"):
            raise TypeError, "mapsize should be an angular size"

    vsizex = qa.convert(mapsize[0], qcellx['unit'])['value']
    vsizey = qa.convert(mapsize[1], qcelly['unit'])['value']

    # Calculate the number of pixels to cover the map size
    npixx = int(pl.ceil(vsizex/qcellx['value']))
    npixy = int(pl.ceil(vsizey/qcelly['value']))

    return [npixx, npixy]


def get_taskstr(taskname, params):
    out = ("%s(" % taskname)
    for key, val in params.items():
        out += (key + "=" + _get_str(val) + ", ")

    return ( out.rstrip(", ") + ")" )

def _get_str(paramval):
    if type(paramval) == str:
        return ("'%s'" % paramval)
    # else
    return str(paramval)
