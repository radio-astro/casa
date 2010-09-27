from taskinit import *
import time
import os
import sys

debug = False
def flagcmd(vis=None,mode=None,flagfile=None,command=None,tbuff=None,antenna=None,reason=None,useapplied=None,optype=None,flagsort=None,outfile=None,flagbackup=None,clearall=None):
	#
	# Task flagcmd
	#    Reads flag commands from file or string and applies to MS
	# Created v1.0 STM 2010-08-31 from flagdata
	# Updated v2.0 STM 2010-09-16 add Flag.xml reading
	# Updated v2.1 STM 2010-09-24 add FLAG_CMD use, optype
	#
	try:
		from xml.dom import minidom
	except:
		raise Exception, 'Failed to load xml.dom.minidom into python'

        casalog.origin('flagcmd')

        fg.done()
        fg.clearflagselection(0)
        
        try: 
                if ((type(vis)==str) & (os.path.exists(vis))):
                        fg.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		myflagcmd = {}
		cmdlist = []
                if mode == 'table':
			print 'Reading from FLAG_CMD table'
                        # Read from FLAG_CMD table into command list
			if flagfile=='':
				msfile = vis
			else:
				msfile = flagfile

			# read from FLAG_CMD table in msfile
			myflagd = readflagcmd(msfile)
			
			# construct flag commands
			keylist = myflagd.keys()
			if keylist.__len__()>0:
				for key in keylist:
					applied = myflagd[key]['applied']
					if useapplied or not applied:
						cmd = myflagd[key]['command']
						reas = myflagd[key]['reason']
						cmdstr = cmd + " reason='"+reas+"'"
						cmdlist.append(cmdstr)
				print 'Created '+str(cmdlist.__len__())+' flag commands'
				casalog.post('Created '+str(cmdlist.__len__())+' flag commands')

				# Now reparse back to flagcmd dictionary
				if cmdlist.__len__()>0:
					myflagcmd = getflagcmds(cmdlist) 
			else:
				print 'Warning: no flags in command list'
				casalog.post('Warning: no flags in command list')
                elif mode == 'file':
			print 'Reading from ASCII flag table'
                        # Read ASCII file into command list
			if flagfile=='': 
				flagtable = vis+'/FlagCMD.txt'
			else:
				flagtable = flagfile

			if ((type(flagtable)==str) & (os.path.exists(flagtable))):
				try:
					ff = open(flagtable,'r')
				except:
					raise Exception, "Error opening file "+flagtable
			else:
				raise Exception, 'Flag command file not found - please verify the name'
			#
			# Parse file
			try:
				cmdlist = ff.readlines()
			except:
				raise Exception, "Error reading lines from file "+flagtable
			ff.close()
			print 'Read '+str(cmdlist.__len__())+' lines from file '+flagtable
			casalog.post('Read '+str(cmdlist.__len__())+' lines from file '+flagtable)

			if cmdlist.__len__()>0:
				myflagcmd = getflagcmds(cmdlist)

		elif mode == 'xml':
			print 'Reading from Flag.xml'
                        # Read from Flag.xml (also needs Antenna.xml)
			if flagfile == '':
				flagtable = vis
			else:
				flagtable = flagfile
			# Actually parse table
			myflags = readflags(flagtable,mytbuff=tbuff)

                        # Construct flags per antenna, selecting by reason if desired
			# Do not resort at this time
			if antenna!='' or reason!='':
				myflagcmd = getflags(myflags,myantenna=antenna,myreason=reason,myflagsort='')
			else:
				myflagcmd = myflags

			# Extract flag commands into list
			cmdlist = []
			keylist = myflagcmd.keys()
			for key in keylist:
				cmdlist.append(myflagcmd[key]['cmdstr'])
			
			print 'Created '+str(cmdlist.__len__())+' flag strings from '+flagtable+'/Flag.xml'
			casalog.post('Created '+str(cmdlist.__len__())+' flag strings from '+flagtable+'/Flag.xml')
		else:
			print 'Reading from input list'
			cmdlist = command
			print 'Input '+str(cmdlist.__len__())+' lines from input list'
			casalog.post('Input '+str(cmdlist.__len__())+' lines from input list')

			if cmdlist.__len__()>0:
				myflagcmd = getflagcmds(cmdlist)

		# Now have list of commands - do something with them
		if optype == 'apply':
			# Possibly resort flags by antenna or reason
			if flagsort!='' and flagsort!='id':
				myflagd = getflags(myflagcmd,myantenna='',myreason='',myflagsort=flagsort)
				mycmdlist = []
				keylist = myflagd.keys()
				for key in keylist:
					mycmdlist.append(myflagd[key]['cmdstr'])
			else:
				mycmdlist = cmdlist
			
			# Apply flags to data using flagger
			manualflag_cmdtest(flagbackup, command=mycmdlist)

			# Save flags to file
			if outfile=='':
				if mode=='table' and flagfile=='':
					# These flags came from internal FLAG_CMD, so update their status
					print 'Updating APPLIED status in FLAG_CMD'
					updateflagcmd(vis,mycol='APPLIED',myval=True)
				else:
					# These flags came from outside the MS, so add to FLAG_CMD
					savetoflagcmd(vis,mycmdlist,applied=True)
			else:
				# Save to ascii file
				try:
					ffout = open(outfile,'w')
				except:
					raise Exception, "Error opening output file "+outfile
				try:
					for cmd in mycmdlist:
						print >>ffout,'%s' % cmd
				except:
					raise Exception, "Error writing lines to file "+outfile
				ffout.close()
			
		elif optype=='list':
			keylist = myflagcmd.keys()
			if mode=='xml' and keylist.__len__()>0:
				# List online flag commands (from dictionary)
				listflags(myflagcmd,myantenna='',myreason='',myoutfile=outfile)
			elif cmdlist.__len__()==0:
				print 'Warning: no flags to list'
				casalog.post('Warning: no flags to list')
			elif outfile=='':
				# List flag commands directly to screen and logger
				for cmd in cmdlist:
					pstr = '%s' % cmd
					print pstr
					casalog.post(pstr)
			else:
				# List flag commands directly to file
				try:
					lfout = open(outfile,'w')
				except:
					raise Exception, "Error opening list file "+outfile
				try:
					for cmd in cmdlist:
						print >>lfout,'%s' % cmd
				except:
					raise Exception, "Error listing to file "+outfile
				lfout.close()
		elif optype=='save':
			# Possibly resort flags by antenna or reason
			if flagsort!='' and flagsort!='id':
				myflagd = getflags(myflagcmd,myantenna='',myreason='',myflagsort=flagsort)
				mycmdlist = []
				keylist = myflagd.keys()
				for key in keylist:
					mycmdlist.append(myflagcmd[key]['cmdstr'])
			else:
				mycmdlist = cmdlist
			
			# Save flag commands to file
			if outfile=='':
				if mode=='table' and flagfile=='':
					# Flags came from internal FLAG_CMD, treat as "list"
					for cmd in mycmdlist:
						pstr = '%s' % cmd
						print pstr
						casalog.post(pstr)
				else:
					savetoflagcmd(vis,mycmdlist)
			else:
				# Save to ascii file
				try:
					ffout = open(outfile,'w')
				except:
					raise Exception, "Error opening output file "+outfile
				try:
					for cmd in mycmdlist:
						print >>ffout,'%s' % cmd
				except:
					raise Exception, "Error writing lines to file "+outfile
				ffout.close()
		elif optype=='clear':
			# Clear flag commands from FLAG_CMD in vis
			msfile = vis
			if clearall:
				print 'Warning: deleting all rows from FLAG_CMD in MS '+msfile
				casalog.post('Warning: deleting all rows from FLAG_CMD in MS '+msfile)
				clearflagcmd(msfile,clearmode='all')
			else:
				print 'Warning: partial clear not yet supported'
				casalog.post('Warning: partial clear not yet supported')
				
		elif optype=='plot':
			keylist = myflagcmd.keys()
			if keylist.__len__()>0:
				# Plot flag commands from FLAG_CMD or xml
				print 'Warning: will only reliably plot individual per-antenna flags'
				casalog.post('Warning: will only reliably plot individual per-antenna flags')
				plotflags(myflagcmd,outfile)
			else:
				print 'Warning: empty flagcmd dictionary, nothing to plot'
				casalog.post('Warning: empty flag dictionary, nothing to plot')
				
        except Exception, instance:
                fg.done()
                print '*** Error ***', instance
                #raise
        fg.done()
        
        #write history
        ms.open(vis,nomodify=False)
        ms.writehistory(message='taskname = flagcmd', origin='flagcmd')
        ms.writehistory(message='vis      = "' + str(vis) + '"', origin='flagcmd')
        ms.writehistory(message='mode     = "' + str(mode) + '"', origin='flagcmd')
	if mode == 'file':
		ms.writehistory(message='flagfile = "' + str(flagfile) + '"', origin='flagcmd')
	elif mode=='cmd':
		for cmd in command:
			ms.writehistory(message='command  = "' + str(cmd) + '"', origin='flagcmd')
        ms.close()

        return

