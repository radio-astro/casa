import os
from taskinit import *

def flagmanager(vis=None,mode=None,versionname=None,comment=None,merge=None):
        """Enable list, save, restore and delete flag version files.

        These flag version files are copies of the flag column for a
        measurement set.  They can be restored to the data set to get
        back to a previous flag version.  On running importvla, a flag
        version call 'Original' is automatically produced.

        Keyword arguments:
        vis -- Name of input visibility file
                default: none. example: vis='ngc5921.ms'
        mode -- Flag version operation
                default: 'list'; to list existing flagtables
                'save' will save flag column from vis to a specified flag file
                'restore' will place the specified flag file into vis
                'delete ' will delete specified flag file
        versionname -- Flag version name
                default: none; example: versionname='original_data'
                No imbedded blanks in the versionname
        comment -- Short description of a versionname (used for mode='save')
                default: ''; example: comment='Clip above 1.85'
                comment = versionname
        merge -- Merge operation
                Options: 'or','and', but not recommended for now.

        """

        casalog.origin('flagmanager')

	#Python script
	#parameter_printvalues(arg_names,arg_values,arg_types)
	try:
                if ((type(vis)==str) & (os.path.exists(vis))):
                        fg.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
		if (mode=='list'):
			fg.getflagversionlist()
			print 'See logger for flag versions for this MS'
		elif (mode=='save'):
			fg.saveflagversion(versionname=versionname,comment=comment,merge=merge)
		elif (mode=='restore'):
			fg.restoreflagversion(versionname=versionname,merge=merge)
		elif (mode=='delete'):
			fg.deleteflagversion(versionname=versionname)
		fg.done()
	except Exception, instance:
		print '*** Error ***',instance

