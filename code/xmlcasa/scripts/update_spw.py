#!/usr/bin/python
# The above is for running the doctest, not normal use.

"""
A set of functions for manipulating spw:chan selection strings.

If this is run from a shell (i.e. not in casapy), doctest will be used to run
several unit tests from the doc strings, including the one below:

Example:
>>> from update_spw import update_spw
>>> update_spw('0~2,5', None)[0]
'0~2,3'
>>> update_spw('0~2,5', None)[1]['5']  # doctest warning!  dicts don't always print out in the same order!
'3'
"""

import copy
#from taskinit import mstool
from casac import homefinder

def update_spw(spw, spwmap=None):
    """
    Given an spw:chan selection string, return what it should be after the spws
    have been remapped (i.e. by split), and a map from input to output spws
    (spwmap).  It does not change spw OR the *channels* part of the output spw
    string!  (See update_spwchan)

    If given, spwmap will be used as a dictionary from (string) input spw to
    (string) output spws.  Otherwise it will be freshly calculated.  Supplying
    spwmap doesn't just save work: it is also necessary for chaining
    update_spw() calls when the first selection includes more spws than the
    subsequent one(s).  HOWEVER, if given, spwmap must have slots for all the
    spws that will appear in the output MS, i.e. it can't be grown once made.

    Examples:
    >>> from update_spw import update_spw
    >>> myfitspw, spws = update_spw('0~3,5;6:1~7;11~13', None)
    >>> myfitspw
    '0~3,4;5:1~7;11~13'
    >>> myspw = update_spw('1,5;6:8~10', spws)[0]
    >>> myspw   # not '0,1,2:8~10'
    '1,4;5:8~10'
    >>> update_spw('0~3,5;6:1~7;11~13,7~9:0~3,11,7~8:6~8', None)[0]
    '0~3,4;5:1~7;11~13,6~8:0~3,9,6~7:6~8'
    
    # Let's say we want updates of both fitspw and spw, but fitspw and spw
    # are disjoint (in spws).
    >>> fitspw = '1~10:5~122,15~22:5~122'
    >>> spw = '6~14'
    
    #  Initialize spwmap with the union of them.
    >>> spwmap = update_spw(join_spws(fitspw, spw), None)[1]
    
    >>> myfitspw = update_spw(fitspw, spwmap)[0]
    >>> myfitspw
    '0~9:5~122,14~21:5~122'
    >>> myspw = update_spw(spw, spwmap)[0]
    >>> myspw
    '5~13'
    >>> myspw = update_spw('0,1,3;5~8:20~30;44~50^2', None)[0]
    >>> myspw
    '0,1,2;3~6:20~30;44~50^2'
    """
    # Blank is valid.  Blank is good.
    if not spw:
        return '', {}

    # A list of [spw, chan] pairs.  The chan parts will not be changed.
    spwchans = [] 

    make_spwmap = False
    if not spwmap:
        spwmap = {}
        make_spwmap = True
        spws = set([])

    # Because ; means different things when it separates spws and channel
    # ranges, I can't think of a better way to construct spwchans than an
    # explicit state machine.  (But $spws_alone =~ s/:[^,]+//g;)
    inspw = True     # until a : is encountered.
    spwgrp = ''
    chagrp = ''

    def store_spwchan(sstr, cstr):
        spwchans.append([sstr, cstr])
        if make_spwmap:
            for sgrp in sstr.split(';'):
                if sgrp.find('~') > -1:
                    start, end = map(int, sgrp.split('~'))
                    spws.update(range(start, end + 1))
                else:
                    spws.add(int(sgrp))        
    
    for c in spw:
        if c == ',':               # Start new [spw, chan] pair.
            # Store old one.
            store_spwchan(spwgrp, chagrp)

            # Initialize new one.
            spwgrp = ''
            chagrp = ''
            inspw = True
        elif c == ':':
            inspw = False
        elif inspw:
            spwgrp += c
        else:
            chagrp += c
    # Store final [spw, chan] pair.
    store_spwchan(spwgrp, chagrp)

    # print "spwchans =", spwchans
    # print "spws =", spws
        
    # Update spw (+ fitspw)
    if make_spwmap:
        i = 0
        for s in sorted(spws):
            spwmap[str(s)] = str(i)
            i += 1
    outstr = ''
    for sc in spwchans:
        sgrps = sc[0].split(';')
        for sind in xrange(len(sgrps)):
            sgrp = sgrps[sind]
            if sgrp.find('~') > -1:
                start, end = sgrp.split('~')
                sgrps[sind] = spwmap[start] + '~' + spwmap[end]
            else:
                sgrps[sind] = spwmap[sgrp]
        outstr += ';'.join(sgrps)
        if sc[1]:
            outstr += ':' + sc[1]
        outstr += ','

    return outstr.rstrip(','), spwmap # discard final comma.

