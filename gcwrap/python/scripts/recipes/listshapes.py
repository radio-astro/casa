#!/usr/bin/env python

from glob import glob
import os

try:
    from  casac import *  # No-op if already in casapy.
except:
    import sys
    
    casacpath = glob(os.sep.join(os.environ["CASAPATH"].split() +
                                 ['python', '2.*']))  # devs
    casacpath.sort()
    casacpath.reverse()
    casacpath.extend(glob(os.sep.join([os.environ["CASAPATH"].split()[0],
                                       'lib', 'python2.*'])))  # users
    #print "casacpath =", "\n".join(casacpath)
    sys.path.extend(casacpath)


def get_tool(toolname):
    """
    Returns a CASA tool with name toolname, or None on failure.
    """
    tool = None
    if toolname != 'table':
        tool = casac.table()
    else:
        print "The factory name for", toolname, "is unknown."
    return tool


def listshapes(musthave=[], mspat="*[-_.][Mm][Ss]", combine='or',
               sortfirst=False, incl_ddid=False):
    """
    Lists the data shapes of the MSes matched by mspat.
    """
    if type(musthave) == str:
        musthave = [s.replace(',', '') for s in musthave.split()]

    holderdict = {}
    holderdict['mytb'] = get_tool('tb')
    holderdict['incl_ddid'] = incl_ddid

    splitatdoubleglob = mspat.split('**/')
    if len(splitatdoubleglob) > 1:
        if splitatdoubleglob[0] == '':
            splitatdoubleglob[0] = '.'
        holderdict['mspat'] = splitatdoubleglob[1]
        os.path.walk(splitatdoubleglob[0], checkMSes, holderdict)
    else:
        holderdict['mspat'] = mspat
        checkMSes(holderdict, '', [])
    return holderdict['msdict']


def checkMSes(holderdict, dir, files):
    """
    Updates holderdict['msdict'] with a list of (ncorr, nchan)s for
    each MS in dir that matches holderdict['mspat'].
    """        
    # Yup, ignore files.  It's just a os.path.walk()ism.
    mses = glob(os.path.join(dir, holderdict['mspat']))

    #musthave = holderdict.get('musthave', set([]))
    #use_and = holderdict.get('use_and', False)
    #listall = holderdict.get('listall', False)

    if not holderdict.get('msdict'):   # Initialize it so retval
        holderdict['msdict'] = {}      # can be tied to it.
    retval = holderdict['msdict']
    
    #needed_items = holderdict.get('needed_items', {})
    
    mytb = holderdict['mytb']
    incl_ddid = holderdict['incl_ddid']

    def myopen(mytb, whichtab):
        """
        A wrapper around (my)tb.open(whichtab) which is smarter about error
        handling.  It will still throw an exception on an error, but it tries
        to make the message less misleading.
        """
        retval = False
        if not hasattr(mytb, 'open'):
            raise ValueError, 'mytb is not a tb tool'
        try:
            mytb.open(whichtab)
            retval = True
        except Exception, e:
            # Typically if we are here whichtab is too malformed for
            # mytb to handle, and e is usually "whichtab does not exist",
            # which is usually incorrect.
            if str(e)[-15:] == " does not exist":
                print "tb could not open", whichtab
            else:
                print "Error", e, "from tb.open(", whichtab, ")"
            mytb.close()  # Just in case.
        return retval
    
    for currms in mses:
        if currms[:2] == './':  # strip off leading ./, if present.
            currms = currms[2:]    # cosmetic.

        if incl_ddid:
            retval[currms] = {}
        else:
            retval[currms] = set([])

        if not myopen(mytb, currms + '/POLARIZATION'):
            break
        num_corrs = mytb.getcol('NUM_CORR')
        mytb.close()
        
        if not myopen(mytb, currms + '/SPECTRAL_WINDOW'):
            break
        num_chans = mytb.getcol('NUM_CHAN')
        mytb.close()
        
        if not myopen(mytb, currms + '/DATA_DESCRIPTION'):
            break

        for row in xrange(mytb.nrows()):
            if not mytb.getcell('FLAG_ROW', row):
                key = (num_corrs[mytb.getcell('POLARIZATION_ID', row)],
                       num_chans[mytb.getcell('SPECTRAL_WINDOW_ID', row)])
                if incl_ddid:
                    if retval[currms].has_key(key):
                        retval[currms][key].append(row)
                    else:
                        retval[currms][key] = [row]
                else:
                    retval[currms].add(key)
        mytb.close()


if __name__ == '__main__':
    import pprint
    import sys
    mspat = '*.ms'
    musthave = []
    incl_ddid = False
    if len(sys.argv) > 1:
        incl_ddid = sys.argv[1]
        mspat = sys.argv[2]
        musthave = sys.argv[3:]
    msdict = listshapes(musthave, mspat, incl_ddid=incl_ddid)
    pprint.pprint(msdict)
    
