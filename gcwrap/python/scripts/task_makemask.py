################################################
# Task to make masks.
#  reorganized after diesuccsion on Oct1, 2012
#  with Jeurgen and Urvashi
#
#  modified by TT
# based on the original code, 
# v1.0: 2012.03.20, U.Rau
#
################################################
# Notes (to self) - TT 
# 1. expanding one mask to another 
#    e.g.) expanding a continuum mask (both image mask/boolean mask)
#          channel mask 
# 2. part of copy mode func.: merging of different types of masks 
#    e.g.) inpimage and inpmask are lists of the mask to be merged
#          output mask is written to either outimage or outmask as embedded
#           T/F mask 
# 3. copying mask to another or create a new one
#    regrid if necessary (i.e. if the coords are different) 
# ----------------------------------------------
# basic rules:
# for mask image (1/0 mask): as it is
# for internal mask : parent_imagename:mask_name
#
# For input,
# inpimage is the casa image
# - mode='list': list internal masks of inpimage
# - other mode: used as a template for output
#               if region files are specified -> make mask specifeid with the regions on to inpimage
#               output is '' => modified inpimage unless overwrite=F else exception
#
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
import shutil
from taskinit import *
from recipes.pixelmask2cleanmask import pixelmask2cleanmask
(csys,) = gentools(['cs']) 

