import os
from taskinit import *
from caltab_convert2 import *

def caltab_convert( caltab_old=None, vis=None, ptype=None, caltab_new=None ):

	"""
    	This task converts old-style caltables into new-style caltables.  It is
    	provided as a convenience and is strictly temporary.  The information
    	transferred should be enough for most calibration purposes.  BPOLY and
    	GSPLINE versions are not supported.  Only simple bugs will be fixed.  If
    	there are other issues, it is suggested that a new-style caltable be
    	created.

    	Arguments:

    	caltab_old -- Name of the old-style caltable
        	default: none
		example: caltab_old='gronk.g0'

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

    	caltab_new -- Name of the new-style caltable.
        	default: "" --> the suffix ".new" is appended to the name of the old-style caltable
        	example: caltab_old='gronk_new.g0'
	"""

	try:
		success = caltab_convert2( caltab_old=caltab_old, ms=vis,
		    pType=ptype, caltab_new=caltab_new )
		return success
	except Exception, instance:
		print '*** Error ***', instance
		raise Exception, instance