#===============================================================================
#
# helper functions to parse flagging commands into fg method calls
#
#===============================================================================
def manualflag_cmdtest(flagbackup, command):
        #
	# Takes input list of command strings
	#
        if debug: print command

	# List of valid params by mode
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
        
	# prepare to input flagging commands
	fg.setdata()
	cmdlist = []
	param_set = {}
        for cmd in command:
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
				# strip quotes from value
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
				parse_cmdparamstest(param_i)
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
				backup_cmdflagstest('flagcmd_'+mode)
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

# rename some parameters,
# in order to match the interface of fg.tool
#
# validate parameter quackmode

def parse_cmdparamstest(params):
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

def backup_cmdflagstest(mode):

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

#===============================================================================
# Flag dictionary manipulation routines
#===============================================================================

def readflags(sdmfile, mytbuff):
#
#   readflags: reads Antenna.xml and Flag.xml SDM tables and parses
#               flag commands into returned dictionary
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
	intervs = startmjds-endmjds
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
        cmdstr = cmd + " reason='"+reas+"'"
        flagdict[fid]['cmd'] = cmd
        flagdict[fid]['cmdstr'] = cmdstr
	#
	flagdict[fid]['type'] = 'FLAG'
	flagdict[fid]['applied'] = False

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

def getflags(myflags=None,myantenna='',myreason='',myflagsort=''):
    #
    # Return dictionary of flagging command strings selected from myflags dictionary
    # (using selection) extracted from Flag.xml online flags SDM table
    #
    # flagcmds key:'id' = flag id string
    #              'antenna' = antenna selections string (if present)
    #              'timerange' = timerange selection string (if present)
    #              'reason' = REASON entry string
    #              'cmd' = command strings (excluding reason) for FLAG_CMD table use
    #              'cmdstr' = full command strings for ASCII use
    #
    # If myflagsort='', then will keep individual flags separate
    # 
    # If myflagsort='antenna', then will combine all flags with a particular antenna
    #
    # If myflagsort='reason', then will combine all flags with a particular reason
    #
    #
    flagcmds = {}

    keylist = myflags.keys()
    print 'Selecting from '+str(keylist.__len__())+' flagging commands'
    if keylist.__len__() > 0:
        #
        # Sort by key
        #
        keylist.sort()
        flagd={}
        #
        # Construct flag command list for selected ant,reason
        #
	#print 'Selecting flags by antenna="'+str(myantenna)+'"'
	casalog.post('Selecting flags by antenna="'+str(myantenna)+'"')
	myantlist = myantenna.split(',')
	#print 'Selecting flags by reason="'+str(myreason)+'"'
	casalog.post('Selecting flags by reason="'+str(myreason)+'"')
	myreaslist = myreason.split(',')
        for key in keylist:
            ant = myflags[key]['antenna']
            if myantenna=='' or myantlist.count(ant)>0:
                reas = myflags[key]['reason']
                if myreason=='' or myreaslist.count(reas)>0:
                    # print '%16s %8s %32s %48s' % (key, ant, reas, tim)
                    timestr = myflags[key]['timerange']
		    if myflagsort=='antenna':
			    if flagd.has_key(ant):
				    flagd[ant]['timerange'] += ','+timestr
				    flagd[ant]['reason'] += ','+reas
			    else:
				    flagd[ant] = {}
				    flagd[ant]['id'] = ant
				    flagd[ant]['antenna'] = ant
				    flagd[ant]['timerange'] = timestr
				    flagd[ant]['reason'] = reas
		    elif myflagsort=='reason':
			    if flagd.has_key(reas):
				    flagd[reas] = {}
				    flagd[reas]['antenna'] += ','+ant
				    flagd[reas]['timerange'] += ','+timestr
			    else:
				    flagd[reas]['id'] = reas
				    flagd[reas]['antenna'] = ant
				    flagd[reas]['timerange'] = timestr
				    flagd[reas]['reason'] = reas
		    else:
			    flagd[key] = {}
			    flagd[key]['id'] = myflags[key]['id']
			    flagd[key]['antenna'] = ant
			    flagd[key]['timerange'] = timestr
			    flagd[key]['reason'] = reas
			    flagd[key]['cmd'] = myflags[key]['cmd']
			    flagd[key]['cmdstr'] = myflags[key]['cmdstr']
			    
        #
        # Form flag commands if needed
	flagkeys = flagd.keys()
	if flagkeys.__len__() > 0:
		if myflagsort=='antenna' or myflagsort=='reason' :
			for key in flagkeys:
				ant = flagd[key]['antenna']
				reas = flagd[key]['reason']
				timestr = flagd[key]['timerange']
				cmd = "antenna='"+ant+"' timerange='"+timestr+"'"
				cmdstr = cmd + " reason='"+reas+"'"
				flagd[key]['cmd'] = cmd
				flagd[key]['cmdstr'] = cmdstr
			print 'Constructed '+str(flagkeys.__len__())+' flagging commands by '+myflagsort
			casalog.post('Constructed '+str(flagkeys.__len__())+' flagging commands by '+myflagsort)
		else:
			print 'Constructed '+str(flagkeys.__len__())+' flagging commands'
			casalog.post('Constructed '+str(flagkeys.__len__())+' flagging commands')
        else:
            print 'No flagging commands found meeting criteria'
            casalog.post('No flagging commands found meeting criteria')
    else:
       print 'No flags found in input dictionary'
       casalog.post('No flags found in input dictionary')

    return flagd
