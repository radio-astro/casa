import os
from taskinit import *

import asap as sd
import task_flagmanager
import sdutil

def sdflagmanager(infile, mode, versionname, oldname, comment, merge):
        casalog.origin('sdflagmanager')
	fg.done()

	try:
                sdutil.assert_infile_exists(infile)
                infilename=sdutil.get_abspath(infile)

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
			
                        sdutil.save(sd.scantable(infilename,False), msfilename, 'MS2', False)
			task_flagmanager.flagmanager(msfilename, mode, versionname, oldname, comment, merge)

			if mode=='restore':
				backupinfile = infilename + '-sdflagmanager-backup-' + dtstr + '.asap'
				# if a directory with the same name as backupinfile exists, rename it for backup
				os.system('mv %s %s' % (infilename, backupinfile))
                                sdutil.save(sd.scantable(msfilename,False), infilename, 'ASAP', False)
			
			os.system('rm -rf %s' % (msfilename))
			os.system('mv %s %s' % (msfverfile, sdfverfile))
			
			# if backupms and backupmsfver exist, rename them back
			# to msfilename and msfilename.flagversions, respectively
			
		else:
			raise Exception, "Unknown mode" + str(mode)
		
	except Exception, instance:
                sdutil.process_exception(instance)
                raise Exception, instance