def spwchan_to_ranges(vis, spw):
    """
    Returns the spw:chan selection string spw as a dict of channel selection
    ranges for vis, keyed by spectral window ID.

    The ranges are stored as tuples of (start channel,
                                        end channel (inclusive!),
                                        step).

    Note that '' returns an empty set!  Use '*' to select everything!

    Example:
    >>> from update_spw import spwchan_to_ranges
    >>> selranges = spwchan_to_ranges('uid___A002_X1acc4e_X1e7.ms', '7:10~20^2;40~55')
    ValueError: spwchan_to_ranges() does not support multiple channel ranges per spw.
    >>> selranges = spwchan_to_ranges('uid___A002_X1acc4e_X1e7.ms', '0~1:1~3,5;7:10~20^2')
    >>> selranges
    {0: (1,  3,  1), 1: (1,  3,  1), 5: (10, 20,  2), 7: (10, 20,  2)}
    """
    mstool = homefinder.find_home_by_name('msHome')
    myms = mstool.create()
    selarr = myms.msseltoindex(vis, spw=spw)['channel']
    nspw = selarr.shape[0]
    selranges = {}
    for s in xrange(nspw):
        if selranges.has_key(selarr[s][0]):
            raise ValueError, 'spwchan_to_ranges() does not support multiple channel ranges per spw.'
        selranges[selarr[s][0]] = tuple(selarr[s][1:])
    return selranges