# Done

def getflagcmds(cmdlist):
#
#   Parse list of flag command strings and return dictionary myflagcmds
#   which contains key : 'id','antenna','timerange','reason','cmd','cmdstr','unflag'
#   
#   (this is the inverse of getflags)
#
    myflagd={}
    nrows = cmdlist.__len__()
    if nrows==0:
	    print 'Warning: empty flag command list'
	    return myflagd
    ncmds = 0
    for i in range(nrows):
	    cmdstr = cmdlist[i]
	    # break string into key=val sets
	    keyvlist = cmdstr.split()
	    if keyvlist.__len__()>0:
		    ant = ''
		    tim = ''
		    reas = ''
		    cmd = ''
		    fid = str(i)
		    typ = 'FLAG'
		    for keyv in keyvlist:
			    (xkey,val) = keyv.split('=')
			    xval = val
			    # strip quotes from value
			    if xval.count("'")>0: xval=xval.strip("'")
			    if xval.count('"')>0: xval=xval.strip('"')

			    # Strip reason and id out of string
			    if xkey=='reason':
				    reas = xval
			    elif xkey=='id':
				    fid = xval
			    elif xkey=='unflag':
				    if xval=='True':
					    typ = 'UNFLAG'
			    else:
				    # Extract timerange and antenna (but keep in string)
				    if xkey=='timerange':
					    tim = xval
				    elif xkey=='antenna':
					    ant = xval
				    cmd = cmd+' '+keyv
		    # Done parsing keyvals
		    # Make sure there is a non-blank command string after reason/id extraction
		    if cmd!='':
			    flagd = {}
			    flagd['id'] = fid
			    flagd['antenna']=ant
			    flagd['timerange']=tim
			    flagd['reason']=reas
			    flagd['cmd']=cmd
			    flagd['cmdstr']=cmdstr
			    flagd['type']=typ
			    # Insert into main dictionary
			    myflagd[ncmds] = flagd
			    ncmds+=1
    print 'Parsed '+str(ncmds)+' flag command strings'
    casalog.post('Parsed '+str(ncmds)+' flag command strings')

    return myflagd

