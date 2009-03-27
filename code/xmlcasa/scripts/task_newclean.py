import os
from taskinit import *
from cleanhelper import *


def newclean(vis,imagename,field, spw, selectdata, timerange, uvrange, antenna, scan, mode,niter,gain,threshold, csclean, psfmode, mosaicmode, ftmachine, mosweight, scaletype, multiscale, negcomponent,interactive,mask, nchan,start,width,imsize,cell, phasecenter, restfreq, stokes,weighting,robust, uvtaper,outertaper,innertaper, modelimage,restoringbeam,pbcor, minpb,  noise, npixels, npercycle, cyclefactor, cyclespeedup):

	#Python script

        casalog.origin('clean')

	maskimage=''
	if((mask==[]) or (mask=='')):
		mask=['']
	if (interactive):
		if( (mask=='') or (mask==['']) or (mask==[])):
			maskimage=imagename+'.mask'

	#try:
	if(1):
		imCln=imtool.create()
		imset=cleanhelper(imCln, vis)
		
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
					 outertaper=outertaper)
		if(maskimage==''):
			maskimage=imagename+'.mask'
		imset.makemaskimage(outputmask=maskimage,imagename=imagename,
				    maskobject=mask)


		
		###define clean alg
		alg=psfmode
		if(multiscale==[0]):
			multiscale=[]
		if((type(multiscale)==list) and (len(multiscale)>0)):
			alg='multiscale' 
			imCln.setscales(scalemethod='uservector',
					uservector=multiscale)
		if(csclean):
			alg='mf'+alg
		if(mosaicmode):
			if(alg.count('mf') <1):
				alg='mf'+alg
			imCln.setoptions(ftmachine=ftmachine, padding=1.0)
			imCln.setvp(dovp=True)
		###PBCOR or not
		sclt='SAULT'
		if(pbcor):
			sclt='NONE'
			imCln.setvp(dovp=True)
		else:
			if(not mosaicmode):
				##make a pb for flux scale
				imCln.setvp(dovp=True)
				imCln.makeimage(type='pb', image=imagename+'.flux')
				imCln.setvp(dovp=False)
		##restoring
		imset.setrestoringbeam(restoringbeam)
		###model image
		imset.convertmodelimage(modelimages=modelimage,
					outputmodel=imagename+'.model')

		####after all the mask shenanigans...make sure to use the
		####last mask
		maskimage=imset.outputmask
		if(mosaicmode or pbcor):
			imCln.setmfcontrol(stoplargenegatives=negcomponent,scaletype=sclt,minpb=minpb,cyclefactor=cyclefactor,cyclespeedup=cyclespeedup,fluxscale=[imagename+'.flux'])
		else:
			imCln.setmfcontrol(stoplargenegatives=negcomponent,cyclefactor=cyclefactor,cyclespeedup=cyclespeedup)
			
		imCln.clean(algorithm=alg,niter=niter,gain=gain,threshold=qa.quantity(threshold,'mJy'),model=[imagename+'.model'],residual=[imagename+'.residual'],image=[imagename+'.image'], psfimage=[imagename+'.psf'], mask=maskimage, interactive=interactive, npercycle=npercycle)
		imCln.close()
		del imCln

#	except Exception, instance:
#		print '*** Error *** ',instance
#		raise Exception, instance

