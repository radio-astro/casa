from glob import glob
import os
import shutil

def replace_with_links(fpats):
    """
    For the list of files/directories returned by glob(fpats), keep the first
    one and replace the rest with symbolic links to the first one.

    Returns the number of links made.
    """
    if isinstance(fpats, str):     # glob('*.1 *.2') does NOT work like shell
        fpats = fpats.split(' ')   # expansion.
    flist = []
    for fpat in fpats:
        sflist = glob(fpat)
        sflist.sort()
        flist.extend(sflist)
    if os.path.islink(flist[0]):
        print "Uh oh!  The first entry in the list of clones is already a link!"
        print "fpats:", fpats
        print "flist:", flist
        print "Exiting..."
        return 0
    nlinks = 0
    for f in flist[1:]:
        if not os.path.islink(f):
            # shutil.rmtree() is a pain compared to rm -rf.  I got the onerror arg
            # to work, but couldn't get it to work quietly.
            if os.path.isdir(f):
                shutil.rmtree(f)
                #print "rmtreed", f
            else:
                os.remove(f)
            os.symlink(flist[0], f)
            nlinks += 1
    return nlinks

def rm_scratch_cols(fpat, keep='data'):
    """
    For each of the MSes matched by glob(fpat), check whether it has scratch
    columns, and if so, remove them.

    The kept column is specified by keep.
    """
    mses = glob(fpat)
    nprocessed = 0
    for vis in mses:
        try:
            tb.open(vis)
            colnames = tb.colnames()
            tb.close()
            if 'MODEL_DATA' in colnames:
                split(vis, '_r_s_c_tmp.ms', datacolumn=keep)
                shutil.rmtree(vis)
                os.rename('_r_s_c_tmp.ms', vis)
                nprocessed += 1
        except Exception, e:
            print "Exception %s rming %s's scratch columns." % (e, vis)
    return nprocessed
