from taskinit import *
import time
import os
import sys

debug = False
def flagcmd(vis=None,flagmode=None,flagfile=None,flagrows=None,command=None,tbuff=None,antenna=None,reason=None,useapplied=None,optype=None,flagsort=None,outfile=None,flagbackup=None,clearall=None,rowlist=None,setcol=None,setval=None):
	#
	# Task flagcmd
	#    Reads flag commands from file or string and applies to MS
	# Created v1.0 STM 2010-08-31 from flagdata
	# Updated v2.0 STM 2010-09-16 add Flag.xml reading
	# Updated v2.1 STM 2010-09-24 add FLAG_CMD use, optype
	# Updated v2.2 STM 2010-10-14 compatible with new importevla
	# Updated v2.3 STM 2010-10-22 improvements
	#
	try:
		from xml.dom import minidom
	except:
		raise Exception, 'Failed to load xml.dom.minidom into python'

        casalog.origin('flagcmd')

        fg.done()
        fg.clearflagselection(0)
        
        try: 
                if not os.path.exists(vis):
                        raise Exception, 'Visibility data set not found - please verify the name'

		myflagcmd = {}
		cmdlist = []
                if optype=='clear' or optype=='set':
			print 'optype "clear" or "set" will disregard flagmode (no reading)'
			casalog.post('optype "clear" or "set" will disregard flagmode (no reading)')
                elif flagmode == 'table':
			print 'Reading from FLAG_CMD table'
                        # Read from FLAG_CMD table into command list
			if flagfile=='':
				msfile = vis
			else:
				msfile = flagfile

			# read from FLAG_CMD table in msfile
			myflagcmd = readflagcmd(msfile,myflagrows=flagrows,useapplied=useapplied,myreason=reason)
			listmode = 'cmd'
			
                elif flagmode == 'file':
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
				myflagcmd = getflagcmds(cmdlist,ms_startmjds,ms_endmjds)

			listmode = ''

		elif flagmode == 'xml':
			print 'Reading from Flag.xml'
                        # Read from Flag.xml (also needs Antenna.xml)
			if flagfile == '':
				flagtable = vis
			else:
				flagtable = flagfile
			# Actually parse table
			myflags = readflagxml(flagtable,mytbuff=tbuff)

                        # Construct flags per antenna, selecting by reason if desired
			# Do not resort at this time
			if antenna!='' or reason!='Any':
				myflagcmd = sortflags(myflags,myantenna=antenna,myreason=reason,myflagsort='')
			else:
				myflagcmd = myflags

			listmode = 'online'

		else:
			# command strings
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

			print 'Reading from input list'
			cmdlist = command
			print 'Input '+str(cmdlist.__len__())+' lines from input list'
			casalog.post('Input '+str(cmdlist.__len__())+' lines from input list')

			if cmdlist.__len__()>0:
				myflagcmd = getflagcmds(cmdlist,ms_startmjds,ms_endmjds)

			listmode = ''

		# Turn into command string list (add reason back in)
		mycmdlist = []
		keylist = myflagcmd.keys()
		if keylist.__len__()>0:
			for key in keylist:
				cmdstr = myflagcmd[key]['cmd']
				if myflagcmd[key]['reason']!='':
					cmdstr += " reason='"+myflagcmd[key]['reason']+"'"
				mycmdlist.append(cmdstr)
		casalog.post('Extracted '+str(mycmdlist.__len__())+' flag command strings')
			
		# Now have list of commands - do something with them
		if optype == 'apply':
			# Possibly resort flags before application
			# NEEDED TO AVOID FLAG AGENT ERROR
			if flagsort!='' and flagsort!='id':
				myflagd = sortflags(myflagcmd,myantenna='',myreason='Any',myflagsort=flagsort)
			else:
				myflagd = myflagcmd

			# Apply flags to data using flagger
			nappl = applyflagcmd(vis, flagbackup, myflagd)
			# Save flags to file
			if nappl>0:
			    if outfile=='':
			        if flagmode=='table' and flagfile=='':
			            # These flags came from internal FLAG_CMD, update status
				    print 'Updating APPLIED status in FLAG_CMD'
				    myrowlist = myflagcmd.keys()
				    if myrowlist.__len__()>0:
				        updateflagcmd(vis,mycol='APPLIED',myval=True,myrowlist=myrowlist)
				else:
				    # These flags came from outside the MS, so add to FLAG_CMD
				    # Tag APPLIED=True
				    # NOTE: original flags, not using resorted version
				    writeflagcmd(vis,myflagcmd,tag='applied')
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
			else:
			    print 'WARNING: No flags were applied from list'
			    casalog.post('WARNING: No flags were applied from list','WARN')
		elif optype=='list':
			# List flags
			listflags(myflagcmd,myoutfile=outfile,listmode=listmode)
			
		elif optype=='save':
			# Possibly resort and/or select flags by antenna or reason
			if flagsort!='' and flagsort!='id':
				myflagd = getflags(myflagcmd,myantenna='',myreason='',myflagsort=flagsort)
				mycmdl = []
				keylist = myflagd.keys()
				if keylist.__len__()>0:
					for key in keylist:
						cmdstr = myflagd[key]['cmd']
						if myflagd[key]['reason']!='':
							cmdstr += ' reason="'+myflagd[key]['reason']+'"'
						mycmdl.append(cmdstr)
			else:
				myflagd = myflagcmd
				mycmdl = mycmdlist
			
			# Save flag commands to file
			if outfile=='':
				# Save to FLAG_CMD
				if flagmode=='table' and flagfile=='':
					# Flags came from internal FLAG_CMD, treat as "list" to screen
					for cmd in mycmdl:
						pstr = '%s' % cmd
						print pstr
						casalog.post(pstr)
				else:
					writeflagcmd(vis,myflagd)
			else:
				# Save to ascii file
				try:
					ffout = open(outfile,'w')
				except:
					raise Exception, "Error opening output file "+outfile
				try:
					for cmd in mycmdl:
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
				clearflagcmd(msfile,myrowlist=rowlist)
			else:
				print 'Safety Mode: you chose not to set clearall=True, no action'
				casalog.post('Safety Mode: you chose not to set clearall=True, no action')
				
		elif optype=='set':
			# Set FLAG_CMD cells in vis
			msfile = vis
			if setcol!='':
				updateflagcmd(vis,mycol=setcol,myval=setval,myrowlist=rowlist)
			else:
				print 'Safety Mode: no cell chosen, no action'
				casalog.post('Safety Mode: no cell chosen, no action')
				
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
                #fg.done()
                print '*** Error ***', instance
                #raise
				
        #write history
        ms.open(vis,nomodify=False)
        ms.writehistory(message='taskname = flagcmd', origin='flagcmd')
        ms.writehistory(message='vis      = "' + str(vis) + '"', origin='flagcmd')
        ms.writehistory(message='flagmode = "' + str(flagmode) + '"', origin='flagcmd')
	if flagmode == 'file':
		ms.writehistory(message='flagfile = "' + str(flagfile) + '"', origin='flagcmd')
	elif flagmode=='cmd':
		for cmd in command:
			ms.writehistory(message='command  = "' + str(cmd) + '"', origin='flagcmd')
        ms.close()

        return

