from taskinit import *
import time
import os
import sys

debug = False
def flagcmd(vis=None,flagbackup=None,mode=None,flagfile=None,command=None):

        casalog.origin('flagcmd')

        fg.done()
        fg.clearflagselection(0)
        
        try: 
                if ((type(vis)==str) & (os.path.exists(vis))):
                        fg.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

                if mode == 'file':
                        # Read file into command list
			cmd = []

			if ((type(flagfile)==str) & (os.path.exists(flagfile))):
				try:
					ff = open(flagfile,'r')
				except:
					raise Exception, "Error opening file "+flagfile
			else:
				raise Exception, 'Flag command file not found - please verify the name'
			#
			# Parse file
			try:
				cmd = ff.readlines()
			except:
				raise Exception, "Error reading lines from file "+flagfile
			ff.close()
		else:
			cmd = command

		# Now have list of commands
                manualflag_cmd(flagbackup, command=cmd)

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

#
# helper function to parse flagging commands into fg method calls
#
def manualflag_cmd(flagbackup, command):
        if debug: print command

	#List of valid params by mode
	kmodes = {}
	uparams = ['reason','flagtime'] # currently not used
	sparams = ['antenna','timerange','correlation','scan','feed','array','uvrange','spw','field']
	aparams = sparams
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
						else:
						        #raise Exception(str(x)+' has unknown key')
							print str(x)+' has unknown key'
				# Have list of params (besides a mode), may be zero length
				casalog.post(param_list)
				# Special cases of parameter mapping to flagger tool
				parse_params(param_i)
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
				backup_flags('flagcmd_'+mode)
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

# rename some parameters,
# in order to match the interface of fg.tool
#
# validate parameter quackmode

def parse_params(params):
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

def backup_flags(mode):

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