# Done

def listflags(myflags=None,myantenna='',myreason='',myoutfile=''):
    #
    # List flags in myflag dictionary
    #
    if myoutfile!='':
	    try:
		    lfout = open(myoutfile,'w')
	    except:
		    raise Exception, "Error opening list output file "+myoutfile
    # Set up any selection
    #print 'Selecting flags by antenna="'+str(myantenna)+'"'
    casalog.post('Selecting flags by antenna="'+str(myantenna)+'"')
    myantlist = myantenna.split(',')
    #print 'Selecting flags by reason="'+str(myreason)+'"'
    casalog.post('Selecting flags by reason="'+str(myreason)+'"')
    myreaslist = myreason.split(',')
    # Loop over flags
    keylist = myflags.keys()
    keylist.sort
    phdr = '%6s %12s %8s %32s %48s' % ('Key', 'FlagID', 'Antenna', 'Reason', 'Timerange')
    if myoutfile!='':
	    # list to output file
	    print >>lfout,phdr
    else:
	    # list to screen and logger
	    print phdr
	    casalog.post(phdr)
    for key in keylist:
        fld = myflags[key]
	if fld.has_key('id'):
		fid = fld['id']
	else:
		fid = 'None'
	if fld.has_key('antenna'):
		ant = fld['antenna']
	else:
		ant = '""'
	if fld.has_key('timerange'):
		tim = fld['timerange']
	else:
		tim = '""'
	if fld.has_key('timerange'):
		reas = fld['reason']
	else:
		reas = '""'
	if myantenna=='' or myantlist.count(ant)>0:
                if myreason=='' or myreaslist.count(reas)>0:
			pstr = '%6i %12s %8s %32s %48s' % (key, fid, ant, reas, tim)
			if myoutfile!='':
				# list to output file
				print >>lfout,pstr
			else:
				# list to screen and logger
				print pstr
				casalog.post(pstr)
    if myoutfile!='':
	    lfout.close()

