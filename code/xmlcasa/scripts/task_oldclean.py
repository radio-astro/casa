import os
from taskinit import *

def oldclean(vis,imagename,mode,alg,niter,gain,threshold,mask,cleanbox,nchan,start,width,imsize,cell,stokes,field,spw,weighting,rmode,robust,uvfilter,uvfilterbmaj,uvfilterbmin,uvfilterbpa,timerange,restfreq, noise, npixels, scales, npercycle, phasecenter):

	#Python script

        casalog.origin('clean')

	isInteractive=False;
	if (cleanbox== 'interactive'):
	        isInteractive=True
		if( (mask=='') or (mask==[''])):
			mask=imagename+'_interactive.mask'

	try:
		imCln=imtool.create()
		if((len(imagename)==0) or (imagename.isspace())):
			raise Exception, 'Cannot proceed with blank imagename'
		casalog.origin('clean')
		if (field==''): 
			field='0'
		if(type(phasecenter)==str):
			### blank means take field[0]
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
		
		if( (spw==-1) or (spw=='-1')  or (spw=='*') or (spw=='') or (spw==' ')):
			spwindex=-1
		else:
			spwindex=ms.msseltoindex(vis, spw=spw)['spw'].tolist()
			if(len(spwindex)==0):
				spwindex=-1

                if ((type(imsize)==int)):
                        imsize=[imsize,imsize]
                if ((type(cell)==int) | (type(cell)==float) | (type(cell)==str)):
                        cell=[cell,cell]
                if ((len(imsize)==1)): 
                        imsize=[imsize[0],imsize[0]]
                if ((len(cell)==1)):
                        cell=[cell[0],cell[0]]

                if ((type(vis)==str) & (os.path.exists(vis))):
                        imCln.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
                if ((width!=1) | (mode == 'frequency') | (mode =='velocity')):
                        imCln.selectvis(field=field, spw=spw,time=timerange)
                else:
                        imCln.selectvis(field=field,spw=spw,time=timerange)
		cellx=cell[0]
		celly=cell[1]
		if((type(cell[0])==int) or (type(cell[0])==float)):
			cellx=qa.quantity(cell[0], 'arcsec')
			celly=qa.quantity(cell[1], 'arcsec')
	
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
			if((type(width) != int) or 
			   (type(start) != int)):
				raise TypeError, "start, width have to be integers with mode %s" %mode
		#print 'cell', cellx, celly, restfreq
		imCln.defineimage(nx=imsize[0],ny=imsize[1],cellx=cellx,celly=celly,mode=mode,nchan=nchan,start=start,step=width,spw=spwindex,stokes=stokes, restfreq=restfreq,phasecenter=phasecenter)

		if ((mask==['']) & (cleanbox!=[])):
			setcleanbox = False
			if (type(cleanbox[0])==int):
				cleanbox=[cleanbox]
				setcleanbox = True
			if ((type(cleanbox[0])==str) & (cleanbox[0]!='') & (cleanbox != 'interactive')): 
				cleanbox=readboxfile(cleanbox)
				setcleanbox = True
			if setcleanbox :
				mask=[imagename+'.cleanbox.mask']
				if(os.path.exists(mask[0])):
					casalog.post('mask '+mask[0]+' exists on disk, will add new mask to it', 'WARN')
				imCln.regionmask(mask=mask[0],
					      boxes=cleanbox)

		if weighting=='briggs':
			imCln.weight(type=weighting,rmode=rmode,robust=robust, npixels=npixels, noise=noise)
		else:
			imCln.weight(type=weighting, npixels=npixels)
		if uvfilter==True:
			imCln.filter(type='gaussian',bmaj=qa.quantity(uvfilterbmaj,'arcsec'),bmin=qa.quantity(uvfilterbmin,'arcsec'),bpa=qa.quantity(uvfilterbpa,'deg'))
		if alg=='multiscale' :
			imCln.setscales(scalemethod='uservector', uservector=scales)
		imCln.clean(algorithm=alg,niter=niter,gain=gain,threshold=qa.quantity(threshold,'mJy'),model=[imagename+'.model'],residual=[imagename+'.residual'],image=[imagename+'.image'],mask=mask, interactive=isInteractive, npercycle=npercycle)
		imCln.close()
		del imCln

	except Exception, instance:
		print '*** Error *** ',instance
		raise Exception, instance