def makemask(mode,inpimage, inpmask, output, overwrite, inpfreqs, outfreqs):
    """
    make /manipulate masks

    """
    (ia,rg,im,) = gentools(['ia','rg','im']) 
    casalog.origin('makemask')
    #print "params(mode,inpimage,inpmask,output,overwrite)=",mode,inpimage,inpmask,output,overwrite

    try:
        # temp files
        tmp_maskimage='__tmp_makemaskimage'
	tmp_outmaskimage='__tmp_outmakemaskimage'
        tmp_regridim='__tmp_regridim'

        # do parameter check first
        # check names of inpimage, inpmask check for existance
        # inpimage == output (exact match) then check overwrite
        #   => T overwrite inpimage
        #   => F exception

        # check inpimage 
        if (['list','copy','expand'].count(mode)==1):
            if inpimage=='': raise Exception, "inpimage is empty"
            if not os.path.isdir(inpimage):
                raise Exception, "inpimage=%s does not exist" % inpimage    
       
        # === list mode ===
        if mode == 'list':
           inpOK=checkinput(inpimage)
           if inpOK: 
              
               ia.open(inpimage)
               inmasklist=ia.maskhandler('get')
               # now ia.maskhandler returns ['T'] if no internal mask is there...
               if inmasklist.count('T')!=0:
                   inmasklist.remove('T')
               if len(inmasklist) ==0:
                   casalog.post('No internal (T/F) masks were found in %s' % (inpimage),'INFO')
               else:
                   defaultmaskname=ia.maskhandler('default')[0]
                   printinmasks=''
                   for mname in inmasklist:
                       if mname==defaultmaskname:
                           printinmasks+='\''+mname+'\''+'(default)'
                       else:
                           printinmasks+='\''+mname+'\''
                       if mname != inmasklist[-1]:
                           printinmasks+=', '
                 
                   casalog.post('Internal (T/F) masks in %s: %s' % (inpimage, printinmasks),'INFO')
               ia.close()
 
        elif mode == 'setdefaultmask':
            inpOK=checkinput(inpmask)
            if inpOK:
                (parentimage,bmask)=extractmaskname(inpmask)
		if bmask=='':
		    raise Exception, "Missing an internal mask name"
                ia.open(parentimage)
                defaultmaskname=ia.maskhandler('default')[0]
                inmasklist=ia.maskhandler('get')
                if defaultmaskname==bmask:
                    casalog.post('No change. %s is already a default internal mask' % bmask, 'INFO')
                else:
                    ia.maskhandler('set',bmask)
                    casalog.post('Set %s as a default internal mask' % bmask, 'INFO')
                    if len(inmasklist)>1:
                        casalog.post('Current internal masks are %s' % str(inmasklist), 'INFO')
                ia.close()

        elif mode == 'delete':
            inpOK=checkinput(inpmask)
            if inpOK:
                (parentimage,bmask)=extractmaskname(inpmask)
                if bmask=='':
                    raise Exception, "Missing an internal mask name"
                ia.open(parentimage)
                casalog.post('Deleting the internal mask, %s ' % bmask, 'INFO')
                defaultmaskname=ia.maskhandler('default')[0]
                ia.maskhandler('delete',bmask)
                inmasklist=ia.maskhandler('get')
                if inmasklist.count('T')!=0:
                    inmasklist.remove('T')
                if len(inmasklist) !=0 and defaultmaskname==bmask:
                    ia.maskhandler('set',inmasklist[0])
                    casalog.post('Set %s as a default internal mask' % inmasklist[0], 'INFO')
                    if len(inmasklist)>1:
                        casalog.post('Current internal masks are %s' % str(inmasklist), 'INFO')
          
                ia.close()

        else:
           #DEBUG
           #print "mode=",mode
           import commands
           # copy can have multiple input masks, expand has only one.
           # check inpimage, inpmask, output, overwrite
           # 
           storeinmask = False # used to check if output to a new internal mask
           inpOK=checkinput(inpimage)
           if inpOK:
               (immask,inmask)=extractmaskname(inpimage)
          
           # seperate text files(region files), images(with full ':'), and direct region 
           # input mask(s)
           if inpmask=='':
              raise Exception, "Input errror. The inpmask parameter is not specified."
           if type(inpmask)!=list: 
              inpmask=[inpmask]
           
           # check if inpmask contains region file or region specification
           rgfiles=[]
           imgfiles=[]
           rglist=[]
           bmasks=[]
           for masklet in inpmask:
               # is text file?
               if type(masklet)==str: # text file or image
                   if os.path.exists(masklet):
                       if (commands.getoutput('file '+masklet).count('directory')):
                          if os.path.exists(masklet+'/table.f1'):
                              #casalog.post("%s is not in a recognized format for inpmask, ignored." % masklet, 'WARN') 
                              raise Exception, "%s is not in a recognized format for inpmask" % masklet
                          else:
                          # probably image file (with no mask extension)
                              imgfiles.append(masklet)
                       elif (commands.getoutput('file '+masklet).count('text')):
                          rgfiles.append(masklet)
                       else:
                          #casalog.post("%s does not recognized format for inpmask, ignored." % masklet, 'WARN')
                          raise Exception, "%s is not in a recognized format for inpmask" % masklet
                   else:
                       if masklet.count('[') and masklet.count(']'): # rough check on region specification 
                           rglist.append(masklet) 
                       else:
                           (parentim, mask)=extractmaskname(masklet)
                           if mask!='':
                               bmasks.append(masklet)
                           else:
                               raise Exception, "%s is not an existing file/image or a region format" % masklet
      
           # expand allows only a string for inpmask
           if mode=='expand':
               if type(inpmask)==list:
                   inpmask=inpmask[0] 
           # check for overwrite condition
           if output=='':
               if overwrite: 
                   output=inpimage
               else: 
                   raise Exception, "output is not specified. If you want to overwrite inpimage, please set overwrite=True"

	   if inpimage==output:
               if overwrite:
	           tmp_outmaskimage=tmp_maskimage
               else:
                   raise Exception, "output=inpimage. If you want to overwrite inpimage, please set overwrite=True"

           outparentim=output
           outbmask=''
           if os.path.isdir(output): 
               if not overwrite:
                    raise Exception, "output=%s exists. If you want to overwrite it, please set overwrite=True" % output
           else:
               (outparentim, outbmask)=extractmaskname(output)
               if outbmask!='':
                   (parentimexist,maskexist)=checkinmask(outparentim,outbmask)    
                   if parentimexist and maskexist: 
                       if not overwrite:
                           raise Exception, "output=%s exists. If you want to overwrite it, please set overwrite=True" % output
                       else:
                        casalog.post("Will overwrite the existing internal mask, %s in %s" % (outbmask,outparentim))
                        storeinmask=True

                   #if parentimexist and not maskexist:
                   else:
                       storeinmask=True
               else:
                  outparentim=output
               
           #print "param checks before branching out for mode=========="
           #print "storeinmask = ",storeinmask
           #print "output=",output, " is exist?=",os.path.isdir(output)
           #print "outparentim=",outparentim, " is exist?=",os.path.isdir(outparentim)

    # the following code is somewhat duplicated among the modes but keep separated from each mode
    # for now.... copy now handle merge as well
    # === old copy mode === NOW combined to 'merge mode'
