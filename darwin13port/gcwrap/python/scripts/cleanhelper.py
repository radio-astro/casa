from  casac import *
import os
import commands
import math
import pdb
import numpy
import shutil
import string
from numpy import unique
from odict import odict

###some helper tools
ms = casac.ms()
tb = casac.table()
qa = casac.quanta()
me = casac.measures()
rg = casac.regionmanager()
ia = casac.image()

class cleanhelper:
    def __init__(self, imtool='', vis='', usescratch=False, casalog=None):
        """
        Contruct the cleanhelper object with an imager tool
        like so:
        a=cleanhelper(im, vis)
        """
        ###fix for assumption that if it is a list of 1 it is sngle ms mode
        if((type(vis)==list) and (len(vis)==1)):
            vis=vis[0]
        ####
        if((type(imtool) != str) and (len(vis) !=0)):
            # for multi-mses input (not fully implemented yet)
            if(type(vis)!=list):
                vis=[vis]
                self.sortedvisindx=[0]
            self.initmultims(imtool, vis, usescratch)
        #    else:
        #        self.initsinglems(imtool, vis, usescratch)
        #self.maskimages={}
        self.maskimages=odict()
        self.finalimages={}
        self.usescratch=usescratch
        self.dataspecframe='LSRK'
        self.usespecframe='' 
        self.inframe=False
        # to use phasecenter parameter in initChannelizaiton stage
        # this is a temporary fix need. 
        self.srcdir=''
        # for multims handling
        self.sortedvislist=[]
        if not casalog:  # Not good!
            casalog = casac.logsink()
            #casalog.setglobal(True)
        self._casalog = casalog
        
    @staticmethod
    def getsubtable(visname='', subtab='SPECTRAL_WINDOW'):
        """needed because mms has it somewhere else
        """
        tb.open(visname)
        spectable=string.split(tb.getkeyword(subtab))
        if(len(spectable) ==2):
            spectable=spectable[1]
        else:
            spectable=visname+"/"+subtab
        return spectable

    def initsinglems(self, imtool, vis, usescratch):
        """
        initialization for single ms case
        """
        self.im=imtool
        # modified for self.vis to be a list for handling multims
        #self.vis=vis
        self.vis=[vis]
        self.sortedvisindx=[0]
        
        if ((type(vis)==str) & (os.path.exists(vis))):
            self.im.open(vis, usescratch=usescratch)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
        self.phasecenter=''
        self.spwindex=-1
        self.fieldindex=-1
        self.outputmask=''
        self.csys={}

    def initmultims(self, imtool, vislist, usescratch):
        """
        initialization for multi-mses 
        """
        self.im=imtool
        self.vis=vislist
        if type(vislist)==list:
            #        self.im.selectvis(vis)
            if ((type(self.vis[0])==str) & (os.path.exists(self.vis[0]))):
                pass
            else:
                raise Exception, 'Visibility data set not found - please verify the name'
        self.phasecenter=''
        self.spwindex=-1
        self.fieldindex=-1
        self.outputmask=''
        self.csys={}

    def sortvislist(self,spw,mode,width):
        """
        sorting user input vis if it is multiple MSes based on
        frequencies (spw). So that in sebsequent processing such
        as setChannelization() correctly works.
        Returns sorted vis. name list
        """
        import operator
        reverse=False
        if mode=='velocity':
          if qa.quantity(width)['value']>0:
            reverse=True
        elif mode=='frequency':
          if qa.quantity(width)['value']<0:
            reverse=True 
        minmaxfs = []
        # get selection
        if len(self.vis) > 1:
          for i in range(len(self.vis)):
            visname = self.vis[i]
            if type(spw)==list and len(spw) > 1:
              inspw=spw[i]
            else:
              inspw=spw
            if len(inspw)==0:
              # empty string = select all (='*', for msselectindex)
              inspw='*'
            mssel=ms.msseltoindex(vis=visname,spw=inspw)
            spectable=self.getsubtable(visname, "SPECTRAL_WINDOW")
            tb.open(spectable)
            chanfreqs=tb.getvarcol('CHAN_FREQ')
            kys = chanfreqs.keys()
            selspws=mssel['spw']
            # find extreme freq in each selected spw
            minmax0=0.
            firstone=True
            minmaxallspw=0.
            for chansel in mssel['channel']:
              if reverse:
                minmaxf = max(chanfreqs[kys[chansel[0]]][chansel[1]:chansel[2]+1])
              else:
                minmaxf = min(chanfreqs[kys[chansel[0]]][chansel[1]:chansel[2]+1])
              if firstone:
                minmaxf0=minmaxf
                firstone=False
              if reverse:
                minmaxallspw=max(minmaxf,minmaxf0) 
              else:
                minmaxallspw=min(minmaxf,minmaxf0) 
            minmaxfs.append(minmaxallspw)
          self.sortedvisindx = [x for x, y in sorted(enumerate(minmaxfs),
                                key=operator.itemgetter(1),reverse=reverse)] 
          self.sortedvislist = [self.vis[k] for k in self.sortedvisindx]
        else:
          self.sortedvisindx=[0]
          self.sortedvislist=self.vis
        #print "sortedvislist=",self.sortedvislist
        #print "sortedvisindx=",self.sortedvisindx
        return


    def defineimages(self, imsize, cell, stokes, mode, spw, nchan, start,
                     width, restfreq, field, phasecenter, facets=1, outframe='',
                     veltype='radio'):
        """
        Define image parameters -calls im.defineimage.
        for processing a single field 
        (also called by definemultiimages for multifield) 
        """
        if((type(cell)==list) and (len(cell)==1)):
            cell.append(cell[0])
        elif ((type(cell)==str) or (type(cell)==int) or (type(cell)==float)):
            cell=[cell, cell]
        elif (type(cell) != list):
            raise TypeError, "parameter cell %s is not understood" % str(cell)
        cellx=qa.quantity(cell[0], 'arcsec')
        celly=qa.quantity(cell[1], 'arcsec')
        if(cellx['unit']==''):
            #string with no units given
            cellx['unit']='arcsec'
        if(celly['unit']==''):
            #string with no units given
            celly['unit']='arcsec'
        if((type(imsize)==list) and (len(imsize)==1)):
            imsize.append(imsize[0])
        elif(type(imsize)==int):
            imsize=[imsize, imsize]
        elif(type(imsize) != list):
            raise TypeError, "parameter imsize %s is not understood" % str(imsize)
            
	elstart=start
        if(mode=='frequency'):
        ##check that start and step have units
            if(qa.quantity(start)['unit'].find('Hz') < 0):
                raise TypeError, "start parameter %s is not a valid frequency quantity " % str(start)
            ###make sure we respect outframe
            if(self.usespecframe != ''):
                elstart=me.frequency(self.usespecframe, start)
            if(qa.quantity(width)['unit'].find('Hz') < 0):
                raise TypeError, "width parameter %s is not a valid frequency quantity " % str(width)	
        elif(mode=='velocity'): 
        ##check that start and step have units
            if(qa.quantity(start)['unit'].find('m/s') < 0):
                raise TypeError, "start parameter %s is not a valid velocity quantity " % str(start)
            ###make sure we respect outframe
            if(self.usespecframe != ''):
                elstart=me.radialvelocity(self.usespecframe, start)
            if(qa.quantity(width)['unit'].find('m/s') < 0):
                raise TypeError, "width parameter %s is not a valid velocity quantity " % str(width)	
        else:
            if((type(width) != int) or 
               (type(start) != int)):
                raise TypeError, "start (%s), width (%s) have to be integers with mode %s" % (str(start),str(width),mode)

        # changes related multims handling added below (TT)
        # multi-mses are sorted internally (stored in self.sortedvislist and
        # indices in self.sortedvisindx) in frequency-wise so that first vis
        # contains lowest/highest frequency. Note: self.vis is in original user input order.
        #####understand phasecenter
        if(type(phasecenter)==str):
            ### blank means take field[0]
            if (phasecenter==''):
                fieldoo=field
                if(fieldoo==''):
                    fieldoo='0'
                #phasecenter=int(ms.msseltoindex(self.vis,field=fieldoo)['field'][0])
                phasecenter=int(ms.msseltoindex(self.vis[self.sortedvisindx[0]],field=fieldoo)['field'][0])
            else:
                tmppc=phasecenter
                try:
                    #if(len(ms.msseltoindex(self.vis, field=phasecenter)['field']) > 0):
                    #    tmppc = int(ms.msseltoindex(self.vis,
                    #                                field=phasecenter)['field'][0])
                    # to handle multims set to the first ms that matches
                    for i in self.sortedvisindx:
                        try:
                            if(len(ms.msseltoindex(self.vis[i], field=phasecenter)['field']) > 0):
                                tmppc = int(ms.msseltoindex(self.vis[i],
                                                    field=phasecenter)['field'][0])
                                #print "tmppc,i=", tmppc, i
                        except Exception, instance:
                             pass

                    ##succesful must be string like '0' or 'NGC*'
                except Exception, instance:
                    ##failed must be a string 'J2000 18h00m00 10d00m00'
                    tmppc = phasecenter
                phasecenter = tmppc
        self.phasecenter = phasecenter
        #print 'cell', cellx, celly, restfreq
        ####understand spw
        if spw in (-1, '-1', '*', '', ' '):
            spwindex = -1
        else:
            # old, single ms case
            #spwindex=ms.msseltoindex(self.vis, spw=spw)['spw'].tolist()
            #if(len(spwindex) == 0):
            #    spwindex = -1
        #self.spwindex = spwindex

        # modified for multims hanlding
            # multims case
            if len(self.vis)>1:
                self.spwindex=[]
                # will set spwindex in datsel
                spwindex=-1
                for i in self.sortedvisindx:
                   if type(spw)==list:
                      inspw=spw[i]
                   else:
                      inspw=spw
                   if len(inspw)==0: 
                      inspw='*'
                   self.spwindex.append(ms.msseltoindex(self.vis[i],spw=inspw)['spw'].tolist())
            # single ms
            else:
                spwindex=ms.msseltoindex(self.vis[0], spw=spw)['spw'].tolist()
                if(len(spwindex) == 0):
                    spwindex = -1
                self.spwindex = spwindex

        ##end spwindex

        if self.usespecframe=='': 
            useframe=self.dataspecframe
        else:
            useframe=self.usespecframe

        self.im.defineimage(nx=imsize[0],      ny=imsize[1],
                            cellx=cellx,       celly=celly,
                            mode=mode,         nchan=nchan,
                            start=elstart,       step=width,
                            spw=spwindex,      stokes=stokes,
                            restfreq=restfreq, outframe=useframe,
                            veltype=veltype, phasecenter=phasecenter,
                            facets=facets)

    def definemultiimages(self, rootname, imsizes, cell, stokes, mode, spw,
                          nchan, start, width, restfreq, field, phasecenters,
                          names=[], facets=1, outframe='', veltype='radio',
                          makepbim=False, checkpsf=False):
        """
        Define image parameters - multiple field version
        This fucntion set "private" variables (imagelist and imageids),
        and then calls defineimages for ecah field. 
        """
        #will do loop in reverse assuming first image is main field
        if not hasattr(imsizes, '__len__'):
            imsizes = [imsizes]
        self.nimages=len(imsizes)
        #if((len(imsizes)<=2) and ((type(imsizes[0])==int) or
        #                          (type(imsizes[0])==long))):
        if((len(imsizes)<=2) and (numpy.issubdtype(type(imsizes[0]), int))):
            self.nimages=1
            if(len(imsizes)==2):
                imsizes=[(imsizes[0], imsizes[1])]
            else:
                imsizes=[(imsizes[0], imsizes[0])]
        self._casalog.post('Number of images: ' + str(self.nimages), 'DEBUG1')

        #imagelist is to have the list of image model names
        self.imagelist={}
        #imageids is to tag image to mask in aipsbox style file 
        self.imageids={}
        if(type(phasecenters) == str):
            phasecenters=[phasecenters]
        if(type(phasecenters) == int or type(phasecenters) == float ):
            phasecenters=[phasecenters]
        self._casalog.post('Number of phase centers: ' + str(len(phasecenters)),
                           'DEBUG1')

        if((self.nimages==1) and (type(names)==str)):
            names=[names]
        if((len(phasecenters)) != (len(imsizes))):
            errmsg = "Mismatch between the number of phasecenters (%d), image sizes (%d) , and images (%d)" % (len(phasecenters), len(imsizes), self.nimages)
            self._casalog.post(errmsg, 'SEVERE')
            raise ValueError, errmsg
        self.skipclean=False
        lerange=range(self.nimages)
        lerange.reverse()
        for n in lerange:
            self.defineimages(list(imsizes[n]), cell, stokes, mode, spw, nchan,
                              start, width, restfreq, field, phasecenters[n],
                              facets,outframe,veltype)
            if(len(names)==self.nimages):
                self.imageids[n]=names[n]
                if(rootname != ''):
                    self.imagelist[n]=rootname+'_'+names[n]
                else:
                    self.imagelist[n]=names[n]
            else:
                self.imagelist[n]=rootname+'_'+str(n)
            ###make the image only if it does not exits
            ###otherwise i guess you want to continue a clean
            if(not os.path.exists(self.imagelist[n])):
                self.im.make(self.imagelist[n])
	    #if(makepbim and n==0):
	    if(makepbim):
    		##make .flux image 
                # for now just make for a main field 
                ###need to get the pointing so select the fields
                # single ms
