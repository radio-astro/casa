#!/usr/bin/python

def cmptabs(tab1, tab2):
    """
    Returns 1 if any of the columns in tab1 differ from the corresponding ones
    in tab2, 0 otherwise.
    """
    tabs = (tab1, tab2)
    tb.open(tab1)
    colnames = set(tb.colnames())
    nrows1 = tb.nrows()
    tb.close()
    tb.open(tab2)
    colnames2 = set(tb.colnames())
    nrows2 = tb.nrows()
    tb.close()

    if colnames2 !=  colnames:
        print tab1, "has cols", str(colnames)
        print tab2, "has cols", str(colnames2)
        return 1

    del colnames2

    # After this point, both tables are assumed to have some rows, so check now.
    if 0 in (nrows1, nrows2):
        if nrows1 == nrows2:
            return 0
        else:
            return 1
    
    for col in colnames:
        cols = [-1, -1]      # Default to a semi-invalid code.
        for i in xrange(2):
            tb.open(tabs[i])
            try:
                if tb.iscelldefined(col):    # col in colnames does not
                    cols[i] = tb.getcol(col) # imply col is valid.
            except:
                print "Error getting", tabs[i], " col =", col
                return 1
            finally:
                tb.close()
        diff = cols[0] != cols[1]
        if hasattr(diff, 'any'):
            diff = diff.any()
        if diff:
            print col, "differs"
            return 1
    return 0

def cmpmses(ms1, ms2, subtabs_to_check=['ANTENNA', 'DATA_DESCRIPTION', 'FEED',
                                        'FLAG_CMD', 'FIELD',
                                        'OBSERVATION', 'POINTING',
                                        'POLARIZATION', 'PROCESSOR',
                                        'SPECTRAL_WINDOW', 'STATE',
                                        'SOURCE', 'WEATHER'],
            kws_to_check=['MS_VERSION']):
    """
    Returns 1 if any of the columns, listed subtables, or listed keywords in
    ms1 differ from the corresponding ones in ms2, 0 otherwise.
    """
    mses = (ms1, ms2)
    kws = []
    for i in xrange(2):
        tb.open(mses[i])
        kws.append(tb.keywordnames())
        tb.close()
    if kws[0] != kws[1]:
        print "They have different keyword sets."
        return 1
    for kw in kws_to_check:
        kwvals = ['', '']
        for i in xrange(2):
            if kw in kws[i]:
                tb.open(mses[i])
                kwvals[i] = tb.getkeyword(kw)
                tb.close()
        if kwvals[0] != kwvals[1]:
            print kw, "differs"
            return 1
    for subtab in subtabs_to_check:
        if subtab in kws[0]:
            if cmptabs(ms1 + '/' + subtab, ms2 + '/' + subtab):
                return 1
    return cmptabs(ms1, ms2)
