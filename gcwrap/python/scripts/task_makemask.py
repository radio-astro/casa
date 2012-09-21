################################################
# Task to make masks.
#  modified by TT
# based on the original code, 
# v1.0: 2012.03.20, U.Rau
#
################################################
# Notes (to self) - TT 
# 1. expanding one mask to another 
#    e.g.) expanding a continuum mask (both image mask/boolean mask)
#          channel mask 
# 2. merging of different types of masks 
#    e.g.) inpimage and inpmask are lists of the mask to be merged
#          output mask is written to either outimage or outmask as embedded
#           T/F mask 
# 3. copying mask to another or create a new one
#    regrid if necessary (i.e. if the coords are different) 
# ----------------------------------------------
# basic rules:
# For input,
# if inpmask='': use inpimage as input mask
# if inpmask='mask0' or other embedded mask name of the inpimage, 
# use that T/F mask
# 
# =expand=
# case1: on the same image (outimage=''), expand mask image from 
# prev. run etc. No regriding. Use nearest chan mask
# image to expand.  
# 1.a: inpimage is clean image mask (1s and 0s)
#      i) outimage != inpimage and outmask='' => new expanded mask image to outimage
#     ii) outimage != inpimage and outmask!='' => convert expanded mask image to T/F mask to store inside outimage
#    iii) outimage ==inpimage and outmask='' => update input mask image by expanding it 
#     iv) outimage ==inpimage and outmask!=''=> update input image with the expanded T/F mask
# 1.b: if inpmask!='', do T/F mask to 1/0 image mask conversion, then do as in 1.a 
 
# case2: outimage is in diffirent coords. (need to regrid)
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
(csys,) = gentools(['cs']) 

