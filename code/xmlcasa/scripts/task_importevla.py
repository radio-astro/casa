import os
from taskinit import *

debug = False
def importevla(asdm=None, vis=None, ocorr_mode=None, compression=None, asis=None, scans=None, switchedpower=None, verbose=None, overwrite=None, online=None, tbuff=None, flagzero=None, cliplevel=None, flagpol=None, shadow=None, diameter=None, applyflags=None, flagbackup=None):
	""" Convert a Science Data Model (SDM) dataset into a CASA Measurement Set (MS)
	This version is under development and is geared to handling EVLA specific flag and
	system files, and is otherwise equivalent to importasdm.
	
	Keyword arguments:
	asdm -- Name of input SDM file (directory)
		default: none; example: asdm='TOSR0001_sb1308595_1.55294.83601028935'

	"""
	#Python script
	#Origninator: Steven T. Myers
	#Written (3.0.1) STM 2010-03-11 modify importasdm to include flagging from xml
	#Vers1.0 (3.0.1) STM 2010-03-16 add tbuff argument
	#Vers2.0 (3.0.1) STM 2010-03-29 minor improvements
	#Vers3.0 (3.0.2) STM 2010-04-13 add flagzero, doshadow
	#Vers4.0 (3.0.2) STM 2010-04-20 add flagpol
	#Vers5.0 (3.0.2) STM 2010-05-27 combine flagzero clips
	#Vers6.0 (3.1.0) STM 2010-07-01 flagbackup option
	#Vers7.0 (3.1.0) STM 2010-08-18 remove corr_mode,wvr_corrected_data,singledish,antenna
	#Vers7.1 (3.1.0) STM 2010-10-07 remove time_sampling, srt
	#Vers7.1 (3.1.0) STM 2010-10-07 use helper functions, flagger tool, fill FLAG_CMD 
	#Vers7.2 (3.1.0) STM 2010-10-29 minor modifications to defaults and messages
	#Vers8.0 (3.2.0) STM 2010-11-23 tbuff not sub-par of applyflags=T
	#Vers8.1 (3.2.0) STM 2010-12-01 prec=9 on timestamps
	#Vers8.2 (3.2.0) MKH 2010-12-06 added scan selection
	#Vers8.3 (3.2.0) GAM 2011-01-18 added switchedpower option (sw power gain/tsys)
	#Vers8.4 (3.2.0) STM 2011-03-24 fix casalog.post line-length bug
	#Vers8.5 (3.4.0) STM 2011-12-08 new readflagxml for new Flag.xml format
	#Vers8.6 (3.4.0) STM 2011-02-22 full handling of new Flag.xml ant+spw+pol flags
	#Vers8.6 (3.4.0) STM 2011-03-01 bug fix for compression
	#
	if applyflags:
		fglocal = casac.homefinder.find_home_by_name('flaggerHome').create()
	#
	try:
                casalog.origin('importevla')
		casalog.post('You are using importevla v8.6 STM Updated 2012-03-01')
		viso = ''
                casalog.post('corr_mode is forcibly set to all.')
		if(len(vis) > 0) :
		   viso = vis
		else :
		   viso = asdm + '.ms'
		   vis = asdm
		corr_mode = 'all'
		wvr_corrected_data = 'no'
		singledish=False
		srt='all'
		time_sampling='all'
		showversion=True
		execute_string='asdm2MS  --icm \"' +corr_mode + '\" --isrt \"' + srt+ '\" --its \"' + time_sampling+ '\" --ocm \"' + ocorr_mode + '\" --wvr-corrected-data \"' + wvr_corrected_data + '\" --asis \"' + asis + '\" --scans \"' + scans + '\" --logfile \"' +casalog.logfile() +'\"'
		if(showversion) :
		   casalog.post('asdm2MS --revision --logfile \"' +casalog.logfile() +'\"')
		   os.system('asdm2MS --revision --logfile \"' +casalog.logfile() +'\"')
		if(compression) :
		   execute_string= execute_string +' --compression'
		if (switchedpower) :
		   execute_string = execute_string +' --process-syspower --process-caldevice'
		if(verbose) :
		   execute_string= execute_string +' --verbose'
		if not overwrite and os.path.exists(viso) :
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
		casalog.post('Running the asdm2MS standalone invoked as:')
		#print execute_string
		casalog.post(execute_string)
        	os.system(execute_string)
		if compression :
                    #viso = viso + '.compressed'
                    viso = viso.rstrip('.ms') + '.compressed.ms'
		if flagbackup:
		    ok=fg.open(viso);
		    ok=fg.saveflagversion('Original',comment='Original flags on import',merge='save')
		    ok=fg.done();
		    print "Backed up original flag column to "+viso+".flagversions"
		    casalog.post("Backed up original flag column to "+viso+".flagversions")
	        else:
		    casalog.post("Warning: will not back up original flag column",'WARN')
		#
		# =============================
		# Begin EVLA specific code here
		# =============================
		nflags = 0
		myflagd = {}
		if os.access(asdm+'/Flag.xml',os.F_OK):
	            # Find (and copy) Flag.xml
		    print "  Found Flag.xml in SDM, copying to MS"
		    casalog.post("Found Flag.xml in SDM, copying to MS")
		    os.system('cp -rf '+asdm+'/Flag.xml '+viso+'/')
		    # Find (and copy) Antenna.xml
		    if os.access(asdm+'/Antenna.xml',os.F_OK):
		       print "  Found Antenna.xml in SDM, copying to MS"
		       casalog.post("Found Antenna.xml in SDM, copying to MS")
		       os.system('cp -rf '+asdm+'/Antenna.xml '+viso+'/')
	            else:
		       raise Exception, "Failed to find Antenna.xml in SDM"
		    # Find (and copy) SpectralWindow.xml
		    if os.access(asdm+'/SpectralWindow.xml',os.F_OK):
		       print "  Found SpectralWindow.xml in SDM, copying to MS"
		       casalog.post("Found SpectralWindow.xml in SDM, copying to MS")
		       os.system('cp -rf '+asdm+'/SpectralWindow.xml '+viso+'/')
	            else:
		       raise Exception, "Failed to find SpectralWindow.xml in SDM"
		    #
		    # Parse Flag.xml into flag dictionary
		    #
                    if online:
                        myflagd = readflagxml(asdm,tbuff)
                        #
                        keylist = myflagd.keys()
                        nkeys = keylist.__len__()
                        nflags += nkeys
		else:
                    if online:
                        #print 'ERROR: No Flag.xml in SDM'
                        casalog.post('ERROR: No Flag.xml in SDM','SEVERE')
                    else:
                        #print 'WARNING: No Flag.xml in SDM'
                        casalog.post('WARNING: No Flag.xml in SDM','WARN')

		if flagzero or shadow:
		    # Get overall MS time range for later use (if needed)
		    (ms_startmjds,ms_endmjds,ms_starttime,ms_endtime) = getmsmjds(viso)
		    
		# Now add zero and shadow flags
		if flagzero:
		    flagz = {}
		    # clip low-amp data
		    # NOTE: currently hard-wired to RL basis
		    # assemble into flagging commands and add to myflagd
		    flagz['time'] = 0.5*(ms_startmjds+ms_endmjds)
		    flagz['interval'] = ms_endmjds-ms_startmjds
		    flagz['level'] = 0
		    flagz['severity'] = 0
		    flagz['type'] = 'FLAG'
		    flagz['applied'] = False
		    flagz['antenna'] = ''
		    flagz['mode'] = 'clip'
		    # Add to myflagz
		    flagz['reason'] = 'CLIP_ZERO_RR'
		    flagz['cmd'] = "mode='clip' cliprange='0~"+str(cliplevel)+"' clipexpr='ABS_RR'"
		    flagz['id'] = 'ZERO_RR'
		    myflagd[nflags] = flagz.copy()
		    nflags += 1
		    # 
		    flagz['reason'] = 'CLIP_ZERO_LL'
		    flagz['cmd']= "mode='clip' cliprange='0~"+str(cliplevel)+"' clipexpr='ABS_LL'"
		    flagz['id'] = 'ZERO_LL'
		    myflagd[nflags] = flagz.copy()
		    nflags += 1
		    nflagz = 2
		    if (flagpol):
		        flagz['reason'] = 'CLIP_ZERO_RL'
			flagz['cmd']= "mode='clip' cliprange='0~"+str(cliplevel)+"' clipexpr='ABS_RL'"
			flagz['id'] = 'ZERO_RL'
			myflagd[nflags] = flagz.copy()
			nflags += 1
		        flagz['reason'] = 'CLIP_ZERO_LR'
			flagz['cmd']= "mode='clip' cliprange='0~"+str(cliplevel)+"' clipexpr='ABS_LR'"
			flagz['id'] = 'ZERO_RL'
			myflagd[nflags] = flagz.copy()
			nflags += 1
			nflagz += 2
			       
		    print 'Flagging low-amplitude points, using clip level ',cliplevel
		    casalog.post('Flagging low-amplitude points, using clip level '+str(cliplevel))
		    print 'Added '+str(nflagz)+' clipping flags to list'
		    casalog.post('Added '+str(nflagz)+' clipping flags to list')
		    
		if shadow:
		    flagh = {}
		    # flag shadowed data
		    if diameter==-1.0:
		        print 'Flagging shadowed data, using antenna diameter '
			casalog.post('Flagging shadowed data, using antenna diameter ')
		    else:
		        print 'Flagging shadowed data, using diameter ',diameter
			casalog.post('Flagging shadowed data, using diameter '+str(diameter))
		    flagh['time'] = 0.5*(ms_startmjds+ms_endmjds)
		    flagh['interval'] = ms_endmjds-ms_startmjds
		    flagh['level'] = 0
		    flagh['severity'] = 0
		    flagh['type'] = 'FLAG'
		    flagh['applied'] = False
		    flagh['antenna'] = ''
		    flagh['mode'] = 'shadow'
		    # Add to myflagz
		    flagh['reason'] = 'SHADOW'
		    flagh['cmd'] = "mode='shadow' diameter="+str(diameter)
		    flagh['id'] = 'SHADOW'
		    myflagd[nflags] = flagh.copy()
		    nflags += 1
		    print 'Added 1 shadow flag to list'
		    casalog.post('Added 1 shadow flag to list')

		#
		# Write commands to FLAG_CMD in MS
		#
		writeflagcmd(viso,myflagd)

		if applyflags:
		    if nflags>0:
		        #
			# First sort the flags by antenna to compress
			# Needed to avoid error "too many agents"
			myantenna = ''
			myreason = 'Any'
			myflagsort = 'antpol'
			myflagsd = sortflags(myflagd,myantenna,myreason,myflagsort)
		        #
			# Now flag the data
			#
			reset = False
			nappl = applyflagcmd(fglocal, viso, False, myflagsd, reset)
		        #
			# Set APPLIED=T for those flags
			# NOTE: do this in the unsorted list to writeflagcmd
			if nappl>0:
			    myrowlist = myflagd.keys()
			    napprows = myrowlist.__len__()
			    print 'Updating APPLIED status in FLAG_CMD for '+str(napprows)+' rows'
			    casalog.post('Updating APPLIED status in FLAG_CMD for '+str(napprows)+' rows')
			    if napprows>0:
			        updateflagcmd(viso,mycol='APPLIED',myval=True,myrowlist=myrowlist)
		    else:
		        print 'No flagging found in Flag.xml'
			casalog.post('No flagging found in Flag.xml')
		else:
		    #print 'Warning: will not be applying flags (applyflags=False), use flagcmd to apply'
		    casalog.post('Will not be applying flags (applyflags=False), use flagcmd to apply','WARN')

	except Exception, instance:
		print '*** Error ***',instance

        #write history
        ms.open(viso,nomodify=False)
        ms.writehistory(message='taskname   = importevla', origin='importevla')
        ms.writehistory(message='asdm       = "' + str(asdm) + '"', origin='importevla')
        ms.writehistory(message='vis        = "' + str(viso) + '"', origin='importevla')
	if flagzero:
	    ms.writehistory(message='flagzero   = T"', origin='importevla')
	    ms.writehistory(message='cliplevel  = "' + str(cliplevel) + '"', origin='importevla')
	    if flagpol:
	        ms.writehistory(message='flagpol    = T"', origin='importevla')
	if shadow:
	    ms.writehistory(message='shadow     = T"', origin='importevla')
	    ms.writehistory(message='diameter   = "' + str(diameter) + '"', origin='importevla')
	if applyflags:
	    ms.writehistory(message='applyflags = T"', origin='importevla')
		
        ms.close()

