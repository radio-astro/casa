import os
from taskinit import *
from caltab_convert2 import *

def caltabconvert( caltabold=None, vis=None, ptype=None, caltabnew=None ):

	"""
    	This task converts old-style caltables into new-style caltables.  It is
    	provided as a convenience and is strictly temporary.  The information
    	transferred should be enough for most calibration purposes.  BPOLY and
    	GSPLINE versions are not supported.  Only simple bugs will be fixed.  If
    	there are other issues, it is suggested that a new-style caltable be
    	created.

    	Arguments:

    	caltabold -- Name of the old-style caltable
        	default: none
		example: caltabold='gronk.g0'

    	vis -- Name of the visibility file (MS) associated with the old-style caltable
       		default: none
		example: vis='blurp.ms'

   	ptype -- Type of data in the new-format caltable
        	default: "complex"; allowed values: "complex" or "float"
        	example: ptype="complex"

        	NB: The old-style caltables do not have this information, so it is
		imperative that users get it correct.  "complex" refers to caltables
        	that have complex gains (e.g., produced by gaincal, bpcal, etc.).
        	"float" referes to caltables that real numbers such as delays (e.g.,
        	produced by gencal).

    	caltabnew -- Name of the new-style caltable.
        	default: "" --> the suffix ".new" is appended to the name of the old-style caltable
        	example: caltabold='gronk_new.g0'
	"""

	try:
		success = caltab_convert2( caltabold=caltabold, ms=vis,
		    pType=ptype, caltabnew=caltabnew )
		return success
	except Exception, instance:
		print '*** Error ***', instance
		raise Exception, instance
