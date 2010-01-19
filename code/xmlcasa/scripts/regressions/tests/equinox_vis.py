import math
import os
import shutil
from taskinit import *
from tasks import *

def description():
    # clean, imfit, and im(.advise) are also used.
    return "Tests converting the equinoxes (B1950, J2000, etc.) of uvw data with fixvis."

def data():
    """As I understand it, this must return the filenames of needed input data."""
    return ['0420+417.ms']
    
    
def run():
    """Run the tasks and compare the results."""
    nsigma    = 3.0
    advice    = None
    lazy      = True
    equinoxes = ['B1950_VLA', 'B1950', 'J2000']
    # 'BMEAN', 'BTRUE', 'APP', 'JMEAN', 'JTRUE']
    convert_phasecenter = False
    origvisses = data()
    
    for origvis in origvisses:
        peaks = run_tasks(origvis, advice, lazy, equinoxes, convert_phasecenter)
        compare(peaks, nsigma)

    return []

def conv_dirstr(orig_dirstr, newframe):
    """
    Returns a direction string (suitable for use in clean) made by converting
    orig_dirstr to newframe.
    """
    orig_dirmeas = me.direction(*(orig_dirstr.split()))
    converted_dirmeas = me.measure(orig_dirmeas, newframe)
    lon = qa.formxxx(converted_dirmeas['m0'], format='hms')
    lat = qa.formxxx(converted_dirmeas['m1'], format='dms')
    return "%s %s %s" % (converted_dirmeas['refer'], lon, lat)
    

def run_tasks(origvis='0420+417.ms', advice=None, lazy=True,
              equinoxes=['B1950_VLA', 'B1950', 'J2000', 'BMEAN', 'BTRUE',
                         'APP', 'JMEAN', 'JTRUE'],
              convert_phasecenter=False):
    """
    Fill the peaks dictionary by running fixvis, clean, and imfit for 
    equinoxes, and the original one.
    """
    if not advice:
        im.open(origvis, False, True) # advise looks like a read-only function,
        advice = im.advise()          # but it needs the imaging_weight col.
        im.close()
    origphasecenter = advice['phasecenter']

    peaks = {}
    peaks['original'] = getpeak(origvis, advice)

#    for equinox in ['B1950_VLA', 'B1950', 'J2000', 'BMEAN', 'BTRUE',
#                    'APP', 'JMEAN', 'JTRUE']:
    for equinox in equinoxes:
        # This would insert equinox everywhere there's a .
        # outputvis = origvis.replace('.', '_' + equinox + '.')

        # This inserts equinox exactly once, even if origvis does not have a .
        visparts = origvis.split('.')
        visparts.insert(-1, equinox)
        outputvis = '.'.join(visparts)

        try:
            if os.path.isdir(outputvis):
                if not lazy:
                    shutil.rmtree(outputvis)
                    print "Running fixvis(%s, %s, refcode=%s)" % (origvis,
                                                                  outputvis,
                                                                  equinox)
                    fixvis(origvis, outputvis, refcode=equinox)
            else:
                print "Running fixvis(%s, %s, refcode=%s)" % (origvis,
                                                              outputvis,
                                                              equinox)
                fixvis(origvis, outputvis, refcode=equinox)

            if convert_phasecenter:
                advice['phasecenter'] = conv_dirstr(origphasecenter, equinox)
                
            peaks[equinox] = getpeak(outputvis, advice)

            if convert_phasecenter:
                advice['phasecenter'] = origphasecenter
                
        except Exception, e:
            print "Error", e, "trying to test equinox", equinox
    return peaks