def makemask(mode, outimage, outmask, inpimage, inpmask, inpfreqs, outfreqs, template):
    """
    make /manipulate masks

    """
    (ia,) = gentools(['ia']) 
    casalog.origin('makemask')
    try:
        # temp files
        tmp_maskimage='__tmp_makemaskimage'
	tmp_outmaskimage='__tmp_outmakemaskimage'
        tmp_regridim='__tmp_regridim'

        if mode != 'merge':
           if outimage=='':
               outimage=inpimage

	   if inpimage==outimage:
	       tmp_outmaskimage=tmp_maskimage

    # the following code is somewhat duplicated among the modes but keep separated from each mode
    # for now....
    # === copy mode ===
        if mode=='copy':
           #print "Copy mode"
           needregrid=True
           #if outimage=='':
               #overwrite
           #    outimage=inpimage

           if not os.path.isdir(outimage):
               needregrid=False

	   if inpmask!='':
	   # need to extract the mask and put in tmp_maskimage
	       pixelmask2cleanmask(imagename=inpimage, maskname=inpmask, maskimage=tmp_maskimage, usemasked=True)    
	   else:
	       shutil.copytree(inpimage, tmp_maskimage)
           if needregrid:
               casalog.post("Regridding...",'DEBUG1')
               regridmask(tmp_maskimage,outimage,tmp_outmaskimage)
               # regrid may produce <1.0 pixels for mask so be sure to its all in 1.0
               #ia.open(tmp_outmaskimage) 
               #ia.calc('iif (%s>0.0 && %s<1.0,1,%s)'%(tmp_outmaskimage,tmp_outmaskimage,tmp_outmaskimage))
               #ia.close()
               #print "Copying regrid output=",tmp_outmaskimage
           else:
               shutil.copytree(tmp_maskimage,tmp_outmaskimage)
	   if outmask!='':
	   #convert the image mask to T/F mask
               if not os.path.isdir(outimage):
                   shutil.copytree(inpimage,outimage)
               #
               ia.open(outimage)
	       casalog.post("convert the output image mask to T/F mask")
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
            (rg,) = gentools(['rg']) 
            needtoregrid=False
	    bychanindx=False

	    try: 
		#print "expand mode..."
		# do not allow list in this mode (for inpimage and inpmask)
		if type(inpimage)==list:
		    raise TypeError, 'A list for inpimage is not allowed for mode=expand'
		if type(inpmask)==list:
		    raise TypeError, 'A list for inpmask is not allowed for mode=expand'

		 
		# input image info
		ia.open(inpimage)
		inshp = ia.shape()
		incsys = ia.coordsys()
		# if outimage not specified modified inpimage
		#if outimage=='': outimage=inpimage
		    # no regrid is necessary
		    # case 1:  
		    # work mask manipulation in images
		    # if inmask is not empty need to extract embedded mask
		    # if outmask is not empty put the resultant mask image as a T/F mask 
		ia.close() 

                # prepare working input image (tmp_maskiamge)
		if inpmask!='':
		  # need to extract the mask and put in tmp_maskimage
		    pixelmask2cleanmask(imagename=inpimage, maskname=inpmask, maskimage=tmp_maskimage, usemasked=True)    
		    ia.open(tmp_maskimage)
		else:
		    # copy of inpimage in tmp_maskimage
		    ia.fromimage(outfile=tmp_maskimage, infile=inpimage)

                #setting up the output image (copy from inpimage or template)
                if not os.path.isdir(outimage):
                    if template=='':
                        shutil.copytree(tmp_maksimage,tmp_outmaskimage)
                        needtoregrid=False
                    else:
                        if os.path.isdir(template):
                            # make empty image....
                            ia.open(template)
                            tshp=ia.shape()
                            tcsys=ia.coordsys()
                            ia.close()
                            ia.fromshape(tmp_outmaskimage,shape=tshp,csys=tcsys.torecord())
                            ia.done()
                        else:
                            raise IOError, "template image, %s does not exist" % template
                else:
                    shutil.copytree(outimage,tmp_outmaskimage)
                     
		# check for working output image
		#if not os.path.exists(tmp_outmaskimage):
		    # if tmp_outmaskimage does not exist yet make a copy
		#    shutil.copytree(tmp_maskimage,tmp_outmaskimage)
		#    needtoregrid=False

		# get mask info from input image
		# regrid output image first???
		# if inpfreq/outfreq are channel indices (int) then
		# regrid in x y coords only and extract specified channel mask
		# to specified output channels. (no regriding in spectral axis)
		# if inpfreqs/outfreqs are velocity or freqs, 
		# it assumes it expressed in the range with minval~maxval
		# create subimage of the input mask with the range,
		# do regrid with the subimage to output.
	  
		# decide to regrid or not
		# 1. for the case all channels are selected for input and output, simply regrid
		# 2. for inpfreqs and outfreqs are integers (= channel indices), regrid only in
		#    first and second axes (e.g. ra,dec) 
		
		if ((inpfreqs==[[]] or inpfreqs==[]) and (outfreqs==[[]] or outfreqs==[])) \
                    or (inpfreqs=='' and outfreqs==''):
		    # try regrid
		    needtoregrid=True
		    # detach input(tmp) image and open output tmp image
		    ia.open(tmp_outmaskimage)
		else: 
                    # make sure tmp_maskimage is open
                    if ia.isopen():
                        if ia.name(strippath=True)!=tmp_maskimage:
                            ia.close()
                            ia.open(tmp_maskimage)
                    else:
                        ia.open(tmp_maskimage)

		    # selection by channel indices (number) 
		    # if both inpfreqs and outfreqs are int skip regridding
		    # if outfreqs is vel or freq ranges, try regridding 
		    if inpfreqs==[[]] or inpfreqs==[]:
                        # select all channels for input
			inpfreqs=range(inshp[3])
			#print "set inpfreqs=",inpfreqs

                    # check inpfreqs and outfreqs types
                    selmode='bychan'
                    if type(inpfreqs)==list:
                        if type(inpfreqs[0])==int:
                            if type(outfreqs)==list and type(outfreqs[0])==int:
                                selmode='bychan'
                            elif type(outfreqs)==str:
                                if inpfreqs[0]==0: #contintuum
                                    selmode='byvf'
                                else:
                                    raise TypeError, "Mixed types in infreqs and outfreqs" 
                        else:
                            raise TypeError, "Non-integer in inpfreq is not supported" 
                    elif type(inpfreqs)==str:
                        if type(outfreqs)!=str:
                            raise TypeError, "Mixed types in infreqs and outfreqs" 
                        selmode='byvf'
                    else:
                        raise TypeError, "Wrong type for infreqs"

                    # inpfreqs and outfreqs are list of int
                    # match literally without regridding.
                    #if type(inpfreqs)==list and type(inpfreqs[0]==int):
                    if selmode=='bychan': 
                        casalog.post("selection of input and output ranges by channel")
		        refchanst = inpfreqs[0]
		        refchanen = inpfreqs[-1]
			bychanindx=True
		        slst = [0,0,0,refchanst]
		        slen = [inshp[0]-1,inshp[1]-1,0,refchanen]
		        #print "getting chunk at blc=",slst," trc=",slen
		        #refchanchunk=ia.getchunk(blc=slst,trc=slen).transpose()
		        refchanchunk=ia.getchunk(blc=slst,trc=slen)
		        #print "Before transpose refchanchunk.shape=",refchanchunk.shape
		        refchanchunk=refchanchunk.transpose()
		        ia.close()
		        #print "refchanchunk:shape=",refchanchunk.shape
	     
		        ia.open(tmp_outmaskimage)
		        # need find nearest inchan 
		        # store by chan indices (no regrid)
		        usechanims={}  # list of input mask to be use for each outpfreq
		        for i in outfreqs:
			    nearestch = findnearest(inpfreqs,i)
			    usechanims[i]=nearestch 
		        #print "usechanims=",usechanims
		        for j in outfreqs:
			    pix = refchanchunk[usechanims[j]-refchanst]
			    #print "pix=",pix
			    #print "pix.shape=",pix.shape
			    #print "inshp=",inshp, ' j=',j
			    #ia.putchunk(pixels=pix,blc=[inshp[0]-1,inshp[1]-1,0,j])
			    ia.putchunk(pixels=pix.transpose(),blc=[0,0,0,j])

                    elif selmode=='byvf': # outfreqs are quantities (freq or vel)
                        casalog.post("selection of input/output ranges by frequencies/velocities")
                        
                        inpfreqlist = translatefreqrange(inpfreqs,incsys)
                        # close input image
                        ia.close()
                        #regrid to output image coordinates
                        if len(inpfreqlist)==1: # continuum
                            #do not regrid, use input image
                            shutil.copytree(tmp_maskimage,tmp_regridim)
                        else:
                            regridmask(tmp_maskimage,template,tmp_regridim,chanrange=inpfreqlist)
                            # find edge masks (nonzero planes)
                            ia.open(tmp_regridim)
                            sh=ia.shape()
                            chanlist=range(sh[3])
                            for i in chanlist:
                                sl1=[0,0,0,i]
                                sl2=[sh[0]-1,sh[1]-1,sh[2]-1,i]
                                psum = ia.getchunk(sl1,sl2).sum()
                                if psum>0.0: 
                                    indlo=i
                                    break
                            chanlist.reverse()
                            for i in chanlist:
                                sl1=[0,0,0,i]
                                sl2=[sh[0]-1,sh[1]-1,sh[2]-1,i]
                                psum = ia.getchunk(sl1,sl2).sum()
                                if psum>0.0: 
                                    indhi=i
                                    break
                            #print "indhi=",indhi, " indlo=",indlo
                            if indhi < indlo:
                                raise IOError, "Incorrectly finding edges of input masks! Probably some logic error in the code!!!" 

                        # find channel indices for given outfreqs
                        ia.open(tmp_outmaskimage)
                        ocsys=ia.coordsys()
                        oshp=ia.shape() 
                        outfreqlist = translatefreqrange(outfreqs,ocsys)
                        rtn=ocsys.findcoordinate('spectral')
                        px=rtn[1][0]
                        wc=rtn[2][0]
                        world=ocsys.referencevalue()
                        # assume chanrange are in freqs
                        world['numeric'][wc]=qa.convert(qa.quantity(outfreqlist[0]),'Hz')['value']
                        p1 = ocsys.topixel(world)['numeric'][px]
                        world['numeric'][wc]=qa.convert(qa.quantity(outfreqlist[1]),'Hz')['value']
                        p2 = ocsys.topixel(world)['numeric'][px]
                        #print "p1=",p1, " p2=",p2
                        if len(inpfreqs)==1:
                            inpfreqchans=inpfreqs
                        else:
                            inpfreqchans=[indlo,indhi]
                        outfreqchans=range(int(p1),int(p2)+1)
                        #print "inpfreqchans=",inpfreqchans
                        #print "outfreqchans=",outfreqchans

                        expandchanmask(tmp_regridim,inpfreqchans,tmp_outmaskimage,outfreqchans)

