from casac import casac
import os
import numpy as np
import pdb
iaim=casac.image()
iamask=casac.image()
qa=casac.quanta()
rg=casac.regionmanager()

def automask(image='', maskimage='', fracofpeak=0, rmsthresh=3.0, resolution=None, twopass=False):
    """
    image : dirty image or residual to mask
    maskimage: name of mask to create. If already existant has to be same shape 
    and coordsys as image
    fracofpeak : Fraction of peak to use as masking threshold (a number between 0 and 1)
                       if 0  then rmsthresh or  a value based on the rms of the image is used
    rmsthresh : Threshold in sigma  to use; default is 3 sigma
    resolution: if image has no restoring beam ..this value is used for resolution of 
                     image (e.g '4arcsec')
    twopass: set to True if faint fluffy stuff is wanted else false if a less agressive masking 
    is needed especially a multi pass masking after some cleaning

    """
#    print '2pass ', twopass
    iaim.open(image)
    stat=iaim.statistics(mask='abs("'+image+'") > 0.0')
    csys=iaim.coordsys()
    rb=iaim.restoringbeam()
    numpix=10
    shp=iaim.shape()
    resol=qa.quantity(resolution, 'arcsec')
    if(shp[0] < 10):
        raise TypeError, 'you cannot be serious... less than 10 pixels image'
    if(len(rb) ==0 and resolution==None):
        raise ValueError, 'No restoring beam or resolution given'
    if(len(rb) > 0):
        numpix=2*(int(2.0*abs(qa.div(rb['major'], qa.convert(csys.increment('q')['quantity']['*1'],rb['major']['unit']))['value']))/2)
    elif(resol['value'] != 0.0):
         numpix=2*(int(2.0*abs(qa.div(resol, qa.convert(csys.increment('q')['quantity']['*1'],resol['unit']))['value']))/2)
    else:
        numpix=iaim.shape()[0]/10;
    if(numpix < 5):
        numpix=6
    #print 'NUMPIX', numpix
    if((fracofpeak >0) and (fracofpeak < 1)):
        rms=stat['max'][0]*fracofpeak
    elif(rmsthresh !=0):
        rms=stat['rms'][0]*rmsthresh
    else:
        rms=stat['rms'][0]*3.0
    
    ib=iaim.rebin('__rebin.image', [numpix, numpix, 1, 1], overwrite=True)
    ib.done()
    ib.open('__rebin.image')
    if(twopass):
        thresh=5.0*rms/np.sqrt(float(numpix))
    else:
        thresh=3.0*rms/np.sqrt(float(numpix))
    ic=ib.imagecalc(outfile='__thresh.image', pixels='iif(abs("__rebin.image")> '+str(thresh)+',1.0,0.0)', overwrite=True)
    ib.remove(done=True, verbose=False)
    ie=ic.regrid(outfile='__threshreg.image', shape=shp, csys=csys.torecord(), axes=[0,1], overwrite=True)
    ic.remove(done=True, verbose=False)
    convpix=str(numpix/2)+'pix' if(twopass) else str(numpix)+'pix'
    ig=ie.convolve2d(outfile='__newmask2.image', major=convpix, minor=convpix, overwrite=True)
    ie.remove(done=True, verbose=False)
    ratiostr='3.0' if(twopass) else '2.0' 
#    print 'pixels=', 'iif(__newmask.image > '+str(stat['rms'][0])+'/'+ratiostr+', 1.0, 0.0)'
    ig.done()
    ih=iamask.imagecalc(outfile='__newmask.image', pixels='iif(__newmask2.image > '+str(rms)+'/'+ratiostr+', 1.0, 0.0)')
#    print ih.statistics()
    ih.done()
    iamask.removefile('__newmask2.image')
    
    if(not os.path.exists(maskimage)):
        iamask.fromimage(outfile=maskimage, infile=image)
        iamask.open(maskimage)
        iamask.set(0.0)
        iamask.done()
    else:
        ih=iamask.imagecalc(outfile='__newmask2.image', pixels='"'+maskimage+'"'+'+ __newmask.image')
        ih.done()
        iamask.removefile('__newmask.image')
        os.rename('__newmask2.image', '__newmask.image')
   
    iamask.open(maskimage)
#    print iamask.statistics()
    iamask.calc(pixels='iif(__newmask.image > 0.0, 1.0 ,0)')
#    print 'post adding', iamask.statistics()
    iamask.done()
    iaim.done()
    iamask.removefile('__newmask.image')
    if(twopass):
        iaim.open(image)
        iaim.calcmask(mask='"'+maskimage+'"'+' < 0.9', name='mulligatawni')
        iaim.done()
        automask(image=image, maskimage=maskimage, fracofpeak=fracofpeak, rmsthresh=rmsthresh, resolution=resolution, twopass=False)
        iaim.open(image)
        iaim.maskhandler('set', '')
        iaim.done()
        rg.deletefromtable(image, 'mulligatawni') 
   

def automask2(image='', maskimage=''):
    pdb.set_trace()
    iaim.open(image)
    stat=iaim.statistics(list=True, verbose=True)
    thresh=stat['rms'][0]*3.0
    fac=stat['max'][0]/thresh
    ret=iaim.decompose(threshold=stat['rms'][0]*4.0, ncontour=3, fit=False)
    if(not os.path.exists(maskimage)):
        iamask.fromimage(outfile=maskimage, infile=image)
        iamask.open(maskimage)
        iamask.set(0.0)
        iamask.done()
    iamask.fromimage(outfile='maskoo', infile=maskimage, overwrite=True)
    iamask.open('maskoo')
    arr=iamask.getchunk()
    iscube=(ret['blc'].shape[1] >2)
    cubeblc=0
    cubetrc=0
    for j in range(ret['blc'].shape[0]):
        if(iscube):
            cubeblc=ret['blc'][j,2]
            cubetrc=ret['trc'][j,2]
            #print 'cubeblc-trc', cubeblc, cubetrc 
            arr[ret['blc'][j,0]:ret['trc'][j,0], ret['blc'][j,1]:ret['trc'][j,1], 0, cubeblc:cubetrc]=ret['components'][j,0]
        else:
             arr[ret['blc'][j,0]:ret['trc'][j,0], ret['blc'][j,1]:ret['trc'][j,1], 0, 0]=ret['components'][j,0]
    iamask.putchunk(arr)
    ib=iamask.convolve2d(outfile='masky', major='10pix', minor='10pix', overwrite=True)
    rej=ib.statistics(list=True, verbose=True)['max'][0]
    rej=rej/fac
    ib.calc(pixels='iif("'+'masky'+'" > '+str(rej)+', 1.0, 0.0)')
    ib.done()
    iamask.open(maskimage)
    iamask.calc(pixels='iif("'+maskimage+'" >= masky,"'+maskimage+'", masky)')
    iamask.done()
    iamask.removefile('maskoo')
    iamask.removefile('masky')
