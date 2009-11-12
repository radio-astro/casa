import os
import shutil
from taskinit import *
from cleanhelper import *
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
    reffreqVal=1.4e9;
    padding=1.0;
    if (facets > 1):
        padding=1.2;
    try:
        if nterms > 1:
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
        localAlgorithm = getAlgorithm(psfmode, imagermode, gridmode, mode, 
                                      multiscale, multifield, facets, nterms,
                                      '')
        #some default value handling for channelization
        if (mode=='velocity' or mode=='frequency'):
            (localnchan, localstart, localwidth)=imset.setChannelization(mode,spw,field,nchan,start,width,outframe,veltype)
        else:
            localnchan=nchan
            localstart=start
            localwidth=width

        #setup for 'per channel' clean
        dochaniter=False
        if interactive and (chaniter=='chan' or chaniter=='channel'):
            if localnchan > 1:
                dochaniter=True

        if dochaniter:
            nchaniter=localnchan
            finalimagename=imagename
            if type(finalimagename)==str:
                finalimagename=[finalimagename]
            # create a temporary directory to put channel images
            tmpdir='_tmpimdir/'
            tmppath=[]
            for imname in finalimagename:
                if os.path.dirname(imname)=='':
                    #tmppath=tmpdir
                    tmppath.append(tmpdir)
                else:
                    #tmppath=os.path.dirname(imname)+'/'+tmpdir
                    tmppath.append(os.path.dirname(imname)+'/'+tmpdir)
                    if os.path.isdir(tmppath[-1]):
                        os.system('rm -rf '+tmppath[-1])
                    os.mkdir(tmppath[-1])

            #internally converted to frequency mode for mode='channel'
            #to ensure correct frequency axis for output image
            # put in helper function
            if mode == 'channel':
                freqs, finc = imset.getfreqs(localnchan, spw, localstart, localwidth)
                mode = 'frequency'
        else:
            nchaniter=1
            finalimagename=''

        # loop over channels for per-channel clean
        for j in xrange(nchaniter):
            if dochaniter:
                imset.maskimages={}
                imagename=[tmppath[indx]+os.path.basename(imn)+'.ch'+str(j)
                           for indx, imn in enumerate(finalimagename)]

                print "Processing for Ch %s starts..." % j
                casalog.post("Processing for Channel %s "% j)

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

            ###PBCOR or not 
            sclt='SAULT'
            makepbim=False
            if (scaletype=='PBCOR') or (scaletype=='pbcor'):
                sclt='NONE'
                imCln.setvp(dovp=True)
            else: 
                if imagermode != 'mosaic': 
                    makepbim=True 
                elif pbcor:         # mosaic and pbcor=true
                    sclt='NONE'     # do the division in c++
            ###always setvp for mosaic mode
            if(imagermode=='mosaic'):
                imCln.setvp(dovp=True)

            # Select only subset of vis data if possible.
            # It does not work well for multi-spw so need
            # to select with nchan=-1
            if dochaniter:
                imnchan=1
                chanslice=j
                qat=qatool.create();
                q = qat.quantity

                if len(spw)==1:
                    if localwidth>1:
                        visnchan=localwidth
                    else:
                        visnchan=1
                else:
                    visnchan=-1
                #visstart=imstart
                visstart=0

                if type(localstart)==int:
                    # need to convert to frequencies
                    # to ensure correct frequencies in
                    # output images(especially for multi-spw)
                    # Use freq list instead
                    imstart=q(freqs[j],'Hz')
                    localwidth=q(finc,'Hz')
                elif localstart.find('m/s')>0:
                    imstart=qat.add(q(localstart),qat.mul(j,q(localwidth)))
                elif localstart.find('Hz')>0:
                    imstart=qat.add(q(localstart),qat.mul(j,q(localwidth)))

            else:
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
                                    makepbim=makepbim) 

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
                for k in range(len(imset.maskimages)):
                    maskimage.append(imset.maskimages[imset.imagelist[k]])

            imCln.setoptions(ftmachine=localFTMachine,
                             wprojplanes=wprojplanes,
                             freqinterp=interpolation, padding=padding,
                             cfcachedirname=cfcache, pastep=painc,
                             epjtablename=epjtable,
                             applypointingoffsets=applyoffsets,
                             dopbgriddingcorrections=pbgridcorrect)

            if (mode=='mfs') and (nterms > 1):
                imCln.settaylorterms(ntaylorterms=nterms,
                                     reffreq=reffreqVal);

            #if(alg=='mfmultiscale' and multifield): 
            #    raise Exception, 'Multiscale clean with flanking fields is not supported yet'

            ##restoring
            imset.setrestoringbeam(restoringbeam)
            ###model image
            #if not multifield:
            #         imset.convertmodelimage(modelimages=modelimage,
            #                 outputmodel=imagename+'.model')
            if modelimage != '' and modelimage != []:
                if dochaniter:
                    chanmodimg=[]
                    if type(modelimage)==str:
                        modelimage=[modelimage]
                    for modimg in modelimage:
                         if type(modimg)==list:
                             chanmodimg=[]
                             for img in modimg:
                                 if os.path.dirname(img) != '':
                                     chanmodimg.append(tmppath[0]
                                                       + '_tmp.' +
                                                       os.path.basename(img))
                                 else:
                                     chanmodimg.append(tmppath[0] +
                                                       '_tmp.' + img)
                                 imset.getchanimage(cubeimage=img,
                                                    outim=chanmodimg[-1],
                                                    chan=j)
                             imset.convertmodelimage(modelimages=chanmodimg,
                                 outputmodel=imset.imagelist.values()[0]+'.model')
                             chanmodimg=[]
                         else:
                             if os.path.dirname(modimg) != '':
                                 chanmodimg.append(tmppath[0] + '_tmp.' +
                                                   os.path.basename(modimg))
                             else:
                                 chanmodimg.append(tmppath[0] + '_tmp.' +
                                                   modimg)
                             imset.getchanimage(cubeimage=modimg,
                                                outim=chanmodimg[-1],chan=j)

                             imset.convertmodelimage(modelimages=chanmodimg,
                                    outputmodel=imset.imagelist.values()[0]+'.model')
                         # clean up tempoarary channel model image
                         for img in chanmodimg:
                             if os.path.exists(img):
                                 os.system('rm -rf ' + img)
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

                for tt in range(1, nterms):
                    modelimages.append(imset.imagelist[0] + '.model'
                                       + '.tt' +str(tt))
                    restoredimage.append(imset.imagelist[0] + '.image' +
                                         '.tt' + str(tt))
                    residualimage.append(imset.imagelist[0] + '.residual'
                                         + '.tt' + str(tt))
                    imCln.make(modelimages[tt])
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


            imCln.clean(algorithm=localAlgorithm, niter=niter, gain=gain,
                        threshold=qa.quantity(threshold,'mJy'),
                        model=modelimages, residual=residualimage,
                        image=restoredimage, psfimage=psfimage,
                        mask=maskimage, interactive=interactive,
                        npercycle=npercycle);
        imCln.close()
        #
        # If MS-MFS was used, comput alpha (spectral index)
        # and beta (sp. index variation) images from the
        # Taylor term images.
        #
        if (localAlgorithm=='msmfs'):
            SimCalcAlphaBeta(imtemplate=restoredimage[0],
                             taylorlist=restoredimage[1:],
                             namealpha=imset.imagelist[0]+'.restored.alpha',
                             namebeta=imset.imagelist[0]+'.restored.beta',
                             threshold=0.01)

        if len(finalimagename)!=0:
            imagext = ['.image','.model','.flux','.residual','.psf','.mask']
            if imagermode=='mosaic':
                imagext.append('.flux.pbcoverage')

            for indx, imf in enumerate(finalimagename):
                for imext in imagext:
                    if os.path.isdir(tmppath[indx] + os.path.basename(imf)
                                     + '.ch0' + imext):
                        ia.imageconcat(outfile=imf+imext,
                                       infiles=[tmppath[indx]+os.path.basename(imf)+'.ch'+
                                                str(chno)+imext for chno in xrange(localnchan)],
                                       relax=True, overwrite=True)
                        ia.close()
                        #os.system('rm -rf %s' % tmpdir+imf+'.ch*'+imext)
                masktextf=open(imf+imagext[-1]+'.text','w')
                for k in xrange(localnchan):
                    if os.path.isfile(tmppath[indx] + os.path.basename(imf)
                                      + '.ch' + str(k) + imagext[-1] + '.text'):
                        masktextf.write(str(k)+'\n')
                        shutil.copyfileobj(open(tmppath[indx] +
                                                os.path.basename(imf) +
                                                '.ch'+str(k)+imagext[-1] +
                                                '.text'), masktextf)
                masktextf.close()
                #os.system('rm -rf %s' % tmppath[indx]+os.path.basename(imf)+'.ch*'+imext[-1]+'.text')
            # clean up temp. directory
            for k in range(len(finalimagename)):
                if os.path.isdir(tmppath[k]):
                    #print tmppath[k], " exists, removing..."
                    os.system('rm -rf %s' % tmppath[k])
        if dochaniter:
            imset.imagelist=finalimagename
        presdir=os.path.realpath('.')
        for k in range(len(imset.imagelist)):
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