#		        usechanims={}  # list of input mask to be use for each outpfreq
#		        for i in outfreqchans:
#			    nearestch = findnearest(inpfreqchans,i)
#			    usechanims[i]=nearestch 
#                        # put masks from inp image channel by channel
#		        for j in outfreqs:
#		            pix = refchanchunk[usechanims[j]-refchanst]
#			    #ia.putchunk(pixels=pix,blc=[inshp[0]-1,inshp[1]-1,0,j])
#			    ia.putchunk(pixels=pix.transpose(),blc=[0,0,0,j])
                        needtoregrid=False
                        ia.open(tmp_outmaskimage)
                # 
                
		# regrid if spectral axis is different
		#osys=ia.coordsys()
		#oshp=ia.shape()
		# need regrid
		#if inshp!=oshp or incsys!=osys.torecord() and outfreqs is not specified
		#by channel no.
                # 
		#if (inshp[0]!=oshp[0] or inshp[1]!=oshp[1] or inshp[3]!=oshp[3]) and not bychanindx:
		#    needtoregrid=True

		if needtoregrid:
		    print "Regridding ..."
		    # closing current output image
		    ia.close()
		    ia.open(tmp_maskimage)
		    os.system('cp -r %s beforeregrid.im' % tmp_maskimage)
		    print "Removing %s" % tmp_outmaskimage
		    os.system('rm -r '+tmp_outmaskimage)
                    regridmask(tmp_maskimage,outimage,tmp_outmaskimage)
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
		ia.done()

            except Exception, instance:
                print "*** Error ***", instance
                ia.done()
                raise Exception, instance
	    finally:
		if os.path.exists(tmp_maskimage):
		    shutil.rmtree(tmp_maskimage)
		if os.path.exists(tmp_regridim):
		    shutil.rmtree(tmp_regridim)
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
            #print "merge mode..."
	    sum_tmp_outfile='__tmp_outputmask'
            tmp_inmask='__tmp_frominmask'
            try:
		if not os.path.isdir(outimage):
                    # figure out which input mask to be used as template
                    # if inpimage is defined use the first one else try the first one
                    # inpmask
                    if outimage=='':
                        if type(inpimage)==list:
                             outimage=inpimage[0]
                        elif type(inpimage)==str:
                             outimage=inpimage
                        casalog.post("No outimage is specified. Will overwrite input image: "+outimage,'INFO')
                    if type(inpimage)==list and len(inpimage)!=0:
                        tmp_template=inpimage[0]
                    elif inpimage!='' and inpimage!=[]:
                        tmp_template=inpimage # string
                    else:
                        if type(inpmask)==list and len(inpmask)!=0:
                            range=inpmask[0].rfind(':')
                            if range ==-1:
                                raise IOError, "Cannot resolve inpmask name, check the format"
                            else:
                                tmp_template=inpmask[0][:inpmask[0].rfind(':')]
                        elif inpmask!='' and inpmask!=[]:
                            # this is equivalent to 'copy' the inpmask
                            tmp_template=inpmask #string
                        
		    shutil.copytree(tmp_template,sum_tmp_outfile)
		    #print "copied inpimage to tmp file"
		else:
		    shutil.copytree(outimage,sum_tmp_outfile)
		    #print "copied outimage to tmp file"
     
                if type(inpimage)==str:
                    inpimage=[inpimage]
                if type(inpmask)==str:
                    inpmask=[inpmask]

		if len(inpimage)>0:
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
                
		if len(inpmask)>0:
                    casalog.post('Summing all T/F mask in inpmask and normalized to 1 for mask','INFO')
		    for msk in inpmask:
			if msk.find(':')<0:
			    # assume default mask
			    msk=msk+':mask0'
                        imname=msk[:msk.rfind(':')]
			ia.open(imname)
			inmasks=ia.maskhandler('get')
                        ia.close()
                        #print "inmasks=",str(inmasks)
			if not inmasks.count(msk.split(':')[-1]):
			    raise TypeError, msk.split(':')[-1]+" does not exist in "+imname+" -available masks:"+str(inmasks)
			# move T/F mask to image mask
			pixelmask2cleanmask(imname, msk.split(':')[-1], tmp_inmask, True)    
			regridmask(tmp_inmask,sum_tmp_outfile,'__tmp_fromTFmask')
			addimagemask(sum_tmp_outfile,'__tmp_fromTFmask')
                        shutil.rmtree('__tmp_fromTFmask') 

		if outmask!='':
                    casalog.post('Putting mask in T/F','INFO')
		    ia.open(sum_tmp_outfile)
		    ia.calcmask(mask='%s<0.5' % sum_tmp_outfile,name=outmask,asdefault=True)
		    ia.done()
	        # if outfile exists initially outfile is copied to sum_tmp_outfile
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
	if os.path.isdir(tmp_maskimage):
	    shutil.rmtree(tmp_maskimage)
	if os.path.isdir(tmp_outmaskimage):
	    shutil.rmtree(tmp_outmaskimage)
        if os.path.isdir(tmp_regridim):
            shutil.rmtree(tmp_regridim)

