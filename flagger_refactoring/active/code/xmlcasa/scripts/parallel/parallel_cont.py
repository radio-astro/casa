import os
import pdb
from taskinit import *
import time
import numpy as np
from cleanhelper import *
class imagecont():
    def __init__(self, ftmachine='ft', wprojplanes=10, facets=1, pixels=[3600, 3600], cell=['3arcsec', '3arcsec'], spw='', field='', phasecenter='', weight='natural', robust=0.0, stokes='I'):
        self.im=imtool.create()
        self.imperms={}
        self.dc=dctool.create()
        self.ft=ftmachine
        self.origms=''
        self.wprojplanes=wprojplanes
        self.facets=facets
        print 'cell', cell
        self.pixels=pixels
        self.cell=cell
        if(spw==''):
            spw='*'
        self.spw=spw
        self.field=field
        self.phCen=phasecenter
        self.weight=weight
        self.imageparamset=False
        self.robust=robust
        self.weightnpix=0
        self.stokes=stokes
        self.imagetilevol=1000000
        self.visInMem=False
        self.painc=360.0
        self.pblimit=0.1
        self.dopbcorr=True
        self.novaliddata={}
        self.applyoffsets=False
        self.cfcache='cfcache.dir'
        self.epjtablename=''
    def setparamcont(self, im, freq, band, singleprec=False):
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], 
                       celly=self.cell[1], phasecenter=self.phCen, mode='frequency', 
                       nchan=1, start=freq, step=band, facets=self.facets, 
                       stokes=self.stokes)
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, 
                      pastep=self.painc, pblimit=self.pblimit, 
                      cfcachedirname=self.cfcache, dopbgriddingcorrections=self.dopbcorr, 
                      applypointingoffsets=self.applyoffsets, imagetilevol=self.imagetilevol,
                       singleprecisiononly=singleprec)

    def imagecontmultims(self, msnames=[''], start=0, numchan=1, spw=0, field=0, freq='1.20GHz', band='200MHz', imname='newmodel'):
        im=self.im
        ###either psf 0 or no channel selected
        if(self.novaliddata==True):
            return
        else:
            self.novaliddata=False
        if(not self.imageparamset):
          try:
              conlis=lambda a,n: a if (type(a)==list) else [a]*n
              if (type(msnames)==str):
                  msnames=[msnames]
              numms=len(msnames)
              start=conlis(start,numms)
              numchan=conlis(numchan, numms)
              spw=conlis(spw, numms)
              field=conlis(field, numms)
              numfail=0
              for k  in range(numms):
                  try:
                      im.selectvis(vis=msnames[k], field=field[k], spw=spw[k], nchan=numchan[k], start=start[k], step=1, datainmemory=self.visInMem)
                  except:
                      numfail+=1
              if(numfail==numms):
                  self.novaliddata=True 
          except Exception, instance:
                ###failed to selectdata
                self.novaliddata=True  
          self.setparamcont(im, freq, band, singleprec=False)
          if((len(numchan)==0) or (np.sum(numchan)==0)):
              self.novaliddata=True
        self.makecontimage(im, self.novaliddata, imname)
        self.imageparamset=True



 ####
    def imagecont(self, msname='spw00_4chan351rowTile.ms', start=0, numchan=1, spw=0, field=0, freq='1.20GHz', band='200MHz', imname='newmodel'):
        #casalog.post('KEYS '+str(self.imperms.keys()))
        if(not self.imperms.has_key(msname)):
            self.imageparamset=False
            im=imtool.create()
            self.imperms[msname]=im
            self.novaliddata[msname]=False
            #casalog.post('MSNAME '+msname)
        else:
            #casalog.post('reMSNAME '+msname)
            im=self.imperms[msname]
            self.imageparamset=True
        ###either psf 0 or no channel selected
        if(self.novaliddata[msname]):
            return
        #j=start
        #end=start+numchan-1
        #spwstring=str(spw)+':'+str(start)+'~'+str(end)
        #print 'spwstring', spwstring
        if(not self.imageparamset):
            self.origms=msname
            try:
                im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem)
            except Exception, instance:
                ###failed to selectdata
                self.novaliddata[msname]=True