#                if len(self.vis)==1:
#                  self.im.selectvis(field=field,spw=spw)
#                # multi-ms
#                else: 
#                  if len(self.vis) > 1:
#                  # multi-mses case: use first vis that has the specified field
#                  # (use unsorted vis list)
#                  nvis=len(self.vis)
#                  for i in range(nvis):
#                    #if type(field)!=list:
#                    #  field=[field]
#                    try:
#                      selparam=self._selectlistinputs(nvis,i,self.paramlist)
#                      self.im.selectvis(vis=self.vis[i],field=selparam['field'],spw=selparam['spw'])
#                    except:
#                      pass

                # set to default minpb(=0.1), should use input minpb?
                self.im.setmfcontrol()
                self.im.setvp(dovp=True)
                self.im.makeimage(type='pb', image=self.imagelist[n]+'.flux',
                                  compleximage="", verbose=False)
		self.im.setvp(dovp=False, verbose=False)

                
    def checkpsf(self,chan):
        """
        a check to make sure selected channel plane is not entirely flagged
        (for chinter=T interactive clean)
        """
        #lerange=range(self.nimages)
        #lerange.reverse()
        #for n in lerange:
        #    self.getchanimage(self.finalimages[n]+'.psf',self.imagelist[n]+'.test.psf',chan)
        #    ia.open(self.imagelist[n]+'.test.psf')
        #    imdata=ia.getchunk()
        #    print "imagelist[", n, "]=", self.imagelist[n], " imdata.sum()=",imdata.sum()
            #if n==0 and imdata.sum()==0.0:
            #    self.skipclean=True 
        #    if self.skipclean:
        #        pass
        #    elif imdata.sum()==0.0:
        #        self.skipclean=True
        
        # need to check only for main field
        self.getchanimage(self.finalimages[0]+'.psf',self.imagelist[0]+'.test.psf',chan)
        ia.open(self.imagelist[0]+'.test.psf')
        imdata=ia.getchunk()
        if imdata.sum()==0.0:
            self.skipclean=True


    def makeEmptyimages(self):
        """
        Create empty images (0.0 pixel values) for 
        image, residual, psf
        must run after definemultiimages()
        and it is assumed that definemultiimages creates 
        empty images (self.imagelist). 
        """ 
        lerange=range(self.nimages)
        for n in lerange:
            os.system('cp -r '+self.imagelist[n]+' '+self.imagelist[n]+'.image')
            os.system('cp -r '+self.imagelist[n]+' '+self.imagelist[n]+'.residual')
            os.system('cp -r '+self.imagelist[n]+' '+self.imagelist[n]+'.psf')
            os.system('cp -r '+self.imagelist[n]+' '+self.imagelist[n]+'.model')
            os.system('cp -r '+self.imagelist[n]+' '+self.imagelist[n]+'.mask')

    
    def makemultifieldmask(self, maskobject=''):
        """
        This function assumes that the function definemultiimages has been run and thus
        self.imagelist is defined
        if single image use the single image version
        (this is not up to date for current mask handling but used in task_widefield,
         to be removed after task_widefield is gone)
        """
        if((len(self.maskimages)==(len(self.imagelist)))):
            if(not self.maskimages.has_key(self.imagelist[0])):
                self.maskimages={}
        else:
            self.maskimages={}
        masktext=[]
        if (not hasattr(maskobject, '__len__')) \
           or (len(maskobject) == 0) or (maskobject == ['']):
            return
        if(type(maskobject)==str):
            maskobject=[maskobject]
        if(type(maskobject) != list):
            ##don't know what to do with this
            raise TypeError, 'Dont know how to deal with mask object'
        n=0
        for masklets in maskobject:
            if(type(masklets)==str):
                    if(os.path.exists(masklets)):
                        if(commands.getoutput('file '+masklets).count('directory')):
                            self.maskimages[self.imagelist[n]]=masklets
                            n=n+1
                        elif(commands.getoutput('file '+masklets).count('text')):
                            masktext.append(masklets)
                        else:
                            raise TypeError, 'Can only read text mask files or mask images'
                    else:
                       raise TypeError, masklets+' seems to be non-existant' 
        if(len(masktext) > 0):
            circles, boxes=self.readmultifieldboxfile(masktext)
            if(len(self.maskimages)==0):
                for k in range(len(self.imageids)):
                    if(not self.maskimages.has_key(self.imagelist[k])):
                        self.maskimages[self.imagelist[k]]=self.imagelist[k]+'.mask'
            for k in range(len(self.imageids)):
                ###initialize mask if its not there yet
                if(not (os.path.exists(self.maskimages[self.imagelist[k]]))):
                    ia.fromimage(outfile=self.maskimages[self.imagelist[k]],
                                 infile=self.imagelist[k])
                    ia.open(self.maskimages[self.imagelist[k]])
                    ia.set(pixels=0.0)
                    ia.done(verbose=False)
                if(circles.has_key(self.imageids[k]) and boxes.has_key(self.imageids[k])):
                    self.im.regiontoimagemask(mask=self.maskimages[self.imagelist[k]],
                                              boxes=boxes[self.imageids[k]],
                                              circles=circles[self.imageids[k]])
                elif(circles.has_key(self.imageids[k])):
                    self.im.regiontoimagemask(mask=self.maskimages[self.imagelist[k]],
                                              circles=circles[self.imageids[k]])
                elif(boxes.has_key(self.imageids[k])):
                    self.im.regiontoimagemask(mask=self.maskimages[self.imagelist[k]],
                                              boxes=boxes[self.imageids[k]])
                else:
                    ###need to make masks that select that whole image
                    ia.open(self.maskimages[self.imagelist[k]])
                    ia.set(pixels=1.0)
                    ia.done(verbose=False)


    def makemultifieldmask2(self, maskobject='',slice=-1, newformat=True, interactive=False):

        """
        Create mask images for multiple fields (flanking fields) 
        This new makemultifieldmask to accomodate different kinds of masks supported
        in clean with flanking fields.

        Keyword arguments:
        maskobject -- input mask, a list (of string or of list(s)) or a string 
        slice      -- channel slice (to handle chaniter mode): default = -1 (all)
        newformat  -- if mask is read from new format text file: default = True 

        Prerequiste: definemultiimages has already ran so that imagelist is defined 
       
        Notes: 
        * It makes empty mask images at begenning, calls makemaskimage, and if no
          mask to be specified, the corresponding empty mask image is removed
  
        * When clean is executed in commnad line style (e.g. clean(vis='..', ..))
          it is possible to have mask parameter consists of a mix of strings and int lists
          (i.e. mask=[['newreg.txt',[55,55,65,70]],['outliermask.rgn']]), and this function
          should be able to parse these properly. - although this won't work for the task execution
          by go() and tends to messed up inp() after such execution 
 
        * Currently it is made to handle old outlier text file format and boxfile-style
          mask box specification for backward compartibility. But it is planned to
          be removed for 3.4.

        * This is a refactored version of the previous makemultifieldmask2
          and  calls makemaskimage() for each field. 
          this was called makemultifieldmask3 in CASA 3.3 release but now renamed 
          makemultifieldmask2 as the previous makemultifieldmask2 was removed.
        """
        #print "Inside makemultifieldmask2"
        if((len(self.maskimages)==(len(self.imagelist)))):
            if(not self.maskimages.has_key(self.imagelist[0])):
                self.maskimages=odict()
        else:
            self.maskimages=odict()
        # clean up temp mask image 
        if os.path.exists('__tmp_mask'):
           shutil.rmtree('__tmp_mask')
      
        if (not hasattr(maskobject, '__len__')) \
           or (len(maskobject) == 0) or (maskobject == ['']):
            return
        # for empty maskobject list 
        if all([msk==[''] or msk==[] for msk in maskobject]):
            return
        # determine number of input elements
        if (type(maskobject)==str):
            maskobject=[maskobject]
        if(type(maskobject) != list):
            ##don't know what to do with this
            raise TypeError, 'Dont know how to deal with maskobject with type: %s' % type(maskobject)
        #if(type(maskobject[0])==int or type(maskobject[0])==float):
        if(numpy.issubdtype(type(maskobject[0]),int) or numpy.issubdtype(type(maskobject[0]),float)):
            maskobject=[maskobject] 
        if(type(maskobject[0][0])==list):
            #if(type(maskobject[0][0][0])!=int and type(maskobject[0][0][0])!=float):        
            if not (numpy.issubdtype(type(maskobject[0][0][0]),int) or \
                    numpy.issubdtype(type(maskobject[0][0][0]),float)):        
                maskobject=maskobject[0]
                    
        # define maskimages
        if(len(self.maskimages)==0):
            for k in range(len(self.imageids)):
                if(not self.maskimages.has_key(self.imagelist[k])):
                    self.maskimages[self.imagelist[k]]=self.imagelist[k]+'.mask'
        # initialize maskimages - create empty maskimages
        # --- use outframe or dataframe for mask creation
        # * It appears somewhat duplicating with makemaskimage 
        #   but it is necessary to create a maskimage for
        #   each field at this point...      
        if self.usespecframe=='': 
            maskframe=self.dataspecframe
        else:
            maskframe=self.usespecframe
        #print "Frame : ", maskframe
        #print "dataframe : ", self.dataspecframe , "   useframe : ", self.usespecframe
        for k in range(len(self.imagelist)):
            if(not (os.path.exists(self.maskimages[self.imagelist[k]]))):
                ia.fromimage(outfile=self.maskimages[self.imagelist[k]],
                        infile=self.imagelist[k])
                ia.open(self.maskimages[self.imagelist[k]])
                ia.set(pixels=0.0)
                #mcsys=ia.coordsys().torecord()
                #if mcsys['spectral2']['conversion']['system']!=maskframe:
                #    mcsys['spectral2']['conversion']['system']=maskframe
                #ia.setcoordsys(mcsys)
                #
                ## This code to set the maskframe is copied from makemaskimages()
#                mycsys=ia.coordsys()
#                if mycsys.torecord()['spectral2']['conversion']['system']!=maskframe:
#                    mycsys.setreferencecode(maskframe,'spectral',True)
#                self.csys=mycsys.torecord()
#                if self.csys['spectral2']['conversion']['system']!=maskframe:
#                    self.csys['spectral2']['conversion']['system']=maskframe
#                ia.setcoordsys(self.csys)

                ia.done(verbose=False)

                self.setReferenceFrameLSRK(img =self.maskimages[self.imagelist[k]])

        # take out extra []'s
        maskobject=self.flatten(maskobject)
        masktext=[]
        # to keep backward compatibility for a mixed outlier file
        # look for boxfiles contains multiple boxes with image ids
        for maskid in range(len(maskobject)):
            if type(maskobject[maskid])==str:
                maskobject[maskid] = [maskobject[maskid]]
                  
            for masklets in maskobject[maskid]:
                if type(masklets)==str:
                    if (os.path.exists(masklets) and 
                        (not commands.getoutput('file '+masklets).count('directory')) and
                        (not commands.getoutput('file '+masklets).split(':')[-1].count('data'))):
                           # extract boxfile name
                        masktext.append(masklets)

        # === boxfile handling ===
        #extract boxfile mask info only for now the rest is
        #processed by makemaskimage. - DEPRECATED and will be removed 
        #in 3.4
        #
        #group circles and boxes in dic for each image field 
        circles, boxes, oldfmts=self.readmultifieldboxfile(masktext)

	# Loop over imagename
	# take out text file names contain multifield boxes and field info  
	# from maskobject and create updated one (updatedmaskobject)
	# by adding boxlists to it instead.
	# Use readmultifieldboxfile to read old outlier/box text file format
	# Note: self.imageids and boxes's key are self.imagelist for new outlier
	#       format while for the old format, they are 'index' in string.

	#maskobject_tmp = maskobject 
	# need to do a deep copy
	import copy
	maskobject_tmp=copy.deepcopy(maskobject)
	updatedmaskobject = [] 
        for maskid in range(len(maskobject_tmp)):
	    if len(circles)!=0 or len(boxes)!=0:
		# remove the boxfiles from maskobject list
		for txtf in masktext:
		    if maskobject_tmp[maskid].count(txtf) and oldfmts[txtf]:
			maskobject_tmp[maskid].remove(txtf)
		updatedmaskobject = maskobject_tmp
	    else:
		updatedmaskobject = maskobject 
	# adjust no. of elements of maskoject list with []
	if len(updatedmaskobject)-len(self.imagelist)<0:
	    for k in range(len(self.imagelist)-len(updatedmaskobject)):
		updatedmaskobject.append([])            
	#for maskid in range(len(self.maskimages)):

	# === boxfile handling ====
	for maskid in self.maskimages.keys(): 
	    # for handling old format
	    #if nmaskobj <= maskid:
	    # add circles,boxes back
	    maskindx = self.maskimages.keys().index(maskid)
	    if len(circles) != 0:
		for key in circles:
		    if  (newformat and maskid==key) or \
			(not newformat and maskid.split('_')[-1]==key):
			if len(circles[key])==1:
			   incircles=circles[key][0]
			else: 
			   incircles=circles[key]
			# put in imagelist order
			if len(incircles)>1 and isinstance(incircles[0],list):
			    updatedmaskobject[self.imagelist.values().index(maskid)].extend(incircles)
			else:
			    updatedmaskobject[self.imagelist.values().index(maskid)].append(incircles)
	    if len(boxes) != 0:
		for key in boxes:
		    #try: 
		    #    keyid=int(key)
		    #except:
		    #    keyid=key
		    if  (newformat and maskid==key) or \
			(not newformat and maskid.split('_')[-1]==key):
			if len(boxes[key])==1:
			    inboxes=boxes[key][0]
			else: 
			    inboxes=boxes[key]
			# add to maskobject (extra list bracket taken out)
			# put in imagelist order
			# take out extra []
			if len(inboxes)>1 and isinstance(inboxes[0],list):
			   updatedmaskobject[self.imagelist.values().index(maskid)].extend(inboxes)
			else:
			   updatedmaskobject[self.imagelist.values().index(maskid)].append(inboxes)
	# === boxfile handling ====

        # do maskimage creation (call makemaskimage)
        for maskid in range(len(self.maskimages)):
            if maskid < len(updatedmaskobject):
                self._casalog.post("Matched masks: maskid=%s mask=%s" % (maskid, updatedmaskobject[maskid]), 'DEBUG1')
                self.outputmask=''
                self.makemaskimage(outputmask=self.maskimages[self.imagelist[maskid]],
                imagename=self.imagelist[maskid], maskobject=updatedmaskobject[maskid], slice=slice)
#            self._casalog.post("Matched masks: maskid=%s mask=%s" % (maskid, updatedmaskobject[maskid]), 'DEBUG1')
#            self.outputmask=''
#            self.makemaskimage(outputmask=self.maskimages[self.imagelist[maskid]], 
#            imagename=self.imagelist[maskid], maskobject=updatedmaskobject[maskid], slice=slice)

        for key in self.maskimages.keys():
            if(os.path.exists(self.maskimages[key])):
                ia.open(self.maskimages[key])
                fsum=ia.statistics(verbose=False,list=False)['sum']
                if(len(fsum)!=0 and fsum[0]==0.0):
                    # make an empty mask
                    ia.set(pixels=0.0)
                    # should not remove empty mask for multifield case
                    # interactive=F.
                    # Otherwise makemaskimage later does not work
                    # remove the empty mask
                    if not interactive:
                        ia.remove()
                ia.done(verbose=False)


    def make_mask_from_threshhold(self, imagename, thresh, outputmask=None):
        """
        Makes a mask image with the same coords as imagename where each
        pixel is True if and only if the corresponding pixel in imagename
        is >= thresh.

        The mask will be named outputmask (if provided) or imagename +
        '.thresh_mask'.  The name is returned on success, or False on failure.
        """
        if not outputmask:
            outputmask = imagename + '.thresh_mask'

        # im.mask would be a lot shorter, but it (unnecessarily) needs im to be
        # open with an MS.
        # I am not convinced that im.mask should really be using Quantity.
        # qa.quantity(quantity) = quantity.
        self.im.mask(imagename, outputmask, qa.quantity(thresh))
        
        ## # Copy imagename to a safe name to avoid problems with /, +, -, and ia.
        ## ia.open(imagename)
        ## shp = ia.shape()
        ## ia.close()
        ## self.copymaskimage(imagename, shp, '__temp_mask')
        
        ## self.copymaskimage(imagename, shp, outputmask)
        ## ia.open(outputmask)
        ## ###getchunk is a mem hog
        ## #arr=ia.getchunk()
        ## #arr[arr>0.01]=1
        ## #ia.putchunk(arr)
        ## #inpix="iif("+"'"+outputmask.replace('/','\/')+"'"+">0.01, 1, 0)"
        ## #ia.calc(pixels=inpix)
        ## ia.calc(pixels="iif(__temp_mask>" + str(thresh) + ", 1, 0)")
        ## ia.close()
        ## ia.removefile('__temp_mask')
        return outputmask
        
    def makemaskimage(self, outputmask='', imagename='', maskobject=[], slice=-1):
        """
        This function is an attempt to convert all the kind of 'masks' that
        people want to throw at it and convert it to a mask image to be used
        by imager...For now 'masks' include
        
        a)set of previous mask images
        b)lists of blc trc's
        c)record output from rg tool for e.g

        * for a single field 
        """
        if (not hasattr(maskobject, '__len__')) \
           or (len(maskobject) == 0) or (maskobject == ['']):
            return
        maskimage=[]
        masklist=[]
        textreglist=[]
        masktext=[]
        maskrecord={}
        tablerecord=[]
        # clean up any left over temp files from previous clean runs
        if os.path.exists("__temp_mask"):
          shutil.rmtree("__temp_mask")
        if os.path.exists("__temp_mask2"):
          shutil.rmtree("__temp_mask2")
  
        # relax to allow list input for imagename 
        if(type(imagename)==list):
           imagename=imagename[0] 

        if(type(maskobject)==dict):
            maskrecord=maskobject
            maskobject=[]
        if(type(maskobject)==str):
            maskobject=[maskobject]
        
        if(type(maskobject) != list):
            ##don't know what to do with this
            raise TypeError, 'Dont know how to deal with maskobject'
        if(numpy.issubdtype(type(maskobject[0]),numpy.int) or \
            numpy.issubdtype(type(maskobject[0]),numpy.float)):
            # check and convert if list consist of python int or float  
            maskobject_tmp = convert_numpydtype(maskobject)
            masklist.append(maskobject_tmp)
        else:
            for masklets in maskobject:
                if(type(masklets)==str): ## Can be a file name, or an explicit region-string
                    if(os.path.exists(masklets)):
                        if(commands.getoutput('file '+masklets).count('directory')):
                            maskimage.append(masklets)
                        elif(commands.getoutput('file '+masklets).count('text')):
                            masktext.append(masklets)
                        else:
                            tablerecord.append(masklets)
                    else:
                       textreglist.append(masklets);
                       #raise TypeError, masklets+' seems to be non-existant' 
                if(type(masklets)==list):
                    masklets_tmp = convert_numpydtype(masklets)
                    masklist.append(masklets_tmp)
                if(type(masklets)==dict):
                    maskrecord=masklets
        if(len(outputmask)==0):
            outputmask=imagename+'.mask'
        if(os.path.exists(outputmask)):
            # for multiple field 
            # outputmask is always already defined
            # cannot use copymaskiamge since self.csys used in the code
            # fixed to that of main field
            if len(self.imagelist)>1:
              ia.fromimage('__temp_mask',outputmask,overwrite=True)
              ia.close()        
            else:
              self.im.make('__temp_mask')    
            ia.open('__temp_mask')
            shp=ia.shape()
            self.csys=ia.coordsys().torecord()
            ia.close()
            ia.removefile('__temp_mask')
            ia.open(outputmask)
            outim = ia.regrid(outfile='__temp_mask',shape=shp,axes=[3,0,1], csys=self.csys,overwrite=True, asvelocity=False)
            outim.done(verbose=False)
            ia.done(verbose=False)
            ia.removefile(outputmask)
            os.rename('__temp_mask',outputmask)
        else:
            self.im.make(outputmask)
            if len(self.imagelist)>1:
                raise Exception, "Multifield case - requires initial mask images but undefined."   

        # respect dataframe or outframe
        if self.usespecframe=='': 
            maskframe=self.dataspecframe
        else:
            maskframe=self.usespecframe
        if len(self.vis)!=1:
            if  not self.inframe:
                # for multi-ms case default output frame is default to LSRK
                # (set by baseframe in imager_cmt.cc) 
                maskframe='LSRK'
                
        ia.open(outputmask)
        shp=ia.shape()
        self.csys=ia.coordsys().torecord()
        # keep this info for reading worldbox
        self.csysorder=ia.coordsys().coordinatetype()
