from taskinit import *
from simutil import *
import os
import re
import pylab as pl
import pdb
from sdimagingold import sdimagingold
from imregrid import imregrid
from immath import immath

def simanalyze(
    project=None,
    image=None,
    # if image==False:
    imagename=None, 
    skymodel=None,
    # else:
    vis=None, modelimage=None, imsize=None, imdirection=None, cell=None,
    interactive=None, niter=None, threshold=None,
    weighting=None, mask=None, outertaper=None, pbcor=None, stokes=None,
    featherimage=None,
    # endif
    analyze=None,
    showuv=None, showpsf=None, showmodel=None,
    showconvolved=None, showclean=None, showresidual=None, showdifference=None,
    showfidelity=None,
    graphics=None,
    verbose=None,
    overwrite=None,
    dryrun=False,
    logfile=None
    ):

#def simanalyze(project='sim', image=True, imagename='default', skymodel='', vis='default', modelimage='', imsize=[128, 128], imdirection='', cell='', interactive=False, niter=0, threshold='0.1mJy', weighting='natural', mask=[], outertaper=[''], stokes='I', featherimage='', analyze=False, showuv=True, showpsf=True, showmodel=True, showconvolved=False, showclean=True, showresidual=False, showdifference=True, showfidelity=True, graphics='both', verbose=False, overwrite=True, dryrun=False):


    # Collect a list of parameter values to save inputs
    in_params =  locals()

    import re
    import glob

    casalog.origin('simanalyze')
    if verbose: casalog.filter(level="DEBUG2")

    a = inspect.stack()
    stacklevel = 0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel = k
    myf = sys._getframe(stacklevel).f_globals
    
    # create the utility object:    
    myutil = simutil()
    if logfile: 
        myutil.reportfile=logfile
        myutil.openreport()
    if verbose: myutil.verbose = True
    msg = myutil.msg
    from simutil import is_array_type

    # put output in directory called "project"
    fileroot = project
    if not os.path.exists(fileroot):
        msg(fileroot+" directory doesn't exist - the task expects to find results from creating the datasets there, like the skymodel.",priority="error")
        # msg should raise an exception for priority=error


    saveinputs = myf['saveinputs']
    saveinputs('simanalyze',fileroot+"/"+project+".simanalyze.last")
