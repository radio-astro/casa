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

import copy, re

def update_spw(spw, spwmap=None):
    """
    Given an spw:chan selection string, return what it should be after
    the spws have been remapped (i.e. by split).  It does not change spw.

    If given, spwmap will be used as a dictionary from (string) input spw to
    (string) output spws.  Otherwise it will be freshly calculated.
    Supplying spwmap doesn't just save work: it is also necessary for
    chaining update_spw() calls when the first selection includes more spws
    than the subsequent one(s).

    Examples:
    >>> from update_spw import update_spw
    >>> myfitspw, spws = update_spw('0~3,5;6:1~7;11~13', None)
    >>> myfitspw
    '0~3,4,5:1~7;11~13'
    >>> myspw = update_spw('1,5;6:8~10', spws)[0]
    >>> myspw   # not '0,1,2:8~10'
    '1,4,5:8~10'
    >>> update_spw('0~3,5;6:1~7;11~13,7~9:0~3,11,7~8:6~8', None)[0]
    '0~3,4,5:1~7;11~13,6~8:0~3,9,6~7:6~8'
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
    inspw = True    # Must start with an spw.
    spwgrp = ''
    chagrp = ''
    
    for c in spw:
        if c == ',' or (inspw and c == ';'):  # Start new [spw, chan] pair.
            # Store old one.
            spwchans.append([spwgrp, chagrp])
            if make_spwmap:
                if spwgrp.find('~') > -1:
                    start, end = map(int, spwgrp.split('~'))
                    spws.update(range(start, end + 1))
                else:
                    spws.add(int(spwgrp))

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
    spwchans.append([spwgrp, chagrp])
    if make_spwmap:
        if spwgrp.find('~') > -1:
            start, end = map(int, spwgrp.split('~'))
            spws.update(range(start, end + 1))
        else:
            spws.add(int(spwgrp))

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
        if sc[0].find('~') > -1:
            start, end = sc[0].split('~')
            outstr += spwmap[start] + '~' + spwmap[end]
        else:
            outstr += spwmap[sc[0]]
        if sc[1]:
            outstr += ':' + sc[1]
        outstr += ','

    return outstr.rstrip(','), spwmap # discard final comma.

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
