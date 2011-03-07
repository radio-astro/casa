import os
import shutil
from taskinit import *

def imregrid(imagename, template, output):
    casalog.origin('imregrid')
    # First check to see if the output file exists.  If it
    # does then we abort.  CASA doesn't allow files to be
    # over-written, just a policy.
    if len(output) < 1:
        output = imagename + '.regridded'
        casalog.post("output was not specified - defaulting to\n\t"
                     + output, 'INFO')
    if os.path.exists(output):
        raise Exception, 'Output destination ' + output + \
              " exists.\nPlease remove it or change the output file name."
    
    try:
        if not os.path.isdir(imagename) or not os.access(imagename, os.R_OK):
            raise TypeError, 'Cannot read source image ' + imagename

        # Figure out what the user wants.
        if not isinstance(template, dict):
            if template.lower() == 'get':
                ia.open(imagename)
                csys = ia.coordsys().torecord()
                shap = ia.shape()
                ia.done()
                tb.clearlocks()                        # Still needed?
                return {'csys': csys, 'shap': shap}
            elif template.upper() in ('J2000', 'B1950', 'B1950_VLA',
                                      'GALACTIC', 'HADEC', 'AZEL',
                                      'AZELSW', 'AZELNE', 'ECLIPTIC',
                                      'MECLIPTIC', 'TECLIPTIC',
                                      'SUPERGAL'):       
                ia.open(imagename)
                csys = ia.coordsys().torecord()
                shap = ia.shape()
                ia.done()
                tb.clearlocks()                        # Still needed?

                newrefcode = template.upper()
                oldrefcode = csys['direction0']['system']
                if oldrefcode == newrefcode:
                    casalog.post(imagename + ' is already in ' + oldrefcode,
                                 'INFO')
                    casalog.post("...making a straight copy...", 'INFO')
                    shutil.copytree(imagename, output)
                    return True
                    
                casalog.post("Changing coordinate system from " + oldrefcode
                             + " to " + newrefcode, 'INFO')
                csys['direction0']['conversionSystem'] = newrefcode
                csys['direction0']['system'] = newrefcode
                refdir = me.direction(oldrefcode,
                                      {'unit': 'rad',
                                       'value': csys['direction0']['crval'][0]},
                                      {'unit': 'rad',
                                       'value': csys['direction0']['crval'][1]})
                refdir = me.measure(refdir, newrefcode)
                csys['direction0']['crval'][0] = refdir['m0']['value']
                csys['direction0']['crval'][1] = refdir['m1']['value']
            else:                   # Don't use a template named 'get', people.
                if not os.path.isdir(template) or not os.access(template,
                                                                os.R_OK):
                    raise TypeError, 'Cannot read template image ' + template

                ia.open(template)
                csys = ia.coordsys().torecord()
                shap = ia.shape()
                ia.done()
        else:
            csys = template['csys']
            shap = template['shap']

        # The actual regridding.
        ia.open(imagename)
        ib=ia.regrid(outfile=output, shape=shap, csys=csys, overwrite=True)
        ia.done()
        ib.done()
        tb.clearlocks()
        return True
        
    except Exception, instance:
        print '*** Error ***',instance
        try:
            ia.close()
            tb.clearlocks()
        except: pass
        return False
        
