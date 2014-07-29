import os
import pdb
from taskinit import *
import time
import numpy as np
from cleanhelper import *
from casac import casac
class imagecont():
    def __init__(self, ftmachine='ft', wprojplanes=10, facets=1, pixels=[3600, 3600], cell=['3arcsec', '3arcsec'], spw='', 
                 field='', phasecenter='', weight='natural', robust=0.0, stokes='I', npixels=0, uvtaper=False, outertaper=[], 
                 timerange='', uvrange='', baselines='', scan='', observation='', gain=0.1, numthreads=-1, pbcorr=False, 
                 minpb=0.2, cyclefactor=1.5,
                 painc=360., pblimit=0.1, dopbcorr=True, applyoffsets=False, cfcache='cfcache.dir',
                 epjtablename='',mterm=True,wbawp=True,aterm=True,psterm=True,conjbeams=True):

        self.im=casac.imager()
        self.imperms={}
        self.dc=casac.deconvolver()
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
        self.weightnpix=npixels
        self.stokes=stokes
        self.numthreads=numthreads
        self.imagetilevol=1000000
        self.visInMem=False
        self.cyclefactor=cyclefactor
        self.novaliddata={}
        self.gain=gain
        self.uvtaper=uvtaper
        self.outertaper=outertaper
        self.timerange=timerange
        self.uvrange=uvrange
        self.baselines=baselines
        self.scan=scan
        self.observation=observation
        self.pbcorr=pbcorr
        self.minpb=minpb

        self.painc=painc;
        self.pblimit=pblimit;
        self.dopbcorr=dopbcorr;
        self.applyoffsets=applyoffsets;
        self.cfcache=cfcache;
        self.epjtablename=epjtablename;
        self.mterm=mterm;
        self.aterm=aterm;
        self.psterm=psterm;
        self.wbawp=wbawp;
        self.conjbeams=conjbeams;

    def setextraoptions(self, im, cyclefactor=1.5, fluxscaleimage='', scaletype='SAULT'):
        im.setmfcontrol(scaletype=scaletype, fluxscale=[fluxscaleimage], 
                        cyclefactor=cyclefactor, minpb=self.minpb)
        if(self.pbcorr):
            im.setvp(True)
            im.setmfcontrol(scaletype='NONE', cyclefactor=cyclefactor, minpb=self.minpb, fluxscale=[fluxscaleimage])
            
    def setparamcont(self, im, freq, band, singleprec=False):
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], 
                       celly=self.cell[1], phasecenter=self.phCen, mode='frequency', 
                       nchan=1, start=freq, step=band, facets=self.facets, 
                       stokes=self.stokes);

        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, 
                      imagetilevol=self.imagetilevol,
                      singleprecisiononly=singleprec, numthreads=self.numthreads,
                      pastep=self.painc, pblimit=self.pblimit, dopbgriddingcorrections=self.dopbcorr, 
                      applypointingoffsets=self.applyoffsets, cfcachedirname=self.cfcache,
                      epjtablename=self.epjtablename,
                      mterm=self.mterm, wbawp=self.wbawp, aterm=self.aterm, 
                      psterm=self.psterm, conjbeams=self.conjbeams)

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
                      im.selectvis(vis=msnames[k], field=field[k], spw=spw[k], nchan=numchan[k], start=start[k], step=1, datainmemory=self.visInMem, time=self.timerange, uvrange=self.uvrange, baseline=self.baselines, scan=self.scan, observation=self.observation, writeaccess=False)
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
        self.novaliddata=self.makecontimage(im, self.novaliddata, imname)
        self.imageparamset=True



 #### uniform and brigg's weight combination functions
    def getweightgrid(self, msname='', wgttype='imaging'):
        if(self.imperms.has_key(msname) and (not self.novaliddata[msname])):
            return self.imperms[msname].getweightgrid(type=wgttype)
    def setweightgrid(self, msname='', weight=[], wgttype='imaging'):
        if(self.imperms.has_key(msname) and (not self.novaliddata[msname])):
            self.imperms[msname].setweightgrid(weight=weight, type=wgttype)
    def getftweight(self, msname='', wgtimage=['']):
        if(self.imperms.has_key(msname) and (not self.novaliddata[msname])):
            self.imperms[msname].getweightgrid(type='ftweight', wgtimages=wgtimage)
