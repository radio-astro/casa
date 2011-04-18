from taskinit import *
from vishead_util import *
import re
import shutil

def fixvis(vis, outputvis, fldids=None, refcode=None, reuse=True
           #, proj=None, ptcs=None
           ):
    proj=None
    ptcs=None
    casalog.origin('fixvis')
    for arg in ('vis', 'outputvis', 'fldids', 'refcode', 'proj', 'ptcs'):
        casalog.post("parameter %9s: %s" % (arg, eval(arg)), 'DEBUG1')

    if refcode or proj:
        badcsys = False
        csys = cs.newcoordsys(True)

        if refcode:
            refcodes = csys.referencecode('dir', True)
            if refcode.upper() not in refcodes:
                casalog.post("refcode %s is invalid" % refcode, 'SEVERE')
                casalog.post("Valid codes are %s" % refcodes, 'NORMAL')
                badcsys = True

        if proj:
            projs = csys.projection('all')['types']
            if proj.upper() not in projs:
                casalog.post("proj %s is invalid" % proj, 'SEVERE')
                casalog.post("Valid projection codes are %s" % projs, 'NORMAL')
                badcsys = True

        csys.done()
        if badcsys:
            return

    if not refcode:   # measures must be told to use the one in vis.
        # UVW is often mislabelled as ITRF when it is really J2000.
        #refcode = tb.getcolkeyword('UVW', 'MEASINFO')['Ref']

        # Because of that bug, it is better to make UVW have the same frame as
        # PHASE_DIR.  imager assumes that anyway.
        try:
            tb.open(vis + '/FIELD')
            refcode = tb.getcolkeyword('PHASE_DIR', 'MEASINFO')['Ref']
            tb.close()
        except Exception, instance:
            casalog.post("*** Error \'%s\' getting the reference frame from PHASE_DIR of %s/FIELD." % (instance, vis),
                         'SEVERE')
            return
            

    if outputvis != vis:
        try:
            #if os.path.isdir(outputvis):
            #    shutil.
            shutil.copytree(vis, outputvis)
        except Exception, instance:
            casalog.post("*** Error %s copying %s to %s." % (instance,
                                                             vis, outputvis),
                         'SEVERE')
            return
        vis = outputvis
    
    try:
        # Get field IDs before opening the ms so that tb doesn't interfere with
        # im.
        allflds = getput_keyw('get', vis, ['FIELD', 'NAME'], '')[0]

        # getput_keyw('get', vis, ['FIELD', 'PHASE_DIR'], '')   
        # Out[135]: 
        #({'r1': array([[[ 0.41377626]], [[ 0.57431281]]]),
        #  'r2': array([[[ 1.13649565]], [[ 0.72813219]]]),
        #  'r3': array([[[ 1.388739  ]], [[ 0.28956418]]]),
        #  'r4': array([[[ 1.24585202]], [[ 0.78404044]]])},
        # {'MEASINFO': {'Ref': 'B1950_VLA', 'type': 'direction'},
        #  'QuantumUnits': array(['rad', 'rad'], dtype='|S4')})

        if not fldids:
            fldids = range(len(allflds))
        else:
            fldnams = []
            if isinstance(fldids, str):
                fldnams = re.split(r"(,|\s)+", fldids) # Is TaQL supportable?

                # I can't believe that even re.split() in Python is this stupid.
                fldnams = [f for f in fldnams if f != ' ']

            # Separate selected field names from selected field numbers.
            fldnums = []
            for i in xrange(len(fldnams)):
                try:
                    fldid = int(fldnams[i])
                    fldnums.append(fldid)
                    fldnams.pop(i)
                except:
                    pass

            fldids  = [-1 for i in range(len(allflds))]
            for i in xrange(len(allflds)):
                if (i in fldnums) or (allflds[i] in fldnams):
                    fldids[i] = i

        casalog.post("fldid vector: " + str(fldids), 'DEBUG1' )

        # We don't really want to create or use scratch columns, but im will
        # open vis read-only unless usescratch is set to True.
        im.open(vis, usescratch=True)
        
        im.calcuvw(fldids, refcode, reuse)
        im.close()
    except Exception, instance:
        casalog.post('*** Error *** ' + str(instance), 'SEVERE')
    return