#        ia.close()

#        self.setReferenceFrameLSRK( outputmask )
                
        mycsys=ia.coordsys()
        if mycsys.torecord()['spectral2']['conversion']['system']!=maskframe:
            mycsys.setreferencecode(maskframe,'spectral',True)
        self.csys=mycsys.torecord()
        if self.csys['spectral2']['conversion']['system']!=maskframe:
            self.csys['spectral2']['conversion']['system']=maskframe
        ia.setcoordsys(self.csys)
        #ia.setcoordsys(mycsys.torecord())
#        ia.close()

        if(len(maskimage) > 0):
            for ima in maskimage :
                if slice>-1:
                    self.getchanimage(ima, ima+'chanim',slice)
                    self.copymaskimage(ima+'chanim',shp,'__temp_mask')
                    ia.removefile(ima+'chanim')
                else:
                    self.copymaskimage(ima, shp, '__temp_mask')
                #ia.open(ima)
                #ia.regrid(outfile='__temp_mask',shape=shp,csys=self.csys,
                #          overwrite=True)
                #ia.done(verbose=False)
                os.rename(outputmask,'__temp_mask2')
                outim = ia.imagecalc(outfile=outputmask,
                             pixels='__temp_mask + __temp_mask2',
                             overwrite=True)
                outim.done(verbose=False)
                ia.done(verbose=False)
                ia.removefile('__temp_mask')
                ia.removefile('__temp_mask2')
	    if(not os.path.exists(outputmask)):
                outputmask = self.make_mask_from_threshhold(outputmask, 0.01,
                                                                  outputmask)
        #pdb.set_trace()
        #### This goes when those tablerecord goes
        ### Make masks from tablerecords
        if(len(tablerecord) > 0):
            reg={}
            for tabl in tablerecord:
                try:
                    reg.update({tabl:rg.fromfiletorecord(filename=tabl, verbose=False)})
                except:
                    raise Exception,'Region-file (binary) format not recognized. Please check. If box-file, please start the file with \'#boxfile\' on the first line';
            if(len(reg)==1):
                reg=reg[reg.keys()[0]]
            else:
                reg=rg.makeunion(reg)
            self.im.regiontoimagemask(mask=outputmask, region=reg)
        ###############
        ### Make masks from region dictionaries
        if((type(maskrecord)==dict) and (len(maskrecord) > 0)):
            self.im.regiontoimagemask(mask=outputmask, region=maskrecord)
        ### Make masks from text files
        if(len(masktext) >0):
            for textfile in masktext :
                # Read a box file
                polydic,listbox=self.readboxfile(textfile);

                masklist.extend(listbox)
                if(len(polydic) > 0):
                    ia.open(outputmask)
                    ia.close()
                    self.im.regiontoimagemask(mask=outputmask, region=polydic)
                # If box lists are empty, it may be a region format
                if(len(polydic)==0 and len(listbox)==0):
                    # Read in a region file
                    try:
                        ia.open(outputmask);
                        mcsys = ia.coordsys();
                        mshp = ia.shape();
                        ia.close();
                        mreg = rg.fromtextfile(filename=textfile,shape=mshp,csys=mcsys.torecord());
                        self.im.regiontoimagemask(mask=outputmask, region=mreg);
                    except:
                        raise Exception,'Region-file (text) format not recognized. Please check. If box-file, please start the file with \'#boxfile\' on the first line, and have at-least one valid box in it';
        ### Make masks from inline lists of pixel coordinates
        if((type(masklist)==list) and (len(masklist) > 0)):
            self.im.regiontoimagemask(mask=outputmask, boxes=masklist)
        ### Make masks from inline region-strings
        if((type(textreglist)==list) and (len(textreglist)>0)):
             ia.open(outputmask);
             mcsys = ia.coordsys();
             mshp = ia.shape();
             ia.close();
             for textlet in textreglist:
                try:
                    mreg = rg.fromtext(text=textlet,shape=mshp,csys=mcsys.torecord());
                    self.im.regiontoimagemask(mask=outputmask, region=mreg);
                except:
                    raise Exception,'\''+textlet+'\' is not recognized as a text file on disk or as a region string';
        ### Make mask from an image-mask
        if(os.path.exists(imagename) and (len(rg.namesintable(imagename)) !=0)):
            regs=rg.namesintable(imagename)
            if(type(regs)==str):
                    regs=[regs]
            for reg in regs:
                elrec=rg.fromtabletorecord(imagename, reg)
                self.im.regiontoimagemask(mask=outputmask, region=elrec)

        self.outputmask=outputmask

        ## CAS-5227
        ia.open( outputmask )
        ia.calc('iif("'+outputmask+'"!=0.0,1.0,0.0)')
        ia.close()
        
        ## CAS-5221
        self.setReferenceFrameLSRK( outputmask )
        #Done with making masks


    def datselweightfilter(self, field, spw, timerange, uvrange, antenna,scan,
                           wgttype, robust, noise, npixels, mosweight,
                           innertaper, outertaper, usescratch, nchan=-1, start=0, width=1):
        """
        Make data selection 
        (not in use, split into datsel and  datweightfileter)
        """  
        rmode='none'
        weighting='natural';
        if(wgttype=='briggsabs'):
            weighting='briggs'
            rmode='abs'
        elif(wgttype=='briggs'):
            weighting='briggs'
            rmode='norm'
        else:
            weighting=wgttype
            
        self.fieldindex=ms.msseltoindex(self.vis,field=field)['field'].tolist()
        if(len(self.fieldindex)==0):
            fieldtab=self.getsubtable(self.vis, 'FIELD')
            tb.open(fieldtab)
            self.fieldindex=range(tb.nrows())
            tb.close()
        #weighting and tapering should be done together
        if(weighting=='natural'):
            mosweight=False
        self.im.selectvis(nchan=nchan,start=start,step=width,field=field,spw=spw,time=timerange,
                              baseline=antenna, scan=scan, uvrange=uvrange, usescratch=usescratch)
        self.im.weight(type=weighting,rmode=rmode,robust=robust, npixels=npixels, noise=qa.quantity(noise,'Jy'), mosaic=mosweight)
        if((type(outertaper)==list) and (len(outertaper) > 0)):
            if(len(outertaper)==1):
                outertaper.append(outertaper[0])
                outertaper.append('0deg')
            if(qa.quantity(outertaper[0])['value'] > 0.0):    
                self.im.filter(type='gaussian', bmaj=outertaper[0],
                               bmin=outertaper[1], bpa=outertaper[2])

    
    # split version of datselweightfilter
    def datsel(self, field, spw, timerange, uvrange, antenna, scan, observation,intent,
                           usescratch, nchan=-1, start=0, width=1):
        """
        Make selections in visibility data 
        """ 

        # for multi-MSes, if field,spw,timerage,uvrange,antenna,scan are not
        # lists the same selection is applied to all the MSes.
        self.fieldindex=[]
        #nvislist=range(len(self.vis))
        vislist=self.sortedvisindx
        self.paramlist={'field':field,'spw':spw,'timerange':timerange,'antenna':antenna,
                        'scan':scan, 'observation': observation, 'intent':intent, 'uvrange':uvrange}
        for i in vislist:
          selectedparams=self._selectlistinputs(len(vislist),i,self.paramlist)
          tempfield=selectedparams['field']
#         if type(field)==list:
#            if len(field)==nvislist:
#              tempfield=field[i]
#            else:
#              if len(field)==1:
#                 tempfield=field[0]
#              else:
#	          raise Exception, 'The number of field list does not match with the number of vis list'
#         else:
#            tempfield=field
              
          if len(tempfield)==0:
            tempfield='*'
          self.fieldindex.append(ms.msseltoindex(self.vis[i],field=tempfield)['field'].tolist())

        ############################################################
        # Not sure I need this now.... Nov 15, 2010
        vislist.reverse()
        writeaccess=True
        for i in vislist: 
            writeaccess=writeaccess and os.access(self.vis[i], os.W_OK)
        #if any ms is readonly then no model will be stored, MSs will be in readmode only...but clean can proceed
        for i in vislist:
          # select apropriate parameters
          selectedparams=self._selectlistinputs(len(vislist),i,self.paramlist)
          inspw=selectedparams['spw'] 
          intimerange=selectedparams['timerange'] 
          inantenna=selectedparams['antenna'] 
          inscan=selectedparams['scan']
          inobs = selectedparams['observation']
          inintent = selectedparams['intent']
          inuvrange=selectedparams['uvrange'] 

          #if len(self.vis)==1:
            #print "single ms case"
          #  self.im.selectvis(nchan=nchan,start=start,step=width,field=field,
          #                    spw=inspw,time=intimerange, baseline=inantenna,
          #                    scan=inscan, observation=inobs, intent=inintent, uvrange=inuvrange,
          #                    usescratch=usescratch)
          #else:
            #print "multims case: selectvis for vis[",i,"]: spw,field=", inspw, self.fieldindex[i]
          self.im.selectvis(vis=self.vis[i],nchan=nchan,start=start,step=width,
                            field=self.fieldindex[i], spw=inspw,time=intimerange,
                            baseline=inantenna, scan=inscan,
                            observation=inobs, intent=inintent,
                            uvrange=inuvrange, usescratch=usescratch, writeaccess=writeaccess)

    # private function for datsel and datweightfilter
    def _selectlistinputs(self,nvis,indx,params):
        """
        A little private function to do selection and checking for a parameter 
        given in list of strings.
        It checks nelement in each param either match with nvis or nelement=1
        (or a string) otherwise exception is thrown. 
        """
        outparams={}
        if type(params)==dict:
          for param,val in params.items():
            msg = 'The number of %s list given in list does not match with the number of vis list given.' % param
            if type(val)==list:
              if len(val)==nvis:
                outval=val[indx]
              else:
                if len(val)==1:
                  outval=val[0]
                else:
                  raise Exception, msg
              outparams[param]=outval
            else:
              #has to be a string
              outparams[param]=val
          return outparams
        else:
          raise Exception, 'params must be a dictionary'
 
    # weighting/filtering part of datselweightfilter.
    # The scan parameter is not actually used, so observation is not included
    # as a parameter.  Both are used via self._selectlistinputs().
    def datweightfilter(self, field, spw, timerange, uvrange, antenna,scan,
                        wgttype, robust, noise, npixels, mosweight,
                        uvtaper,innertaper, outertaper, usescratch, nchan=-1, start=0, width=1):
        """
        Apply weighting and tapering 
        """
        rmode='none'
        weighting='natural';
        if(wgttype=='briggsabs'):
            weighting='briggs'
            rmode='abs'
        elif(wgttype=='briggs'):
            weighting='briggs'
            rmode='norm'
        else:
            weighting=wgttype
         #weighting and tapering should be done together
        if(weighting=='natural'):
            mosweight=False
#        vislist=self.sortedvisindx
        #nvislist.reverse()