#               myparams=in_params)

    if (not image) and (not analyze):
        casalog.post("No operation to be done. Exiting from task.", "WARN")
        return True

    grscreen = False
    grfile = False
    if graphics == "both":
        grscreen = True
        grfile = True
    if graphics == "screen":
        grscreen = True
    if graphics == "file":
        grfile = True
    
    try:

        # Predefined parameters 
        pbcoeff = 1.13 ##  PB defined as pbcoeff*lambda/d


        # handle '$project' in modelimage
        modelimage = modelimage.replace('$project',project)
        featherimage = featherimage.replace('$project',project)

        #=========================
        # things we need: model_cell, model_direction if user doesn't specify - 
        # so find those first, and get information using util.modifymodel
        # with skymodel=newmodel


        # we need to parse either the mslist or imagename (if image=False) 
        # first, so that we can pick the appropriate skymodel, 
        # if there are several.
        skymodel_searchstring="NOT SPECIFIED"

        if not (image or dryrun):
            user_imagename=imagename
            if user_imagename=="default" or len(user_imagename)<=0:
                images= glob.glob(fileroot+"/*image")
                if len(images)<1:
                    msg("can't find any image in project directory",priority="error")
                    return False
                if len(images)>1:
                    msg("found multiple images in project directory",priority="warn")
                    msg("using  "+images[0],priority="warn")
                imagename=images[0]
            # trim .image suffix:
            imagename= imagename.replace(".image","")
            
            # if the user hasn't specified a sky model image, we can try to 
            # see if their imagename contains something like the project and 
            # configuration, as it would if simobserve created it.
            user_skymodel=skymodel
            if not os.path.exists(user_skymodel):
                if os.path.exists(fileroot+"/"+user_skymodel):
                    user_skymodel=fileroot+"/"+user_skymodel
                elif len(user_skymodel)>0:
                    raise Exception,"Can't find your specified skymodel "+user_skymodel
            # try to strip a searchable identifier
            tmpstring=user_skymodel.split("/")[-1]
            skymodel_searchstring=tmpstring.replace(".image","")
            


        if image:
            # check for default measurement sets:
            default_mslist = glob.glob(fileroot+"/*ms")
            n_default=len(default_mslist)
            # is the user requesting this ms?
            default_requested=[]
            for i in range(n_default): default_requested.append(False)


            # parse ms parameter and check for existance;
            # initial ms list
            if vis=="default" or len(vis)==0:
                mslist0=default_mslist
            else:
                mslist0 = vis.split(',')
            # verified found ms list
            mslist = []
            mstype = []
            
            mstoimage=[]
            tpmstoimage=None

            for ms0 in mslist0:
                if not len(ms0): continue
                ms1 = ms0.replace('$project',project)

                # MSes in fileroot/ have priority
                if os.path.exists(fileroot+"/"+ms1):
                    ms1 = fileroot + "/" + ms1

                if os.path.exists(ms1)or dryrun:
                    mslist.append(ms1)

                    # mark as requested
                    if default_mslist.count(ms1):
                        i=default_mslist.index(ms1)
                        default_requested[i]=True

                    # check if noisy in name
                    if re.search('noisy.',ms1):
                        ms1_raw=str.join("",re.split('noisy.',ms1))
                        if default_mslist.count(ms1_raw):
                            i=default_mslist.index(ms1_raw)
                            default_requested[i]=True
                    else: # not noisy
                        if ms1.endswith(".sd.ms"):
                            ms1_noisy=re.split('.sd.ms',ms1)[0]+'.noisy.sd.ms'
                        else:
                            ms1_noisy=re.split('.ms',ms1)[0]+'.noisy.ms'
                        if default_mslist.count(ms1_noisy):
                            i=default_mslist.index(ms1_noisy)
                            default_requested[i]=True
                            if vis == "default": continue
                            msg("You requested "+ms1+" but there is a noisy version of the ms in your project directory - if your intent is to model noisy data you may want to check inputs",priority="warn")

                    # check if the ms is tp data or not.
                    if dryrun:
                        # HACK
                        mstype.append('INT')
                        mstoimage.append(ms1)

                    elif myutil.ismstp(ms1,halt=False):
                        mstype.append('TP')
                        tpmstoimage = ms1
                        # XXX TODO more than one TP ms will not be handled
                        # correctly
                        msg("Found a total power measurement set, %s." % ms1,origin='simanalyze')
                    else:
                        mstype.append('INT')
                        mstoimage.append(ms1)
                        msg("Found a synthesis measurement set, %s." % ms1,origin='simanalyze')
                else:          
                    msg("measurement set "+ms1+" not found -- removing from imaging list")
            
            # check default mslist for unrequested ms:
            for i in range(n_default):
                if not default_requested[i]:
                    msg("Project directory contains "+default_mslist[i]+" but you have not requested to include it in your simulated image.")


            if not mstoimage  and len(tpmstoimage) == 0:
                raise Exception,"No MS found to image"

            # now try to parse the mslist for an identifier string that 
            # we can use to find the right skymodel if there are several
            if len(mstoimage) == 0 and len(tpmstoimage) > 0:
                tmpstring = tpmstoimage.split("/")[-1]
            else:
                tmpstring=(mstoimage[0]).split("/")[-1]
            skymodel_searchstring=tmpstring.replace(".ms","")



            # more than one to image?
            if len(mstoimage) > 1:
                msg("Multiple interferometric ms found:",priority="info",origin='simanalyze')
                for i in range(len(mstoimage)):
                    msg(" "+mstoimage[i],priority="info",origin='simanalyze')
                msg(" will be concated and simultaneously deconvolved; if something else is desired, please specify vis, or image manually and use image=F",priority="info",origin='simanalyze')
                concatms=project+"/"+project+".concat.ms"
                from concat import concat
                weights = get_concatweights(mstoimage)
                msg(" concat('"+mstoimage+"',concatvis='"+concatms+"',visweightscale="+str(weights)+")",origin='simanalyze')
                if not dryrun:
                    concat(mstoimage,concatvis=concatms,visweightscale=weights)
                    mstoimage=[concatms]



        #========================================================
        # now we can search for skymodel, and if there are several, 
        # pick the one that is closest to either the imagename, 
        # or the first MS if there are several MS to image.

        components_only=False

        # first look for skymodel, if not then compskymodel
        skymodels=glob.glob(fileroot+"/"+project+"*.skymodel")+glob.glob(fileroot+"/"+project+"*.newmodel")
        nmodels=len(skymodels)
        skymodel_index=0
        if nmodels>1:
            msg("Found %i sky model images:" % nmodels,origin='simanalyze')
            # use the skymodel_searchstring to try to pick the right one
            # print them out for the user while we're at it.
            for i in range(nmodels):
                msg("   "+skymodels[i])
                if skymodels[i].count(skymodel_searchstring)>0:
                    skymodel_index=i
            msg("Using skymodel "+skymodels[skymodel_index],origin='simanalyze')
        if nmodels>=1:
            skymodel=skymodels[skymodel_index]
        else:
            skymodel=""
        
        if os.path.exists(skymodel):
            msg("Sky model image "+skymodel+" found.",origin='simanalyze')
        else:
            skymodels=glob.glob(fileroot+"/"+project+"*.compskymodel")
            nmodels=len(skymodels)
            if nmodels>1:
                msg("Found %i sky model images:" % nmodels,origin='simanalyze')
                for ff in skymodels:
                    msg("   "+ff)
                msg("Using "+skymodels[0],origin='simanalyze')
            if nmodels>=1:
                skymodel=skymodels[0]
            else:
                skymodel=""

            if os.path.exists(skymodel):
                msg("Sky model image "+skymodel+" found.",origin='simanalyze')
                components_only=True
            elif not dryrun:
                msg("Can't find a model image in your project directory, named skymodel or compskymodel - output image will be created, but comparison with the input model is not possible.",priority="warn",origin='simanalyze')
                analyze=False

        modelflat = skymodel+".flat"

        if os.path.exists(skymodel):
            if not (os.path.exists(modelflat) or dryrun):
                myutil.flatimage(skymodel,verbose=verbose)
            
            # modifymodel just collects info if skymodel==newmodel
            (model_refdir,model_cell,model_size,
             model_nchan,model_center,model_width,
             model_stokes) = myutil.modifymodel(skymodel,skymodel,
                                              "","","","","",-1,
                                              flatimage=False)
            
            cell_asec=qa.convert(model_cell[0],'arcsec')['value']


        #####################################################################
        # clean if desired, use noisy image for further calculation if present
        # todo suggest a cell size from psf?
        #####################################################################
        beam_current = False
        if image:

            # make sure cell is defined
            if is_array_type(cell):
                if len(cell) > 0:
                    cell0 = cell[0]
                else:
                    cell0 = ""
            else:
                cell0 = cell
            if len(cell0)<=0:
                cell = model_cell
            if is_array_type(cell):
                if len(cell) == 1:
                    cell = [cell[0],cell[0]]
            else:
                cell = [cell,cell]
            
            # cells are positive by convention
            cell = [qa.abs(cell[0]),qa.abs(cell[1])]
            
            # and imsize
            if is_array_type(imsize):
                if len(imsize) > 0:
                    imsize0 = imsize[0]
                    if len(imsize) > 1:
                        imsize1 = imsize[1]
                    else:
                        imsize1 = imsize0
                else:
                    imsize0 = -1
            else:
                imsize0 = imsize
            if imsize0 <= 0:
                imsize = [int(pl.ceil(qa.convert(qa.div(model_size[0],cell[0]),"")['value'])),
                          int(pl.ceil(qa.convert(qa.div(model_size[1],cell[1]),"")['value']))]
            else:
                imsize=[imsize0,imsize1]


            if len(mstoimage) == 0:
                if tpmstoimage:
                    sd_only = True
                else:
                    msg("no measurement sets found to image",priority="error",origin='simanalyze')
            else:
                sd_only = False
                # get some quantities from the interferometric ms
                # TODO use something like aU.baselineStats for this, and the 90% baseline
                maxbase=0.
                if len(mstoimage)>1 and dryrun:
                    msg("imaging multiple ms not possible in dryrun mode",priority="warn",origin="simanalyze")
                # TODO make work better for multiple MS
                for msfile in mstoimage:
                    if os.path.exists(msfile):
                        tb.open(msfile)
                        rawdata = tb.getcol("UVW")
                        tb.done()
                        maxbase = max([max(rawdata[0,]),max(rawdata[1,])])  # in m
                        psfsize = 0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/maxbase*3600.*180/pl.pi # lambda/b converted to arcsec
                        minimsize = 8* int(psfsize/cell_asec)
                    elif dryrun:
                        minimsize = min(imsize)
                        psfsize = qa.mul(cell[0],3) # HACK
                    else:
                        raise Exception,mstoimage+" not found."

                if imsize[0] < minimsize:
                    msg("The number of image pixel in x-axis, %d, is small to cover 8 x PSF. Setting x pixel number, %d." % (imsize[0], minimsize), priority='warn',origin='simanalyze')
                    imsize[0] = minimsize
                if imsize[1] < minimsize:
                    msg("The number of image pixel in y-axis, %d, is small to cover 8 x PSF. Setting y pixel number, %d" % (imsize[1], minimsize), priority='warn',origin='simanalyze')
                    imsize[1] = minimsize

            tpimage=None
            # Do single dish imaging first if tpmstoimage exists.
            if tpmstoimage and os.path.exists(tpmstoimage):
                msg('creating image from ms: '+tpmstoimage,origin='simanalyze')
                #if len(mstoimage):
                #    tpimage = project + '.sd.image'
                #else:
                #    tpimage = project + '.image'
                tpimage = project + '.sd.image'
                tpimage = fileroot + "/" + tpimage

                if len(mstoimage):
                    if len(modelimage) and tpimage != modelimage and \
                           tpimage != fileroot+"/"+modelimage:
                        msg("modelimage parameter set to "+modelimage+" but also creating a new total power image "+tpimage,priority="warn",origin='simanalyze')
                        msg("assuming you know what you want, and using modelimage="+modelimage+" in deconvolution",priority="warn",origin='simanalyze')
                    elif len(featherimage) and tpimage != featherimage and \
                           tpimage != fileroot+"/"+featherimage:
                        msg("featherimage parameter set to "+featherimage+" but also creating a new total power image "+tpimage,priority="warn",origin='simanalyze')
                        msg("assuming you know what you want, and using featherimage="+featherimage+" in feather",priority="warn",origin='simanalyze')
