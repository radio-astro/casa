import os
from taskinit import *

import asap as sd
from asap._asap import Scantable
import pylab as pl


def sdbaseline(sdfile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, tau, blmode, blpoly, verify, masklist, thresh, avg_limit, edge, batch, outfile, outform, overwrite, plotlevel):
	
	
	casalog.origin('sdbaseline')

	try:
		#load the data with or without averaging
		if sdfile=='':
			raise Exception, 'sdfile is undefined'
		
		filename = os.path.expandvars(sdfile)
		filename = os.path.expanduser(filename)
		if not os.path.exists(filename):
			s = "File '%s' not found." % (filename)
			raise Exception, s
		
		# Default file name
		if ( outfile == '' ):
			project = sdfile.rstrip('/') + '_bs'
		else:
			project = outfile
		outfilename = os.path.expandvars(project)
		outfilename = os.path.expanduser(outfilename)
		if not overwrite and (outform!='ascii' and outform!='ASCII'):
			if os.path.exists(outfilename):
				s = "Output file '%s' exist." % (outfilename)
				raise Exception, s
		
		s=sd.scantable(sdfile,average=False,antenna=antenna)
		
		if ( abs(plotlevel) > 1 ):
			casalog.post( "Initial Raw Scantable:" )
			casalog.post( s._summary() )
			casalog.post( "--------------------------------------------------------------------------------" )
		
		# check if the data contains spectra
		if (s.nchan()==1):
			s = "Cannot process the input data. It contains only single channel data."
			raise Exception, s
		
		# get telescope name
		#'ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43' (Tid), 'CEDUNA', and 'HOBART'
		antennaname = s.get_antennaname()
		
		# check current fluxunit
		# for GBT if not set, set assumed fluxunit, Kelvin
		fluxunit_now = s.get_fluxunit()
		if ( antennaname == 'GBT'):
			if (fluxunit_now == ''):
				casalog.post( "No fluxunit in the data. Set to Kelvin." )
				s.set_fluxunit('K')
				fluxunit_now = s.get_fluxunit()
		
		casalog.post( "Current fluxunit = "+fluxunit_now )
		
		# convert flux
		# set flux unit string (be more permissive than ASAP)
		if ( fluxunit == 'k' ):
			fluxunit = 'K'
		elif ( fluxunit == 'JY' or fluxunit == 'jy' ):
			fluxunit = 'Jy'
		
		# fix the fluxunit if necessary
		if ( telescopeparm == 'FIX' or telescopeparm == 'fix' ):
			if ( fluxunit != '' ):
				if ( fluxunit == fluxunit_now ):
					casalog.post( "No need to change default fluxunits" )
				else:
					s.set_fluxunit(fluxunit)
					casalog.post( "Reset default fluxunit to "+fluxunit )
					fluxunit_now = s.get_fluxunit()
			else:
				casalog.post( "no fluxunit for set_fluxunit", priority = 'WARN' )
		
		elif ( fluxunit=='' or fluxunit==fluxunit_now ):
			if ( fluxunit==fluxunit_now ):
				casalog.post( "No need to convert fluxunits" )
		
		elif ( type(telescopeparm) == list ):
			# User input telescope params
			if ( len(telescopeparm) > 1 ):
				D = telescopeparm[0]
				eta = telescopeparm[1]
				casalog.post( "Use phys.diam D = %5.1f m" % (D) )
				casalog.post( "Use ap.eff. eta = %5.3f " % (eta) )
				s.convert_flux(eta=eta,d=D)
			elif ( len(telescopeparm) > 0 ):
				jypk = telescopeparm[0]
				casalog.post( "Use gain = %6.4f Jy/K " % (jypk) )
				s.convert_flux(jyperk=jypk)
			else:
				casalog.post( "Empty telescope list" )
		
		elif ( telescopeparm=='' ):
			if ( antennaname == 'GBT'):
				# needs eventually to be in ASAP source code
				casalog.post( "Convert fluxunit to "+fluxunit )
				# THIS IS THE CHEESY PART
				# Calculate ap.eff eta at rest freq
				# Use Ruze law
				#   eta=eta_0*exp(-(4pi*eps/lambda)**2)
				# with
				casalog.post( "Using GBT parameters" )
				eps = 0.390  # mm
				eta_0 = 0.71 # at infinite wavelength
				# Ideally would use a freq in center of
				# band, but rest freq is what I have
				rf = s.get_restfreqs()[0][0]*1.0e-9 # GHz
				eta = eta_0*pl.exp(-0.001757*(eps*rf)**2)
				casalog.post( "Calculated ap.eff. eta = %5.3f " % (eta) )
				casalog.post( "At rest frequency %5.3f GHz" % (rf) )
				D = 104.9 # 100m x 110m
				casalog.post( "Assume phys.diam D = %5.1f m" % (D) )
				s.convert_flux(eta=eta,d=D)
				
				casalog.post( "Successfully converted fluxunit to "+fluxunit )
			elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
				s.convert_flux()
			
			else:
				# Unknown telescope type
				casalog.post( "Unknown telescope - cannot convert", priority = 'WARN' )
		
		# set default spectral axis unit
		if ( specunit != '' ):
			s.set_unit(specunit)
		
		# reset frame and doppler if needed
		if ( frame != '' ):
			s.set_freqframe(frame)
		else:
			casalog.post( 'Using current frequency frame' )
		
		if ( doppler != '' ):
			if ( doppler == 'radio' ):
				ddoppler = 'RADIO'
			elif ( doppler == 'optical' ):
				ddoppler = 'OPTICAL'
			elif ( doppler == 'z' ):
				ddoppler = 'Z'
			else:
				ddoppler = doppler
			
			s.set_doppler(ddoppler)
		else:
			casalog.post( 'Using current doppler convention' )
		
		# Select scan and field
		sel = sd.selector()
		
		# Set up scanlist
		if ( type(scanlist) == list ):
			# is a list
			scans = scanlist
		else:
			# is a single int, make into list
			scans = [ scanlist ]
		# Now select them
		if ( len(scans) > 0 ):
			sel.set_scans(scans)
		
		# Select source names
		if ( field != '' ):
			sel.set_name(field)
			# NOTE: currently can only select one
			# set of names this way, will probably
			# need to do a set_query eventually
		
		# Select IFs
		if ( type(iflist) == list ):
			# is a list
			ifs = iflist
		else:
			# is a single int, make into list
			ifs = [ iflist ]
		if ( len(ifs) > 0 ):
			# Do any IF selection
			sel.set_ifs(ifs)
		
		# Select polarizations
		if (type(pollist) == list):
			pols = pollist
		else:
			pols = [pollist]
		if(len(pols) > 0 ):
			sel.set_polarisations(pols)
		
		try:
			#Apply the selection
			s.set_selection(sel)
		except Exception, instance:
			casalog.post( str(instance), priority = 'ERROR' )
			return
		del sel
		
		scanns = s.getscannos()
		sn=list(scanns)
		casalog.post( "Number of scans to be processed: %d" % (len(sn)) )
		
		# Warning for multi-IF data
		if ( len(s.getifnos()) > 1 and not blmode == 'auto' ):
			casalog.post( 'The scantable contains multiple IF data.', priority = 'WARN' )
			casalog.post( 'Note the same mask(s) are applied to all IFs based on CHANNELS.', priority = 'WARN' )
			casalog.post( 'Baseline ranges may be incorrect for all but IF=%d.' % (s.getif(0)), priority = 'WARN' )
		
		# do opacity (atmospheric optical depth) correction
		if ( tau > 0.0 ):
			# recalculate az/el (NOT needed for GBT data)
			if ( antennaname != 'GBT'): s.recalc_azel()
			s.opacity(tau)
		
		# Header data for saving parameters of baseline fit
		header = "Source Table: "+sdfile+"\n"
		header += " Output File: "+project+"\n"
		header += "   Flux Unit: "+s.get_fluxunit()+"\n"
		header += "     Abscissa: "+s.get_unit()+"\n"
		header += "   Fit order: %d\n"%(blpoly)
		header += "    Fit mode: "+blmode+"\n"
		if blmode == 'auto':
			header += "   Threshold: %f\n"%(thresh)
			header += "   avg_limit: %d\n"%(avg_limit)
			header += "        Edge: "+str(edge)+"\n"
		elif blmode == 'list':
			header += "   Fit Range: "+str(masklist)+"\n"
		
		blfile = project + "_blparam.txt"
		blf = open(blfile, "w")
		separator = "#"*60 + "\n"
		blf.write(separator)
		
		nrow = s.nrow()
		
		# Polynomial baseline (if requested, e.g. blpoly=5)
		if ( blmode == 'auto' ):
			blf.write(header)
			blf.write(separator)
			
			if( blpoly >= 0 ):
				if (len(masklist) > 0):
					# Use baseline mask for regions to INCLUDE in baseline fit
					# Create mask using list, e.g. masklist=[[500,3500],[5000,7500]]
					basemask=s.create_mask(masklist)
					for r in xrange(nrow):
						s.auto_poly_baseline(mask=basemask,order=blpoly,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,rows=r)
						msk = s.actualmask[0]
						rmsofrow = s._math._statsrow(s, msk, 'rms', r)[0]
						masklistofrow = s.masklists[0]
						dataout = _format_output_row(s, s.blpars, rmsofrow, masklistofrow, r)
						blf.write(dataout)
						del msk,rmsofrow,masklistofrow,dataout

					del basemask
				else:
					for r in xrange(nrow):
						s.auto_poly_baseline(order=blpoly,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,rows=r)
						msk = s.actualmask[0]
						rmsofrow = s._math._statsrow(s, msk, 'rms', r)[0]
						masklistofrow = s.masklists[0]
						dataout = _format_output_row(s, s.blpars, rmsofrow, masklistofrow, r)
						blf.write(dataout)
						del msk,rmsofrow,masklistofrow,dataout


				blf.write("-"*60 + "\n")
			
		elif ( blmode == 'list' ):
			blf.write(header)
			blf.write(separator)
			
			if( blpoly >= 0 ):
				if (len(masklist) > 0):
					# Use baseline mask for regions to INCLUDE in baseline fit
					# Create mask using list, e.g. masklist=[[500,3500],[5000,7500]]
					basemask=s.create_mask(masklist)
					for r in xrange(nrow):
						s.poly_baseline(mask=basemask,order=blpoly,plot=verify,batch=batch,rows=r)
						msk = s.actualmask[0]
						rmsofrow = s._math._statsrow(s, msk, 'rms', r)[0]
						masklistofrow = s.masklists[0]
						dataout = _format_output_row(s, s.blpars, rmsofrow, masklistofrow, r)
						blf.write(dataout)
						del msk,rmsofrow,masklistofrow,dataout
					
					del basemask
				else:
					for r in xrange(nrow):
						s.poly_baseline(order=blpoly,plot=verify,batch=batch,rows=r)
						msk = s.actualmask[0]
						rmsofrow = s._math._statsrow(s, msk, 'rms', r)[0]
						masklistofrow = s.masklists[0]
						dataout = _format_output_row(s, s.blpars, rmsofrow, masklistofrow, r)
						blf.write(dataout)
						del msk,rmsofrow,masklistofrow,dataout
						
				blf.write("-"*60 + "\n")
			
		elif ( blmode == 'interact'):
			# Interactive masking
			new_mask=sd.interactivemask(scan=s)
			if (len(masklist) > 0):
				new_mask.set_basemask(masklist=masklist,invert=False)
			new_mask.select_mask(once=False,showmask=True)
			# Wait for user to finish mask selection
			finish=raw_input("Press return to calculate statistics.\n")
			new_mask.finish_selection()
			
			# Get final mask list
			msk=new_mask.get_mask()
			del new_mask
			msks=s.get_masklist(msk)
			if len(msks) < 1:
				casalog.post( 'No channel is selected. Exit without baselining.', priority = 'WARN' )
				return
			casalog.post( 'final mask list ('+s._getabcissalabel()+') ='+str(msks) )
			header += "   Fit Range: "+str(msks)+"\n"
			
			blf.write(header)
			blf.write(separator)
			
			# Calculate base-line RMS
			if len(msks) > 0:
				s.poly_baseline(mask=msk,order=blpoly,plot=verify,batch=False)
				rmsl=list(s.stats('rms',msk))
				# NOTICE: Do not modify scantable before formatting output
				dataout=_format_output(s,s.blpars,rmsl)
			else:
				s.poly_baseline(order=blpoly,plot=verify,batch=False)
				rmsl=list(s.stats('rms'))
				# NOTICE: Do not modify scantable before formatting output
				dataout=_format_output(s,s.blpars,rmsl)
			del msk, msks

			blf.write(dataout)

		
		# Plot final spectrum
		if ( abs(plotlevel) > 0 ):
			# each IF is separate panel, pols stacked
			sd.plotter.plot(s)
			sd.plotter.set_mode(stacking='p',panelling='i')
			sd.plotter.set_histogram(hist=True)
			sd.plotter.axhline(color='r',linewidth=2)
			if ( plotlevel < 0 ):
				# Hardcopy - currently no way w/o screen display first
				pltfile=project+'_bsspec.eps'
				sd.plotter.save(pltfile)
		
		# Now save the spectrum and write out final ms
		if ( (outform == 'ASCII') or (outform == 'ascii') ):
			outform = 'ASCII'
			spefile = project + '_'
		elif ( (outform == 'ASAP') or (outform == 'asap') ):
			outform = 'ASAP'
			spefile = project
		elif ( (outform == 'SDFITS') or (outform == 'sdfits') ):
			outform = 'SDFITS'
			spefile = project
		elif ( (outform == 'MS') or (outform == 'ms') or (outform == 'MS2') or (outform == 'ms2') ):
			outform = 'MS2'
			spefile = project
		else:
			outform = 'ASAP'
			spefile = project
		
		if overwrite and os.path.exists(outfilename):
			os.system('rm -rf %s' % outfilename)
		
		# to apply data selections by selector, make copy
		tmpscn=s.copy()
		tmpscn.save(spefile,outform,overwrite)
		if outform!='ASCII':
			casalog.post( "Wrote output "+outform+" file "+spefile )
		
		# Clean up scantable
		del s, tmpscn
		
		blf.close()

	
	except Exception, instance:
		casalog.post( str(instance), priority = 'ERROR' )
		return


