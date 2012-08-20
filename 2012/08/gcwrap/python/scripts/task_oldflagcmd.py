from taskinit import *
import time
import os
import sys

debug = False
def oldflagcmd(vis=None,flagmode=None,flagfile=None,flagrows=None,command=None,tbuff=None,antenna=None,reason=None,useapplied=None,optype=None,flagsort=None,outfile=None,flagbackup=None,reset=None,clearall=None,rowlist=None,setcol=None,setval=None):
	#
	# Task oldflagcmd
	#    Reads flag commands from file or string and applies to MS
	# v1.0 Created STM 2010-08-31 (3.1.0) from flagdata
	# v2.0 Updated STM 2010-09-16 (3.1.0) add Flag.xml reading
	# v2.1 Updated STM 2010-09-24 (3.1.0) add FLAG_CMD use, optype
	# v2.2 Updated STM 2010-10-14 (3.1.0) compatible with new importevla
	# v2.3 Updated STM 2010-10-22 (3.1.0) improvements
	# v2.4 Updated STM 2010-10-26 (3.1.0) optype unapply
	# v2.5 Updated STM 2010-11-01 (3.1.0) bug fix
	# v2.6 Updated STM 2010-11-02 (3.1.0) bug fix
	# v3.0 Updated STM 2010-11-29 (3.2.0) bug fix history, backup
	# v3.1 Updated STM 2010-12-01 (3.2.0) prec=9 on timestamps
	# v3.2 Updated STM 2010-12-03 (3.2.0) handle comments # again
	# v3.2 Updated STM 2010-12-08 (3.2.0) bug fixes in flagsort use, parsing
        # v3.3 Updated STM 2010-12-20 (3.2.0) bug fixes parsing errors
        # v3.4 Updated STM 2011-02-14 (3.2.0) bug fix mode online from xml
        # v3.5 Updated STM 2011-03-21 (3.2.0) go to fglocal and mslocal
        # v3.5 Updated STM 2011-03-23 (3.2.0) bug fix casalog.post long lines
        # v3.6 Updated STM 2011-05-31 (3.3.0) optype extract, bug fix plotting
        # v3.6 Updated STM 2011-06-28 (3.3.0) version for replacement in test
	# v3.6 Updated SMC 2011-09-22 (3.3.0) added observation, intent parameters
        # v4.0 Updated STM 2011-07-26 (3.3.0) parse with spaces in names
        # v4.1 Updated STM 2011-10-19 (3.4.0) handle new ALMA SDM Flag.xml
        # v4.1 Updated STM 2011-11-02 (3.4.0) work on SDMs for some modes
        # v4.1 Updated STM 2011-11-03 (3.4.0) improved plotting
	# v4.1 Updated STM 2011-12-14 (3.4.0) merge with SMC changes
	# v4.1 Updated STM 2011-12-19 (3.4.0) bug fix, better history handling
	# v4.2 Updated STM 2012-02-10 (3.4.0) allow save to file for sdm
	# v4.2 Updated STM 2012-02-13 (3.4.0) readflagxml spectral window fix
	# v4.2 Updated STM 2012-02-16 (3.4.0) newplotflags trim flag times to data times
	# v4.2 Updated STM 2012-02-21 (3.4.0) update subroutines for ant+spw+sol sorting
	# v4.2 Updated STM 2012-02-23 (3.4.0) final fixes
	# v4.2 Updated STM 2012-02-27 (3.4.0) final final fixes
	# v4.2 Updated STM 2012-03-01 (3.4.0) allow multiple timeranges in cmd mode
	#
	try:
		from xml.dom import minidom
	except:
		raise Exception, 'Failed to load xml.dom.minidom into python'

        casalog.origin('oldflagcmd')
        
	
	casalog.post('You are using oldflagcmd v4.2 Updated STM 2012-03-01')

        fglocal = casac.flagger()
        mslocal = casac.ms()

        msopened = False
	ms_starttime = ''
	ms_endtime = ''
	t1sdata = 0.0
	t2sdata = 0.0

	datatype = -1
	# Try to identify type for vis
	if os.path.exists(vis+'/table.dat'):
		# this is probably an ms
		# Get overall MS time range for later use (if needed)
		# Updated STM 2012-02-16 v4.2
		(ms_starttime,ms_endtime,t1sdata,t2sdata) = getmstimes(vis)
		msopened = ms_starttime != ''
		if msopened:
			print 'It appears that vis is a MS'
			casalog.post('It appears that vis is a MS')
			datatype = 0
			print 'Found data times from '+ms_starttime+' to '+ms_endtime
			#casalog.post('Found data times from '+ms_starttime+' to '+ms_endtime)
		else:
			print 'WARNING: I thought that vis is a MS, but cannot open it'
			casalog.post('I thought that vis is a MS, but cannot open it','WARN')
			datatype = -1
	elif os.path.exists(vis+'/ASDM.xml'):
		# this is probably a SDM
		print 'It appears that vis is a SDM'
		casalog.post('It appears that vis is a SDM')
		datatype = 1
	else:
		print 'WARNING: Unknown data type for vis = '+vis
		casalog.post('Unknown data type for vis = '+vis,'WARN')

	if datatype != 0:
		print 'This is not a MS, only listing, plotting, and save-to-file available'
		casalog.post('This is not a MS, only listing, plotting, and save-to-file  available')

	if msopened or (flagmode!='table' and (optype=='list' or optype=='plot' or
					       (optype=='save' and outfile!=''))):
            try: 
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
				myflagcmd = getflagcmds(cmdlist,t1sdata,t2sdata)

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
			print 'Reading from input list'
			cmdlist = command
			print 'Input '+str(cmdlist.__len__())+' lines from input list'
			casalog.post('Input '+str(cmdlist.__len__())+' lines from input list')

			if cmdlist.__len__()>0:
				myflagcmd = getflagcmds(cmdlist,t1sdata,t2sdata)

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

		print 'Executing optype = '+optype
		casalog.post('Executing optype = '+optype)
		
		# Now have list of commands - do something with them
		if optype == 'apply':
			# Possibly resort flags before application
			# NEEDED TO AVOID FLAG AGENT ERROR IN PRE-3.3
			if flagsort!='' and flagsort!='id':
				myflagd = sortflags(myflagcmd,myantenna='',myreason='Any',myflagsort=flagsort)
			else:
				myflagd = myflagcmd

			# Apply flags to data using flagger
			nappl = applyflagcmd(fglocal, vis, flagbackup, myflagd, reset)
			# Save flags to file
			if nappl>0:
			    if outfile=='':
			        if flagmode=='table' and flagfile=='':
			            # These flags came from internal FLAG_CMD, update status
				    print 'Updating APPLIED status in FLAG_CMD'
				    casalog.post('Updating APPLIED status in FLAG_CMD')
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
		elif optype == 'unapply':
			# Possibly resort flags before application
			# NEEDED TO AVOID FLAG AGENT ERROR
			if flagsort!='' and flagsort!='id':
				myflagd = sortflags(myflagcmd,myantenna='',myreason='Any',myflagsort=flagsort)
			else:
				myflagd = myflagcmd

			# (Un)Apply flags to data using flagger
			nappl = applyflagcmd(fglocal, vis, flagbackup, myflagd, flagtype='UNFLAG')
			# Save flags to file
			if nappl>0:
			    if flagmode=='table' and flagfile=='':
			        # These flags came from internal FLAG_CMD, update status
				# to APPLIED=True and TYPE='UNFLAG'
				print 'Updating APPLIED status in FLAG_CMD'
				myrowlist = myflagcmd.keys()
				if myrowlist.__len__()>0:
				    updateflagcmd(vis,mycol='APPLIED',myval=False,myrowlist=myrowlist)
			    else:
			        # These flags came from outside the MS, so add to FLAG_CMD
				# Tag APPLIED=False
				# NOTE: original flags, not using resorted version
				writeflagcmd(vis,myflagcmd,tag='unapplied')
			else:
			    print 'WARNING: No flags were unapplied from list'
			    casalog.post('WARNING: No flags were unapplied from list','WARN')
		elif optype=='list':
			# List flags
			listflags(myflagcmd,myoutfile=outfile,listmode=listmode)
			
		elif optype=='save':
			# Possibly resort and/or select flags by antenna or reason
			if flagsort!='' and flagsort!='id':
				myflagd = sortflags(myflagcmd,myantenna='',myreason='Any',myflagsort=flagsort)
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
					print '   Saving to file '+outfile
					casalog.post('Saving to file '+outfile)
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
				#plotflags(myflagcmd,outfile,t1sdata,t2sdata)
				newplotflags(myflagcmd,outfile,t1sdata,t2sdata)
			else:
				print 'Warning: empty flagcmd dictionary, nothing to plot'
				casalog.post('Warning: empty flag dictionary, nothing to plot')
		elif optype=='extract':
			# Output flag dictionary
			print 'Secret mode extract : Returning flag dictionary'
			casalog.post('Secret mode extract : Returning flag dictionary')
			return myflagcmd
				
            except Exception, instance:
                #fglocal.done()
                print '*** Error ***', instance
                #raise
				
        #write history
	if msopened and not (optype=='list' or optype=='plot' or optype=='extract'):
	    try:
		mslocal.open(vis,nomodify=False)
		mslocal.writehistory(message='taskname = oldflagcmd', origin='oldflagcmd')
		mslocal.writehistory(message='vis      = "' + str(vis) + '"', origin='oldflagcmd')
		mslocal.writehistory(message='flagmode = "' + str(flagmode) + '"', origin='oldflagcmd')
		if flagmode == 'file':
			mslocal.writehistory(message='flagfile = "' + str(flagfile) + '"', origin='oldflagcmd')
		elif flagmode=='cmd':
			for cmd in command:
				mslocal.writehistory(message='command  = "' + str(cmd) + '"', origin='oldflagcmd')
		mslocal.close()
	    except:
		casalog.post('Cannot open vis for history, ignoring','WARN')

        return

