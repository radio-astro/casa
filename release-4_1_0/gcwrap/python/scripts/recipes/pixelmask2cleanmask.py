import shutil
from casa import image as ia
from casa import table as tb

def pixelmask2cleanmask(imagename='',maskname='mask0',maskimage='',usemasked=False):
    """
    convert pixel(T/F) mask (in a CASA image) to a mask image (1/0)
    used for clean
    imagename - input imagename that contain a mask to be used
    maskname - mask name in the image (default: mask0)
    maskimage - output mask image name
    usemasked - if True use masked region as a valid region
    """
    ia.open(imagename)
    masks=ia.maskhandler('get')
    ia.close()

    inmaskname=''
    if type(masks)!=list:
        masks=[masks]
    for msk in masks:
        if maskname == msk:
             inmaskname=msk
             break
    if inmaskname=='':
        raise Exception, "mask %s does not exist. Available masks are: %s" % (maskname,masks)

    tb.open(imagename+'/'+maskname)
    dat0=tb.getcol('PagedArray')
    tb.close()

    #os.system('cp -r %s %s' % (imagename, maskimage))
    shutil.copytree(imagename,maskimage)
    ia.open(maskimage)
    # to unset mask
    ia.maskhandler('set',[''])
    # make all valid
    if (usemasked):
        ia.set(1)
    else:
        ia.set(0) 
    ia.close()
    #
    tb.open(maskimage,nomodify=False)
    imd=tb.getcol('map')
    # maybe shape check here
    #by default use True part of bool mask
    masked=1
    if (usemasked): masked=0
    imd[dat0]=masked
    tb.putcol('map',imd)
    tb.close()