#===============================================================================
def getmsmjds(vis):
    # Get start and end times from MS, return in mjds
    # this might take too long for large MS
    # NOTE: could also use values from OBSERVATION table col TIME_RANGE
    mslocal2 = casac.homefinder.find_home_by_name('msHome').create()
    success = True
    ms_time1 = ''
    ms_time2 = ''
    ms_startmjds=0.0
    ms_endmjds=0.0
    try:
	    mslocal2.open(vis)
	    timd = mslocal2.range(["time"])
	    mslocal2.close()
    except:
	    success=False
	    print "Error opening MS "+vis
    if success:
	    ms_startmjds = timd['time'][0]
	    ms_endmjds = timd['time'][1]
	    t = qa.quantity(ms_startmjds,'s')
	    t1sd = t['value']
	    ms_time1 = qa.time(t,form="ymd",prec=9)
	    t = qa.quantity(ms_endmjds,'s')
	    t2sd = t['value']
	    ms_time2 = qa.time(t,form="ymd",prec=9)
	    casalog.post('MS spans timerange '+ms_time1+' to '+ms_time2)
    else:
	    print 'WARNING: Could not open vis as MS to find times'
	    casalog.post('WARNING: Could not open vis as MS to find times')
    return (ms_startmjds, ms_endmjds, ms_time1, ms_time2)

#===============================================================================
# Flag dictionary manipulation routines
# Same versions as flagcmd v4.2 2012-02-27
#===============================================================================

