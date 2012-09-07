################################################
# Task to make masks.
#  modified by TT
# based on the original code, 
# v1.0: 2012.03.20, U.Rau
#
################################################
# Notes (to self) - TT 
# main functionality to implement (things requested)
# 1. expanding one mask to another 
#    e.g.) expanding a continuum mask (both image mask/boolean mask)
#          channel mask 
# 2. merging of different types of masks 
#    e.g.) inpimage and inpmask are lists of the mask to be merged
#          output mask is written to either outimage or outmask as embedded
#           T/F mask 
# 3. regrid mode (can be left out if it is bit tricky)
# ----------------------------------------------
#
# basic rules:
# if inpmask='': use inpimage as input mask
# if inpmask='mask0' or other embedded mask name of the inpimage, 
# use that T/F mask
# 
# expand:
# case1: on the same image, expand mask image from 
# prev. run etc. No regriding. Use nearest chan mask
# image to expand.  
# 1.a: inpimage is clean image mask (1s and 0s)
#      i) outimage != inpimage and outmask='' => new expanded mask image to outimage
#     ii) outimage != inpimage and outmask!='' => convert expanded mask image to T/F mask to store inside outimage
#    iii) outimage ==inpimage and outmask='' => update input mask image by expanding it 
#     iv) outimage ==inpimage and outmask!=''=> update input image with the expanded T/F mask
#  
# case2: outimage is in diffirent spec. coords. (need regrid)
#
#################
# tests 
# 1. for input: mask image 
# 2. for input: mask/or regular image with internal mask
# 3. for input: mask image; for output: mask image with different spectral grid
# 4. for input: mask/regular image with internal mask; for output: image with 
#      internal mask with different spectral grid
###################
import numpy as np
from taskinit import *
from recipes.pixelmask2cleanmask import *