#                    else:
#                        # This forces to use TP image as a model for clean
#                        if len(modelimage) <= 0:
#                            msg("you are generating total power image "+tpimage+". this is used as a model image for clean",priority="warn")
#                        modelimage = tpimage
                
                # Get PB size of TP Antenna
                # !! aveant will only be set if modifymodel or setpointings and in 
                # any case it will the the aveant of the INTERFM array - we want the SD
                if os.path.exists(tpmstoimage):
                    tb.open(tpmstoimage+"/ANTENNA")
                    diams = tb.getcol("DISH_DIAMETER")
                    tb.done()
                elif dryrun:
                    # HACK
                    diams=[12.]
                else:
                    raise Exception, tpmstoimage+" not found."

                aveant = pl.mean(diams)
                # model_center should be set even if we didn't predict this execution
                pb_asec = pbcoeff*0.29979/qa.convert(qa.quantity(model_center),'GHz')['value']/aveant*3600.*180/pl.pi # arcsec
                # default PSF from PB of antenna
                imbeam = {'major': qa.quantity(pb_asec,'arcsec'),
                          'minor': qa.quantity(pb_asec,'arcsec'),
                          'positionangle': qa.quantity(0.0,'deg')}
                
                if True: #SF gridding
                    msg("Generating TP image using 'SF' kernel.",origin='simanalyze')
                    beamsamp = 6.42857
                    sfcell_asec = pb_asec/beamsamp
                    sfcell = qa.tos(qa.quantity(sfcell_asec, "arcsec"))
                    cell_asec = [qa.convert(cell[0],"arcsec")['value'],
                                 qa.convert(cell[1],"arcsec")['value']]
                    if cell_asec[0] > sfcell_asec or \
                           cell_asec[1] > sfcell_asec:
                        # imregrid() may not work properly for regrid of
                        # small to large cell
                        msg("The requested cell size is too large to invoke SF gridding. Please set cell size <= %f arcsec or grid TP MS '%s' manually" % (sfcell_asec, tpmstoimage),priority="error",origin='simanalyze')

                    sfsupport = 4
                    temp_out = tpimage+"0"
                    temp_cell = [sfcell, sfcell]
                    # too small - is imsize too small to start with?
                    # needs to cover all pointings.
                    temp_imsize = [int(pl.ceil(cell_asec[0]/sfcell_asec*imsize[0])),
                                   int(pl.ceil(cell_asec[1]/sfcell_asec*imsize[1]))]
                    msg("Using predefined algorithm to define grid parameters.",origin='simanalyze')
                    msg("SF gridding summary",origin='simanalyze')
                    msg("- Antenna primary beam: %f arcsec" % pb_asec,origin='simanalyze')
                    msg("- Image pixels per antenna PB (predefined): %f" % beamsamp,origin='simanalyze')
                    msg("- Cell size (arcsec): [%s, %s]" % (temp_cell[0], temp_cell[1]),origin='simanalyze')
                    msg("- Imsize to cover final TP image area: [%d, %d] (type: %s)" % (temp_imsize[0], temp_imsize[1], type(temp_imsize[0])),origin='simanalyze')
                    msg("- convolution support: %d" % sfsupport,origin='simanalyze')
                    msg("sdimagingold(infiles=['"+tpmstoimage+"'], gridfunction='SF', convsupport = "+str(sfsupport)+
                        ",outfile='"+temp_out+"', overwrite="+str(overwrite)+
                        ",imsize="+str(temp_imsize)+", cell="+str(temp_cell)+
                        ",phasecenter='"+model_refdir+"', dochannelmap=True,"+
                        "nchan="+str(model_nchan)+", start=0, step=1, spw=[0])",priority="info")
                    if not dryrun:
                        sdimagingold(infiles=[tpmstoimage], gridfunction='SF',
                              convsupport = sfsupport,
                              outfile=temp_out, overwrite=overwrite,
                              imsize=temp_imsize, cell=temp_cell,
                              phasecenter=model_refdir, dochannelmap=True,
                              nchan=model_nchan, start=0, step=1, spw=[0])
                        if not os.path.exists(temp_out):
                            raise RuntimeError, "TP imaging failed."

                        # Define PSF of image
                        qpb = qa.quantity(pb_asec,"arcsec")
                        # TODO get the sampling from the ms and put it in here
                        qpsf0 = myutil.sfBeam1d(qpb, cell=temp_cell[0],
                                              convsupport=sfsupport)
                        qpsf1 = myutil.sfBeam1d(qpb, cell=temp_cell[1],
                                              convsupport=sfsupport)
                        imbeam['major'] = max(qpsf0, qpsf1)
                        imbeam['minor'] = min(qpsf0, qpsf1)
                        imbeam['positionangle'] = qa.quantity(pl.arctan(qa.getvalue(qa.div(qpsf1,qpsf0))), "rad")
                        # Scale image by convolved beam / antenna primary beam
                        beam_area_ratio = qa.getvalue(qa.convert(qpsf0, "arcsec")) \
                                          * qa.getvalue(qa.convert(qpsf1, "arcsec")) \
                                          / pb_asec**2
                        msg("Scaling TP image intensity by %f." % (beam_area_ratio),origin='simanalyze')
                        temp_in = temp_out
                        temp_out = temp_out + ".scaled"
                        immath(imagename=temp_in, mode='evalexpr', expr="IM0*%f" % (beam_area_ratio),
                               outfile=temp_out)
                        if not os.path.exists(temp_out):
                            raise RuntimeError, "TP image scaling failed."
                        ia.open(temp_out)
                        beam = ia.restoringbeam()
                        if len(beam) == 0: ia.setrestoringbeam(beam=imbeam)
                        ia.close()
                        
                    # Regrid TP image to final resolution
                    msg("Regridding TP image to final resolution",origin='simanalyze')
                    msg("- cell size (arecsec): [%s, %s]" % (cell[0], cell[1]),origin='simanalyze')
                    msg("- imsize: [%d, %d]" % (imsize[0], imsize[1]),origin='simanalyze')
                    if not dryrun:
                        ia.open(temp_out)
                        newcsys = ia.coordsys()
                        ia.close()
                        dir_idx = newcsys.findcoordinate("direction")['world']
                        newcsys.setreferencepixel([imsize[0]/2., imsize[1]/2.],
                                                  type="direction")
                        incr = newcsys.increment(type='direction')['numeric']
                        newincr = [incr[0]*cell_asec[0]/sfcell_asec,
                                   incr[1]*cell_asec[1]/sfcell_asec,]
                        newcsys.setincrement(newincr, type="direction")
                        #
                        sdtemplate = imregrid(imagename=temp_out, template="get")
                        sdtemplate['csys'] = newcsys.torecord()
                        for idx in range(len(dir_idx)):
                            sdtemplate['shap'][ dir_idx[idx] ] = imsize[idx]
                        imregrid(imagename=temp_out, interpolation="cubic",
                             template=sdtemplate, output=tpimage,
                             overwrite=overwrite)
                        del newcsys, sdtemplate, incr, newincr, dir_idx
                        del temp_out, temp_cell, temp_imsize, sfcell_asec, cell_asec
                else: #PB grid
                    msg("Generating TP image using 'PB' kernel.",origin='simanalyze')
                    # Final TP cell and image size.
                    # imsize and cell are already int and quantum arrays
                    sdimsize = imsize
                    sdcell = [qa.tos(cell[0]), qa.tos(cell[1])]
                    ### TODO: need to set phasecenter properly based on imdirection
                    msg("sdimagingold(infiles=['"+tpmstoimage+"'], gridfunction='PB'"+
                        ",outfile='"+temp_out+"', overwrite="+str(overwrite)+
                        ",imsize="+str(sdimsize)+", cell="+str(sdcell)+
                        ",phasecenter='"+model_refdir+"', dochannelmap=True,"+
                        "nchan="+str(model_nchan)+", start=0, step=1, spw=[0])",priority="info")
                    if not dryrun:
                        sdimagingold(infiles=[tpmstoimage],gridfunction='PB',
                              outfile=tpimage, overwrite=overwrite,
                              imsize=sdimsize, cell=sdcell,
                              phasecenter=model_refdir, dochannelmap=True,
                              nchan=model_nchan,start=0,step=1,spw=[0])
                    del sdimsize, sdcell
                    # TODO: Define PSF of image here
                    # for now use default 

                # For single dish: manually set the primary beam
                beam=None
                if os.path.exists(tpimage):
                    ia.open(tpimage)
                    beam = ia.restoringbeam()
                if len(beam) == 0:
                    msg('setting primary beam information to image.',origin='simanalyze')
                    beam['major'] = imbeam['major']
                    beam['minor'] = imbeam['minor']
                    beam['positionangle'] = imbeam['positionangle']
                    msg('Primary beam: '+str(beam['major']),origin='simanalyze')
                    if ia.isopen():
                        ia.setrestoringbeam(beam=beam)
                        ia.close()

                if sd_only:
                    beam_current = True
                    bmarea = beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
                    bmarea = bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
                else: del beam
                #del beam

                msg('generation of total power image '+tpimage+' complete.',origin='simanalyze')
                # update TP ms name the for following steps
                sdmsfile = tpmstoimage
                sd_any = True
                
                imagename = re.split('.image$',tpimage)[0]
                # End of single dish imaging part

        outflat_current = False
        convsky_current = False

        if image and len(mstoimage) > 0:

            # for reruns
            foo=mstoimage[0]
            foo=foo.replace(".ms","")
            foo=foo.replace(project,"")
            foo=foo.replace("/","")
            project=project+foo

            imagename = fileroot + "/" + project

            # get nfld, sourcefieldlist, from (interfm) ms if it was not just created
            # TODO make work better for multiple mstoimage (figures below)
            if os.path.exists(mstoimage[0]):
                tb.open(mstoimage[0]+"/SOURCE")
                code = tb.getcol("CODE")
                sourcefieldlist = pl.where(code=='OBJ')[0]
                nfld = len(sourcefieldlist)
                tb.done()
            elif dryrun:
                nfld=1 # HACK
            msfile = mstoimage[0]

            # set cleanmode automatically (for interfm)
            if nfld == 1:
                cleanmode = "csclean"
            else:
                cleanmode = "mosaic"




            # clean insists on using an existing model if its present
            if os.path.exists(imagename+".image"): shutil.rmtree(imagename+".image")
            if os.path.exists(imagename+".model"): shutil.rmtree(imagename+".model")

            # An image in fileroot/ has priority
            if len(modelimage) > 0 and os.path.exists(fileroot+"/"+modelimage):
                modelimage = fileroot + "/" + modelimage
                msg("Found modelimage, %s." % modelimage,origin='simanalyze')

            # in simdata we use imdirection instead of model_refdir
            if not myutil.isdirection(imdirection,halt=False):
                imdirection=model_refdir
        
            myutil.imclean(mstoimage,imagename,
                         cleanmode,cell,imsize,imdirection,
                         interactive,niter,threshold,weighting,
                         outertaper,pbcor,stokes, #sourcefieldlist=sourcefieldlist,
                         modelimage=modelimage,mask=mask,dryrun=dryrun)


            # create imagename.flat and imagename.residual.flat:
            if not dryrun:
                myutil.flatimage(imagename+".image",verbose=verbose)
                myutil.flatimage(imagename+".residual",verbose=verbose)
            outflat_current = True

            # feather
            if featherimage:
                if not os.path.exists(featherimage):
                    raise Exception,"Could not find featherimage "+featherimage
            else:
                featherimage=""
                if tpimage:
                    # if you set modelimage, then it won't force tpimage into 
                    # featherimage.  this could be hard to explain 
                    # to the user.
                    if os.path.exists(tpimage) and not os.path.exists(modelimage):
                        featherimage=tpimage
                    

            if os.path.exists(featherimage):
                msg("feathering the interfermetric image "+imagename+".image with "+featherimage,origin='simanalyze',priority="info")
                from feather import feather 
                # TODO call with params?
                msg("feather('"+imagename+".feather.image','"+imagename+".image','"+featherimage+"')",priority="info")
                if not dryrun:
                    feather(imagename+".feather.image",imagename+".image",featherimage)
                    # copy residual flat image
                    shutil.copytree(imagename+".residual.flat",imagename+".feather.residual.flat")
                    imagename=imagename+".feather"
                    # but replace combined flat image
                    myutil.flatimage(imagename+".image",verbose=verbose)



            if verbose:
                msg(" ")
            msg("done inverting and cleaning",origin='simanalyze')
            if not is_array_type(cell):
                cell = [cell,cell]
            if len(cell) <= 1:
                cell = [qa.quantity(cell[0]),qa.quantity(cell[0])]
            else:
                cell = [qa.quantity(cell[0]),qa.quantity(cell[1])]
            cell = [qa.abs(cell[0]),qa.abs(cell[0])]

            # get beam from output clean image
            if verbose: msg("getting beam from "+imagename+".image",origin='simanalyze')
            if os.path.exists(imagename+".image"):
                ia.open(imagename+".image")
                beam = ia.restoringbeam()
                beam_current = True
                ia.close()
                # model has units of Jy/pix - calculate beam area from clean image
                # (even if we are not plotting graphics)
                bmarea = beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
                bmarea = bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
                msg("synthesized beam area in output pixels = %f" % bmarea,origin='simanalyze')










        if image:
            # show model, convolved model, clean image, and residual
            if grfile:
                file = fileroot + "/" + project + ".image.png"
            else:
                file = ""
        else:
            mslist=[]

        if dryrun:
            grscreen=False
            grfile=False
            analyze=False

        if image and len(mstoimage) > 0:
            if grscreen or grfile:
                myutil.newfig(multi=[2,2,1],show=grscreen)

                # create regridded and convolved sky model image
                myutil.convimage(modelflat,imagename+".image.flat")
                convsky_current = True # don't remake this for analysis in this run

                disprange = []  # passing empty list causes return of disprange

                # original sky regridded to output pixels but not convolved with beam
                discard = myutil.statim(modelflat+".regrid",disprange=disprange,showstats=False)
                myutil.nextfig()

                # convolved sky model - units of Jy/bm
                disprange = []
                discard = myutil.statim(modelflat+".regrid.conv",disprange=disprange)
                myutil.nextfig()

                # clean image - also in Jy/beam
                # although because of DC offset, better to reset disprange
                disprange = []
                discard = myutil.statim(imagename+".image.flat",disprange=disprange)
                myutil.nextfig()

                if len(mstoimage) > 0:
                    myutil.nextfig()

                    # clean residual image - Jy/bm
                    discard = myutil.statim(imagename+".residual.flat",disprange=disprange)
                myutil.endfig(show=grscreen,filename=file)




        #####################################################################
        # analysis

        if analyze:

            if not os.path.exists(imagename+".image"):
                if os.path.exists(fileroot+"/"+imagename+".image"):
                    imagename=fileroot+"/"+imagename
                else:
                    msg("Can't find a simulated image - expecting "+imagename,priority="error")
                    return False

            # we should have skymodel.flat created above 

            if not image:
                if not os.path.exists(imagename+".image"):
                    msg("you must image before analyzing.",priority="error")
                    return False

                # get beam from output clean image
                if verbose: msg("getting beam from "+imagename+".image",origin="analysis")
                ia.open(imagename+".image")
                beam = ia.restoringbeam()
                beam_current = True
                ia.close()
                # model has units of Jy/pix - calculate beam area from clean image
                cell = myutil.cellsize(imagename+".image")
                cell= [ qa.convert(cell[0],'arcsec'),
                        qa.convert(cell[1],'arcsec') ]
                # (even if we are not plotting graphics)
                bmarea = beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
                bmarea = bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
                msg("synthesized beam area in output pixels = %f" % bmarea)


            # flat output:?  if the user manually cleaned, this may not exist
            outflat = imagename + ".image.flat"
            if (not outflat_current) or (not os.path.exists(outflat)):
                # create imagename.flat and imagename.residual.flat
                myutil.flatimage(imagename+".image",verbose=verbose)
                if os.path.exists(imagename+".residual"):
                    myutil.flatimage(imagename+".residual",verbose=verbose)
                else:
                    if showresidual:
                        msg(imagename+".residual not found -- residual will not be plotted",priority="warn")
                    showresidual = False
                outflat_current = True

            # regridded and convolved input:?
            if not convsky_current:
                myutil.convimage(modelflat,imagename+".image.flat")
                convsky_current = True

            # now should have all the flat, convolved etc even if didn't run "image" 

            # make difference image.
            # immath does Jy/bm if image but only if ia.setbrightnessunit("Jy/beam") in convimage()
            convolved = modelflat + ".regrid.conv"
            difference = imagename + '.diff'
            diff_ia = ia.imagecalc(difference, "'%s' - '%s'" % (convolved, outflat), overwrite=True)
            diff_ia.setbrightnessunit("Jy/beam")

            # get rms of difference image for fidelity calculation
            #ia.open(difference)
            diffstats = diff_ia.statistics(robust=True, verbose=False,list=False)
            diff_ia.close()
            del diff_ia
            maxdiff = diffstats['medabsdevmed']
            if maxdiff != maxdiff: maxdiff = 0.
            if type(maxdiff) != type(0.):
                if maxdiff.__len__() > 0:
                    maxdiff = maxdiff[0]
                else:
                    maxdiff = 0.
            # Make fidelity image.
            absdiff = imagename + '.absdiff'
            calc_ia = ia.imagecalc(absdiff, "max(abs('%s'), %f)" % (difference,
                                                          maxdiff/pl.sqrt(2.0)), overwrite=True)
            calc_ia.close()
            fidelityim = imagename + '.fidelity'
            calc_ia = ia.imagecalc(fidelityim, "abs('%s') / '%s'" % (convolved, absdiff), overwrite=True)
            calc_ia.close()
            msg("fidelity image calculated",origin="analysis")

            # scalar fidelity
            absconv = imagename + '.absconv'
            calc_ia = ia.imagecalc(absconv, "abs('%s')" % convolved, overwrite=True)
            if ia.isopen(): ia.close() #probably not necessary
            calc_ia.close()
            del calc_ia

            ia.open(absconv)
            modelstats = ia.statistics(robust=True, verbose=False,list=False)
            maxmodel = modelstats['max']
            if maxmodel != maxmodel: maxmodel = 0.
            if type(maxmodel) != type(0.):
                if maxmodel.__len__() > 0:
                    maxmodel = maxmodel[0]
                else:
                    maxmodel = 0.
            ia.close()
            scalarfidel = maxmodel/maxdiff
            msg("fidelity range (max model / rms difference) = "+str(scalarfidel),origin="analysis")


            # now, what does the user want to actually display?

            # need MS for showuv and showpsf
            if not image:
                msfile = fileroot + "/" + project + ".ms"
            elif sd_only:
                # imaged and single dish only
                msfile = tpmstoimage