#===============================================================================
#
# helper functions to parse flagging commands into fg method calls
#
#===============================================================================

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
	# Returns number of flags applied
	#
	ncmd = 0

        fg.done()
        fg.clearflagselection(0)

	try:
	    if ((type(msfile)==str) & (os.path.exists(msfile))):
	        fg.open(msfile)
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

	return ncmd

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
#         'reason' (string)
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
    #         'mode' (string)         flag mode '','clip','shadow','quack'
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
			        # check if this is a new antenna
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

def getflagcmds(cmdlist, ms_startmjds, ms_endmjds):
#
#   Parse list of flag command strings and return dictionary of flagcmds
#   Inputs:
#      cmdlist (list,string) list of command strings (default for TIME,INTERVAL)
#      ms_startmjds (float)  starting mjd (sec) of MS (default for TIME,INTERVAL)
#      ms_endmjds (float)    ending mjd (sec) of MS
#
#   Usage: myflagcmd = getflags(cmdlist)
#
#   Dictionary structure:
#   fid : 'id' (string)
#         'mode' (string)         flag mode '','clip','shadow','quack'
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
#   
#
    myflagd={}
    nrows = cmdlist.__len__()
    if nrows==0:
	    print 'WARNING: empty flag command list'
	    casalog.post('WARNING: empty flag command list', 'WARN')
	    return myflagd

    t = qa.quantity(ms_startmjds,'s')
    ms_startdate = qa.time(t,form=["ymd","no_time"])
    t0 = qa.totime(ms_startdate+'/00:00:00.0')
    #t0d = qa.convert(t0,'d')
    t0s = qa.convert(t0,'s')

    ncmds = 0
    for i in range(nrows):
	    cmdstr = cmdlist[i]
	    # break string into key=val sets
	    keyvlist = cmdstr.split()
	    if keyvlist.__len__()>0:
		    ant = ''
		    timstr = ''
		    tim = 0.5*(ms_startmjds+ms_endmjds)
		    intvl = ms_endmjds - ms_startmjds
		    reas = ''
		    cmd = ''
		    fid = str(i)
		    mode = ''
		    typ = 'FLAG'
		    appl = False
		    levl = 0
		    sevr = 0
		    fmode = ''
		    for keyv in keyvlist:
			    (xkey,val) = keyv.split('=')
			    xval = val
			    # strip quotes from value
			    if xval.count("'")>0: xval=xval.strip("'")
			    if xval.count('"')>0: xval=xval.strip('"')

			    # Strip these out of command string
			    if xkey=='reason':
				    reas = xval
			    elif xkey=='applied':
				    appl = False
				    if xval=='True':
					    appl = True
			    elif xkey=='level':
				    levl = int(xval)
			    elif xkey=='severity':
				    sevr = int(xval)
			    elif xkey=='time':
				    tim = xval
			    elif xkey=='interval':
				    intv = xval
			    else:
				    # Extract (but keep in string)
				    if xkey=='timerange':
					    timstr = xval
					    # Extract TIME,INTERVAL
					    (startstr,endstr) = timstr.split('~')
					    t = qa.totime(startstr)
					    starts = qa.convert(t,'s')
					    if starts['value']<1.E6:
						    # assume a time offset from ref
						    starts = qa.add(t0s,starts)
					    startmjds = starts['value']
					    if endstr=='':
					       endstr = startstr
				            t = qa.totime(endstr)
					    ends = qa.convert(t,'s')
					    if ends['value']<1.E6:
						    # assume a time offset from ref
						    ends = qa.add(t0s,ends)
					    endmjds = ends['value']
					    tim = 0.5*(startmjds+endmjds)
					    intvl = endmjds - startmjds
					    
				    elif xkey=='antenna':
					    ant = xval
				    elif xkey=='id':
					    fid = xval
				    elif xkey=='unflag':
					    if xval=='True':
						    typ = 'UNFLAG'
				    elif xkey=='mode':
					    fmode = xval
				    cmd = cmd+' '+keyv
		    # Done parsing keyvals
		    # Make sure there is a non-blank command string after reason/id extraction
		    if cmd!='':
			    flagd = {}
			    flagd['id'] = fid
			    flagd['mode'] = fmode
			    flagd['antenna']=ant
			    flagd['timerange']=timstr
			    flagd['reason']=reas
			    flagd['cmd']=cmd
			    flagd['time']=tim
			    flagd['interval']=interval
			    flagd['type']=typ
			    flagd['level']=levl
			    flagd['severity']=sevr
			    flagd['applied']=appl
			    # Insert into main dictionary
			    myflagd[ncmds] = flagd
			    ncmds+=1
    print 'Parsed '+str(ncmds)+' flag command strings'
    casalog.post('Parsed '+str(ncmds)+' flag command strings')

    return myflagd