def makemask(mode, outimage, outmask, inpimage, inpmask, inpfreqs, outfreqs, regionlist, maskname, replace):
    """
    make /manipulate masks

    """
    (ia,) = gentools(['ia']) 
    casalog.origin('makemask')
    print "Run the task"
    
    try:
        tmp_maskimage='__tmp_makemaskimage'
	tmp_outmaskimage='__tmp_outmakemaskimage'

	if inpimage==outimage:
	    tmp_outmaskimage=tmp_maskimage

    # the following code is somewhat duplicated among the modes but keep separated from each mode
    # for now....
    # === copy mode ===
        if mode=='copy':
           print "Copy mode"
           needregrid=True
           print "outimage=",outimage
           if outimage=='':
               #overwrite
               outimage=inpimage

           if not os.path.isdir(outimage):
               needregrid=False

	   if inpmask!='':
	   # need to extract the mask and put in tmp_maskimage
	       pixelmask2cleanmask(imagename=inpimage, maskname=inpmask, maskimage=tmp_maskimage, usemasked=True)    
               print "pix2cmask tmp_maskimage=",tmp_maskimage
               print "pix2cmask tmp_outmaskimage=",tmp_outmaskimage
	   else:
	       shutil.copytree(inpimage, tmp_maskimage)
           if needregrid:
               print "Regridding..."
               regridmask(tmp_maskimage,outimage,tmp_outmaskimage)
               # regrid may produce <1.0 pixels for mask so be sure to its all in 1.0
               #ia.open(tmp_outmaskimage) 
               #ia.calc('iif (%s>0.0 && %s<1.0,1,%s)'%(tmp_outmaskimage,tmp_outmaskimage,tmp_outmaskimage))
               #ia.close()
               print "Copying regrid output=",tmp_outmaskimage
           else:
               shutil.copytree(tmp_maskimage,tmp_outmaskimage)
	   if outmask!='':
	   #convert the image mask to T/F mask
               if not os.path.isdir(outimage):
                   shutil.copytree(inpimage,outimage)
               print "is %s exist?=%s" % (outimage, os.path.exists(outimage))
               #
               ia.open(outimage)
	       print "convert the image mask to T/F mask"
	       ia.calcmask(mask='%s<0.5' % tmp_outmaskimage,name=outmask,asdefault=True)
               ia.done()
           else:
               # if regridded - tmp_outmaskimage is created by regridmask
               # if not, tmp_outmaskimage=outimage
               ia.open(tmp_outmaskimage)
               ia.rename(outimage,overwrite=True)
               ia.done()

                
    # === expand mode ===
        if mode=='expand':
            needtoregrid=False
	    bychanindx=False

	    try: 
		print "expand mode..."
		# do not allow list in this mode (for inpimage and inpmask)
		if type(inpimage)==list:
		    raise TypeError, 'A list for inpimage is not allowed for mode=expand'
		if type(inpmask)==list:
		    raise TypeError, 'A list for inpmask is not allowed for mode=expand'

		 
		# input image info
		ia.open(inpimage)
		inshp = ia.shape()
		incsys = ia.coordsys().torecord()
		# if outimage not specified modified inpimage
		if outimage=='': outimage=inpimage
		    # no regrid is necessary
		    # case 1:  
		    # work mask manipulation in images
		    # if inmask is not empty need to extract embedded mask
		    # if outmask is not empty put the resultant mask image as a T/F mask 
		ia.close() 

		if inpmask!='':
		  # need to extract the mask and put in tmp_maskimage
		    pixelmask2cleanmask(imagename=inpimage, maskname=inpmask, maskimage=tmp_maskimage, usemasked=True)    
		    ia.open(tmp_maskimage)
		else:
		    # copy of inpimage in tmp_maskimage
		    ia.fromimage(outfile=tmp_maskimage, infile=inpimage)
		 
		# check for working output image
		if not os.path.exists(tmp_outmaskimage):
		    # if tmp_outmaskimage does not exist yet make a copy
		    shutil.copytree(tmp_maskimage,tmp_outmaskimage)
		    needtoregrid=False

		# get mask info from input image
		# regrid output image first???
		# if infreq/outfreq are channel indices (int) then
		# regrid in x y coords only and extract specified channel mask
		# to specified output channels. (no regriding in spectral axis)
		# if infreqs/outfreqs are velocity or freqs, 
		# it assumes it expressed in the range with minval~maxval
		# create subimage of the input mask with the range,
		# do regrid with the subimage to output.
	  
		# decide to regrid or not
		# 1. for the case all channels are selected for input and output, simply regrid
		# 2. for inpfreqs and outfreqs are integers (= channel indices), regrid only in
		#    first and second axes (e.g. ra,dec) 
		
		if (inpfreqs==[[]] or inpfreqs==[]) and (outfreqs==[[]] or outfreqs==[]):
		    # try regrid
		    needtoregrid=True
		    # detach input image and open output tmp image
		    ia.open(tmp_outmaskimage)
		else: 
		    # selection by channel indices (number) 
		    # if both inpfreqs and outfreqs are int skip regridding
		    # if outfreqs is vel or freq ranges, try regridding ???
		    if inpfreqs==[[]] or inpfreqs==[]:
			inpfreqs=range(inshp[3])
			print "set inpfreqs=",inpfreqs
		    refchanst = inpfreqs[0]
		    refchanen = inpfreqs[-1]
		    if type(outfreqs[0])==int:
			bychanindx=True
		    #slst = [inshp[0]-1,inshp[1]-1,0,refchanst]
		    #slen = [inshp[0]-1,inshp[1]-1,0,refchanen]
		    #refchanstim = ia.getchunk(blc=slst,trc=slst)
		    #refchanenim = ia.getchunk(blc=slen,trc=slen)
		    slst = [0,0,0,refchanst]
		    slen = [inshp[0]-1,inshp[1]-1,0,refchanen]
		    print "getting chunk at blc=",slst," trc=",slen
		    #refchanchunk=ia.getchunk(blc=slst,trc=slen).transpose()
		    refchanchunk=ia.getchunk(blc=slst,trc=slen)
		    print "Before transpose refchanchunk.shape=",refchanchunk.shape
		    refchanchunk=refchanchunk.transpose()
		    ia.close()
		    print "refchanchunk:shape=",refchanchunk.shape
	     
		    ia.open(tmp_outmaskimage)
		    print "tmp_outmaskimage=",tmp_outmaskimage, " shape=",ia.shape()
		    # need find nearest inchan 
		    # store by chan indices (no regrid)
		    usechanims={}  # list of input mask to be use for each outpfreq
		    for i in outfreqs:
			nearestch = findnearest(inpfreqs,i)
			usechanims[i]=nearestch 
		    print "usechanims=",usechanims
		    for j in outfreqs:
			pix = refchanchunk[usechanims[j]-refchanst]
			#print "pix=",pix
			print "pix.shape=",pix.shape
			print "inshp=",inshp, ' j=',j
			#ia.putchunk(pixels=pix,blc=[inshp[0]-1,inshp[1]-1,0,j])
			ia.putchunk(pixels=pix.transpose(),blc=[0,0,0,j])
			print "DONE putchunk for j=", j 
		# regrid if spectral axis is different
		osys=ia.coordsys()
		oshp=ia.shape()
		print "out shape(oshp)=",oshp
		print "out csys(osys)=",osys.torecord()
		# need regrid
		#if inshp!=oshp or incsys!=osys.torecord() and outfreqs is not specified
		#by channel no.

		if (inshp[0]!=oshp[0] or inshp[1]!=oshp[1] or inshp[3]!=oshp[3]) and not bychanindx:
		    needtoregrid=True

		if needtoregrid:
		    print "Regridding ..."
		    # closing current output image
		    ia.close()
		    ia.open(tmp_maskimage)
		    os.system('cp -r %s beforeregrid.im' % tmp_maskimage)
		    print "Removing %s" % tmp_outmaskimage
		    os.system('rm -r '+tmp_outmaskimage)
		    ia.regrid(outfile=tmp_outmaskimage, shape=oshp, axes=[0,1,3], 
			      csys=osys.torecord(),method='neareset',force=True, overwrite=True)
		    #ia.close()
		    ia.remove()
		    print "closing after regrid"
		    ia.open(tmp_outmaskimage) # reopen output tmp image
		if outmask!='':
		    #convert the image mask to T/F mask
		    print "convert the image mask to T/F mask"
		    ia.calcmask(mask='%s<0.5' % tmp_outmaskimage,name=outmask,asdefault=True)
		ow = False
		if inpimage==outimage:
		    casalog.post("Updating "+outimage+"with new mask","WARN")
		else:
		    casalog.post(outimage+" exists, overwriting","WARN")
		    ow=True
		ia.rename(outimage,ow)
		ia.close()
		#ia.remove(tmp_outmaskimage)
		# delete tmp_maskimage...

            except Exception, instance:
                print "*** Error ***", instance
                ia.close()
                raise Exception, instance
	    finally:
		if os.path.exists(tmp_maskimage):
		    shutil.rmtree(tmp_maskimage)
	#            
	#       (5) Expand a boolean mask from one range of channels to another range
	#     in the same image.
	#
	#     makemask(mode='expand',
	#              inpimage='oldmask.im', inpmask='mask0', inpfreq=[5,6], outfreq=[4,5,6,7],
	#              outimage='oldmask.im', outmask='mask0')


	#(6) Expand a boolean mask from a range of channels in the input image to another range
	#     of channels in a  different image with a different spectral-coordinate system.
	#     Save the mask as ones/zeros so that it can be given to 'clean'.
	#
	#     makemask(mode='expand',
	#              inpimage='smallmask.im', inpmask='mask0', inpfreq='1.5MHz~1.6MHz', outfreq='1.2MHz~1.8MHz',
	#              outimage='bigmask.im', outmask='')

    # === merge mode ===
	# avaialble parameters: inpimage (list/string) , inpmask (list/string), outimage(string), outmask(string)
	# input inpimage as a list (of image mask 1/0) , inpmask as list of image containing bool-mask
	# makemask(mode='merge', inpimage=['mask1.im','mask2.im'],
	#           inpmask=['myimage1.im','myimage2.im:mask2','myimage2.im:mask3'], ...
	# if inpimage = string but not a list or single elem. list, it will try to merge all bool mask image to one bool
	# mask name in outmask(''=mask0) of outimage.  e.g makemask(mode='merge', inpimage='myimage1.im', outmask='')
	# if outmask ='', create an image mask. if outmask !='': write boolean mask of name specified in outmask in outimage
	if mode=='merge':
	    sum_tmp_outfile='__tmp_outputmask'
            tmp_inmask='__tmp_frominmask'
            try:
		if not os.path.isdir(outimage):
		    shutil.copytree(inpimage,sum_tmp_outfile)
		    #print "copied inpimage to tmp file"
		else:
		    shutil.copytree(outimage,sum_tmp_outfile)
		    #print "copied outimage to tmp file"
     
		if type(inpimage)==list and len(inpimage)>1:
		    # summing all the images
                    casalog.post('Summing all mask images in inpimage and  normalized to 1 for mask','INFO')
		    for img in inpimage:
                        tmpregrid='__tmp_regrid.'+img
			regridmask(img,sum_tmp_outfile,tmpregrid)
                        #print "addimagemask... for ",tmpregrid
			addimagemask(sum_tmp_outfile,tmpregrid)
                        shutil.rmtree(tmpregrid)
		    # get boolean masks
		    #  will work in image(1/0) masks
		if type(inpmask)==list and len(inpmask)>1:
                    casalog.post('Summing all T/F mask in inpmask and normalized to 1 for mask','INFO')
		    for msk in inpmask:
			if msk.find(':')<0:
			    # assume default mask
			    msk=msk+':mask0'
			ia.open(msk)
			inmasks=ia.maskhandler('get')
			if not inmasks.count(msk.split(':')[-1]):
			    raise TypeError, msk.split(':')[-1]+" does not exist in "+msk.split(':')[0]  
			# move T/F mask to image mask
			pixelmask2cleanmask(msk.split(':')[-2], msk.split(':')[-1], tmp_inmask, True)    
			regridmask(tmp_inmask,sum_tmp_outfile,'__tmp_fromTFmask')
			addimagemask(sum_tmp_outfile,'__tmp_fromTFmask')
                        shutil.rmtree('__tmp_fromTFmask') 

		if outmask!='':
                    casalog.post('Putting mask in T/F','INFO')
		    ia.open(sum_tmp_outfile)
		    ia.calcmask(mask='%s<0.5' % sum_tmp_outfile,name=outmask,asdefault=True)
		    ia.done()
	        # if outfile exist initially outfile is copied to sum_tmp_outfile
		# if outfile does not exist initially sum_tmp_outfile is a copy of inpimage
		# so rename it with overwrite=T all the cases
                #print "open sum_tmp_outfile=",sum_tmp_outfile
		ia.open(sum_tmp_outfile) 
		ia.rename(outimage,overwrite=True) 
		ia.done()

            except Exception, instance:
                print "*** Error ***", instance
                raise Exception, instance
	    finally:
		if os.path.exists(sum_tmp_outfile):
		    shutil.rmtree(sum_tmp_outfile)
		if os.path.exists(tmp_inmask):
		    shutil.rmtree(tmp_inmask)
                if type(inpimage)==list:
                   for im in inpimage:
                       if os.path.isdir('__tmp_regrid.'+im):
		            shutil.rmtree('__tmp_regrid.'+im)
                       
             
                 
    # === draw mode ===
    # disabled - drawmaskinimage (working with viewer) a bit flaky
    # when run in succession.
    #    if mode=='draw':
    #        #call drawmaskinimage
    #        from recipes.drawmaskinimage import drawmaskinimage
    #        drawmaskinimage(inpimage,outmask)
            

    except Exception, instance:
        print '*** Error ****', instance
        raise Exception, instance

    finally:
        # final clean up 
	if os.path.exists(tmp_maskimage):
	    shutil.rmtree(tmp_maskimage)
	if os.path.exists(tmp_outmaskimage):
	    shutil.rmtree(tmp_outmaskimage)