#            if sd_only and os.path.exists(sdmsfile):
#                # use TP ms for UV plot if only SD sim, i.e.,
#                # image=sd_only=T or (image=F=predict_uv and predict_sd=T)
#                msfile = sdmsfile
            # psf is not available for SD only sim
            if os.path.exists(msfile) and myutil.ismstp(msfile,halt=False):
                if showpsf: msg("single dish simulation -- psf will not be plotted",priority='warn')
                showpsf = False
            if (not image) and (not os.path.exists(msfile)):
                if showpsf or showuv:
                    msg("No image is generated in this run. Default MS, '%s', does not exist -- uv and psf will not be plotted" % msfile,priority='warn')
                showpsf = False
                showuv = False
            

            # if the order in the task input changes, change it here too
            figs = [showuv,showpsf,showmodel,showconvolved,showclean,showresidual,showdifference,showfidelity]
            nfig = figs.count(True)
            if nfig > 6:
                msg("only displaying first 6 selected panels in graphic output",priority="warn")
            if nfig <= 0:
                return True
            if nfig < 4:
                multi = [1,nfig,1]
            else:
                if nfig == 4:
                    multi = [2,2,1]
                else:
                    multi = [2,3,1]

            if grfile:
                file = fileroot + "/" + project + ".analysis.png"
            else:
                file = ""

            if grscreen or grfile:
                myutil.newfig(multi=multi,show=grscreen)

                # if order in task parameters changes, change here too
                if showuv:
# TODO loop over all ms - show all UV including zero
                    if len(mslist)>1:
                        msg("Using only "+msfile+" for uv plot",priority="warn",origin='simanalyze')
                    tb.open(msfile)
                    rawdata = tb.getcol("UVW")
                    tb.done()
                    pl.box()
                    maxbase = max([max(rawdata[0,]),max(rawdata[1,])])  # in m
                    klam_m = 300/qa.convert(model_center,'GHz')['value']
                    pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
                    pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
                    ax = pl.gca()
                    ax.yaxis.LABELPAD = -4
                    pl.xlabel('u[klambda]',fontsize='x-small')
                    pl.ylabel('v[klambda]',fontsize='x-small')
                    pl.axis('equal')
                    # Add zero-spacing (single dish) if not yet plotted
# TODO make this a check over all ms
#                    if predict_sd and not myutil.ismstp(msfile,halt=False):
#                        pl.plot([0.],[0.],'r,')
                    myutil.nextfig()

                if showpsf:
                    if image:
                        psfim = imagename + ".psf"
                    else:
                        psfim = project + ".quick.psf"
                        if not os.path.exists(psfim):
                            if len(mslist)>1:
                                msg("Using only "+msfile+" for psf generation",priority="warn")
                            im.open(msfile)
                            # TODO spectral parms
                            im.defineimage(cellx=qa.tos(model_cell[0]),nx=max([minimsize,128]))
                            if os.path.exists(psfim):
                                shutil.rmtree(psfim)
                            im.approximatepsf(psf=psfim)
                            # beam is set above (even in "analyze" only)
                            # note that if image, beam has fields 'major' whereas if not, it
                            # has fields like 'bmaj'.
                            # beam=im.fitpsf(psf=psfim)
                            im.done()
                    ia.open(psfim)
                    beamcs = ia.coordsys()
                    beam_array = ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'][0],beamcs.findcoordinate("stokes")['pixel'][0]],dropdeg=True)
                    nn = beam_array.shape
                    xextent = nn[0]*cell_asec*0.5
                    xextent = [xextent,-xextent]
                    yextent = nn[1]*cell_asec*0.5
                    yextent = [-yextent,yextent]
                    flipped_array = beam_array.transpose()
                    ttrans_array = flipped_array.tolist()
                    ttrans_array.reverse()
                    pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
                    psfim.replace(project+"/","")
                    pl.title(psfim,fontsize="x-small")
                    b = qa.convert(beam['major'],'arcsec')['value']
                    pl.xlim([-3*b,3*b])
                    pl.ylim([-3*b,3*b])
                    ax = pl.gca()
                    pl.text(0.05,0.95,"bmaj=%7.1e\nbmin=%7.1e" % (beam['major']['value'],beam['minor']['value']),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
                    ia.close()
                    myutil.nextfig()

                disprange = []  # first plot will define range
                if showmodel:
                    discard = myutil.statim(modelflat+".regrid",incell=cell,disprange=disprange,showstats=False)
                    myutil.nextfig()
                    disprange = []

                if showconvolved:
                    discard = myutil.statim(modelflat+".regrid.conv")
                    # if disprange gets set here, it'll be Jy/bm
                    myutil.nextfig()

                if showclean:
                    # own scaling because of DC/zero spacing offset
                    discard = myutil.statim(imagename+".image.flat")
                    myutil.nextfig()

                if showresidual:
                    # it gets its own scaling
                    discard = myutil.statim(imagename+".residual.flat")
                    myutil.nextfig()

                if showdifference:
                    # it gets its own scaling.
                    discard = myutil.statim(imagename+".diff")
                    myutil.nextfig()

                if showfidelity:
                    # it gets its own scaling.
                    discard = myutil.statim(imagename+".fidelity",showstats=False)
                    myutil.nextfig()

                myutil.endfig(show=grscreen,filename=file)

            sim_min,sim_max,sim_rms,sim_units = myutil.statim(imagename+".image.flat",plot=False)
            # if not displaying still print stats:
            # 20100505 ia.stats changed to return Jy/bm:
            msg('Simulation rms: '+str(sim_rms/bmarea)+" Jy/pix = "+
                str(sim_rms)+" Jy/bm",origin="analysis")
            msg('Simulation max: '+str(sim_max/bmarea)+" Jy/pix = "+
                str(sim_max)+" Jy/bm",origin="analysis")
            #msg('Simulation rms: '+str(sim_rms)+" Jy/pix = "+
            #    str(sim_rms*bmarea)+" Jy/bm",origin="analysis")
            #msg('Simulation max: '+str(sim_max)+" Jy/pix = "+
            #    str(sim_max*bmarea)+" Jy/bm",origin="analysis")
            msg('Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value']),origin="analysis")



        # cleanup - delete newmodel, newmodel.flat etc
        if os.path.exists(imagename+".image.flat"):
            shutil.rmtree(imagename+".image.flat")
        if os.path.exists(imagename+".residual.flat"):
            shutil.rmtree(imagename+".residual.flat")
        # .flux.pbcoverage is nessesary for feather.
        #if os.path.exists(imagename+".flux.pbcoverage"):
        #    shutil.rmtree(imagename+".flux.pbcoverage")
        absdiff = imagename + '.absdiff'
        if os.path.exists(absdiff):
            shutil.rmtree(absdiff)
        absconv = imagename + '.absconv'
        if os.path.exists(absconv):
            shutil.rmtree(absconv)
