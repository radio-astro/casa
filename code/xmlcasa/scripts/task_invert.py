import os
from taskinit import *

def invert(vis,imagename,mode,nchan,start,width,imsize,cell,stokes,field,spw,weighting,rmode,robust,noise, npixels, restfreq,phasecenter):

	#Python script
        casalog.origin('invert')

        #parameter_printvalues(arg_names,arg_values,arg_types)
        try:
		imInv=imtool.create()
		if((len(imagename)==0) or (imagename.isspace())):
			raise Exception, 'Cannot proceed with blank imagename'
                if (field==''):
                        field='0'

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
                if( (spw==-1) or (spw=='-1') or (spw=='*')  or (spw=='') or (spw==' ')):
                        spwindex=-1
                else:
                        spwindex=ms.msseltoindex(vis, spw=spw)['spw'].tolist()
		

                if ((type(vis)==str) & (os.path.exists(vis))):
                        imInv.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
                seldata=True
		seldata=imInv.selectvis(field=field, spw=spw)
		if(not seldata):
			raise Exception, 'Data selection is invalid: selected spw and field do not overlap'

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
                if(type(cell[0])==int or type(cell[0])==float):
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
			if((type(width) != int)
			   or (type(start) != int)):
				raise TypeError, "start,  width have to be integers with mode %s" %mode
		imInv.defineimage(nx=imsize[0],ny=imsize[1],cellx=cellx,celly=celly,mode=mode,nchan=nchan,start=start,step=width,spw=spwindex,stokes=stokes, restfreq=restfreq,phasecenter=phasecenter)

                if weighting=='briggs':
                        imInv.weight(type=weighting,rmode=rmode,robust=robust,
				     noise=noise, npixels=npixels)
                else:
                        imInv.weight(type=weighting, npixels=npixels)

		imInv.makeimage(type='corrected',image=imagename+'.dirty')
		imInv.makeimage(type='psf',image=imagename+'.beam')
		imInv.close()
		del imInv

	except Exception, instance:
		print '*** Error ***', instance
		raise Exception, instance