def update_spwchan(vis, sch0, sch1):
    """
    Given an spw:chan selection string sch1, return what it must be changed to
    to get the same result if used with the output of split(vis, spw=sch0).

    Examples:
    >>> from update_spw import update_spwchan
    >>> newspw = update_spwchan('uid___A002_X1acc4e_X1e7.ms', '0~1:1~3,5;7:10~20^2;40~55', '0~1:1~3,5;7:10~20^2;40~55')
    >>> newspw
    ''
    >>> newspw = update_spwchan('uid___A002_X1acc4e_X1e7.ms', '0~1:1~3,5;7:10~20^2;40~55', '0~1:2,3,5;7:12~18^2')
    ValueError: update_spwchan(vis, sch0, sch1) does not support multiple channel ranges per spw in sch0.
    >>> newspw = update_spwchan('uid___A002_X1acc4e_X1e7.ms', '0~1:1~3,5;7:10~20^2', '0~1:2~3,5;7:12~18^2')
    >>> newspw
    '0~1:1~2,2;3:1~4'
    >>> newspw = update_spwchan('uid___A002_X1acc4e_X1e7.ms', '7', '3')
    ValueError: '3' is not a subset of '7'.
    >>> newspw = update_spwchan('uid___A002_X1acc4e_X1e7.ms', '7:10~20^2', '7:12~18^3')
    ValueError: '7:12~18^3' is not a subset of '7:10~20^2'.
    
    # Let's say we want updates of both fitspw and spw, but fitspw and spw
    # are disjoint (in spws).
    >>> fitspw = '1~10:5~122,15~22:5~122'
    >>> spw = '6~14'
    
    #  Initialize spwchanmap with the union of them.
    >>> spwchanmap = update_spwchan(join_spws(fitspw, spw), None)[1]
    
    >>> myfitspw = update_spwchan(fitspw, spwchanmap)[0]
    >>> myfitspw
    '0~9:5~122,14~21:5~122'
    >>> myspw = update_spwchan(spw, spwchanmap)[0]
    >>> myspw
    '5~13'
    >>> myspw = update_spwchan('0,1,3;5~8:20~30;44~50^2', None)[0]
    >>> myspw
    '0,1,2;3~6:0~10;24~30^2'
    """
    # Convert '' to 'select everything'.
    if not sch0:
        sch0 = '*'
    if not sch1:
        sch1 = '*'
        
    if sch1 == sch0:
        return ''
    elif sch0 == '*':
        return sch1

    sch0ranges = spwchan_to_ranges(vis, sch0)
    sch1ranges = spwchan_to_ranges(vis, sch1)

    allchans = spwchan_to_ranges(vis, '*')

    outranges = {}
    for s in sch1ranges:
        try:
            sch0range = sch0ranges[s]
            sch1range = sch1ranges[s]
            if (sch1range[0] < sch0range[0]) or (sch1range[1] > sch0range[1]):
                raise ValueError
            if sch0range[2] > 1:  # There are gaps, so check more closely.
                s0list = list(sch0range)
                s1list = list(sch1range)
                s0list[1] += 1  # MSSelect is inclusive, but python's
                s1list[1] += 1  # range() excludes the end value.
                s0 = set(range(*s0list))
                s1 = set(range(*s1list))
                if s1.difference(s0):
                    raise ValueError
            outranges[s] = ((sch1range[0] -
                             sch0range[0]) / sch0range[2],
                            (sch1range[1] -
                             sch0range[0]) / sch0range[2],
                            sch1range[2] / sch0range[2])
        except:
            raise ValueError, "'%s' is not a subset of '%s'." % (sch1, sch0)
        
    s0spws = sch0ranges.keys()
    s0spws.sort()
    s1spws = sch1ranges.keys()
    s1spws.sort()

    in_to_out_spwmap = {}
    outspw = 0
    for s in s0spws:
        in_to_out_spwmap[s] = outspw
        outspw += 1

    def ranges_to_grp(startspw, startchans, s1prevspw):
        grp = str(in_to_out_spwmap[startspw])
        if s1prevspw != startspw:
            grp += ';' + str(in_to_out_spwmap[s1prevspw])
        if startchans != allchans[startspw]:
            grp += ':' + str(startchans[0])
            if startchans[1] > startchans[0]:
                grp += '~' + str(startchans[1])
                if startchans[2] != 1:
                    grp += '^' + str(startchans[2])
        return grp

    newgroups = []
    startspw = s1spws[0]
    startchans = outranges[s1spws[0]]
    s1prevspw = startspw
    for s1currspw in s1spws:
        if (s1currspw == s1spws[-1] or
            (in_to_out_spwmap[s1currspw] > (in_to_out_spwmap[s1prevspw] + 1)) or
            (outranges[s1currspw] != startchans)):
            # Finish the old group.
            if s1currspw == s1spws[-1]:
                newgroups.append(ranges_to_grp(startspw, startchans, s1currspw))
            else:
                newgroups.append(ranges_to_grp(startspw, startchans, s1prevspw))

            # Start a new group.
            startspw = s1currspw
            startchans = outranges[s1currspw]
        s1prevspw = s1currspw

    return ','.join(newgroups)

def expand_tilde(tstr):
    """
    Expands a string like '8~11' to [8, 9, 10, 11].
    Returns '*' if tstr is ''!

    Examples:
    >>> from update_spw import expand_tilde
    >>> expand_tilde('8~11')
    [8, 9, 10, 11]
    >>> expand_tilde(None)
    '*'
    """
    if not tstr:
        return '*'
    if tstr.find('~') > -1:
        start, end = map(int, tstr.split('~'))
    else:
        start = int(tstr)
        end = start
    return range(start, end + 1)