#        for i in vislist:
#          # select apropriate parameters
#          selectedparams=self._selectlistinputs(len(vislist),i,self.paramlist)
#          inspw=selectedparams['spw'] 
#          intimerange=selectedparams['timerange'] 
#          inantenna=selectedparams['antenna'] 
#          inscan=selectedparams['scan'] 
#          inobs=selectedparams['observation'] 
#          inuvrange=selectedparams['uvrange'] 
#          
#          if len(self.vis) > 1:
#            print 'from datwtfilter - multi';
#            self.im.selectvis(vis=self.vis[i], field=self.fieldindex[i],spw=inspw,time=intimerange,
#                              baseline=inantenna, scan=inscan, observation=inobs,
#                              uvrange=inuvrange, usescratch=calready)
#          else: 
#            print 'from datwtfilter - single';
#            self.im.selectvis(field=field,spw=inspw,time=intimerange,
#                              baseline=inantenna, scan=inscan, observation=inobs,
#                              uvrange=inuvrange, usescratch=calready)
#          self.im.weight(type=weighting,rmode=rmode,robust=robust, 
#                         npixels=npixels, noise=qa.quantity(noise,'Jy'), mosaic=mosweight)
        self.im.weight(type=weighting,rmode=rmode,robust=robust, 
                         npixels=npixels, noise=qa.quantity(noise,'Jy'), mosaic=mosweight)
     
        if((uvtaper==True) and (type(outertaper) in (str, int, float, long))):
            outertaper=[outertaper]
        if((uvtaper==True) and (type(outertaper)==list) and (len(outertaper) > 0)):
            if(len(outertaper)==1):
                outertaper.append(outertaper[0])
            if(len(outertaper)==2):
                outertaper.append('0deg')
            if(qa.quantity(outertaper[0])['unit']==''):
                outertaper[0]=qa.quantity(qa.quantity(outertaper[0])['value'],'lambda')
            if(qa.quantity(outertaper[1])['unit']==''):
                outertaper[1]=qa.quantity(qa.quantity(outertaper[1])['value'],'lambda')
            if(qa.quantity(outertaper[0])['value'] > 0.0):
                self.im.filter(type='gaussian', bmaj=outertaper[0],
                               bmin=outertaper[1], bpa=outertaper[2])


    def setrestoringbeam(self, restoringbeam):
        """
        Set restoring beam
        """
        if((restoringbeam == ['']) or (len(restoringbeam) ==0)):
            return
        resbmaj=''
        resbmin=''
        resbpa='0deg'
        if((type(restoringbeam) == list)  and len(restoringbeam)==1):
            restoringbeam=restoringbeam[0]
        if((type(restoringbeam)==str)):
            if(qa.quantity(restoringbeam)['unit'] == ''):
                restoringbeam=restoringbeam+'arcsec'
            resbmaj=qa.quantity(restoringbeam, 'arcsec')
            resbmin=qa.quantity(restoringbeam, 'arcsec')
        if(type(restoringbeam)==list):			
            resbmaj=qa.quantity(restoringbeam[0], 'arcsec')
            resbmin=qa.quantity(restoringbeam[1], 'arcsec')
            if(resbmaj['unit']==''):
                resbmaj=restoringbeam[0]+'arcsec'
            if(resbmin['unit']==''):
                resbmin=restoringbeam[1]+'arcsec'
            if(len(restoringbeam)==3):
                resbpa=qa.quantity(restoringbeam[2], 'deg')
                if(resbpa['unit']==''):
                    resbpa=restoringbeam[2]+'deg'
        if((resbmaj != '') and (resbmin != '')):
           self.im.setbeam(bmaj=resbmaj, bmin=resbmin, bpa=resbpa)
        
    def convertmodelimage(self, modelimages=[], outputmodel='',imindex=0):
        """
        Convert model inputs to a model image

        Keyword arguments:
        modleimages -- input model list
        outputmodel -- outout modelimage name
        imindex     -- image name index (corresponding to imagelist) 
                       for multi field hanlding
        """
        modelos=[]
        maskelos=[]
        if((modelimages=='') or (modelimages==[]) or (modelimages==[''])):
        #if((modelimages=='') or (modelimages==[])):
            return
        if(type(modelimages)==str):
            modelimages=[modelimages]
        k=0
        for modim in modelimages:
            if not os.path.exists(modim):
                raise Exception, "Model image file name="+modim+" does not exist."

            ia.open(modim)
            modelosname='modelos_'+str(k) 

            # clean up any temp files left from preveous incomplete run
            if os.path.exists(modelosname):
                ia.removefile(modelosname)
            if os.path.exists('__temp_model2'):
                ia.removefile('__temp_model2')

            modelos.append(modelosname)

            
            if( (ia.brightnessunit().count('/beam')) > 0):
                ##single dish-style model
                maskelos.append(modelos[k]+'.sdmask')
                self.im.makemodelfromsd(sdimage=modim,modelimage=modelos[k],maskimage=maskelos[k])
                ia.open(maskelos[k])
                ##sd mask cover whole image...delete it as it is not needed
                if((ia.statistics(verbose=False,list=False)['min']) >0):
                    ia.remove(done=True, verbose=False)
                    maskelos.remove(maskelos[k])
                ia.done()
            else:
                ##assuming its a model image already then just regrid it
                #self.im.make(modelos[k])
                shutil.copytree(self.imagelist[imindex],modelos[k])
                ia.open(modelos[k])
                newcsys=ia.coordsys()
                newshape=ia.shape()
                ia.open(modim)
                ib=ia.regrid(outfile=modelos[k], shape=newshape, axes=[0,1,3], csys=newcsys.torecord(), overwrite=True, asvelocity=False)
                ib.done(verbose=False)
                
            k=k+1
            if ia.isopen(): ia.close()
        #########
        if((len(maskelos)==1) and  (self.outputmask == '')):
            self.outputmask=modelimages[0]+'.mask'
            if(os.path.exists(self.outputmask)):
                ia.removefile(self.outputmask)
            os.rename(maskelos[0],self.outputmask)
        elif(len(maskelos) > 0):
            if(self.outputmask == ''):
                self.outputmask=modelimages[0]+'.mask'
                
            else:
                outputmask=self.outputmask
            ##okay if outputmask exists then we need to do an "and" with
            ##the sdmask one
            doAnd=False;
            if(os.path.exists(outputmask)):
                ia.open(outputmask)
                if((ia.statistics(verbose=False, list=False)['max'].max()) > 0.00001):
                    doAnd=True
                ia.close()
            if(doAnd):
                tmpomask='__temp_o_mask'
                self.makemaskimage(outputmask=tmpomask, maskobject=maskelos)
                os.rename(outputmask, '__temp_i_mask')
                outim = ia.imagecalc(outfile=outputmask, pixels='__temp_o_mask * __temp_i_mask', overwrite=True)
                outim.done(verbose=False)
                ia.removefile('__temp_o_mask')
                ia.removefile('__temp_i_mask')
                self.outputmask=outputmask
            else:
                self.makemaskimage(outputmask=outputmask, maskobject=maskelos)
        for ima in maskelos:
            if(os.path.exists(ima)):
                ia.removefile(ima)
        if(not (os.path.exists(outputmodel))):
            # im.make uses the main field coord. so it does
            # not make correct coord. for outlier fields
            if len(self.imagelist)>1:
               ia.fromimage(outputmodel, self.imagelist[imindex])
            else:
               self.im.make(outputmodel)
        #ia.open(outputmodel)
        #ia.close()
        for k in range(len(modelos)):
            # if os.rename() or shutil.move() is used here,
            # for k=1 and at image.imagecalc, it seems to cause 
            # casapy to crash... 
            #os.rename(outputmodel,'__temp_model2')
            shutil.copytree(outputmodel,'__temp_model2')
            
            outim = ia.imagecalc(outfile=outputmodel,
                             pixels=modelos[k]+' + '+'__temp_model2',
                             overwrite=True)
            outim.done(verbose=False)
            ia.removefile('__temp_model2')
            ia.removefile(modelos[k]);
    
    def readboxfile(self, boxfile):
        """ Read a file containing clean boxes (compliant with AIPS BOXFILE)

        Format is:
        #FIELDID BLC-X BLC-Y TRC-X TRC-Y
        0       110   110   150   150 
        or
        0       hh:mm:ss.s dd.mm.ss.s hh:mm:ss.s dd.mm.ss.s
        
        Note all lines beginning with '#' are ignored.
        
        """
        union=[]
        polyg={}
        f=open(boxfile)
        temprec={}
        counter=0
        while 1:
            try:
                counter=counter+1
                line=f.readline()
                if(len(line)==0):
                    raise Exception
                if (line.find('#')!=0):
                    if(line.count('[')==2):
                        ##its an output from qtclean
                        line=line.replace('\n','')
                        line=line.replace('\t',' ')
                        line=line.replace('[',' ')
                        line=line.replace(']',' ')
                        line=line.replace(',',' ')
                        splitline=line.split()
                        if(len(splitline)==5):
                            ##its box
                            if(int(splitline[4]) > 0):
                                ##it was a "mask" region not "erase"
                                boxlist=[int(splitline[0]),int(splitline[1]),
                                         int(splitline[2]),int(splitline[3])]
                                union.append(boxlist)
                        else:
                            #its a polygon
                            x=[]
                            y=[]
                            if(int(splitline[len(splitline)-1]) > 0):
                                ###ignore erase regions
                                nnodes=(len(splitline)-1)/2
                                for kk in range(nnodes):
                                    x.append(splitline[kk]+'pix')
                                    y.append(splitline[kk+nnodes]+'pix')
                                elreg=rg.wpolygon(x=x, y=y, csys=self.csys)
                                temprec.update({counter:elreg})
                                
                    elif(line.count('worldbox')==1): 
                        self._casalog.post('\'worldbox\' is deprecated please use CRTF format','WARN')
                        #ascii box file from viewer or boxit
                        # expected foramt: 'worldbox' pos_ref [lat
                        line=line.replace('[',' ')
                        line=line.replace(']',' ')
                        line=line.replace(',',' ')
                        line=line.replace('\'',' ')
                        splitline=line.split() 
                        if len(splitline) != 13:
                           raise TypeError, 'Error reading worldbox file'      
                        #
                        refframe=self.csys['direction0']['conversionSystem']
                        if refframe.find('_VLA')>0:
                          refframe=refframe[0:refframe.find('_VLA')]
                        ra =[splitline[2],splitline[3]]
                        dec = [splitline[4],splitline[5]]
                        #set frames
                        obsdate=self.csys['obsdate']
                        me.doframe(me.epoch(obsdate['refer'], str(obsdate['m0']['value'])+obsdate['m0']['unit']))
                        me.doframe(me.observatory(self.csys['telescope']))
                        #
                        if splitline[1]!=refframe: 
                            # coversion between different epoch (and to/from AZEL also)
                            radec0 = me.measure(me.direction(splitline[1],ra[0],dec[0]), refframe)
                            radec1 = me.measure(me.direction(splitline[1],ra[1],dec[1]), refframe) 
                            ra=[str(radec0['m0']['value'])+radec0['m0']['unit'],\
                                str(radec1['m0']['value'])+radec1['m0']['unit']]
                            dec=[str(radec0['m1']['value'])+radec0['m1']['unit'],\
                                 str(radec1['m1']['value'])+radec1['m1']['unit']]
                        # check for stokes 
                        stokes=[]
                        imstokes = self.csys['stokes1']['stokes']
                        for st in [splitline[10],splitline[11]]:
                            prevlen = len(stokes)
                            for i in range(len(imstokes)):
                                if st==imstokes[i]:
                                    stokes.append(str(i)+'pix')
                                elif st.count('pix') > 0:
                                    stokes.append(st)
                            if len(stokes)<=prevlen:
                                #raise TypeError, "Stokes %s for the box boundaries is outside image" % st              
                                self._casalog.post('Stokes %s for the box boundaries is outside image, -ignored' % st, 'WARN')              
                        # frequency
                        freqs=[splitline[7].replace('s-1','Hz'), splitline[9].replace('s-1','Hz')]
                        fframes=[splitline[6],splitline[8]]
                        #imframe = self.csys['spectral2']['system']
                        imframe = self.csys['spectral2']['conversion']['system']
                        #print "imframe=",imframe," system frame =",self.csys['spectral2']['system']," frame in boxfile=", fframes[0]
                        # the worldbox file created viewer's "box in file"
                        # currently says TOPO in frequency axis but seems to
                        # wrong (the freuencies look like in the image's base
                        # frame). 
                        for k in [0,1]:
                            if fframes[k]!=imframe and freqs[k].count('pix')==0:
                                #do frame conversion
                                #self._casalog.post('Ignoring the frequency frame of the box for now', 'WARN')              
                                # uncomment the following when box file correctly labeled the frequency frame
                                me.doframe(me.direction(splitline[1],ra[k],dec[k]))
                                mf=me.measure(me.frequency(fframes[k],freqs[k]),imframe)
                                freqs[k]=str(mf['m0']['value'])+mf['m0']['unit']
                        coordorder=self.csysorder
                        wblc = []
                        wtrc = []
                        for type in coordorder:
                          if type=='Direction':
                             wblc.append(ra[0])
                             wblc.append(dec[0])
                             wtrc.append(ra[1])
                             wtrc.append(dec[1])     
                          if type=='Stokes':
                             wblc.append(stokes[0])
                             wtrc.append(stokes[1])
                          if type=='Spectral':
                             wblc.append(freqs[0])
                             wtrc.append(freqs[1])

                        #wblc = [ra[0], dec[0], stokes[0], freqs[0]]
                        #wtrc = [ra[1], dec[1], stokes[1], freqs[1]]
                        #wblc = ra[0]+" "+dec[0]
                        #wtrc = ra[1]+" "+dec[1]
                        #print"wblc=",wblc," wtrc=",wtrc," using system frame=",self.csys['spectral2']['system'], " convertion frame=",self.csys['spectral2']['conversion']['system']
                        wboxreg = rg.wbox(blc=wblc,trc=wtrc,csys=self.csys)
                        temprec.update({counter:wboxreg})
 
                    else:
                        ### its an AIPS boxfile
                        splitline=line.split('\n')
                        splitline2=splitline[0].split()
                        if (len(splitline2)<6):
                            if(int(splitline2[1])<0):
                                ##circle
                                #circlelist=[int(splitline2[2]),
                                #     int(splitline2[3]),int(splitline2[4])]
                                #circles[splitline2[0]].append(circlelist)
                                continue
                            else:
                                boxlist=[int(splitline2[1]),int(splitline2[2]),
                                         int(splitline2[3]),int(splitline2[4])]
                                union.append(boxlist)
                        else:
                           ## Don't know what that is
                           ## might be a facet definition 
                           continue
    
            except:
                break

        f.close()
        if(len(temprec)==1):
            polyg=temprec[temprec.keys()[0]]
        elif (len(temprec) > 1):
            #polyg=rg.dounion(temprec)
            polyg=rg.makeunion(temprec)
        
        return polyg,union

    def readmultifieldboxfile(self, boxfiles):
        """ 
        Read boxes and circles in text files in the 
        AIPS clean boxfile format.

        Keyword arguments:
        boxfiles -- text files in boxfile format

        returns:
        circles -- dictionary containing circles      
        boxes   -- dictionary conatining boxes ([blc, trc])
        oldfileformats -- a list of boolean if the input textfiles
                   are boxfile format. 
        """  
        circles={}
        boxes={}
        oldfilefmts={}
        for k in range(len(self.imageids)):
            circles[self.imageids[k]]=[]
            boxes[self.imageids[k]]=[]
        for boxfile in boxfiles:
            f=open(boxfile)
            setonce=False
            oldfilefmts[boxfile]=False
            while 1:
                try:
                    line=f.readline()
                    if(len(line)==0):
                        raise Exception
                    if line.find('#')==0:
                        if not setonce and line.find('boxfile')>0:
                            oldfilefmts[boxfile]=True
                            setonce=True
                            self._casalog.post(boxfile+" is in a deprecated boxfile format,"+\
                                " will not be supported in the future releases","WARN")
                            #raise Exception
                    else:
                        ### its an AIPS boxfile
                        splitline=line.split('\n')
                        splitline2=splitline[0].split()
                        if (len(splitline2)<6):
                            ##circles
                            if(int(splitline2[1]) <0):
                                circlelist=[int(splitline2[2]),
                                            int(splitline2[3]),int(splitline2[4])]
                                #circles[splitline2[0]].append(circlelist)
                                circles[self.imageids[int(splitline2[0])]].append(circlelist)
                            else:
                                #boxes
			        if(len(splitline2)==5):
                                   boxlist=[int(splitline2[1]),int(splitline2[2]),
                                            int(splitline2[3]),int(splitline2[4])]
                                   #boxes[splitline2[0]].append(boxlist)
                                   boxes[self.imageids[int(splitline2[0])]].append(boxlist)
                        else:
                           ## Don't know what that is
                           ## might be a facet definition 
                            continue
    
                       
    
                except:
                    break

            f.close()
        ###clean up the records
        for k in range(len(self.imageids)):
            if(circles[self.imageids[k]]==[]):
                circles.pop(self.imageids[k])
            if(boxes[self.imageids[k]]==[]):
                boxes.pop(self.imageids[k])
                
        return circles,boxes,oldfilefmts


    def readoutlier(self, outlierfile):
        """ Read a file containing clean boxes (kind of
        compliant with AIPS FACET FILE)
                    
        Format is:
         col0    col1   col2  col3 col4 col5 col6 col7  col8   col9
          C    FIELDID SIZEX SIZEY RAHH RAMM RASS DECDD DECMM DECSS      
        why first column has to have C ... because its should
        not to be A or B ...now D would be a totally different thing.
        
        'C' as in AIPS BOXFILE format indicates the file specify the coordiates
        for field center(s). 

        Note all lines beginning with '#' are ignored.
        (* Lines with first column other than C or c are also ignored)
        
        """
        imsizes=[]
        phasecenters=[]
        imageids=[]
        f=open(outlierfile)
        while 1:
            try:
                line=f.readline()
                    
                if(len(line)==0):
                    raise Exception
                if (line.find('#')!=0):
                    splitline=line.split('\n')
                    splitline2=splitline[0].split()
                    if (len(splitline2)==10):
                        if(splitline2[0].upper()=='C'):
                            imageids.append(splitline2[1])
                            imsizes.append((int(splitline2[2]),int(splitline2[3])))
                            mydir='J2000  '+splitline2[4]+'h'+splitline2[5]+'m'+splitline2[6]+'  '+splitline2[7]+'d'+splitline2[8]+'m'+splitline2[9]
                            phasecenters.append(mydir)
    
            except:
                break

        f.close()
        return imsizes,phasecenters,imageids

    def newreadoutlier(self, outlierfile): 
        """
	Read a outlier file (both old and new format) 
         
	The new format consists of a set of task parameter inputs.
	imagename="outlier1" imsize=[128,128] phasecenter="J2000 hhmmss.s ddmmss.s" 
	imagename="outlier2" imsize=[128,128] phasecenter="J2000 hhmmss.s ddmmss.s"
	mask=['viewermask.rgn','box[[50,60],[50,60]]'] ...

	Currently supported paramters are:
	imagename (required: used to delinate a paramater set for each field)
	imsize (required)
	phasecenter (required)
	mask (optional)
        modelimage (optional) 
	* other parameters can be included in the file but not parsed

        For the old format, readoutlier() is called internally currently. But
        the support will be removed by CASA3.4. 

        Returns:
        lists of imageids, imsizes, phasecenters, masks, 
        and modelimages, and a dictionary contains all the parameters,
        and a boolean to indicate read file is in the new format. 
        """
        import ast
        import re

        imsizes=[]
        phasecenters=[]
        imageids=[]
        masks=[]
        modelimages=[]
        keywd = "imagename"
        oldformat=False
        nchar= len(keywd)
        content0=''
        # set this to True to disallow old outlier file
        noOldOutlierFileSupport=False;

        with open(outlierfile) as f:  
            for line in f:
                try:
                    if len(line)!=0 and line.split()!=[]:
                        if line.split()[0]=='C' or line.split()[0]=='c':
                            oldformat=True
                        elif line.split()[0]!='#':
                            content0+=line     
                except:
                    print "Unkown error while reading the file", outlierfile
                    break
        if oldformat:
            if noOldOutlierFileSupport:
                self._casalog.post("You are using the old outlier file format no longer supported. Please use the new format (see help).","SEVERE")
                raise Exception
            else:
                self._casalog.post("This file format is deprecated. Use of a new format is encouraged.","WARN")
                # do old to new data format conversion....(watch out for different order of return parameters...)
                (imsizes,phasecenters,imageids)=self.readoutlier(outlierfile)
                for i in range(len(imageids)):
                    modelimages.append('')
        #f.seek(0)
        #content0 = f.read()
        #f.close()
        content=content0.replace('\n',' ')
        last = len(content)
        # split the content using imagename as a key 
        # and store each parameter set in pars dict
        pars={}
        initi = content.find(keywd)
        if initi > -1:
            i = 0
            prevstart=initi
            while True:
                #step = nchar*(i+1)
                step = nchar+1
                start = prevstart+step
                nexti = content[start:].find(keywd)
                #print "With start=",start, " found next one at(nexti)=",nexti, " step used =",step, " prevstart=",prevstart

                if nexti == -1:
                    pars[i]=content[prevstart:]
                #    print "range=",prevstart, " to the end"
                    break
                pars[i]=content[prevstart:prevstart+nexti+step]
                #print "pars[",i,"]=",pars[i]
                #print "range=",prevstart, " to", prevstart+nexti+step-1
                prevstart=prevstart+nexti+step
                i+=1

        # for parsing of indiviual par (per field)
        #print "pars=",pars
        dparm ={}
        indx=0
        for key in pars.keys():
            # do parsing
            parstr = pars[key]
            # clean up extra white spaces
            parm=' '.join(parstr.split())
            # more clean up
            parm=parm.replace("[ ","[")
            parm=parm.replace(" ]","]")
            parm=parm.replace(" ,",",")
            parm=parm.replace(", ",",")
            parm=parm.replace(" =","=")
            parm=parm.replace("= ","=")
            #print "parm=",parm
            subdic={}
            # final parameter sets 
            values=re.compile('\w+=').split(parm)
            values=values[1:len(values)]
           
            ipar = 0 
            for pv in parm.split():
                if pv.find('=') != -1:
                    if ipar >= len(values):
                        raise Exception, TypeError("mismath in no. parameters in parsing outlier file.")
                    (k,v) = pv.split('=')
                    # fix a string to proper litral value
                    # take out any commas at end which will
                    # confuse literal_eval function
                    pat = re.compile(',+$')
                    subdic[k]=ast.literal_eval(pat.sub('',values[ipar]))
                    ipar += 1
            dparm[indx]=subdic
            indx+=1
        #print "DONE for parsing parm for each field"
        # put into list of parameters
        # imsizes, phasecenters, imagenames(imageids)
        # mask is passed to other function for forther processing
        if not oldformat:
            #pack them by parameter name
            for fld in dparm.keys():
                # before process, check if it contains all required keys
                # namely, imagename, phasecenter, imsize
                #print "dparm[",fld,"]=",dparm[fld]
                if not (dparm[fld].has_key("imagename") and\
                        dparm[fld].has_key("phasecenter") and\
                        dparm[fld].has_key("imsize")):
                    raise Exception, TypeError("Missing one or more of the required parameters: \
                     imagename, phasecenter, and imsize in the outlier file. Please check the input outlier file.") 
                for key in dparm[fld].keys():  
                    if key == "imagename":
                        imageids.append(dparm[fld][key])
                    if key == "phasecenter":
                        phasecenters.append(dparm[fld][key])
                    if key == "imsize":
                        imsizes.append(dparm[fld][key])
                    if key == "mask":
                        if type(dparm[fld][key])==str:
                            masks.append([dparm[fld][key]])
                        else:
                            masks.append(dparm[fld][key])
                    if key == "modelimage":
                        if type(dparm[fld][key])==str:
                            modelimages.append([dparm[fld][key]])
                        else:
                            modelimages.append(dparm[fld][key])
                if not dparm[fld].has_key("mask"):
                    masks.append([])
                if not dparm[fld].has_key("modelimage"):
                    modelimages.append('')
                
                 
        return (imageids,imsizes,phasecenters,masks,modelimages,dparm, not oldformat) 


    def copymaskimage(self, maskimage, shp, outfile):
        """
        Copy mask image
        
        Keyword arguments:
        maskimage -- input maskimage 
        shp       -- shape of output image 
        outfile   -- output image name
        """
        if outfile == maskimage:     # Make it a no-op,
            return                   # this is more than just peace of mind.
        #pdb.set_trace() 
        ia.open(maskimage)
        oldshp=ia.shape()
        if((len(oldshp) < 4) or (shp[2] != oldshp[2]) or (shp[3] != oldshp[3])):
            #take the first plane of mask
            tmpshp=oldshp
            tmpshp[0]=shp[0]
            tmpshp[1]=shp[1]
            if len(oldshp)==4: # include spectral axis for regrid
              tmpshp[3]=shp[3]
              ib=ia.regrid(outfile='__looloo', shape=tmpshp, axes=[3,0,1], csys=self.csys, overwrite=True, asvelocity=False)
            else:
              ib=ia.regrid(outfile='__looloo', shape=tmpshp, axes=[0,1], csys=self.csys, overwrite=True, asvelocity=False)

            #dat=ib.getchunk()
            ib.done(verbose=False)
            ia.fromshape(outfile=outfile, shape=shp, csys=self.csys, overwrite=True)
            ##getchunk is a massive memory hog
            ###so going round in a funny fashion
            #arr=ia.getchunk()
            #for k in range(shp[2]):
            #    for j in range(shp[3]):
            #        if(len(dat.shape)==2):
            #            arr[:,:,k,j]=dat
            #        elif(len(dat.shape)==3):
            #            arr[:,:,k,j]=dat[:,:,0]
            #        else:
            #            arr[:,:,k,j]=dat[:,:,0,0]
            #ia.putchunk(arr)
            ia.calc(outfile+'[index3 in [0]]+__looloo') 
            ia.calcmask('mask(__looloo)')
            ia.done(verbose=False)
            ia.removefile('__looloo')
        else:
            ib=ia.regrid(outfile=outfile ,shape=shp, axes=[0,1], csys=self.csys, overwrite=True, asvelocity=False)
            ia.done(verbose=False)
            ib.done(verbose=False)


    def flatten(self,l):
        """
        A utility function to flatten nested lists 
        but allow nesting of [[elm1,elm2,elm3],[elm4,elm5],[elm6,elm7]]
        to handle multifield masks.
        This does not flatten if an element is a list of int or float. 
        And also leave empty list as is.
        """ 
        retlist = []
        l = list(l)
        #print 'l=',l
        for i in range(len(l)):
            #print "ith l=",i, l[i] 
            if isinstance(l[i],list) and l[i]:
                # and (not isinstance(l[i][0],(int,float))):
                #print "recursive l=",l
                if isinstance(l[i][0],list) and isinstance(l[i][0][0],list):
                   retlist.extend(self.flatten(l[i]))
                else:
                   retlist.append(l[i])
            else:
                retlist.append(l[i])
        return retlist 

    def getchanimage(self,cubeimage,outim,chan):
        """
        Create a slice of channel image from cubeimage

        Keyword arguments:
        cubeimage -- input image cube
        outim     -- output sliced image
        chan      -- nth channel
        """
        #pdb.set_trace()
        ia.open(cubeimage)
        modshape=ia.shape()
        if modshape[3]==1:
          return False
        if modshape[3]-1 < chan:
          return False
        blc=[0,0,modshape[2]-1,chan]
        trc=[modshape[0]-1,modshape[1]-1,modshape[2]-1,chan]
        sbim=ia.subimage(outfile=outim, region=rg.box(blc,trc), overwrite=True)
        sbim.close()
        ia.close()
        return True

    def putchanimage(self,cubimage,inim,chan):
        """
        Put channel image back to a pre-exisiting cubeimage
 
        Keyword arguments:
        cubimage -- image cube
        inim     -- input channel image 
        chan     -- nth channel
        """
        ia.open(inim)
        inimshape=ia.shape()
        imdata=ia.getchunk()
        immask=ia.getchunk(getmask=True)
        ia.close()
        blc=[0,0,inimshape[2]-1,chan]
        trc=[inimshape[0]-1,inimshape[1]-1,inimshape[2]-1,chan]
        ia.open(cubimage)
        cubeshape=ia.shape()
        if not (cubeshape[3] > (chan+inimshape[3]-1)):
            return False
        rg0=ia.setboxregion(blc=blc,trc=trc)
        if any(inimshape[0:3]!=cubeshape[0:3]): 
            return False
        #ia.putchunk(pixels=imdata,blc=blc)
        ia.putregion(pixels=imdata,pixelmask=immask, region=rg0)
        ia.close()
        return True

    def qatostring(self,q):
        """
        A utility function to return a quantity in string
        (currently only used in setChannelization which is deprecated)
        """
        if not q.has_key('unit'):
            raise TypeError, "Does not seems to be quantity"
        return str(q['value'])+q['unit']

    def convertvf(self,vf,frame,field,restf,veltype='radio'):
        """
        returns doppler(velocity) or frequency in string
        currently use first rest frequency
        Assume input vf (velocity or fequency in a string) and 
        output are the same 'frame'.
        """
        #pdb.set_trace()
        docalcf=False
        #if(frame==''): frame='LSRK' 
        #Use datasepcframe, it is cleanhelper initialized to set
        #to LSRK
        if(frame==''): frame=self.dataspecframe
        if(qa.quantity(vf)['unit'].find('m/s') > -1):
            docalcf=True
        elif(qa.quantity(vf)['unit'].find('Hz') > -1):
            docalcf=False
        else:
            if vf !=0:
                raise TypeError, "Unrecognized unit for the velocity or frequency parameter"
        ##fldinds=ms.msseltoindex(self.vis, field=field)['field'].tolist()
        fldinds=ms.msseltoindex(self.vis[self.sortedvisindx[0]], field=field)['field'].tolist()
        if(len(fldinds) == 0):
            fldid0=0
        else:
            fldid0=fldinds[0]
        if restf=='':
            #tb.open(self.vis+'/FIELD')
            fldtab=self.getsubtable(self.vis[self.sortedvisindx[0]],'FIELD')
            tb.open(fldtab)
            nfld = tb.nrows()
            if nfld >= fldid0:
              srcid=tb.getcell('SOURCE_ID',fldid0)
            else:
              raise TypeError, ("Cannot set REST_FREQUENCY from the data: "+
                  "no SOURCE corresponding field ID=%s, please supply restfreq" % fldid0)
            tb.close()
            # SOUECE_ID in FIELD table = -1 if no SOURCE table
            if srcid==-1:
                raise TypeError, "Rest frequency info is not supplied"
            #tb.open(self.vis+'/SOURCE')
            sourcetab=self.getsubtable(self.vis[self.sortedvisindx[0]], 'SOURCE')
            tb.open(sourcetab)
            tb2=tb.query('SOURCE_ID==%s' % srcid)
            tb.close()
            nsrc = tb2.nrows()
            if nsrc > 0:
              rfreq=tb2.getcell('REST_FREQUENCY',0)
            else:
              raise TypeError, ("Cannot set REST_FREQUENCY from the data: "+
                   " no SOURCE corresponding field ID=%s, please supply restfreq" % fldid0)
            tb2.close()
            if(rfreq<=0):
                raise TypeError, "Rest frequency does not seems to be properly set, check the data"
        else:
            if type(restf)==str: restf=[restf]
            if(qa.quantity(restf[0])['unit'].find('Hz') > -1):
                rfreq=[qa.convert(qa.quantity(restf[0]),'Hz')['value']] 
                #print "using user input rest freq=",rfreq
            else:
                raise TypeError, "Unrecognized unit or type for restfreq"
        if(vf==0):
            # assume just want to get a restfrequecy from the data
            ret=str(rfreq[0])+'Hz'
        else:
            if(docalcf):
                dop=me.doppler(veltype, qa.quantity(vf)) 
                rvf=me.tofrequency(frame, dop, qa.quantity(rfreq[0],'Hz'))
            else:
                frq=me.frequency(frame, qa.quantity(vf))
                rvf=me.todoppler(veltype, frq, qa.quantity(rfreq[0],'Hz')) 
            ret=str(rvf['m0']['value'])+rvf['m0']['unit']
        return ret 


    def getfreqs(self,nchan,spw,start,width, dummy=False):
        """
        (not in used - currently commented out in its caller, initChaniter()) 
        returns a list of frequencies to be used in output clean image
        if width = -1, start is actually end (max) freq 
        """
        #pdb.set_trace()
        freqlist=[]
        finc=1
        loc_nchan=0

        if spw in (-1, '-1', '*', '', ' '):
            spwinds = -1
        else:
            #spwinds=ms.msseltoindex(self.vis, spw=spw)['spw'].tolist()
            spwinds=ms.msseltoindex(self.vis[self.sortedvisindx[0]], spw=spw)['spw'].tolist()
            if(len(spwinds) == 0):
                spwinds = -1

        if(spwinds==-1):
            # first row
            spw0=0
        else:
            spw0=spwinds[0]
        #tb.open(self.vis+'/SPECTRAL_WINDOW')
        spectable=self.getsubtable(self.vis[self.sortedvisindx[0]], "SPECTRAL_WINDOW")
        tb.open(spectable)
        chanfreqscol=tb.getvarcol('CHAN_FREQ')
        chanwidcol=tb.getvarcol('CHAN_WIDTH')
        spwframe=tb.getcol('MEAS_FREQ_REF');
        tb.close()
        # assume spw[0]  
        elspecframe=["REST",
                     "LSRK",
                     "LSRD",
                     "BARY",
                     "GEO",	    
                     "TOPO",
                     "GALACTO",
                     "LGROUP",
                     "CMB"]
        self.dataspecframe=elspecframe[spwframe[spw0]];
        if(dummy):
            return freqlist, finc
        #DP extract array from dictionary returned by getvarcol
        chanfreqs1dx = numpy.array([])
        chanfreqs=chanfreqscol['r'+str(spw0+1)].transpose()
        chanfreqs1dx = chanfreqs[0]
        if(spwinds!=-1):
            for ispw in range(1,len(spwinds)):
                chanfreqs=chanfreqscol['r'+str(spwinds[ispw]+1)].transpose()            
                chanfreqs1dx = numpy.concatenate((chanfreqs1dx, chanfreqs[0]))
        chanfreqs1d = chanfreqs1dx.flatten()        
        #RI this is woefully inadequate assuming the first chan's width
        #applies to everything selected, but we're going to replace all
        #this with MSSelect..
        chanwids=chanwidcol['r'+str(spw0+1)].transpose()
        chanfreqwidth=chanwids[0][0]
        
        if(type(start)==int or type(start)==float):
            if(start > len(chanfreqs1d)):
                raise TypeError, "Start channel is outside the data range"
            startf = chanfreqs1d[start]
        elif(type(start)==str):
            if(qa.quantity(start)['unit'].find('Hz') > -1):
                startf=qa.convert(qa.quantity(start),'Hz')['value']
            else:
                raise TypeError, "Unrecognized start parameter"
        if(type(width)==int or type(width)==float):
            if(type(start)==int or type(start)==float):
                #finc=(chanfreqs1d[start+1]-chanfreqs1d[start])*width
                finc=(chanfreqwidth)*width
                # still need to convert to target reference frame!
            elif(type(start)==str):
                if(qa.quantity(start)['unit'].find('Hz') > -1):
                   # assume called from setChannelization with local width=1
                   # for the default width(of clean task parameter)='' for
                   # velocity and frequency modes. This case set width to 
                   # first channel width (for freq) and last one (for vel) 
                   if width==-1:
                       finc=chanfreqs1d[-1]-chanfreqs1d[-2]
                   else:
                       finc=chanfreqs1d[1]-chanfreqs1d[0]

                   # still need to convert to target reference frame!
        elif(type(width)==str):
            if(qa.quantity(width)['unit'].find('Hz') > -1):
                finc=qa.convert(qa.quantity(width),'Hz')['value']
        if(nchan ==-1):
            if(qa.quantity(start)['unit'].find('Hz') > -1):
                if width==-1: # must be in velocity order (i.e. startf is max)
                    bw=startf-chanfreqs1d[0]
                else:
                    bw=chanfreqs1d[-1]-startf
            else:
                bw=chanfreqs1d[-1]-chanfreqs1d[start]
            if(bw < 0):
                raise TypeError, "Start parameter is outside the data range"
            if(qa.quantity(width)['unit'].find('Hz') > -1):
                qnchan=qa.convert(qa.div(qa.quantity(bw,'Hz'),qa.quantity(width)))
                #DP loc_nchan=int(math.ceil(qnchan['value']))+1
                loc_nchan=int(round(qnchan['value']))+1
            else:
                #DP loc_nchan=int(math.ceil(bw/finc))+1
                loc_nchan=int(round(bw/finc))+1
        else:
            loc_nchan=nchan
        for i in range(int(loc_nchan)):
            if(i==0): 
                freqlist.append(startf)
            else:
                freqlist.append(freqlist[-1]+finc) 
        return freqlist, finc


    def setChannelizeDefault(self,mode,spw,field,nchan,start,width,frame,veltype,phasec, restf,obstime=''):
        """
        Determine appropriate values for channelization
        parameters when default values are used
        for mode='velocity' or 'frequency' or 'channel'
        This makes use of ms.cvelfreqs.
        """
        ###############
        # for debugging
        ###############
        debug=False
        ###############
        spectable=self.getsubtable(self.vis[self.sortedvisindx[0]], "SPECTRAL_WINDOW")
        tb.open(spectable)
        chanfreqscol=tb.getvarcol('CHAN_FREQ')
        chanwidcol=tb.getvarcol('CHAN_WIDTH')
        spwframe=tb.getcol('MEAS_FREQ_REF');
        tb.close()
        # first parse spw parameter:
        # use MSSelect if possible
        if len(self.sortedvislist) > 0:
          invis = self.sortedvislist[0]
          inspw = self.vis.index(self.sortedvislist[0])
        else:
          invis = self.vis[0]
          inspw = 0
        ms.open(invis)
        if type(spw)==list:
          spw=spw[inspw]
        if spw in ('-1', '*', '', ' '):
          spw='*'
        if field=='':
          field='*'
        mssel=ms.msseltoindex(vis=invis, spw=spw, field=field)
        selspw=mssel['spw']
        selfield=mssel['field']
        chaninds=mssel['channel'].tolist()
        chanst0 = chaninds[0][1]

        # frame
        spw0=selspw[0]
        chanfreqs=chanfreqscol['r'+str(spw0+1)].transpose()[0]
        chanres = chanwidcol['r'+str(spw0+1)].transpose()[0]

        # ascending or desending data frequencies?
        # based on selected first spw's first CHANNEL WIDTH 
        # ==> some MS data may have positive chan width
        # so changed to look at first two channels of chanfreq (TT)
        #if chanres[0] < 0:
        descending = False
        if len(chanfreqs) > 1 :
          if chanfreqs[1]-chanfreqs[0] < 0:
            descending = True        
        else:
          if chanres[0] < 0:
            descending = True

        # set dataspecframe:
        elspecframe=["REST",
                     "LSRK",
                     "LSRD",
                     "BARY",
                     "GEO",
                     "TOPO",
                     "GALACTO",
                     "LGROUP",
                     "CMB"]
        self.dataspecframe=elspecframe[spwframe[spw0]];

        # set usespecframe:  user's frame if set, otherwise data's frame
        if(frame != ''):
            self.usespecframe=frame
            self.inframe=True
        else:
            self.usespecframe=self.dataspecframe

        # some start and width default handling
        if mode!='channel':
          if width==1:
             width=''
          if start==0:
             start=''

        #get restfreq
        if restf=='':
          fldtab=self.getsubtable(invis,'FIELD')
          tb.open(fldtab)
          nfld=tb.nrows()
          try:
            if nfld >= selfield[0]:
              srcid=tb.getcell('SOURCE_ID',selfield[0])
            else:
              if mode=='velocity':
                raise TypeError, ("Cannot set REST_FREQUENCY from the data: "+
                  "no SOURCE corresponding field ID=%s, please supply restfreq" % selfield[0])
          finally:
            tb.close()
          #SOUECE_ID in FIELD table = -1 if no SOURCE table
          if srcid==-1:
            if mode=='velocity':
              raise TypeError, "Rest frequency info is not supplied"
          try:
            srctab=self.getsubtable(invis, 'SOURCE')
            tb.open(srctab)
            tb2=tb.query('SOURCE_ID==%s' % srcid)
            nsrc = tb2.nrows()
            if nsrc > 0 and tb2.iscelldefined('REST_FREQUENCY',0):
              rfqs = tb2.getcell('REST_FREQUENCY',0)
              if len(rfqs)>0:  
                restf=str(rfqs[0])+'Hz'
              else:
                if mode=='velocity':  
                  raise TypeError, ("Cannot set REST_FREQUENCY from the data: "+
                    "REST_FREQUENCY entry for ID %s in SOURCE table is empty, please supply restfreq" % srcid)    
            else:
              if mode=='velocity':
                raise TypeError, ("Cannot set REST_FREQUENCY from the data: "+
                 "no SOURCE corresponding field ID=%s, please supply restfreq" % selfield[0])
          finally:
            tb.close()
            tb2.close()

        if type(phasec)==list:
           inphasec=phasec[0]
        else:
           inphasec=phasec
        if type(inphasec)==str and inphasec.isdigit():
          inphasec=int(inphasec)
        #if nchan==1:
          # use data chan freqs
        #  newfreqs=chanfreqs
        #else:
          # obstime not included here
        if debug: print "before ms.cvelfreqs (start,width,nchan)===>",start, width, nchan
        newfreqs=ms.cvelfreqs(spwids=selspw,fieldids=selfield,mode=mode,nchan=nchan,
                              start=start,width=width,phasec=inphasec, restfreq=restf,
                              outframe=self.usespecframe,veltype=veltype).tolist()
        #print newfreqs
        descendingnewfreqs=False
        if len(newfreqs)>1:
          if newfreqs[1]-newfreqs[0] < 0:
            descendingnewfreqs=True
        if debug: print "Mode, Start, width after cvelfreqs =",mode, start,width 
        if type(newfreqs)==list and len(newfreqs) ==0:
          raise TypeError, ("Output frequency grid cannot be calculated: "+
                 " please check start and width parameters")
        if debug:
          if len(newfreqs)>1:
            print "FRAME=",self.usespecframe
            print "newfreqs[0]===>",newfreqs[0]
            print "newfreqs[1]===>",newfreqs[1]
            print "newfreqs[-1]===>",newfreqs[-1]
            print "len(newfreqs)===>",len(newfreqs)
          else:
            print "newfreqs=",newfreqs
        ms.close()

        # set output number of channels
        if nchan ==1:
          retnchan=1
        else:
          if len(newfreqs)>1:
            retnchan=len(newfreqs)
          else:
            retnchan=nchan
            newfreqs=chanfreqs

        # set start parameter
        # first analyze data order etc
        reverse=False
        negativew=False
        if descending:
          # channel mode case (width always >0) 
          if width!="" and (type(width)==int or type(width)==float):
            if descendingnewfreqs:
              reverse=False 
            else:
              reverse=True
          elif width=="": #default width
            if descendingnewfreqs and mode=="frequency":
              reverse=False
            else:
              reverse=True
               
          elif type(width)==str:
            if width.lstrip().find('-')==0:
              negativew=True
            if descendingnewfreqs:
              if negativew: 
                reverse=False
              else:
                reverse=True
            else:
              if negativew:
                reverse=True
              else:
                reverse=False
        else: #ascending data
          # depends on sign of width only
          # with CAS-3117 latest change(rev.15179), velocity start
          # means lowest velocity for default width
          if width=="" and mode=="velocity": #default width
              # ms.cvelfreqs returns correct order so no reversing
              reverse=False
          elif type(width)==str:
            if width.lstrip().find('-')==0:
                reverse=True
            else:
                reverse=False

        if reverse:
           newfreqs.reverse()
        #if (start!="" and mode=='channel') or \
        #   (start!="" and type(start)!=int and mode!='channel'):
        # for now to avoid inconsistency later in imagecoordinates2 call
        # user's start parameter is preserved for channel mode only.
        # (i.e. the current code may adjust start parameter for other modes but
        # this probably needs to be changed, especially for multiple ms handling.)
        if (start!="" and mode=='channel'):
          retstart=start
        else:
          # default cases
          if mode=="frequency":
            retstart=str(newfreqs[0])+'Hz'
          elif mode=="velocity":
            #startfreq=str(newfreqs[-1])+'Hz'
            startfreq=(str(max(newfreqs))+'Hz') if(start=="") else  (str(newfreqs[-1])+'Hz')
            retstart=self.convertvf(startfreq,frame,field,restf,veltype)
          elif mode=="channel":
            # default start case, use channel selection from spw
            retstart=chanst0
        
        # set width parameter
        if width!="":
          retwidth=width
        else:
          if nchan==1:
            finc = chanres[0]
          else:
            finc = newfreqs[1]-newfreqs[0]
            if debug: print "finc(newfreqs1-newfreqs0)=",finc
          if mode=="frequency":
            # It seems that this is no longer necessary... TT 2013-08-12
            #if descendingnewfreqs:
            #  finc = -finc
            retwidth=str(finc)+'Hz'
          elif mode=="velocity":
            # for default width assume it is vel<0 (incresing in freq)
            if descendingnewfreqs:
              ind1=-2
              ind0=-1
            else:
              ind1=-1
              ind0=-2
            v1 = self.convertvf(str(newfreqs[ind1])+'Hz',frame,field,restf,veltype=veltype)
            v0 = self.convertvf(str(newfreqs[ind0])+'Hz',frame,field,restf,veltype=veltype)
            ##v1 = self.convertvf(str(newfreqs[-1])+'Hz',frame,field,restf,veltype=veltype)
            ##v0 = self.convertvf(str(newfreqs[-2])+'Hz',frame,field,restf,veltype=veltype)
            #v1 = self.convertvf(str(newfreqs[1])+'Hz',frame,field,restf,veltype=veltype)
            #v0 = self.convertvf(str(newfreqs[0])+'Hz',frame,field,restf,veltype=veltype)
            if(qa.lt(v0, v1) and start==""):
                ###user used "" as start make sure step is +ve in vel as start is min vel possible for freqs selected
                retwidth=qa.tos(qa.sub(v1, v0))
            else:
                retwidth = qa.tos(qa.sub(v0, v1))
          else:
            retwidth=1
          if debug: print "setChan retwidth=",retwidth
        return retnchan, retstart, retwidth

    def setChannelizeNonDefault(self, mode,spw,field,nchan,start,width,frame,
                                veltype,phasec, restf):
        """
        Determine appropriate values for channelization
        parameters when default values are used
        for mode='velocity' or 'frequency' or 'channel'
        This does not replaces setChannelization and make no use of ms.cvelfreqs.
        """
    
    
        #spw='0:1~4^2;10~12, ,1~3:3~10^3,4~6,*:7'
        #vis='ngc5921/ngc5921.demo.ms'
    
        if type(spw)!=str:
            spw=''
    
        if spw.strip()=='':
            spw='*'
    
        freqs=set()
        wset=[]
        chunk=spw.split(',')
        for i in xrange(len(chunk)):
            #print chunk[i], '------'
            ck=chunk[i].strip()
            if len(ck)==0:
                continue
    
            wc=ck.split(':')
            window=wc[0].strip()
    
            if len(wc)==2:
                sec=wc[1].split(';')
                for k in xrange(len(sec)):
                    chans=sec[k].strip()
                    sep=chans.split('^')
                    se=sep[0].strip()
                    t=1
                    if len(sep)==2:
                        t=sep[1].strip()
                    se=se.split('~')
                    s=se[0].strip()
                    if len(se)==2:
                        e=se[1].strip() 
                    else:
                        e=-1
                    wd=window.split('~')
                    if len(wd)==2:
                        wds=int(wd[0])
                        wde=int(wd[1])
                        for l in range(wds, wde):
                            #print l, s, e, t
                            wset.append([l, s, e, t])
                    else:
                        #print wd[0], s, e, t
                        if e==-1:
                            try:
                                e=int(s)+1
                            except:
                                e=s
                        wset.append([wd[0], s, e, t])
            else:
                win=window.split('~')
                if len(win)==2:
                    wds=int(win[0])
                    wde=int(win[1])
                    for l in range(wds, wde):
                        #print l, 0, -1, 1
                        wset.append([l, 0, -1, 1])
                else:
                    #print win[0], 0, -1, 1
                    wset.append([win[0], 0, -1, 1])
    
        #print wset
        for i in range(len(wset)):
            for j in range(4):
                try:
                    wset[i][j]=int(wset[i][j])
                except:
                    wset[i][j]=-1
        #print wset
        spectable=self.getsubtable(self.vis[self.sortedvisindx[0]], "SPECTRAL_WINDOW")
        tb.open(spectable)
        nr=tb.nrows()
        for i in range(len(wset)):
            if wset[i][0]==-1:
                w=range(nr)
            elif wset[i][0]<nr:
                w=[wset[i][0]]
            else:
                w=range(0)
            for j in w:
                chanfreqs=tb.getcell('CHAN_FREQ', j)
                if wset[i][2]==-1:
                    wset[i][2]=len(chanfreqs)
                if wset[i][2]>len(chanfreqs):
                    wset[i][2]=len(chanfreqs)
                #print wset[i][1], wset[i][2], len(chanfreqs), wset[i][3]
                for k in range(wset[i][1], wset[i][2], wset[i][3]):
                    #print k
                    freqs.add(chanfreqs[k]) 
        tb.close()
        freqs=list(freqs)
        freqs.sort()
        #print freqs[0], freqs[-1]
    
        if mode=='channel':
            star=0
            if type(start)==str:
               try:
                   star=int(start)
               except:
                   star=0
            if type(start)==int:
                star=start
            if star>len(freqs) or star<0:
                star=0
 
            if nchan==-1:
                nchan=len(freqs)
    
            widt=1
            if type(width)==str:
               try:
                   widt=int(width)
               except:
                   widt=1
            if type(width)==int:
               widt=width
            if widt==0:
                widt=1
            if widt>0:
                nchan=max(min(int((len(freqs)-star)/widt), nchan), 1)
            else:
                nchan=max(min(int((-star)/widt), nchan), 1)
                widt=-widt
                star=max(star-nchan*widt, 0)
       
        if mode=='frequency':
            star=freqs[0]
            if type(start)!=str:
                star=freqs[0]
            else:
                star=max(qa.quantity(start)['value'], freqs[0])

            if nchan==-1:
                nchan=len(freqs)
    
            widt=freqs[-1]
            if len(freqs)>1:
                for k in range(len(freqs)-1):
                    widt=min(widt, freqs[k+1]-freqs[k])
            if type(width)==str and width.strip()!='':
                widt=qa.quantity(width)['value']
    
            if widt>0:
                #print star, widt, (freqs[-1]-star)/widt
                nchan=max(min(int((freqs[-1]-star)/widt), nchan), 1)
            else:
                nchan=max(min(int(freqs[0]-star)/widt, nchan), 1)
                widt=-widt
                star=max(star-nchan*widt, freqs[0])
    
            widt=str(widt)+'Hz'
            star=str(star)+'Hz'
    
        if mode=='velocity':
            beg1=self.convertvf(str(freqs[0])+'Hz',frame,field,restf,veltype=veltype)
            beg1=qa.quantity(beg1)['value']
            end0=self.convertvf(str(freqs[-1])+'Hz',frame,field,restf,veltype=veltype)
            end0=qa.quantity(end0)['value']
            star=beg1
            if type(start)==str and start.strip()!='':
                star=min(qa.quantity(start)['value'], star)
                star=min(star, end0)
            
            #print beg1, star, end0

            widt=-end0+beg1
            if len(freqs)>1:
                for k in range(len(freqs)-1):
                    st=self.convertvf(str(freqs[k])+'Hz',frame,field,restf,veltype=veltype)
                    en=self.convertvf(str(freqs[k+1])+'Hz',frame,field,restf,veltype=veltype)
                    widt=min(widt, qa.quantity(en)['value']-qa.quantity(st)['value'])
                widt=-abs(widt)

            if type(width)==str and width.strip()!='':
                widt=qa.quantity(width)['value']

            #print widt
            if widt>0:
                nchan=max(min(int((beg1-star)/widt), nchan), 1)
                #star=0
            else:
                nchan=max(min(int((end0-star)/widt), nchan), 1)
                #widt=-widt

            widt=str(widt)+'m/s'
            star=str(star)+'m/s'
    
        return nchan, star, widt

    def convertframe(self,fin,frame,field):
        """
        (not in use: its caller is setChannelization...)
        convert freq frame in dataframe to specfied frame, assume fin in Hz
        retruns converted freq in Hz (value only)
        """
        # assume set to phasecenter before initChanelization is called
        pc=self.srcdir
        if(type(pc)==str):
            if (pc==''):
                fieldused = field
                if (fieldused ==''):
                    fieldused ='0'
                #dir = int(ms.msseltoindex(self.vis,field=fieldused)['field'][0])
                dir = int(ms.msseltoindex(self.vis[self.sortedvisindx[0]],field=fieldused)['field'][0])
            else:
                tmpdir = phasecenter
                try:
                    #if(len(ms.msseltoindex(self.vis, field=pc)['field']) > 0):
                    if(len(ms.msseltoindex(self.vis[self.sortedvisindx[0]], field=pc)['field']) > 0):
                        #tmpdir  = int(ms.msseltoindex(self.vis,field=pc)['field'][0])
                        tmpdir  = int(ms.msseltoindex(self.vis[self.sortedvisindx[0]],field=pc)['field'][0])
                except Exception, instance:
                    tmpdir = pc
                dir = tmpdir
        if type(dir)==str:
            try:
                mrf, ra, dec = dir.split()
            except Exception, instance:
                raise TypeError, "Error in a string format  for phasecenter"
            mdir = me.direction(mrf, ra, dec)
        else:
            #tb.open(self.vis+'/FIELD')
            fldtab=self.getsubtable(self.vis[self.sortedvisindx[0]],'FIELD')
            tb.open(fldtab)
            srcdir=tb.getcell('DELAY_DIR',dir)
            mrf=tb.getcolkeywords('DELAY_DIR')['MEASINFO']['Ref']
            tb.close()
            mdir = me.direction(mrf,str(srcdir[0][0])+'rad',str(srcdir[1][0])+'rad')
            #tb.open(self.vis+'/OBSERVATION')
            obstab=self.getsubtable(self.vis[self.sortedvisindx[0]],'OBSERVATION')
            tb.open(obstab)
        telname=tb.getcell('TELESCOPE_NAME',0)
        # use time in main table instead?
        tmr=tb.getcell('TIME_RANGE',0)
        tb.close()
        #print "direction=", me.direction(mrf,str(srcdir[0][0])+'rad',str(srcdir[1][0])+'rad')
        #print "tmr[1]=",tmr[1]
        #print "epoch=", me.epoch('utc',qa.convert(qa.quantity(str(tmr[1])+'s'),'d'))
        me.doframe(me.epoch('utc',qa.convert(qa.quantity(str(tmr[0])+'s'),'d')))
        me.doframe(me.observatory(telname))
        me.doframe(mdir)
        f0 = me.frequency(self.dataspecframe, str(fin)+'Hz')
        #print "frame=", frame, ' f0=',f0
        fout = me.measure(f0,frame)['m0']['value']
        return fout

    def setspecframe(self,spw):
        """
        set spectral frame for mfs to data frame based
        on spw selection 
        (part copied from setChannelization)
        """
        #tb.open(self.vis+'/SPECTRAL_WINDOW')
        spectable=self.getsubtable(self.vis[self.sortedvisindx[0]], "SPECTRAL_WINDOW")
        tb.open(spectable)
        spwframe=tb.getcol('MEAS_FREQ_REF');
        tb.close()

        # first parse spw parameter:

        # use MSSelect if possible
        if type(spw)==list:
          spw=spw[self.sortedvisindx[0]]

        if spw in (-1, '-1', '*', '', ' '):
            spw="*"

        sel=ms.msseltoindex(self.vis[self.sortedvisindx[0]], spw=spw)
        # spw returned by msseletoindex, spw='0:5~10;10~20' 
        # will give spw=[0] and len(spw) not equal to len(chanids)
        # so get spwids from chaninds instead.
        chaninds=sel['channel'].tolist()
        spwinds=[]
        for k in range(len(chaninds)):
            spwinds.append(chaninds[k][0])
        if(len(spwinds) == 0):
            raise Exception, 'unable to parse spw parameter '+spw;
            
        # the first selected spw 
        spw0=spwinds[0]

        # set dataspecframe:
        elspecframe=["REST",
                     "LSRK",
                     "LSRD",
                     "BARY",
                     "GEO",	    
                     "TOPO",
                     "GALACTO",
                     "LGROUP",
                     "CMB"]
        self.dataspecframe=elspecframe[spwframe[spw0]];
        return 

    def initChaniter(self,nchan,spw,start,width,imagename,mode,tmpdir='_tmpimdir/'):
        """
        initialize for channel iteration in interactive clean
        --- create a temporary directory, get frequencies for
        mode='channel'
       
        Keyword arguments:
        nchan -- no. channels
        spw   -- spw 
        start -- start modified after channelization function 
        width -- width modified after channelization function
        imagename -- from task input 
        mode  -- from task input
        tmpdir -- temporary directory name to store channel images
        
        returns: 
        frequencies in a list
        frequency increment
        newmode -- force to set mode to frequency
        tmppath -- path for the temporary directory
        """
        # create a temporary directory to put channel images
        tmppath=[]
        freqs=[]
        finc=0
        newmode=mode
        for imname in imagename:
            if os.path.dirname(imname)=='':
                tmppath.append(tmpdir)
            else:
                tmppath.append(os.path.dirname(imname)+'/'+tmpdir)
            # clean up old directory
            if os.path.isdir(tmppath[-1]):
                shutil.rmtree(tmppath[-1])
            os.mkdir(tmppath[-1])
        #internally converted to frequency mode for mode='channel'
        #to ensure correct frequency axis for output image
        #if mode == 'channel':
        #    freqs, finc = self.getfreqs(nchan, spw, start, width)
        #    newmode = 'frequency'
        if mode == 'channel':
            # get spectral axis info from the dirty image
            ia.open(imagename[0]+'.image')
            imcsys=ia.coordsys().torecord()
            ia.close()
            # for optical velocity mode, the image will be in tabular form.
            if imcsys['spectral2'].has_key('tabular'):
              key='tabular'
            else:
              key='wcs'
            cdelt=imcsys['spectral2'][key]['cdelt']
            crval=imcsys['spectral2'][key]['crval']
            #cdelt=imcsys['spectral2']['wcs']['cdelt']
            #crval=imcsys['spectral2']['wcs']['crval']
            for i in range(nchan):
                if i==0: freqs.append(crval)
                freqs.append(freqs[-1]+cdelt)
            finc = cdelt
            newmode = 'frequency'
        return freqs,finc,newmode,tmppath


    def makeTemplateCubes(self, imagename,outlierfile, field, spw, selectdata, timerange,
          uvrange, antenna, scan, observation, intent, mode, facets, cfcache, interpolation, 
          imagermode, localFTMachine, mosweight, locnchan, locstart, locwidth, outframe,
          veltype, imsize, cell, phasecenter, restfreq, stokes, weighting,
          robust, uvtaper, outertaper, innertaper, modelimage, restoringbeam,
          usescratch, noise, npixels, padding):
        """
        make template cubes to be used for chaniter=T interactive clean
        """
        imageids=[]
        imsizes=[]
        phasecenters=[]
        rootname=''
        multifield=False
        loc_modelimage=modelimage
        newformat=False

        if len(outlierfile) != 0:
            f_imageids,f_imsizes,f_phasecenters,f_masks,f_modelimages,parms,newformat=self.newreadoutlier(outlierfile)
            if type(imagename) == list or newformat:
                rootname = ''
            else:
                rootname = imagename

            # combine with the task parameter input
            if type(imagename) == str:
                if newformat:
                    imageids.append(imagename)
                    imsizes.append(imsize)
                    phasecenters.append(phasecenter)
            else:
                imageids=imagename
                imsizes=imsize
                phasecenters=phasecenter

            #if type(mask) !=  list:
            #    mask=[mask]
            #elif type(mask[0]) != list:
            #    mask=[mask]
            if type(loc_modelimage) != list:
                loc_modelimage=[loc_modelimage]

            #elif type(loc_modelimage[0]) != list and type(imagename) != str:
            #if type(loc_modelimage[0]) != list and \
            #    (type(imagename) != str or (type(imageids)==list and len(imageids)=1)):
            #    loc_modelimage=[loc_modelimage]
            if type(loc_modelimage[0]) != list:
                loc_modelimage=[loc_modelimage]

            # now append readoutlier content
            for indx, name in enumerate(f_imageids):
                imageids.append(name)
                imsizes.append(f_imsizes[indx])
                phasecenters.append(f_phasecenters[indx])
               
                if newformat:
                    #mask.append(f_masks[indx])
                    loc_modelimage.append([f_modelimages[indx]])
                else:
                    if indx!=0:
                        loc_modelimage.append([f_modelimages[indx]])

            ##if len(imageids) > 1:
            #    multifield=True
        else:
            imsizes=imsize
            phasecenters=phasecenter
            imageids=imagename
               
        if len(imageids) > 1:
            multifield=True

        self.imageids=imageids
        # readoutlier need to be run first....
        self.datsel(field=field, spw=spw, timerange=timerange, uvrange=uvrange, 
                    antenna=antenna,scan=scan, observation=observation, intent=intent, 
                    usescratch=usescratch,
                    nchan=-1, start=0, width=1)

        self.definemultiimages(rootname=rootname,imsizes=imsizes,cell=cell,
                                stokes=stokes,mode=mode,
                               spw=spw, nchan=locnchan, start=locstart,
                               width=locwidth, restfreq=restfreq,
                               field=field, phasecenters=phasecenters,
                               names=imageids, facets=facets,
                               outframe=outframe, veltype=veltype,
                               makepbim=False, checkpsf=False)

        self.datweightfilter(field=field, spw=spw, timerange=timerange, 
                             uvrange=uvrange, antenna=antenna,scan=scan,
                             wgttype=weighting, robust=robust, noise=noise, 
                             npixels=npixels, mosweight=mosweight,
                             uvtaper=uvtaper, innertaper=innertaper, outertaper=outertaper, 
                             usescratch=usescratch, nchan=-1, start=0, width=1)
        # split this 
        #self.datselweightfilter(field=field, spw=spw,
        #                         timerange=timerange, uvrange=uvrange,
        #                         antenna=antenna, scan=scan,
        #                         wgttype=weighting, robust=robust,
        #                         noise=noise, npixels=npixels,
        #                         mosweight=mosweight,
        #                         innertaper=innertaper,
        #                         outertaper=outertaper,
        #                         calready=calready, nchan=-1,
        #                         start=0, width=1)
       
        #localAlgorithm = getAlgorithm(psfmode, imagermode, gridmode, mode,
        #                             multiscale, multifield, facets, nterms,
        #                             'clark');

        #localAlgorithm = 'clark'
        #print "localAlogrithm=",localAlgorithm

        #self.im.setoptions(ftmachine=localFTMachine,
        #                     wprojplanes=wprojplanes,
        #                     freqinterp=interpolation, padding=padding,
        #                     cfcachedirname=cfcache, pastep=painc,
        #                     epjtablename=epjtable,
        #                     applypointingoffsets=False,
        #                     dopbgriddingcorrections=True)
        self.im.setoptions(ftmachine=localFTMachine,
                             freqinterp=interpolation, padding=padding,
                             cfcachedirname=cfcache)

        modelimages=[]
        restoredimage=[]
        residualimage=[]
        psfimage=[]
        fluximage=[]
        for k in range(len(self.imagelist)):
            ia.open(self.imagelist[k])
            #if (modelimage =='' or modelimage==[]) and multifield:
            #    ia.rename(self.imagelist[k]+'.model',overwrite=True)
            #else:
            #    ia.remove(verbose=False)
            if ((loc_modelimage =='' or loc_modelimage==[]) or \
                (type(loc_modelimage)==list and \
                 (loc_modelimage[k]=='' or loc_modelimage[k]==[''] or loc_modelimage[k]==[]))) and multifield:
                ia.rename(self.imagelist[k]+'.model',overwrite=True)
            else:
                modlist=[]
                if type(modelimage)==str:
                    modlist=[modelimage]
                # make sure input model image is not removed
                if (not any([inmodel == self.imagelist[k] for inmodel in modlist])) and \
                    (not any([inmodel == self.imagelist[k]+'.model' for inmodel in modlist])):
                #    ia.remove(verbose=False)
                     ia.rename(self.imagelist[k]+'.model',overwrite=True)
            ia.close()

            modelimages.append(self.imagelist[k]+'.model')
            restoredimage.append(self.imagelist[k]+'.image')
            residualimage.append(self.imagelist[k]+'.residual')
            psfimage.append(self.imagelist[k]+'.psf')
            if(imagermode=='mosaic'):
                fluximage.append(self.imagelist[k]+'.flux')

        # make dirty image cube
        if multifield:
           alg='mfclark'
        else:
           alg='clark'
       
        self.im.clean(algorithm=alg, niter=0,
                   model=modelimages, residual=residualimage,
                   image=restoredimage, psfimage=psfimage,
                   mask='', interactive=False)


    def setChaniterParms(self,finalimagename, spw,chan,start,width,freqs,finc,tmppath):
        """
        Set parameters for channel by channel iterations
        returns:
        start and width to define each channel image plane
        """
        retparms={}
        self.maskimages={}
        retparms['imagename']=[tmppath[indx]+os.path.basename(imn)+'.ch'+str(chan)
                   for indx, imn in enumerate(finalimagename)]

        #print "Processing channel %s " % chan
        #self._casalog.post("Processing channel %s "% chan)

        # Select only subset of vis data if possible.
        # It does not work well for multi-spw so need
        # to select with nchan=-1
        retparms['imnchan']=1
        retparms['chanslice']=chan
        qat=casac.quanta()
        q = qat.quantity

        # 2010-08-18 note: disable this. Has the problem 
        # getting imaging weights correctly when the beginning 
        # channels were flagged.
        #if type(spw)==int or len(spw)==1:
        #    if width>1:
        #        visnchan=width
        #    else:
        #        visnchan=1
        #else:
        #    visnchan=-1

        visnchan=-1
        retparms['visnchan']=visnchan
        visstart=0

        if type(start)==int:
            # need to convert to frequencies
            # to ensure correct frequencies in
            # output images(especially for multi-spw)
            # Use freq list instead generated in initChaniter
            imstart=q(freqs[chan],'Hz')
            width=q(finc,'Hz')
        elif start.find('m/s')>0:
            imstart=qat.add(q(start),qat.mul(chan,q(width)))
        elif start.find('Hz')>0:
            imstart=qat.add(q(start),qat.mul(chan,q(width)))
        retparms['width']=width
        retparms['imstart']=imstart
        retparms['visstart']=visstart

        #
        return retparms

    def defineChaniterModelimages(self,modelimage,chan,tmppath):
        """
        chaniter=T specific function to convert input models 
        to a model image 
        """
        chanmodimg=[]
        if type(modelimage)==str:
            modelimage=[modelimage]
        indx=0
        for modimg in modelimage:
            if modimg=='':
                return 
            if type(modimg)==list:
                chanmodimg=[]
                for img in modimg:
                    if img!='':
                        if os.path.dirname(img) != '':
                            chanmodimg.append(tmppath[0] + '_tmp.' +
                                              os.path.basename(img))
                        else:
                            chanmodimg.append(tmppath[0] + '_tmp.' + img)
                        self.getchanimage(cubeimage=img, outim=chanmodimg[-1], chan=chan)
                #self.convertmodelimage(modelimages=chanmodimg,
                #                        outputmodel=self.imagelist.values()[0]+'.model')
                self.convertmodelimage(modelimages=chanmodimg,
                                        outputmodel=self.imagelist.values()[indx]+'.model', imindex=indx)
                chanmodimg=[]
                indx+=1
            else:
                if os.path.dirname(modimg) != '':
                    chanmodimg.append(tmppath[0] + '_tmp.' + os.path.basename(modimg))
                else:
                    chanmodimg.append(tmppath[0] + '_tmp.' + modimg)
                self.getchanimage(cubeimage=modimg, outim=chanmodimg[-1],chan=chan)

                #self.convertmodelimage(modelimages=chanmodimg,
                #                        outputmodel=self.imagelist.values()[0]+'.model')
                self.convertmodelimage(modelimages=chanmodimg,
                                        outputmodel=self.imagelist.values()[indx]+'.model',imindex=indx)
            # clean up temporary channel model image
            self.cleanupTempFiles(chanmodimg)

    def convertAllModelImages_old(self,modelimage, mode, nterms, dochaniter, chan, tmppath):
        """
        wrapper function for convertmodelimage for all different cases
        """
        if (type(modelimage)!=str and type(modelimage)!=list):
                    raise Exception,'modelimage must be a string or a list of strings';
        #spectralline modes
        if (not mode=='mfs') or (mode=='mfs' and nterms==1):
            if (not all(img=='' or img==[] or img==[''] for img in modelimage)):
                if dochaniter:
                    self.defineChaniterModelimages(modelimage,chan,tmppath)
                else:
                    if type(modelimage)== str or \
                       (type(modelimage)==list and len(self.imagelist)==1 and len(modelimage)>1):
                        modelimage=[modelimage]
                    
                    #print "Run convertmodelimage for this list : ", self.imagelist, " with these models : ", modelimage;
                    for j in range(len(self.imagelist)):
                        self._casalog.post("Use modelimages: "+str(modelimage[j])+" to create a combined modelimage: " \
                                           +self.imagelist.values()[j]+".model", 'DEBUG1')
                        if modelimage[j] != '' and modelimage[j] != []:
                            self.convertmodelimage(modelimages=modelimage[j],
                                    outputmodel=self.imagelist.values()[j]+'.model',imindex=j)

        # elif .......
        # put mfs with nterms>1 case here

    ##########################################################
    # Multiple models for one field : [ [ 'm0', 'm1' ] ]
    # Multiple taylor terms and one field : [ [ 't0','t1'] ]
    # Multiple models per field : [ [ 'm0f0', 'm1f0' ] ,  [ 'm0f1', 'm1f1' ] ]
    # Multiple taylor terms per field : [ [ 't0f0','t1f0' ] , [ 't0f1','t1f1' ] ]
    ##########################################################
    # Cannot do multiple models per taylor term and per field for now.
    # ....... later...  [  [ ['m0t0f0','m1t0f0'],['m0t1f0','m1t1f0'] ] , [ [ ['t0f1'] ],[ ['t1f1'] ] ] ]
    ##########################################################
    def convertAllModelImages(self,modelimage, mode, nterms, dochaniter, chan, tmppath):
        """
        wrapper function for convertmodelimage for all different cases
        """
        if (type(modelimage)!=str and type(modelimage)!=list):
                    raise Exception,'modelimage must be a string or a list of strings';
        if (not all(img=='' or img==[] or img==[''] for img in modelimage)):
             if dochaniter:
                    self.defineChaniterModelimages(modelimage,chan,tmppath)
             else:
                    if type(modelimage)== str or \
                       (type(modelimage)==list and len(self.imagelist)==1 and len(modelimage)>1):
                        modelimage=[modelimage]

             #print "Run convertmodelimage for this list : ", self.imagelist, " with these models : ", modelimage;
             #spectralline modes + basic mfs
