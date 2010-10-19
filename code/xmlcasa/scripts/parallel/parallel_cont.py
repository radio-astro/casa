import os
from taskinit import *
import time
import numpy as np
from cleanhelper import *
class imagecont():
    def __init__(self, ftmachine='ft', wprojplanes=10, facets=1, pixels=[3600, 3600], cell=['3arcsec', '3arcsec'], spw='', field='', phasecenter='', weight='natural'):
        self.im=imtool.create()
        self.dc=dctool.create()
        self.ft=ftmachine
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
        self.robust=0.0
        self.weightnpix=0
        self.imagetilevol=1000000
        self.visInMem=False
        self.painc=360.0
        self.pblimit=0.1
        self.dopbcorr=True
        self.novaliddata=False
        self.applyoffsets=False
        self.cfcache='cfcache.dir'
        self.epjtablename=''
    def imagecont(self, msname='spw00_4chan351rowTile.ms', start=0, numchan=1, spw=0, field=0, freq='1.20GHz', band='200MHz', imname='newmodel'):
        im=self.im
        origname=msname
        ###either psf 0 or no channel selected
        if(self.novaliddata):
            return
        #j=start
        #end=start+numchan-1
        #spwstring=str(spw)+':'+str(start)+'~'+str(end)
        #print 'spwstring', spwstring
        msname=origname
        if(not self.imageparamset):
            im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem)
            #im.selectvis(vis=msname, field=field, spw=spwstring, datainmemory=True)
####
        #imname=imname+'_%02d'%(j)
            im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, mode='frequency', nchan=1, start=freq, step=band, facets=self.facets)
            if(np.sum(numchan)==0):
                self.novaliddata=True
                ###make blanks
                im.make(imname+'.image')
                im.make(imname+'.residual')
                im.make(imname+'.model')
                im.make(imname+'.psf')
                return
            im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
            im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, pastep=self.painc, pblimit=self.pblimit, cfcachedirname=self.cfcache, dopbgriddingcorrections=self.dopbcorr, applypointingoffsets=self.applyoffsets, imagetilevol=self.imagetilevol)
        #im.regionmask(mask='lala.mask', boxes=[[0, 0, 3599, 3599]])
        #im.setmfcontrol(cyclefactor=0.0)
        if(not self.imageparamset):
            try:
                im.clean(algorithm='mfclark', niter=0, threshold='0.05mJy', model=imname+'.model', image=imname+'.image', residual=imname+'.residual', psfimage=imname+'.psf')
            except Exception, instance:
                if(string.count(instance.message, 'PSFZero') >0):
                    self.novaliddata=True
                    ###make a blank image
                    im.make(imname+'.image')
                else:
                    raise instance
        else:
            if(not self.novaliddata):
                im.restore(model=imname+'.model',  image=imname+'.image', residual=imname+'.residual')
            
        #im.done()
        self.imageparamset=True
    def cleancont(self, niter=100, alg='clark', thr='0.0mJy', psf='newmodel.psf', dirty='newmodel.dirty', model='newmodel.model', mask='', scales=[0]):
        dc=self.dc
        dc.open(dirty=dirty, psf=psf)
        if((alg=='hogbom') or (alg == 'msclean')):
            sca=scales if (alg=='msclean') else [0]
            dc.setscales(scalemethod='uservector', uservector=sca)
            alg='fullmsclean'
        if(alg=='clark'):
            dc.clarkclean(niter=niter, threshold=thr, model=model, mask=mask)
        else:
            dc.clean(algorithm=alg, niter=niter, threshold=thr, model=model, mask=mask)
        dc.done()

    def imagechan(self, msname='spw00_4chan351rowTile.ms', start=0, numchan=1, spw=0, field=0, imroot='newmodel', imchan=0, niter=100, alg='clark', thr='0.0mJy', mask='', majcycle=1, scales=[0]):
        origname=msname
#        a=cleanhelper()
        imname=imroot+str(imchan)
 #       a.getchanimage(cubeimage=imroot+'.model', outim=imname+'.model', chan=imchan)
        im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem)
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets)
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True)
        im.setscales(scalemethod='uservector', uservector=scales)
        im.setmfcontrol(cyclefactor=0.0)  
        majcycle = majcycle if (niter/majcycle) >0 else 1
        for kk in range(majcycle):
            im.clean(algorithm=alg, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', psfimage='')
        im.done()
  #      a.putchanimage(cubimage=imroot+'.model', inim=imname+'.model', 
  #                     chan=imchan)
  #     a.putchanimage(cubimage=imroot+'.residual', inim=imname+'.residual', 
  #                    chan=imchan) 
  #     a.putchanimage(cubimage=imroot+'.image', inim=imname+'.image', 
  #                  chan=imchan)
        ###need to clean up here the channel image...will do after testing phase