def readflagxml(sdmfile, mytbuff):
#
#   readflagxml: reads Antenna.xml and Flag.xml SDM tables and parses
#                into returned dictionary as flag command strings
#      sdmfile (string)  path to SDM containing Antenna.xml and Flag.xml
#      mytbuff (float)   time interval (start and end) padding (seconds)
#
#   Usage: myflags = readflagxml(sdmfile,tbuff)
#
#   Dictionary structure:
#   fid : 'id' (string)
#         'mode' (string)         flag mode ('online')
#         'antenna' (string)
#         'timerange' (string)
#         'reason' (string)
#         'time' (float)          in mjd seconds
#         'interval' (float)      in mjd seconds
#         'cmd' (string)          string (for COMMAND col in FLAG_CMD)
#         'type' (string)         'FLAG' / 'UNFLAG'
#         'applied' (bool)        set to True here on read-in
#         'level' (int)           set to 0 here on read-in
#         'severity' (int)        set to 0 here on read-in
#      Optional keys:
#         'spw' (string)
#         'poln' (string)
#
#   Updated STM 2011-11-02 handle new SDM Flag.xml format from ALMA
#   Updated STM 2012-02-14 handle spectral window indices, names, IDs
#   Updated STM 2012-02-21 handle polarization types
#
#   Mode to use for spectral window selection in commands:
#   spwmode =  0 none (flag all spw)
#   spwmode =  1 use name
#   spwmode = -1 use index (counting rows in SpectralWindow.xml)
#
#   Mode to use for polarization selection in commands:
#   polmode =  0 none (flag all pols/corrs)
#   polmode =  1 use polarization type
#
#   CURRENT DEFAULT: Use spw names, flag pols
    spwmode = 1
    polmode = 1

