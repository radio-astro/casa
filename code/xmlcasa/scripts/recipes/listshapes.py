#!/usr/bin/env python

from glob import glob
import os

try:
    import casac  # No-op if already in casapy.
except:
    import glob, os, sys
    
    casacpath = glob(os.sep.join(os.environ["CASAPATH"].split() +
                                 ['python', '2.*']))  # devs
    casacpath.sort()
    casacpath.reverse()
    casacpath.extend(glob(os.sep.join([os.environ["CASAPATH"].split()[0],
                                       'lib', 'python2.*'])))  # users
    #print "casacpath =", "\n".join(casacpath)
    sys.path.extend(casacpath)
    import casac


def get_tool(toolname):
    """
    Returns a CASA tool with name toolname, or None on failure.
    """
    toolhomes = {'tb': 'tableHome'}
    tool = None
    if toolhomes.has_key(toolname):
        toolhome = casac.homefinder.find_home_by_name(toolhomes[toolname])
        tool = toolhome.create()
    else:
        print "The factory name for", toolname, "is unknown."
    return tool


def listshapes(musthave=[], mspat="*[-_.][Mm][Ss]", combine='or',
               sortfirst=False):
    """
    Lists the data shapes of the MSes matched by mspat.
    """
    if type(musthave) == str:
        musthave = [s.replace(',', '') for s in musthave.split()]

    holderdict = {}
    holderdict['mytb'] = get_tool('tb')

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
    
    for currms in mses:
        if currms[:2] == './':  # strip off leading ./, if present.
            currms = currms[2:]    # cosmetic.
        
        retval[currms] = set([])

        try:
            mytb.open(currms + '/POLARIZATION')
        except Exception, e:
            # Typically if we are here currms is too malformed for
            # mytb to handle, and e is usually "currms does not exist",
            # which is usually incorrect.
            #print "mses =", ", ".join(mses)
            if str(e)[-15:] == " does not exist":
                print "tb could not open", currms
            else:
                print "Error", e, "from tb.open(", currms, ")"
            mytb.close()  # Just in case.
            break
        num_corrs = mytb.getcol('NUM_CORR')
        
        try:
            mytb.open(currms + '/SPECTRAL_WINDOW')
        except Exception, e:
            # Typically if we are here currms is too malformed for
            # mytb to handle, and e is usually "currms does not exist",
            # which is usually incorrect.
            #print "mses =", ", ".join(mses)
            if str(e)[-15:] == " does not exist":
                print "tb could not open", currms
            else:
                print "Error", e, "from tb.open(", currms, ")"
            mytb.close()  # Just in case.
            break
        num_chans = mytb.getcol('NUM_CHAN')
        
        try:
            mytb.open(currms + '/DATA_DESCRIPTION')
        except Exception, e:
            # Typically if we are here currms is too malformed for
            # mytb to handle, and e is usually "currms does not exist",
            # which is usually incorrect.
            #print "mses =", ", ".join(mses)
            if str(e)[-15:] == " does not exist":
                print "tb could not open", currms
            else:
                print "Error", e, "from tb.open(", currms, ")"
            mytb.close()  # Just in case.
            break

        for row in xrange(mytb.nrows()):
            if not mytb.getcell('FLAG_ROW', row):
                retval[currms].add((num_corrs[mytb.getcell('POLARIZATION_ID',
                                                           row)],
                                    num_chans[mytb.getcell('SPECTRAL_WINDOW_ID',
                                                           row)]))
        mytb.close()


if __name__ == '__main__':
    import sys
    mspat = '*.ms'
    musthave = []
    if len(sys.argv) > 1:
        mspat = sys.argv[1]
        musthave = sys.argv[2:]
    listshapes(musthave, mspat)