#        if os.path.exists(imagename+".diff"):
#            shutil.rmtree(imagename+".diff")
        if os.path.exists(imagename+".quick.psf") and os.path.exists(imagename+".psf"):
            shutil.rmtree(imagename+".quick.psf")

        finalize_tools()
        if myutil.isreport():
            myutil.closereport()


    except TypeError, e:
        finalize_tools()
        #msg("simanalyze -- TypeError: %s" % e,priority="error")
        casalog.post("simanalyze -- TypeError: %s" % e, priority="ERROR")
        raise TypeError, e
        return
    except ValueError, e:
        finalize_tools()
        #print "simanalyze -- OptionError: ", e
        casalog.post("simanalyze -- OptionError: %s" % e, priority="ERROR")
        raise ValueError, e
        return
    except Exception, instance:
        finalize_tools()
        #print '***Error***',instance
        casalog.post("simanalyze -- Exception: %s" % instance, priority="ERROR")
        raise Exception, instance
        return


def finalize_tools():
    if ia.isopen(): ia.close()
    im.close()
    tb.close()

### A helper function to get concat weight
def get_concatweights(mslist):
    from simutil import is_array_type
    if type(mslist) == str:
        mslist = [mslist]
    if not is_array_type(mslist):
        raise TypeError("get_concatweights: input should be a list of MS names")
    if len(mslist) < 2 and os.path.exists(mslist[0]):
        return (1.)

    if not os.path.exists(mslist[0]):
        raise ValueError("Could not find input file, %s" % mslist[0])
    # Get integration to compare.
    (mytb, mymsmd) = gentools(['tb', 'msmd'])
    try:
        mytb.open(mslist[0])
        tint0 = mytb.getcell('INTERVAL',0)
    finally:
        mytb.close()
    # Get antenna diameter of the first MS.
    try:
        mytb.open(mslist[0]+'/ANTENNA')
        diam0 = mytb.getcell('DISH_DIAMETER', 0)
    finally:
        mytb.close()
    
    weights = []
    for thems in mslist:
        if not os.path.exists(thems):
            raise ValueError("Could not find input file, %s" % thems)
        # MS check - assumes that all antennas, spws, and data are used
        # Check the number of spw
        try:
            mytb.open(thems+'/SPECTRAL_WINDOW')
            if mytb.nrows() > 1:
                casalog.post("simanalyze can not calculate concat weight of MSes with multiple spectral windows in an MS. Please concatenate them manually.", priority="ERROR")
        finally:
            mytb.close()

        # Check antenna diameters in MS
        try:
            mytb.open(thems+'/ANTENNA')
            diams = mytb.getcol('DISH_DIAMETER')
        finally:
            mytb.close()

        for diam in diams:
            if (diam != diams[0]):
                casalog.post("simanalyze can not calculate concat weight of MSes with multiple antenna diameters in an MS. Please concatenate them manually.", priority="ERROR")
        # Check integrations in MS
        try:
            mytb.open(thems)
            integs = mytb.getcol('INTERVAL')
        finally:
            mytb.close()

        for tint in integs:
            if (tint != integs[0]):
                casalog.post("simanalyze can not calculate concat weight of MSes with multiple integration times in an MS. Please concatenate them manually.", priority="ERROR")

        # Check integration is equal to tint0
        if integs[0] != tint0:
            casalog.post("simanalyze can not calculate concat weight of MSes with different integration times. Please concatenate them manually.", priority="ERROR")
        del integs
        # Now calculate weight
        weights.append((diams[0]/diam0)**2)

    # end of MS loop
    if len(mslist) != len(weights):
        raise RuntimeError("Could not calculate weight of some MSes.")

    return weights
    