#             if (not mode=='mfs') or (mode=='mfs' and nterms==1):
#                    for j in range(len(self.imagelist)):   # = nfield
#                        self._casalog.post("Use modelimages: "+str(modelimage[j])+" to create a combined modelimage: " \
#                                           +self.imagelist.values()[j]+".model", 'DEBUG1')
#                        if modelimage[j] != '' and modelimage[j] != []:
#                            self.convertmodelimage(modelimages=modelimage[j],
#                                    outputmodel=self.imagelist.values()[j]+'.model',imindex=j)

#             else: # mfs and nterms>1
             if 1:
                    nfld = len(self.imagelist);
                    # if only one field, then modelimage must be a list of strings. convert to list of list of str
                    # if multiple fields, then model image : list of list of strings
                    if nfld != len(modelimage):
                       raise Exception,'Model images must be same length as fields : '+str(nfld) + str(modelimage);

                    for fld in range(nfld):
                       modsforfield = modelimage[fld]; # a list
                       if type(modsforfield)==str:
                            modsforfield = [modsforfield];
                       if nterms==1:
                            nimages = len(modsforfield);
                       else:
		            nimages = min( len(modsforfield), nterms ); ## one model per term
		       for tt in range(0,nimages):
                           if nterms==1:
                               modname = self.imagelist[fld]+'.model';
                           else:
                               modname = self.imagelist[fld]+'.model.tt'+str(tt) ;
			   if( os.path.exists(modsforfield[tt]) ):