#        #if mode=='copy':
#           #print "Copy mode"
#           needregrid=True
#           #if outimage=='':
#               #overwrite
#           #    outimage=inpimage
#
#           if not os.path.isdir(outimage):
#               needregrid=False
#
#	   if inpmask!='':
#	   # need to extract the mask and put in tmp_maskimage
#	       pixelmask2cleanmask(imagename=inpimage, maskname=inpmask, maskimage=tmp_maskimage, usemasked=True)    
#	   else:
#	       shutil.copytree(inpimage, tmp_maskimage)
#           if needregrid:
#               casalog.post("Regridding...",'DEBUG1')
#               regridmask(tmp_maskimage,outimage,tmp_outmaskimage)
#               # regrid may produce <1.0 pixels for mask so be sure to its all in 1.0
#               #ia.open(tmp_outmaskimage) 
#               #ia.calc('iif (%s>0.0 && %s<1.0,1,%s)'%(tmp_outmaskimage,tmp_outmaskimage,tmp_outmaskimage))
#               #ia.close()
#               #print "Copying regrid output=",tmp_outmaskimage
#           else:
#               shutil.copytree(tmp_maskimage,tmp_outmaskimage)
#	   if outmask!='':
#	   #convert the image mask to T/F mask
#               if not os.path.isdir(outimage):
#                   shutil.copytree(inpimage,outimage)
#               #
#               ia.open(outimage)
#	       casalog.post("convert the output image mask to T/F mask")
#	       ia.calcmask(mask='%s<0.5' % tmp_outmaskimage,name=outmask,asdefault=True)
#               ia.done()
#           else:
#               # if regridded - tmp_outmaskimage is created by regridmask
#               # if not, tmp_outmaskimage=outimage
#               ia.open(tmp_outmaskimage)
#               ia.rename(outimage,overwrite=True)
#               ia.done()

                
    # === expand mode ===
        if mode=='expand':
            (rg,) = gentools(['rg']) 
            needtoregrid=False
	    bychanindx=False

	    try: 
		#print "expand mode main processing blocks..."
		# do not allow list in this mode (for inpimage and inpmask) - maybe this is redundant now
		if type(inpmask)==list:
		    raise TypeError, 'A list for inpmask is not allowed for mode=expand'

		# input image info, actually this will be output coordinates
		ia.open(inpimage)
		inshp = ia.shape()
		incsys = ia.coordsys()
		ia.close() 
                #print "inpimage=",inpimage," is exist?=",os.path.isdir(inpimage)
                #print " inshp for inpimage=",inshp

                # prepare working input image (tmp_maskiamge)
		if inpmask!='': # inpmask is either image mask or T/F mask now
		  # need to extract the mask and put in tmp_maskimage
                  # Note: changed usemasked=F, so that True (unmasked) part to be used. CAS- 
                  # ==> tmp_maskiamge is an input mask image
                    (parentimage,bmask)=extractmaskname(inpmask)
                    if bmask!='':
		        pixelmask2cleanmask(imagename=parentimage, maskname=bmask, maskimage=tmp_maskimage, usemasked=False)    
		        #ia.open(tmp_maskimage)
                    else:
                        #print "parentimage=",parentimage, " exist?=",os.path.isdir(parentimage)
		        # copy of inpimage in tmp_maskimage
		        ia.fromimage(outfile=tmp_maskimage, infile=parentimage)
		else:
                    raise Exception, "inpmask must be specified"
                if ia.isopen(): ia.close() 
                #setting up the output image (copy from inpimage or template)
                if not os.path.isdir(outparentim):
                    #shutil.copytree(inpimage,tmp_outmaskimage)
                    ia.fromshape(outfile=tmp_outmaskimage,shape=inshp, csys=incsys.torecord()) 
                    ia.close() 
                    needtoregrid=False
                else:
                    shutil.copytree(outparentim,tmp_outmaskimage)
                     
		# if inpfreq/outfreq are channel indices (int) then
		# regrid in x y coords only and extract specified channel mask
		# to specified output channels. (no regriding in spectral axis)
		# if inpfreqs/outfreqs are velocity or freqs, 
		# it assumes it is expressed in the range with minval~maxval
		# create subimage of the input mask with the range,
		# do regrid with the subimage to output.
	  
		# decide to regrid or not
		# 1. the case all channels are selected for input and output, simply regrid
		# 2. if inpfreqs and outfreqs are integers (= channel indices), regrid only in
		#    first and second axes (e.g. ra,dec) and no regridding along spectral axis
                # 3. if inpfreqs and outfreqs are ranges in freq or vel, make subimage and regrid
                ia.open(tmp_maskimage)
                inmaskshp = ia.shape()
                inmaskcsys = ia.coordsys()
                ia.close()
		if inmaskshp[3]!=1 and ((inpfreqs==[] and outfreqs==[]) \
                    or (inpfreqs=='' and outfreqs=='')):
		    # unless inpimage is continuum, skip chan selection part and regrid 
		    needtoregrid=True
		    # detach input(tmp) image and open output tmp image
		#    ia.open(tmp_outmaskimage)
		else: 
                #    if ia.isopen():
                #        if ia.name(strippath=True)!=tmp_maskimage:
                #            ia.close()
                #            ia.open(tmp_maskimage)
                #    else:
                #        ia.open(tmp_maskimage)

                    #if inshp[3]!=1: casalog.post("inpmask is continuum..","INFO")
                    if inmaskshp[3]==1: casalog.post("inpmask is continuum..","INFO")
		    # selection by channel indices (number) 
		    # if both inpfreqs and outfreqs are int skip regridding
		    # if outfreqs is vel or freq ranges, try regridding 
		    if inpfreqs==[[]] or inpfreqs==[]: 
                        # select all channels for input
			inpfreqs=range(inmaskshp[3])

                    # check inpfreqs and outfreqs types
                    # index based
                    selmode='bychan'
                    if type(inpfreqs)==list:
                        if type(inpfreqs[0])==int:
                            if type(outfreqs)==list and (len(outfreqs)==0 or type(outfreqs[0])==int):
                                selmode='bychan'
                            elif type(outfreqs)==str:
                                #if inpfreqs[0]==0: #contintuum -allow index-type specification
                                if len(inpfreqs)==1: #contintuum -allow index-type specification
                                    selmode='byvf'
                                else:
                                    raise TypeError, "Mixed types in infreqs and outfreqs are not allowed" 
                        else:
                            raise TypeError, "Non-integer in inpfreq is not supported" 
                    # by velocity or frequency
                    elif type(inpfreqs)==str:
                        if type(outfreqs)!=str:
                            raise TypeError, "Mixed types in infreqs and outfreqs" 
                        selmode='byvf'
                    else:
                        raise TypeError, "Wrong type for infreqs"

                    # inpfreqs and outfreqs are list of int
                    # match literally without regridding.
                    if selmode=='bychan': 
                        casalog.post("selection of input and output ranges by channel")
                        
                        if ia.isopen():
                            ia.close()
                        if outfreqs==[] or outfreqs==[[]]:
                            outchans=[]
                        else:
                            outchans=outfreqs
                        expandchanmask(tmp_maskimage,inpfreqs,tmp_outmaskimage,outchans)
                        ia.open(tmp_outmaskimage)

                    elif selmode=='byvf': # outfreqs are quantities (freq or vel)
                        casalog.post("selection of input/output ranges by frequencies/velocities")
                        
                        # do it for input mask image (not the template )
                        inpfreqlist = translatefreqrange(inpfreqs,inmaskcsys)
                        # close input image
                        if ia.isopen():
                            ia.close()
                        
                        #regrid to output image coordinates
                        if len(inpfreqlist)==1: # continuum
                            #do not regrid, use input image
                            shutil.copytree(tmp_maskimage,tmp_regridim)
                        else:
                            regridmask(tmp_maskimage,inpimage,tmp_regridim,chanrange=inpfreqlist)
                            # find edge masks (nonzero planes)
                            ia.open(tmp_regridim)
                            sh=ia.shape()
                            chanlist=range(sh[3])
                            indlo=0
                            indhi=0
                            for i in chanlist:
                                sl1=[0,0,0,i]
                                sl2=[sh[0]-1,sh[1]-1,sh[2]-1,i]
                                psum = ia.getchunk(sl1,sl2).sum()
                                pmsum = ia.getchunk(sl1,sl2,getmask=True).sum()
                                if pmsum!=0 and psum>0.0: 
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
                            if indhi < indlo:
                                raise IOError, "Incorrectly finding edges of input masks! Probably some logic error in the code!!!" 
                            else:
                                casalog.post("Determined non-zero channel range to be "+str(indlo)+"~"+str(indhi), 'DEBUG1')

                        # find channel indices for given outfreqs
                        ia.open(tmp_outmaskimage)
                        ocsys=ia.coordsys()
                        oshp=ia.shape() 
                        outfreqlist = translatefreqrange(outfreqs,ocsys)
                        rtn=ocsys.findcoordinate('spectral')
                        px=rtn['pixel'][0]
                        wc=rtn['world'][0]
                        world=ocsys.referencevalue()
                        # assume chanrange are in freqs
                        world['numeric'][wc]=qa.convert(qa.quantity(outfreqlist[0]),'Hz')['value']
                        p1 = ocsys.topixel(world)['numeric'][px]
                        world['numeric'][wc]=qa.convert(qa.quantity(outfreqlist[1]),'Hz')['value']
                        p2 = ocsys.topixel(world)['numeric'][px]
                        casalog.post("translated channel indices:"+qa.tos(outfreqlist[0])+"->p1="+str(p1)+\
                        " "+qa.tos(outfreqlist[0])+"->  p2="+str(p2))
                        if len(inpfreqs)==1:
                            inpfreqchans=inpfreqs
                        else:
                            inpfreqchans=[indlo,indhi]
                        outfreqchans=range(int(round(p1)),int(round(p2))+1)
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
                
		if needtoregrid:
		    # closing current output image
                    if ia.isopen():
		        ia.close()
		    ia.open(tmp_maskimage)
		    #os.system('cp -r %s beforeregrid.im' % tmp_maskimage)
                    if os.path.isdir(tmp_outmaskimage):
		        #print "Removing %s" % tmp_outmaskimage
		        shutil.rmtree(tmp_outmaskimage)
                    regridmask(tmp_maskimage,outparentim,tmp_outmaskimage)
		    ia.remove()
		    #print "closing after regrid"
		    ia.open(tmp_outmaskimage) # reopen output tmp image
                
                # for debugging
                #os.system('cp -r '+outparentim+" expandmode-copy-"+outparentim)
                #os.system('cp -r '+tmp_outmaskimage+" expandmode-copy-"+tmp_outmaskimage)
		if outbmask!='':
		    #convert the image mask to T/F mask
		    casalog.post("Convert the image mask to T/F mask",'INFO')
                    # regions will be masked if == 0.0 for a new outfile, if outfile exists 
                    # the pixel values inside specified mask is preserved and the rest is masked
                    if os.path.isdir(outparentim):
		      ia.calcmask(mask='%s==1.0' % tmp_outmaskimage,name=outbmask,asdefault=True)
                    else:
		      ia.calcmask(mask='%s!=0.0' % tmp_outmaskimage,name=outbmask,asdefault=True)
                if storeinmask:
                    isNewFile=False
		    if not os.path.isdir(outparentim):
                      makeEmptyimage(inpimage,outparentim)
                      isNewFile=True
                    ia.open(outparentim)
                    if isNewFile:
                      ia.set(1) 
                      # if output image exist its image pixel values will not be normalized the region
                      # outside input mask will be masked.
                   
                    ia.maskhandler('copy',[tmp_outmaskimage+':'+outbmask, outbmask])
                    ia.maskhandler('set',outbmask)
                    ia.done()
		    casalog.post("Output the mask to %s in %s" % (outbmask,outparentim) ,"INFO")
		else:
		    ow = False
                    if  inpimage==output:
		        casalog.post("Updating "+output+"with new mask","INFO")
		    else:
                        if os.path.isdir(outparentim):
		            casalog.post(outparentim+" exists, overwriting","INFO")
		            ow=True
                        else:
		            casalog.post("Output the mask to "+outparentim ,"INFO")
		    ia.rename(outparentim,ow)
		    ia.done()

            except Exception, instance:
                print "*** Error ***", instance
                if ia.isopen():
                    ia.close()
                ia.done()
                raise Exception, instance
	    finally:
		if os.path.exists(tmp_maskimage):
		    shutil.rmtree(tmp_maskimage)
		if os.path.exists(tmp_regridim):
		    shutil.rmtree(tmp_regridim)

    # === now called copy mode: copy/merge mode ===
        # copy is a just special case of merge mode
        # CHANGE:
        # all input masks should be specified in inpmask 
        # type of inpmask accepted: 1/0 mask, T/F mask, region file, and region expression in a string 
        # already stored internally in seperate lists
        #   rgfiles - region files
        #   imgfiles - 1/0 image masks
        #   rglist - region expression in strings
        #   bmasks - T/F internal masks
        #
	# avaialble parameters: inpimage (string) , inpmask (list/string), output(string)
	# input inpimage as a template or image used for defining regions when it is given in inpmask 
        # inpmask as list of all the masks to be merged (image masks, T/F internal masks, regions)

	#was: if mode=='merge':
	if mode=='copy':
	    sum_tmp_outfile='__tmp_outputmask'
            tmp_inmask='__tmp_frominmask'
            tmp_allrgmaskim='__tmp_fromAllRgn'
            tmp_rgmaskim='__tmp_fromRgn'
            usedimfiles=[]
            usedbmasks=[]
            usedrgfiles=[]
            usedrglist=[]
            #print "outparentim=",outparentim
            try:
                # check outparentim - image part of output and set as a template image
		if not (os.path.isdir(outparentim) or (outparentim==inpimage)):
                    # figure out which input mask to be used as template
                    # if inpimage is defined use the first one else try the first one
                    # inpmask
                    #if output=='':
                    #    if type(inpimage)==list:
                    #         raise Exception, "inputimage must be a string"
                    #    elif type(inpimage)==str:
                    #         outimage=inpimage
                    #    casalog.post("No outimage is specified. Will overwrite input image: "+outimage,'INFO')

                    #if type(inpimage)==list and len(inpimage)!=0:
                    #    tmp_template=inpimage[0]
                    #elif inpimage!='' and inpimage!=[]:
                    #    tmp_template=inpimage # string
                    #tmp_template=inpimage # string
                    #else:
                    #     if type(inpmask)==list and len(inpmask)!=0:
                    #         fsep=inpmask[0].rfind(':')
                    #         if fsep ==-1:
                    #            raise IOError, "Cannot resolve inpmask name, check the format"
                    #        else:
                    #            tmp_template=inpmask[0][:inpmask[0].rfind(':')]
                    #    elif inpmask!='' and inpmask!=[]:
                    #        # this is equivalent to 'copy' the inpmask
                    #        tmp_template=inpmask #string
                    
                    # create an empty image with the coords from inpimage
                    makeEmptyimage(inpimage,sum_tmp_outfile)    
                    #print "making an empty image from inpimage to sum_tmp_outfile"
		else:
                    #use output image - does not do zeroeing out, so output image is only modified     
		    shutil.copytree(outparentim,sum_tmp_outfile)
                    # temporary clear out the internal masks from the working image
                    ia.open(sum_tmp_outfile)
                    origmasks = ia.maskhandler('get') 
                    ia.maskhandler('delete',origmasks)
                    ia.close()
                     
                #if type(inpimage)==str:
                #    inpimage=[inpimage]
                #if type(inpmask)==str:
                #    inpmask=[inpmask]

		if len(imgfiles)>0:
		    # summing all the images
                    casalog.post('Summing all mask images in inpmask and  normalized to 1 for mask','INFO')
		    for img in imgfiles:
                        tmpregrid='__tmp_regrid.'+img
                        # regrid to output image coords
			regridmask(img,sum_tmp_outfile,tmpregrid)
			addimagemask(sum_tmp_outfile,tmpregrid)
                        usedimfiles.append(img)
                        shutil.rmtree(tmpregrid)
		    # get boolean masks
		    #  will work in image(1/0) masks
                
		if len(bmasks)>0:
                    casalog.post('Summing all T/F mask in inpmask and normalized to 1 for mask','INFO')
		    for msk in bmasks:
                        (imname,mskname) = extractmaskname(msk)
			#if msk.find(':')<0:
			#    # assume default mask
			#    msk=msk+':mask0'
                        #imname=msk[:msk.rfind(':')]
			ia.open(imname)
			inmasks=ia.maskhandler('get')
                        ia.close()
			if not inmasks.count(mskname):
			    raise TypeError, mskname+" does not exist in "+imname+" -available masks:"+str(inmasks)
			# move T/F mask to image mask
                        # changed to usemasked=False as of CAS-5443  

			pixelmask2cleanmask(imname, mskname, tmp_inmask, False)    
			regridmask(tmp_inmask,sum_tmp_outfile,'__tmp_fromTFmask')
			addimagemask(sum_tmp_outfile,'__tmp_fromTFmask')
                        usedbmasks.append(msk)
                        shutil.rmtree('__tmp_fromTFmask') 
                        shutil.rmtree(tmp_inmask) 
                        # if overwriting to inpimage and if not writing to in-mask, delete the boolean mask
                        if outparentim==inpimage and inpimage==imname:
                            if outbmask=="":
                                ia.open(imname)
                                ia.maskhandler('delete',[mskname])
                                ia.close()
                        ia.open(imname)
                        ia.close()
                      
                if len(rgfiles)>0 or len(rglist)>0:
                    # create an empty image with input image coords.
                    #print "Using %s as a template for regions" % inpimage 
                    ia.open(inpimage)
                    tshp=ia.shape()
                    tcsys=ia.coordsys() 
                    ia.fromshape(tmp_allrgmaskim,shape=tshp, csys=tcsys.torecord(),overwrite=True)
                    ia.done()    
                    if os.path.isdir(tmp_rgmaskim):
                       shutil.rmtree(tmp_rgmaskim)
                    shutil.copytree(tmp_allrgmaskim,tmp_rgmaskim)

		    if len(rgfiles)>0:
                        #print "Has rgfiles..."
			nrgn=0
			for rgn in rgfiles:
                            firstline=True
                            subnrgn=0
                            with open(rgn) as f:
				for line in f:
                                    if firstline:
				        if line.count('#CRTF')==0:
				            raise Exception, "Input text file does not seems to be in a correct format \
                                                              (must contains #CRTF)"
					firstline=False
                                    else:     
					try:
					    if line.count('^#'):
						pass
					    else:
					       if len(line)!=0:
						  # reset temp mask image
						  ia.open(tmp_rgmaskim)
						  ia.set(pixels=0.0)
						  ia.close()
						  #print "tshp=",tshp
						  #print "tcsys.torecord=",tcsys.torecord()
						  inrgn=rg.fromtext(line, tshp, tcsys.torecord())
						  #print "inrgn=",inrgn
						  im.regiontoimagemask(tmp_rgmaskim,region=inrgn)
						  addimagemask(tmp_allrgmaskim,tmp_rgmaskim)
						  #shutil.rmtree(tmp_rgmaskim)
                                                  subnrgn +=1
						  nrgn +=1
					except:
					    break
                            if subnrgn>0:
                                usedrgfiles.append(rgn)    
			casalog.post("Converted %s regions from %s region files to image mask" % (nrgn,len(rgfiles)),"INFO")
						
		    if len(rglist)>0:
                        #print "Has rglist..."
                        nrgn=0
			for rgn in rglist:
                            # reset temp mask image
                            ia.open(tmp_rgmaskim)
                            ia.set(pixels=0.0)
                            ia.close()
			    inrgn=rg.fromtext(rgn, tshp, tcsys.torecord())
			    im.regiontoimagemask(tmp_rgmaskim,region=inrgn)
			    addimagemask(tmp_allrgmaskim,tmp_rgmaskim)
                            #shutil.rmtree(tmp_rgmaskim)
                            usedrglist.append(rgn)
                            nrgn+=1
			casalog.post("Converted %s regions to image mask" % (nrgn),"INFO")
                
                        
                    # regrid if necessary
                    regridded_mask='__tmp_regrid_allrgnmask'
                    regridmask(tmp_allrgmaskim, sum_tmp_outfile,regridded_mask)
                    addimagemask(sum_tmp_outfile,regridded_mask)
                    #shutil.rmtree('__tmp_regridded_allrgnmask')
		    casalog.post("Added mask based on regions to output mask","INFO")
                    #cleanup
                    for tmpfile in [tmp_allrgmaskim,tmp_rgmaskim,regridded_mask]:
                        if os.path.isdir(tmpfile):
                            shutil.rmtree(tmpfile)

                                        
		if outbmask!='':
                    casalog.post('Putting mask in T/F','INFO')
		    ia.open(sum_tmp_outfile)
		    ia.calcmask(mask='%s==1.0' % sum_tmp_outfile,name=outbmask,asdefault=True)
                    # mask only pixel == 0.0 (for a new outfile), mask region !=1.0 and preserve
                    # the pixel values if outfile exists
                    #if os.path.isdir(outparentim):
		    #  ia.calcmask(mask='%s==1.0' % sum_tmp_outfile,name=outbmask,asdefault=True)
                    #else:
		    #  ia.calcmask(mask='%s!=0.0' % sum_tmp_outfile,name=outbmask,asdefault=True)
		    ia.done()
	        # if outfile exists initially outfile is copied to sum_tmp_outfile
		# if outfile does not exist initially sum_tmp_outfile is a copy of inpimage
		# so rename it with overwrite=T all the cases
                #print "open sum_tmp_outfile=",sum_tmp_outfile
                if storeinmask:
                    # by a request in CAS-6912 no setting of 1 for copying mask to the 'in-mask'
                    # (i.e. copy the values of inpimage as well for this mode)
                    #isNewfile = False
		    if not os.path.isdir(outparentim):
                      #makeEmptyimage(inpimage,outparentim)
                      #isNewfile=True
                      shutil.copytree(inpimage,outparentim)
                    ia.open(outparentim)
                    #if isNewfile: 
                    #  ia.set(1)
                    # using maskexist at the initial test before branching out for each mode
                    if maskexist and overwrite: 
                      ia.maskhandler('delete',outbmask)    
                    ia.maskhandler('copy',[sum_tmp_outfile+':'+outbmask, outbmask])    
                    ia.maskhandler('set',outbmask)
                    ia.done()
		    outputmsg="to create an output mask: %s in %s" % (outbmask,outparentim)
                else:
		    ia.open(sum_tmp_outfile) 
		    ia.rename(outparentim,overwrite=True) 
		    ia.done()
		    outputmsg="to create an output mask: %s " % outparentim

                casalog.post("Merged masks from:","INFO")
                if len(usedimfiles)>0:
                    casalog.post("mask image(s): "+str(usedimfiles),"INFO")
                if len(usedbmasks)>0:
                    casalog.post("internal mask(s): "+str(usedbmasks),"INFO")
                if len(usedrgfiles)>0:
                    casalog.post("region txt file(s): "+str(usedrgfiles),"INFO")
                if len(usedrglist)>0:
                    casalog.post("region(s) from direct input: "+str(usedrglist),"INFO")
                casalog.post(outputmsg,"INFO")
                

            except Exception, instance:
                print "*** Error ***", instance
                raise Exception, instance
	    finally:
		if os.path.exists(sum_tmp_outfile):
		    shutil.rmtree(sum_tmp_outfile)
		if os.path.exists(tmp_inmask):
		    shutil.rmtree(tmp_inmask)
		if os.path.exists(tmp_allrgmaskim):
		    shutil.rmtree(tmp_allrgmaskim)
		if os.path.exists(tmp_rgmaskim):
		    shutil.rmtree(tmp_rgmaskim)

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
    #print "inputmask=",inputmask," template=",template," outputmask=",outputmask
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
    # check if all 4 axes exist
    errmsg = ""
    for axname in reforder:
      if axisorder.count(axname) == 0:
        errmsg += axname+" "
    if len(errmsg) != 0:
      errmsg = "There is no "+errmsg+" axes inpimage. ia.adddegaxis or importfits with defaultaxes=True can solve this problem"    
      raise Exception, errmsg

    tmp_axes=[]
    for axi in axes:
        tmp_axes.append(axisorder.index(reforder[axi]))        
    axes=tmp_axes
    if type(chanrange)==list and len(chanrange)==2:
        incsys=ia.coordsys()
        spaxis=incsys.findcoordinate('spectral')['world']
        # create subimage based on the inpfreqs range
        inblc=chanrange[0]
        intrc=chanrange[1]
        casalog.post("Regridmask: spaxis=%s, inblc=%s, intrc=%s" % (spaxis,inblc,intrc), 'DEBUG1')
        rgn = rg.wbox(blc=inblc,trc=intrc,pixelaxes=spaxis.tolist(),csys=incsys.torecord())
    else:
        rgn={}     
    # for continuum case
    if oshp[tmp_axes[0]]==1:
       axes=[0,1]
    ir=ia.regrid(outfile=outputmask,shape=oshp,csys=ocsys.torecord(),axes=axes,region=rgn,method=method)       
    ia.done()
    # to ensure to create 1/0 mask image
    #ir.calc('iif (%s>0.0 && %s<1.0,1,%s)'%(outputmask,outputmask,outputmask))
    # treat everything not = 0.0 to be mask
    ir.calc('iif (abs(%s)>0.0,1,%s)'%(outputmask,outputmask),False)
    ir.done()

