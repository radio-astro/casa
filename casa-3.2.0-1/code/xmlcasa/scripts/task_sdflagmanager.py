import os
from taskinit import *

import asap as sd
import task_sdsave
import task_flagmanager

def sdflagmanager(sdfile, mode, versionname, oldname, comment, merge):
	
        casalog.origin('sdflagmanager')
	fg.done()

	try:
		if sdfile=='':
			raise Exception, 'infile is undefined'

		sdfilename = os.path.expandvars(sdfile)
		sdfilename = os.path.expanduser(sdfilename)
		if not os.path.exists(sdfilename):
			s = "File '%s' not found." % (sdfilename)
			raise Exception, s

		if (mode=='list') or (mode=='save') or (mode=='restore') or (mode=='delete') or (mode=='rename'):
			import datetime
			dt = datetime.datetime.today()
			dtstr = '%d%02d%02d%02d%02d%02d' % (dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)
			
			msfilename = sdfilename + '-sdflagmanager-temp-' + dtstr + '.ms'
			# if directories 'msfilename' and 'msfilename.flagversions' already exist accidentally,
			# rename it for backup (backupms, backupmsfver)

			sdfverfile = sdfilename + '.flagversions'
			msfverfile = msfilename + '.flagversions'

			if os.path.exists(sdfverfile):
				os.system('mv %s %s' % (sdfverfile, msfverfile))
			
			task_sdsave.sdsave(sdfilename, 0, [], [], '', [], [], False, False, 'none', False, 'none', msfilename, 'MS', False)
			task_flagmanager.flagmanager(msfilename, mode, versionname, oldname, comment, merge)

			if mode=='restore':
				backupsdfile = sdfilename + '-sdflagmanager-backup-' + dtstr + '.asap'
				# if a directory with the same name as backupsdfile exists, rename it for backup
				os.system('mv %s %s' % (sdfilename, backupsdfile))
				task_sdsave.sdsave(msfilename, 0, [], [], '', [], [], False, False, 'none', False, 'none', sdfilename, 'ASAP', False)
			
			os.system('rm -rf %s' % (msfilename))
			os.system('mv %s %s' % (msfverfile, sdfverfile))
			
			# if backupms and backupmsfver exist, rename them back
			# to msfilename and msfilename.flagversions, respectively
			
		else:
			raise Exception, "Unknown mode" + str(mode)
		
	except Exception, instance:
		print "*** Error ***", instance
