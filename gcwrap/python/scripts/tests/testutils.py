import os
import shutil

# A function object that can be passed to ignore parameter
# of shutil.copytree. It will ignore subversion directory
# when data are copied to working directory.
ignore_subversion = shutil.ignore_patterns('.svn')

def copytree_ignore_subversion(datadir, name, outname=None):
    if outname is None:
        outname = name
    if not os.path.exists(name):
        shutil.copytree(os.path.join(datadir, name), outname,
                        ignore=ignore_subversion)

