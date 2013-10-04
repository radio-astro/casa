import os
from taskinit import *

def importfits(fitsimage,imagename,whichrep,whichhdu,zeroblanks,overwrite,defaultaxes,defaultaxesvalues):
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
	defaultaxes -- Add the default 4D coordinate axes where they are missing
	        default=False, example: defaultaxes=True
	defaultaxesvalues -- List of values to assign to added degenerate axes when defaultaxes==True (ra,dec,freq,stokes)
	        default = [], example: defaultaxesvalues=['13.5h', '-2.5deg', '88.5GHz', 'Q'] 

	"""

	#Python script
	casalog.origin('importfits')
	_myia = iatool()
	tmpname = imagename
	reorder = False
	addaxes = False
	adddir = False
	addstokes = False
	addfreq=False
	defaultorder = ['Right Ascension', 'Declination', 'Stokes', 'Frequency']

	try:

		if os.path.exists(imagename):
			if not overwrite:
				raise RuntimeError, 'Output image exists already and you did not set overwrite to True.'
			else:
				os.system('rm -rf '+imagename)

		if defaultaxes:
			if len(defaultaxesvalues)!=4:
				raise TypeError, 'When defaultaxes==True, parameter defaultaxesvalues must be provided as a list of 4 values: RA, Dec, Freq, Stokes,\n e.g. [\'13.5h\', \'-2.5deg\', \'88.5GHz\', \'I\']'
			_myia.open(fitsimage)
			_mycs=_myia.coordsys()
			acts = _mycs.axiscoordinatetypes()
			cnames = _mycs.names()
			_myia.close()
			if ('Direction' in acts and not ('Right Ascension' in cnames and 'Declination' in cnames)):
				raise RuntimeError, 'Non-standard direction axes. Cannot add default axes.'
			if ('Spectral' in acts and not 'Frequency' in cnames):
				raise RuntimeError, 'Non-standard spectral axis. Cannot add default axes.'
			if ('Stokes' in acts and not 'Stokes' in cnames):
				raise RuntimeError, 'Non-standard Stokes axis. Cannot add default axes.'

			if not ('Right Ascension' in cnames and 'Declination' in cnames):
				addaxes = True
				adddir = True
			if not ('Frequency' in cnames):
				addaxes = True
				addfreq = True
			if not ('Stokes' in cnames):
				addaxes = True
				addstokes = True
			if not addaxes and cnames!=defaultorder:
				reorder = True
			if addaxes or reorder:
				tmpname = imagename+'.tmp'
				os.system('rm -rf '+tmpname)

		_myia.fromfits(tmpname,fitsimage,whichrep,whichhdu,zeroblanks)
		_myia.close()

		if addaxes:
			casalog.post('Adding missing coodinate axes ...', 'INFO')
			tmpname2 = imagename+'.tmp2'
			os.system('rm -rf '+tmpname2)
			
			_myia.open(tmpname)
			ia2  = _myia.adddegaxes(outfile=tmpname2, direction=True, spectral=True, stokes=defaultaxesvalues[3],
						silent=True)
			_myia.close()
			os.system('rm -rf '+tmpname)
			ia2.close()
			ia2.open(tmpname2)

			# set the right reference values in the added axes
			_mynewcs=ia2.coordsys()
			raval = 0.
			decval = 0.
			freqval = 0.
			if adddir:
				ra = defaultaxesvalues[0]
				if type(ra)==int or type(ra)==float:
					raval = ra
				else:
					qara = qa.quantity(qa.angle(ra)[0])
					if qara['unit'].find('deg') < 0:
						raise TypeError, "RA default value is not a valid angle quantity " %ra
					raval = qara['value']

				dec = defaultaxesvalues[1]
				if type(dec)==int or type(dec)==float:
					decval = dec
				else:
					qadec = qa.quantity(qa.angle(dec)[0])
					if qadec['unit'].find('deg') < 0:
						raise TypeError, "DEC default value is not a valid angle quantity " %dec
					decval = qadec['value']
					
				_mynewcs.setreferencevalue(type='direction', value=[raval,decval])
				
			if addfreq:
				freq = defaultaxesvalues[2]
				if type(freq)==int or type(freq)==float:
					freqval = freq
				else:
					qafreq = qa.quantity(freq)
					if qafreq['unit'].find('Hz') < 0:
						raise TypeError, "Freq default value is not a valid frequency quantity " %freq
					freqval = qa.convertfreq(qafreq,'Hz')['value']
				_mynewcs.setreferencevalue(type='spectral', value=freqval)
				_mynewcs.setrestfrequency(freqval)

			# Note: stokes default value was already set in adddegaxes

			if adddir or addfreq:
				ia2.setcoordsys(_mynewcs.torecord())

			ia2.close()

			cnames = _mynewcs.names()
			
			if len(cnames)==4 and not (cnames == defaultorder):
				# need to reorder
				reorder = True
				tmpname = tmpname2
			else:
				os.system('mv '+tmpname2+' '+imagename)
				
		if reorder:
			casalog.post('Transposing coodinate axes ...', 'INFO')
			_myia.open(tmpname)
			ia2 = _myia.transpose(outfile=imagename, order=defaultorder)
			_myia.close()
			ia2.close()
			os.system('rm -rf '+tmpname)

	except Exception, instance:
		casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
		raise
