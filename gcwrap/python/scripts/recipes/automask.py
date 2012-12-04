from casac import casac
import os
iaim=casac.image()
iamask=casac.image()
qa=casac.quanta()

def automask(image='', maskimage=''):
    iaim.open(image)
    stat=iaim.statistics()
    csys=iaim.coordsys()
    rb=iaim.restoringbeam()
    numpix=10
    shp=iaim.shape()
    if(shp[0] < 10):
        raise TypeError, 'you cannot be serious... less than 10 pixels image'
    if(len(rb) > 0):
        numpix=2*(int(2.0*abs(qa.div(rb['major'], qa.convert(csys.increment('q')['quantity']['*1'],rb['major']['unit']))['value']))/2)
    else:
        numpix=iaim.shape()[0]/10;
        if(numpix < 5):
            numpix=6
    ib=iaim.rebin('__rebin.image', [numpix, numpix, 1, 1], overwrite=True)
    ic=ib.imagecalc(outfile='__thresh,image', pixels='iif(abs(__rebin.image)> 2*'+str(stat['rms'][0])+',1.0,0.0)', overwrite=True)
    ib.remove(done=True, verbose=False)
    ie=ic.regrid(outfile='__threshreg.image', shape=shp, csys=csys.torecord(), axes=[0,1], overwrite=True)
    ic.remove(done=True, verbose=False)
    ig=ie.convolve2d(outfile='__newmask.image', major='10pix', minor='10pix', overwrite=True)
    ie.remove(done=True, verbose=False)
    ig.calc(pixels='iif(__newmask.image > '+str(stat['rms'][0])+'/3.0, 1.0, 0.0)')
    if(not os.path.exists(maskimage)):
        iamask.fromimage(outfile=maskimage, infile=image)
        iamask.open(maskimage)
        iamask.set(0.0)
        iamask.done()
    ig.done()
    iamask.open(maskimage)
    iamask.calc(pixels='iif("'+maskimage+'" >= __newmask.image,"'+maskimage+'", __newmask.image)')
    iamask.done()
    iamask.removefile('__newmask.image')

def automask2(image='', maskimage=''):
    iaim.open(image)
    stat=iaim.statistics()
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
            print 'cubeblc-trc', cubeblc, cubetrc 
        arr[ret['blc'][j,0]:ret['trc'][j,0], ret['blc'][j,1]:ret['trc'][j,1], 0, cubeblc:cubetrc]=ret['components'][j,0]
    iamask.putchunk(arr)
    ib=iamask.convolve2d(outfile='masky', major='10pix', minor='10pix', overwrite=True)
    rej=ib.statistics()['max'][0]
    rej=rej/fac
    ib.calc(pixels='iif("'+'masky'+'" > '+str(rej)+', 1.0, 0.0)')
    ib.done()
    iamask.open(maskimage)
    iamask.calc(pixels='iif("'+maskimage+'" >= masky,"'+maskimage+'", masky)')
    iamask.done()
    iamask.removefile('maskoo')
    iamask.removefile('masky')
