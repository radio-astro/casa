import os
from taskinit import *

def mosaic(vis,imagename,mode,alg,imsize,cell,phasecenter,stokes,niter,gain,threshold,mask,cleanbox,nchan,start,width,field,spw,timerange,restfreq,sdimage,modelimage,weighting,mosweight,rmode,robust,ftmachine,cyclefactor,cyclespeedup,scaletype,minpb,sigma,targetflux,constrainflux,prior,negcomponent,scales,npercycle,npixels, noise):
	"""Calculate a multi-field deconvolved image with selected algorithm:

	"""

        ###
	#Python script
        casalog.origin('mosaic')

	if(mask==[]):
		mask=['']
	isInteractive=False;
	alg = 'mf'+ alg
        if (cleanbox== 'interactive'):
                isInteractive=True
                if( (mask=='') or (mask==[''])):
                        mask=[imagename+'_interactive.mask']

	#parameter_printvalues(arg_names,arg_values,arg_types)

	try:
###Set image parameters
		imMos=imtool.create()
		casalog.origin('mosaic')
		if((len(imagename)==0) or (imagename.isspace())):
			raise Exception, 'Cannot proceed with blank imagename'
                if (field==''):
                        field='*'

		if(type(phasecenter)==str):
			### blank means take field 0
			if (phasecenter==''):
				phasecenter=ms.msseltoindex(vis,field=field)['field'][0]
			else:
				tmppc=phasecenter
				try:
					if(len(ms.msseltoindex(vis, field=phasecenter)['field']) > 0):
						tmppc=ms.msseltoindex(vis, field=phasecenter)['field'][0]
					##succesful must be string like '0' or 'NGC*'
				except Exception, instance:
					##failed must be a string 'J2000 18h00m00 10d00m00'
					tmppc=phasecenter
				phasecenter=tmppc

                spwindex=-1;
                if( (spw==-1) or (spw=='-1')  or (spw=='') or (spw==' ')):
                        spwindex=-1
                else:
                        spwindex=ms.msseltoindex(vis, spw=spw)['spw'].tolist()

		fieldindex=ms.msseltoindex(vis,field=field)['field'].tolist()
                if ((type(vis)==str) & (os.path.exists(vis))):
                        imMos.open(vis, usescratch=mosweight)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
		imMos.setvp(dovp=True)
                if ((type(imsize)==int)):
                        imsize=[imsize,imsize]
                if ((type(cell)==int) | (type(cell)==float) | (type(cell)==str)):
                        cell=[cell,cell]
                if ((len(imsize)==1)): 
                        imsize=[imsize[0],imsize[0]]
                if ((len(cell)==1)):
                        cell=[cell[0],cell[0]]
		
		cellx=cell[0]
		celly=cell[1]
		if((type(cell[0])==int) or (type(cell[0])==float)):
			cellx=qa.quantity(cell[0], 'arcsec')
			celly=qa.quantity(cell[1], 'arcsec')


		print 'phasecenter= ', phasecenter
		#parameter_printvalues(arg_names,arg_values,arg_types)
		if(mode=='frequency'):
			##check that start and step have units
			if(qa.quantity(start)['unit'].find('Hz') < 1):
				raise TypeError, "start parameter is not a valid frequency quantity "
			if(qa.quantity(width)['unit'].find('Hz') < 1):
				raise TypeError, "start parameter is not a valid frequency quantity "	
                elif(mode=='velocity'):
                        ##check that start and step have units
                        if(qa.quantity(start)['unit'].find('m/s') < 0):
                                raise TypeError, "start parameter is not a valid velocity quantity "
                        if(qa.quantity(width)['unit'].find('m/s') < 0):
                                raise TypeError, "start parameter is not a valid velocity quantity "
		else:
			if((type(width) != int) 
			   or (type(start) != int)):
				raise TypeError, "start, width have to be  integers with mode %s" %mode

		imMos.defineimage(nx=imsize[0],ny=imsize[1],cellx=cellx,celly=celly,mode=mode,nchan=nchan,start=start,step=width,phasecenter=phasecenter,spw=spwindex,stokes=stokes, restfreq=restfreq)


