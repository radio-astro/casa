import os
from taskinit import *

debug = False
def importevla2(asdm=None, vis=None, ocorr_mode=None, compression=None, asis=None, verbose=None, overwrite=None, flagzero=None, cliplevel=None, flagpol=None, shadow=None, diameter=None, applyflags=None, tbuff=None, flagbackup=None):
	""" Convert a Science Data Model (SDM) dataset into a CASA Measurement Set (MS)
	This version is under development and is geared to handling EVLA specific flag and
	system files, and is otherwise identical to importasdm.
	
	Keyword arguments:
	asdm -- Name of input SDM file (directory)
		default: none; example: asdm='ExecBlock3'

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
	try:
                casalog.origin('importevla')
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
		execute_string='asdm2MS  --icm \"' +corr_mode + '\" --isrt \"' + srt+ '\" --its \"' + time_sampling+ '\" --ocm \"' + ocorr_mode + '\" --wvr-corrected-data \"' + wvr_corrected_data + '\" --asis \"' + asis + '\" --logfile \"' +casalog.logfile() +'\"'
		if(showversion) :
		   casalog.post('asdm2MS --revision --logfile \"' +casalog.logfile() +'\"')
		   os.system('asdm2MS --revision --logfile \"' +casalog.logfile() +'\"')
		if(compression) :
		   execute_string= execute_string +' --compression'
		if(verbose) :
		   execute_string= execute_string +' --verbose'
		if not overwrite and os.path.exists(viso) :
		   raise Exception, "You have specified and existing ms and have indicated you do not wish to overwrite it"
		execute_string = execute_string + ' ' + asdm + ' ' + viso
		casalog.post('Running the asdm2MS standalone invoked as:')
		#print execute_string
		casalog.post(execute_string)
        	os.system(execute_string)
		if compression :
                    #viso = viso + '.compressed'
                    viso = visover.rstrip('.ms') + '.compressed.ms'
		if flagbackup:
		    ok=fg.open(viso);
		    ok=fg.saveflagversion('Original',comment='Original flags on import',merge='save')
		    ok=fg.done();
		    print "Backed up original flag column to "+viso+".flagversions"
		    casalog.post("Backed up original flag column to "+viso+".flagversions")
		#
		# =============================
		# Begin EVLA specific code here
		# =============================
		nflags = 0
		myflagd = {}		
		if os.access(asdm+'/Flag.xml',os.F_OK):
		    # Find (and copy) Antenna.xml and Flag.xml
		    if os.access(asdm+'/Antenna.xml',os.F_OK):
		       print "  Found Antenna.xml in SDM, copying to MS"
		       casalog.post("Found Antenna.xml in SDM, copying to MS")
		       os.system('cp -rf '+asdm+'/Antenna.xml '+viso+'/')
	            else:
		       raise Exception, "Failed to find Antenna.xml in SDM"
	            #
		    print "  Found Flag.xml in SDM, copying to MS"
		    casalog.post("Found Flag.xml in SDM, copying to MS")
		    os.system('cp -rf '+asdm+'/Flag.xml '+viso+'/')
		    #
		    # Parse Flag.xml into flag dictionary
		    #
		    myflagd = readflagxml(asdm,tbuff)
		    #
		    keylist = myflagd.keys()
		    nkeys = keylist.__len__()
		    nflags += nkeys
		else:
		    print 'WARNING: No Flag.xml in SDM'
		    casalog.post('WARNING: No Flag.xml in SDM')

		if flagzero or shadow:
		    # Get overall MS time range for later use (if needed)
		    try:
		        # this might take too long for large MS
			ms.open(viso)
			timd = ms.range(["time"])
			ms.close()
		    except:
		        raise Exception, "Error opening MS "+viso 
		    ms_startmjds = timd['time'][0]
		    ms_endmjds = timd['time'][1]
		    t = qa.quantity(ms_startmjds,'s')
		    ms_starttime = qa.time(t,form="ymd")
		    ms_startdate = qa.time(t,form=["ymd","no_time"])
		    t0 = qa.totime(ms_startdate+'/00:00:00.0')
		    t0d = qa.convert(t0,'d')
		    t0s = qa.convert(t0,'s')
		    t = qa.quantity(ms_endmjds,'s')
		    ms_endtime = qa.time(t,form="ymd")
		    # NOTE: could also use values from OBSERVATION table col TIME_RANGE
		    casalog.post('MS spans timerange '+ms_starttime+' to '+ms_endtime)

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
		    nflags += 1
		    myflagd[nflags] = flagz
		    # 
		    flagz['reason'] = 'CLIP_ZERO_LL'
		    flagz['cmd']= "mode='clip' cliprange='0~"+str(cliplevel)+"' clipexpr='ABS_LL'"
		    flagz['id'] = 'ZERO_LL'
		    nflags += 1
		    myflagd[nflags] = flagz
		    nflagz = 2
		    if (flagpol):
		        flagz['reason'] = 'CLIP_ZERO_RL'
			flagz['cmd']= "mode='clip' cliprange='0~"+str(cliplevel)+"' clipexpr='ABS_RL'"
			flagz['id'] = 'ZERO_RL'
			nflags += 1
			myflagd[nflags] = flagz
		        flagz['reason'] = 'CLIP_ZERO_LR'
			flagz['cmd']= "mode='clip' cliprange='0~"+str(cliplevel)+"' clipexpr='ABS_LR'"
			flagz['id'] = 'ZERO_RL'
			nflags += 1
			myflagd[nflags] = flagz
			nflagz += 2
			       
		    print 'Flagging low-amplitude points, using clip level ',cliplevel
		    casalog.post('Flagging low-amplitude points, using clip level '+str(cliplevel))
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
		    nflags += 1
		    myflagd[nflags] = flagh
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
			myflagsd = sortflags(myflagd,myflagsort='antenna')
		        #
			# Now flag the data
			#
			applyflagcmd(viso, False, myflagsd)
		    else:
		        print 'No flagging found in Flag.xml'
			casalog.post('No flagging found in Flag.xml')
		else:
		    print 'Warning: will not be applying flags (applyflags=False)'
		    casalog.post('Will not be applying flags (applyflags=False)')

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
# Flag dictionary manipulation routines
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
#         'mode' (string)         flag mode '','clip','shadow','quack'
#         'antenna' (string)
#         'timerange' (string)
#         'time' (float)          in mjd seconds
#         'interval' (float)      in mjd seconds
#         'cmd' (string)          string (for COMMAND col in FLAG_CMD)
#         'type' (string)         'FLAG' / 'UNFLAG'
#         'applied' (bool)        set to True here on read-in
#         'level' (int)           set to 0 here on read-in
#         'severity' (int)        set to 0 here on read-in
#
    try:
        from xml.dom import minidom
    except ImportError, e:
        print "failed to load xml.dom.minidom:\n", e
        exit(1)

    if type(mytbuff)!=float:
        casalog.post('Warning: incorrect type for tbuff, found "'+str(mytbuff)+'", setting to 1.0')
        mytbuff=1.0
    
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

    # now read Flag.xml into dictionary row by row
    xmlflags = minidom.parse(sdmfile+'/Flag.xml')
    flagdict = {}
    rowlist = xmlflags.getElementsByTagName("row")
    nrows = rowlist.length
    for fid in range(nrows):
        rownode = rowlist[fid]
        rowfid = rownode.getElementsByTagName("flagId")
        fidstr = str(rowfid[0].childNodes[0].nodeValue)
        flagdict[fid] = {}
        flagdict[fid]['id'] = fidstr
        rowid = rownode.getElementsByTagName("antennaId")
        antid = str(rowid[0].childNodes[0].nodeValue)
        antname = antdict[antid]
        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName("startTime")
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjds = (float(start)*1.0E-9) - mytbuff
        t = qa.quantity(startmjds,'s')
        starttime = qa.time(t,form="ymd")
        rowend = rownode.getElementsByTagName("endTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjds = (float(end)*1.0E-9) + mytbuff
        t = qa.quantity(endmjds,'s')
        endtime = qa.time(t,form="ymd")
	# time and interval for FLAG_CMD use
	times = 0.5*(startmjds+endmjds)
	intervs = endmjds-startmjds
        flagdict[fid]['time'] = times
        flagdict[fid]['interval'] = intervs
        # reasons
        rowreason = rownode.getElementsByTagName("reason")
        reas = str(rowreason[0].childNodes[0].nodeValue)
        # Construct antenna name and timerange and reason strings
        flagdict[fid]['antenna'] = antname
        timestr = starttime+'~'+endtime
        flagdict[fid]['timerange'] = timestr
        flagdict[fid]['reason'] = reas
        # Construct command strings (per input flag)
        cmd = "antenna='"+antname+"' timerange='"+timestr+"'"
        flagdict[fid]['cmd'] = cmd
	#
	flagdict[fid]['type'] = 'FLAG'
	flagdict[fid]['applied'] = False
	flagdict[fid]['level'] = 0
	flagdict[fid]['severity'] = 0
	flagdict[fid]['mode'] = ''

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

def sortflags(myflags=None,myantenna='',myreason='',myflagsort=''):
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
    #         'mode' (string)         flag mode '','clip','shadow','quack'
    #         'antenna' (string)
    #         'timerange' (string)
    #         'time' (float)          in mjd seconds
    #         'interval' (float)      in mjd seconds
    #         'cmd' (string)          string (for COMMAND col in FLAG_CMD)
    #         'type' (string)         'FLAG' / 'UNFLAG'
    #         'applied' (bool)        set to True here on read-in
    #         'level' (int)           set to 0 here on read-in
    #         'severity' (int)        set to 0 here on read-in
    #
    # NOTE: flag sorting is needed to avoid error "Too many flagging agents instantiated"
    # If myflagsort=''              keep individual flags separate
    #              ='antenna'       combine all flags with a particular antenna
    #
    #
    # Check if any operation is needed
    if myantenna=='' and myreason=='' and myflagsort=='':
        print 'No selection or sorting needed - sortflags returning input dictionary'
        casalog.post('No selection or sorting needed - sortflags returning input dictionary')
        flagd = myflags
	return flagd
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
	myreaslist = myreason.split(',')
	# Note antenna and reason selection checks for inclusion not exclusivity

	# Now loop over flags, break into sorted and unsorted groups
	if myflagsort=='antenna':
	    # will be resorting by antenna
	    nsortd = 0
	    sortd = {}
	    for key in keylist:
	        myd = myflags[key]
		# can only sort mode manualflag together
		mymode = myd['mode']
		if mymode=='' or mymode=='manualflag':
		    ant = myd['antenna']
		    antlist = ant.split(',')
		    # break this flag by antenna
		    for a in antlist:
		        if myantenna=='' or myantlist.count(a)>0:
			    addf = False
			    reas = myd['reason']
			    reaslist = reas.split(',')
			    reastr = ''
			    for r in reaslist:
			        if myreason=='' or myreaslist.count(r)>0:
			            addf = True
				    # check if this is a new reason
				    if sortd.has_key(a):
				        rlist = reastr.split(',')
					if rlist.count(r)==0:
					    reastr += ','+r
			    #
			    if addf:
			        # check if this is a new antenna
				if sortd.has_key(a):
				    t = sortd[a]['timerange']
				    tlist = t.split(',')
				    timelist = myd['timerange'].split(',')
				    for tim in timelist:
			                # check if this is a new timerange
					if tlist.count(tim)==0:
				            t += ',' + tim
				    sortd[a]['timerange'] = t
			            sortd[a]['reason'] = reastr
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
				    sortd[a]['antenna'] = a
				    sortd[a]['reason'] = reastr
		else:
		    # cannot compress flags from this mode, add to flagd instead
		    # doesn't clash
		    flagd[nflagd] = myd
		    nflagd += 1

	    # Add sorted keys back in to flagd
	    sortdlist = sortd.keys()
	    nsortd = sortdlist.__len__()
	    if nsortd>0:
		print 'Adding '+str(nsortd)+' sorted flags to '+str(nflagd)+' incompressible flags'
		casalog.post('Adding '+str(nsortd)+' sorted flags to '+str(nflagd)+' incompressible flags')
	        sortdlist.sort()
	        for skey in sortdlist:
	            flagd[nflagd] = sortd[skey]
		    nflagd += 1
	else:
	    # no antenna sorting, can keep flags as-is (just select)
	    for key in keylist:
	        myd = myflags[key]
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
		        for r in reaslist:
			    if myreason=='' or myreaslist.count(r)>0:
			        addr = True
				# check if this is a new reason
				rlist = reastr.split(',')
				if rlist.count(r)==0:
				    reastr += ','+r
			if addr:
			    addf = True
			    # check if this is a new antenna
			    alist = antstr.split(',')
			    if alist.count(a)==0:
			        antstr += ','+a
		if addf:
		    flagd[nflagd] = myd
		    flagd[nflagd]['antenna'] = antstr
		    flagd[nflagd]['reason'] = reastr
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

def writeflagcmd(msfile,myflags):
    #
    # Takes input flag dictionary (e.g. from readflagxml) 
    # Save the flag commands to the FLAG_CMD table for msfile
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
	    app_list.append( myflags[key]['applied'] )
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

#===============================================================================
# Apply flag commands using flagger tool
#===============================================================================

def applyflagcmd(msfile, flagbackup, myflags):
        #
	# Takes input flag dictionary myflags (e.g. from readflagxml) 
	# and applies using flagger tool to MS msfile.
	#
	# If flagbackup=True will save copy of flags before flagging.
	#

        fg.done()
        fg.clearflagselection(0)

	try:
	    if ((type(msfile)==str) & (os.path.exists(msfile))):
	        fg.open(msfile)
	    else:
	        raise Exception, 'MS '+msfile+' not found'

	    # ============================
	    # Important boilerplate:
	    # Define lists of valid params by mode
	    kmodes = {}
	    # Keys to ignore
	    iparams = ['reason','flagtime','id','level','severity'] # currently not used
	    # Keys to recognize uparams=Universal sparams=Selection
	    uparams = [] 
	    sparams = ['antenna','timerange','correlation','scan','feed','array','uvrange','spw','field']
	    aparams = uparams + sparams
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
	        fg.setdata()
		cmdlist = []
		param_set = {}
		for key in keylist:
	            cmd = myflags[key]['cmd']
		    if debug: print 'Processing command '+cmd
		    # Parse each command - currently just split by whitespace into key=value strings
		    params = {}
		    param_list = ''
		    # First ignore comment lines starting with "#"
		    if cmd[:1]!='#':
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
			    else:
				mode = 'manualflag'
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
				casalog.post(param_list)
				# Special cases of parameter mapping to flagger tool
				parse_cmdparams(param_i)
				if debug: print param_i
				# Now save by mode
				if param_set.has_key(mode):
				    n = param_set[mode].__len__()
				    name = mode+'_'+str(n)
				    param_set[mode][name] = param_i
				else:
				    name = mode+'_0'
				    param_set[mode]={}
				    param_set[mode][name] = param_i
				cmdlist.append(param_list)
				# Done with this flag command
			    else:
				print ' Warning: ignoring unknown mode '+mode
	        # Were any valid flagging commands set up?
		# Process these for each mode
		modelist = param_set.keys()
		modelist.sort()
		for mode in modelist:
		    nf = param_set[mode].__len__()
		    if nf > 0:
			fg.setdata()
			fg.clearflagselection(0)
			for s in param_set[mode].keys():
			    param_i = param_set[mode][s]
			    if mode=='shadow':
			        if debug: print 'Applying shadow with params: ',param_i
				if param_i.__len__()>0:
				    fg.setshadowflags(**param_i)
			        else:
				    fg.setshadowflags()
			    else:
			        if param_i.__len__()>0:
				    fg.setmanualflags(**param_i)
			        else:
				    fg.setmanualflags()
			if flagbackup:
			    backup_cmdflags('importevla_'+mode)
			fg.run()
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
                fg.done()
                print '*** Error ***', instance
                #raise
        fg.done()

def parse_cmdparams(params):
	# rename some parameters,
	# in order to match the interface of fg.tool
	#
	# validate parameter quackmode

        if params.has_key('quackmode') and \
          not params['quackmode'] in ['beg', 'endb', 'end', 'tail']:
                raise Exception, "Illegal value '%s' of parameter quackmode, must be either 'beg', 'endb', 'end' or 'tail'" % (params['quackmode'])
        
	if params.has_key('antenna'):
		params['baseline']=params['antenna']
		del(params['antenna'])
	if params.has_key('timerange'):
		params['time']=params['timerange']
		del(params['timerange'])
	if params.has_key('cliprange'):
		v = params['cliprange']
		# turn string into [min,max] range
		r = v.split('~')
		rmin = float(r[0])
		rmax = float(r[1])
		params['cliprange'] = [rmin,rmax]
		params['outside']=False
	if params.has_key('clipoutside'):
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
        if params.has_key('autocorr'):
		params['autocorrelation'] = params['autocorr']
		del params['autocorr']
        if params.has_key('quackinterval'):
		params['quackinterval'] = float(params['quackinterval'])
        if params.has_key('diameter'):
		params['diameter'] = float(params['diameter'])

def backup_cmdflags(mode):

        # Create names like this:
        # before_manualflag_1,
        # before_manualflag_2,
        # before_manualflag_3,
        # etc
        #
        # Generally  before_<mode>_<i>, where i is the smallest
        # integer giving a name, which does not already exist
       
        existing = fg.getflagversionlist(printflags=False)

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

        fg.saveflagversion(versionname=versionname,
                           comment='flagcmd autosave before ' + mode + ' on ' + time_string,
                           merge='replace')

