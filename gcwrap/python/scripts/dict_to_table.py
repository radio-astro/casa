import numpy
import os
import shutil
import tempfile
from taskinit import tbtool, me

def dict_to_table(indict, tablepath, kwkeys=[], colkeys=[], info=None, keepcolorder=False):
    """
    Converts a dictionary to a CASA table, and attempts to
    save it to tablepath.  Returns whether or not it was successful.

    kwkeys is a list of keys in dict that should be treated as table keywords,
    and colkeys is a list of keys to be treated as table columns.  If a key in
    indict is not in either kwkeys or colkeys, it will be appended to colkeys
    if it refers to a list, array, or specially formed dict with the right
    number of rows, or kwkeys otherwise.

    "Specially formed dict" means a python dictionary with the right keys to
    provide a comment and/or keywords to specify a (measure) frame or
    (quantity) unit for the column.

    The number of rows is set by the first column.  The order of the columns is
    the order of colkeys, followed by the remaining columns in alphabetical
    order.

    Example:
    mydict = {'delta': [1.2866, 1.2957, 1.3047],
              'obs_code': ['*', 'U', 't'],
              'date': {'m0': {'unit': 'd',
                              'value': [55317.0, 55318.0, 55319.0]},
                       'refer': 'UT1',
                       'type': 'epoch'},
              'phang': {'comment': 'phase angle',
                        'data': {'unit': 'deg',
                                 'value': array([37.30, 37.33, 37.36])}}}
                                 
    # Produces a table with, in order, a measure column (date), two bare
    # columns (delta and obs_code), and a commented quantity column (phang).
    # The comment goes in the 'comment' field of the column description.
    # Measure and straight array columns can also be described by using a
    # {'comment': (description), 'data': (measure, quantity, numpy.array or
    # list)} dict.
    dict_to_table(mydict, 'd_vs_phang.tab')

    TODO: detect non-float data types, including array cells.
    """
    nrows = 0
    dkeys = indict.keys()
    keywords = []
    cols = []

    def get_bare_col(col):
        """
        Given a col that could be a bare column (list or array), or measure or
        quantity containing a bare column, return the bare column.
        """
        barecol = col
        if hasattr(barecol, 'has_key'):
            if barecol.has_key('comment'):
                barecol = barecol.get('data')
            if type(barecol)==dict and me.ismeasure(barecol):
                barecol = barecol['m0']
            # if qa.isquantity(data) can't be trusted.
            if hasattr(barecol, 'has_key') and barecol.has_key('unit') and barecol.has_key('value'):
                barecol = barecol['value']
        return barecol
        
    # Divvy up the known keywords and columns, if present, preserving the
    # requested order.
    for kw in kwkeys:
        if kw in dkeys:
            # Take kw out of dkeys and put it in keywords.
            keywords.append(dkeys.pop(dkeys.index(kw)))
    for c in colkeys:
        if c in dkeys:
            cols.append(dkeys.pop(dkeys.index(c)))
            if nrows == 0:
                nrows = len(get_bare_col(indict[c]))
                print "Got nrows =", nrows, "from", c

    # Go through what's left of dkeys and assign them to either keywords or
    # cols.
    dkeys.sort()
    for d in dkeys:
        used_as_col = False
        colcand = get_bare_col(indict[d])
        # Treat it as a column if it has the right number of rows.
        if type(colcand) in (list, numpy.ndarray):
            if nrows == 0:
                nrows = len(colcand)
            if len(colcand) == nrows:
                cols.append(d)
                used_as_col = True
        if not used_as_col:
            keywords.append(d)

    # Make the table's description.
    tabdesc = {}
    # Initialize the column descriptor with defaults (these come from
    # data/ephemerides/DE200, but I replaced IncrementalStMan with StandardStMan).
    coldesc = {'comment': '',
               'dataManagerGroup': '',
               'dataManagerType': 'StandardStMan',
               'maxlen': 0,
               'option': 0,
               'valueType': 'double'} # Use double (not float!) for columns
                                      # that will be read by MeasIERS.
    for c in cols:
        #print "Setting coldesc for", c
        data = indict[c]  # Place to find the valueType.
        
        if hasattr(data, 'has_key'):
            #print "comment =", data.get('comment', '')
            coldesc['comment'] = data.get('comment', '')
            
        data = get_bare_col(data)
        valtype = str(type(data[0]))[7:-2]
        if valtype == 'str':
            valtype = 'string'
        valtype = valtype.replace('64', '')      # Table uses 'float', not 'float64'.
        valtype = valtype.replace('numpy.', '')  # or 'numpy.float'.

        # Use double (not float!) for columns that will be read by MeasIERS.
        if valtype == 'float':
            valtype = 'double'
            
        coldesc['valueType'] = valtype

        tabdesc[c] = coldesc.copy()

    # Since tables are directories, it saves a lot of grief if we first check
    # whether the table exists and is under svn control.
    svndir = None
    if os.path.isdir(tablepath):
        if os.path.isdir(tablepath + '/.svn'):
            # tempfile is liable to use /tmp, which can be too small and/or slow.
            # Use the directory that tablepath is in, since we know the user
            # approves of writing to it.
            workingdir = os.path.abspath(os.path.dirname(tablepath.rstrip('/')))

            svndir = tempfile.mkdtemp(dir=workingdir)
            shutil.move(tablepath + '/.svn', svndir)
        print "Removing %s directory" % tablepath
        shutil.rmtree(tablepath)

    # Create and fill the table.
    retval = True
    try:
        mytb = tbtool()
        tmpfname='_tmp_fake.dat'
        if keepcolorder:
            # try to keep order of cols 
            # Ugly, but since tb.create() cannot accept odered dictionary
            # for tabledesc, I cannot find any other way to keep column order.
            # * comment for each column will not be filled
            f = open(tmpfname,'w')
            zarr=numpy.zeros(len(cols))
            szarr=str(zarr.tolist())
            szarr=szarr.replace('[','')
            szarr=szarr.replace(']','')
            szarr=szarr.replace(',','')
            scollist=''
            sdtypes='' 
            for c in cols:
                scollist+=c+' '   
                vt=tabdesc[c]['valueType']
                if vt=='string':
                   sdtypes+='A '    
                elif vt=='integer':
                   sdtypes+='I '
                elif vt=='double':
                   sdtypes+='D '
                elif vt=='float':
                   sdtypes+='R '
            f.write(scollist+'\n')
            f.write(sdtypes+'\n')
            f.write(szarr)
            f.close()
            mytb.fromascii(tablepath,tmpfname,sep=' ')     
            # close and re-open since tb.fromascii(nomodify=False) has not
            # implemented yet
            mytb.close() 
            os.remove(tmpfname) 
            mytb.open(tablepath, nomodify=False)
            mytb.removerows(0)
        else: 
            mytb.create(tablepath, tabdesc)
        if type(info) == dict:
            mytb.putinfo(info)
        mytb.addrows(nrows)     # Must be done before putting the columns.
    except Exception, e:
        print "Error", e, "trying to create", tablepath
        retval = False
    for c in cols:
        try:
            #print "tabdesc[%s] =" % c, tabdesc[c]
            data = indict[c]  # Note the trickle-down nature below.
            if hasattr(indict[c], 'has_key') and indict[c].has_key('comment'):
                data = data['data']
            if type(data)==dict and me.ismeasure(data):
                mytb.putcolkeyword(c, 'MEASINFO', {'Ref': data['refer'],
                                                   'type': data['type']})
                data = data['m0']   # = quantity         
            # if qa.isquantity(data) can't be trusted.
            if hasattr(data, 'has_key') and data.has_key('unit') and data.has_key('value'):
                mytb.putcolkeyword(c, 'QuantumUnits',
                                 numpy.array([data['unit']]))
                data = data['value']
            mytb.putcol(c, data)
        except Exception, e:
            print "Error", e, "trying to put column", c, "in", tablepath
            print "data[0] =", data[0]
            print "tabdesc[c] =", tabdesc[c]
            retval = False
    for k in keywords:
        try:
            mytb.putkeyword(k, indict[k])
        except Exception, e:
            print "Error", e, "trying to put keyword", k, "in", tablepath
            retval = False
    mytb.close()

    if svndir:
        shutil.move(svndir, tablepath + '/.svn')
    return retval