def findnearest(arr, val):
    import numpy
    if type(arr)==list:
        arr = np.array(arr) 
    indx = numpy.abs(arr - val).argmin()
    return arr[indx] 

def regridmask(inputmask,template,outputmask,axes=[3,0,1],method='linear'):
    '''
    Regrid input mask (image) to output mask using a template.
    Currently the template must be a CASA image.
    The default interpolation method is set to 'linear' (since 'nearest'
    sometime fails).
    '''
    if not os.path.isdir(template):
        raise IOError, "template image %s does not exist" % template
    
    (ia,) = gentools(['ia']) 
    ia.open(template)
    ocsys = ia.coordsys()
    oshp = ia.shape()
    ia.done()
    ia.open(inputmask)
    # check axis order, if necessary re-interprete input axes correctly 
    # assumed order of axes 
    reforder=['Right Ascension', 'Declination', 'Stokes', 'Frequency']
    axisorder=ia.summary(list=False)['axisnames'].tolist()
    tmp_axes=[]
    for axi in axes:
        tmp_axes.append(axisorder.index(reforder[axi]))        
    print "tmp_axes=",tmp_axes
    axes=tmp_axes
    ir=ia.regrid(outfile=outputmask,shape=oshp,csys=ocsys.torecord(),axes=axes,method=method)       
    ia.done()
    # to ensure to create 1/0 mask image
    ir.calc('iif (%s>0.0 && %s<1.0,1,%s)'%(outputmask,outputmask,outputmask))
    ir.done()

def addimagemask(sumimage, imagetoadd, threshold=0.0):
    (ia,) = gentools(['ia']) 
    print "addimagemask: sumimage=",sumimage," imagetoadd=",imagetoadd
    ia.open(sumimage)
    ia.calc('iif ('+imagetoadd+'>'+str(threshold)+',('+sumimage+'+'+imagetoadd+')/('+sumimage+'+'+imagetoadd+'),'+sumimage+')')
    print "calc done"
    ia.close()  
