import os
import shutil
from taskinit import *

def imregrid(
    imagename, template, output, asvelocity, axes, shape,
    interpolation, decimate, replicate, overwrite
):
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
        _myia = iatool()
        # Figure out what the user wants.
        if not isinstance(template, dict):
            if template.lower() == 'get':
                _myia.open(imagename)
                csys = _myia.coordsys()
                shape = _myia.shape()
                _myia.done()
                return {'csys': csys.torecord(), 'shap': shape}
            elif template.upper() in (
                'J2000', 'B1950', 'B1950_VLA', 'GALACTIC',
                'HADEC', 'AZEL', 'AZELSW', 'AZELNE', 'ECLIPTIC',
                'MECLIPTIC', 'TECLIPTIC', 'SUPERGAL'
            ):
                _imregrid_to_new_ref_frame(
                    _myia, imagename, template, output,
                    axes, shape, overwrite
                )
                return True
            else:                   # Don't use a template named 'get', people.
                if not os.path.isdir(template) or not os.access(template,
                                                                os.R_OK):
                    raise TypeError, 'Cannot read template image ' + template
                template_ia = iatool()
                template_ia.open(template)
                template_csys = template_ia.coordsys()
                image_ia = iatool()
                image_ia.open(imagename)
                image_csys = image_ia.coordsys()
                
                axestoregrid = axes
                if (axes[0] < 0):
                    axestoregrid = []
                    image_ncoords = image_csys.ncoordinates()
                    for i in range(image_ncoords):
                        ctype = image_csys.coordinatetype(i)[0]
                        if ctype != 'Stokes' and template_csys.findcoordinate(ctype)[0]:
                            world_axes = image_csys.findcoordinate(ctype)[1]
                            axestoregrid.extend(world_axes)
                    if len(axestoregrid) == 0:
                        raise Exception("Found no axes to regrid!")
                    axestoregrid.sort()                                        
                # axesnames = _myia.coordsys().names()
                if (len(shape) == 1 and shape[0] == -1):
                    # CAS-4959, output shape should have template shape
                    # for axes being regridded, input image shape for axes
                    # not being regridded
                    tempshape = template_ia.shape()
                    imshape = image_ia.shape()
                    shape = imshape
                    targetaxesnames = image_csys.names()
                    for i in range(len(imshape)):
                        for j in axestoregrid:
                            if i == j:
                                # axis numbers may not correspond so have to look for the template axis
                                # location by the axis name, CAS-4960
                                shape[i] = tempshape[template_csys.findaxisbyname(targetaxesnames[i])] 
                                break
                template_ia.done()
                image_ia.done()
                csys = template_csys
        else:
            csys = cstool()
            csys.fromrecord(template['csys'])
            shape = template['shap']

        # The actual regridding.
        _myia.open(imagename)
        _tmp = _myia.regrid(
            outfile=output, shape=shape, csys=csys.torecord(),
            axes=axes, asvelocity=asvelocity,
            method=interpolation, decimate=decimate,
            replicate=replicate, overwrite=overwrite
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
            
def _imregrid_to_new_ref_frame(
    _myia, imagename, template, output,
    axes, shape, overwrite
):
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
        casalog.post(
            imagename + ' is already in ' + oldrefcode,
            'INFO'
        )
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
                
    angle = csys.convertdirection(newrefcode)
    mysin = qa.getvalue(qa.sin(angle))
    mycos = qa.getvalue(qa.cos(angle))
    xnew = 0
    ynew = 0
    for xx in [-centerpix[0], centerpix[0]]:
        for yy in [-centerpix[1], centerpix[1]]:
            xnew = max(xnew, abs(xx*mycos - yy*mysin + 1))
            ynew = max(ynew, abs(xx*mysin + yy*mycos + 1))
    pad = int(max(xnew - shape[0]/2, ynew - shape[1]/2))
    if pad > 0:
        casalog.post(
            "Padding image by " + str(pad)
            + " pixels so no pixels are cut off in the rotation",
            "NORMAL"
        )
        _myia = _myia.pad("", pad, wantreturn=True) 
        shape = _myia.shape()
        newrefpix = csys.referencepixel()['numeric']
        newrefpix[diraxes[0]] = newrefpix[diraxes[0]] + pad
        newrefpix[diraxes[1]] = newrefpix[diraxes[1]] + pad
        csys.setreferencepixel(newrefpix)            
    casalog.post(
        "Will rotate direction coordinate by "
        + qa.tos(qa.convert(angle,"deg"))
      , 'NORMAL'
    )
    rot = _myia.rotate(outfile="", shape=shape, pa=angle)
    rot.setcoordsys(csys.torecord())
    # now crop
                
    casalog.post("Cropping masked image boundaries", "NORMAL")
    cropped = rot.crop(outfile=output, axes=diraxes, overwrite=overwrite)
    cropped.done()   
    rot.done()
    _myia.done()
    return True
                    
        
        
