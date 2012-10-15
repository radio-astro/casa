import os
import shutil
import re
import glob
import numpy

from taskinit import *
from simutil import *
from simobserve import simobserve
from simanalyze import simanalyze

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

    # Collect a list of parameter values to save inputs
    in_params =  locals()

    try:
        casalog.origin('simalma')
        if verbose:
            casalog.filter(level="DEBUG2")
            t_priority = "WARN"
        else:
            t_priority = "INFO"
    
        a = inspect.stack()
        stacklevel = 0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel = k
        myf = sys._getframe(stacklevel).f_globals

        # Parameter verifications
        # antennalist should be one of ALMA config.
        if antennalist.upper().find("ALMA") < 0:
            raise ValueError, "antennalist should be ALMA configuration in simalma."

        # put output in directory called "project"
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

        saveinputs = myf['saveinputs']
        saveinputs('simalma',fileroot+"/"+project+".simalma.last",
                   myparams=in_params)

        # create the utility object:
        myutil = simutil(direction)
        if verbose: myutil.verbose = True
        msg = myutil.msg

        arraytype = [list, tuple, numpy.ndarray]

        # Fixed parameters
        nyquist = 0.48113 ## Nyquist
        maptype_bl = 'ALMA'
        maptype_aca = 'ALMA'
        maptype_tp = 'square'
        pbgridratio_tp = 0.36
        refdate = '2013/05/21'
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

        # format mapsize
        if not (type(mapsize) in arraytype):
            mapsize = [mapsize, mapsize]
        elif len(mapsize) < 2:
            mapsize = [mapsize[0], mapsize[0]]

        # Operation flags
        addnoise = (thermalnoise != '')
        # Rectangle setup mode
        multiptg = (not setpointings) \
                   or ((type(direction) in arraytype) and len(direction) > 1)
        rectmode = (not multiptg)

        # Use full model image as a mapsize = ["", ""]
        fullsize = (len(mapsize[0]) == 0) or (len(mapsize[1]) == 0)
        # Test for cycle 1
        #q = re.compile('.*CYCLE.?1.?;(.*)')
        q = re.compile('.*CYCLE.?1.*')
        isC1 = q.match(antennalist.upper())
        if isC1:
            msg("Cycle-1 ALMA simulation", origin="simalma", priority="warn")
        else:
            msg("Full Science ALMA simulation", origin="simalma", priority="warn")
            
        # antennalist of ACA and TP
        antlist_tp = "aca.tp.cfg"

        if not acaconfig: # default
            if isC1:
                acaconfig = "cycle1"
            else:
                acaconfig = "i"

        warnmsg = "You are simulating Cycle1 observation but requested Full Science configuration for ACA. Assuming you know what you want and using config '%s'."
        if acaconfig.upper().startswith("I"):
            antlist_aca = "aca.i.cfg"
            if isC1:
                msg(warnmsg % antlist_aca, origin="simalma", priority="warn")
        elif acaconfig.upper().startswith("N"):
            antlist_aca = "aca.ns.cfg"
            if isC1:
                msg(warnmsg % antlist_aca, origin="simalma", priority="warn")
        else:
            antlist_aca = "aca_cycle1.cfg"

        # Resolve prefixes of simulation data (as defined in 
        # simobserve and simanalyze)
        pref_bl = get_data_prefix(antennalist, project)
        pref_aca = get_data_prefix(antlist_aca, project)
        pref_tp = get_data_prefix(antlist_tp, project)
        # Resolve output names (as defined in simobserve and simanalyze)
        ptgfile_bl = fileroot+"/"+pref_bl+".ptg.txt"
        if addnoise:
            msname_bl = pref_bl+".noisy.ms"
            msname_aca = pref_aca+".noisy.ms"
            msname_tp = pref_tp+".noisy.sd.ms"
            imagename_aca = pref_aca+".noisy.image"
        else:
            msname_bl = pref_bl+".ms"
            msname_aca = pref_aca+".ms"
            msname_tp = pref_tp+".sd.ms"
            imagename_aca = pref_aca+".image"

        # Either skymodel or complist should exists
        if type(skymodel) in arraytype:
            skymodel = skymodel[0]
        skymodel = skymodel.replace('$project',pref_bl)
    
        if type(complist) in arraytype:
            complist = complist[0]

        if((not os.path.exists(skymodel)) and (not os.path.exists(complist))):
            raise ValueError, "No sky input found.  At least one of skymodel or complist must be set."

        ###########################
        # Get model_size and model_center
        if os.path.exists(skymodel):
            outmodel = fileroot+"/"+project+"temp.skymodel"
            model_vals = myutil.modifymodel(skymodel, outmodel, inbright,
                                            indirection, incell, incenter,
                                            inwidth, -1, False)
            shutil.rmtree(outmodel)
            model_size = model_vals[2]
            model_center = model_vals[4]
            del model_vals
        else:
            compdirs = []
            cl.open(complist)
            for i in range(cl.length()):
                compdirs.append(myutil.dir_m2s(cl.getrefdir(i)))

            model_refdir, coffs = myutil.average_direction(compdirs)
            model_center = cl.getspectrum(0)['frequency']['m0']
            cmax = 0.0014 # ~5 arcsec
            for i in range(coffs.shape[1]):
                xc = numpy.absolute(coffs[0,i])  # offsets in deg
                yc = numpy.absolute(coffs[1,i])
                cmax = max(cmax, xc)
                cmax = max(cmax, yc)
            model_size = ["%fdeg" % (2*cmax), "%fdeg" % (2*cmax)]
            cl.done()
            del compdirs, model_refdir, coffs, xc, yc, cmax

        # Calculate 12-m PB
        Dant = 12.
        pbval = 0.3 / qa.convert(qa.quantity(model_center),'GHz')['value'] \
                / Dant * 3600. * 180 / numpy.pi # arcsec
        # Definition of PB in OT and simulator differs.
        PB12ot = qa.quantity(pbval, "arcsec")
        PB12sim = qa.quantity(pbval*1.2, "arcsec")
        msg("PB size - OT: %s, simulator: %s" % (qa.tos(PB12ot), qa.tos(PB12sim)), origin="simalma", priority='DEBUG2')
        nyqpbratio = nyquist/1.2   # OT Nyquist spacing in unit of simulator PB
        pointingspacing = str(nyqpbratio)+"PB"
        #msg("Setting Nyquist pointing spacing %s" % pointingspacing, origin="simalma", priority="warn")
        
        ############################################################
        # ALMA-BL simulation
        step = 1
        msg("="*60, origin="simalma", priority="warn")
        msg(" Step %d: simulating ALMA 12-m array observation." % step, origin="simalma", priority="warn")
        msg("="*60, origin="simalma", priority="warn")

        obsmode_int = 'int'            
        # BL mapsize should be 1 PB smaller than skymodel when using ACA
        #if acaratio > 0 and rectmode and fullsize:
        #    mapx = qa.sub(model_size[0], PB12ot)
        #    mapy = qa.sub(model_size[1], PB12ot)
        #    mapsize_bl = [qa.tos(mapx), qa.tos(mapy)]
        #else:
        #    mapsize_bl = mapsize
        mapsize_bl = mapsize
        
        taskstr = "simobserve(project='"+project+"', skymodel='"+skymodel+"', inbright='"+inbright+"', indirection='"+indirection+"', incell='"+incell+"', incenter='"+incenter+"', inwidth='"+inwidth+"', complist='"+complist+"', compwidth='"+compwidth+"', setpointings="+str(setpointings)+", ptgfile='"+ptgfile+"', integration='"+integration+"', direction='"+str(direction)+"', mapsize="+str(mapsize_bl)+", maptype='"+maptype_bl+"', pointingspacing='"+pointingspacing+"', caldirection='"+caldirection+"', calflux='"+calflux+"',  obsmode='"+obsmode_int+"', refdate='"+refdate+"', hourangle='"+hourangle+"', totaltime='"+totaltime+"', antennalist='"+antennalist+"', sdantlist='', sdant="+str(0)+", thermalnoise='"+thermalnoise+"', user_pwv="+str(pwv)+", t_ground="+str(t_ground)+", leakage="+str(leakage)+", graphics='"+graphics+"', verbose="+str(verbose)+", overwrite="+str(overwrite)+")"
        msg("Executing: "+taskstr, origin="simalma", priority=t_priority)

        simobserve(project=project,
                   skymodel=skymodel, inbright=inbright,
                   indirection=indirection, incell=incell,
                   incenter=incenter, inwidth=inwidth,
                   complist=complist, compwidth=compwidth,
                   setpointings=setpointings, ptgfile=ptgfile,
                   integration=integration,
                   direction=direction, mapsize=mapsize_bl,
                   maptype=maptype_bl, pointingspacing=pointingspacing,
                   caldirection=caldirection, calflux=calflux, 
                   obsmode=obsmode_int, refdate=refdate,
                   hourangle=hourangle, totaltime=totaltime,
                   antennalist=antennalist,
                   sdantlist="", sdant=0,
                   thermalnoise=thermalnoise, user_pwv=pwv,
                   t_ground=t_ground, #t_sky=t_sky, tau0=tau0, seed=seed,
                   leakage=leakage,
                   graphics=graphics, verbose=verbose, overwrite=overwrite)#,
                   #async=False)

        if acaratio > 0:
            ########################################################
            # ACA-7m simulation
            step += 1
            msg("="*60, origin="simalma", priority="warn")
            msg(" Step %d: simulating ACA 7-m array observation." % step, origin="simalma", priority="warn")
            msg("="*60, origin="simalma", priority="warn")
            # Calculate total time for ACA and TP
            if qa.compare(totaltime,'s'):
                tottime_aca = qa.tos(qa.mul(totaltime, acaratio))
            else: # number of visit (calc ACA tottime)
                npts, pointings, time = myutil.read_pointings(ptgfile_bl)
                if len(time) == npts:
                    tottime_aca = qa.tos(qa.quantity(sum(time)*acaratio,'s'))
                else:
                    tottime_aca = qa.tos(qa.mul(integration, ntps*acaratio))
                del npts, pointings, time
            msg("Total observing time of ACA = %s" % tottime_aca, origin="simalma", priority='warn')

            # Same pointings as BL
            #ptgfile_aca = ptgfile_bl

            taskstr = "simobserve(project='"+project+"', skymodel='"+skymodel+"', inbright='"+inbright+"', indirection='"+indirection+"', incell='"+incell+"', incenter='"+incenter+"', inwidth='"+inwidth+"', complist='"+complist+"', compwidth='"+compwidth+"', setpointings="+str(setpointings)+", ptgfile='"+ptgfile+"', integration='"+integration+"', direction='"+str(direction)+"', mapsize="+str(mapsize_bl)+", maptype='"+maptype_aca+"', pointingspacing='"+pointingspacing+"', caldirection='"+caldirection+"', calflux='"+calflux+"',  obsmode='"+obsmode_int+"', refdate='"+refdate+"', hourangle='"+hourangle+"', totaltime='"+tottime_aca+"', antennalist='"+antlist_aca+"', sdantlist='', sdant="+str(0)+", thermalnoise='"+thermalnoise+"', user_pwv="+str(pwv)+", t_ground="+str(t_ground)+", leakage="+str(leakage)+", graphics='"+graphics+"', verbose="+str(verbose)+", overwrite="+str(overwrite)+")"
            msg("Executing: "+taskstr, origin="simalma", priority=t_priority)
            
            simobserve(project=project,
                       skymodel=skymodel, inbright=inbright,
                       indirection=indirection, incell=incell,
                       incenter=incenter, inwidth=inwidth,
                       complist=complist, compwidth=compwidth,
                       #setpointings=False, ptgfile=ptgfile_aca,
                       setpointings=setpointings, ptgfile=ptgfile,
                       integration=integration,
                       direction=direction, mapsize=mapsize_bl,
                       maptype=maptype_aca, pointingspacing=pointingspacing,
                       caldirection=caldirection, calflux=calflux, 
                       obsmode=obsmode_int, refdate=refdate,
                       hourangle=hourangle, totaltime=tottime_aca,
                       antennalist=antlist_aca,
                       sdantlist="", sdant=0,
                       thermalnoise=thermalnoise, user_pwv=pwv,
                       t_ground=t_ground, #t_sky=t_sky, tau0=tau0, seed=seed,
                       leakage=leakage,
                       graphics=graphics, verbose=verbose, overwrite=overwrite)#,
                       #async=False)

            ########################################################
            # ACA-TP  simulation
            step += 1
            msg("="*60, origin="simalma", priority="warn")
            msg(" Step %d: simulating ACA total power observation." % step, origin="simalma", priority="warn")
            msg("="*60, origin="simalma", priority="warn")
            obsmode_sd = "sd"
            # Resolve mapsize of TP. Add 1 PB to pointing extent of BL
            if rectmode:
                # Add 1PB to mapsize
                if fullsize:
                    mapx = qa.add(PB12ot,model_size[0])   # in the unit same as PB
                    mapy = qa.add(PB12ot,model_size[1])   # in the unit same as PB
                    mapsize_tp = [qa.tos(mapx), qa.tos(mapy)]
                    msg("Full skymodel mapped by ALMA 12-m and ACA 7-m arrays. The total power antenna observes 1PB larger extent.", origin="simalma", priority='warn')
                else:
                    # mapsize is defined. Add 1 PB to mapsize.
                    mapx = qa.add(qa.quantity(mapsize[0]), PB12ot)
                    mapy = qa.add(qa.quantity(mapsize[1]), PB12ot)
                    mapsize_tp = [qa.tos(mapx), qa.tos(mapy)]
                    msg("A part of skymodel mapped by ALMA 12-m and ACA 7-m arrays. The total power antenna observes 1PB larger extent.", origin="simalma", priority='warn')
            else:
                # multi-pointing mode
                npts, pointings, time = myutil.read_pointings(ptgfile_bl)
                center, offsets = myutil.average_direction(pointings)
                del time
                qx = qa.quantity(max(offsets[0])-min(offsets[0]),"deg")
                qy = qa.quantity(max(offsets[1])-min(offsets[1]),"deg")
                mapx = qa.add(qa.mul(PB12ot,2.),qx)   # in the unit same as PB
                mapy = qa.add(qa.mul(PB12ot,2.),qy)   # in the unit same as PB
                mapsize_tp = [qa.tos(mapx), qa.tos(mapy)]
                # number of pointings to map vicinity of each pointings
                npts_multi = npts * int(2./pbgridratio_tp)**2
                msg("Number of pointings to map vicinity of each direction = %d" % npts_multi, origin="simalma", priority="DEBUG2")

            grid_tp = qa.mul(PB12ot, pbgridratio_tp)
            pbunit = PB12ot['unit']
            # number of pointings to map pointing region
            npts_rect = int(qa.convert(mapx, pbunit)['value'] \
                            / qa.convert(grid_tp, pbunit)['value']) \
                        * int(qa.convert(mapy, pbunit)['value'] \
                              / qa.convert(grid_tp, pbunit)['value'])
            msg("Number of pointings to map a rect region = %d" % npts_rect, origin="simalma", priority="DEBUG2")

            if rectmode:
                dir_tp = direction
                npts_tp = npts_rect
                msg("Rectangle mode: The total power antenna observes 1PB larger region compared to ALMA 12-m and ACA 7-m arrays", origin="simalma", priority='warn')
            else:
                if npts_multi < npts_rect:
                    # Map +-1PB extent of each direction
                    # need to get a list of pointings
                    dir_tp = []
                    locsize = qa.mul(2, PB12ot)
                    for dir in pointings:
                        dir_tp += myutil.calc_pointings2(qa.tos(grid_tp),
                                                         qa.tos(locsize),
                                                         "square", dir)
                    mapsize_tp = ["", ""]
                    npts_tp = npts_multi
                    msg("Multi-pointing mode: The total power antenna observes +-1PB of each point", origin="simalma", priority='warn')
                else:
                    # Map a region that covers all directions
                    dir_tp = center
                    npts_tp = npts_rect
                    msg("Multi-pointing mode: The total power antenna maps a region that covers all pointings", origin="simalma", priority='warn')
                    msg("- Center of poinings: %s" % center, origin="simalma", priority='warn')
                    msg("- Map size: [%s, %s]" % (mapsize_tp[0], mapsize_tp[1]), origin="simalma", priority='warn')
            
            ptgspacing_tp = str(pbgridratio_tp*PB12ot['value']/PB12sim['value'])+"PB"

            # Scale integration time of TP (assure >= 1 visit per direction)
            tottime_tp = tottime_aca
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
            
            taskstr = "simobserve(project='"+project+"', skymodel='"+skymodel+"', inbright='"+inbright+"', indirection='"+indirection+"', incell='"+incell+"', incenter='"+incenter+"', inwidth='"+inwidth+"', complist='"+complist+"', compwidth='"+compwidth+"', setpointings="+str(True)+", ptgfile='$project.ptg.txt', integration='"+integration_tp+"', direction='"+str(dir_tp)+"', mapsize="+str(mapsize_tp)+", maptype='"+maptype_tp+"', pointingspacing='"+ptgspacing_tp+"', caldirection='"+caldirection+"', calflux='"+calflux+"',  obsmode='"+obsmode_sd+"', refdate='"+refdate+"', hourangle='"+hourangle+"', totaltime='"+tottime_tp+"', antennalist='', sdantlist='"+antlist_tp+"', sdant="+str(tpantid)+", thermalnoise='"+thermalnoise+"', user_pwv="+str(pwv)+", t_ground="+str(t_ground)+", leakage="+str(leakage)+", graphics='"+graphics+"', verbose="+str(verbose)+", overwrite="+str(overwrite)+")"
            msg("Executing: "+taskstr, origin="simalma", priority=t_priority)

            simobserve(project=project,
                       skymodel=skymodel, inbright=inbright,
                       indirection=indirection, incell=incell,
                       incenter=incenter, inwidth=inwidth,
                       complist=complist, compwidth=compwidth,
                       setpointings=True, ptgfile='$project.ptg.txt',
                       integration=integration_tp,
                       direction=dir_tp, mapsize=mapsize_tp,
                       maptype=maptype_tp, pointingspacing=ptgspacing_tp,
                       caldirection=caldirection, calflux=calflux, 
                       obsmode=obsmode_sd, refdate=refdate,
                       hourangle=hourangle, totaltime=tottime_tp,
                       antennalist="", sdantlist=antlist_tp, sdant=tpantid,
                       thermalnoise=thermalnoise, user_pwv=pwv,
                       t_ground=t_ground, #t_sky=t_sky, tau0=tau0, seed=seed,
                       leakage=leakage,
                       graphics=graphics, verbose=verbose, overwrite=overwrite)#,
                       #async=False)

        ################################################################
        # Imaging
        if image:
            modelimage = ""
            if acaratio > 0:
                ########################################################
                # Image ACA-7m + ACA-TP
                step += 1
                msg("="*60, origin="simalma", priority="warn")
                msg(" Step %d: generating ACA 7-m array + total power image. " % step, origin="simalma", priority="warn")
                msg("="*60, origin="simalma", priority="warn")
                if os.path.exists(fileroot+"/"+msname_aca):
                    vis_aca = msname_aca+","
                else:
                    msg("ACA is requested but ACA 7-m MS '%s' is not found" \
                        % msname_aca, origin="simalma", priority="error")
                if os.path.exists(fileroot+"/"+msname_tp):
                    vis_aca += msname_tp
                else:
                    msg("ACA is requested but total power MS '%s' is not found" \
                        % msname_tp, origin="simalma", priority="error")
                cell_aca = cell

                imsize_aca = 0

                taskstr = "simanalyze(project='"+project+"', image="+str(image)+", vis='"+vis_aca+"', modelimage='', cell='"+str(cell_aca)+"', imsize="+str(imsize_aca)+", imdirection='"+imdirection+"', niter="+str(niter)+", threshold='"+threshold+"', weighting='"+weighting+"', mask="+str([])+", outertaper="+str([])+", stokes='I', analyze="+str(True)+", graphics='"+graphics+"', verbose="+str(verbose)+", overwrite="+str(overwrite)+")"
                msg("Executing: "+taskstr, origin="simalma", priority=t_priority)

                simanalyze(project=project, image=image,
                           vis=vis_aca, modelimage="",
                           cell=cell_aca, imsize=imsize_aca,
                           imdirection=imdirection, niter=niter,
                           threshold=threshold, weighting=weighting,
                           mask=[], outertaper=[], stokes='I',
                           analyze=True,
                           #showuv=None, showpsf=None, showmodel=None,
                           #showconvolved=None, showclean=None,
                           #showresidual=None, showdifference=None,
                           #showfidelity=None,
                           graphics=graphics, verbose=verbose,
                           overwrite=overwrite)#, async=False)
                if not os.path.exists(fileroot+"/"+imagename_aca):
                    msg("ACA model image '%s' is not found" \
                        % imagename_aca, origin="simalma", priority="error")
                modelimage = imagename_aca
            ############################################################
            # Image ALMA-BL
            step += 1
            msg("="*60, origin="simalma", priority="warn")
            if acaratio > 0:
                msg(" Step %d: generating ALMA 12-m + ACA image." % step, origin="simalma", priority="warn")
            else:
                msg(" Step %d: generating ALMA 12-m array image." % step, origin="simalma", priority="warn")
            msg("="*60, origin="simalma", priority="warn")

            if os.path.exists(fileroot+"/"+msname_bl):
                vis_bl = msname_bl
            else:
                msg("Could not find MS to image, '%s'" \
                    % msname_bl, origin="simalma", priority="error")

            taskstr = "simanalyze(project='"+ project+"', image="+str(image)+", vis='"+ vis_bl+"', modelimage='"+ modelimage+"', cell='"+str(cell)+"', imsize="+str(imsize)+", imdirection='"+ imdirection+"', niter="+str(niter)+", threshold='"+ threshold+"', weighting='"+ weighting+"', mask="+str([])+", outertaper="+str([])+", stokes='I', analyze="+str(True)+", graphics='"+ graphics+"', verbose="+str(verbose)+", overwrite="+ str(overwrite)+")"
            msg("Executing: "+taskstr, origin="simalma", priority=t_priority)

            simanalyze(project=project, image=image,
                       vis=vis_bl, modelimage=modelimage,
                       cell=cell, imsize=imsize, imdirection=imdirection,
                       niter=niter, threshold=threshold, weighting=weighting,
                       mask=[], outertaper=[], stokes='I',
                       analyze=True,
                       #showuv=None, showpsf=None, showmodel=None,
                       #showconvolved=None, showclean=None,
                       #showresidual=None, showdifference=None,
                       #showfidelity=None,
                       graphics=graphics, verbose=verbose,
                       overwrite=overwrite)#, async=False)
            

    except TypeError, e:
        finalize_tools()
        casalog.post("simalma -- TypeError: %s" % str(e), priority="ERROR")
        raise TypeError, e
        return False
    except ValueError, e:
        finalize_tools()
        casalog.post("simalma -- OptionError: %s" % str(e), priority="ERROR")
        raise ValueError, e
        return False
    except Exception, instance:
        finalize_tools()
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

