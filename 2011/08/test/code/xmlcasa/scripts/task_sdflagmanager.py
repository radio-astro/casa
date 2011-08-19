import os
from taskinit import *

import asap as sd
import task_sdsave
import task_flagmanager

def sdflagmanager(infile, mode, versionname, oldname, comment, merge):
	
        casalog.origin('sdflagmanager')
	fg.done()

	try:
		if infile=='':
			raise Exception, 'infile is undefined'

		infilename = os.path.expandvars(infile)
		infilename = os.path.expanduser(infilename)
		if not os.path.exists(infilename):
			s = "File '%s' not found." % (infilename)
			raise Exception, s

		if (mode=='list') or (mode=='save') or (mode=='restore') or (mode=='delete') or (mode=='rename'):
			import datetime
			dt = datetime.datetime.today()
			dtstr = '%d%02d%02d%02d%02d%02d' % (dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)
			
			msfilename = infilename + '-sdflagmanager-temp-' + dtstr + '.ms'
			# if directories 'msfilename' and 'msfilename.flagversions' already exist accidentally,
			# rename it for backup (backupms, backupmsfver)

			sdfverfile = infilename + '.flagversions'
			msfverfile = msfilename + '.flagversions'

			if os.path.exists(sdfverfile):
				os.system('mv %s %s' % (sdfverfile, msfverfile))
			
			task_sdsave.sdsave(infilename, 0, [], [], '', [], [], False, False, 'none', False, 'none', msfilename, 'MS', False)
			task_flagmanager.flagmanager(msfilename, mode, versionname, oldname, comment, merge)

			if mode=='restore':
				backupinfile = infilename + '-sdflagmanager-backup-' + dtstr + '.asap'
				# if a directory with the same name as backupinfile exists, rename it for backup
				os.system('mv %s %s' % (infilename, backupinfile))
				task_sdsave.sdsave(msfilename, 0, [], [], '', [], [], False, False, 'none', False, 'none', infilename, 'ASAP', False)
			
			os.system('rm -rf %s' % (msfilename))
			os.system('mv %s %s' % (msfverfile, sdfverfile))
			
			# if backupms and backupmsfver exist, rename them back
			# to msfilename and msfilename.flagversions, respectively
			
		else:
			raise Exception, "Unknown mode" + str(mode)
		
	except Exception, instance:
		print "*** Error ***", instance
