import casac
import os
import commands
import pdb

###some helper tools
mstool = casac.homefinder.find_home_by_name('msHome')
ms = mstool.create()
tbtool = casac.homefinder.find_home_by_name('tableHome')
tb = tbtool.create()
qatool = casac.homefinder.find_home_by_name('quantaHome')
qa = qatool.create()
metool = casac.homefinder.find_home_by_name('measuresHome')
me = metool.create()
rgtool=casac.homefinder.find_home_by_name('regionmanagerHome')
rg = rgtool.create()
iatool=casac.homefinder.find_home_by_name('imageHome')
ia = iatool.create()


class cleanhelper:
    def __init__(self, imtool='', vis='', usescratch=False, casalog=None):
        """
        Contruct the cleanhelper object with an imager tool
        like so:
        a=cleanhelper(im, vis)
        """
        if((type(imtool) != str) and (len(vis) !=0)):
            self.initsinglems(imtool, vis, usescratch)
        self.maskimages={}
        self.usescratch=usescratch

        if not casalog:  # Not good!
            loghome =  casac.homefinder.find_home_by_name('logsinkHome')
            casalog = loghome.create()
            #casalog.setglobal(True)
        self._casalog = casalog
        
        
    def initsinglems(self, imtool, vis, usescratch):
        self.im=imtool
        self.vis=vis
        if ((type(vis)==str) & (os.path.exists(vis))):
            self.im.open(vis, usescratch=usescratch)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
        self.phasecenter=''
        self.spwindex=-1
        self.fieldindex=-1
        self.outputmask=''
        self.csys={}

    def defineimages(self, imsize, cell, stokes, mode, spw, nchan, start,
                      width, restfreq, field, phasecenter, facets=1, outframe='', veltype='radio'):
        if((type(cell)==list) and (len(cell)==1)):
            cell.append(cell[0])
        elif ((type(cell)==str) or (type(cell)==int) or (type(cell)==float)):
            cell=[cell, cell]
        elif (type(cell) != list):
            raise TypeError, "parameter cell is not understood"
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
            raise TypeError, "parameter imsize is not understood"
            
	
        if(mode=='frequency'):
        ##check that start and step have units
            if(qa.quantity(start)['unit'].find('Hz') < 1):
                raise TypeError, "start parameter is not a valid frequency quantity "
            if(qa.quantity(width)['unit'].find('Hz') < 1):
                raise TypeError, "width parameter is not a valid frequency quantity "	
        elif(mode=='velocity'):
        ##check that start and step have units
            if(qa.quantity(start)['unit'].find('m/s') < 0):
                raise TypeError, "start parameter is not a valid velocity quantity "
            if(qa.quantity(width)['unit'].find('m/s') < 0):
                raise TypeError, "width parameter is not a valid velocity quantity "	
        else:
            if((type(width) != int) or 
               (type(start) != int)):
                raise TypeError, "start, width have to be integers with mode %s" %mode

        #####understand phasecenter
        if(type(phasecenter)==str):
            ### blank means take field[0]
            if (phasecenter==''):
                fieldoo=field
                if(fieldoo==''):
                    fieldoo='0'
                phasecenter=int(ms.msseltoindex(self.vis,field=fieldoo)['field'][0])
            else:
                tmppc=phasecenter
                try:
                    if(len(ms.msseltoindex(self.vis, field=phasecenter)['field']) > 0):
                        tmppc = int(ms.msseltoindex(self.vis,
                                                    field=phasecenter)['field'][0])
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
            spwindex=ms.msseltoindex(self.vis, spw=spw)['spw'].tolist()
            if(len(spwindex) == 0):
                spwindex = -1
        self.spwindex = spwindex
        ##end spwindex
        self.im.defineimage(nx=imsize[0],      ny=imsize[1],
                            cellx=cellx,       celly=celly,
                            mode=mode,         nchan=nchan,
                            start=start,       step=width,
                            spw=spwindex,      stokes=stokes,
                            restfreq=restfreq, outframe=outframe,
                            veltype=veltype, phasecenter=phasecenter,
                            facets=facets)

    def definemultiimages(self, rootname, imsizes, cell, stokes, mode, spw,
                          nchan, start, width, restfreq, field, phasecenters,
                          names=[], facets=1, outframe='',veltype='radio',  makepbim=False):
        #will do loop in reverse assuming first image is main field
        if not hasattr(imsizes, '__len__'):
            imsizes = [imsizes]
        self.nimages=len(imsizes)
        if((len(imsizes)<=2) and ((type(imsizes[0])==int) or (type(imsizes[0]==long)))):
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
        if(type(phasecenters) == int):
            phasecenters=[phasecenters]
        self._casalog.post('Number of phase centers: ' + str(len(phasecenters)),
                           'DEBUG1')

        if((self.nimages==1) and (type(names)==str)):
            names=[names]
        if((len(phasecenters)) != (len(imsizes))):
            errmsg = "Mismatch between the number of phasecenters (%d), image sizes (%d) , and images (%d)" % (len(phasecenters), len(imsizes), self.nimages)
            self._casalog.post(errmsg, 'SEVERE')
            raise ValueError, errmsg
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
                self.im.selectvis(field=field)
		self.im.setvp(dovp=True)
                self.im.makeimage(type='pb', image=self.imagelist[n]+'.flux')
		self.im.setvp(dovp=False)
    
    def makemultifieldmask(self, maskobject=''):
        """
        This function assumes that the function definemultiimages has been run and thus
        self.imagelist is defined
        if single image use the single image version
        """
        if((len(self.maskimages)==(len(self.imagelist)))):
            if(not self.maskimages.has_key(self.imagelist[0])):
                self.maskimages={}
        else:
            self.maskimages={}
        masktext=[]
        if( (len(maskobject)==0) or (maskobject==[''])):
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
                    ia.done()
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
                    ia.done()

    def makemultifieldmask2(self, maskobject=''):
        """
        New makemultifieldmask to accomodate different kinds of masks supported
        in clean with flanking fields (added by TT)
        required: definemultiimages has already run so that imagelist is defined 
        """
        if((len(self.maskimages)==(len(self.imagelist)))):
            if(not self.maskimages.has_key(self.imagelist[0])):
                self.maskimages={}
        else:
            self.maskimages={}

        #print "makemultifieldmask2: intial self.imagelist=",self.imagelist
        if((len(maskobject)==0) or (maskobject==[''])):
            return
        # determine number of input elements
        if (type(maskobject)==str):
            maskobject=[maskobject]
        if(type(maskobject) != list):
            ##don't know what to do with this
            raise TypeError, 'Dont know how to deal with mask object'
        if(type(maskobject[0])==int or type(maskobject[0])==float):
            maskobject=[maskobject] 
        if(type(maskobject[0][0])==list):
            if(type(maskobject[0][0][0])!=int and type(maskobject[0][0][0])!=float):        
                maskobject=maskobject[0]
                    
        # define maskimages
        if(len(self.maskimages)==0):
            for k in range(len(self.imageids)):
                if(not self.maskimages.has_key(self.imagelist[k])):
                    self.maskimages[self.imagelist[k]]=self.imagelist[k]+'.mask'
        # initialize maskimages
        for k in range(len(self.imagelist)):
            if(not (os.path.exists(self.maskimages[self.imagelist[k]]))):
                ia.fromimage(outfile=self.maskimages[self.imagelist[k]],
                        infile=self.imagelist[k])
                ia.open(self.maskimages[self.imagelist[k]])
                ia.set(pixels=0.0)
                ia.done()

        # assume a file name list for each field
        masktext=[]
        # take out extra []'s
        maskobject=self.flatten(maskobject)
        for maskid in range(len(maskobject)):
            masklist=[]
            tablerecord=[]
            if(type(maskobject[maskid]))==str:
                if(maskobject[maskid])=='':
                    #skipped
                    continue
                else:
                    maskobject[maskid]=[maskobject[maskid]]
            for masklets in maskobject[maskid]:
                if(type(masklets)==int):
                    masklist.append(maskobject[maskid])
                if(type(masklets)==str):
                    if(masklets==''):
                        #skip
                        continue
                    if(os.path.exists(masklets)):
                        if(commands.getoutput('file '+masklets).count('directory')):
                            if(self.maskimages[self.imagelist[maskid]] == masklets):
                                self.maskimages[self.imagelist[maskid]]=masklets
                            else:
                                # make a copy
                                ia.open(self.imagelist[maskid])
                                self.csys=ia.coordsys().torecord()
                                shp = ia.shape()
                                ia.done()
                                self.copymaskimage(masklets,shp,'__tmp_mask')
                                ia.open(self.maskimages[self.imagelist[maskid]])
                                ia.calc(pixels='+ __tmp_mask')
                                ia.done()
                                ia.removefile('__tmp_mask')

                        elif(commands.getoutput('file '+masklets).count('text')):
                            masktext.append(masklets)
                        else:
                            tablerecord.append(masklets)
                            #raise TypeError, 'Can only read text mask files or mask images'
                    else:
                        raise TypeError, masklets+' seems to be non-existant'

                if(type(masklets)==list):
                    masklist.append(masklets)
            # initialize mask
            #if (len(self.maskimages) > 0):
            #    if(not (os.path.exists(self.maskimages[self.imagelist[maskid]]))):
            #        ia.fromimage(outfile=self.maskimages[self.imagelist[maskid]],
            #                     infile=self.imagelist[maskid])
            #        ia.open(self.maskimages[self.imagelist[maskid]])
            #        ia.set(pixels=0.0)
            #        ia.done()
            #        print "INITIALIZED: ",self.maskimages[self.imagelist[maskid]]

            # handle boxes in lists
            if(len(masklist) > 0):
                self.im.regiontoimagemask(mask=self.maskimages[self.imagelist[maskid]], boxes=masklist)
            if(len(tablerecord) > 0 ):
                reg={}
                for tabl in tablerecord:
                    reg.update({tabl:rg.fromfiletorecord(filename=tabl, verbose=False)})
                if(len(reg)==1):
                    reg=reg[reg.keys()[0]]
                else:
                    reg=rg.makeunion(reg)
                self.im.regiontoimagemask(mask=self.maskimages[self.imagelist[maskid]], region=reg)
        #boxfile handling done all at once
        if(len(masktext) > 0):
            # fill for all fields in boxfiles
            circles, boxes=self.readmultifieldboxfile(masktext)
            # doit for all fields
            for k in range(len(self.imageids)):
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
        # set unused mask images to 1 for a whole field
        for key in self.maskimages:
            if(os.path.exists(self.maskimages[key])):
                ia.open(self.maskimages[key])
                fsum=ia.statistics()['sum']
                if(fsum[0]==0.0):
                    ia.set(pixels=1.0)
                ia.done()
                
        
    def makemaskimage(self, outputmask='', imagename='', maskobject=[]):
        """
        This function is an attempt to convert all the kind of 'masks' that
        people want to throw at it and convert it to a mask image to be used
        by imager...For now 'masks' include
        
        a)set of previous mask images
        b)lists of blc trc's
        c)record output from rg tool for e.g
        """
        if( (len(maskobject)==0) or (maskobject==[''])):
            return
        maskimage=[]
        masklist=[]
        masktext=[]
        maskrecord={}
        tablerecord=[]
        if(type(maskobject)==dict):
            maskrecord=maskobject
            maskobject=[]
        if(type(maskobject)==str):
            maskobject=[maskobject]
        
        if(type(maskobject) != list):
            ##don't know what to do with this
            raise TypeError, 'Dont know how to deal with maskobject'
        if((type(maskobject[0])==int) or  (type(maskobject[0])==float)):
            masklist.append(maskobject)
        else:
            for masklets in maskobject:
                
                if(type(masklets)==str):
                    if(os.path.exists(masklets)):
                        if(commands.getoutput('file '+masklets).count('directory')):
                            maskimage.append(masklets)
                        elif(commands.getoutput('file '+masklets).count('text')):
                            masktext.append(masklets)
                        else:
                            tablerecord.append(masklets)
                    else:
                       raise TypeError, masklets+' seems to be non-existant' 
                if(type(masklets)==list):
                    masklist.append(masklets)
                if(type(masklets)==dict):
                    maskrecord=masklets
                
        if(len(outputmask)==0):
            outputmask=imagename+'.mask'
        if(os.path.exists(outputmask)):
            self.im.make('__temp_mask')    
            ia.open('__temp_mask')
            shp=ia.shape()
            self.csys=ia.coordsys().torecord()
            ia.close()
            ia.removefile('__temp_mask')
            ia.open(outputmask)
            ia.regrid(outfile='__temp_mask',shape=shp,axes=[0,1], csys=self.csys,overwrite=True)
            ia.done()
            ia.removefile(outputmask)
            os.rename('__temp_mask',outputmask)
        else:
            self.im.make(outputmask)
        ia.open(outputmask)
        shp=ia.shape()
        self.csys=ia.coordsys().torecord()
        ia.close()
        if(len(maskimage) > 0):
            for ima in maskimage :
                self.copymaskimage(ima, shp, '__temp_mask')
                #ia.open(ima)
                #ia.regrid(outfile='__temp_mask',shape=shp,csys=self.csys,
                #          overwrite=True)
                #ia.done()
                os.rename(outputmask,'__temp_mask2')
                ia.imagecalc(outfile=outputmask,
                             pixels='__temp_mask + __temp_mask2',
                             overwrite=True)
                ia.done()
                ia.removefile('__temp_mask')
                ia.removefile('__temp_mask2')
            #make image a mask image i.e 1 and 0 only
            ia.open(outputmask)
            ###getchunk is a mem hog
            #arr=ia.getchunk()
            #arr[arr>0.01]=1
            #ia.putchunk(arr)
            ia.calc(pixels='iif('+outputmask+'>0.01, 1, 0)')
            ia.close()
        #### This goes when those tablerecord goes
        if(len(tablerecord) > 0):
            reg={}
            for tabl in tablerecord:
                reg.update({tabl:rg.fromfiletorecord(filename=tabl, verbose=False)})
            if(len(reg)==1):
                reg=reg[reg.keys()[0]]
            else:
                reg=rg.makeunion(reg)
            self.im.regiontoimagemask(mask=outputmask, region=reg)
        ###############
        if((type(maskrecord)==dict) and (len(maskrecord) > 0)):
            self.im.regiontoimagemask(mask=outputmask, region=maskrecord)
        if(len(masktext) >0):
            for textfile in masktext :
                polydic,listbox=self.readboxfile(textfile)
                masklist.extend(listbox)
                if(len(polydic) > 0):
                    self.im.regiontoimagemask(mask=outputmask, region=polydic)
        if((type(masklist)==list) and (len(masklist) > 0)):
            self.im.regiontoimagemask(mask=outputmask, boxes=masklist)
        if(os.path.exists(imagename) and (len(rg.namesintable(imagename)) !=0)):
            regs=rg.namesintable(imagename)
            if(type(regs)==str):
                    regs=[regs]
            for reg in regs:
                elrec=rg.fromtabletorecord(imagename, reg)
                self.im.regiontoimagemask(mask=outputmask, region=elrec)
        self.outputmask=outputmask
        #Done with making masks
    def datselweightfilter(self, field, spw, timerange, uvrange, antenna,scan,
                           wgttype, robust, noise, npixels, mosweight,
                           innertaper, outertaper, calready):
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
            tb.open(self.vis+'/FIELD')
            self.fieldindex=range(tb.nrows())
            tb.close()
        #weighting and tapering should be done together
        if(weighting=='natural'):
            mosweight=False
        if(mosweight):
            calready=True
            for k in self.fieldindex :
                self.im.selectvis(field=k, spw=self.spwindex,time=timerange, usescratch=calready)
                self.im.weight(type=weighting,rmode=rmode,robust=robust, npixels=npixels, noise=qa.quantity(noise,'Jy'))
            ###now redo the selectvis
            self.im.selectvis(field=field,spw=spw,time=timerange,
                              baseline=antenna, scan=scan, uvrange=uvrange, usescratch=calready)
        else:
            self.im.selectvis(field=field,spw=spw,time=timerange,
                              baseline=antenna, scan=scan, uvrange=uvrange, usescratch=calready)
            self.im.weight(type=weighting,rmode=rmode,robust=robust,
                           npixels=npixels, noise=qa.quantity(noise, 'Jy'))
        if((type(outertaper)==list) and (len(outertaper) > 0)):
            if(len(outertaper)==1):
                outertaper.append(outertaper[0])
                outertaper.append('0deg')
            if(qa.quantity(outertaper[0])['value'] > 0.0):    
                self.im.filter(type='gaussian', bmaj=outertaper[0],
                               bmin=outertaper[1], bpa=outertaper[2])
    def setrestoringbeam(self, restoringbeam):
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
                    resbmin=restoringbeam[2]+'deg'
        if((resbmaj != '') and (resbmin != '')):
           self.im.setbeam(resbmaj, resbmin, resbpa)
        
    def convertmodelimage(self, modelimages=[], outputmodel=''):
        modelos=[]
        maskelos=[]
        if((modelimages=='') or (modelimages==[])):
            return
        if(type(modelimages)==str):
            modelimages=[modelimages]
        k=0
        for modim in modelimages:
            ia.open(modim)
            modelos.append('modelos_'+str(k))
            if( (ia.brightnessunit().count('/beam')) > 0):
                maskelos.append(modelos[k]+'.sdmask')
                self.im.makemodelfromsd(sdimage=modim,modelimage=modelos[k],maskimage=maskelos[k])
            else:
                ##assuming its a model image already then just regrid it
                self.im.make(modelos[k])
                ia.open(modelos[k])
                newcsys=ia.coordsys()
                newshape=ia.shape()
                ia.open(modim)
                ib=ia.regrid(outfile=modelos[k], shape=newshape, axes=[0,1,3], csys=newcsys.torecord(), overwrite=True)
                ib.done()
                
            k=k+1
            ia.close()
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
                if((ia.statistics()['max'].max()) > 0.00001):
                    doAnd=True
                ia.close()
            if(doAnd):
                tmpomask='__temp_o_mask'
                self.makemaskimage(outputmask=tmpomask, maskobject=maskelos)
                os.rename(outputmask, '__temp_i_mask')
                ia.imagecalc(outfile=outputmask, pixels='__temp_o_mask * __temp_i_mask', overwrite=True)
                ia.removefile('__temp_o_mask')
                ia.removefile('__temp_i_mask')
                self.outputmask=outputmask
            else:
                self.makemaskimage(outputmask=outputmask, maskobject=maskelos)
        for ima in maskelos:
            if(os.path.exists(ima)):
                ia.removefile(ima)
        if(not (os.path.exists(outputmodel))):
            self.im.make(outputmodel)
        for k in range(len(modelos)):
            os.rename(outputmodel,'__temp_model2')
            ia.imagecalc(outfile=outputmodel,
                             pixels=modelos[k]+' + '+'__temp_model2',
                             overwrite=True)
            
            ia.removefile('__temp_model2')
            ia.removefile(modelos[k])
            
    
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
            polyg=rg.dounion(temprec)
        return polyg,union

    def readmultifieldboxfile(self, boxfiles):
        circles={}
        boxes={}
        for k in range(len(self.imageids)):
            circles[self.imageids[k]]=[]
            boxes[self.imageids[k]]=[]
        for boxfile in boxfiles:
            f=open(boxfile)
            while 1:
                try:
                    line=f.readline()
                    if(len(line)==0):
                        raise Exception
                    if (line.find('#')!=0):
                        ### its an AIPS boxfile
                        splitline=line.split('\n')
                        splitline2=splitline[0].split()
                        print "splitline2=",splitline2
                        if (len(splitline2)<6):
                            ##circles
                            if(int(splitline2[1]) <0):
                                circlelist=[int(splitline2[2]),
                                            int(splitline2[3]),int(splitline2[4])]
                                #circles[splitline2[0]].append(circlelist)
                                circles[self.imageids[int(splitline2[0])]].append(circlelist)
                            else:
                                #boxes
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
                
        return circles,boxes

    def readoutlier(self, outlierfile):
        """ Read a file containing clean boxes (kind of
        compliant with AIPS FACET FILE)
            
        Format is:
         col0    col1   col2  col3 col4 col5 col6 col7  col8   col9
          C    FIELDID SIZEX SIZEY RAHH RAMM RASS DECDD DECMM DECSS      
        why first column has to have C ... because its should
        not to be A or B ...now D would be a totally different thing

        Note all lines beginning with '#' are ignored.
        
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
                        if(splitline2[0]=='C'):
                            imageids.append(splitline2[1])
                            imsizes.append((int(splitline2[2]),int(splitline2[3])))
                            mydir='J2000  '+splitline2[4]+'h'+splitline2[5]+'m'+splitline2[6]+'  '+splitline2[7]+'d'+splitline2[8]+'m'+splitline2[9]
                            phasecenters.append(mydir)
    
            except:
                break

        f.close()
        return imsizes,phasecenters,imageids

    def copymaskimage(self, maskimage, shp, outfile):
        
        ia.open(maskimage)
        oldshp=ia.shape()
        if((len(oldshp) < 4) or (shp[2] != oldshp[2]) or (shp[3] != oldshp[3])):
            #take the first plane of mask
            tmpshp=oldshp
            tmpshp[0]=shp[0]
            tmpshp[1]=shp[1]
            ib=ia.regrid(outfile='__looloo', shape=tmpshp, axes=[0,1], csys=self.csys, overwrite=True)
            #dat=ib.getchunk()
            ib.done()
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
            ia.calc('__temp_mask[index3 in [0]]+__looloo') 
            ia.done()
            ia.removefile('__looloo')
        else:
            ib=ia.regrid(outfile=outfile ,shape=shp, axes=[0,1], csys=self.csys, overwrite=True)
            ia.done()
            ib.done()


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

