from casa import imager as im
from casa import image as ia
from casa import regionmanager as rg
import os 
import shutil

def drawmaskinimage(image='', maskname='maskoo', makedefault=True):
    """
    Till casa think graphical interaction is important.
    Ugly Function that uses interactive clean image mask  to 
    allow users to draw a mask and store it as a bit mask in the image 
    so it can be used by imageanalysis methods or to mask an image

    Parameters

    image: is the image to put a bit mask in
    maskname: name of bit mask 
    makedefault: if True will make new bit mask drawn be the default mask

    """
    if(not os.path.exists(image)):
        return
    tmpmaskimage='__muluktani'
    if(os.path.exists(tmpmaskimage)):
        shutil.rmtree(tmpmaskimage, True)
    im.drawmask(image=image, mask=tmpmaskimage)
    ia.open(image)
    ia.calcmask(mask='__muluktani < 0.5', name=maskname, asdefault=makedefault)
    ia.done()
    print 'regions/mask in ', image, rg.namesintable(image)
    shutil.rmtree(tmpmaskimage, True)
