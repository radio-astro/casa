import os
import time
from taskinit import *


def flagmanager(
    vis=None,
    mode=None,
    versionname=None,
    oldname=None,
    comment=None,
    merge=None,
    ):

    casalog.origin('flagmanager')
    aflocal = casac.agentflagger()

    try:
        if type(vis) == str and os.path.exists(vis):
            if mode != 'rename':
                aflocal.open(vis)
        else:
            raise Exception, \
                'Visibility data set not found - please verify the name'
        if mode == 'list':
            aflocal.getflagversionlist()
            print 'See logger for flag versions for this MS'
            
        elif mode == 'save':
            if versionname == '':
                raise IOError, "Illegal empty versionname: ''"
            
            newdir = vis+'.flagversions/flags.'+versionname
            if os.path.exists(newdir):
                tt = int(time.time())
                tmpname = versionname+'.old.'+str(tt)
                casalog.post("Version name \'%s\' already exist. Will rename it to %s"%(versionname,tmpname), 'WARN')
                
                tmpdir = newdir+'.old.'+str(tt)
                # Rename existing versionname to old name
                os.rename(newdir, tmpdir)
    
                # Edit entry in .flagversions/FLAG_VERSION_LIST
                # For realistic usecases, this file is short enough to keep in memory
                file = vis + '.flagversions/FLAG_VERSION_LIST'
                fd = open(file)
                lines = fd.readlines()
                fd.close()
    
                for i in range(len(lines)):
                    if (lines[i])[:len(versionname) + 3] == versionname + ' : ':
                        lines[i] = tmpname + ' : ' + comment + '\n'
                        break
    
                fd = open(file, 'w')
                fd.writelines(lines)
                fd.close()                                
            
            casalog.post('Save current flagversions to ' + versionname)
            aflocal.saveflagversion(versionname=versionname,
                                    comment=comment, merge=merge)
        elif mode == 'restore':
            if versionname == '':
                raise Exception("Illegal versionname: ''")
            
            casalog.post('Restore flagversions ' + versionname)
            aflocal.restoreflagversion(versionname=versionname,
                    merge=merge)
        elif mode == 'delete':
            if versionname == '':
                raise Exception("Illegal versionname: ''")
            
            aflocal.deleteflagversion(versionname=versionname)
        elif mode == 'rename':
            if versionname == '':
                raise Exception("Illegal versionname: ''")

            if oldname == '':
                raise Exception("Illegal oldname: ''")

            # The directory structure is unlikely to change
            olddir = vis + '.flagversions/flags.' + oldname
            newdir = vis + '.flagversions/flags.' + versionname
            if not os.path.isdir(olddir):
                raise Exception, 'No such flagversions: ' + str(oldname)
            
            if os.path.exists(newdir):
                raise Exception, 'Flagversions ' + str(versionname) \
                    + ' already exists!'

            casalog.post('Rename flagversions "%s" to "%s"' % (oldname,
                         versionname))

            os.rename(olddir, newdir)

            # Edit entry in .flagversions/FLAG_VERSION_LIST
            # For realistic usecases, this file is short enough to keep in memory
            file = vis + '.flagversions/FLAG_VERSION_LIST'
            fd = open(file)
            lines = fd.readlines()
            fd.close()

            for i in range(len(lines)):
                if (lines[i])[:len(oldname) + 3] == oldname + ' : ':
                    lines[i] = versionname + ' : ' + comment + '\n'
                    break

            fd = open(file, 'w')
            fd.writelines(lines)
            fd.close()
            
        else:
            raise Exception, 'Unknown mode' + str(mode)
        
        aflocal.done()
    except Exception, instance:
#        print '*** Error ***', instance
        raise Exception, instance