def addimagemask(sumimage, imagetoadd, threshold=0.0):
    """
    add image masks (assumed the images are already in the same coordinates)
    """
    (ia,) = gentools(['ia']) 
    #print "addimagemask: sumimage=",sumimage," imagetoadd=",imagetoadd
    ia.open(sumimage)
    ia.calc('iif ('+imagetoadd+'>'+str(threshold)+',('+sumimage+'+'+imagetoadd+')/('+sumimage+'+'+imagetoadd+'),'+sumimage+')',False)
    # actually should be AND?
    #ia.calc('iif ('+imagetoadd+'>'+str(threshold)+','+sumimage+'*'+imagetoadd+','+sumimage+')')
    #ia.calc('iif ('+imagetoadd+'>'+str(threshold)+',('+sumimage+'*'+imagetoadd+')/('+sumimage+'*'+imagetoadd+'),'+sumimage+')')
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
    #print "refchanst=",refchanst," refchanen=",refchanen," inshp=",inshp," inchans=",inchans
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
    outshp=ia.shape()
    if outchans==[]:
        #select all channels
        outchans=range(outshp[3])
    usechanims={}  # list of input mask to be use for each outpfreq
    for i in outchans:
        nearestch = findnearest(inchans,i)
        usechanims[i]=nearestch
    #print "usechanims=",usechanims
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

