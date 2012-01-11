import casac
from locatescript import copydata

imager = casac.homefinder.find_home_by_name('imagerHome')
image  = casac.homefinder.find_home_by_name('imageHome')

quantity=casac.Quantity
im = imager.create()
ia = image.create()

def data():
    return ['ic2233_1.ms']

def run(fetch=False) :

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    im.open('ic2233_1.ms')
    npix=1024
    im.selectvis(spw='0', nchan=[6], start=[0], step=[1])
    im.defineimage(nx=npix, ny=npix, cellx='3.0arcsec', celly='3.0arcsec', stokes="IV", spw=[0])
    im.weight(type='briggs', robust=0.7)
    im.setvp(dovp=bool(1), usedefaultvp=bool(1), dosquint=bool(1),
             parangleinc='5.0deg')
    im.make('squint_corr')
    im.clean(algorithm='mfhogbom', niter=1000, model=['squint_corr'], residual=['squint_corr.residual'], image=['squint_corr.restored'], threshold='0Jy')
    im.done()
    return ['squint_corr.restored']