def getpeak(vis, advice):
    """Clean vis and find the peak of the resulting image."""
    print "Getting peak of", vis
    pixsize = str(advice['cell']['value']) + advice['cell']['unit']
    npix = advice['pixels']    

    imroot = vis.replace('.ms', '')
    for ext in ['image', 'psf', 'residual', 'model', 'flux']:
        if os.path.isdir(imroot + '.' + ext):
            shutil.rmtree(imroot + '.' + ext)
    clean(vis=vis, imagename=imroot,
          imsize=[npix, npix],
          #imsize=npix,
          cell=[pixsize, pixsize],
          phasecenter=advice['phasecenter'])
    for ext in ['residual', 'model', 'flux']:
        if os.path.isdir(imroot + '.' + ext):
            shutil.rmtree(imroot + '.' + ext)

    # Compare peak positions (and PSFs?) (from imstat?) to each other and
    # original.
    npixm1 = str(npix - 1)
    return imfit(imroot + '.image',
                 box='0,0,' + npixm1 + ',' + npixm1)['results']['component0']

def direction_var(direction):
    """
    Given a direction dictionary from imfit with estimated latitude and
    longitude errors, returns the position variance collapsed down to a
    single number, in arcsec**2.
    """
    dirvars = [qa.convert(direction['error'][way],
                          'arcsec')['value']**2 for way in ['latitude', 'longitude']]
    return sum(dirvars)
    

def compare(peaks, nsigma=3.0):
    origdir = peaks['original']['shape']['direction']
    origdirvar = direction_var(origdir)

    # I in Jy
    origflux = qa.convert(peaks['original']['flux'], 'Jy')['value'][0]
    origfluxvar = qa.convert({'unit':  peaks['original']['flux']['unit'],
                              'value': peaks['original']['flux']['error'][0]},
                             'Jy')['value']**2

    equinoxes = peaks.keys()
    equinoxes.remove('original')
    equinoxes.sort()
    for equinox in equinoxes:
        try:
            dist_from_original = me.separation(origdir,
                                               peaks[equinox]['shape']['direction'])
            dist_from_original = qa.convert(dist_from_original, 'arcsec')
            peaks[equinox]['shape']['direction']['dist_from_original'] = dist_from_original
            dirvar = direction_var(peaks[equinox]['shape']['direction'])
            septol = nsigma * math.sqrt(dirvar + origdirvar)
            if dist_from_original['value'] > septol:
                errmsg = "The %s peak is %.1g\" away from the original peak.\n" % (equinox,
                                                           dist_from_original['value'])
                errmsg += " (%.1f x the tolerance)" % (dist_from_original['value']
                                                       / septol) 
                raise Exception, errmsg

            # I in Jy
            newflux = qa.convert(peaks[equinox]['flux'], 'Jy')['value'][0]
            newfluxvar = qa.convert({'unit':  peaks[equinox]['flux']['unit'],
                                     'value': peaks[equinox]['flux']['error'][0]},
                                    'Jy')['value']
            fluxtol = nsigma * math.sqrt(newfluxvar + origfluxvar)
            if abs(newflux - origflux) > fluxtol:
                print "The flux density of %s, %g Jy, is %.1f x the tolerance away from" % (equinox, newflux, abs(newflux - origflux) / fluxtol)
                print "the original flux density, %g Jy." % (origflux)

            # Different uv distributions might show up best in the beam shape.
            for q in ('majoraxis', 'minoraxis', 'positionangle'):
                cmp_shape_param(peaks, equinox, q, nsigma)
        except Exception, e:
            print "Error", e, "comparing to test equinox", equinox

        
def cmp_shape_param(peaks, equinox, q, nsigma):
    """
    Compare shape parameter q ('majoraxis', 'minoraxis', or 'positionangle') in
    peaks[equinox] to the originals.
    """
    qerr = q + 'error'
    unit = peaks['original']['shape'][qerr]['unit']

    origq = qa.convert(peaks['original']['shape'][q], unit)['value']
    newq  = qa.convert(peaks[equinox]['shape'][q], unit)['value']

    origvar = peaks['original']['shape'][qerr]['value']**2
    newvar = qa.convert(peaks[equinox]['shape'][qerr], unit)['value']**2

    tol = nsigma * math.sqrt(origvar + newvar)
    err = abs(newq - origq)
    if err > tol:
        print "The %s of %s, %g %s, is %.1f x the tolerance away from" % (q,
                                                                          equinox,
                                                                          newq,
                                                                          unit,
                                                                          err / tol)
        print "the original %s, %g %s." % (q, origq, unit)
