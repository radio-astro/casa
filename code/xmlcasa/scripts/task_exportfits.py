import os
from taskinit import *

def exportfits(imagename,fitsimage,velocity,optical,bitpix,minpix,maxpix,overwrite,dropdeg,deglast):
	"""Convert a CASA image to a FITS file:
	CASA-produced images can be written to disk for transporting
	to other software packages.  No subimaging of the fits image
	can be made with this task.

	Keyword arguments:
	imagename -- Name of input CASA image
		default: none; example: imagename='3C273XC1.image'
	fitsimage -- Name of input image FITS file
		default: none; example='3C273XC1.fits'
	velocity -- Prefer velocity (rather than frequency) as spectral axis
		default: False
	optical -- Use the optical (rather than radio) velocity convention
		default: True;
	bitpix -- Bits per pixel
		default: -32 (floating point)
		<Options: -32 (floating point), 16 (integer)>
	minpix -- Minimum pixel value
		default: 0 = autoscale
	maxpix -- Maximum pixel value
		default: 0 = autoscale
	overwrite -- Overwrite pre-existing imagename
		default=False; example: overwrite=True
	dropdeg -- Drop degenerate axes?
		default: False; example: dropdeg=True
	deglast -- Put degenerate axes last in header?
		default: False; example: deglast=True

	"""

	#Python script
	casalog.origin('exportfits')

	ia.open(imagename)
	ia.tofits(outfile=fitsimage,velocity=velocity,optical=optical,bitpix=bitpix,minpix=minpix,maxpix=maxpix,overwrite=overwrite,dropdeg=dropdeg,deglast=deglast)
	ia.close()
