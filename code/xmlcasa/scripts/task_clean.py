import os
import shutil
from taskinit import *
from cleanhelper import *
im,cb,ms,tb,fg,af,me,ia,po,sm,cl,cs,rg,sl,dc,vp=gentools()
#import pdb

def clean(vis, imagename,outlierfile, field, spw, selectdata, timerange,
          uvrange, antenna, scan, mode, gridmode, wprojplanes, facets, cfcache,
          painc, epjtable, interpolation, niter, gain, threshold, psfmode,
          imagermode, ftmachine, mosweight, scaletype, multiscale, negcomponent,
          smallscalebias, interactive, mask, nchan, start, width, outframe,
          veltype, imsize, cell, phasecenter, restfreq, stokes, weighting,
          robust, uvtaper, outertaper, innertaper, modelimage, restoringbeam,
          pbcor, minpb, calready, noise, npixels, npercycle, cyclefactor,
          cyclespeedup, nterms, reffreq, chaniter):

    #Python script
    
    casalog.origin('clean')

    applyoffsets=False;
    pbgridcorrect=True;
    reffreqVal=0.0;
    padding=1.2;
    #
    # While the following condition is irrelavent due to the change in
    # the previous line, I (SB) am leaving it here till after some of
    # us discuss this change more carefully. (Feb. 25, 2010).
    #
    if (facets > 1):
        padding=1.2;

    # Handle selectdata explicitly
    #   (avoid use of hidden globals)
    if (not selectdata):
        timerange=''
        uvrange=''
        antenna=''
        scan=''

    # handle mode='mfs' explicitly
    if (mode=='mfs'):
        start=0

    try:
        if nterms > 1:
            print '***WARNING: Multi-term MFS imaging algorithm is new and under active development and testing.  Use it on a shared risk basis.'
            qat=qatool.create();
            try:
                rff=qat.canonical(reffreq);
            except Exception, instance:
                print '*** Error *** In conversion of reffreq=\'',reffreq,'\' to a numerical value';
                raise Exception, instance
            reffreqVal=rff['value'];  # This is the frequency in Hz
            #qat.close()
            
        # if (gridmode =='widefield'):
        #     if (wprojplanes > 1):
        #         ftmachine='wproject';
        # elif (gridmode == 'aprojection'):
        #     ftmachine='pbwproject';

        imCln=imtool.create()
        ###if calready open ms with scratch column
        ###if mosweight use scratch columns as there in no
        ###mosweight available for no scratch column /readonly ms yet
        imset=cleanhelper(imCln, vis, (calready or mosweight), casalog)

        if((len(imagename) == 0) or
           ((type(imagename) == str) and imagename.isspace())):
            raise Exception, 'Cannot proceed with blank imagename'
    
        multifield=False
        if (type(imagename)==list) & (len(imagename) > 1):
            multifield=True
        elif (type(phasecenter) == list) and (len(phasecenter) >1):
            raise TypeError, 'Number of phasecenters has to be equal to number of images'

        localFTMachine = getFTMachine(gridmode, imagermode, mode, wprojplanes,
                                      ftmachine);

        casalog.post("FTMachine used is  %s "%localFTMachine)
        
        #some default value handling for channelization
        if (mode=='velocity' or mode=='frequency' or mode=='channel'):
            (localnchan, localstart, localwidth)=imset.setChannelization(mode,spw,field,nchan,start,width,outframe,veltype,restfreq)
        else:
            imset.setspecframe(spw)
            localnchan=nchan
            localstart=start
            localwidth=width

        #setup for 'per channel' clean
        dochaniter=False
        #if interactive and chaniter:
        if chaniter:
        #    if veltype=="optical":
        #        raise Exception, 'The chaniter=True interactive clean for optical velocity mode is not implemented yet.'
            if localnchan > 1:
                dochaniter=True

        # make a template cube for interactive chanter=T
        if dochaniter:
            imset.makeTemplateCubes(imagename,outlierfile, field, spw, selectdata,
                                    timerange, uvrange, antenna, scan, mode, facets, cfcache, 
                                    interpolation, imagermode, localFTMachine, mosweight, 
                                    localnchan, localstart, localwidth, outframe, veltype, imsize, cell, 
                                    phasecenter, restfreq, stokes, weighting,
                                    robust, uvtaper, outertaper, innertaper, modelimage, 
                                    restoringbeam, calready, noise, npixels, padding)

            nchaniter=localnchan
            # check nchan in templatecube
            ia.open(imagename+'.image')
            if localnchan > ia.shape()[3]:
                nchaniter = ia.shape()[3]
            ia.close()
            finalimagename=imagename
            if type(finalimagename)==str:
                finalimagename=[finalimagename]
            imset.finalimages=finalimagename
            # move the following to a helper func.
            # create a temporary directory to put channel images
            (freqs,finc,newmode,tmppath)=imset.initChaniter(localnchan,spw,localstart,localwidth,finalimagename,mode)
            mode=newmode
        else:
            nchaniter=1
            finalimagename=''
         
        # loop over channels for per-channel clean
        for j in xrange(nchaniter):
            if dochaniter:

                print "Processing channel %s of %s" % (j+1, nchaniter)
                casalog.post("Processing channel %s of %s"% (j+1, nchaniter))
                #chaniterParms=imset.setChaniterParms(finalimagename,spw,j,localstart,localwidth,freqs,finc,tmppath)
                chaniterParms=imset.setChaniterParms(finalimagename,spw,j,localstart,width,freqs,finc,tmppath)
                imagename=chaniterParms['imagename']
                imnchan=chaniterParms['imnchan']
                chanslice=chaniterParms['chanslice']
                localwidth=chaniterParms['width']
                imstart=chaniterParms['imstart']
                visnchan=chaniterParms['visnchan']
                visstart=chaniterParms['visstart']

            # change to handle multifield masks
            maskimage=''
            if mask in ([], ''):
                mask=['']
            if interactive:
                if mask==['']:
                    if type(imagename) == list:
                        maskimage = []
                        for namoo in imagename:
                            maskimage.append(namoo + '.mask')
                            imset.maskimages[namoo] = namoo + '.mask'
                    elif (type(imagename) == str) and (len(outlierfile)
                                                       != 0):
                        maskimage = imagename + '.mask'
                        imset.maskimages[imagename] = imagename + '.mask'

            #read outlierfile
            imageids=[]
            imsizes=[]
            phasecenters=[]
            rootname=''
            if len(outlierfile) != 0:
                imsizes,phasecenters,imageids=imset.readoutlier(outlierfile)
                if type(imagename) == list:
                    rootname = imagename[0]
                else:
                    rootname = imagename
                if len(imageids) > 1:
                    multifield=True
            else:
                 imsizes=imsize
                 phasecenters=phasecenter
                 imageids=imagename