#===============================================================================
#
# helper functions to parse flagging commands into fg method calls
#
#===============================================================================

#===============================================================================
# Apply flag commands using flagger tool
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
        # Updated STM v4.2 2012-02-21 handle polarization (poln) flags
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
                           comment='oldflagcmd autosave before ' + mode + ' on ' + time_string,
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
# Optional keys:
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
        starttime = qa.time(t,form="ymd",prec=9)[0]
        rowend = rownode.getElementsByTagName("endTime")
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjds = (float(end)*1.0E-9) + mytbuff
        t = qa.quantity(endmjds,'s')
        endtime = qa.time(t,form="ymd",prec=9)[0]
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
# Optional keys:
#         'spw' (string)
#         'poln' (string)
#   
# Updated STM 2010-12-03 (3.2.0) handle comments # again
# Updated STM 2010-12-08 (3.2.0) bug fixes in flagsort use, parsing
# Updated STM 2010-12-20 (3.2.0) bug fixes parsing errors
# Updated STM 2012-02-21 (3.4.0) handle poln flags
# Updated STM 2012-02-23 (3.4.0) handle spw,poln as optional keys
# Updated STM 2012-02-27 (3.4.0) bug fix
# Updated STM 2012-02-29 (3.4.0) allow multiple timeranges
#
    myflagd={}
    nrows = cmdlist.__len__()
    if nrows==0:
	    print 'WARNING: empty flag command list'
	    casalog.post('WARNING: empty flag command list', 'WARN')
	    return myflagd

    t = qa.quantity(ms_startmjds,'s')
    ms_startdate = qa.time(t,form=["ymd","no_time"])[0]
    t0 = qa.totime(ms_startdate+'/00:00:00.0')
    #t0d = qa.convert(t0,'d')
    t0s = qa.convert(t0,'s')

    ncmds = 0
    for i in range(nrows):
	    cmdstr = cmdlist[i]
	    # break string into key=val sets
	    keyvlist = cmdstr.split()
	    if keyvlist.__len__()>0:
		    # Fixed key defaults
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
		    # Optional keys
		    spw = 'Unset'
		    poln = 'Unset'
		    for keyv in keyvlist:
			    # check for comment character #
			    if keyv.count("#")>0:
				    # break out of loop parsing keyvals
				    break
			    try:
				    (xkey,val) = keyv.split('=')
			    except:
				    print 'Not a key=val pair: '+keyv
				    break
			    xval = val
			    # Use eval to deal with conversion from string
			    #xval = eval(val)
			    # strip quotes from value (if still a string)
			    if type(xval)==str:
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
				    intvl = xval
			    else:
				    # Extract (but keep in string)
				    if xkey=='timerange':
					    timstr = xval
					    # Extract TIME,INTERVAL
					    # STM 2012-03-01 Allow for comma-separated ranges
					    if timstr!='':
						    startmjds = 0.0
						    endmjds = 0.0
						    timlist = timstr.split(',')
						    
						    for timr in timlist:
							    try:
								    (startstr,endstr) = timr.split('~')
							    except:
								    if timr.count('~')==0:
									    # print 'Assuming a single start time '
									    startstr = timr
									    endstr = timr
								    else:
									    print 'Not a start~end range: '+timr
									    print "ERROR: too may ~'s "
									    raise Exception, "Error parsing "+timr
							    t = qa.totime(startstr)
							    starts = qa.convert(t,'s')
							    if starts['value']<1.E6:
								    # assume a time offset from ref
								    starts = qa.add(t0s,starts)
							    startm = starts['value']
							    if endstr=='':
								    endstr = startstr
							    t = qa.totime(endstr)
							    ends = qa.convert(t,'s')
							    if ends['value']<1.E6:
								    # assume a time offset from ref
								    ends = qa.add(t0s,ends)
							    endm = ends['value']
							    if startmjds==0.0 or startm<startmjds:
								    startmjds = startm
							    if endm>endmjds:
								    endmjds = endm
						    if startmjds==0.0:
							    startmjds = ms_startmjds
						    if endmjds==0.0:
							    endmjds = ms_endmjds
						    # Use the span of the timeranges
						    tim = 0.5*(startmjds+endmjds)
						    intvl = endmjds - startmjds
					    
				    elif xkey=='antenna':
					    ant = xval
				    elif xkey=='spw':
					    spw = xval
				    elif xkey=='poln':
					    poln = poln
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
			    # Fixed keys
			    flagd['id'] = fid
			    flagd['mode'] = fmode
			    flagd['antenna']=ant
			    flagd['timerange']=timstr
			    flagd['reason']=reas
			    flagd['cmd']=cmd
			    flagd['time']=tim
			    flagd['interval']=intvl
			    flagd['type']=typ
			    flagd['level']=levl
			    flagd['severity']=sevr
			    flagd['applied']=appl
			    # Optional keys
			    if spw!='Unset':
				    flagd['spw']=spw
			    if poln!='Unset':
				    flagd['poln']=poln
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
    # Optional keys:
    #         'spw' (string)
    #         'poln' (string)
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
        # STM 2012-02-27 add spw poln
        #phdr = '%8s %12s %8s %32s %48s' % ('Key', 'FlagID', 'Antenna', 'Reason', 'Timerange')
	phdr = '%8s %12s %8s %32s %48s %4s %s' % ('Key', 'FlagID', 'Antenna', 'Reason', 'Timerange', 'Poln', 'Spw')
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
	# STM 2012-02-27 add spw poln
	if fld.has_key('spw'):
		spw = fld['spw']
	else:
		spw = ''
	if fld.has_key('poln'):
		poln = fld['poln']
	else:
		poln = ''
	# Print out listing
	if myantenna=='' or myantlist.count(ant)>0:
	    if myreason=='' or myreaslist.count(reas)>0:
	        if listmode=='online':
		    #pstr = '%8s %12s %8s %32s %48s' % (skey, fid, ant, reas, timr)
		    # STM 2012-02-27 add spw poln
		    pstr = '%8s %12s %8s %32s %48s %4s %s' % (skey, fid, ant, reas, timr, poln, spw)
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
    # Optional keys:
    #         'spw' (string)
    #         'poln' (string)
    #
    # Updated STM 2012-02-27 (3.4.0) handle spw,poln as optional keys
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
			    try:
			        (xkey,val) = keyv.split('=')
			    except:
			        print 'Error: not key=value pair for '+keyv
				break
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
			    # STM 2012-02-27 add spw and poln
			    elif xkey=='spw':
			        flagd['spw'] = xval
			    elif xkey=='poln':
			        flagd['poln'] = xval
	            # STM 2010-12-08 Do not put timerange if not in command
		    # if timstr=='':
		    # 	    # Construct timerange from time,interval
		    # 	    centertime = f_time[i]
		    # 	    interval = f_interval[i]
		    # 	    startmjds = centertime - 0.5*interval
		    # 	    t = qa.quantity(startmjds,'s')
		    # 	    starttime = qa.time(t,form="ymd",prec=9)
		    # 	    endmjds = centertime + 0.5*interval
		    # 	    t = qa.quantity(endmjds,'s')
		    # 	    endtime = qa.time(t,form="ymd",prec=9)
		    # 	    timstr = starttime+'~'+endtime
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
    
