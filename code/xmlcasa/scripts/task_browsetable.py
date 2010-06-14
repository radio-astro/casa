import os
import re
import shutil
import tempfile
from taskinit import *

def browsetable(tablename=None, mightedit=None,
                sortlist=None, taql=None, skipcols=None):
    """ Browse a table (visibility data set, calibration table, or image):

    Brings up a GUI browser that can open and display any CASA table.

    Parameters:
        tablename -- Name of table directory on disk (MS, cal. table, image)
                     default: none; example: tablename='ngc5921.ms'
        mightedit -- If True disable the filtering options (below) and allow
                     editing the table.  Warning: the GUI appears to ignore
                     whether the table tool is opened read-only - just be
                     aware that you should not edit filtered tables unless
                     you know what you are doing.
        sortlist  -- List of columns to sort by.
                     default: [] (none)
        taql      -- TaQL query string for prefiltering the table.
                     default: '' (none); example: taql='ANTENNA2 < 6'
        skipcols  -- Columns to NOT display.
                     default: [] (none); example: skipcols='feed1, feed2'
    """
    #Python script
    try:
        casalog.origin('browsetable')
        if type(tablename) == str:
            t = tbtool.create()
            mightfilter = sortlist or taql or skipcols
            if os.path.exists(tablename):
                t.open(tablename, nomodify=not mightedit)
            if not tablename or mightedit or not mightfilter:
                t.browse()
            else:
                colnames = t.colnames()
                if type(skipcols) == str:
                    skipcols = re.split(r',?\s+', skipcols)
                for c in skipcols:
                    colnames.remove(c.upper())
                if type(sortlist) == list:
                    sortlist = ', '.join(sortlist)
                   
                # ft.browse won't work unless ft is given a name.  This does NOT
                # appear to cause more than about 116 kB to be written to disk,
                # but it does mean that the directory needs to be removed
                # afterwards.
                tempdir = '/tmp'
                if not os.path.isdir(tempdir):
                    tempdir = os.getcwd()
                ftname = tempfile.mkdtemp(prefix=tablename + '_filtered_', dir=tempdir)
                #print "ftname =", ftname
                casalog.post('Using ' + ftname + ' as the filtered table.')
               
                ft = t.query(taql, ftname, sortlist=sortlist.upper(),
                             columns=', '.join(colnames))
                #print "ft.name() =", ft.name()
                ft.browse()
                ft.close()
                #shutil.rmtree(ftname)  # Can't do until browser exits.
            t.close()  # Can be closed even if not open.
        else:
            raise Exception, 'Table ' + str(tablename) + ' not found - please check the name'
    except Exception, instance:
        print '*** Error ***', instance
