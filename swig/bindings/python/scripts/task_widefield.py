import os
#import pdb
from taskinit import *
from cleanhelper import *

def widefield(vis, imagename, outlierfile, field, spw, selectdata, timerange, uvrange, antenna, scan, mode, niter, gain, threshold, psfmode, ftmachine, facets, wprojplanes,multiscale, negcomponent, interactive, mask, nchan, start, width, imsize, cell, phasecenter, restfreq, stokes, weighting, robust, npixels, noise, cyclefactor, cyclespeedup, npercycle, uvtaper, outertaper, innertaper, restoringbeam, calready):
	"""Calculate a wide-field deconvolved image with selected algorithm:

	"""
	ms1=mstool.create()
        ###
	#Python script
	try:
		casalog.origin('widefield')
		multims=False
		if((type(vis)==list)):
			if(len(vis) > 1):
				multims=True
			else:
				vis=vis[0]
		multifield=False
		if(multims):
			print ' multiple ms not handle for now'
			return


		if((type(imagename)==list) & (len(imagename) >1)):
			multifield=True
		else:
			if((type(phasecenter) == list) and (len(phasecenter) >1)):
				raise TypeError, 'Number of phasecenters has be equal to number of images'
			
	


###Set image parameters
		casalog.origin('widefield')
		im1=imtool.create()
		if(multims):
			imset=cleanhelper()
		else:
			imset=cleanhelper(im1, vis, calready)
#
#                if(multims):
#			visi=vis[len(vis)-1]
#		else:
#			visi=vis

                


#		if(multifield):
#			if(type(phasecenter) != list):
#				raise TypeError, 'Number of phasecenters has be equal to number of images'
#			for k in range(len(phasecenter)):
#				pc=phasecenter[k]
#				phasecenter[k]=test_phasecenter(pc, visi)
#		else:
#			phasecenter=test_phasecenter(phasecenter, visi)
#
#		spwindex=-1;
#		spws=spw
#		if(multims & (type(spw)==list)):
#			spws=spw[0]
#		if( (spws==-1) or (spws=='-1')  or (spws=='') or (spws==[-1])):
#                        spwindex=-1
#			spws=''
#                else:
#			thems=vis
#			if(multims):
#				thems=vis[0]
#                        spwindex=ms1.msseltoindex(thems, spw=spws)['spw'].tolist()
                

		

#####
		imageids=[]
		imsizes=[]
		phasecenters=[]
		rootname=''
		if(len(outlierfile) != 0):
			
			imsizes,phasecenters,imageids=imset.readoutlier(outlierfile)
			if(type(rootname)==list):
				rootname=imagename[0]
			else:
				rootname=imagename
			if(len(imageids) > 1):
				multifield=True
				
		else:
			imsizes=imsize
			phasecenters=phasecenter
			imageids=imagename

		

		imset.definemultiimages(rootname, imsizes, cell, stokes, mode, spw, nchan, start,width, restfreq, field, phasecenters, imageids, facets)

############Data Selection
		if(multims):
			if(type(spw) == list):
				if(len(spw) != len(vis)):
					raise ValueError, 'Number of spw selection should match number of data sets'
			if(type(field) == list):
				if(len(field) != len(vis)):
					raise ValueError, 'Number of field selection should match number of data sets'
			for k in range(len(vis)):
				spws=spw
				if(type(spw) != list):
					spws=spw
				else:
					spws=spw[k]
				fields=field
				if(type(field) != list):
					fields=field
				else:
					fields=field[k]
				if(not os.path.exists(vis[k])):
					raise Exception, 'Visibility data %s set not found - please verify the name'%vis[k]
				im1.selectvis(vis=vis[k], spw=spws, field=fields)
		elif ((type(vis)==str) & (os.path.exists(vis))):
			imset.datselweightfilter(field=field, spw=spw,
					 timerange=timerange, uvrange=uvrange,
					 antenna=antenna, scan=scan,
					 wgttype=weighting, robust=robust,
					 noise=noise, npixels=npixels,
					 mosweight=False,
					 innertaper=innertaper,
					 outertaper=outertaper,
						 calready=calready)
                ############End of Data Selection
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'


###deal with masks
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




####
		padd=1.0
		if(facets > 1):
			padd=1.2
		im1.setoptions(ftmachine=ftmachine,padding=padd, wprojplanes=wprojplanes)
		#im1.setmfcontrol(scaletype=scaletype,minpb=minpb)
		im1.setmfcontrol(stoplargenegatives=negcomponent,cyclefactor=cyclefactor,cyclespeedup=cyclespeedup)
		
		##do themask
		if((maskimage=='') and (mask != [''])):
			maskimage=imset.imagelist[0]+'.mask'
	
		if(not multifield):
			imset.makemaskimage(outputmask=maskimage, imagename=imagename,
						    maskobject=mask)
			
			
		else:
			imset.makemultifieldmask(mask)
			maskimage=[]
			for k in range(len(imset.maskimages)):
				maskimage.append(imset.maskimages[imset.imagelist[k]])


		imset.setrestoringbeam(restoringbeam)

		alg='mfclark'
#####
###Determine algorithm to use
		if((psfmode=='clark') or (psfmode=='hogbom')):
			if(facets > 1):
				alg='wf'+psfmode
			else:
				alg='mf'+psfmode

		if(multiscale==[0]):
			multiscale=[]
		if((type(multiscale)==list) and (len(multiscale)>0)):
			if(facets >1):
				raise Exception, 'multiscale with facets > 1 not allowed for now'
			alg='mfmultiscale' 
			im1.setscales(scalemethod='uservector',
					uservector=multiscale)

		modelimage=[]
		restoredimage=[]
		residualimage=[]
		for k in range(len(imset.imagelist)):
			modelimage.append(imset.imagelist[k])
			restoredimage.append(imset.imagelist[k]+'.image')
			residualimage.append(imset.imagelist[k]+'.residual')
			
		##will have to enable entropy later
		if (alg=='mfentropy' or alg=='mfemptiness'):
			im1.mem(algorithm=alg,niter=niter,sigma=sigma,targetflux=targetflux,constrainflux=constrainflux,keepfixed=[False],prior=prior,model=modelimage,residual=residualimage,image=restoredimage,mask=maskimage)
		else:
			im1.clean(algorithm=alg,niter=niter,gain=gain,threshold=qa.quantity(threshold,'mJy'),model=modelimage,residual=residualimage,image=restoredimage,mask=maskimage,interactive=interactive,npercycle=npercycle)

		im1.done()
		del im1
	#	del ms1
	
        except Exception, instance:
		print '*** Error *** ',instance
		raise Exception, instance