def getmstimes(vis):
    # Get start and end times from MS
    # this might take too long for large MS
    # NOTE: could also use values from OBSERVATION table col TIME_RANGE
    mslocal2 = casac.ms()
    success = True
    ms_time1 = ''
    ms_time2 = ''
    t1sd=0.0
    t2sd=0.0
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
	    ms_time1 = qa.time(t,form="ymd",prec=9)[0]
	    t = qa.quantity(ms_endmjds,'s')
	    t2sd = t['value']
	    ms_time2 = qa.time(t,form="ymd",prec=9)[0]
	    casalog.post('MS spans timerange '+ms_time1+' to '+ms_time2)
    else:
	    print 'WARNING: Could not open vis as MS to find times'
	    casalog.post('WARNING: Could not open vis as MS to find times')
    return (ms_time1, ms_time2, t1sd, t2sd)

def newplotflags(myflags, plotname, t1sdata, t2sdata):
    #
    # Function to plot flagging dictionary
    # Adapted from J.Marvil
    # Updated STM v4.1 2011-11-02 to handle ALMA flags
    # Updated STM v4.2 2012-02-16 trim flag times to data times
    try: 
        import casac 
    except ImportError, e: 
        print "failed to load casa:\n", e 
        exit(1) 
    qatool = casac.quanta()
    qa = casac.qa = qatool.create()
    
    try:
        import pylab as pl
    except ImportError, e:
        print "failed to load pylab:\n", e
        exit(1)

    # list of supported colors (in order)
    colorlist = ['red',
		 'blue',
		 'green',
		 'black',
		 'cyan',
		 'magenta',
		 'yellow',
		 'orange'
		 ]
    ncolors = colorlist.__len__()

    # get list of flag keys
    keylist=myflags.keys()

    # get lists of antennas and reasons
    # make plotting dictionary
    myants=[]
    myreas=[]
    plotflag = {}
    ipf = 0
    for key in keylist:
	    antstr = myflags[key]['antenna']
	    reastr = myflags[key]['reason']
	    timstr = myflags[key]['timerange']
	    if antstr!='':
		    # flags that have antenna specified
		    antlist = antstr.split(',')
		    nantlist = antlist.__len__()
	    else:
		    # Special
		    antlist = ['All']
		    nantlist = 1
	    #
	    realist = reastr.split(',')
	    nrealist = realist.__len__()
	    #
	    timlist = timstr.split(',')
	    ntimlist = timlist.__len__()
	    #
	    # Break these into nants x ntimes flags
	    # Trick is assigning multiple reasons
	    # Normal cases:
	    # A. One reason, single/multiple antennas x times
	    # B. Multiple reasons=times, single/multiple antenna(s)
	    # C. Single time, multiple antennas/reasons
	    # D. Multiple reasons, no way to correspond with times
	    #
	    timmin = 1.0E11
	    timmax = 0.0
	    if nrealist==1:
		    # simplest case, single reason
		    reas = realist[0]
		    if reas=='':
			    reas = 'Unknown'
		    if myreas.count(reas)==0:
			    myreas.append(reas)
		    for ia in range(nantlist):
			    ant = antlist[ia]
			    if myants.count(ant)==0:
				    myants.append(ant)
			    for it in range(ntimlist):
				    times = timlist[it]
				    plotflag[ipf] = {}
				    plotflag[ipf]['antenna'] = ant
				    plotflag[ipf]['reason'] = reas
				    plotflag[ipf]['timerange'] = times
				    ipf += 1
	    elif nrealist==ntimlist:
		    # corresponding reasons and times
		    for ia in range(nantlist):
			    ant = antlist[ia]
			    if myants.count(ant)==0:
				    myants.append(ant)
			    for it in range(ntimlist):
				    times = timlist[it]
				    reas = realist[it]
				    if reas=='':
					    reas = 'Unknown'
				    if myreas.count(reas)==0:
					    myreas.append(reas)
				    plotflag[ipf] = {}
				    plotflag[ipf]['antenna'] = ant
				    plotflag[ipf]['reason'] = reas
				    plotflag[ipf]['timerange'] = times
				    ipf += 1
	    else:
		    # no correspondence between multiple reasons and ants/times
		    # assign reason 'Miscellaneous'
		    reas = 'Miscellaneous'
		    if myreas.count(reas)==0:
			    myreas.append(reas)
		    for ia in range(nantlist):
			    ant = antlist[ia]
			    if myants.count(ant)==0:
				    myants.append(ant)
			    for it in range(ntimlist):
				    times = timlist[it]
				    plotflag[ipf] = {}
				    plotflag[ipf]['antenna'] = ant
				    plotflag[ipf]['reason'] = reas
				    plotflag[ipf]['timerange'] = times
				    ipf += 1
			    
    myants.sort()
    nants = myants.__len__()
    nreas = myreas.__len__()
    print '  Found '+str(nreas)+' reasons to plot for '+str(nants)+' antennas'
    npf = ipf
    print '  Found '+str(npf)+' total flag ranges to plot'

    # sort out times
    for ipf in range(npf):
	    times = plotflag[ipf]['timerange']
	    if times!='':
		    if times.count('~')>0:
			    t1=times[:times.find('~')]
			    t2=times[times.find('~')+1:]
		    else:
			    t1=times
			    t2=t1
		    t1s, t2s=qa.convert(t1,'s')['value'], qa.convert(t2,'s')['value']
		    plotflag[ipf]['t1s'] = t1s
		    plotflag[ipf]['t2s'] = t2s
		    if t1s<timmin:
			    timmin=t1s
		    if t2s>timmax:
			    timmax=t2s
    # min,max times
    q1 = qa.quantity(timmin,'s')
    time1=qa.time(q1,form='ymd',prec=9)[0]
    q2 = qa.quantity(timmax,'s')
    time2=qa.time(q2,form='ymd',prec=9)[0]
    print '  Found flag times from '+time1+' to '+time2
    
    # sort out blank times
    for ipf in range(npf):
	    times = plotflag[ipf]['timerange']
	    if times=='':
		    if t2sdata >= t1sdata > 0:
			    plotflag[ipf]['t1s'] = t1sdata
			    plotflag[ipf]['t2s'] = t2sdata
		    else:
			    plotflag[ipf]['t1s'] = timmin
			    plotflag[ipf]['t2s'] = timmax
		    
	    
    # if flag times are beyond range of data, trim them
    # Added STM 2012-02-16
    if (t2sdata >= t1sdata > 0) and (timmin<t1sdata or timmax>t2sdata):
	    # min,max data times
	    q1 = qa.quantity(t1sdata,'s')
	    tdata1=qa.time(q1,form='ymd',prec=9)[0]
	    q2 = qa.quantity(t2sdata,'s')
	    tdata2=qa.time(q2,form='ymd',prec=9)[0]
	    print '  WARNING: Trimming flag times to data limits '+tdata1+' to '+tdata2
	    
	    for ipf in range(npf):
		    t1s = plotflag[ipf]['t1s']
		    t2s = plotflag[ipf]['t2s']
		    if t1s < t1sdata:
			    plotflag[ipf]['t1s'] = t1sdata
		    if t2s > t2sdata:
			    plotflag[ipf]['t2s'] = t2sdata
	    
    # make reason dictionary with mapping of colors and offsets (-0.3 to 0.3)
    readict = {}
    reakeys = []
    if nreas>ncolors:
	    for i in range(nreas):
		    reas = myreas[i]
		    readict[reas] = {}
		    if i < ncolors-1:
			    colr = colorlist[i]
			    readict[reas]['color'] = colr
			    readict[reas]['index'] = i
			    offs = 0.3-(float(i)*0.6/float(ncolors-1))
			    readict[reas]['offset'] = offs
			    reakeys.append(reas)
		    else:
			    colr = colorlist[ncolors-1]
			    readict[reas]['color'] = colr
			    readict[reas]['index'] = ncolors-1
			    readict[reas]['offset'] = -0.3
	    reakeys.append('Other')
	    readict['Other'] = {}
	    readict['Other']['color'] = colorlist[ncolors-1]
	    readict['Other']['index'] = ncolors-1
	    readict['Other']['offset'] = -0.3
    else:
	    for i in range(nreas):
		    reas = myreas[i]
		    reakeys.append(reas)
		    colr = colorlist[i]
		    offs = 0.3-(float(i)*0.6/float(ncolors-1))
		    readict[reas] = {}
		    readict[reas]['color'] = colr
		    readict[reas]['index'] = i
		    readict[reas]['offset'] = offs
    nlegend = reakeys.__len__()
    print '  Will plot '+str(nlegend)+' reasons in legend'

    antind=0
    if plotname=='':
	    pl.ion()
    else:
	    pl.ioff()

    f1=pl.figure()
    ax1=f1.add_axes([.15,.1,.75,.85])