# Done

def listflags(myflags=None,myantenna='',myreason='',myoutfile='',listmode=''):
    #
    # List flags in myflags dictionary
    #
    # Format according to listmode:
    #     =''          Format for flag command strings
    #     ='cmd'       Format for FLAG_CMD flags
    #     ='online'    Format for online flags
    #
    #   Dictionary structure:
    #   fid : 'id' (string)
    #         'mode' (string)         flag mode '','clip','shadow','quack'
    #         'antenna' (string)
    #         'timerange' (string)
    #         'reason' (string)
    #         'time' (float)          in mjd seconds
    #         'interval' (float)      in mjd seconds
    #         'cmd' (string)          string (for COMMAND col in FLAG_CMD)
    #         'type' (string)         'FLAG' / 'UNFLAG'
    #         'applied' (bool)        
    #         'level' (int)           
    #         'severity' (int)        
    #
    useid = False
    doterm = False
    #
    if myoutfile!='':
	    try:
		    lfout = open(myoutfile,'w')
	    except:
		    raise Exception, "Error opening list output file "+myoutfile

    keylist = myflags.keys()
    if keylist.__len__()==0:
	    print 'WARNING: no flags to list'
	    casalog.post('WARNING: no flags to list', 'WARN')
	    return
    # Sort keys
    keylist.sort

    # Set up any selection
    #print 'Selecting flags by antenna="'+str(myantenna)+'"'
    casalog.post('Selecting flags by antenna="'+str(myantenna)+'"')
    myantlist = myantenna.split(',')
    #print 'Selecting flags by reason="'+str(myreason)+'"'
    casalog.post('Selecting flags by reason="'+str(myreason)+'"')
    myreaslist = myreason.split(',')

    if listmode=='online':
	    phdr = '%8s %12s %8s %32s %48s' % ('Key', 'FlagID', 'Antenna', 'Reason', 'Timerange')
    elif listmode=='cmd':
	    phdr = '%8s %45s %32s %6s %7s %3s %3s %s' % ('Row', 'Timerange', 'Reason', 'Type', 'Applied', 'Lev', 'Sev', 'Command')
    else:
	    phdr = '%8s %s' % ('Key', 'Command')
    if myoutfile!='':
	    # list to output file
	    print >>lfout,phdr
    else:
	    # list to logger and screen
	    if doterm:
		    print phdr
	    else:
		    print 'Output will be in logger'
	    casalog.post(phdr)
    # Loop over flags
    for key in keylist:
        fld = myflags[key]
	# Get fields
	skey = str(key)
	if fld.has_key('id') and useid:
		fid = fld['id']
	else:
		fid = str(key)
	if fld.has_key('antenna'):
		ant = fld['antenna']
	else:
		ant = 'Unset'
	if fld.has_key('timerange'):
		timr = fld['timerange']
	else:
		timr = 'Unset'
	if fld.has_key('reason'):
		reas = fld['reason']
	else:
		reas = 'Unset'
	if fld.has_key('cmd'):
		cmd = fld['cmd']
	else:
		cmd = 'Unset'
	if fld.has_key('type'):
		typ = fld['type']
	else:
		typ = 'FLAG'
	if fld.has_key('level'):
		levl = str( fld['level'] )
	else:
		levl = '0'
	if fld.has_key('severity'):
		sevr = str( fld['severity'] )
	else:
		sevr = '0'
	if fld.has_key('applied'):
		appl = str( fld['applied'] )
	else:
		appl = 'Unset'
	# Print out listing
	if myantenna=='' or myantlist.count(ant)>0:
                if myreason=='' or myreaslist.count(reas)>0:
			if listmode=='online':
				pstr = '%8s %12s %8s %32s %48s' % (skey, fid, ant, reas, timr)
			elif listmode=='cmd':
				pstr = '%8s %45s %32s %6s %7s %3s %3s %s' % (skey, timr, reas, typ, appl, levl, sevr, cmd)
			else:
				pstr = '%8s %s' % (skey, cmd)
			if myoutfile!='':
				# list to output file
				print >>lfout,pstr
			else:
				# list to logger and screen
				if doterm: print pstr
				casalog.post(pstr)
    if myoutfile!='':
	    lfout.close()

