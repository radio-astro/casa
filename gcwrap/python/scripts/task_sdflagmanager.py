import os
import string

from taskinit import casalog, gentools
import asap as sd
import task_flagmanager
import sdutil

@sdutil.sdtask_decorator
def sdflagmanager(infile, mode, versionname, oldname, comment, merge):
    with sdutil.sdtask_manager(sdflagmanager_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
    

class sdflagmanager_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdflagmanager_worker,self).__init__(**kwargs)
        self.infile_abs = sdutil.get_abspath(self.infile)

    def parameter_check(self):
        availablemodes = ['list', 'save', 'restore', 'delete', 'rename']
        if not (self.mode in availablemodes):
            raise Exception, "Unknown mode" + str(mode)

        namer = filenamer('sdflagmanager', self.infile_abs)
        self.msfile = namer.configure_name(kind='temp',suffix='ms')
        self.sdfverfile = namer.flagversion_name(self.infile_abs)
        self.msfverfile = namer.flagversion_name(self.msfile)
        self.backupfile = namer.configure_name(kind='backup',suffix='asap')

    def initialize_scan(self):
        self.scan = sd.scantable(self.infile_abs, average=False)

    def execute(self):
        if os.path.exists(self.sdfverfile):
            move(self.sdfverfile, self.msfverfile)

        sdutil.save(self.scan, self.msfile, 'MS2', False)
        task_flagmanager.flagmanager(self.msfile,
                                     self.mode,
                                     self.versionname,
                                     self.oldname,
                                     self.comment,
                                     self.merge)

        if self.mode=='restore':
            # if a directory with the same name as backupinfile exists, rename it for backup
            del self.scan
            move(self.infile_abs, self.backupfile)
            sdutil.save(sd.scantable(self.msfile,False), self.infile_abs, 'ASAP', False)

        move(self.msfverfile, self.sdfverfile)

    def cleanup(self):
        if hasattr(self, 'msfile'):
            remove(self.msfile)

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
