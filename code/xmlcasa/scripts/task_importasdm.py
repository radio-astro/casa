import os
from taskinit import *

def importasdm(asdm=None, vis=None, singledish=None, antenna=None, corr_mode=None, srt=None, time_sampling=None, ocorr_mode=None, compression=None, asis=None, wvr_corrected_data=None, scans=None, ignore_time=None, process_syspower=None, process_caldevice=None, verbose=None, overwrite=None, showversion=None, useversion=None):
	"""Convert an ALMA Science Data Model observation into a CASA visibility file (MS) or single-dish data format (Scantable).
           The conversion of the ALMA SDM archive format into a measurement set.  This version
           is under development and is geared to handling many spectral windows of different
           shapes.
				          
           Keyword arguments:
           asdm -- Name of input ASDM file (directory)
               default: none; example: asdm='ExecBlock3'

	   vis       -- Root ms or scantable name, note a prefix (.ms or .asap) is NOT appended to this name
               default: none

	   singledish   -- Set True to write data as single-dish format (Scantable)
               default: False

            &gt;&gt;&gt; singledish expandable parameter
                 antenna -- antenna name or id.
 
	   corr_mode -- correlation mode to be considered on input. Could
	         be one or more of the following, ao, co, ac, or all
	       default: all

	   srt       -- spectral resolution type. Could be one or more of
	                the following, fr, ca, bw, or all
	       default: all

	   time_sampling -- specifies the time sampling, INTEGRATION and/or
                            SUBINTEGRAION. could be one or more of the following
                            i, si, or all.
		 default: all

	   ocorr_mode    -- output data for correlation mode AUTO_ONLY 
                            (ao) or CROSS_ONLY (co) or CROSS_AND_AUTO (ca)
		 default: ca
		 
	   compression  -- produces comrpressed columns in the resulting measurement set.
                 default: False

	   asis         --  creates verbatim copies of the ASDM tables in 
	                    the output measurement set. The value given to
		            this option must be a list of table names separated
		            by space characters; the wildcard character '*' is 
                            allowed in table names.

	   wvr_corrected_data -- specifies wich values are considered in the 
                      ASDM binary data to fill the DATA column in 
                      the MAIN table of the MS. Expected values for 
                      this option are 'no' for the uncorrected data 
                      (this is the default), 'yes' for the corrected
                      data and 'both' for corrected and uncorrected 
                      data. In the latter case, two measurement sets
                      are created, one containing the uncorrected 
                      data and the other one, whose name is suffixed
                      by '-wvr-corrected', containing the corrected 
                      data.

	   scans --	  processes only the scans specified in the option's value. This value is a semicolon 
	              separated list of scan specifications. A scan specification consists in an exec bock index 
                      followed by the character ':' followed by a comma separated list of scan indexes or scan 
                      index ranges. A scan index is relative to the exec block it belongs to. Scan indexes are 
                      1-based while exec blocks's are 0-based. "0:1" or "2:2~6" or "0:1,1:2~6,8;2:,3:24~30" "1,2" 
                      are valid values for the option. "3:" alone will be interpreted as 'all the scans of the 
                      exec block#3'. An scan index or a scan index range not preceded by an exec block index will
                      be interpreted as 'all the scans with such indexes in all the exec blocks'.  By default 
                      all the scans are considered.

	   ignore_time -- All the rows of the tables Feed, History, Pointing, Source, SysCal, CalDevice, SysPower,
                      and Weather are processed independently of the time range of the selected exec block / scan.

	   process_syspower -- The SysPower table is processed if and only if this parameter is set to True.

	   process_caldevice -- The CalDevice table is processed if and only if this parameter is set to True.

	   verbose     -- produce log output as asdm2MS is being run

	   showversion -- report the version of the asdm2MS being used.

	   useversion -- Selects the version of asdm2MS to be used (\'v2\' (default) or \'v3\' (use for ALMA data))
                     default: v2
        """
	#Python script

	# make fg tool local 
	fg = casac.homefinder.find_home_by_name('flaggerHome').create()	

	try:
                casalog.origin('importasdm')
		viso = ''
		visoc = '' # for the wvr corrected version, if needed
                if singledish:
                        theexecutable = 'asdm2ASAP'
                        if useversion == 'v2':
                                theexecutable = 'oldasdm2ASAP'
                        if compression:
                                casalog.post('compression=True has no effect for single-dish format.')
                        cmd = 'which %s > /dev/null 2>&1'%(theexecutable)
                        ret = os.system( cmd )
                        if ret == 0:
                                import commands
                                casalog.post( 'found %s'%(theexecutable) )
                                casapath_split = os.environ['CASAPATH'].split()
                                casaroot = casapath_split[0]
                                if ( len(casapath_split) > 1 ):
                                        casaarch = casapath_split[1]
                                else:
                                        casaarch = 'none'
                                if os.path.exists( casaroot+'/'+casaarch ):
                                        cmd = 'find %s -name asap'%(casaroot+'/'+casaarch)
                                else:
                                        cmd = 'find %s -name asap'%(casaroot)
                                lpath = commands.getoutput( cmd )
                                if lpath.find('\n') != -1:
                                        paths = lpath.split('\n')
                                        for p in paths:
                                                if p.find('python') != -1:
                                                        lpath = p
                                                        break
                                casalog.post( 'path to asap library is %s'%(lpath) )
                                if lpath not in os.environ['LD_LIBRARY_PATH'].split(':'):
                                        casalog.post( 'appending %s to LD_LIBRARY_PATH'%(lpath) )
                                        os.environ['LD_LIBRARY_PATH'] += ':%s'%(lpath)
                                if showversion:
                                        execute_string = theexecutable + ' --help'
                                else:
                                        execute_string = theexecutable + ' -asdm ' + asdm 
                                        if ( len(vis) != 0 ):
                                                execute_string += ' -asap ' + vis.rstrip('/')
                                        execute_string += ' -antenna ' + str(antenna) + ' -apc ' + wvr_corrected_data + ' -time-sampling ' + time_sampling.lower() + ' -overwrite ' + str(overwrite) 
                                        if ( corr_mode == 'all' ):
                                                execute_string += ' -corr-mode ao,ca -ocorr-mode ao'
                                        else:
                                                execute_string += ' -corr-mode ' + corr_mode.replace(' ',',') + ' -ocorr-mode ao'
                                        if ( srt == 'all' ):
                                                execute_string += ' -srt ' + srt
                                        else:
                                                execute_string += ' -srt ' + srt.replace(' ',',')
                                        execute_string += ' -logfile ' + casalog.logfile()
                                casalog.post( 'execute_string is' )
                                casalog.post( '   '+execute_string )
                                ret = os.system( execute_string )
                                if ret != 0 and not showversion:
                                        casalog.post(theexecutable+' terminated with exit code '+str(ret),'SEVERE')
                                        raise Exception, "ASDM conversion error, please check if it is a valid ASDM and/or useversion='%s' is consistent with input ASDM."%(useversion)
                        else:
                                casalog.post( 'You have to build ASAP to be able to create single-dish data.','SEVERE' )
                        return 
		if(len(vis) > 0) :
		   viso = vis
		   tmps = vis.rstrip('.ms')
		   if(tmps==vis):
			   visoc = vis+"-wvr-corrected"
		   else:
			   visoc = tmps+"-wvr-corrected.ms"			   
                   if singledish:
                      viso = vis.rstrip('/') + '.importasdm.tmp.ms'
		else :
		   viso = asdm + '.ms'
		   visoc = asdm + '-wvr-corrected.ms'
		   vis = asdm
                   if singledish:
                      viso = asdm.rstrip('/') + '.importasdm.tmp.ms'
                      vis = asdm.rstrip('/') + '.asap'

		theexecutable = 'asdm2MS'
		if(useversion == 'v3'):
			theexecutable = 'asdm2MS_v3'

		execute_string = theexecutable+' --icm \"' +corr_mode + '\" --isrt \"' + srt+ '\" --its \"' \
				 + time_sampling+ '\" --ocm \"' + ocorr_mode + '\" --wvr-corrected-data \"' \
				 + wvr_corrected_data + '\" --asis \"' + asis + '\" --logfile \"' +casalog.logfile() +'\"'
			
		if(len(scans) > 0) :
		   execute_string= execute_string +' --scans ' + scans
		if (ignore_time) :
			execute_string= execute_string +' --ignore-time'
		if (process_syspower) :
			execute_string = execute_string +' --process-syspower'
		if (process_caldevice) :
			execute_string = execute_string +' --process-caldevice'
		if(compression) :
		   execute_string= execute_string +' --compression'
		if(verbose) :
		   execute_string= execute_string +' --verbose'
		if not overwrite and os.path.exists(viso):
		   raise Exception, "You have specified and existing ms and have indicated you do not wish to overwrite it"

		#
		# If viso+".flagversions" then process differently depending on the value of overwrite..
		#
		dotFlagversion = viso + ".flagversions"
		if os.path.exists(dotFlagversion):
			if overwrite :
				casalog.post("Found '"+dotFlagversion+"' . It'll be deleted before running the filler.")
				os.system('rm -rf %s'%dotFlagversion)
			else :
				casalog.post("Found '%s' but can't overwrite it."%dotFlagversion)
				raise Exception, "Found '%s' but can't overwrite it."%dotFlagversion
	   
		execute_string = execute_string + ' ' + asdm + ' ' + viso

		if(showversion) :
			casalog.post("You set option \'showversion\' to True. Will just output the version information and then terminate.", 'WARN')
			execute_string= theexecutable +' --revision'

		casalog.post('Running '+theexecutable+' standalone invoked as:')
		#print execute_string
		casalog.post(execute_string)
        	exitcode=os.system(execute_string)
                if exitcode!=0:
			if(not showversion):
				casalog.post(theexecutable+' terminated with exit code '+str(exitcode),'SEVERE')
				raise Exception, "ASDM conversion error, please check if it is a valid ASDM and that data/alma/asdm is up-to-date"
		if compression :
                   #viso = viso + '.compressed'
                   viso = viso.rstrip('.ms') + '.compressed.ms'
                   visoc = visoc.rstrip('.ms') + '.compressed.ms'

		if(wvr_corrected_data=='no' or wvr_corrected_data=='both'):
			if os.path.exists(viso):
			   fg.open(viso)
			   fg.saveflagversion('Original',comment='Original flags at import into CASA',merge='save')
			   fg.done();
		elif(wvr_corrected_data=='yes' or wvr_corrected_data=='both'):
			if os.path.exists(visoc):
			   fg.open(visoc)
			   fg.saveflagversion('Original',comment='Original flags at import into CASA',merge='save')
			   fg.done();			

	except Exception, instance:
		print '*** Error ***',instance