def spw_to_dict(spw, spwdict):
    """
    Expand an spw:chan string to {s0: [s0chans], s1: [s1chans, ...], ...}
    where s0, s1, ... are integers for _each_ selected spw, and s0chans is a
    set of selected chans (as integers) for s0.  '' instead of a channel set
    means that all of the channels are selected.

    The spw:chan dict is unioned with spwdict.

    Returning an empty dict means everything should be selected (i.e. spw = '').

    Examples:
    >>> from update_spw import spw_to_dict
    >>> spw_to_dict('', {})
    {}
    >>> spw_to_dict('6~8:2~5', {})[6]
    set([2, 3, 4, 5])
    >>> spw_to_dict('6~8:2~5', {})[8]
    set([2, 3, 4, 5])
    >>> spw_to_dict('6~8:2~5', {6: ''})[6]
    ''
    >>> spw_to_dict('6~8:2~5', {6: '', 7: set([1, 7])})[7]
    set([1, 2, 3, 4, 5, 7])
    >>> spw_to_dict('7', {6: '', 7: set([1, 7])})[7]
    ''
    >>> spw_to_dict('7:123~127;233~267', {6: '', 7: set([1, 7])})[7]  # Multiple chan ranges
    ''
    >>> spw_to_dict('5,7:123~127;233~267', {6: '', 7: set([1, 7])})[5]
    ''
    >>> spw_to_dict('5:3~5,7:123~127;233~267', {6: '', 7: set([1, 7])})[5]
    set([3, 4, 5])
    """
    if not spw:
        return {}

    myspwdict = copy.deepcopy(spwdict)

    # Because ; means different things when it separates spws and channel
    # ranges, I can't think of a better way to construct myspwdict than an
    # explicit state machine.  (But $spws_alone =~ s/:[^,]+//g;)
    inspw = True    # Must start with an spw.
    spwgrp = ''
    chagrp = ''

    def enter_ranges(spwg, chag):
        spwrange = expand_tilde(spwgrp)
        if spwrange == '*':  # This shouldn't happen.
            return {}
        if chagrp.find(';') > -1:            # split can't yet handle multiple
            charange = '*'                   # channel ranges, so select everything.
        else:
            charange = expand_tilde(chagrp)
        for s in spwrange:
            if charange == '*':
                myspwdict[s] = ''
            else:
                if not myspwdict.has_key(s):
                    myspwdict[s] = set([])
                if myspwdict[s] != '':
                    myspwdict[s].update(charange)        

    for c in spw:
        if c == ',' or (inspw and c == ';'):  # Start new [spw, chan] pair.
            # Store old one.
            enter_ranges(spwgrp, chagrp)

            # Initialize new one.
            spwgrp = ''
            chagrp = ''
            inspw = True
        elif c == ':':
            inspw = False
        elif inspw:
            spwgrp += c
        else:
            chagrp += c

    # Store final [spw, chan] pair.
    enter_ranges(spwgrp, chagrp)
    return myspwdict

def join_spws(spw1, spw2):
    """
    Returns the union of spw selection strings spw1 and spw2.  For any spws
    that have > 1 channel range, the entire spw will be selected.

    Examples:
    >>> from update_spw import join_spws
    >>> join_spws('0~2:3~5,3:9~13', '')
    ''
    >>> join_spws('0~2:3~5,3:9~13', '1~3:4~7')
    '0:3~5,1~2:3~7,3'
    >>> join_spws('1~10:5~122,15~22:5~122', '1~10:5~122,15~22:5~122')
    '1~10:5~122,15~22:5~122'
    >>> join_spws('', '')
    ''
    """
    if not spw1 or not spw2:
        return ''
        
    spwdict = spw_to_dict(spw1, {})
    spwdict = spw_to_dict(spw2, spwdict)

    res = ''
    # Convert channel sets to strings
    for s in spwdict:
        cstr = ''
        if isinstance(spwdict[s], set) and len(spwdict[s]) > 0:
            clist = list(spwdict[s])
            clist.sort()
            cstr = str(clist[0])
            laststart = 0
            for i in xrange(1, len(clist)):
                if clist[i] == clist[laststart] + 1:
                    cstr += '~'
                elif clist[i] > clist[i - 1] + 1:
                    cstr = ''                      # Multiple channel ranges are not yet supported,
                    break                          # just select everything in the spw.
            if len(cstr) > 0 and cstr[-1] == '~':
                cstr += str(clist[-1])
        spwdict[s] = cstr

    # If consecutive spws have the same channel selection, merge them.
    slist = spwdict.keys()
    slist.sort()
    res = str(slist[0])
    laststart = 0
    for i in xrange(1, len(slist)):
        # If consecutive spws have the same channel list,
        if slist[i] == slist[i - 1] + 1 and spwdict[slist[i]] == spwdict[slist[i - 1]]:
            if slist[i] == slist[laststart] + 1:
                res += '~'  # Continue the spw range.
        else:           # Terminate it and start a new one.
            if res[-1] == '~':          # if start != end
                res += str(slist[i - 1])
            if spwdict[slist[i - 1]] != '':          # Add channel range, if any.
                res += ':' + spwdict[slist[i - 1]]
            res += ',' + str(slist[i])
            laststart = i
    if res[-1] == '~':               # Finish the last range if it is dangling.
        res += str(slist[-1])
    if spwdict[slist[-1]] != '':          # Add channel range, if any.
        res += ':' + spwdict[slist[i - 1]]
    return res

