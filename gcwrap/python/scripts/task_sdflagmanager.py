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

                domanage(infilename, mode.lower(), versionname, oldname, comment, merge)

	except Exception, instance:
                sdutil.process_exception(instance)
                raise Exception, instance

def domanage(infilename, mode, versionname, oldname, comment, merge):
    availablemodes = ['list','save','restore','delete','rename']
    if mode in availablemodes:
        namer = filenamer('sdflagmanager',infilename)
        msfilename = namer.configure_name(kind='temp',suffix='ms')
        # if directories 'msfilename' and 'msfilename.flagversions' already exist accidentally,
        # rename it for backup (backupms, backupmsfver)
        sdfverfile = namer.flagversion_name(infilename)
        msfverfile = namer.flagversion_name(msfilename)

        if os.path.exists(sdfverfile):
            move(sdfverfile, msfverfile)

        sdutil.save(sd.scantable(infilename,False), msfilename, 'MS2', False)
        task_flagmanager.flagmanager(msfilename, mode, versionname, oldname, comment, merge)

        if mode=='restore':
            backupinfile = namer.configure_name(kind='backup',suffix='asap')
            # if a directory with the same name as backupinfile exists, rename it for backup
            move(infilename, backupinfile)
            sdutil.save(sd.scantable(msfilename,False), infilename, 'ASAP', False)

        remove(msfilename)
        move(msfverfile, sdfverfile)

        # if backupms and backupmsfver exist, rename them back
        # to msfilename and msfilename.flagversions, respectively
    else:
        raise Exception, "Unknown mode" + str(mode)


class filenamer(object):
    def __init__(self, task, infile):
        import datetime
        dt = datetime.datetime.today()
        timestamp = '%d%02d%02d%02d%02d%02d' % (dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)
        self.tmp = string.Template('%s-%s-${kind}-%s.${suffix}'%(infile,task,timestamp))
        self.flagver = 'flagversions'
    def configure_name(self,*args,**kwargs):
        return self.tmp.safe_substitute(**kwargs)
    def flagversion_name(self,filename):
        return string.join([filename.rstrip('/'),self.flagver],'.')

def move(fromfile, tofile):
    os.system('mv %s %s'%(fromfile,tofile))
def remove(filename):
    os.system('rm -rf %s'%(filename))