# Done

def savetoflagcmd(msfile,cmdlist,applied=False):
    # Save the flag commands to the FLAG_CMD table for msfile
    #
    try:
        import pylab as pl
    except ImportError, e:
        print "failed to load pylab:\n", e
        exit(1)
    # Get overall MS time range for later use (if needed)
    try:
	    # this might take too long for large MS
	    ms.open(msfile)
	    timd = ms.range(["time"])
	    ms.close()
    except:
	    raise Exception, "Error opening MS "+msfile 
					
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
    print 'MS spans timerange '+ms_starttime+' to '+ms_endtime
    casalog.post('MS spans timerange '+ms_starttime+' to '+ms_endtime)
    #
    # parse command string into flag dictionary
    flagd = getflagcmds(cmdlist)
    keylist = flagd.keys()
    nadd = keylist.__len__()
    if nadd>0:
	    # make arrays columns to add
	    tim_list = []
	    intv_list = []
	    cmd_list = []
	    reas_list = []
	    sev_list = []
	    lev_list = []
	    app_list = []
	    typ_list = []
	    for key in keylist:
		    fid = flagd[key]['id']
		    timr = flagd[key]['timerange']
		    reas = flagd[key]['reason']
		    cmdstr = flagd[key]['cmd']
		    typ = flagd[key]['type']
		    #
		    if timr=='':
			    # use start/end from MS
			    startmjds = ms_startmjds
			    endmjds = ms_endmjds
		    else:
			    mintime = ms_endmjds
			    maxtime = ms_startmjds
			    # first break time string by commas
			    timelist = timr.split(',')
			    for xtim in timelist:
				    # then by range operator
				    timestr = xtim.split('~')
				    startstr = timestr[0]
				    t = qa.totime(startstr)
				    startd = qa.convert(t,'d')
				    if startd['value']<5000.0:
					    # assume a time offset from ref
					    startd = qa.add(t0d,startd)
				    starts = qa.convert(startd,'s')
				    startmjds = starts['value']
				    if timestr.__len__()>1:
					    endstr = timestr[1]
				    else:
					    endstr = startstr
				    t = qa.totime(endstr)
				    endd = qa.convert(t,'d')
				    if endd['value']<5000.0:
					    # assume a time offset from ref
					    endd = qa.add(t0d,endd)
				    ends = qa.convert(endd,'s')
				    endmjds = ends['value']
				    if startmjds<mintime: mintime=startmjds
				    if endmjds>maxtime: maxtime=endmjds
		    tim = 0.5*(startmjds+endmjds)
		    interval = endmjds-startmjds
		    #
		    # append to lists
		    tim_list.append(tim)
		    intv_list.append(interval)
		    cmd_list.append(cmdstr)
		    reas_list.append(reas)
		    typ_list.append(typ)
		    # other FLAG_CMD tags
		    sev_list.append(0)
		    lev_list.append(0)
		    app_list.append(applied)
	    # Append new commands to existing table
	    # Save to FLAG_CMD table, not file
	    print 'Saving flags to FLAG_CMD MS table'
	    casalog.post('Saving flags to FLAG_CMD MS table')
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
	    #tim_arr = tb.getcol('TIME',startrow=nrows,nrow=nadd)
	    #tim_arr = pl.array(tim_list)
	    tb.putcol('TIME',pl.array(tim_list),startrow=nrows,nrow=nadd)
	    tb.putcol('INTERVAL',pl.array(intv_list),startrow=nrows,nrow=nadd)
	    tb.putcol('REASON',pl.array(reas_list),startrow=nrows,nrow=nadd)
	    tb.putcol('COMMAND',pl.array(cmd_list),startrow=nrows,nrow=nadd)
	    # Other columns
	    sev_arr = tb.getcol('SEVERITY',startrow=nrows,nrow=nadd)
	    lev_arr = tb.getcol('LEVEL',startrow=nrows,nrow=nadd)
	    appl_arr = tb.getcol('APPLIED',startrow=nrows,nrow=nadd)
	    tb.close()
	    #
	    print 'Wrote '+str(nadd)+' rows to FLAG_CMD'
	    casalog.post('Wrote '+str(nadd)+' rows to FLAG_CMD')
    else:
	    print 'Added zero rows to FLAG_CMD, no flags found'
	    casalog.post('Added zero rows to FLAG_CMD, no flags found')