def findnearest(arr, val):
    import numpy as np
    if type(arr)==list:
        arr = np.array(arr) 
    indx = np.abs(arr - val).argmin()
    return arr[indx] 

def regridmask(inputmask,template,outputmask,axes=[3,0,1],method='linear',chanrange=None):
    '''
    Regrid input mask (image) to output mask using a template.
    Currently the template must be a CASA image.
    The default interpolation method is set to 'linear' (since 'nearest'
    sometime fails).
    '''
    #print "Regrid.."
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
    axes=tmp_axes
    #print "chanrange=",chanrange
    if type(chanrange)==list and len(chanrange)==2:
        incsys=ia.coordsys()
        spaxis=incsys.findcoordinate('spectral')[2]
        # create subimage based on the inpfreqs range
        inblc=chanrange[0]
        intrc=chanrange[1]
        #print "spaxis=",spaxis," inblc=",inblc," intrc=",intrc
        rgn = rg.wbox(blc=inblc,trc=intrc,pixelaxes=spaxis.tolist(),csys=incsys.torecord())
    else:
        rgn={}     
    ir=ia.regrid(outfile=outputmask,shape=oshp,csys=ocsys.torecord(),axes=axes,region=rgn,method=method)       
    ia.done()
    # to ensure to create 1/0 mask image
    ir.calc('iif (%s>0.0 && %s<1.0,1,%s)'%(outputmask,outputmask,outputmask))
    ir.done()