### Format baseline parameters for output
def _format_output(scan=None,pars=None,rms=None,masklists=None):
	# Check input scantable
	if not isinstance(scan, Scantable):
		casalog.post( "Data is not scantable", priority = 'ERROR' )
		return

	nrow = scan.nrow()
	
	# Check baseline fit parameters
	if not isinstance(pars,list):
		casalog.post( "Invalid baseline parameters.", priority = 'ERROR' )
		return
	elif nrow != len(pars):
		casalog.post( "Number of rows != baseline params sets.", priority = 'ERROR' )
		return
	# Check rms data
	if isinstance(rms,list) and len(rms) == nrow: sflag=True
	else:
		casalog.post( "Invalid rms data", priority = 'ERROR' )
		return
	# Check masklists data
	mflag=False
	if masklists is None: mflag=False
	elif isinstance(masklists,list) and len(masklists) == nrow:
		mflag=True
	else:
		casalog.post( "Invalid masklists", priority = 'ERROR' )
		return
	
	# Format data output
	out = ""
	sep = "-"*60+"\n"
	out += sep

	for r in xrange(nrow):
		out+=" Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]: \n" % \
			  (scan.getscan(r), scan.getbeam(r), scan.getif(r), \
			   scan.getpol(r), scan.getcycle(r))
		if mflag:
			out += "Fitted range = "+str(masklists[r])+"\n"
		out+="Baseline parameters\n"
		if pars[r] is not None: 
			cpars = pars[r]['params']
			cfixed = pars[r]['fixed']
			c = 0
			for i in xrange(len(cpars)):
				fix = ""
				if len(cfixed) and cfixed[i]: fix = "(fixed)"
				out += '  p%d%s= %3.6f,' % (c,fix,cpars[i])
				c+=1
			out = out[:-1]  # remove trailing ','
			out+="\n"
		else: out += '  Not fitted\n'
		out += "Results of baseline fit\n"
		out += "  rms = %3.6f\n" % (rms[r][0])
		out += sep
	return out



### Format baseline parameters of a scan for output
def _format_output_row(scan=None, pars=None, rms=None, masklist=None, row=0):
	out = "-"*60 + "\n"

	out += " Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]: \n" % \
	       (scan.getscan(row), scan.getbeam(row), scan.getif(row), \
		scan.getpol(row), scan.getcycle(row))

	if masklist is None:
		mflag = False
	elif isinstance(masklist, list):
		mflag = True
	else:
		casalog.post("Invalid masklist", priority = "ERROR")
		return
	
	if mflag:
		out += "Fitter range = " + str(masklist) + "\n"

	out += "Baseline parameters\n"

	if pars is not None:
		cpars = pars[0]["params"]
		cfixed = pars[0]["fixed"]
		c = 0
		for i in xrange(len(cpars)):
			fix = ""
			if len(cfixed) and cfixed[i]:
				fix = "(fixed)"
			out += "  p%d%s= %3.6f," % (c, fix, cpars[i])
			c += 1
		out = out[:-1]
		out += "\n"
	else:
		out += "  Not fitted\n"

	out += "Results of baseline fit\n"
	out += "  rms = %3.6f\n" % (rms)

	return out
