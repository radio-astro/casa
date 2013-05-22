import os
from taskinit import *

def exportfits(
	imagename, fitsimage, velocity, optical, bitpix,
	minpix, maxpix, overwrite, dropstokes, stokeslast,
	history, dropdeg
):
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
		default: 0 (if minpix > maxpix, value is automatically determined)
	maxpix -- Maximum pixel value
		default: -1 (if minpix > maxpix, value is automatically determined)
	overwrite -- Overwrite pre-existing imagename
		default=False; example: overwrite=True
	dropstokes -- Drop the Stokes axis?
		default: False; example: dropstokes=True
	stokeslast -- Put the stokes axis last in header?
		default: True; example: stokeslast=False
	history -- write the image history into the FITS file?
	        default: True
	dropdeg -- drop all degenerate axes (e.g. Stokes and/or Frequency)?
	        default: False

	"""

	#Python script
	casalog.origin('exportfits')
	_myia = iatool()
	_myia.open(imagename)
	_myia.tofits(
		outfile=fitsimage, velocity=velocity, optical=optical,
		bitpix=bitpix, minpix=minpix, maxpix=maxpix,
		overwrite=overwrite, dropstokes=dropstokes, stokeslast=stokeslast,
		history=history, dropdeg=dropdeg
	)
	_myia.close()