#			       print "Found user-specified model image : "+modsforfield[tt]+" . Adding to starting model : "+modname;
			       self._casalog.post("Found user-specified model image : "+modsforfield[tt]+" . Adding to starting model : "+modname);
			       self.convertmodelimage(modelimages=modsforfield[tt],outputmodel=modname, imindex=fld);
			   else:
			       self._casalog.post("Cannot find user-specified model image : "+modsforfield[tt]+" . Continuing with current model : "+modname);



        

    def storeCubeImages(self,cubeimageroot,chanimageroot,chan,imagermode):
        """
        Put channel images back into CubeImages for chaniter=T mode
        
        Keyword arguments:
        cubeimageroot -- root name for output cube image
        chanimageroot -- root name for channel image
        chan          -- channel plane index
        imagermode    -- imagermode  
        """
        imagext = ['.image','.model','.flux','.residual','.psf','.mask']
        if imagermode=='mosaic':
            imagext.append('.flux.pbcoverage')
        lerange=range(self.nimages)
        for n in lerange:
            cubeimagerootname=cubeimageroot[n]
            chanimagerootname=chanimageroot[n]
        for ext in imagext:
            nomaskim=False
            cubeimage=cubeimagerootname+ext
            chanimage=chanimagerootname+ext
            if not os.path.exists(cubeimage):
                if os.path.exists(chanimage):
                    outim=ia.newimagefromimage(cubeimagerootname+'.model',cubeimage)
                    outim.done(verbose=False)
                elif ext=='.mask':
                    # unless mask image is given or in interactive mode
                    # there is no mask image
                    nomaskim=True
            if not nomaskim: 
                self.putchanimage(cubeimage, chanimage,chan)

    def cleanupTempFiles(self, tmppath):
        """
        Remove the directories listed by tmppath.
        """
        # Created to deal with temporary dirs created by chaniter=T clean,
        # now used elsewhere too.
        for dir in tmppath:
            if os.path.exists(dir):
               shutil.rmtree(dir)

    def convertImageFreqFrame(self,imlist):
        """
        Convert output images to proper output frame
        (after im.clean() executed)
        """
        if type(imlist)==str:
          imlist=[imlist]
        if self.usespecframe.lower() != 'lsrk':
          if self.usespecframe=='':
            inspectral=self.dataspecframe
          else: 
            inspectral=self.usespecframe
          for img in imlist:
            if os.path.exists(img):
              ia.open(img)
              csys=ia.coordsys()
              csys.setconversiontype(spectral=inspectral)
              #print "csys.torecord spectral2=", csys.torecord()['spectral2']
              ia.setcoordsys(csys.torecord())
              ia.close()

    def setFrameConversionForMasks(self):
        ''' To be called at the end of clean, so that the output csys can be
            read and set for the mask. This will have the users desired 
            conversion layer '''
        if self.usespecframe=='':
            useframe=self.dataspecframe
        else: 
            useframe=self.usespecframe

        #print 'maskimages.keys : ', self.maskimages.keys()
        #print 'imagelist : ', self.imagelist

        for key in self.imagelist.keys():
             imgmask = self.imagelist[key]+'.mask'
             img = self.imagelist[key]+'.image'
             if not os.path.exists(img):
                 img = img+'.tt0'