def intersect_spws(spw1, spw2):
    """
    Almost the opposite of join_spws(), this returns the list of spws that the
    spw:chan selection strings spw1 and spw2 have in common.  Unlike join_spws(),
    channel ranges are ignored.  '' in the input counts as 'select everything',
    so the intersection of '' with anything is anything.  If the intersection
    really is everything, '' is returned instead of a set.

    Examples:
    >>> from update_spw import intersect_spws
    >>> intersect_spws('0~2:3~5,3:9~13', '')
    set([0, 1, 2, 3])
    >>> intersect_spws('0~2:3~5,3:9~13', '0~2:7~9,5')
    set([0, 1, 2])
    >>> intersect_spws('0~2:3~5;10~13,3:9~13', '0~2:7~9,5')
    set([0, 1, 2])
    >>> intersect_spws('0~2:3~5,3:9~13', '10~12:7~9,5')  # Empty set
    set([])
    >>> intersect_spws('', '')                           # Everything
    ''
    """
    if spw1 == '':
        if spw2 == '':
            return ''     # intersection('', '') = ''
        else:             # intersection('', spw2) = spw2
            return set(spw_to_dict(spw2, {}).keys()) # Just the spws, no chan ranges
    elif spw2 == '':      # intersection('', spw1) = spw1
        return set(spw_to_dict(spw1, {}).keys())     # Just the spws, no chan ranges
    else:
        spwset1 = set(spw_to_dict(spw1, {}).keys())  # spws are the keys, chan
        spwset2 = set(spw_to_dict(spw2, {}).keys())  # ranges are the values.
        return spwset1.intersection(spwset2)

def subtract_spws(spw1, spw2):
    """
    Returns the set of spws of spw selection string spw1 that are not in spw2.
    Like intersect_spws(), this intentionally ignores channel ranges.  It
    assumes that spw1 and spw2 refer to the same MS (this only matters for '').
    subtract_spws('', '0~5') is a tough case: it is impossible to know whether
    '' is equivalent to '0~5' without reading the MS's SPECTRAL_WINDOW
    subtable, so it returns 'UNKNOWN'.

    Examples:
    >>> from update_spw import subtract_spws
    >>> subtract_spws('0~2:3~5,3:9~13', '') # Anything - Everything
    set([])
    >>> subtract_spws('0~2:3~5,3:9~13', '0~2:7~9,5')
    set([3])
    >>> subtract_spws('', '0~2:7~9,5') # Everything - Something
    'UNKNOWN'
    >>> subtract_spws('0~2,3:9~13', '4~7:7')  # Something - Something Else
    set([0, 1, 2, 3])
    >>> subtract_spws('', '')              # Everything - Everything
    set([])
    """
    if spw1 == '':
        if spw2 == '':
            return set([])
        else:
            return 'UNKNOWN'
    elif spw2 == '':
        return set([])
    else:
        spwset1 = set(spw_to_dict(spw1, {}).keys())  # spws are the keys, chan
        spwset2 = set(spw_to_dict(spw2, {}).keys())  # ranges are the values.
        return spwset1.difference(spwset2)
    
if __name__ == '__main__':
    import doctest, sys
    doctest.testmod(verbose=True)