# Done

def readflagcmd(msfile,myflagrows=[],useapplied=True,myreason='Any'):
    #
    # Read flag commands from flagrows of the FLAG_CMD table of msfile
    # If useapplied=False then include only rows with APPLIED=False
    # If myreason is anything other than 'Any', then select on that
    #
    # Return flagcmd structure:
    #
    # The flagcmd key is the integer row number from FLAG_CMD
    #
    #   Dictionary structure:
    #   key : 'id' (string)
    #         'mode' (string)         flag mode '','clip','shadow','quack'
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
    #
    #
    # Open and read columns from FLAG_CMD
    mstable = msfile+'/FLAG_CMD'

    # Note, tb.getcol doesn't allow random row access, read all
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

    myreaslist = []
    # Parse myreason
    if type(myreason)==str:
	    if myreason!='Any':
		    myreaslist.append(myreason)
    elif type(myreason)==list:
	    myreaslist=myreason
    else:
	    print 'ERROR: reason contains unknow variable types'
	    casalog.post('ERROR: reason contains unknow variable types','SEVERE')
	    return
    
    myflagcmd = {}
    if nrows>0:
	    nflagd = 0
	    if myflagrows.__len__()>0:
		    rowlist = myflagrows
	    else:
		    rowlist = range(nrows)
	    # Prune rows if needed
	    if not useapplied:
		    rowl = []
		    for i in rowlist:
			    if not f_applied[i]: rowl.append(i)
		    rowlist = rowl
	    if myreaslist.__len__()>0:
		    rowl = []
		    for i in rowlist:
			    if myreaslist.count(f_reas[i])>0: rowl.append(i)
		    rowlist = rowl
		    
	    for i in rowlist:
		    flagd = {}
		    flagd['id'] = str(i)
		    flagd['antenna'] = ''
		    flagd['mode'] = ''
		    flagd['time'] = f_time[i]
		    flagd['interval'] = f_interval[i]
		    flagd['type'] = f_type[i]
		    flagd['reason'] = f_reas[i]
		    flagd['level'] = f_level[i]
		    flagd['severity'] = f_severity[i]
		    flagd['applied'] = f_applied[i]
		    cmd = f_cmd[i]
		    flagd['cmd'] = cmd
		    # Extract antenna and timerange strings from cmd
		    antstr = ''
		    timstr = ''
		    keyvlist = cmd.split()
		    if keyvlist.__len__()>0:
		        for keyv in keyvlist:
			    (xkey,val) = keyv.split('=')
			    xval = val
			    # strip quotes from value
			    if xval.count("'")>0: xval=xval.strip("'")
			    if xval.count('"')>0: xval=xval.strip('"')
			    if xkey=='timerange':
			        timstr = xval
			    elif xkey=='antenna':
			        flagd['antenna'] = xval
			    elif xkey=='id':
			        flagd['id'] = xval
			    elif xkey=='mode':
			        flagd['mode'] = xval
		    if timstr=='':
			    # Construct timerange from time,interval
			    centertime = f_time[i]
			    interval = f_interval[i]
			    startmjds = centertime - 0.5*interval
			    t = qa.quantity(startmjds,'s')
			    starttime = qa.time(t,form="ymd")
			    endmjds = centertime + 0.5*interval
			    t = qa.quantity(endmjds,'s')
			    endtime = qa.time(t,form="ymd")
			    timstr = starttime+'~'+endtime
		    flagd['timerange'] = timstr
		    # Keep original key index, might need this later
		    myflagcmd[i] = flagd
		    nflagd += 1
	    print 'Read '+str(nflagd)+' rows from FLAG_CMD table in MS'
	    casalog.post('Read '+str(nflagd)+' rows from FLAG_CMD table in MS')
    else:
	    print 'WARNING: FLAG_CMD table is empty, no flags extracted'
	    casalog.post('WARNING: FLAG_CMD table is empty, no flags extracted','WARN')

    return myflagcmd
    