####
        #imname=imname+'_%02d'%(j)               
            self.setparamcont(im, freq, band)
            if((len(numchan)==0) or (np.sum(numchan)==0)):
                self.novaliddata[msname]=True
        self.makecontimage(im, self.novaliddata[msname], imname)
        self.imageparamset=True

    def imagecontbychan(self, msname='spw00_4chan351rowTile.ms', start=[0], numchan=[1], spw=[0], field=0, freq='1.20GHz', band='200MHz', imname='newmodel'):
        ia=iatool.create()
        if(type(spw) == int):
            spw=[spw]
            start=[start]
            numchan=[numchan]
        totchan=np.sum(numchan)
        if(not self.imageparamset):
            if(totchan==0):
                ###make blanks
                self.im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1)
                self.im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, mode='frequency', nchan=1, start=freq, step=band, facets=self.facets)
                self.im.make(imname+'.image')
                self.im.make(imname+'.residual')
                self.im.make(imname+'.model')
                self.im.make(imname+'.psf')
                self.imageparamset=True
                return
            del self.im
            self.im=[]
            del self.novaliddata
            self.novaliddata=[]
            selkey={}
            spwind=0
            chancounter=0
            for k in range(totchan):
                if(chancounter==numchan[spwind]):
                    chancounter=0
                    spwind+=1
                selkey[k]= {'spw': spw[spwind], 'start':start[spwind]+chancounter}
                chancounter += 1
                self.im.append(imtool.create())
                self.novaliddata.append(False)
            print 'selkey', selkey
        else:
             if(totchan==0):
                 return
        origname=msname
        #pdb.set_trace()
        for k in range(totchan): 
            ###either psf 0 or no channel selected
            if(k==0):
                image=imname+'.image'
                residual=imname+'.residual'
                psf=imname+'.psf'
            else:
                image=imname+'_'+str(k)+'.image'
                residual=imname+'_'+str(k)+'.residual'
                psf=imname+'_'+str(k)+'.psf'
            if(not self.novaliddata[k]):
                msname=origname
                if(not self.imageparamset):
                    self.im[k].selectvis(vis=msname, field=field, spw=selkey[k]['spw'], nchan=1, start=selkey[k]['start'], step=1, datainmemory=self.visInMem)
                    self.im[k].defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, mode='frequency', nchan=1, start=freq, step=band, facets=self.facets)
                    self.im[k].weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                              robust=self.robust)
                    self.im[k].setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, pastep=self.painc, pblimit=self.pblimit, cfcachedirname=self.cfcache, dopbgriddingcorrections=self.dopbcorr, applypointingoffsets=self.applyoffsets, imagetilevol=self.imagetilevol, singleprecisiononly=True)
               #im.regionmask(mask='lala.mask', boxes=[[0, 0, 3599, 3599]])
               #im.setmfcontrol(cyclefactor=0.0)
                if(not self.imageparamset):
                    try:
                        self.im[k].clean(algorithm='clark', niter=0, model=imname+'.model', image=image, residual=residual, psfimage=psf)
                        if(k > 0):
                            ia.open(imname+'.psf')
                            ia.calc('"'+imname+'.psf'+'" + "'+psf+'"')
                            ia.done()
                    except Exception, instance:
                        if(string.count(instance.message, 'PSFZero') >0):
                            self.novaliddata[k]=True
                        ###make a blank image
                            if(k==0):
                                self.im[k].make(image)
                                self.im[k].make(residual)
                        else:
                            raise instance
                else:
                    if(not self.novaliddata[k]):
                        self.im[k].restore(model=imname+'.model',  image=image, residual=residual)
                if(k > 0):
                    ia.open(imname+'.residual')
                    ia.calc('"'+imname+'.residual'+'" + "'+residual+'"')
                    ia.done()
                    ia.open(imname+'.image')
                    ia.calc('"'+imname+'.image'+'" + "'+residual+'"')
                    ia.done()
                tb.showcache()
                             
        ia.open(imname+'.residual')
        ia.calc('"'+imname+'.residual'+'"/'+str(totchan))
        ia.done()
        ia.open(imname+'.image')
        ia.calc('"'+imname+'.image'+'"/'+str(totchan))
        ia.done()
        if(not self.imageparamset):
           ia.open(imname+'.psf')
           ia.calc('"'+imname+'.psf'+'"/'+str(totchan))
           ia.done() 
        #im.done()
        del ia
        tb.showcache()
        self.imageparamset=False

    def cleancont(self, niter=100, alg='clark', thr='0.0mJy', psf='newmodel.psf', dirty='newmodel.dirty', model='newmodel.model', mask='', scales=[0]):
        dc=dctool.create()
        dc.open(dirty=dirty, psf=psf)
        if((alg=='hogbom') or (alg == 'msclean')):
            sca=scales if (alg=='msclean') else [0]
            dc.setscales(scalemethod='uservector', uservector=sca)
            alg='fullmsclean'
        if(alg=='clark'):
            dc.fullclarkclean(niter=niter, threshold=thr, model=model, mask=mask)
        else:
            dc.clean(algorithm=alg, niter=niter, threshold=thr, model=model, mask=mask)
        dc.done()
        del dc

    def imagechan(self, msname='spw00_4chan351rowTile.ms', start=0, numchan=1, spw=0, field=0, imroot='newmodel', imchan=0, niter=100, alg='clark', thr='0.0mJy', mask='', majcycle=1, scales=[0], fstart='1GHz', width='10MHz', chanchunk=1):
        origname=msname