#             print 'Converting frame for ', imgmask, ' to ', useframe
             if os.path.exists(imgmask) and os.path.exists(img):
                  ia.open(img)
                  imcsys = ia.coordsys()
                  ia.close()
                  ia.open(imgmask)
#                  csys=ia.coordsys()
#                  csys.setreferencecode('LSRK','spectral',True)
#                  val = csys.setconversiontype(spectral=useframe)
#                  print 'Ret val : ', val, csys.getconversiontype('spectral')
#                  ia.setcoordsys(csys.torecord())
##                  print 'conv type : ', imcsys.getconversiontype('spectral')
                  ia.setcoordsys( imcsys.torecord() )
                  ia.close()
             else:
                 self._casalog.post('Not converting spectral reference frame for mask image','DEBUG1')


    def setReferenceFrameLSRK(self, img = ''):
        ''' To be called to reset reference code and conversion layer to LSRK '''
        if os.path.exists( img ):
            ia.open( img )
            mycsys=ia.coordsys()
            if (mycsys.torecord()['spectral2']['conversion']['system']=='REST') :
                ia.close()
                return
            if (mycsys.torecord()['spectral2']['conversion']['system']!='LSRK') :
                mycsys.setreferencecode('LSRK','spectral',True)
            mycsys.setconversiontype(spectral='LSRK')
            ia.setcoordsys( mycsys.torecord() )
            ia.close()
 
    def resmooth(self, model, residual, restored, minOrMax):
        if(minOrMax=="common"):
            ia.open(restored)
            beam=ia.restoringbeam();
            if(not beam.has_key('nChannels')):
                return 
            combeam=ia.commonbeam()
            ia.done()
            ia.fromimage(outfile='__restored-copy', infile=restored, overwrite=True)
            ia.open('__restored-copy')
            ib=ia.convolve2d(outfile=restored, major='', minor='', pa='', targetres=True, beam=combeam, overwrite=True)
            ib.done()
            ia.remove()
            ia.done()
            ia.fromimage(outfile='__residual-copy', infile=residual, overwrite=True)
            ia.open('__residual-copy')
            ###need to set a beam first to go around CAS-5433 and then loop
            ia.setrestoringbeam(major=beam['beams']['*0']['*0']['major'], minor=beam['beams']['*0']['*0']['minor'], pa=beam['beams']['*0']['*0']['positionangle'], channel=0, polarization=0)
            nchan=beam['nChannels']
            for k in range(nchan):
                chstr='*'+str(k)
                ia.setrestoringbeam(beam=beam['beams'][chstr]['*0'], channel=k, polarization=0)
            ib=ia.convolve2d(outfile=residual, major='', minor='', pa='', targetres=True, beam=combeam, overwrite=True)
            ib.done()
            ia.remove()
            ia.done()
            return

        ###############for min or max
        ia.open(restored)
        beams=ia.restoringbeam()
        if(not beams.has_key('beams')):
           ########already has one beam only
            ia.done()
            return
        minArea=1e37
        maxArea=-1e37
        maxchan=-1
        minchan=-1
        theArea=numpy.zeros(beams['nChannels'])
        for k in range(beams['nChannels']):
           ##it must have been really hard to provide proper indices
            theArea[k]=qa.convert(beams['beams']['*'+str(k)]['*0']['major'], 'arcsec')['value'] * qa.convert(beams['beams']['*'+str(k)]['*0']['minor'], 'arcsec')['value']
            if(theArea[k] > maxArea):
                maxArea=theArea[k]
                maxchan=k
            if(theArea[k] < minArea):
                minArea=theArea[k]
                minchan=k
        maxbeam=[beams['beams']['*'+str(maxchan)]['*0']['major'], beams['beams']['*'+str(maxchan)]['*0']['minor'], beams['beams']['*'+str(maxchan)]['*0']['positionangle']]
        minbeam=[beams['beams']['*'+str(minchan)]['*0']['major'], beams['beams']['*'+str(minchan)]['*0']['minor'], beams['beams']['*'+str(minchan)]['*0']['positionangle']]
        thebeam=minbeam
        tobeDiv=theArea[minchan]
        if(minOrMax=='max'):
            thebeam=maxbeam
            tobeDiv=theArea[maxchan]
        ia.open(residual)
        shp=ia.shape()
        for k in range(beams['nChannels']):
            reg=rg.box(blc=[0,0,0,k], trc=[shp[0]-1, shp[1]-1, shp[2]-1, k])
            pix=ia.getregion(region=reg)
            pix=pix*theArea[k]/tobeDiv
            ia.putregion(pixels=pix, region=reg)
        ia.done()
        ia.open(model)
        ib=ia.convolve2d(outfile=restored, axes=[0,1], major=thebeam[0], minor=thebeam[1], pa=thebeam[2], overwrite=True)
        ib.calc('"'+restored+'" + '+'"'+residual+'"')
        ib.done()
        ia.done()
           
       



    @staticmethod
    def getOptimumSize(size):
        '''
        This returns the next largest even composite of 2, 3, 5, 7
        '''
        def prime_factors(n, douniq=True):
            """ Return the prime factors of the given number. """
            factors = []
            lastresult = n
            sqlast=int(numpy.sqrt(n))+1
           # 1 pixel must a single dish user
            if n == 1:
                return [1]
            c=2
            while 1:
                if (lastresult == 1) or (c > sqlast):
                    break
                sqlast=int(numpy.sqrt(lastresult))+1
                while 1:
                    if(c > sqlast):
                        c=lastresult
                        break
                    if lastresult % c == 0:
                        break            
                    c += 1

                factors.append(c)
                lastresult /= c
            if(factors==[]): factors=[n]
            return  numpy.unique(factors).tolist() if douniq else factors 
        n=size
        if(n%2 != 0):
            n+=1
        fac=prime_factors(n, False)
        for k in range(len(fac)):
            if(fac[k] > 7):
                val=fac[k]
                while(numpy.max(prime_factors(val)) > 7):
                    val +=1
                fac[k]=val
        newlarge=numpy.product(fac)
        for k in range(n, newlarge, 2):
            if((numpy.max(prime_factors(k)) < 8)):
                return k
        return newlarge


