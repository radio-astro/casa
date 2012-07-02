#import os
from taskinit import *
from imregion import *

from odict import odict

def specfit(imagename=None,mode=None,fitorder=None,ngauss=None,box=None,stokes=None,mask=None,estimate=None,estfile=None, sigmafile=None,fitfile=None,residualfile=None):
	"""Performs profile and polygonial fitting of images.
        """

	#Python script
	#
	try:
	    casalog.origin('specfit')
            # Create the region from the information provided by
            # the user.
	    reg=imregion( imagename, '', stokes, box, '', '' )
            print "Done getting region"

            if ( mode=='single' ):
                print "Fitting a single profile"
                ia.open(imagename)

                if ( estimate ):
                    fit = ia.fitprofile( region=reg, mask=mask, ngauss=ngauss, poly=fitorder, fit=False, sigma=sigmafile )

                    print "return keys are: ", fit.keys()
                    print "return is: ", fit['return']
                    residual=fit['resid']   
                    results=fit['values']  # Saved to estimate file
                    
                else:
                    fit=ia.fitprofile( region=reg, mask=mask, estimate=estfile, ngauss=ngauss, poly=fitorder, fit=True, sigma=sigmafile )

                    print "return keys are: ", fit.keys()
                    print "return is: ", fit['return']
                    residual=fit['resid']   
                    results=fit['values']  # Saved to fit file
                
            elif( mode=='all' ):
                print "Fitting all profiles"
                ia.open(imagename)
                fit=ia.fitallprofiles( region=reg, mask=mask, ngauss=ngauss, poly=fitorder, sigma=sigmafile, fit=fitfile, resid=residualfile )
               
            elif( mode=='poly' ):              
                print "Doing a polynomial fit"
                ia.open(imagename)
                ia.fitpolynomial( region=reg, mask=mask, order=fitorder, sigma=sigmafile, fit=fitfile, resid=residualfile )
                
                
            else:
                print '*** Error *** Invalid mode: ', mode, 
 
        except Exception, instance:
            print '*** Error ***',instance
	    return 
              
        return 
