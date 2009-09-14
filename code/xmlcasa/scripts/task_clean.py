import os
from taskinit import *
from cleanhelper import *


def clean(vis,imagename,outlierfile, field, spw, selectdata, timerange, uvrange,antenna,
	  scan, mode,interpolation,niter,gain,threshold, psfmode,imagermode,
	  ftmachine, mosweight, scaletype, multiscale,negcomponent,smallscalebias,
          interactive,mask,nchan,start,width,imsize,cell,phasecenter,restfreq, stokes,
	  weighting,robust,uvtaper,outertaper,innertaper,modelimage,restoringbeam,
	  pbcor, minpb,calready, noise, npixels, npercycle, cyclefactor, cyclespeedup,
	  cfcache,painc,epjtable,nterms,reffreq):

	#Python script

        casalog.origin('clean')
	reffreq=reffreq*1e9;
	#maskimage=''
	#if((mask==[]) or (mask=='')):
        #	mask=['']
	#if (interactive):
        #	if( (mask=='') or (mask==['']) or (mask==[])):
        #		maskimage=imagename+'.mask'




	try:
	
		casalog.origin('clean')
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
                                raise TypeError, 'Number of phasecenters has be equal to number of images'


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
                #imset.defineimages(imsize=imsize, cell=cell, stokes=stokes,
	        #		   mode=mode, spw=spw, nchan=nchan,
	        #		   start=start,  width=width,
	        #		   restfreq=restfreq, field=field,
	        #		   phasecenter=phasecenter)
	        imset.definemultiimages(rootname=rootname, imsizes=imsizes, cell=cell, 
                                        stokes=stokes, mode=mode, spw=spw, 
                                        nchan=nchan, start=start,width=width, 
                                        restfreq=restfreq, field=field, phasecenters=phasecenters,
					names=imageids, facets=1, makepbim=makepbim) 

                imset.datselweightfilter(field=field, spw=spw,
					 timerange=timerange, uvrange=uvrange,
					 antenna=antenna, scan=scan,
					 wgttype=weighting, robust=robust,
					 noise=noise, npixels=npixels,
					 mosweight=mosweight,
					 innertaper=innertaper,
					 outertaper=outertaper,
					 calready=calready)
		#if(maskimage==''):
		#	maskimage=imagename+'.mask'
		#imset.makemaskimage(outputmask=maskimage,imagename=imagename,
		#		    maskobject=mask)
                if(maskimage==''):
                        maskimage=imset.imagelist[0]+'.mask'

                if(not multifield):
                        imset.makemaskimage(outputmask=maskimage, imagename=imagename,
                                                    maskobject=mask)


                else:
                        imset.makemultifieldmask2(mask)
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
		elif(imagermode=='mosaic'):
			if(alg.count('mf') <1):
				alg='mf'+alg;
			imCln.setoptions(ftmachine=ftmachine, padding=1.0,
					 freqinterp=interpolation);
			imCln.setvp(dovp=True);
		elif (imagermode=='multiscale'):
			imCln.setoptions(ftmachine=ftmachine,
					 freqinterp=interpolation);
			if((type(multiscale)==list) and (len(multiscale)>0)):
				alg='multiscale' 
                                if (multifield):
                                        alg='mf'+alg
				imCln.setscales(scalemethod='uservector',
						uservector=multiscale)
                                imCln.setsmallscalebias(smallscalebias)
		elif (imagermode=='desquint'):
			alg='cs';
			imCln.setoptions(ftmachine=ftmachine,
					 cfcachedirname=cfcache,
					 pastep=painc,
					 epjtablename="",
					 applypointingoffsets=False,
					 dopbgriddingcorrections=True);
		elif (imagermode=='msmfs'):
			alg='multiscale';
			if((type(multiscale)==list) and (len(multiscale)>0)):
				alg='multiscale';
                                if (multifield):
                                        alg='mf'+alg
				imCln.setscales(scalemethod='uservector',
						uservector=multiscale);
                                imCln.setsmallscalebias(smallscalebias)
			imCln.setoptions(ftmachine=ftmachine,
					 cfcachedirname=cfcache,
					 pastep=painc,
					 epjtablename="",
					 applypointingoffsets=False,
					 dopbgriddingcorrections=True);
			imCln.settaylorterms(ntaylorterms = nterms,
					     reffreq   = reffreq);
		elif (imagermode=='advanced'):
			if((type(multiscale)==list) and (len(multiscale)>0)):
				alg='multiscale';
                                if (multifield):
                                        alg='mf'+alg
				imCln.setscales(scalemethod='uservector',
						uservector=multiscale);
                                imCln.setsmallscalebias(smallscalebias)
			imCln.setoptions(ftmachine=ftmachine,
					 cfcachedirname=cfcache,
					 pastep=painc,
					 epjtablename=epjtable,
					 applypointingoffsets=False,
					 dopbgriddingcorrections=True);
			imCln.settaylorterms(ntaylorterms = nterms,
					     reffreq   = reffreq);
		else:
			imCln.setoptions(freqinterp=interpolation)

                if(alg=='mfmultiscale' and multifield): 
                    raise Exception, 'Multiscale clean with flanking fields is not supported yet'
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
                if modelimage !='' and modelimage!=[]:
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
		imCln.close()
		presdir=os.path.realpath('.')
                for k in range(len(imset.imagelist)):
		        newimage=imset.imagelist[k]
		        if(imset.imagelist[k].count('/') > 0):
			        newimage=os.path.basename(imset.imagelist[k])
			        os.chdir(os.path.dirname(imset.imagelist[k]))
		        result          = '\'' + newimage + '.image' + '\'';
		        fluxscale_image = '\'' + newimage + '.flux'  + '\'';
		        residim         = '\'' + newimage + '.residual'  + '\'';
                        # currently .flux exist only for main field
                        if k==0: 
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