def checkinput(inpname):
    """
    do existance check on image and internal mask 
    """
    (parentimage,tfmaskname)=extractmaskname(inpname)
    (parentimexist,tfmaskexist)=checkinmask(parentimage,tfmaskname)
    if parentimexist:
        if tfmaskname=='':
            return True # only the image
        else:
            if not tfmaskexist: 
                ia.open(parentimage)
                inmasklist=ia.maskhandler('get')
                raise Exception, "Cannot find the internal mask, %s. Candidate mask(s) are %s" % (tfmaskname, str(inmasklist))
            else:
                return True # image mask and internal mask
    else:
        raise Exception, "Cannot find the image=%s" % parentimage 
   

def checkinmask(parentimage,tfmaskname):
    """
    check existance of the internal mask
    """
    if os.path.isdir(parentimage):
        if tfmaskname!='':
            ia.open(parentimage)
            inmasks=ia.maskhandler('get')
            ia.close()
            if not any(tfmaskname in msk for msk in inmasks):
               return (True, False)
            else:
               return (True, True) # image mask and internal mask
        else:
            return (True, False)
    else:
       return (False,False) 

def extractmaskname(maskname):
    """
    split out imagename and maskname from a maskname string
    returns (parentimage, internalmask)
    """
    # the image file name may contains ':' some cases
    # take last one in split list as an internal mask name

    indx = maskname.find(':') 
    for i in range(len(maskname)):
        if indx>-1:
            indx += maskname[indx+1:].find(':') 
            indx +=1
        else:
            break
    if indx != -1: 
        parentimage=maskname[:indx]
        maskn=maskname[indx+1:]
        return (parentimage,maskn) 
    else:
        parentimage=maskname
        return (parentimage, '')

def makeEmptyimage(template,outimage):
    """
    make an empty image with the coords
    from template
    """
    (ia,) = gentools(['ia'])
    ia.open(template)
    inshp=ia.shape()
    incsys=ia.coordsys()
    ia.fromshape(outimage,shape=inshp,csys=incsys.torecord())
    ia.done()