###try to minimize the number of data selection
		### when natural mosweight is irrelevant		
		if(weighting=='natural'):
			mosweight=False 
		imMos.selectvis(field=field,spw=spw,time=timerange, usescratch=True)
		imMos.weight(type=weighting,rmode=rmode,robust=robust,npixels=npixels, noise=noise, mosaic=mosweight)


#####
###Set clean region (file or list)
                if (((mask==['']) or (mask=='')) & ( (len(cleanbox) !=0) and (type(cleanbox)==list))):
			setcleanbox=False
                        if (type(cleanbox[0])==int):
                                cleanbox=[cleanbox]
                                setcleanbox = True
                        if ((type(cleanbox[0])==str) & (cleanbox[0]!='') & (cleanbox != 'interactive')):
                                cleanbox=readboxfile(cleanbox)
                                setcleanbox = True

                        if setcleanbox :
                                mask=[imagename+'.cleanbox.mask']
                                if(os.path.exists(mask[0])):
					print 'mask',mask[0], 'exists on disk, will add new mask to it'
                                        casalog.post('mask '+mask[0]+' exists on disk, will add new mask to it', 'WARN')
                                imMos.regionmask(mask=mask[0], boxes=cleanbox)

#####
		if (ftmachine!='ft'): imMos.setoptions(ftmachine=ftmachine,padding=1.0)
		#imMos.setmfcontrol(scaletype=scaletype,minpb=minpb)
		#hardwire minpb for now
		sclt=scaletype
		if(scaletype=='PBCOR'):
			sclt='NONE'
		imMos.setmfcontrol(stoplargenegatives=negcomponent,scaletype=sclt,minpb=minpb,cyclefactor=cyclefactor,cyclespeedup=cyclespeedup,fluxscale=[imagename+'.flux'])

		if (modelimage==''): modelimage=imagename+'.model'

#		imMos.make(modelimage)
# Using SD image as model
		if (sdimage!=''): 
			if (mask[0]==''):  
				imMos.makemodelfromsd(sdimage=sdimage,modelimage=modelimage,maskimage=imagename+'.mask')
				mask[0]=imagename+'.mask'
			else:
				imMos.makemodelfromsd(sdimage=sdimage,modelimage=modelimage,maskimage=imagename+'.sdmask')
				mask[0]=imagename+'.sdmask'
#####
###Determine algorithm to use
		if (alg=='mfentropy' or alg=='mfemptiness'):
			imMos.mem(algorithm=alg,niter=niter,sigma=sigma,targetflux=targetflux,constrainflux=constrainflux,keepfixed=[False],prior=prior,model=[modelimage],residual=[imagename+'.residual'],image=[imagename+'.image'],mask=mask)
		elif (alg=='mfmultiscale'):
                        if (len(scales)==1):
                                imMos.setscales('nscales',nscales=scales)
                        else:
                                imMos.setscales('uservector',uservector=scales)

			imMos.clean(algorithm=alg,niter=niter,gain=gain,threshold=qa.quantity(threshold,'mJy'),model=[modelimage],residual=[imagename+'.residual'],image=[imagename+'.image'],mask=mask,interactive=isInteractive,npercycle=npercycle)
		else:
			imMos.clean(algorithm=alg,niter=niter,gain=gain,threshold=qa.quantity(threshold,'mJy'),model=[modelimage],residual=[imagename+'.residual'],image=[imagename+'.image'],mask=mask,interactive=isInteractive,npercycle=npercycle)

		imMos.close()
		del imMos
		#if (scaletype=='PBCOR'):
		#	print 'dividing by fluxscale image',imagename+'.flux'
		#	ia.open(imagename+'.image')
		#	result=imagename+'.image'
		#	fluxscale_image=imagename+'.flux'
		#	pixels=result+'/'+fluxscale_image
		#	ia.calc(pixels=pixels)
		#	ia.close()


	except Exception, instance:
		print '*** Error *** ',instance
		raise Exception, instance