def getFTMachine(gridmode, imagermode, mode, wprojplanes, userftm):
    """
    A utility function which implements the logic to determine the
    ftmachine name to be used in the under-laying tool.
    """
#    ftm = userftm;
    ftm='ft';
    if ((gridmode == 'widefield') and(wprojplanes > 1)): ftm = 'wproject';
    elif (gridmode == 'aprojection'):                    ftm = 'awproject';
    elif (gridmode == 'advancedaprojection'):            ftm = 'awproject';
    elif (imagermode == 'csclean'):                      ftm = 'ft';
    elif (imagermode == 'mosaic'):                       ftm = userftm;
    return ftm;

def getAlgorithm(psfmode, imagermode, gridmode, mode, 
                 multiscale, multifield, facets, nterms, useralg):
    """
    A utility function which implements the logic to determine the
    deconvolution algorithm to be used in the under-laying tool.
    """
    alg=useralg
    addMultiField=False;

    if((type(multiscale)==list) and 
       (len(multiscale) > 0) and
       (sum(multiscale) > 0)): alg = 'multiscale';
    elif ((psfmode == 'clark') or (psfmode == 'hogbom')): alg=psfmode;

    if ((imagermode == '') and (multifield)): addMultiField=True;
    if (imagermode == 'mosaic'):              addMultiField=True;
    if (imagermode == 'csclean'):             addMultiField = True; #!!!!

    if ((mode == 'mfs') and (nterms > 1)): 
        alg = 'msmfs';
        if(imagermode == 'mosaic'): 
            ##print 'Multi-Term MFS with a mosaic is experimental'
            raise Exception, 'msmfs (nterms>1) not allowed with imagermode=' + imagermode + '. For now, msmfs automatically performs cs-clean type iterations';
        if (multifield): 
		addMultiField = True;
        if facets > 1:
            raise Exception, 'msmfs (nterms>1) with facets>1 is not yet available'
    if( (mode=='mfs') and (nterms<1) ):
         raise Exception, 'nterms must be > 0';

#    if (gridmode == 'widefield'): alg='mfclark';

    if (gridmode == 'widefield'):
        addMultiField=True;
        if (facets > 1):
            if(alg.count('multiscale') > 0):
                raise Exception, 'multiscale with facets > 1 not allowed for now';
            if (psfmode==''): psfmode='clark';
            if((psfmode == 'clark') or (psfmode == 'hogbom')):
                alg='wf'+psfmode;
                addMultiField=False;
            else:
                addMultiField=True;
#            addMultiField=False;

#
# if facets > 1 && mutliscale ==> fail


    if (addMultiField and (alg[0:2] != 'mf') and (alg != 'msmfs')):  alg = 'mf' + alg;
    return alg;

def convert_numpydtype(listobj):
    """
    utility function to covert list with elements in numpy.int or
    numpy.float types to python int/float
    """
    import array as pyarr
    floatarr=False
    intarr=False
    for elm in listobj:
      if numpy.issubdtype(type(elm), numpy.float):
        floatarr = True
      elif numpy.issubdtype(type(elm), numpy.int):
        intarr = True
    if floatarr or (floatarr and intarr):
      temparr=pyarr.array('f', listobj)
    elif intarr:
      temparr=pyarr.array('i', listobj)
    else:
      temparr = listobj
      return temparr
    return temparr.tolist()
