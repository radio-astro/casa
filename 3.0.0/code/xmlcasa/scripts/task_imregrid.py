import os
from taskinit import *

def imregrid(imagename,template,output):
    #Python script

    casalog.origin('imregrid')
    # First check to see if the output file exists.  If it
    # does then we abort.  CASA doesn't allow files to be
    # over-written, just a policy.
    if ( len( output ) < 1 ):
        output = 'imregrid_results.im'
        casalog.post( "The output paramter is empty, consequently the" \
                      +" regridded image will be\nsaved on disk in file, " \
                      + outfile, 'WARN' )
    if ( len( output ) > 0 and os.path.exists( output ) ):
        raise Exception, 'Output file, '+output+\
              ' exists. imregrid can not proceed, please\n'+\
              'remove it or change the output file name.'

    try:
        
        if os.access(imagename, os.F_OK) is False:
            raise TypeError, '***Image %s is inaccessible***'%imagename
        if os.access(template, os.F_OK) is False:
            raise TypeError, '***Template image %s is inaccessible***'%template
        
        ia.open(template)
        csys=ia.coordsys()
        shap=ia.shape()
        ia.done()
        ia.open(imagename)
        ib=ia.regrid(outfile=output, shape=shap, csys=csys.torecord(),
                  overwrite=True)
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
        
