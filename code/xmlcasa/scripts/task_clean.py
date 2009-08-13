import os
from taskinit import *
from cleanhelper import *


def clean(vis,imagename,field, spw, selectdata, timerange, uvrange,antenna,
	  scan, mode,interpolation,niter,gain,threshold, psfmode,imagermode,
	  ftmachine, mosweight, scaletype, multiscale,negcomponent,smallscalebias,
          interactive,mask,nchan,start,width,imsize,cell,phasecenter,restfreq, stokes,
	  weighting,robust,uvtaper,outertaper,innertaper,modelimage,restoringbeam,
	  pbcor, minpb,calready, noise, npixels, npercycle, cyclefactor, cyclespeedup,
	  cfcache,painc,epjtable,nterms,reffreq):

	#Python script

        casalog.origin('clean')
	reffreq=reffreq*1e9;
	maskimage=''
	if((mask==[]) or (mask=='')):
		mask=['']
	if (interactive):
		if( (mask=='') or (mask==['']) or (mask==[])):
			maskimage=imagename+'.mask'

	#try:
	if(1):
		imCln=imtool.create()
		imset=cleanhelper(imCln, vis, (calready or mosweight))
		
		if((len(imagename)==0) or (imagename.isspace())):
			raise Exception, 'Cannot proceed with blank imagename'
		casalog.origin('clean')
		
                imset.defineimages(imsize=imsize, cell=cell, stokes=stokes,
				   mode=mode, spw=spw, nchan=nchan,
				   start=start,  width=width,
				   restfreq=restfreq, field=field,
				   phasecenter=phasecenter)
		
                imset.datselweightfilter(field=field, spw=spw,
					 timerange=timerange, uvrange=uvrange,
					 antenna=antenna, scan=scan,
					 wgttype=weighting, robust=robust,
					 noise=noise, npixels=npixels,
					 mosweight=mosweight,
					 innertaper=innertaper,
					 outertaper=outertaper,
					 calready=calready)
		if(maskimage==''):
			maskimage=imagename+'.mask'
		imset.makemaskimage(outputmask=maskimage,imagename=imagename,
				    maskobject=mask)


		
		###define clean alg
		alg=psfmode
		if(multiscale==[0]):
			multiscale=[];
		if((type(multiscale)==list) and (len(multiscale)>0)):
			alg='multiscale';
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

		###PBCOR or not
		sclt='SAULT'
		if((scaletype=='PBCOR') or (scaletype=='pbcor')):
			sclt='NONE'
			imCln.setvp(dovp=True)
		else:
			if(imagermode != 'mosaic'):
				##make a pb for flux scale
				imCln.setmfcontrol(minpb=minpb, constpb=1.0)
				imCln.setvp(dovp=True)
				imCln.makeimage(type='pb', image=imagename+'.flux')
				imCln.setvp(dovp=False)
		##restoring
		imset.setrestoringbeam(restoringbeam)
		###model image
		imset.convertmodelimage(modelimages=modelimage,
					outputmodel=imagename+'.model')


		###
		if((imagermode=='mosaic')):
			if(ftmachine=='ft'):
				imCln.setmfcontrol(stoplargenegatives=negcomponent,
						   scaletype=sclt,minpb=minpb,constpb=1.0,
						   cyclefactor=cyclefactor,cyclespeedup=cyclespeedup,
						   fluxscale=[imagename+'.flux'])
			else:
				imCln.setmfcontrol(stoplargenegatives=negcomponent,
						   scaletype=sclt,minpb=minpb,
						   cyclefactor=cyclefactor,
						   cyclespeedup=cyclespeedup,
						   fluxscale=[imagename+'.flux'])
		else:
			imCln.setmfcontrol(stoplargenegatives=negcomponent,
					   cyclefactor=cyclefactor,
					   cyclespeedup=cyclespeedup, minpb=minpb)



		####after all the mask shenanigans...make sure to use the
		####last mask
		maskimage=imset.outputmask
		
			
		imCln.clean(algorithm=alg,niter=niter,gain=gain,
			    threshold=qa.quantity(threshold,'mJy'),
			    model=[imagename+'.model'],
			    residual=[imagename+'.residual'],
			    image=[imagename+'.image'],
			    psfimage=[imagename+'.psf'],
			    mask=maskimage,
			    interactive=interactive,
			    npercycle=npercycle)
		imCln.close()
		presdir=os.path.realpath('.')
		newimage=imagename
		if(imagename.count('/') > 0):
			newimage=os.path.basename(imagename)
			os.chdir(os.path.dirname(imagename))
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

#	except Exception, instance:
#		print '*** Error *** ',instance
#		raise Exception, instance

