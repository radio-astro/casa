import os
from taskinit import *

def importfits(fitsimage,imagename,whichrep,whichhdu,zeroblanks,overwrite):
	"""Convert an image FITS file into a CASA image:

	Keyword arguments:
	fitsimage -- Name of input image FITS file
		default: none; example='3C273XC1.fits'
	imagename -- Name of output CASA image
		default: none; example: imagename='3C273XC1.image'
	whichrep -- If fits image has multiple coordinate reps,
	        choose one.
		default: 0 means first; example: whichrep=1
	whichhdu -- If fits file contains multiple images,
	        choose this one
		default=0 mean first; example: whichhdu=1
	zeroblanks -- Set blanked pixels to zero (not NaN)
		default=True; example: zeroblanks=True
	overwrite -- Overwrite pre-existing imagename
		default=False; example: overwrite=True

	"""

	#Python script
	casalog.origin('importfits')
	ia.fromfits(imagename,fitsimage,whichrep,whichhdu,zeroblanks,overwrite)
	ia.close()

