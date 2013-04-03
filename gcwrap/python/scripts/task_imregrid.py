import os
import shutil
from taskinit import *

def imregrid(imagename, template, output, asvelocity, axes, shape):
    _myia = None
    _tmp = None
    try:
        casalog.origin('imregrid')
        if hasattr(template, 'lower') and not template.lower() == "get":
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
        _myia = iatool()
        # Figure out what the user wants.
        if not isinstance(template, dict):
            if template.lower() == 'get':
                _myia.open(imagename)
                csys = _myia.coordsys().torecord()
                shape = _myia.shape()
                _myia.done()
                return {'csys': csys, 'shap': shape}
            elif template.upper() in ('J2000', 'B1950', 'B1950_VLA',
                                      'GALACTIC', 'HADEC', 'AZEL',
                                      'AZELSW', 'AZELNE', 'ECLIPTIC',
                                      'MECLIPTIC', 'TECLIPTIC',
                                      'SUPERGAL'):       
                _myia.open(imagename)
                csys = _myia.coordsys().torecord()
                shape = _myia.shape()
                _myia.done()

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

                _myia.open(template)
                csys = _myia.coordsys().torecord()
                if (len(shape) == 1 and shape[0] == -1):
                    _myia.open(imagename)
                    shape = _myia.shape()
                _myia.done()
        else:
            csys = template['csys']
            shape = template['shap']

        # The actual regridding.
        _myia.open(imagename)

        _tmp = _myia.regrid(outfile=output, shape=shape, csys=csys, axes=axes, overwrite=True, asvelocity=asvelocity)
        _myia.done()
        _tmp.done()
        return True
        
    except Exception, instance:
        casalog.post(str(instance), "SEVERE")
        raise instance
    finally:
        if (_myia):
            _myia.done()
        if (_tmp):
            _tmp.done()
        
        