def addimagemask(sumimage, imagetoadd, threshold=0.0):
    """
    add image masks (assumed the images are already in the same coordinates)
    """
    (ia,) = gentools(['ia']) 
    #print "addimagemask: sumimage=",sumimage," imagetoadd=",imagetoadd
    ia.open(sumimage)
    ia.calc('iif ('+imagetoadd+'>'+str(threshold)+',('+sumimage+'+'+imagetoadd+')/('+sumimage+'+'+imagetoadd+'),'+sumimage+')')
    ia.close()  

def expandchanmask(inimage,inchans,outimage,outchans):
    """
    expand masks in channel direction,and insert then
    to output image with the same coordinates (post-regridded)
    only differ by channels
    """
    # input image
    ia.open(inimage)
    inshp=ia.shape()
    refchanst=inchans[0]
    refchanen=inchans[-1]
    slst = [0,0,0,refchanst]
    slen = [inshp[0]-1,inshp[1]-1,0,refchanen]
    casalog.post("getting chunk at blc="+str(slst)+" trc="+str(slen),'DEBUG1')
    refchanchunk=ia.getchunk(blc=slst,trc=slen)
    refchanchunk=refchanchunk.transpose()
    ia.close()
    #print "refchanchunk:shape=",refchanchunk.shape

    ia.open(outimage)
    # need find nearest inchan
    # store by chan indices (no regrid)
    usechanims={}  # list of input mask to be use for each outpfreq
    for i in outchans:
        nearestch = findnearest(inchans,i)
        usechanims[i]=nearestch
    casalog.post("Mapping of channels: usechanims="+str(usechanims),'DEBUG1')
    for j in outchans:
        pix = refchanchunk[usechanims[j]-refchanst]
        #print "pix=",pix
        #print "pix.shape=",pix.shape
        #print "inshp=",inshp, ' j=',j
        #ia.putchunk(pixels=pix,blc=[inshp[0]-1,inshp[1]-1,0,j])
        ia.putchunk(pixels=pix.transpose(),blc=[0,0,0,j])
        #print "DONE putchunk for j=", j
    ia.done()

def translatefreqrange(freqrange,csys):
    """
    convert the range in list
    mainly for frequeny and velocity range determination
    """
    if type(freqrange)==list and type(freqrange[0])==int:
        #do nothing
        return freqrange
    elif type(freqrange)==str:
        freqlist=freqrange.split('~') 
        for i in range(len(freqlist)):
            if freqlist[i].find('m/s') > -1:
               fq = qa.quantity(freqlist[i])
               vf=csys.velocitytofrequency(value=fq['value'],velunit=fq['unit'])
               freqlist[i]=str(vf[0])+'Hz'
        return freqlist
    else:
        raise TypeError, "Cannot understand frequency range"