#
# Moved getAlgorithm() to here so that multifield is set using outlier file.
#
            localAlgorithm = getAlgorithm(psfmode, imagermode, gridmode, mode, 
                                          multiscale, multifield, facets, nterms,
                                          'clark');


            ###PBCOR or not 
            sclt='SAULT'
            makepbim=False
            if (scaletype=='PBCOR') or (scaletype=='pbcor'):
                sclt='NONE'
                imCln.setvp(dovp=True)
            else: 
                if imagermode != 'mosaic': 
                    makepbim=True 
                # scaletype is 'SAULT' so use default sclt
                # regardless of pbcor is T/F 
                #elif pbcor:        # mosaic and pbcor=true
                #    sclt='NONE'     # do the division in c++
            ###always setvp for mosaic mode
            if(imagermode=='mosaic'):
                imCln.setvp(dovp=True)

            if not dochaniter:
                imnchan=localnchan
                chanslice=-1
                imstart=localstart
                visnchan=-1
                visstart=0

            imset.definemultiimages(rootname=rootname, imsizes=imsizes,
                                    cell=cell, stokes=stokes, mode=mode,
                                    spw=spw, nchan=imnchan, start=imstart,
                                    width=localwidth, restfreq=restfreq,
                                    field=field, phasecenters=phasecenters,
                                    names=imageids, facets=facets,
                                    outframe=outframe, veltype=veltype,
                                    makepbim=makepbim, checkpsf=dochaniter) 

            imset.datselweightfilter(field=field, spw=spw,
                                     timerange=timerange, uvrange=uvrange,
                                     antenna=antenna, scan=scan,
                                     wgttype=weighting, robust=robust,
                                     noise=noise, npixels=npixels,
                                     mosweight=mosweight,
                                     innertaper=innertaper,
                                     outertaper=outertaper,
                                     calready=calready, nchan=visnchan,
                                     start=visstart, width=1)

            if maskimage=='':
                maskimage=imset.imagelist[0]+'.mask'

            if not multifield:
                imset.makemaskimage(outputmask=maskimage, imagename=imagename,
                                    maskobject=mask, slice=chanslice)

            else:
                imset.makemultifieldmask2(mask,chanslice)
                maskimage=[]
                for img in imset.maskimages:
                    maskimage.append(imset.maskimages[img])

            if dochaniter:
                imset.checkpsf(chanslice)
                if imset.skipclean: # for chaniter=T, and if the channel is flagged.
                    imset.makeEmptyimages()
                    casalog.post('No valid data, skip CLEANing this channel','WARN') 

            imCln.setoptions(ftmachine=localFTMachine,
                             wprojplanes=wprojplanes,
                             freqinterp=interpolation, padding=padding,
                             cfcachedirname=cfcache, pastep=painc,
                             epjtablename=epjtable,
                             applypointingoffsets=applyoffsets,
                             dopbgriddingcorrections=pbgridcorrect);

            #if(alg=='mfmultiscale' and multifield): 
            #    raise Exception, 'Multiscale clean with flanking fields is not supported yet'

            ##restoring
            imset.setrestoringbeam(restoringbeam)
            ###model image
            #if not multifield:
            #         imset.convertmodelimage(modelimages=modelimage,
            #                 outputmodel=imagename+'.model')
	    if (type(modelimage)!=str and type(modelimage)!=list):
		    raise Exception,'modelimage must be a string or a list of strings';
            if modelimage != '' and modelimage != [] and nterms==1:
                if dochaniter:
                    imset.defineChaniterModelimages(modelimage,j,tmppath)
                else:
                    imset.convertmodelimage(modelimages=modelimage,
                                    outputmodel=imset.imagelist.values()[0]+'.model')
            modelimages=[]
            restoredimage=[]
            residualimage=[]
            psfimage=[]
            fluximage=[]
            for k in range(len(imset.imagelist)):
                ia.open(imset.imagelist[k])
                if (modelimage =='' or modelimage==[]) and multifield:
                    ia.rename(imset.imagelist[k]+'.model',overwrite=True)
                else:
                    ia.remove(verbose=False)
                ia.close() 
                modelimages.append(imset.imagelist[k]+'.model')
                restoredimage.append(imset.imagelist[k]+'.image')
                residualimage.append(imset.imagelist[k]+'.residual')
                psfimage.append(imset.imagelist[k]+'.psf')
                if(imagermode=='mosaic'):
                    fluximage.append(imset.imagelist[k]+'.flux')

        #####################################################################
        #
        # The various settings, which trigger the logic which
        # controls (1) whether MFS algo. is fired, and (2)
        # controls the logic of the various settings of the
        # MFS algo. are in this block.
        #
        # multiscale=[] is used in other algorithms to detect
        # if MS-Clean should be used.  However since
        # multi-term MFS algo. is really only MS-MFS, it
        # always requires MS settings to be done. And
        # multiscale should be == [0] for a point-source pixel
        # model to be used during MFS.
        #
        # alg should always be "msmfs".
        #
        # Unless the number of images is same as the number of Taylor Terms (nterms)
        # the MFS algorithm is not triggered (Yuck!)
        #
        # If number of images is > 1 and alg != "msmfs", it
        # will trigger multi-field algorithm.
        #
        # So to trigger MFS, the settings should be:
        #   Algorithm = "msmfs"
        #   mutliscale = [0] or larger
        #   nterms > 1
        #   modelimage,retoredimage, residualimage should all be vectors of length
        #     == nterms
        #
        #   (the names for these, for MS-MFS case, are generated internally in
        #   the following block by appending qualifiers to the imagename given
        #   by the user)
        #
            if (mode == "mfs") and (nterms > 1):
                if len(imset.imagelist) > 1:
                    raise Exception, 'Multi-term MFS for multi-field imaging not supported'
                if multiscale == []:
                    multiscale = [0]
                # imCln.setscales(scalemethod='uservector',
                #         uservector=multiscale);
                # imCln.setsmallscalebias(smallscalebias)
                imCln.settaylorterms(ntaylorterms=nterms,
                                     reffreq=reffreqVal);

                modelimages[0]=modelimages[0]+'.tt'+str(0);
                restoredimage[0]=restoredimage[0]+'.tt'+str(0);
                residualimage[0]=residualimage[0]+'.tt'+str(0);

                # Make image names...
                for tt in range(1, nterms):
                    modelimages.append(imset.imagelist[0] + '.model'
                                       + '.tt' +str(tt))
                    restoredimage.append(imset.imagelist[0] + '.image' +
                                         '.tt' + str(tt))
                    residualimage.append(imset.imagelist[0] + '.residual'
                                         + '.tt' + str(tt))
		# Check if starting models exist 
		#   - either same name as that derived from 'imagename', stored in 'modelimages'
		#   - or, if the user has explicitly specified via 'modelimage'.
		# If neither, make empty ones.
		# Note : modelimages is an internal variable and modelimage is user-specified
                for tt in range(0, nterms):
		    if not os.path.exists(modelimages[tt]):
			 imCln.make(modelimages[tt])
		         casalog.post("No model found. Making empty initial model : "+modelimages[tt]);
	            else:
		         casalog.post("Found and starting from existing model on disk : "+modelimages[tt]);
		# Check for a user-specified modelimage list to add to current model
		if( modelimage != '' and modelimage != [] ):
		   if( type(modelimage)==str ):
		       modelimage = [modelimage];
		   if( type(modelimage)==list ):
		       nimages = min( len(modelimage), len(modelimages) );
		       for tt in range(0,nimages):
			   if( os.path.exists(modelimage[tt]) ):
			       imset.convertmodelimage(modelimages=[modelimage[tt]],outputmodel=modelimages[tt]);
			       casalog.post("Found user-specified model image : "+modelimage[tt]+" . Adding to starting model : "+modelimages[tt]);
			   else:
			       casalog.post("Cannot find user-specified model image : "+modelimage[tt]+" . Continuing with current model : "+modelimages[tt]);
		   else:
		      raise Exception,'Model image(s) must be a string or a list of strings';
		
		casalog.post('Running MS-MFS with '+str(nterms)+' Taylor-terms on dataset : ' + vis);
            #####################################################################
            if len(multiscale) > 0:
                imCln.setscales(scalemethod='uservector',
                                uservector=multiscale)
                imCln.setsmallscalebias(smallscalebias)

            if imagermode == 'mosaic':
                if localFTMachine == 'ft':
                    imCln.setmfcontrol(stoplargenegatives=negcomponent,
                                       scaletype=sclt,minpb=minpb,constpb=1.0,
                                       cyclefactor=cyclefactor,
                                       cyclespeedup=cyclespeedup,
                            #               fluxscale=[imagename+'.flux'])
                                       fluxscale=fluximage)
                else:
                    imCln.setmfcontrol(stoplargenegatives=negcomponent,
                                       scaletype=sclt,minpb=minpb,
                                       cyclefactor=cyclefactor,
                                       cyclespeedup=cyclespeedup,
                    #                   fluxscale=[imagename+'.flux'])
                                       fluxscale=fluximage)
            else:
                imCln.setmfcontrol(stoplargenegatives=negcomponent,
                                   cyclefactor=cyclefactor,
                                   cyclespeedup=cyclespeedup, minpb=minpb)

            ####after all the mask shenanigans...make sure to use the
            ####last mask
            if not multifield:
                maskimage = imset.outputmask
                ##if no effective mask was passed down but interactive
                if maskimage == '' and interactive:
                    if imset.maskimages.has_key(imset.imagelist[0]):
                        maskimage=imset.maskimages[imset.imagelist[0]]
                    else:
                        maskimage = imset.imagelist[0] + '.mask'
                        imset.maskimages[imset.imagelist[0]] = maskimage
            maskimg = mask
            if mask == True:
                maskimg = minpb
            if ((len(maskimage) == 0 or maskimage[0] == '') and
                isinstance(maskimg, float) and maskimg > 0.0 and maskimg < 1.0
                # and imagermode == 'mosaic'
                and not interactive):
                casalog.post('Making a mask at primary beam level ' + str(maskimg),
                             'INFO')
                casalog.post('Running clean with niter=0 to get the primary beam coverage',
                             'INFO')
                # Run clean with niter = 0 to get the pbcoverage.
                imCln.clean(algorithm=localAlgorithm, niter=0, gain=gain,
                            threshold=qa.quantity(threshold,'mJy'),
                            model=modelimages, residual=residualimage,
                            image=restoredimage, psfimage=psfimage,
                            mask=maskimage, interactive=False,
                            npercycle=npercycle)
                pbcov_image = imset.imagelist[0] + '.flux'
                if localFTMachine == 'mosaic':
                    pbcov_image += '.pbcoverage'
                maskimage = imset.make_mask_from_threshhold(pbcov_image, maskimg) 
            if not imset.skipclean: 
                imCln.clean(algorithm=localAlgorithm, niter=niter, gain=gain,
                            threshold=qa.quantity(threshold,'mJy'),
                            model=modelimages, residual=residualimage,
                            image=restoredimage, psfimage=psfimage,
                            mask=maskimage, interactive=interactive,
                            npercycle=npercycle)
		
                #In the interactive mode, deconvlution can be skipped and in that case
                #psf is not generated. So check if all psfs are there if not, generate
                if interactive:
                    for psfim in psfimage:
                        if not os.path.isdir(psfim):
                            imCln.approximatepsf(psf=psfim) 
            
            if dochaniter and not imset.skipclean :
                imset.storeCubeImages(finalimagename,imset.imagelist,j,imagermode)
        
        imCln.close()
        ####################################################################
        if dochaniter:
            imset.cleanupTempFiles(tmppath)
            imset.imagelist=finalimagename
        presdir=os.path.realpath('.')
        for k in xrange(len(imset.imagelist)):
            newimage=imset.imagelist[k]
            if(imset.imagelist[k].count('/') > 0):
                newimage=os.path.basename(imset.imagelist[k])
                os.chdir(os.path.dirname(imset.imagelist[k]))
            result          = '\'' + newimage + '.image' + '\'';
            fluxscale_image = '\'' + newimage + '.flux'  + '\'';
            pbcov_image=fluxscale_image
            if(localFTMachine=='mosaic'):
                pbcov_image = '\'' + newimage + '.flux.pbcoverage'  + '\'';
            residim         = '\'' + newimage + '.residual'  + '\'';
            if (pbcor):
                if(sclt != 'NONE'):
                    ##otherwise its already divided
                    ia.open(newimage+'.image')

                    ##this is not needed as mask is set in C++
                    #pixmask = fluxscale_image+'>'+str(minpb);
                    #ia.calcmask(pixmask,asdefault=True);
                    corrfac=minpb*minpb
                    if(localFTMachine=='ft'):
                        corrfac=minpb
                    pixels='iif('+ pbcov_image+'>'+str(corrfac)+','+ result+'/'+fluxscale_image+', 0)'

                    ia.calc(pixels=pixels)
                    ia.close()
                    ia.open(newimage+'.residual')
                    pixels='iif('+ fluxscale_image+'>'+str(corrfac)+','+ residim+'/'+fluxscale_image+', 0)'
                    ia.calc(pixels=pixels)
                    ia.close()
            elif sclt == 'NONE':
                ia.open(newimage+'.image')
                pixels=result+'*'+fluxscale_image
                ia.calc(pixels=pixels)
                ia.close()
                ia.open(newimage+'.residual')
                pixels=residim+'*'+fluxscale_image
                ia.calc(pixels=pixels)
                ia.close()

            os.chdir(presdir)

        del imCln

    except Exception, instance:
        print '*** Error *** ',instance
        raise Exception, instance

