from casa import image as ia
from casa import table as tb

def pixelmask2cleanmask(imagename,maskname='mask0',maskimage=''):
    """
    convert pixel(T/F) mask (in a CASA image) to a mask image (1/0)
    used for clean
    """
    ia.open(imagename)
    masks=ia.maskhandler('get')
    ia.close()

    inmaskname=''
    for msk in masks:
        if maskname == msk:
             inmaskname=msk
             break
    if inmaskname=='':
        raise Exception, "mask %s does not exist. Available masks are: %s" % (maskname,masks)

    tb.open(imagename+'/'+maskname)
    dat0=tb.getcol('PagedArray')
    tb.close()

    os.system('cp -r %s %s' % (imagename, maskimage))
    ia.open(maskimage)
    # to unset mask
    ia.maskhandler('set',[''])
    # make all valid
    ia.set(1)
    ia.close()
    #
    tb.open(maskimage,nomodify=False)
    imd=tb.getcol('map')
    # maybe shape check here
    #set False part of bool mask to 0
    imd[dat0]=0
    tb.putcol('map',imd)
    tb.close()
