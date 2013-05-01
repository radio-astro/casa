import os
from taskinit import *

def makemask(cleanbox=None,vis=None,imagename=None,mode=None,nchan=None,start=None,width=None,step=None,imsize=None,cell=None,phasecenter=None,stokes=None,field=None,spw=None):
	"""Derive a mask image from a cleanbox blc, trc regions 

	Keyword arguments:
        cleanbox -- Cleaning region:
                default: [] defaults to inner quarter of image
                Two specification types:
                Explicit pixel ranges
                   example: cleanbox=[[110,110,150,145],[180,70,190,80]]
                   cleans two regions defined by the pixel ranges
                Filename with pixel values with ascii format:
                   <fieldid blc-x blc-y trc-x trc-y> on each line
	----------------------------------------------------------------
        vis -- Name of input visibility file (MS)
                default: none; example: vis='ngc5921.ms'
        imagename -- Name of output mask image
                default: none; example: imagename='ngc5921.mask'
        mode -- Type of selection 
                default: 'mfs'; example: mode='channel'; 
                Options: 'mfs', channel'
        nchan -- Number of channels to select
                default: -1 (all); example: nchan=45
        start -- Start channel
                default=0; example: start=5
        width -- Channel width (value > 1 indicates channel averaging)
                default=1; example: width=5
        step -- Step in channel number
                default=1; example: step=2      
        imsize -- Image size in spatial pixels (x,y)
                default = [256,256]; example: imsize=[512,512]
        cell -- Cell size in arcseconds (x,y)
                default=[0,0] = none; example: cell=[0.5,0.5]
        phasecenter -- direction measure  or fieldid for the mosaic center
                default: '' (imply field=0 as center); example: phasecenter=6
                or phasecenter='J2000 19h30m00 -40d00m00'
        stokes -- Stokes parameters to image
                default='I'; example: stokes='IQUV'; 
                Options: 'I','IV','IQU','IQUV'
       field -- Select field using field id(s) or field name(s).
                  [run listobs to obtain the list id's or names]
               default: ''=all fields
               If field string is a non-negative integer, it is assumed a field index
                 otherwise, it is assumed a field name
               field='0~2'; field ids 0,1,2
               field='0,4,5~7'; field ids 0,4,5,6,7
               field='3C286,3C295'; field named 3C286 adn 3C295
               field = '3,4C*'; field id 3, all names starting with 4C
       spw -- Select spectral window/channels
               default: ''=all spectral windows and channels
               spw='0~2,4'; spectral windows 0,1,2,4 (all channels)
               spw='<2';  spectral windows less than 2 (i.e. 0,1)
               spw='0:5~61'; spw 0, channels 5 to 61
               spw='0,10,3:3~45'; spw 0,10 all channels, spw 3, channels 3 to 45.
               spw='0~2:2~6'; spw 0,1,2 with channels 2 through 6 in each.
               spw='0:0~10;15~60'; spectral window 0 with channels 0-10,15-60
               spw='0:0~10,1:20~30,2:1;2;3'; spw 0, channels 0-10,
                        spw 1, channels 20-30, and spw 2, channels, 1,2 and 3

	"""

	#Python script
        casalog.origin('makemask')

	try:

                if (field==''):
                        field='*'

                if (phasecenter==''):
                        phasecenter=ms.msseltoindex(vis,field=field)['field'][0]

                spwindex=-1;
                if( (spw==-1) or (spw=='-1')  or (spw=='') or (spw==' ')):
                        spwindex=-1
                else:
                        spwindex=ms.msseltoindex(vis, spw=spw)['spw'].tolist()

                if ((type(vis)==str) & (os.path.exists(vis))):
                        im.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
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

                #parameter_printvalues(arg_names,arg_values,arg_types)
                if(mode=='frequency'):
                        ##check that start and step have units
                        if(qa.quantity(start)['unit'].find('Hz') < 1):
                                raise TypeError, "start parameter is not a valid frequency quantity "
                        if(qa.quantity(step)['unit'].find('Hz') < 1):
                                raise TypeError, "start parameter is not a valid frequency quantity "
                        im.defineimage(nx=imsize[0], ny=imsize[1], cellx=cellx,
                                       celly=celly, stokes=stokes, mode=mode,
                                       nchan=nchan, start=start, step=step,
                                       spw=spwindex, phasecenter=phasecenter)
                elif(mode=='velocity'):
                        ##check that start and step have units
                        if(qa.quantity(start)['unit'].find('m/s') < 0):
                                raise TypeError, "start parameter is not a valid velocity quantity "
                        if(qa.quantity(step)['unit'].find('m/s') < 0):
                                raise TypeError, "start parameter is not a valid velocity quantity "
                        im.defineimage(nx=imsize[0], ny=imsize[1], cellx=cellx,
                                       celly=celly, stokes=stokes, mode=mode,
                                       nchan=nchan, start=start, step=step,
                                       spw=spwindex, phasecenter=findex)
                else:
                        if(type(width) != int):
                                raise TypeError, "width has to be an integer with mode %s" %mode                
                        if width==1: im.defineimage(nx=imsize[0],ny=imsize[1],cellx=cellx,celly=celly,mode=mode,nchan=nchan,start=start,step=width, phasecenter=phasecenter,spw=spwindex,stokes=stokes)
                        if width!=1:
                                im.defineimage(nx=imsize[0],ny=imsize[1],cellx=cellx,celly=celly,mode=mode,nchan=nchan,start=start,step=width,phasecenter=phasecenter,spw=spwindex,stokes=stokes)

                if (type(cleanbox[0])==int): cleanbox=[cleanbox]

                if (type(cleanbox[0])==str): cleanbox=readboxfile(cleanbox)

                im.regionmask(mask=imagename,boxes=cleanbox)
		im.close()
	except Exception, instance:
		print '*** Error ***',instance