# Done

def readflagcmd(msfile):
    #
    # Read flag commands from the FLAG_CMD table of msfile and return
    # flagcmd structure:
    #
    # flagcmd key: per row number of FLAG_CMD (key)
    #              'time'        double
    #              'interval'    double
    #              'type'        string FLAG or UNFLAG
    #              'reason'      string
    #              'level'       int
    #              'severity'    int
    #              'applied'     bool
    #              'cmd'         string
    #
    #
    # Open and read columns from FLAG_CMD
    mstable = msfile+'/FLAG_CMD'
    try:
	    tb.open(mstable)
	    f_time = tb.getcol("TIME")
	    f_interval = tb.getcol("INTERVAL")
	    f_type = tb.getcol("TYPE")
	    f_reas = tb.getcol("REASON")
	    f_level = tb.getcol("LEVEL")
	    f_severity = tb.getcol("SEVERITY")
	    f_applied = tb.getcol("APPLIED")
	    f_cmd = tb.getcol("COMMAND")
	    tb.close()
    except:
	    raise Exception, "Error reading table "+mstable
    nrows = f_time.__len__()
    myflagcmd = {}
    if nrows>0:
	    for i in range(nrows):
		    flagd = {}
		    flagd['time'] = f_time[i]
		    flagd['interval'] = f_interval[i]
		    flagd['type'] = f_type[i]
		    flagd['reason'] = f_reas[i]
		    flagd['level'] = f_level[i]
		    flagd['severity'] = f_severity[i]
		    flagd['applied'] = f_applied[i]
		    flagd['command'] = f_cmd[i]
		    # Construct command string
		    # cmdstr = f_cmd[i] + " reason='"+f_reas[i]+"'"
		    # flagd['cmdstr'] = cmdstr
		    #
		    myflagcmd[i] = flagd
	    print 'Read '+str(nrows)+' from FLAG_CMD table in MS'
	    casalog.post('Read '+str(nrows)+' from FLAG_CMD table in MS')
    else:
	    print 'Warning: FLAG_CMD table is empty, no flags extracted'
	    casalog.post('Warning: FLAG_CMD table is empty, no flags extracted')

    # centertime = f_time[i]
    # interval = f_interval[i]
    # startmjds = centertime - 0.5*interval
    # t = qa.quantity(startmjds,'s')
    # starttime = qa.time(t,form="ymd")
    # endmjds = centertime + 0.5*interval
    # t = qa.quantity(endmjds,'s')
    # endtime = qa.time(t,form="ymd")
    # timestr = starttime+'~'+endtime

    return myflagcmd
    
