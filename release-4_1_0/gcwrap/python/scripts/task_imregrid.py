import os
import shutil
from taskinit import *

def imregrid(imagename, template, output, asvelocity, axes, shape):
    _myia = None
    _tmp = None
    csys = None
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
                csys = _myia.coordsys()
                shape = _myia.shape()
                _myia.done()
                return {'csys': csys.torecord(), 'shap': shape}
            elif template.upper() in ('J2000', 'B1950', 'B1950_VLA',
                                      'GALACTIC', 'HADEC', 'AZEL',
                                      'AZELSW', 'AZELNE', 'ECLIPTIC',
                                      'MECLIPTIC', 'TECLIPTIC',
                                      'SUPERGAL'):       
                _myia.open(imagename)
                csys = _myia.coordsys()
                if len(shape) > 0 and shape != [-1]:
                    casalog.post(
                        "Specified shape parameter will be ignored when regridding to a new reference frame",
                        "WARN"
                    )
                if len(axes) > 0 and axes != [-1]:
                    casalog.post(
                        "Specified axes parameter will be ignored when regridding to a new reference frame",
                        "WARN"
                    )
                dirinfo = csys.findcoordinate("direction")
                if not dirinfo[0]:
                    raise (Exception, "Image does not have a direction coordinate.")
                newrefcode = template.upper()
                oldrefcode = csys.referencecode("direction")[0]
                if oldrefcode == newrefcode:
                    casalog.post(imagename + ' is already in ' + oldrefcode,
                                 'INFO')
                    casalog.post("...making a straight copy...", 'INFO')
                    shutil.copytree(imagename, output)
                    return True
                casalog.post(
                    "Changing coordinate system from " + oldrefcode
                     + " to " + newrefcode, 'INFO'
                )
                diraxes = dirinfo[1]
                if len(diraxes) != 2:
                    raise Exception("Unsupported number of direction axes. There must be exactly 2.")
                dirrefpix = csys.referencepixel("direction")["numeric"]
                shape = _myia.shape()
                centerpix = [int(shape[diraxes[0]]/2), int(shape[diraxes[1]]/2)]
                if centerpix[0] != dirrefpix[0] or centerpix[1] != dirrefpix[1]:
                    casalog.post(
                        "Center direction pixel and reference pixel are "
                        + "different, making a temporary image and setting "
                        + "the reference pixel equal to the center pixel. "
                        + "The output image will have this modified coordinate system."
                    )
                    newrefpix = csys.referencepixel()["numeric"]
                    newrefpix[diraxes[0]] = centerpix[0]
                    newrefpix[diraxes[1]] = centerpix[1]
                    newrefval = csys.toworld(newrefpix)["numeric"]
                    csys.setreferencepixel(newrefpix)
                    csys.setreferencevalue(newrefval)
                    tsub = _myia.subimage()
                    _myia.done()
                    _myia = tsub
                    _myia.setcoordsys(csys.torecord())
                
                # for some reason, we need to set the old refcode explicity so the
                # conversion doesn't barf
                csys.convertdirection(oldrefcode)
                angle = csys.convertdirection(newrefcode)
                rot = _myia.rotate(
                    outfile=output, shape=shape, pa=qa.neg(angle)
                )
                rot.setcoordsys(csys.torecord())
                rot.done()
                _myia.done()
                return True
                    
            else:                   # Don't use a template named 'get', people.
                if not os.path.isdir(template) or not os.access(template,
                                                                os.R_OK):
                    raise TypeError, 'Cannot read template image ' + template

                
                _myia.open(imagename)
                axestoregrid = axes
                if (axes[0] < 0):
                    axestoregrid = range(len(_myia.shape()))
                axesnames = _myia.coordsys().names()
                _myia.open(template)
                templatenames = _myia.coordsys().names()
                namestoregrid = []
                for axis in axestoregrid:
                    if (
                        axesnames[axis].upper() != "STOKES"
                        and axesnames[axis] != templatenames[axis]
                    ):
                        raise Exception(
                            "ERROR: Axis number " + str(axis) + " mismatch between "
                            + "input (" + axesnames[axis] + ") and template ("
                            + templatenames[axis] + "). The imregrid task currently "
                            + "requires that the ordering of the axes for the input "
                            + "and template images be the same. Please check this "
                            + "using the imhead task with mode='list', and correct as "
                            + "necessary using the imtrans task."
                    )
                csys = _myia.coordsys()
                if (len(shape) == 1 and shape[0] == -1):
                    _myia.open(imagename)
                    shape = _myia.shape()
                _myia.done()
        else:
            # because there is no cstool()
            csys = coordsystool()
            csys.fromrecord(template['csys'])
            shape = template['shap']

        # The actual regridding.
        _myia.open(imagename)
        _tmp = _myia.regrid(
            outfile=output, shape=shape, csys=csys.torecord(),
            axes=axes, overwrite=True, asvelocity=asvelocity
        )
        _myia.done()
        _tmp.done()
        return True
        
    except Exception, instance:
        casalog.post(str(instance), "SEVERE")
        raise instance
    finally:
        if _myia:
            _myia.done()
        if _tmp:
            _tmp.done()
        if csys:
            csys.done()
        
        