#    ax1.set_ylabel('antenna')
#    ax1.set_xlabel('time')
    #badflags=[]
    for thisant in myants:
        antind+=1
	for ipf in range(npf):
	    if plotflag[ipf]['antenna']==thisant:
                thisReason=plotflag[ipf]['reason']
		thisColor=readict[thisReason]['color']
		thisOffset=readict[thisReason]['offset']
		t1s = plotflag[ipf]['t1s']
		t2s = plotflag[ipf]['t2s']
		myTimeSpan=t2s-t1s
		#
		# Old approach, restrict timerange
		# if myTimeSpan < 10000:
		# 	ax1.plot([t1s,t2s],[antind+thisOffset,antind+thisOffset], color=thisColor, lw=2, alpha=.7)
		# else: badflags.append((thisant, myTimeSpan, thisReason))

		ax1.plot([t1s,t2s],[antind+thisOffset,antind+thisOffset], color=thisColor, lw=2, alpha=.7)
    ##badflags are ones which span a time longer than that used above
    ##they can be so long that including them compresses the time axis so that none of the other flags are visible    
#    print 'badflags', badflags
    print '  Plotted flags'

    myXlim=ax1.get_xlim()
    myXrange=myXlim[1]-myXlim[0]
    # Pad the time axis?
    PadTime=0.05
    if PadTime>0:
	    xPad = PadTime*myXrange
	    x0 = myXlim[0] - xPad
	    x1 = myXlim[1] + xPad
	    ax1.set_xlim(x0,x1)
	    myXrange=x1-x0
	    #print '  Rescaled x axis'
    else:
	    x0 = myXlim[0]
	    x1 = myXlim[1]

    legendFontSize=12
    myYupper = nants + nlegend + 1.5
    # place legend text
    i = nants
    x = x0 + 0.05*myXrange
    for reas in reakeys:
	    i += 1
	    colr = readict[reas]['color']
	    ax1.text(x, i, reas, color=colr, size=legendFontSize)
    ax1.set_ylim([0,myYupper])   

    ax1.set_yticks(range(1,len(myants)+1))
    ax1.set_yticklabels(myants)
    #print '  Relabled y axis'

    nxticks = 3
    ax1.set_xticks(pl.linspace(myXlim[0],myXlim[1],nxticks))

    mytime = []
    myTimestr = []
    for itim in range(nxticks):
	    time = myXlim[0] + (myXlim[1]-myXlim[0])*float(itim)/float(nxticks-1)
	    mytime.append(time)
	    q1=qa.quantity(time,'s')
	    time1=qa.time(q1,form='ymd',prec=9)[0]
	    if itim>0:
		    time1s = time1[11:]
	    else:
		    time1s = time1
	    myTimestr.append(time1s)
    
    ax1.set_xticklabels(myTimestr)
    #print myTimestr
    if plotname=='':
	    pl.draw()
    else:
	    pl.savefig(plotname, dpi=150)    
    return