# Done

def clearflagcmd(msfile,clearmode=''):
    #
    # Clearflag commands from the FLAG_CMD table of msfile
    # Open and read columns from FLAG_CMD
    mstable = msfile+'/FLAG_CMD'
    try:
	    tb.open(mstable,nomodify=False)
    except:
	    raise Exception, "Error opening table "+mstable
    nrows = int(tb.nrows())
    print 'There were '+str(nrows)+' rows in FLAG_CMD'
    casalog.post('There were '+str(nrows)+' rows in FLAG_CMD')
    if nrows>0:
	    if clearmode=='' or clearmode=='all':
		    rowlist = range(nrows)
		    tb.removerows(rowlist)
	    print 'Deleted '+str(nrows)+' from FLAG_CMD table in MS'
	    casalog.post('Deleted '+str(nrows)+' from FLAG_CMD table in MS')

	    nnew = int(tb.nrows())
	    print 'There are now '+str(nnew)+' rows in FLAG_CMD'
	    casalog.post('There are '+str(nnew)+' rows in FLAG_CMD')
    else:
	    print 'No rows to clear'
	    casalog.post('No rows to clear')
    tb.close()
    
def updateflagcmd(msfile,mycol='',myval=None):
    #
    # Update commands in the FLAG_CMD table of msfile
    # Open and read columns from FLAG_CMD
    mstable = msfile+'/FLAG_CMD'
    try:
	    tb.open(mstable,nomodify=False)
    except:
	    raise Exception, "Error opening table "+mstable
    nrows = int(tb.nrows())
    # print 'There were '+str(nrows)+' rows in FLAG_CMD'
    # casalog.post('There were '+str(nrows)+' rows in FLAG_CMD')
    if nrows>0 and mycol!='':
	    rowlist = range(nrows)
	    try:
		    tb.putcell(mycol,rowlist,myval)
	    except:
		    raise Exception, "Error updating FLAG_CMD column "+mycol+" to value "+str(myval)
	    
	    print 'Updated '+str(nrows)+' of FLAG_CMD table in MS'
	    casalog.post('Updated '+str(nrows)+' of FLAG_CMD table in MS')
    tb.close()
    
