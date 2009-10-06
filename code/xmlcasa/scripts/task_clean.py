import os
import shutil
from taskinit import *
from cleanhelper import *
import pdb

def clean(vis,imagename,outlierfile, field, spw, selectdata, timerange, uvrange,antenna,
	  scan, mode,gridmode,wprojplanes,facets, cfcache,painc,epjtable, interpolation,
	  niter,gain,threshold, psfmode,imagermode,ftmachine, mosweight, scaletype, 
	  multiscale,negcomponent,smallscalebias, interactive,mask,nchan,start,width,
          outframe,veltype,imsize,cell,phasecenter,restfreq, stokes,weighting,robust,
          uvtaper,outertaper,innertaper,modelimage,restoringbeam,pbcor, minpb,calready, 
          noise, npixels, npercycle, cyclefactor, cyclespeedup,nterms,reffreq,chaniter):

	#pdb.set_trace()

	#Python script

	#For some vague reason despite the clean.py having gotten a generated 
        #default for ftmachine='ft'...it appears as 'mosaic' at this stage
	if(imagermode==''):
		ftmachine='ft'

        casalog.origin('clean')

	applyoffsets=False;
	pbgridcorrect=True;
	reffreqVal=1.4e9;
	padding=1.0;
	if (facets > 1):
		padding=1.2;
	try:
		if (nterms > 1):
			qat=qatool.create();
			try:
				rff=qat.canonical(reffreq);
			except Exception, instance:
				print '*** Error *** In conversion of reffreq=\'',reffreq,'\' to a numerical value';
				raise Exception, instance
			reffreqVal=rff['value'];  # This is the frequency in Hz
			#qat.close()
			
		if (gridmode =='widefield'):
			if (wprojplanes > 1):
				ftmachine='wproject';
		elif (gridmode == 'aprojection'):
			ftmachine='pbwproject';

		imCln=imtool.create()
		###if calready open ms with scratch column
		###if mosweight use scratch columns as there in no
		###mosweight available for no scratch column /readonly ms yet
		imset=cleanhelper(imCln, vis, (calready or mosweight), casalog)

		if((len(imagename)==0) or ((type(imagename)==str) and (imagename.isspace()))):
			raise Exception, 'Cannot proceed with blank imagename'
		
                multifield=False
                if((type(imagename)==list) & (len(imagename) >1)):
                        multifield=True
                else:
                        if((type(phasecenter) == list) and (len(phasecenter) >1)):
                                raise TypeError, 'Number of phasecenters has to be equal to number of images'

                #setup for 'per channel' clean
                dochaniter=False
                if interactive and (chaniter=='chan' or chaniter=='channel'): 
                        if nchan >1:
                                dochaniter=True
                
                if dochaniter:
                        nchaniter=nchan
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
                        if (mode=='channel'):
                                freqs,finc  = imset.getfreqs(nchan,spw,start,width)
                                mode = 'frequency'
                                
                else:
                        nchaniter=1
                        finalimagename=''
                # loop over channels for per-channel clean
                for j in xrange(nchaniter):
                        if dochaniter:
                                imset.maskimages={}
                                #imagename=[os.path.dirname(imn)+'/'+tmpdir+os.path.basename(imn)+'.ch'+str(j) 
                                #           for imn in finalimagename]
                                imagename=[tmppath[indx]+os.path.basename(imn)+'.ch'+str(j) 
                                           for indx, imn in enumerate(finalimagename)]
                                
                                print "Processing for Ch %s starts..." % j
                                casalog.post("Processing for Channel %s "% j)
 

                        # change to handle multifield masks
                        maskimage=''
                        if((mask==[]) or (mask=='')):
                                mask=['']
                        if (interactive):
                                if( (mask=='') or (mask==[''])or (mask==[])):
                                        if(type(imagename)==list):
                                                maskimage=[]
                                                for namoo in imagename:
                                                        maskimage.append(namoo+'.mask')
                                                        imset.maskimages[namoo]=namoo+'.mask'
                                        elif((type(imagename)==str) and (len(outlierfile) != 0)):
                                                maskimage=imagename+'.mask'
                                                imset.maskimages[imagename]=imagename+'.mask'

                        #read outlierfile
                        imageids=[]
                        imsizes=[]
                        phasecenters=[]
                        rootname=''
                        if(len(outlierfile) != 0):

                                imsizes,phasecenters,imageids=imset.readoutlier(outlierfile)
                                if(type(imagename)==list):
                                        rootname=imagename[0]
                                else:
                                        rootname=imagename
                                if(len(imageids) > 1):
                                        multifield=True

                        else:
                                imsizes=imsize
                                phasecenters=phasecenter
                                imageids=imagename

                        ###PBCOR or not 
                        sclt='SAULT'
                        makepbim=False
                        if((scaletype=='PBCOR') or (scaletype=='pbcor')):
                                sclt='NONE'
                                imCln.setvp(dovp=True)
                        else: 
                                if imagermode!='mosaic': 
                                        makepbim=True 

                        # Select only subset of vis data if possible.
                        # It does not work well for multi-spw so need
                        # to select with nchan=-1
                        if dochaniter:
                                imnchan=1
                                chanslice=j
			        qat=qatool.create();
                                q = qat.quantity

                                if len(spw)==1:
                                        if width>1:
                                                visnchan=width
                                        else:
                                                visnchan=1
                                else:
                                        visnchan=-1
                                #visstart=imstart
                                visstart=0

                                #pdb.set_trace()
                                if type(start)==int:
                                        # need to convert to frequencies
                                        # to ensure correct frequencies in 
                                        # output images(especially for multi-spw) 
                                        #imstart=start+j*width
                                        # Use freq list instead
                                        imstart=q(freqs[j],'Hz')
                                        width=q(finc,'Hz')
                                        #imstart=str(freqs[j])+'Hz'
                                        #width=str(finc)+'Hz'
                                elif start.find('m/s')>0:
                                        imstart=qat.add(q(start),qat.mul(j,q(width)))
                                elif start.find('Hz')>0:
                                        imstart=qat.add(q(start),qat.mul(j,q(width)))

                        else:
                                imnchan=nchan
                                chanslice=-1
                                imstart=start
                                visnchan=-1
                                visstart=1

                        imset.definemultiimages(rootname=rootname, imsizes=imsizes, cell=cell, 
                                                stokes=stokes, mode=mode, spw=spw, 
                                                nchan=imnchan, start=imstart, width=width, 
                                                restfreq=restfreq, field=field, phasecenters=phasecenters,
                                                names=imageids, facets=facets, outframe=outframe, 
                                                veltype=veltype, makepbim=makepbim) 

                        imset.datselweightfilter(field=field, spw=spw,
                                                 timerange=timerange, uvrange=uvrange,
                                                 antenna=antenna, scan=scan,
                                                 wgttype=weighting, robust=robust,
                                                 noise=noise, npixels=npixels,
                                                 mosweight=mosweight,
                                                 innertaper=innertaper,
                                                 outertaper=outertaper,
                                                 calready=calready, 
                                                 nchan=visnchan,start=visstart,
                                                 width=1)
                        

                        if(maskimage==''):
                                maskimage=imset.imagelist[0]+'.mask'

                        #for chaniter="channael" get a channel plane image if input mask is
                        #image cube

                        if(not multifield):
                                imset.makemaskimage(outputmask=maskimage, imagename=imagename,
                                                            maskobject=mask, slice=chanslice)


                        else:
                                imset.makemultifieldmask2(mask,slice=chanslice)
                                maskimage=[]
                                for k in range(len(imset.maskimages)):
                                        maskimage.append(imset.maskimages[imset.imagelist[k]])
                        
                         
                        ###define clean alg
                        alg=psfmode
                        if(imagermode==''):
                                if(multifield):
                                       alg='mf'+alg
                        if(multiscale==[0]):
                                multiscale=[];
                        if((type(multiscale)==list) and (len(multiscale)>0)):
                                alg='multiscale';
                                if (multifield):
                                        alg='mf'+alg
                                imCln.setscales(scalemethod='uservector',
                                                uservector=multiscale);
                                imCln.setsmallscalebias(smallscalebias)
                        if(imagermode=='csclean'):
                                alg='mf'+alg;
                                if (gridmode=='aprojection'):
                                        alg='cs';
                        elif(imagermode=='mosaic'):
                                if(alg.count('mf') <1):
                                        alg='mf'+alg;
                                imCln.setoptions(ftmachine=ftmachine, padding=padding,
                                                 wprojplanes=wprojplanes,
                                                 freqinterp=interpolation,
                                                 cfcachedirname=cfcache,
                                                 pastep=painc,epjtablename=epjtable,
                                                 applypointingoffsets=applyoffsets,
                                                 dopbgriddingcorrections=pbgridcorrect);
                                imCln.setvp(dovp=True);
        #
        # Determine the algorithm for wprojection.  Copied the logic from widefield task.
        #
                        if (gridmode=='widefield'):
                                alg='mfclark';
                                if((psfmode=='clark') or (psfmode=='hogbom')):
                                        if(facets > 1):
                                                alg='wf'+psfmode;
                                        else:
                                                alg='mf'+psfmode;
                                if((type(multiscale)==list) and (len(multiscale)>0)):
                                        if(facets >1):
                                                raise Exception, 'multiscale with facets > 1 not allowed for now';
                                        alg='mfmultiscale';

                        imCln.setoptions(ftmachine=ftmachine,
                                         wprojplanes=wprojplanes,
                                         freqinterp=interpolation,padding=padding,
                                         cfcachedirname=cfcache,
                                         pastep=painc,epjtablename=epjtable,
                                         applypointingoffsets=applyoffsets,
                                         dopbgriddingcorrections=pbgridcorrect)

                        if (mode=='mfs'):
                                if((type(multiscale)==list) and (len(multiscale)>0)):
                                        alg='multiscale';
                                        if (multifield or (imagermode != "")):
                                                alg='mf'+alg
                               # imCln.setscales(scalemethod='uservector',
                               # 		       uservector=multiscale);
                               # imCln.setsmallscalebias(smallscalebias)
                               # imCln.setoptions(ftmachine=ftmachine,
                               # 			cfcachedirname=cfcache,
                               # 			pastep=painc,
                               # 			epjtablename="",
                               # 			applypointingoffsets=False,
                               # 			dopbgriddingcorrections=True);
                                if (nterms > 1):
        #				alg='multiscale';
                                        imCln.settaylorterms(ntaylorterms = nterms,
                                                             reffreq   = reffreqVal);

                        #if(alg=='mfmultiscale' and multifield): 
                        #    raise Exception, 'Multiscale clean with flanking fields is not supported yet'

                        ### THIS PART IS MOVED TO BEGINNING OF THE SCRIPT
                        ###PBCOR or not
        #		sclt='SAULT'
        #		if((scaletype=='PBCOR') or (scaletype=='pbcor')):
        #			sclt='NONE'
        #			imCln.setvp(dovp=True)
        #		else:
        #	        	if(imagermode != 'mosaic'):
        #	        		##make a pb for flux scale
        #				imCln.setmfcontrol(minpb=minpb, constpb=1.0)
        #				imCln.setvp(dovp=True)
        #				imCln.makeimage(type='pb', image=imagename+'.flux')
        #				imCln.setvp(dovp=False)
                        ##restoring
                        imset.setrestoringbeam(restoringbeam)
                        ###model image
                        #if not multifield:
                        #         imset.convertmodelimage(modelimages=modelimage,
                        # 				outputmodel=imagename+'.model')
                        #if modelimage !='' and modelimage!=[]:
                        #        imset.convertmodelimage(modelimages=modelimage,
                        #                                outputmodel=imset.imagelist.values()[0]+'.model')
                        
                        # model image: make sure right modeimage is used for chaniter mode 
                        if modelimage !='' and modelimage!=[]:
                                if dochaniter:
                                        chanmodimg=[]
                                        if type(modelimage)==str:
                                                modelimage=[modelimage]
                                        for modimg in modelimage:
                                                 if type(modimg)==list:
                                                         chanmodimg=[]
                                                         for img in modimg:
                                                                 if os.path.dirname(img)!='':
                                                                         chanmodimg.append(tmppath[0]+'_tmp.'+os.path.basename(img))
                                                                 else:
                                                                         chanmodimg.append(tmppath[0]+'_tmp.'+img)
                                                                 imset.getchanimage(cubeimage=img,outim=chanmodimg[-1],chan=j)
                                                         imset.convertmodelimage(modelimages=chanmodimg,
                                                                outputmodel=imset.imagelist.values()[0]+'.model')
                                                         chanmodimg=[]
                                                 else:
                                                         if os.path.dirname(modimg)!='':
                                                                chanmodimg.append(tmppath[0]+'_tmp.'+os.path.basename(modimg))
                                                         else:
                                                                chanmodimg.append(tmppath[0]+'_tmp.'+modimg)
                                                         imset.getchanimage(cubeimage=modimg,outim=chanmodimg[-1],chan=j)

                                                         imset.convertmodelimage(modelimages=chanmodimg,
                                                                outputmodel=imset.imagelist.values()[0]+'.model')
                                                 # clean up tempoarary channel model image
                                                 for img in chanmodimg:
                                                         if os.path.exists(img):
                                                                os.system('rm -rf '+img)
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
                                    ia.remove()
                                ia.close() 
                                modelimages.append(imset.imagelist[k]+'.model')
                                restoredimage.append(imset.imagelist[k]+'.image')
                                residualimage.append(imset.imagelist[k]+'.residual')
                                psfimage.append(imset.imagelist[k]+'.psf')
                                if(imagermode=='mosaic'):
                                        fluximage.append(imset.imagelist[k]+'.flux')

                         
                        ###
                        if((imagermode=='mosaic')):
                                if(ftmachine=='ft'):
                                        imCln.setmfcontrol(stoplargenegatives=negcomponent,
                                                           scaletype=sclt,minpb=minpb,constpb=1.0,
                                                           cyclefactor=cyclefactor,cyclespeedup=cyclespeedup,
                                #			   fluxscale=[imagename+'.flux'])
                                                           fluxscale=fluximage)
                                else:
                                        imCln.setmfcontrol(stoplargenegatives=negcomponent,
                                                           scaletype=sclt,minpb=minpb,
                                                           cyclefactor=cyclefactor,
                                                           cyclespeedup=cyclespeedup,
                        #				   fluxscale=[imagename+'.flux'])
                                                           fluxscale=fluximage)
                        else:
                                imCln.setmfcontrol(stoplargenegatives=negcomponent,
                                                   cyclefactor=cyclefactor,
                                                   cyclespeedup=cyclespeedup, minpb=minpb)



                        ####after all the mask shenanigans...make sure to use the
                        ####last mask
                            
                        if(not multifield):
                            maskimage=imset.outputmask
                             
                 
                        #imCln.clean(algorithm=alg,niter=niter,gain=gain,
                        #	    threshold=qa.quantity(threshold,'mJy'),
                        #	    model=[imagename+'.model'],
                        #	    residual=[imagename+'.residual'],
                        #	    image=[imagename+'.image'],
                        #	    psfimage=[imagename+'.psf'],
                        #	    mask=maskimage,
                        #	    interactive=interactive,
                        #	    npercycle=npercycle)
                        imCln.clean(algorithm=alg,niter=niter,gain=gain,
                                    threshold=qa.quantity(threshold,'mJy'),
                                    model=modelimages,
                                    residual=residualimage,
                                    image=restoredimage,
                                    psfimage=psfimage,
                                    mask=maskimage,
                                    interactive=interactive,
                                    npercycle=npercycle)

                # end of 'per channel' iteration
		imCln.close()
                if len(finalimagename)!=0:
                        imagext = ['.image','.model','.flux','.residual','.psf']
                        if imagermode=='mosaic': 
                                imagext.append('.flux.pbcoverage')
                                imagext.append('.model.mask')
                        else:
                                imagext.append('.mask')

                        for indx, imf in enumerate(finalimagename):
                                for imext in imagext:
                                        if os.path.isdir(tmppath[indx]+ os.path.basename(imf)+'.ch0'+imext):
                                                ia.imageconcat(outfile=imf+imext,
                                                               infiles=[tmppath[indx]+os.path.basename(imf)+'.ch'+
                                                                        str(chno)+imext for chno in xrange(nchan)],
                                                               relax=True, overwrite=True)
                                                ia.close()
                                                #os.system('rm -rf %s' % tmpdir+imf+'.ch*'+imext)
                                masktextf=open(imf+imagext[-1]+'.text','w')
                                for k in xrange(nchan):
                                        if os.path.isfile(tmppath[indx]+os.path.basename(imf)+'.ch'+str(k)+imagext[-1]+'.text'):
                                                masktextf.write(str(k)+'\n')
                                                shutil.copyfileobj(open(tmppath[indx]+os.path.basename(imf)+
                                                                       '.ch'+str(k)+imagext[-1]+'.text'),masktextf)
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
		        residim         = '\'' + newimage + '.residual'  + '\'';
                        if (pbcor):
                                if(sclt != 'NONE'):
                                        ##otherwise its already divided
                                        ia.open(newimage+'.image')

                                        ##this is not needed as mask is set in C++
                                        #pixmask = fluxscale_image+'>'+str(minpb);
                                        #ia.calcmask(pixmask,asdefault=True);
                                        corrfac=minpb*minpb
                                        if(ftmachine=='ft'):
                                                corrfac=minpb
                                        pixels='iif('+ fluxscale_image+'>'+str(corrfac)+','+ result+'/'+fluxscale_image+', 0)'
          
                                        ia.calc(pixels=pixels)
                                        ia.close()
                                        ia.open(newimage+'.residual')
                                        pixels='iif('+ fluxscale_image+'>'+str(corrfac)+','+ residim+'/'+fluxscale_image+', 0)'
                                        ia.calc(pixels=pixels)
                                        ia.close()
                        else:
                                ##  people has imaged the fluxed corrected image
                                ## but want the
                                ## final image to be non-fluxed corrected
                                if(sclt=='NONE'):
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