#### 
    def imagecont(self, msname='spw00_4chan351rowTile.ms', start=[0], numchan=[-1], spw='', field=0, freq='1.20GHz', band='200MHz', imname='newmodel', nterms=1, scales=[0],timerange=''):
        #casalog.post('KEYS '+str(self.imperms.keys()))
        if(not self.imperms.has_key(msname)):
            self.imageparamset=False
            im=casac.imager()
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
                im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem, 
                             time=timerange, uvrange=self.uvrange, baseline=self.baselines, scan=self.scan, 
                             observation=self.observation, writeaccess=False)
                if(self.uvtaper):
                    im.filter(type='gaussian', bmaj=self.outertaper[0],
                               bmin=self.outertaper[1], bpa=self.outertaper[2])
            except Exception, instance:
                ###failed to selectdata
                self.novaliddata[msname]=True
####
        #imname=imname+'_%02d'%(j)               
            self.setparamcont(im, freq, band)
            if(self.ft=='mosaic'):
                self.setextraoptions(im,  fluxscaleimage=imname+'.flux', scaletype='SAULT')
            if((len(numchan)==0) or (np.sum(numchan)==0)):
                self.novaliddata[msname]=True
        if(nterms==1):
            self.novaliddata[msname]=self.makecontimage(im, self.novaliddata[msname], imname)
        else:
            self.makemtcontimage(im, imname, nterms, scales, freq)
        self.imageparamset=True

    def imagecontbychan(self, msname='spw00_4chan351rowTile.ms', start=[0], numchan=[1], spw=[0], field=0, freq='1.20GHz', band='200MHz', imname='newmodel'):
        ia=casac.image()
        if(type(spw) == int):
            spw=[spw]
            start=[start]
            numchan=[numchan]
        totchan=np.sum(numchan)
        if(not self.imageparamset):
            if(totchan==0):
                ###make blanks
                self.im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, time=self.timerange, uvrange=self.uvrange, baseline=self.baselines, scan=self.scan, observation=self.observation,writeaccess=False)
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
                self.im.append(casac.imager())
                self.novaliddata.append(False)
            print 'selkey', selkey
        else:
             if(totchan==0):
                 return
        origname=msname
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
                    self.im[k].selectvis(vis=msname, field=field, spw=selkey[k]['spw'], nchan=1, start=selkey[k]['start'], step=1, datainmemory=self.visInMem, time=self.timerange, uvrange=self.uvrange, baseline=self.baselines, scan=self.scan, observation=self.observation, writeaccess=False)
                    self.im[k].defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, mode='frequency', nchan=1, start=freq, step=band, facets=self.facets)
                    self.im[k].weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                              robust=self.robust)
                    self.im[k].setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, pastep=self.painc, pblimit=self.pblimit, cfcachedirname=self.cfcache, dopbgriddingcorrections=self.dopbcorr, applypointingoffsets=self.applyoffsets, imagetilevol=self.imagetilevol, singleprecisiononly=True, numthreads=self.numthreads)
               #im.regionmask(mask='lala.mask', boxes=[[0, 0, 3599, 3599]])
               #im.setmfcontrol(cyclefactor=0.0)
                if(not self.imageparamset):
                    try:
                        self.im[k].clean(algorithm='clark', gain=self.gain, niter=0, model=imname+'.model', image=image, residual=residual, psfimage=psf)
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
        dc=casac.deconvolver()
        dc.open(dirty=dirty, psf=psf)
        if((alg=='hogbom') or (alg == 'msclean') or (alg == 'multiscale')):
            sca=scales if ((alg=='msclean') or (alg=='multiscale')) else [0]
            dc.setscales(scalemethod='uservector', uservector=sca)
            alg='fullmsclean'
        retval={}
        if(alg=='clark'):
            retval=dc.fullclarkclean(niter=niter, gain=self.gain, threshold=thr, model=model, mask=mask)
        else:
            retval=dc.clean(algorithm=alg, gain=self.gain, niter=niter, threshold=thr, model=model, mask=mask)
        dc.done()
        del dc
        return retval

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
        im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem, time=self.timerange, uvrange=self.uvrange, baseline=self.baselines, scan=self.scan, observation=self.observation, writeaccess=False)
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets, mode='frequency', nchan=chanchunk, start=fstart, step=width)
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
       
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True, numthreads=self.numthreads)
        im.setscales(scalemethod='uservector', uservector=scales)
        self.setextraoptions(im, cyclefactor=0.0)
        majcycle = majcycle if (niter/majcycle) >0 else 1
        for kk in range(majcycle):
            im.clean(algorithm=alg, gain=self.gain, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', mask=maskname, psfimage='')
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

    def imagechan_selfselect(self, msname='spw00_4chan351rowTile.ms', spwids=[0], field=0, imroot='newmodel', imchan=0, niter=100, alg='clark', thr='0.0mJy', mask='', majcycle=1, scales=[0],  chanchunk=1, startchan=0):
        ###need generate fstart, width from 1 channel width, start, numchan and spw 
        origname=msname
        retval={}
        retval['maxresidual']=0.0
        retval['iterations']=0
        retval['converged']=False
        ia.open(imroot+'.model')
        csys=ia.coordsys()
        ia.done()
        im=casac.imager()  
        fstart=csys.toworld([0,0,0,startchan],'n')['numeric'][3]
        fstep=csys.toworld([0,0,0,startchan+1],'n')['numeric'][3]-fstart
        fend=fstep*(chanchunk-1)+fstart
        print 'fstat bef findchansel ', fstart
        spw, start, nchan=self.findchanselLSRK(msname=msname, spw=spwids, 
                                                      field=field, 
                                                      numpartition=1, 
                                                      beginfreq=fstart, endfreq=fend, chanwidth=fstep)
        print 'spw, start, nchan', spw, start, nchan
        if((len(spw[0])==0) or (len(nchan[0])==0) or (len(start[0]) ==0) ):
            return retval
        imname=imroot+str(imchan)
        maskname=''
        if(mask != ''):
            maskname=imname+'.mask'
            if( not self.makemask(inmask=mask, outmask=maskname, imchan=imchan, chanchunk=chanchunk, startchan=startchan)):
                maskname=''
 #       a.getchanimage(cubeimage=imroot+'.model', outim=imname+'.model', chan=imchan)
        im.selectvis(vis=msname, field=field, spw=spw[0], nchan=nchan[0], start=start[0], step=1, datainmemory=self.visInMem, time=self.timerange, uvrange=self.uvrange, baseline=self.baselines, scan=self.scan, observation=self.observation, writeaccess=False)
        print 'fstart bef def', fstart
        if(self.uvtaper):
            im.filter(type='gaussian', bmaj=self.outertaper[0],
                      bmin=self.outertaper[1], bpa=self.outertaper[2])
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets, mode='frequency', nchan=chanchunk, start=str(fstart)+'Hz', step=str(fstep)+'Hz', outframe='LSRK')
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
       
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True, numthreads=self.numthreads)
        im.setscales(scalemethod='uservector', uservector=scales)
        fluxscaleimage=imname+'.flux' if(self.ft=='mosaic') else ''
        majcycle = majcycle if (niter/majcycle) >0 else 1
        self.setextraoptions(im, fluxscaleimage=fluxscaleimage, cyclefactor=(0.0 if(majcycle >1) else self.cyclefactor)) 
        #### non simple ft machine...should use mf
        if((self.ft != 'ft') or ((majcycle <= 1) and (self.cyclefactor > 0))):
            alg='mf'+alg
        try:
            
            for kk in range(majcycle):
                retval=im.clean(algorithm=alg, gain=self.gain, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', mask=maskname, psfimage=imname+'.psf') 
                if(not os.path.exists(imname+'.image')):
                ##all channels flagged for example...make 0 images for concat later
                    im.make(imname+'.image')
                    im.make(imname+'.model')
                    im.make(imname+'.residual')
                    im.make(imname+'.psf')
            im.done()
            del im
        except Exception as instance:
            if(not os.path.exists(imname+'.image')):
                ##all channels flagged for example...make 0 images for concat later
                im.make(imname+'.image')
                im.make(imname+'.model')
                im.make(imname+'.residual')
                im.make(imname+'.psf')
            im.done()
            del im
            if(string.count(str(instance), 'PSFZero') <1):
                raise Exception(instance)
        if(maskname != ''):
            shutil.rmtree(maskname, True)
        
        #self.putchanimage(imroot+'.model', imname+'.model', startchan, False)
        #self.putchanimage(imroot+'.residual', imname+'.residual', startchan, False)
        #self.putchanimage(imroot+'.image', imname+'.image', startchan, False)
        #self.putchanimage(imroot+'.psf', imname+'.psf', startchan, False)
        return retval


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
        im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem, time=self.timerange, uvrange=self.uvrange, baseline=self.baselines, scan=self.scan, observation=self.observation, writeaccess=False)
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets)
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True, numthreads=self.numthreads)
        im.setscales(scalemethod='uservector', uservector=scales)
        self.setextraoptions(im, cyclefactor=0.0)  
        majcycle = majcycle if (niter/majcycle) >0 else 1
        for kk in range(majcycle):
            im.clean(algorithm=alg, gain=self.gain, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', 
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
                trc[3]=(k+1)*chanchunk-1
                if((trc[3]) >= modshape[3]):
                    trc[3]=modshape[3]-1
                sbim=ia.subimage(outfile=inimage+str(k)+'.model', region=rg.box(blc,trc), overwrite=True,list=False)
                sbim.done()   
                #print 'time taken for ', k, time.time()-timbeg
        ia.done()
        tb.clearlocks()

    @staticmethod
    def getchanimage(inimage, outimage, chan, nchan=1):
        """
        create a slice of channels image from cubeimage
        """
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
        sbim=ia.subimage(outfile=outimage, region=rg.box(blc,trc), overwrite=True,list=False)
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

    def cleanupimages(self, readyputchan, imagename, nchanchunk, chanchunk, removefile=True):
        self.cleanupmodelimages(readyputchan, imagename, nchanchunk, chanchunk, removefile)
        self.cleanupresidualimages(readyputchan, imagename, nchanchunk, chanchunk, removefile)
        self.cleanuprestoredimages(readyputchan, imagename, nchanchunk, chanchunk, removefile)
        
    def cleanupmodelimages(self, readyputchan,  imagename, nchanchunk, chanchunk, removefile=True):
        """
        This function will put model images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.model', imagename+str(k)+'.model', k*chanchunk, removefile)
    def cleanupresidualimages(self, readyputchan,  imagename, nchanchunk, chanchunk, removefile=True):
        """
        This function will put residual images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.residual', imagename+str(k)+'.residual', k*chanchunk, removefile)

    def cleanuprestoredimages(self, readyputchan,  imagename, nchanchunk, chanchunk, removefile=True):
        """
        This function will put residual images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.image', imagename+str(k)+'.image', k*chanchunk, removefile) 
                
    @staticmethod
    def concatimages(cubeimage, inim, csys=None, removeinfile=True):
        if((csys==None) and os.path.exists(cubeimage)):
            ia.open(cubeimage)
            csys=ia.coordsys()
            ia.done()
        if(type(inim) != list):
            inim=[inim]
        ###Temporary bypass of  CAS-4423
        #elbeamo={}
        #for elim in inim:
        #    ia.open(elim)
        #   beam=ia.restoringbeam()
            #if( (len(beam) > 0) and (not beam.has_key('beams'))):
            #    ia.setrestoringbeam(remove=True)
            #   nchan=ia.shape()[3]
            #    for k in range(nchan):
            #        ia.setrestoringbeam(major=beam['major'], minor=beam['minor'], pa=beam['positionangle'], channel=k, polarization=0)
         #   ia.setrestoringbeam(remove=True)
         #  elbeamo=beam if(not beam.has_key('beams')) else  beam['beams']['*0']['*0']
         #  ia.done()
        ####                
        if(len(inim)==1):
            ib=ia.fromimage(outfile=cubeimage, infile=inim[0], overwrite=True)
        else:
            #ib=ia.imageconcat(outfile=cubeimage, infiles=inim,  
            #                  axis=3, relax=True,  overwrite=True)
            ############
            strnames='"'
            for k in range(len(inim)):
                strnames=strnames+' '+inim[k] if (k > 0) else strnames+inim[k] 
            strnames=strnames+'"'
            comm='imageconcat '+ strnames + ' '+ cubeimage
            casalog.post('Command '+comm)
            exitval=os.system(comm)
            #print "exitval", exitval
            if(exitval > 0):
                raise Exception('Failed to concat '+str(inim))  
            ib=casac.image()
            ib.open(cubeimage)
            ##############
        ia.done()
        ##### CAS-4423 temp
        #if(len(elbeamo) >0):
        #   ib.setrestoringbeam(beam=elbeamo)
        #####
        if(csys != None):
            if(type(csys)==dict):
                ib.setcoordsys(csys=csys)
            else:
                ib.setcoordsys(csys=csys.torecord())
        ib.done()
        if(removeinfile):
            for k in range(len(inim)):
                ia.removefile(inim[k])
        
    @staticmethod
    def putchanimage(cubimage,inim,chans, removeinfile=True):
        """
        put channels image back to a pre-exisiting cubeimage 
        """
        if(type(chans) != list):
            chans=[chans]
        if(type(inim) != list):
            inim=[inim]
        #if( not os.path.exists(inim[0])):
        #    return False
        #ia.open(inim[0])
        #inimshape=ia.shape()
        ############
        #imdata=ia.getchunk()
        #immask=ia.getchunk(getmask=True)
        ##############
        ia,=gentools(['ia'])
        ia.open(cubimage)
        cubeshape=ia.shape()
        #if inimshape[0:3]!=cubeshape[0:3]: 
        #        return False
        k=0
        ib,=gentools(['ia'])
        for chan in chans:
            casalog.post('Doing chan_' + str(chan))
            if(os.path.exists(inim[k])):
                ib.open(inim[k])
                inimshape=ib.shape()
                #############3
                imdata=ib.getchunk()
                immask=ib.getchunk(getmask=True)
############
                ib.done()
                blc=[0,0,0,chan]
                trc=[inimshape[0]-1,inimshape[1]-1,inimshape[2]-1,chan+inimshape[3]-1]
                if( not (cubeshape[3] > (chan+inimshape[3]-1))):
                    return False

            ############
                rg0=ia.setboxregion(blc=blc,trc=trc)
            ###########
            
            ########
                #ia.putregion(pixels=imdata,pixelmask=immask, region=rg0)
            ###########
                ia.insert(infile=inim[k], locate=blc,verbose=False)
                if(removeinfile):
                    ia.removefile(inim[k])
            k+=1
        ia.done()
        del ia
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

        ####get the shape of an image
        for k in range(len(inim)):
            if(os.path.exists(inim[k])):
                break
        ia.open(inim[k])
        subshape=ia.shape()
        ia.done()
        ##############

        ibig.open(cubimage)
        cubeshape=ibig.shape()
        chanstart=0
        blc=np.array([0,0,0,0])
        #if inimshape[0:3]!=cubeshape[0:3]: 
        #        return False
        nchTile=1
        if((ibig.summary()['tileshape'][3]%subshape[3] ==0) or (subshape[3] > ibig.summary()['tileshape'][3])):
            nchTile=max(ibig.summary()['tileshape'][3], subshape[3])
        else:
            nchTile=subshape[3]
        trc=np.array([int(cubeshape[0]-1),int(cubeshape[1]-1),int(cubeshape[2]-1),nchTile-1])
        arr=ibig.getchunk(blc=blc.tolist(), trc=trc.tolist())
        nchanchunk=cubeshape[3]/nchTile
        nreschan=cubeshape[3]%nchTile
        k=0
        chanchunk=1
        for chan in chans:
            if(chan >= chanchunk*nchTile):
                ibig.putchunk(arr, blc.tolist())
                blc[3]=chanchunk*nchTile
                chanchunk+=1
                if(chanchunk< nchanchunk):
                    trc[3]=chanchunk*nchTile-1
                else:
                    trc[3]=nchanchunk*nchTile+nreschan-1
                arr=ibig.getchunk(blc=blc.tolist(), trc=trc.tolist())
                #print 'new slice', blc, trc
            if(not notprocesschan[k] and os.path.exists(inim[k])):
                ia.open(inim[k])
                inimshape=ia.shape()
                arrsub=ia.getchunk()
                ia.done()
               #blc=[0,0,0,chan]
               #trc=[inimshape[0]-1,inimshape[1]-1,inimshape[2]-1,chan+inimshape[3]-1]
               #if( not (cubeshape[3] > (chan+inimshape[3]-1))):
               #   return False
                relchan=chan-(chanchunk-1)*nchTile
                arr[:,:,:,relchan:relchan+inimshape[3]]=arrsub
                #print 'chan', chan
                arrsub=[]
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
        ibig.putchunk(arr, blc.tolist())
        ibig.unlock()
        ibig.close()
        tb.clearlocks()
        #casalog.post('putLOCKS:  '+ str(inim)+ ' ---  ' + str(tb.listlocks()))
        
        
        return True
    @staticmethod
    def makemask(inmask='', outmask='' , imchan=0, chanchunk=1, startchan=-1):
        if(startchan < 0):
            startchan=imchan*chanchunk
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
        if(shp[3] > (startchan+chanchunk-1)):
            mybox=rg.box(blc=[0, 0, 0, startchan], trc=[shp[0]-1, shp[1]-1, shp[2]-1, startchan+chanchunk-1])
            ia.subimage(outfile=outmask, region=mybox, overwrite=True,list=False)
            ia.done()
            return True
        return False

    @staticmethod
    def findchanselLSRK(msname='', field='*', spw='*', numpartition=1, beginfreq=0.0, endfreq=1e12, chanwidth=0.0):
        im,ms=gentools(['im', 'ms'])
        if(field==''):
            field='*'
        fieldid=0
        fieldids=[]
        if(type(field)==str):
            fieldids=ms.msseltoindex(vis=msname, field=field)['field']
        elif(type(field)==int):
            fieldids=[field]
        elif(type(field)==list):
            fieldids=field
            
        #im.selectvis(vis=msname, field=field, spw=spw)
        partwidth=(endfreq-beginfreq)/float(numpartition)
        spwsel=[]
        nchansel=[]
        startsel=[]
        for k in range(numpartition):
            time0=time.time()
            a={}
            a['ms_0']={}
            a['ms_0']['spw']=np.array([])
            fid=0
            while((a['ms_0']['spw'].tolist()==[]) and (fid < len(fieldids))):
                a=im.advisechansel(freqstart=(beginfreq+float(k)*partwidth), freqend=(beginfreq+float(k+1)*partwidth), freqstep=chanwidth, freqframe='LSRK', msname=msname, fieldid=fieldids[fid])
                fid+=1
                  
                  
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
        makeblanks=lambda imn,im: [im.make(imn+eltypo) for eltypo in ['.image', '.residual', '.model', '.psf', '.wgt'] ]
        if(novaliddata==True):
            ###make blanks
            ###The images may still be open...as we don't have 
                    ###a reset function; setscales is the closest which will destroy
                    ###the skyequation without much of anything else.
            im.setscales()
            makeblanks(imname, im)
            return novaliddata
        if(not self.imageparamset):
            try:
                im.clean(algorithm='mfclark', gain=self.gain, niter=0, threshold='0.05mJy', 
                         model=imname+'.model', image=imname+'.image', 
                         residual=imname+'.residual', psfimage=imname+'.psf')
                ia=casac.image()
                for ima in [imname+'.residual', imname+'.psf']:
                    ia.open(ima)
                    statout=ia.statistics(verbose=False, list=False)
                    ia.done()
                    if(statout['max'][0]==0.0):
                        novaliddata=True
            except Exception, instance:
                if(string.count(instance.message, 'PSFZero') >0):
                    novaliddata=True
                    ###The images may still be open...as we don't have 
                    ###a reset function; setscales is the closest which will destroy
                    ###the skyequation without much of anything else.
                    im.setscales()
                    ###make a blank image
                    makeblanks(imname, im)
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
        return novaliddata
    def makemtcontimage(self, im, imname, nterms, scales, reffreq):
        incremental=self.imageparamset
        models=[]
        psfs=[]
        residuals=[]
        restoreds=[]
        sumwts=[]
        npsftaylor = 2 * nterms - 1
        for tt in range(nterms):
            models.append(imname+'.model.tt'+str(tt));
            residuals.append(imname+'.residual.tt'+str(tt));
            restoreds.append(imname+'.image.tt'+str(tt));
            sumwts.append(imname+'.sumwt.tt'+str(tt));
            if(not os.path.exists(models[tt])):
                im.make(models[tt]);
        for tt in range(0,npsftaylor):
              psfs.append(imname+'.psf.tt'+str(tt));
        #this is a dummy i believe as deconvolution is done else where
        im.setscales(scalemethod='uservector',uservector=scales);
        im.settaylorterms(ntaylorterms=nterms,
                          reffreq=(qa.convert(qa.unit(reffreq),"Hz"))['value'])
        # im.setoptions(ftmachine=self.ft,pastep=self.painc,
	# 	      cfcachedirname=self.cfcache,
	# 	      applypointingoffsets= self.applyoffsets,
	# 	      dopbgriddingcorrections=self.dopbcorr,
	# 	      pblimit=self.pblimit,
	#               wprojplanes=self.wprojplanes,aterm=self.aterm,psterm=self.psterm,wbawp=self.wbawp,conjbeams=self.conjbeams);
        im.clean(model=models,image=restoreds,psfimage=psfs[0:nterms], residual=residuals,  algorithm='msmfs',niter=-1)
        if( incremental == False ) :  # first major cycle
           im.getweightgrid(type='ftweight', wgtimages=sumwts)
           ## rename extra psfs....
           ##im.getextrapsfs(multifieldid=0, psfs = psfs[ntaylor:npsftaylor])
           for p in range( nterms, npsftaylor ):
               cmd='mv '+imname+'.TempPsf.'+str(p)+' '+psfs[p];
               print "Rename extra PSFs: ",cmd;
               os.system(cmd);
        #casalog.post('CACHE:  '+ str(tb.showcache()))
        