# Done

#===============================================================================
#
# helper functions to plot flag results, courteys J. Marvil
# currently only works from individual online flags from xml or FLAG_CMD
#
#===============================================================================
def plotflags(myflags, plotname):
    try: 
        import casac 
    except ImportError, e: 
        print "failed to load casa:\n", e 
        exit(1) 
    qatool = casac.homefinder.find_home_by_name('quantaHome') 
    qa = casac.qa = qatool.create()
    
    try:
        import pylab as pl
    except ImportError, e:
        print "failed to load pylab:\n", e
        exit(1)

    # get list of flag keys
    keylist=myflags.keys()

    # get list of antennas
    myants=[]
    for key in keylist:
	    ant = myflags[key]['antenna']
	    if myants.count(ant)==0:
		    myants.append(ant)
    myants.sort()
		    
    antind=0
    if plotname=='':
	    pl.ion()
    else:
	    pl.ioff()

    f1=pl.figure()
    ax1=f1.add_axes([.15,.1,.75,.85])
#    ax1.set_ylabel('antenna')
#    ax1.set_xlabel('time')
    badflags=[]
    for thisant in myants:
        antind+=1
	for thisflag in myflags:
	    if myflags[thisflag]['antenna']==thisant:
	        #print thisant, myflags[thisflag]['reason'], myflags[thisflag]['timerange']
                thisReason=myflags[thisflag]['reason']
		if thisReason=='FOCUS_ERROR': thisColor='red'; thisOffset=.3
		elif thisReason=='SUBREFLECTOR_ERROR': thisColor='blue'; thisOffset=.15
		elif thisReason=='ANTENNA_NOT_ON_SOURCE': thisColor='green'; thisOffset=0
		elif thisReason=='ANTENNA_NOT_IN_SUBARRAY': thisColor='black'; thisOffset=-.15
		mytimerange=myflags[thisflag]['timerange']
		t1=mytimerange[:mytimerange.find('~')]
		t2=mytimerange[mytimerange.find('~')+1:]
		t1s, t2s=qa.convert(t1,'s')['value'], qa.convert(t2,'s')['value']
                myTimeSpan=t2s-t1s
		if myTimeSpan < 10000: ax1.plot([t1s,t2s],[antind+thisOffset,antind+thisOffset], color=thisColor, lw=2, alpha=.7)
		else: badflags.append((thisant, myTimeSpan, thisReason))

    ##badflags are ones which span a time longer than that used above
    ##they can be so long that including them compresses the time axis so that none of the other flags are visible    
#    print 'badflags', badflags
    myXlim=ax1.get_xlim()
    myXrange=myXlim[1]-myXlim[0]
    legendFontSize=12
    ax1.text(myXlim[0]+.05*myXrange, 29, 'FOCUS', color='red', size=legendFontSize)
    ax1.text(myXlim[0]+.2*myXrange, 29, 'SUBREFLECTOR', color='blue', size=legendFontSize)
    ax1.text(myXlim[0]+.45*myXrange, 29, 'OFF SOURCE', color='green', size=legendFontSize)
    ax1.text(myXlim[0]+.7*myXrange, 29, 'NOT IN SUBARRAY', color='black', size=legendFontSize)
    ax1.set_ylim([0,30])   

    ax1.set_yticks(range(1,len(myants)+1))
    ax1.set_yticklabels(myants)
    ax1.set_xticks(pl.linspace(myXlim[0],myXlim[1],3))

    mytime=[myXlim[0],(myXlim[1]+myXlim[0])/2.0,myXlim[1]]
    myTimestr = []
    for time in mytime:
        q1=qa.quantity(time,'s')
        time1=qa.time(q1,form='ymd')
        myTimestr.append(time1)

    ax1.set_xticklabels([myTimestr[0],myTimestr[1][11:], myTimestr[2][11:]])
    #print myTimestr
    if plotname=='':
	    pl.draw()
    else:
	    pl.savefig(plotname, dpi=150)    
    return