#
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    if type(mytbuff)!=float:
        casalog.post('Warning: incorrect type for tbuff, found "'+str(mytbuff)+'", setting to 1.0')
        mytbuff=1.0

    # make sure Flag.xml and Antenna.xml are available (SpectralWindow.xml depends)
    flagexist = os.access(sdmfile+'/Flag.xml',os.F_OK)
    antexist = os.access(sdmfile+'/Antenna.xml',os.F_OK)
    spwexist = os.access(sdmfile+'/SpectralWindow.xml',os.F_OK)
    if not flagexist:
        print 'ERROR: Cannot open '+sdmfile+'/Flag.xml'
	casalog.post('Cannot open '+sdmfile+'/Flag.xml','SEVERE')
	exit(1)
    if not antexist:
        print 'ERROR: Cannot open '+sdmfile+'/Antenna.xml'
	casalog.post('Cannot open '+sdmfile+'/Antenna.xml','SEVERE')
	exit(1)
    if not spwexist:
	casalog.post('Cannot open '+sdmfile+'/SpectralWindow.xml','WARN')

    # construct look-up dictionary of name vs. id from Antenna.xml
    xmlants = minidom.parse(sdmfile+'/Antenna.xml')
    antdict = {}
    rowlist = xmlants.getElementsByTagName("row")
    for rownode in rowlist:
        rowname = rownode.getElementsByTagName("name")
        ant = str(rowname[0].childNodes[0].nodeValue)
        rowid = rownode.getElementsByTagName("antennaId")
        antid = str(rowid[0].childNodes[0].nodeValue)
        antdict[antid] = ant
    print '  Found '+str(rowlist.length)+' antennas in Antenna.xml'
    casalog.post('Found '+str(rowlist.length)+' antennas in Antenna.xml')

    # construct look-up dictionary of name vs. id from SpectralWindow.xml
    if spwexist:
        xmlspws = minidom.parse(sdmfile+'/SpectralWindow.xml')
	spwdict = {}
	rowlist = xmlspws.getElementsByTagName("row")
	ispw = 0
	for rownode in rowlist:
	    rowname = rownode.getElementsByTagName("name")
	    spw = str(rowname[0].childNodes[0].nodeValue)
	    rowid = rownode.getElementsByTagName("spectralWindowId")
	    spwid = str(rowid[0].childNodes[0].nodeValue)
	    spwdict[spwid] = {}
	    spwdict[spwid]['name'] = spw
	    spwdict[spwid]['index'] = ispw
	    ispw += 1
	print '  Found '+str(rowlist.length)+' spw in SpectralWindow.xml'
	casalog.post('Found '+str(rowlist.length)+' spw in SpectralWindow.xml')

    # report chosen spw and pol modes
    if spwmode>0:
	    print 'Will construct spw flags using Names'
	    casalog.post('Will construct spw flags using names')
    elif spwmode<0:
	    print 'Will construct spw flags using table indices'
	    casalog.post('Will construct spw flags using table indices')
    else:
	    print 'Will not set spw dependent flags (flag all spws)'
	    casalog.post('')
    #
    if polmode==0:
	    print 'Will not set polarization dependent flags (flag all corrs)'
	    casalog.post('Will not set polarization dependent flags (flag all corrs)')
    else:
	    print 'Will construct polarization flags using polarizationType'
	    casalog.post('Will construct polarization flags using polarizationType')
    
    # now read Flag.xml into dictionary row by row
    xmlflags = minidom.parse(sdmfile+'/Flag.xml')
    flagdict = {}
    rowlist = xmlflags.getElementsByTagName("row")
    nrows = rowlist.length
    newsdm = -1
    newspw = -1
    newpol = -1
    for fid in range(nrows):
        rownode = rowlist[fid]
        rowfid = rownode.getElementsByTagName("flagId")
        fidstr = str(rowfid[0].childNodes[0].nodeValue)
        flagdict[fid] = {}
        flagdict[fid]['id'] = fidstr
	rowid = rownode.getElementsByTagName("antennaId")
	antid = rowid[0].childNodes[0].nodeValue
	# check if there is a numAntenna specified (new format)
	rownant = rownode.getElementsByTagName("numAntenna")
	antname = ''
	if rownant.__len__()>0:
		xid = antid.split()
		nant = int(rownant[0].childNodes[0].nodeValue)
		if newsdm<0:
			print '  Found numAntenna='+str(nant)+' must be a new style SDM'
			casalog.post('Found numAntenna='+str(nant)+' must be a new style SDM')
		newsdm=1
		if nant>0:
			for ia in range(nant):
				aid = xid[2+ia]
				ana = antdict[aid]
				if antname=='':
					antname=ana
				else:
					antname+=','+ana
		else:
			# numAntenna = 0 means flag all antennas
			antname = ''
	else:
		if newsdm<0:
			print '  No numAntenna entry found, must be a old style SDM'
			casalog.post('No numAntenna entry found, must be a old style SDM')
		newsdm=0
		nant = 1
		aid = antid
		ana = antdict[aid]
		antname = ana
        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName("startTime")
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjds = (float(start)*1.0E-9) - mytbuff
        t = qa.quantity(startmjds,'s')
        starttime = qa.time(t,form="ymd",prec=9)
        rowend = rownode.getElementsByTagName("endTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjds = (float(end)*1.0E-9) + mytbuff
        t = qa.quantity(endmjds,'s')
        endtime = qa.time(t,form="ymd",prec=9)
	# time and interval for FLAG_CMD use
	times = 0.5*(startmjds+endmjds)
	intervs = endmjds-startmjds
        flagdict[fid]['time'] = times
        flagdict[fid]['interval'] = intervs
        # reasons
        rowreason = rownode.getElementsByTagName("reason")
        reas = str(rowreason[0].childNodes[0].nodeValue)
	# NEW SDM ADDITIONS 2011-11-01
	rownspw = rownode.getElementsByTagName("numSpectralWindow")
	spwstring = ''
	if spwmode!=0 and rownspw.__len__()>0:
		nspw = int(rownspw[0].childNodes[0].nodeValue)
		# has a new-style spw specification
		if newspw<0:
			if not spwexist:
				print 'ERROR: Cannot open '+sdmfile+'/SpectralWindow.xml'
				casalog.post('Cannot open '+sdmfile+'/SpectralWindow.xml','SEVERE')
				exit(1)
			print '  Found numSpectralWindow='+str(nspw)+' must be a new style SDM'
			casalog.post('Found SpectralWindow='+str(nspw)+' must be a new style SDM')
		newspw=1
		if nspw>0:
			rowspwid = rownode.getElementsByTagName("spectralWindowId")
			spwids = rowspwid[0].childNodes[0].nodeValue
			xspw = spwids.split()
			for isp in range(nspw):
				spid = str(xspw[2+isp])
				if spwmode>0:
					spstr = spwdict[spid]['name']
				else:
					spstr = str( spwdict[spid]['index'] )
				if spwstring=='':
					spwstring=spstr
				else:
					spwstring+=','+spstr
	polstring = ''
	rownpol = rownode.getElementsByTagName("numPolarizationType")
	if polmode!=0 and rownpol.__len__()>0:
		npol = int(rownpol[0].childNodes[0].nodeValue)
		# has a new-style pol specification
		if newpol<0:
			print '  Found numPolarizationType='+str(npol)+' must be a new style SDM'
			casalog.post('Found numPolarizationType='+str(npol)+' must be a new style SDM')
		newpol=1
		if npol>0:
			rowpolid = rownode.getElementsByTagName("polarizationType")
			polids = rowpolid[0].childNodes[0].nodeValue
			xpol = polids.split()
			for ipol in range(npol):
				polid = str(xpol[2+ipol])
				if polstring=='':
					polstring=polid
				else:
					polstring+=','+polid
	#
        # Construct antenna name and timerange and reason strings
        flagdict[fid]['antenna'] = antname
        timestr = starttime+'~'+endtime
        flagdict[fid]['timerange'] = timestr
        flagdict[fid]['reason'] = reas
        # Construct command strings (per input flag)
        cmd = "antenna='"+antname+"' timerange='"+timestr+"'"
	if spwstring!='':
		cmd += " spw='"+spwstring+"'"
		flagdict[fid]['spw'] = spwstring
	if polstring!='':
	 	cmd += " poln='"+polstring+"'"
		flagdict[fid]['poln'] = polstring
        flagdict[fid]['cmd'] = cmd
	#
	flagdict[fid]['type'] = 'FLAG'
	flagdict[fid]['applied'] = False
	flagdict[fid]['level'] = 0
	flagdict[fid]['severity'] = 0
	flagdict[fid]['mode'] = 'online'
	
    flags = {}
    if rowlist.length > 0:
        flags = flagdict
	print '  Found '+str(rowlist.length)+' flags in Flag.xml'
	casalog.post('Found '+str(rowlist.length)+' flags in Flag.xml')
    else:
        print 'No valid flags found in Flag.xml'
	casalog.post('No valid flags found in Flag.xml')

    # return the dictionary for later use
    return flags
# Done

def sortflags(myflags=None,myantenna='',myreason='Any',myflagsort=''):
    #
    # Return dictionary of input flags using selection by antenna/reason
    # and grouped/sorted by flagsort.
    #
    #   sortflags: Return dictionary of flags using selection by antenna/reason
    #              and grouped/sorted by flagsort.
    #      myflags (dictionary)  input flag dictionary (e.g. from readflagxml
    #      myantenna (string)    selection by antenna(s)
    #      myreason (string)     selection by reason(s)
    #      myflagsort (string)   toggle for flag group/sort
    #
    #   Usage: myflagd = sortflags(myflags,antenna,reason,flagsort)
    #
    #   Dictionary structure:
    #   fid : 'id' (string)
    #         'mode' (string)         flag mode '','clip','shadow','quack','online'
    #         'antenna' (string)
    #         'timerange' (string)
    #         'reason' (string)
    #         'time' (float)          in mjd seconds
    #         'interval' (float)      in mjd seconds
    #         'cmd' (string)          command string (for COMMAND col in FLAG_CMD)
    #         'type' (string)         'FLAG' / 'UNFLAG'
    #         'applied' (bool)        set to True here on read-in
    #         'level' (int)           set to 0 here on read-in
    #         'severity' (int)        set to 0 here on read-in
    # Optional keys:
    #         'spw' (string)
    #         'poln' (string)
    #
    # NOTE: flag sorting is possibly needed to avoid error "Too many flagging agents
    # instantiated" and will generally speed up flagging in any event
    # If myflagsort=''              keep individual flags separate
    #              ='antenna'       combine all flags with a particular antenna
    #              ='antspw'        combine antenna+spw flags
    #              ='antpol'        combine antenna+spw+poln flags
    #
    # Updated STM 2012-02-21 handle poln flags, myflagsort='antspw'
    # Updated STM 2012-02-27 small cleanup
    #
    # Check if any operation is needed
    if myantenna=='' and myreason=='' and myflagsort=='':
        print 'No selection or sorting needed - sortflags returning input dictionary'
        casalog.post('No selection or sorting needed - sortflags returning input dictionary')
        flagd = myflags
	return flagd
    #
    if myflagsort=='antenna':
        print 'Will sort flags by antenna'
	casalog.post('Will sort flags by antenna')
    elif myflagsort=='antspw':
        print 'Will sort flags by antenna+spw'
	casalog.post('Will sort flags by antenna+spw')
    elif myflagsort=='antpol':
        print 'Will sort flags by antenna+spw+pol'
	casalog.post('Will sort flags by antenna+spw+pol')
    else:
        print 'Will not sort flags'
	casalog.post('Will not sort flags')
    #
    flagd={}
    nflagd = 0
    keylist = myflags.keys()
    print 'Selecting from '+str(keylist.__len__())+' flagging commands'
    if keylist.__len__() > 0:
        #
        # Sort by key
        #
        keylist.sort()
        #
        # Construct flag command list for selected ant,reason
        #
	#print 'Selecting flags by antenna="'+str(myantenna)+'"'
	casalog.post('Selecting flags by antenna="'+str(myantenna)+'"')
	myantlist = myantenna.split(',')

	#print 'Selecting flags by reason="'+str(myreason)+'"'
	casalog.post('Selecting flags by reason="'+str(myreason)+'"')
	myreaslist = []
	# Parse myreason
	if type(myreason)==str:
	    if myreason=='':
	        print 'WARNING: reason='' is treated as selection on a blank REASON!'
		casalog.post('WARNING: reason='' is treated as selection on a blank REASON!', 'WARN')
	    if myreason!='Any':
	        myreaslist.append(myreason)
	elif type(myreason)==list:
	    myreaslist=myreason
        else:
	    print 'ERROR: reason contains unallowed variable type'
	    casalog.post('ERROR: reason contains unknown variable type','SEVERE')
	    return
        if myreaslist.__len__()>0:
	    print 'Selecting for reasons: '+str(myreaslist)
	    casalog.post('Selecting for reasons: '+str(myreaslist))
        else:
	    print 'No selection on reason'
	    casalog.post('No selection on reason')
    
	# Note antenna and reason selection checks for inclusion not exclusivity
	doselect = myantenna!='' or myreaslist.__len__()>0

	# Now loop over flags, break into sorted and unsorted groups
	nsortd = 0
	sortd = {}
	sortdlist = []
	nunsortd = 0
	unsortd = {}
	unsortdlist = []
	if myflagsort=='antenna' or myflagsort=='antspw' or myflagsort=='antpol':
	    # will be resorting by antenna
	    for key in keylist:
	        myd = myflags[key]
		mymode = myd['mode']
		ant = myd['antenna']
		# work out number of selections for this flag command
		nselect = 0
		if myd.has_key('timerange'):
		    if myd['timerange']!='': nselect += 1
		if (myflagsort!='antspw' and myflagsort!='antpol') and myd.has_key('spw'):
		    if myd['spw']!='': nselect += 1
		if myd.has_key('field'):
		    if myd['field']!='': nselect += 1
		if myd.has_key('correlation'):
		    if myd['correlation']!='': nselect += 1
		if myflagsort!='antpol' and myd.has_key('poln'):
		    if myd['poln']!='': nselect += 1
		if myd.has_key('scan'):
		    if myd['scan']!='': nselect += 1
                if myd.has_key('intent'):
                    if myd['intent'] != '': nselect += 1
		if myd.has_key('feed'):
		    if myd['feed']!='': nselect += 1
		if myd.has_key('uvrange'):
		    if myd['uvrange']!='': nselect += 1
                if myd.has_key('observation'):
                    if myd['observation'] != '': nselect += 1
		# check if we can sort this by antenna
		antsort = False
		# can only sort mode manualflag together
		# must have non-blank antenna selection
		if ant!='':
		    if mymode=='online':
		        antsort = nselect==1
		    elif mymode=='' or mymode=='manualflag':
		        # exclude flags with multiple/no selection
			if myd.has_key('timerange'):
			    antsort = myd['timerange']!='' and nselect==1
			
		if antsort:
		    # the antennas
		    if ant.count('&')>0:
		        # for baseline specifications split on ;
		        antlist = ant.split(';')
		    else:
		        # for antenna specifications split on ,
		        antlist = ant.split(',')
		    # the spw
		    if (myflagsort=='antspw' or myflagsort=='antpol') and myd.has_key('spw'):
		        spwsort = True
		        spw = myd['spw']
		        spwlist = spw.split(',')
		    else:
		        spwsort = False
			spw = ''
		    if myflagsort=='antpol' and myd.has_key('pol'):
		        polsort = True
			poln = myd['poln']
		        polnlist = poln.split(',')
		    else:
		        polsort = False
			poln = ''
		    
		    # construct the joint list
		    jlist = []
		    jdict = {}
		    for a in antlist:
		        if spwsort:
			    for s in spwlist:
			        if polsort:
				    for p in polnlist:
				        jx = a+'_spw'+s+'_pol'+p
					jlist.append(jx)
					jdict[jx]={}
					jdict[jx]['antenna']=a
					jdict[jx]['spw']=s
					jdict[jx]['poln']=p
				else:
				    jx = a+'_spw'+s
				    jlist.append(jx)
				    jdict[jx]={}
				    jdict[jx]['antenna']=a
				    jdict[jx]['spw']=s
				    jdict[jx]['poln']=''
			elif polsort:
			    for p in polnlist:
			        jx = a+'_pol'+p
				jlist.append(jx)
				jdict[jx]={}
				jdict[jx]['antenna']=a
				jdict[jx]['spw']=''
				jdict[jx]['poln']=p
			else:
			    jx = a
			    jlist.append(jx)
			    jdict[jx]={}
			    jdict[jx]['antenna']=a
			    jdict[jx]['spw']=''
			    jdict[jx]['poln']=''

		    # break this flag by the criteria
		    for a in jlist:
		        if myantenna=='' or myantlist.count(a)>0:
			    addf = False
			    reas = myd['reason']
			    reaslist = reas.split(',')
			    reastr = ''
			    if myreaslist.__len__()>0:
			        for r in myreaslist:
				    if r==reas or reaslist.count(r)>0:
				        addf = True
					# check if this is a new reason
					if reastr!='':
					    rlist = reastr.split(',')
					    if rlist.count(r)==0:
					        reastr += ','+r
					else:
					    reastr = r
			    else:
			        addf=True
				reastr = reas
			    #
			    if addf:
			        # check if this is a new item
				if sortd.has_key(a):
			            # Already existing flag for this antenna, add this one
				    t = sortd[a]['timerange']
				    tlist = t.split(',')
				    timelist = myd['timerange'].split(',')
				    for tim in timelist:
			                # check if this is a new timerange
					if tlist.count(tim)==0:
				            t += ',' + tim
				    sortd[a]['timerange'] = t
				    reas = sortd[a]['reason']
				    if reastr!='':
				        reas += ',' + reastr
				    sortd[a]['reason'] = reas
				    # adjust timerange in command string
				    cmdstr = ''
				    cmdlist = sortd[a]['cmd'].split()
				    for cmd in cmdlist:
				        (cmdkey,cmdval) = cmd.split('=')
					if cmdkey=='timerange':
					    cmdstr += " timerange='"+t+"'"
					elif cmdkey=='reason':
					    cmdstr += " reason='"+reastr+"'"
					else:
					    cmdstr += " "+cmd
				    sortd[a]['cmd'] = cmdstr
				    # adjust other keys
				    if myd['level']>sortd[a]['level']:
				        sortd[a]['level']=myd['level']
				    if myd['severity']>sortd[a]['severity']:
				        sortd[a]['severity']=myd['severity']
		                else:
			            # add this flag (copy most of it)
				    sortd[a] = myd
				    sortd[a]['id'] = a
				    sortd[a]['antenna'] = jdict[a]['antenna']
				    if spwsort:
					    sortd[a]['spw'] = jdict[a]['spw']
				    if polsort:
					    sortd[a]['poln'] = jdict[a]['poln']
				    sortd[a]['reason'] = reastr
		else:
		    # cannot compress flags from this mode, add to unsortd instead
		    # doesn't clash
		    unsortd[nunsortd] = myd
		    nunsortd += 1
	    sortdlist = sortd.keys()
	    nsortd = sortdlist.__len__()
	    unsortdlist = unsortd.keys()
	else:
	    # All flags are in unsorted list
	    unsortd = myflags.copy()
	    unsortdlist = unsortd.keys()
	    nunsortd = unsortdlist.__len__()

	print 'Found '+str(nsortd)+' sorted flags and '+str(nunsortd)+' incompressible flags'
	casalog.post('Found '+str(nsortd)+' sorted flags and '+str(nunsortd)+' incompressible flags')

	# selection on unsorted flags
	if doselect and nunsortd>0:
	    keylist = unsortd.keys()
	    for key in keylist:
	        myd = unsortd[key]
		ant = myd['antenna']
		antlist = ant.split(',')
		reas = myd['reason']
		reaslist = reas.split(',')
		# break this flag by antenna
		antstr = ''
		reastr = ''
		addf = False
		
		for a in antlist:
		    if myantenna=='' or myantlist.count(a)>0:
		        addr = False
			if myreaslist.__len__()>0:
		            for r in myreaslist:
			        if reas==r or reaslist.count(r)>0:
			            addr = True
				    # check if this is a new reason
				    rlist = reastr.split(',')
				    if reastr!='':
				        rlist = reastr.split(',')
					if rlist.count(r)==0:
				            reastr += ','+r
				    else:
				        reastr = r
			else:
			    addr = True
			    reastr = reas
			if addr:
			    addf = True
			    if antstr!='':
			        # check if this is a new antenna
			        alist = antstr.split(',')
			        if alist.count(a)==0:
				    antstr += ','+a
			    else:
			        antstr = a
		if addf:
		    flagd[nflagd] = myd
		    flagd[nflagd]['antenna'] = antstr
		    flagd[nflagd]['reason'] = reastr
		    nflagd += 1
	    flagdlist = flagd.keys()
	elif nunsortd>0:
	    # just copy to flagd w/o selection
	    flagd = unsortd.copy()
	    flagdlist = flagd.keys()
	    nflagd = flagdlist.__len__()

        if nsortd>0:
	    # Add sorted keys back in to flagd
	    print 'Adding '+str(nsortd)+' sorted flags to '+str(nflagd)+' incompressible flags'
	    casalog.post('Adding '+str(nsortd)+' sorted flags to '+str(nflagd)+' incompressible flags')
	    sortdlist.sort()
	    for skey in sortdlist:
	        flagd[nflagd] = sortd[skey]
		nflagd += 1

        if nflagd>0:
	    print 'Found total of '+str(nflagd)+' flags meeting selection/sorting criteria'
	    casalog.post('Found total of '+str(nflagd)+' flags meeting selection/sorting criteria')
	else:
            print 'No flagging commands found meeting criteria'
            casalog.post('No flagging commands found meeting criteria')
    else:
        print 'No flags found in input dictionary'
        casalog.post('No flags found in input dictionary')

    return flagd
# Done

def writeflagcmd(msfile,myflags,tag=''):
    #
    # Takes input flag dictionary (e.g. from readflagxml) 
    # Save the flag commands to the FLAG_CMD table for msfile
    # Can change applied tag if requested (tag='applied' or tag='unapplied')
    # Returns number of flags written
    #
    nadd = 0
    try:
        import pylab as pl
    except ImportError, e:
        print "failed to load pylab:\n", e
        exit(1)
    #
    # Append new commands to existing table
    keylist = myflags.keys()
    nkeys = keylist.__len__()
    if nkeys>0:
        # Extract flags from dictionary into list
	tim_list = []
	intv_list = []
	cmd_list = []
	reas_list = []
	typ_list = []
	sev_list = []
	lev_list = []
	app_list = []
	for key in keylist:
	    tim_list.append( myflags[key]['time'] )
	    intv_list.append( myflags[key]['interval'] )
	    reas_list.append( myflags[key]['reason'] )
	    cmd_list.append( myflags[key]['cmd'] )
	    #
	    typ_list.append( myflags[key]['type'] )
	    sev_list.append( myflags[key]['severity'] )
	    lev_list.append( myflags[key]['level'] )
	    if tag=='applied':
		    appl = True
	    elif tag=='unapplied':
		    appl = False
	    else:
		    appl = myflags[key]['applied']
	    app_list.append( appl )
	#
	# Save to FLAG_CMD table
	nadd = cmd_list.__len__()
	print 'Saving '+str(nadd)+' flags to FLAG_CMD MS table'
	casalog.post('Saving '+str(nadd)+' flags to FLAG_CMD MS table')
	mstable = msfile+'/FLAG_CMD'
	try:
		tb.open(mstable,nomodify=False)
	except:
		raise Exception, "Error opening FLAG_CMD table "+mstable
	nrows = int(tb.nrows())
	print 'There are '+str(nrows)+' rows already in FLAG_CMD'
	casalog.post('There are '+str(nrows)+' rows already in FLAG_CMD')
	# add blank rows
	tb.addrows(nadd)
	# now fill them in
	tb.putcol('TIME',pl.array(tim_list),startrow=nrows,nrow=nadd)
	tb.putcol('INTERVAL',pl.array(intv_list),startrow=nrows,nrow=nadd)
	tb.putcol('REASON',pl.array(reas_list),startrow=nrows,nrow=nadd)
	tb.putcol('COMMAND',pl.array(cmd_list),startrow=nrows,nrow=nadd)
	# Other columns
	tb.putcol('TYPE',pl.array(typ_list),startrow=nrows,nrow=nadd)
	tb.putcol('SEVERITY',pl.array(sev_list),startrow=nrows,nrow=nadd)
	tb.putcol('LEVEL',pl.array(lev_list),startrow=nrows,nrow=nadd)
	tb.putcol('APPLIED',pl.array(app_list),startrow=nrows,nrow=nadd)
	tb.close()
	#
	print 'Wrote '+str(nadd)+' rows to FLAG_CMD'
	casalog.post('Wrote '+str(nadd)+' rows to FLAG_CMD')
    else:
        print 'Added zero rows to FLAG_CMD, no flags found'
	casalog.post('Added zero rows to FLAG_CMD, no flags found')

    return nadd
# Done

def updateflagcmd(msfile,mycol='',myval=None,myrowlist=[]):
    #
    # Update commands in myrowlist of the FLAG_CMD table of msfile
    #
    # Usage: updateflagcmd(msfile,myrow,mycol,myval)
    # Example:
    #
    #    updateflagcmd(msfile,mycol='APPLIED',myval=True)
    #       Mark all rows as APPLIED=True
    #
    #    updateflagcmd(msfile,mycol='APPLIED',myval=True,myrowlist=[0,1,2])
    #       Mark rows 0,1,2 as APPLIED=True
    #
    if mycol=='':
	    print 'WARNING: No column to specified for updateflagcmd, doing nothing'
	    casalog.post('WARNING: No column to specified for updateflagcmd, doing nothing','WARN')
	    return

    # Open and read columns from FLAG_CMD
    mstable = msfile+'/FLAG_CMD'
    try:
	    tb.open(mstable,nomodify=False)
    except:
	    raise Exception, "Error opening table "+mstable
    nrows = int(tb.nrows())
    #casalog.post('There were '+str(nrows)+' rows in FLAG_CMD')
    #
    # Check against allowed colnames
    colnames = tb.colnames()
    if colnames.count(mycol)<1:
	    print 'Error: column mycol='+mycol+' not one of: '+str(colnames)
	    casalog.post('Error: column mycol='+mycol+' not one of: '+str(colnames))
	    return
    
    nlist = myrowlist.__len__()
    if nlist>0:
	    rowlist = myrowlist
	    #casalog.post('Will update column '+mycol+' for rows '+str(rowlist))
	    casalog.post('Will update column '+mycol+' for '+str(nlist)+' rows')
    else:
	    rowlist = range(nrows)
	    nlist = nrows
	    casalog.post('Will update column '+mycol+' for all rows')
	    
    if nlist>0:
	    try:
		    tb.putcell(mycol,rowlist,myval)
	    except:
		    raise Exception, "Error updating FLAG_CMD column "+mycol+" to value "+str(myval)
	    
	    print 'Updated '+str(nlist)+' rows of FLAG_CMD table in MS'
	    casalog.post('Updated '+str(nlist)+' rows of FLAG_CMD table in MS')
    tb.close()
    
# Done

#===============================================================================
# Apply flag commands using flagger tool
# Same versions as flagcmd v4.2 2012-02-21
#===============================================================================

def applyflagcmd(fglocal, msfile, flagbackup, myflags, reset=False, flagtype='Unset'):
        #
	# Takes input flag dictionary myflags (e.g. from readflagxml) 
	# and applies using flagger tool to MS msfile.
	#
	# If flagbackup=True will save copy of flags before flagging.
	#
	# If reset=True will reset flags before flagging.
	#
	# If flagtype='FLAG' or 'UNFLAG' will override the unflag choice in the 
	# individual flags
	#
	# Returns number of flags applied
	#
        # Updated STM 2012-02-21 handle polarization (poln) flags
	#
	ncmd = 0

        #fglocal.done()
        #fglocal.clearflagselection(-1)

	if flagtype=='FLAG' or flagtype=='flag':
	    mytype='FLAG'
	elif flagtype=='UNFLAG' or flagtype=='unflag':
	    mytype='UNFLAG'
	else:
	    mytype='Unset'

	try:
	    if ((type(msfile)==str) & (os.path.exists(msfile))):
	        fglocal.open(msfile)
	    else:
	        print 'ERROR MS '+msfile+' not found'
		casalog.post('ERROR MS '+msfile+' not found','SEVERE')
		return ncmd

	    # ============================
	    # Important boilerplate:
	    # Define lists of valid params by mode
	    kmodes = {}
	    # Keys to ignore
	    iparams = ['reason','flagtime','id','level','severity'] # currently not used
	    # Keys for online flags
	    oparams = ['antenna','timerange','correlation','feed','array','spw','field','poln']
	    # Keys to recognize uparams=Universal sparams=Selection
	    uparams = [] 
	    # SMC CAS-3320: added observation and intent
	    sparams = ['antenna','timerange','correlation','scan','intent','feed','array','uvrange','observation','spw','field']
	    aparams = uparams + sparams
	    kmodes['online'] = oparams
	    kmodes['manualflag'] = aparams + ['unflag']
	    kmodes['clip'] = aparams + ['unflag','cliprange','clipexpr','clipcolumn','clipchanavg']
	    kmodes['quack'] = aparams + ['unflag','quackinterval','quackmode','quackincrement']
	    kmodes['shadow'] = aparams + ['diameter']
	    # ============================
        
	    # prepare to input flagging commands
	    keylist = myflags.keys()
	    keylist.sort()
	    nkeys = keylist.__len__()
	    casalog.post('Found '+str(nkeys)+' flags to apply')
	    if nkeys>0:
	        fglocal.setdata()
		cmdlist = []
		param_set = {}
		for key in keylist:
	            # Get command from dictionary (must be there)
	            cmd = myflags[key]['cmd']
		    # Get optional type from dictionary
		    if myflags[key].has_key('type'):
		        intype=myflags[key]['type']
			if intype=='FLAG' or intype=='flag':
			    intype='FLAG'
		        elif intype=='UNFLAG' or intype=='unflag':
			    intype='UNFLAG'
		        else:
			    intype='Unset'
		    else:
		        intype='Unset'
			    
		    if debug: print 'Processing command '+cmd
		    # Parse each command - currently just split by whitespace into key=value strings
		    params = {}
		    param_list = ''
		    # First ignore comment lines starting with "#"
		    if cmd[:1]!='#':
			params = parse_cmd(cmd)

			# should now have list of parsed params and values
			if params.__len__() > 0:
			    if debug: print params
			    # Check if a known mode
			    if params.has_key('mode'):
				mode = params['mode']
				if mode=='': mode = 'manualflag'
			    elif params.has_key('cliprange'):
				mode = 'clip'
			    elif params.has_key('quackinterval'):
				mode = 'quack'
			    elif myflags[key].has_key('mode'):
			        mode = myflags[key]['mode']
				if mode=='': mode = 'manualflag'
			    else:
			        mode = 'manualflag'
			    if mode!='shadow':
			        # Logic tree for unflag
			        if mytype=='FLAG':
			            # user has chosen to override flag
				    dounflag=False
			        elif mytype=='UNFLAG':
			            # user has chosen to override unflag
				    dounflag=True
				elif params.has_key('unflag'):
				    # explict unflag in command string
				    if params['unflag']:
				        dounflag=True
				    else:
				        dounflag=False
				elif intype=='FLAG':
			            # user has chosen to override flag
				    dounflag=False
			        elif intype=='UNFLAG':
			            # user has chosen to override unflag
				    dounflag=True
			        else:
				    # default to flagging
				    dounflag=False
				# Now implement choice (possibly adding)
				params['unflag'] = dounflag
						
			    if kmodes.has_key(mode):
				# valid mode, check for known params
				param_i = {}
				param_list = 'mode='+mode
				for p in params.keys():
				    if p != 'mode':
				        v = params[p]
				        if kmodes[mode].count(p)>0:
					    # known param for this mode, keep
					    param_i[p] = v
					    if param_list != '':
					        param_list += '; '
						param_list = param_list+p+'='+str(v)
					    elif iparams.count(p)==0:
					        # Not an ignored key and unknown for this mode
						#raise Exception(str(x)+' has unknown key')
						print str(x)+' has unknown key'
				# Have list of params (besides a mode), may be zero length
				#casalog.post(param_list)
				#the above was too long for logger, truncate
				pstr = param_list[:128]
				casalog.post(pstr)
				# Special cases of parameter mapping to flagger tool
				parse_cmdparams(param_i)
				if debug: print param_i
				# Now save by mode
				if param_set.has_key(mode):
				    n = param_set[mode].__len__()
				    name = mode+'_'+str(n)
				    param_set[mode][name] = param_i.copy()
				else:
				    name = mode+'_0'
				    param_set[mode]={}
				    param_set[mode][name] = param_i.copy()
				cmdlist.append(param_list)
				# Done with this flag command
			    else:
				print ' Warning: ignoring unknown mode '+mode
	        # Were any valid flagging commands set up?
		if cmdlist.__len__()>0:
		    if reset:
		        print 'WARNING: Will reset flags before application'
			casalog.post('Will reset flags before application','WARN')
		# Process these for each mode
		modelist = param_set.keys()
		modelist.sort()
		for mode in modelist:
		    nf = param_set[mode].__len__()
		    if nf > 0:
			print 'Processing '+str(nf)+' flagging commands for mode '+mode
			casalog.post('Processing '+str(nf)+' flagging commands for mode '+mode)
			fglocal.setdata()
			fglocal.clearflagselection(-1)
			for s in param_set[mode].keys():
			    param_i = param_set[mode][s]
			    if mode=='shadow':
			        if debug: print 'Applying shadow with params: ',param_i
				if param_i.__len__()>0:
				    fglocal.setshadowflags(**param_i)
			        else:
				    fglocal.setshadowflags()
			    else:
			        if param_i.__len__()>0:
				    fglocal.setmanualflags(**param_i)
			        else:
				    fglocal.setmanualflags()
			if flagbackup:
			    backup_cmdflags(fglocal, 'flagcmd_'+mode)
			if reset:
			    fglocal.run(reset=True)
		        else:
			    fglocal.run()
			print 'Applied '+str(nf)+' flagging commands for mode '+mode
			casalog.post('Applied '+str(nf)+' flagging commands for mode '+mode)
		
		# Were any valid flagging commands executed?
		ncmd = cmdlist.__len__()
		if ncmd > 0:
		    print 'Applied '+str(ncmd)+' total flagging commands'
		    casalog.post('Applied '+str(ncmd)+' total flagging commands')
	        else:
		    print ' Warning: no valid flagging commands executed'
		    casalog.post('Warning: no valid flagging commands executed')

        except Exception, instance:
                fglocal.done()
                print '*** Error ***', instance
                #raise
        fglocal.done()

	return ncmd

def parse_cmd(cmd):
	# Parse a command string into param key=val pairs
	params = {}
	param_list = ''
	mycmd = cmd.split()
	for x in mycmd:
		# expect each atomic command to be a key=value pair, split on '='
		# expects only one = in string
		if x.count('=')==1:
			xs = x.split('=')
		elif x=='#':
			# start of a comment
			break
		else:
			raise Exception(str(x)+' not a key=value pair')
		xkey=xs[0]
		xval=xs[1]
		if xkey=='':
			raise Exception(str(x)+' no non-blank key')
		# strip external quotes from key (added STM 20111019)
		if xkey.count("'")>0: xkey=xkey.strip("'")
		if xkey.count('"')>0: xkey=xkey.strip('"')

		if debug: print xkey+' has value '+str(xval)
		# check to see if this one is a repeat
		if params.has_key(xkey):
			# a repeat
			print str(x)+' has duplicate key, using only first instance'
		else:
			# strip external quotes from value
			if xval.count("'")>0: xval=xval.strip("'")
			if xval.count('"')>0: xval=xval.strip('"')
			params[xkey] = xval

	return params

def parse_cmdparams(params):
	# rename some parameters,
	# in order to match the interface of fg.tool
	#
	# validate parameter quackmode
	#
        # Updated STM v4.2 2012-02-21 handle polarization (poln) flags
	#

        if params.has_key('quackmode') and \
          not params['quackmode'] in ['beg', 'endb', 'end', 'tail']:
                raise Exception, "Illegal value '%s' of parameter quackmode, must be either 'beg', 'endb', 'end' or 'tail'" % (params['quackmode'])
        
	if params.has_key('antenna'):
		params['baseline']=params['antenna']
		del(params['antenna'])
	if params.has_key('timerange'):
		params['time']=params['timerange']
		del(params['timerange'])
	if params.has_key('poln'):
		# STM 2012-02-21 Kludge for no poln selection
		# flags all cross-corrs
		# does not handle mixed linears and circs
		pp = params['poln']
		if pp.count('R')>0:
			if pp.count('L')>0:
				corr = 'RR,RL,LR,LL'
			else:
				corr = 'RR,RL,LR'
		elif pp.count('L')>0:
			corr = 'LL,LR,RL'
		elif pp.count('X')>0:
			if pp.count('Y')>0:
				corr = 'XX,XY,YX,YY'
			else:
				corr = 'XX,XY,YX'
		elif pp.count('Y')>0:
			corr = 'YY,YX,XY'
		#	
		if params.has_key('correlation'):
			corp = params['correlation']
			params['correlation']=corp + ',' + corr
		else:
			params['correlation']=corr
		del(params['poln'])
	if params.has_key('cliprange'):
		v = params['cliprange']
		# turn string into [min,max] range
		r = v.split('~')
		rmin = float(r[0])
		rmax = float(r[1])
		params['cliprange'] = [rmin,rmax]
		params['outside']=False
	if params.has_key('clipoutside'):
		if type(params['clipoutside'])==str:
			params['outside'] = eval(params['clipoutside'])
		else:
			params['outside'] = params['clipoutside']
		del params['clipoutside']
	if params.has_key('clipexpr'):
		# Unpack using underscore, e.g. 'ABS_RR' => 'ABS RR'
		if params['clipexpr'].count('_')==1:
			v = params['clipexpr'].split('_')
			params['clipexpr'] = v[0]+' '+v[1]
		elif params['clipexpr']=='all':
			print " clipexpr='all' not implemented, using ABS RR"
			params['clipexpr'] = 'ABS RR'
	if params.has_key('clipchanavg'):
		if type(params['clipchanavg'])==str:
			params['clipchanavg'] = eval(params['clipchanavg'])
        if params.has_key('autocorr'):
		if type(params['autocorr'])==str:
			params['autocorrelation'] = eval(params['autocorr'])
		else:
			params['autocorrelation'] = params['autocorr']
		del params['autocorr']
        if params.has_key('quackinterval'):
		params['quackinterval'] = float(params['quackinterval'])
        if params.has_key('quackincrement'):
		if type(params['quackincrement'])==str:
			params['quackincrement'] = eval(params['quackincrement'])
        if params.has_key('diameter'):
		params['diameter'] = float(params['diameter'])

def backup_cmdflags(fglocal, mode):

        # Create names like this:
        # before_manualflag_1,
        # before_manualflag_2,
        # before_manualflag_3,
        # etc
        #
        # Generally  before_<mode>_<i>, where i is the smallest
        # integer giving a name, which does not already exist
       
        existing = fglocal.getflagversionlist(printflags=False)

	# remove comments from strings
	existing = [x[0:x.find(' : ')] for x in existing]
	i = 1
	while True:
		versionname = mode +"_" + str(i)

		if not versionname in existing:
			break
		else:
			i = i + 1

        time_string = str(time.strftime('%Y-%m-%d %H:%M:%S'))

        casalog.post("Saving current flags to " + versionname + " before applying new flags")

        fglocal.saveflagversion(versionname=versionname,
                           comment='flagcmd autosave before ' + mode + ' on ' + time_string,
                           merge='replace')

