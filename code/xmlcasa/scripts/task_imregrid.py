import os
from taskinit import *

def imregrid(imagename,template,output):
    """ imregrid: Task to grid imagename so that the output image
    has the same shape and coordinates structure as the template
    image.
    
    	Keyword arguments:
    	imagename -- Name of the image that needs to be regridded
    		default: none; example: imagename='orion.image'
    	template -- Name of an image which is used as reference to
    		provide shape and coordinate parameters of output image  
    		default: none; example: imagename='orion_vla.im'
    	output -- Name of image which will store out put regridded image
    		default: none; example: imagename='new_orion.im'
    
    """

    #Python script
    try:
	casalog.origin('imregrid')
        if os.access(imagename, os.F_OK) is False:
            raise TypeError, '***Image %s is inaccessible***'%imagename
        if os.access(template, os.F_OK) is False:
            raise TypeError, '***Template image %s is inaccessible***'%template
        ia.open(template)
        csys=ia.coordsys()
        shap=ia.shape()
        ia.close()
        ia.open(imagename)
        ib=ia.regrid(outfile=output, shape=shap, csys=csys.torecord(),
                  overwrite=True)
        ib.close()
        ia.close()
        tb.clearlocks()
        
    except Exception, instance:
        print '*** Error ***',instance
        try:
            ia.close()
            tb.clearlocks()
        except: pass

