from taskinit import *
from simutil import *
import os
import re
import pylab as pl
import pdb

def simanalyze(
    project=None,
    image=None,
    imagename=None,
    vis=None, modelimage=None, cell=None, imsize=None, imdirection=None,
    niter=None, threshold=None,
    weighting=None, mask=None, outertaper=None, stokes=None,
    analyze=None,
    showuv=None, showpsf=None, showmodel=None,
    showconvolved=None, showclean=None, showresidual=None, showdifference=None,
    showfidelity=None,
    graphics=None,
    verbose=None,
    overwrite=None,
    async=False):

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
    util = simutil()
    if verbose: util.verbose = True
    msg = util.msg

    # put output in directory called "project"
    fileroot = project
    if not os.path.exists(fileroot):
        msg(fileroot+" directory doesn't exist - the task expects to find results from creating the datasets there, like the skymodel.",priority="error")
        return False


    saveinputs = myf['saveinputs']
    saveinputs('simanalyze',fileroot+"/"+project+".simanalyze.last",
               myparams=in_params)

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
#    if True:

        # handle '$project' in modelimage
        modelimage = modelimage.replace('$project',project)

# things we need: model_cell, model_direction if user doesn't specify - 
# so find those first, and get information using util.modifymodel
# with skymodel=newmodel

        components_only=False

        # first look for skymodel, if not then compskymodel
        skymodels=glob.glob(fileroot+"/"+project+"*.skymodel")+glob.glob(fileroot+"/"+project+"*.newmodel")
        nmodels=len(skymodels)
        if nmodels>1:
            msg("Found %i sky model images:" % nmodels)
            for ff in skymodels:
                msg("   "+ff)            
            msg("Using "+skymodels[0])
        if nmodels>=1:
            skymodel=skymodels[0]
        else:
            skymodel=""
        
        if os.path.exists(skymodel):
            msg("Sky model image "+skymodel+" found.")
        else:
            skymodels=glob.glob(fileroot+"/"+project+"*.compskymodel")
            nmodels=len(skymodels)
            if nmodels>1:
                msg("Found %i sky model images:" % nmodels)
                for ff in skymodels:
                    msg("   "+ff)
                msg("Using "+skymodels[0])
            if nmodels>=1:
                skymodel=skymodels[0]
            else:
                skymodel=""

            if os.path.exists(skymodel):
                msg("Sky model image "+skymodel+" found.")
                components_only=True
            else:
                msg("Can't find a model image in your project directory, named skymodel or compskymodel - output image will be been created, but comparison with the input model is not possible.",priority="error")
                return False

        modelflat = skymodel+".flat"
        if not os.path.exists(modelflat):
            util.flatimage(skymodel,verbose=verbose)

        # modifymodel just collects info if skymodel==newmodel
        returnpars = util.modifymodel(skymodel,skymodel,
                                      "","","","","",-1,
                                      flatimage=False)
        if not returnpars:
            return False

        (model_refdir,model_cell,model_size,
         model_nchan,model_center,model_width,
         model_stokes) = returnpars


        cell_asec=qa.convert(model_cell[0],'arcsec')['value']


        #####################################################################
        # clean if desired, use noisy image for further calculation if present
        # todo suggest a cell size from psf?
        #####################################################################
        if (not image):
            user_imagename=imagename
            if user_imagename=="default" or len(user_imagename)<=0:
                images= glob.glob(fileroot+"/*image")
                if len(images)<1:
                    msg("can't find any image in project directory",priority="error")
                    return False
                if len(images)>1:
                    msg("found multiple images in project directory")
                    msg("using  "+images[0])
                imagename=images[0]
            # trim .image suffix:
            imagename= imagename.replace(".image","")


        beam_current = False
        if image:

            # make sure cell is defined
            if type(cell) == type([]):
                if len(cell) > 0:
                    cell0 = cell[0]
                else:
                    cell0 = ""
            else:
                cell0 = cell
            if len(cell0) <= 0:
                cell = model_cell
            if type(cell) == type([]):
                if len(cell) == 1:
                    cell = [cell[0],cell[0]]
            else:
                cell = [cell,cell]
            
            # cells are positive by convention
            cell = [qa.abs(cell[0]),qa.abs(cell[1])]
            
            # and imsize
            if type(imsize) == type([]):
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

                if os.path.exists(ms1):
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
                            msg("You requested "+ms1+" but there is a corrupted (noisy) version of the ms in your project directory - if your intent is to model noisy data you may want to check inputs",priority="warn")

                    # check if the ms is tp data or not.
                    if util.ismstp(ms1,halt=False):
                        mstype.append('TP')
                        tpmstoimage = ms1
                        # XXX TODO more than one TP ms will not be handled
                        # correctly
                        msg("Found a total power measurement set, %s." % ms1)
                    else:
                        mstype.append('INT')
                        mstoimage.append(ms1)
                        msg("Found a synthesis measurement set, %s." % ms1)
                else:
                    if verbose:
                        msg("measurement set "+ms1+" not found -- removing from imaging list",priority="warn")

                    else:
                        msg("measurement set "+ms1+" not found -- removing from imaging list")
            
            # check default mslist for unrequested ms:
            if verbose:
                priority="warn"
            else:
                priority="info"
            for i in range(n_default):
                if not default_requested[i]:
                    msg("Project directory contains "+default_mslist[i]+" but you have not requested to include it in your simulated image.",priority=priority)



            # more than one to image?
            if len(mstoimage) > 1:
                msg("Multiple interferometric ms found:",priority="warn")
                for i in range(len(mstoimage)):
                    msg(" "+mstoimage[i],priority="warn")
                msg(" will be concated and simultaneously deconvolved; if something else is desired, please specify vis, or image manually and use image=F",priority="warn")
                concatms=project+"/"+project+".concat.ms"
                from concat import concat
                concat(mstoimage,concatms)
                mstoimage=[concatms]

            if len(mstoimage) == 0:
                if tpmstoimage:
                    sd_only = True
                else:
                    msg("no measurement sets found to image",priority="warn")
                    return False
            else:
                sd_only = False
                # get some quantities from the interferometric ms
                maxbase=0.
                # TODO make work better for multiple MS
                for msfile in mstoimage:
                    tb.open(msfile)
                    rawdata = tb.getcol("UVW")
                    tb.done()
                    maxbase = max([max(rawdata[0,]),max(rawdata[1,])])  # in m
                    psfsize = 0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/maxbase*3600.*180/pl.pi # lambda/b converted to arcsec
                    minimsize = 8* int(psfsize/cell_asec)

                if imsize[0] < minimsize:
                    msg("The number of image pixel in x-axis, %d, is small to cover 8 x PSF. Setting x pixel number, %d." % (imsize[0], minimsize), priority='warn')
                    imsize[0] = minimsize
                if imsize[1] < minimsize:
                    msg("The number of image pixel in y-axis, %d, is small to cover 8 x PSF. Setting y pixel number, %d" % (imsize[1], minimsize), priority='warn')
                    imsize[1] = minimsize


            # Do single dish imaging first if tpmstoimage exists.
            if tpmstoimage and os.path.exists(tpmstoimage):
                msg('creating image from ms: '+tpmstoimage)
                #if len(mstoimage):
                #    tpimage = project + '.sd.image'
                #else:
                #    tpimage = project + '.image'
                tpimage = project + '.sd.image'
                tpimage = fileroot + "/" + tpimage

                if len(mstoimage):
                    if len(modelimage) and tpimage != modelimage and \
                           tpimage != fileroot+"/"+modelimage:
                        msg("modelimage parameter set to "+modelimage+" but also creating a new total power image "+tpimage,priority="warn")
                        msg("assuming you know what you want, and using modelimage="+modelimage+" in deconvolution",priority="warn")
                    else:
                        # This forces to use TP image as a model for clean
                        if len(modelimage) <= 0:
                            msg("you are generating total power image "+tpimage+". this is used as a model image for clean",priority="warn")
                        modelimage = tpimage
                
                # format image size properly
                sdimsize = imsize
                if not isinstance(imsize,list):
                    sdimsize = [imsize,imsize]
                elif len(imsize) == 1:
                    sdimsize = [imsize[0],imsize[0]]

                im.open(tpmstoimage)
                im.selectvis(nchan=model_nchan,start=0,step=1,spw=0)
                im.defineimage(mode='channel',nx=sdimsize[0],ny=sdimsize[1],cellx=cell[0],celly=cell[1],phasecenter=model_refdir,nchan=model_nchan,start=0,step=1,spw=0)
                #im.setoptions(ftmachine='sd',gridfunction='pb')
                im.setoptions(ftmachine='sd',gridfunction='pb')
                im.makeimage(type='singledish',image=tpimage)
                im.close()
                del sdimsize

                # For single dish: manually set the primary beam
                ia.open(tpimage)
                beam = ia.restoringbeam()
                if len(beam) == 0:
                    msg('setting primary beam information to image.')
                    # !! aveant will only be set if modifymodel or setpointings and in 
                    # any case it will the the aveant of the INTERFM array - we want the SD
                    tb.open(tpmstoimage+"/ANTENNA")
                    diams = tb.getcol("DISH_DIAMETER")
                    tb.done()
                    aveant = pl.mean(diams)
                    # model_center should be set even if we didn't predict this execution
                    pb = 1.2*0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/aveant*3600.*180/pl.pi
                    beam['major'] = beam['minor'] = qa.quantity(pb,'arcsec')
                    beam['positionangle'] = qa.quantity(0.0,'deg')
                    msg('Primary beam: '+str(beam['major']))
                    ia.setrestoringbeam(beam=beam)
                ia.close()
                if sd_only:
                    beam_current = True
                    bmarea = beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
                    bmarea = bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
                else: del beam
                #del beam

                msg('generation of total power image '+tpimage+' complete.')
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
            tb.open(mstoimage[0]+"/SOURCE")
            code = tb.getcol("CODE")
            sourcefieldlist = pl.where(code=='OBJ')[0]
            nfld = len(sourcefieldlist)
            tb.done()
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
                msg("Found modelimage, %s." % modelimage)

            # in simdata we use imdirection instead of model_refdir
            if not util.isdirection(imdirection,halt=False):
                imdirection=model_refdir
            util.imclean(mstoimage,imagename,
                       cleanmode,cell,imsize,imdirection,
                       niter,threshold,weighting,
                       outertaper,stokes, #sourcefieldlist=sourcefieldlist,
                       modelimage=modelimage,mask=mask)

            # create imagename.flat and imagename.residual.flat:
            util.flatimage(imagename+".image",verbose=verbose)
            util.flatimage(imagename+".residual",verbose=verbose)
            outflat_current = True

            msg("done inverting and cleaning")
            if not type(cell) == type([]):
                cell = [cell,cell]
            if len(cell) <= 1:
                cell = [qa.quantity(cell[0]),qa.quantity(cell[0])]
            else:
                cell = [qa.quantity(cell[0]),qa.quantity(cell[1])]
            cell = [qa.abs(cell[0]),qa.abs(cell[0])]

            # get beam from output clean image
            if verbose: msg("getting beam from "+imagename+".image",origin="analysis")
            ia.open(imagename+".image")
            beam = ia.restoringbeam()
            beam_current = True
            ia.close()
            # model has units of Jy/pix - calculate beam area from clean image
            # (even if we are not plotting graphics)
            bmarea = beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
            bmarea = bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
            msg("synthesized beam area in output pixels = %f" % bmarea)










        if image:
            # show model, convolved model, clean image, and residual
            if grfile:
                file = fileroot + "/" + project + ".image.png"
            else:
                file = ""
        else:
            mslist=[]

        if image and len(mstoimage) > 0:
            if grscreen or grfile:
                util.newfig(multi=[2,2,1],show=grscreen)

                # create regridded and convolved sky model image
                util.convimage(modelflat,imagename+".image.flat")
                convsky_current = True # don't remake this for analysis in this run

                disprange = []  # passing empty list causes return of disprange

                # original sky regridded to output pixels but not convolved with beam
                discard = util.statim(modelflat+".regrid",disprange=disprange,showstats=False)
                util.nextfig()

                # convolved sky model - units of Jy/bm
                disprange = []
                discard = util.statim(modelflat+".regrid.conv",disprange=disprange)
                util.nextfig()

                # clean image - also in Jy/beam
                # although because of DC offset, better to reset disprange
                disprange = []
                discard = util.statim(imagename+".image.flat",disprange=disprange)
                util.nextfig()

                if len(mstoimage) > 0:
                    util.nextfig()

                    # clean residual image - Jy/bm
                    discard = util.statim(imagename+".residual.flat",disprange=disprange)
                util.endfig(show=grscreen,filename=file)




        #####################################################################
        # analysis

        if analyze:

# set above            
#            if not image:
#                imagename = fileroot + "/" + project

            if not os.path.exists(imagename+".image"):
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
                cell = util.cellsize(imagename+".image")
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
                util.flatimage(imagename+".image",verbose=verbose)
                if os.path.exists(imagename+".residual"):
                    util.flatimage(imagename+".residual",verbose=verbose)
                else:
                    if showresidual:
                        msg(imagename+".residual not found -- residual will not be plotted",priority="warn")
                    showresidual = False
                outflat_current = True

            # regridded and convolved input:?
            if not convsky_current:
                util.convimage(modelflat,imagename+".image.flat")
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
            if os.path.exists(msfile) and util.ismstp(msfile,halt=False):
                if showpsf: msg("single dish simulation -- psf will not be plotted",priority='warn')
                showpsf = False
            if (not image) and (not os.path.exists(msfile)):
                if showpsf or showuv:
                    msg("No image is generated in this run. Default MS, '%s', does not exists -- uv and psf will not be plotted" % msfile,priority='warn')
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
                util.newfig(multi=multi,show=grscreen)

                # if order in task parameters changes, change here too
                if showuv:
# TODO loop over all ms - show all UV including zero
                    if len(mslist)>1:
                        msg("Using only "+msfile+" for uv plot",priority="warn")
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
#                    if predict_sd and not util.ismstp(msfile,halt=False):
#                        pl.plot([0.],[0.],'r,')
                    util.nextfig()

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
                    beam_array = ia.getchunk(axes=[beamcs.findcoordinate("spectral")[1][0],beamcs.findcoordinate("stokes")[1][0]],dropdeg=True)
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
                    util.nextfig()

                disprange = []  # first plot will define range
                if showmodel:
                    discard = util.statim(modelflat+".regrid",incell=cell,disprange=disprange,showstats=False)
                    util.nextfig()
                    disprange = []

                if showconvolved:
                    discard = util.statim(modelflat+".regrid.conv")
                    # if disprange gets set here, it'll be Jy/bm
                    util.nextfig()

                if showclean:
                    # own scaling because of DC/zero spacing offset
                    discard = util.statim(imagename+".image.flat")
                    util.nextfig()

                if showresidual:
                    # it gets its own scaling
                    discard = util.statim(imagename+".residual.flat")
                    util.nextfig()

                if showdifference:
                    # it gets its own scaling.
                    discard = util.statim(imagename+".diff")
                    util.nextfig()

                if showfidelity:
                    # it gets its own scaling.
                    discard = util.statim(imagename+".fidelity",showstats=False)
                    util.nextfig()

                util.endfig(show=grscreen,filename=file)

            sim_min,sim_max,sim_rms,sim_units = util.statim(imagename+".image.flat",plot=False)
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
        if os.path.exists(imagename+".flux"):
            shutil.rmtree(imagename+".flux")
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