# Done

def clearflagcmd(msfile,myrowlist=[]):
    #
    # Delete flag commands (rows) from the FLAG_CMD table of msfile
    #
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
	    if myrowlist.__len__()>0:
		    rowlist = myrowlist
	    else:
		    rowlist = range(nrows)
	    try:
		    tb.removerows(rowlist)
		    print 'Deleted '+str(rowlist.__len__())+' from FLAG_CMD table in MS'
		    casalog.post('Deleted '+str(rowlist.__len__())+' from FLAG_CMD table in MS')
	    except:
		    tb.close()
		    raise Exception, "Error removing rows "+str(rowlist)+" from table "+mstable

	    nnew = int(tb.nrows())
	    print 'There are now '+str(nnew)+' rows in FLAG_CMD'
	    casalog.post('There are '+str(nnew)+' rows in FLAG_CMD')
    else:
	    print 'No rows to clear'
	    casalog.post('No rows to clear')
    tb.close()
    
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
	    casalog.post('Will update column '+mycol+' for rows '+str(rowlist))
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
		else: thisColor='orange'; thisOffset=0.3
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
    ax1.text(myXlim[0]+.17*myXrange, 29, 'SUBREFLECTOR', color='blue', size=legendFontSize)
    ax1.text(myXlim[0]+.42*myXrange, 29, 'OFF SOURCE', color='green', size=legendFontSize)
    ax1.text(myXlim[0]+.62*myXrange, 29, 'NOT IN SUBARRAY', color='black', size=legendFontSize)
    ax1.text(myXlim[0]+.90*myXrange, 29, 'Other', color='orange', size=legendFontSize)
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