#        a=cleanhelper()
        imname=imroot+str(imchan)
        maskname=''
        if(mask != ''):
            maskname=imname+'.mask'
            if( not self.makemask(inmask=mask, outmask=maskname, imchan=imchan, chanchunk=chanchunk)):
                maskname=''
 #       a.getchanimage(cubeimage=imroot+'.model', outim=imname+'.model', chan=imchan)
        im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem)
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets, mode='frequency', nchan=chanchunk, start=fstart, step=width)
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
       
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True)
        im.setscales(scalemethod='uservector', uservector=scales)
        im.setmfcontrol(cyclefactor=0.0)  
        majcycle = majcycle if (niter/majcycle) >0 else 1
        for kk in range(majcycle):
            im.clean(algorithm=alg, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', mask=maskname, psfimage='')
        im.done()
        if(maskname != ''):
            shutil.rmtree(maskname, True)
  #      a.putchanimage(cubimage=imroot+'.model', inim=imname+'.model', 
  #                     chan=imchan)
  #     a.putchanimage(cubimage=imroot+'.residual', inim=imname+'.residual', 
  #                    chan=imchan) 
  #     a.putchanimage(cubimage=imroot+'.image', inim=imname+'.image', 
  #                  chan=imchan)
        ###need to clean up here the channel image...will do after testing phase

    def imagechan_selfselect(self, msname='spw00_4chan351rowTile.ms', spwids=[0], field=0, imroot='newmodel', imchan=0, niter=100, alg='clark', thr='0.0mJy', mask='', majcycle=1, scales=[0],  chanchunk=1):
        ###need generate fstart, width from 1 channel width, start, numchan and spw 
        origname=msname
        ia.open(imroot+'.model')
        csys=ia.coordsys()
        fstart=csys.toworld([0,0,0,imchan*chanchunk],'n')['numeric'][3]
        fstep=csys.toworld([0,0,0,imchan*chanchunk+1],'n')['numeric'][3]-fstart
        fend=fstep*(chanchunk-1)+fstart
        spw, start, nchan=self.findchanselLSRK(msname=msname, spw=spwids, 
                                                      field=field, 
                                                      numpartition=1, 
                                                      beginfreq=fstart, endfreq=fend, chanwidth=fstep)
        if((len(spw[0])==0) or (len(nchan[0])==0) or (len(start[0]) ==0) ):
            return
        imname=imroot+str(imchan)
        maskname=''
        if(mask != ''):
            maskname=imname+'.mask'
            if( not self.makemask(inmask=mask, outmask=maskname, imchan=imchan, chanchunk=chanchunk)):
                maskname=''
 #       a.getchanimage(cubeimage=imroot+'.model', outim=imname+'.model', chan=imchan)
        im.selectvis(vis=msname, field=field, spw=spw[0], nchan=nchan[0], start=start[0], step=1, datainmemory=self.visInMem)
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets, mode='frequency', nchan=chanchunk, start=str(fstart)+'Hz', step=str(fstep)+'Hz')
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
       
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True)
        im.setscales(scalemethod='uservector', uservector=scales)
        im.setmfcontrol(cyclefactor=0.0)  
        majcycle = majcycle if (niter/majcycle) >0 else 1
        try:
            
            for kk in range(majcycle):
                im.clean(algorithm=alg, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', mask=maskname, psfimage='')
            im.done()
        except Exception as instance:
                if(string.count(str(instance), 'PSFZero') <1):
                    raise Exception(instance)
        if(maskname != ''):
            shutil.rmtree(maskname, True)

    def imagechan_new(self, msname='spw00_4chan351rowTile.ms', start=0, numchan=1, spw=0, field=0, cubeim='imagecube', imroot='newmodel', imchan=0, chanchunk=1, niter=100, alg='clark', thr='0.0mJy', mask='', majcycle=1, scales=[0]):
        origname=msname
#        a=cleanhelper()
        imname=imroot+str(imchan)
        maskname=''
        if(mask != ''):
            maskname=imname+'.mask'
            if( not self.makemask(inmask=mask, outmask=maskname, imchan=imchan, chanchunk=chanchunk)):
                maskname=''
        self.getchanimage(inimage=cubeim+'.model', outimage=imname+'.model', chan=imchan*chanchunk, nchan=chanchunk)
        im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem)
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets)
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True)
        im.setscales(scalemethod='uservector', uservector=scales)
        im.setmfcontrol(cyclefactor=0.0)  
        majcycle = majcycle if (niter/majcycle) >0 else 1
        for kk in range(majcycle):
            im.clean(algorithm=alg, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', 
                     mask=maskname, psfimage='')
        im.done()
        if(maskname != ''):
            shutil.rmtree(maskname, True)
        #self.putchanimage(cubeim+'.model', imname+'.model', imchan*chanchunk)
        #self.putchanimage(cubeim+'.residual', imname+'.residual', imchan*chanchunk)
        #self.putchanimage(cubeim+'.image', imname+'.image', imchan*chanchunk)


    @staticmethod
    def getallchanmodel(inimage, chanchunk=1):
        #tim1=time.time()
        ia.open(inimage+'.model')
        modshape=ia.shape()
        nchan=modshape[3]/chanchunk;
        if(nchan >1):
            blc=[0,0,modshape[2]-1,0]
            trc=[modshape[0]-1,modshape[1]-1,modshape[2]-1,0+chanchunk]
            for k in range(nchan):
                #timbeg=time.time()
                blc[3]=k*chanchunk
                trc[3]=k+chanchunk-1
                if((trc[3]) >= modshape[3]):
                    trc[3]=modshape[3]-1
                sbim=ia.subimage(outfile=inimage+str(k)+'.model', region=rg.box(blc,trc), overwrite=True)
                sbim.done()   
                #print 'time taken for ', k, time.time()-timbeg
        ia.done()
        tb.clearlocks()

    @staticmethod
    def getchanimage(inimage, outimage, chan, nchan=1):
        """
        create a slice of channels image from cubeimage
        """
    #pdb.set_trace()
        ia.open(inimage)
        modshape=ia.shape()
        if (modshape[3]==1) or (chan > (modshape[3]-1)) :
            return False
        if((nchan+chan) < modshape[3]):
            endchan= chan+nchan-1
        else:
            endchan=modshape[3]-1
        blc=[0,0,modshape[2]-1,chan]
        trc=[modshape[0]-1,modshape[1]-1,modshape[2]-1,endchan]
        sbim=ia.subimage(outfile=outimage, region=rg.box(blc,trc), overwrite=True)
        sbim.done()
        ia.done()
        tb.clearlocks()
        #casalog.post('getLOCKS:  '+ str(inimage)+ ' ---  ' + str(tb.listlocks()))
        return True
    #getchanimage = staticmethod(getchanimage)
    def cleanupcubeimages(self, readyputchan, doneputchan, imagename, nchanchunk, chanchunk):
        """
        This function will put the True values of readyputchan into the final cubes and set the doneputchan to True
        Confused ..read the code 
        """
        for k in range(nchanchunk):
            if(readyputchan[k] and (not doneputchan[k])):
                self.putchanimage(imagename+'.model', imagename+str(k)+'.model', k*chanchunk, False)
                self.putchanimage(imagename+'.residual', imagename+str(k)+'.residual', k*chanchunk, False)
                self.putchanimage(imagename+'.image', imagename+str(k)+'.image', k*chanchunk, False)
                doneputchan[k]=True
    def cleanupmodelimages(self, readyputchan,  imagename, nchanchunk, chanchunk):
        """
        This function will put model images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.model', imagename+str(k)+'.model', k*chanchunk, True)
    
    def cleanupresidualimages(self, readyputchan,  imagename, nchanchunk, chanchunk):
        """
        This function will put residual images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.residual', imagename+str(k)+'.residual', k*chanchunk, True)
    
    def cleanuprestoredimages(self, readyputchan,  imagename, nchanchunk, chanchunk):
        """
        This function will put residual images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.image', imagename+str(k)+'.image', k*chanchunk, True) 
                
    @staticmethod
    def putchanimage(cubimage,inim,chans, removeinfile=True):
        """
        put channels image back to a pre-exisiting cubeimage 
        """
        #pdb.set_trace()
        if(type(chans) != list):
            chans=[chans]
        if(type(inim) != list):
            inim=[inim]
        if( not os.path.exists(inim[0])):
            return False
        ia.open(inim[0])
        inimshape=ia.shape()
        ############
        #imdata=ia.getchunk()
        #immask=ia.getchunk(getmask=True)
        ##############
        ia.done()
        ia.open(cubimage)
        cubeshape=ia.shape()
        if inimshape[0:3]!=cubeshape[0:3]: 
                return False
        k=0
        for chan in chans:
            blc=[0,0,0,chan]
            trc=[inimshape[0]-1,inimshape[1]-1,inimshape[2]-1,chan+inimshape[3]-1]
            if( not (cubeshape[3] > (chan+inimshape[3]-1))):
                return False

            ############
            #rg0=ia.setboxregion(blc=blc,trc=trc)
            ###########
            
            ########
            #ia.putregion(pixels=imdata,pixelmask=immask, region=rg0)
            ###########
            ia.insert(infile=inim[k], locate=blc)
            if(removeinfile):
                ia.removefile(inim[k])
            k+=1
        ia.close()
        tb.clearlocks()
        #casalog.post('putLOCKS:  '+ str(inim)+ ' ---  ' + str(tb.listlocks()))
        
        
        return True
    @staticmethod
    def putchanimage2(cubimage,inim,chans, notprocesschan, removeinfile=True):
        """
        put channels image back to a pre-exisiting cubeimage 
        """
        if(type(chans) != list):
            chans=[chans]
        if(type(inim) != list):
            inin=[inim]
        if(type(notprocesschan) != list):
            notprocesschan=[notprocesschan]
        #if( not os.path.exists(inim[0])):
        #   return False
        ibig, = gentools(['ia'])
        #ia.open(inim[0])
        #inimshape=ia.shape()
        ############
        #imdata=ia.getchunk()
        #immask=ia.getchunk(getmask=True)
        ##############
        #ia.done()
        ibig.open(cubimage)
        cubeshape=ibig.shape()
        #if inimshape[0:3]!=cubeshape[0:3]: 
        #        return False
        arr=ibig.getchunk()
        k=0
        for chan in chans:
            if(not notprocesschan[k] and os.path.exists(inim[k])):
                ia.open(inim[k])
                inimshape=ia.shape()
                arrsub=ia.getchunk()
                ia.done()
               #blc=[0,0,0,chan]
               #trc=[inimshape[0]-1,inimshape[1]-1,inimshape[2]-1,chan+inimshape[3]-1]
               #if( not (cubeshape[3] > (chan+inimshape[3]-1))):
               #   return False
                arr[:,:,:,chan:chan+inimshape[3]]=arrsub

            ############
            #rg0=ia.setboxregion(blc=blc,trc=trc)
            ###########
            
            ########
            #ia.putregion(pixels=imdata,pixelmask=immask, region=rg0)
            ###########
            #ia.insert(infile=inim[k], locate=blc)
            if(removeinfile):
                shutil.rmtree(inim[k], True)
            k+=1
        ibig.putchunk(arr)
        ibig.close()
        tb.clearlocks()
        #casalog.post('putLOCKS:  '+ str(inim)+ ' ---  ' + str(tb.listlocks()))
        
        
        return True
    @staticmethod
    def makemask(inmask='', outmask='' , imchan=0, chanchunk=1):
        if(not os.path.exists(inmask)):
            return False
        ia,rg=gentools(['ia', 'rg'])
        ia.open(inmask)
        shp=ia.shape()
        if(shp[3]==1):
            shutil.rmtree(outmask, True)
            shutil.copytree(inmask, outmask)
            ia.done()
            return True
        if(shp[3] > ((imchan+1)*chanchunk-1)):
            mybox=rg.box(blc=[0, 0, 0, imchan*chanchunk], trc=[shp[0]-1, shp[1]-1, shp[2]-1, imchan*chanchunk+chanchunk-1])
            ia.subimage(outfile=outmask, region=mybox, overwrite=True)
            ia.done()
            return True
        return False

    @staticmethod
    def findchanselLSRK(msname='', field='*', spw='*', numpartition=1, beginfreq=0.0, endfreq=1e12, chanwidth=0.0):
        im,ms=gentools(['im', 'ms'])
        fieldid=0
        if(type(field)==str):
            fieldid=ms.msseltoindex(vis=msname, field=field)['field'][0]
        elif(type(field)==int):
            fieldid=field
        #im.selectvis(vis=msname, field=field, spw=spw)
        partwidth=(endfreq-beginfreq)/float(numpartition)
        spwsel=[]
        nchansel=[]
        startsel=[]
        for k in range(numpartition):
            time0=time.time()
            a=im.advisechansel(freqstart=(beginfreq+float(k)*partwidth), freqend=(beginfreq+float(k+1)*partwidth), freqstep=chanwidth, freqframe='LSRK', msname=msname, fieldid=fieldid)
            if(a['ms_0']['spw'].tolist() != []):
                spwsel.append(a['ms_0']['spw'].tolist())
                nchansel.append(a['ms_0']['nchan'].tolist())
                startsel.append(a['ms_0']['start'].tolist())
            #print k, 'fstart',  (beginfreq+float(k)*partwidth), 'fend' , (beginfreq+float(k+1)*partwidth)
            print 'TIme taken for lsrk calc', time.time()-time0
        im.done()
        del im
        return spwsel, startsel, nchansel
        
    #putchanimage=staticmethod(putchanimage)
    def makecontimage(self, im, novaliddata, imname):
        if(novaliddata==True):
            ###make blanks
            im.make(imname+'.image')
            im.make(imname+'.residual')
            im.make(imname+'.model')
            im.make(imname+'.psf')
            return
        if(not self.imageparamset):
            try:
                im.clean(algorithm='mfclark', niter=0, threshold='0.05mJy', 
                         model=imname+'.model', image=imname+'.image', 
                         residual=imname+'.residual', psfimage=imname+'.psf')
            except Exception, instance:
                if(string.count(instance.message, 'PSFZero') >0):
                    novaliddata=True
                    ###make a blank image
                    im.make(imname+'.image')
                else:
                    raise instance
        else:  ### else of  (if not self.imageparamset)
            if(not novaliddata):
                #casalog.post('Updating '+msname+' imname '+imname)
                im.updateresidual(model=imname+'.model',  image=imname+'.image', 
                                  residual=imname+'.residual')
                #im.clean(algorithm='mfclark', niter=0, threshold='0.05mJy', 
                 #        model=imname+'.model', image=imname+'.image', 
                 #        residual=imname+'.residual')
                #casalog.post('CACHE:  '+ str(tb.showcache()))
        
