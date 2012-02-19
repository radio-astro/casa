#! /usr/bin/env python
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
import os
#from taskinit import mstool
from casac import *
from taskinit import ms

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
    selarr = ms.msseltoindex(vis, spw=spw)['channel']
    nspw = selarr.shape[0]
    selranges = {}
    for s in xrange(nspw):
        if selranges.has_key(selarr[s][0]):
            raise ValueError, 'spwchan_to_ranges() does not support multiple channel ranges per spw.'
        selranges[selarr[s][0]] = tuple(selarr[s][1:])
    return selranges

def spwchan_to_sets(vis, spw):
    """
    Returns the spw:chan selection string spw as a dict of sets of selected
    channels for vis, keyed by spectral window ID.

    Note that '' returns an empty set!  Use '*' to select everything!

    Example (16.ms has spws 0 and 1 with 16 chans each):
    >>> from update_spw import spwchan_to_sets
    >>> vis = casa['dirs']['data'] + '/regression/unittest/split/unordered_polspw.ms'
    >>> spwchan_to_sets(vis, '0:0')
    {0: set([0])}
    >>> selsets = spwchan_to_sets(vis, '1:1~3;5~9^2,9') # 9 is a bogus spw.
    >>> selsets
    {1: [1, 2, 3, 5, 7, 9]}
    >>> spwchan_to_sets(vis, '1:1~3;5~9^2,8')
    {1: set([1, 2, 3, 5, 7, 9]), 8: set([0])}
    >>> spwchan_to_sets(vis, '')
    {}
    """
    if not spw:        # ms.msseltoindex(vis, spw='')['channel'] returns a
        return {}      # different kind of empty array.  Skip it.

    # Currently distinguishing whether or not vis is a valid MS from whether it
    # just doesn't have all the channels in spw is a bit crude.  Sanjay is
    # working on adding some flexibility to ms.msseltoindex.
    if not os.path.isdir(vis):
        raise ValueError, str(vis) + ' is not a valid MS.'
        
    sets = {}
    try:
        scharr = ms.msseltoindex(vis, spw=spw)['channel']
        for scr in scharr:
            if not sets.has_key(scr[0]):
                sets[scr[0]] = set([])

            # scr[2] is the last selected channel.  Bump it up for range().
            scr[2] += 1
            sets[scr[0]].update(range(*scr[1:]))
    except:
        # spw includes channels that aren't in vis, so it needs to be trimmed
        # down to make ms.msseltoindex happy.
        allrec = ms.msseltoindex(vis, spw='*')
        #print "Trimming", spw
        spwd = spw_to_dict(spw, {}, False)
        for s in spwd:
            if s in allrec['spw']:
                endchan = allrec['channel'][s, 2]
                if not sets.has_key(s):
                    sets[s] = set([])
                if spwd[s] == '':
                    # We need to get the spw's # of channels without using
                    # ms.msseltoindex.
                    mytb = casac.table()
                    mytb.open(vis + '/SPECTRAL_WINDOW')
                    spwd[s] = range(mytb.getcell('NUM_CHAN', s))
                    mytb.close()
                sets[s].update([c for c in spwd[s] if c <= endchan])
    return sets

def set_to_chanstr(chanset, totnchan=None):
    """
    Essentially the reverse of expand_tilde.  Given a set or list of integers
    chanset, returns the corresponding string form.  It will not use non-unity
    steps (^) if multiple ranges (;) are necessary, but it will use ^ if it
    helps to eliminate any ;s.

    totnchan: the total number of channels for the input spectral window, used
              to abbreviate the return string.

    It returns '' for the empty set and '*' if 

    Examples:
    >>> from update_spw import set_to_chanstr
    >>> set_to_chanstr(set([0, 1, 2, 4, 5, 6, 7, 9, 11, 13]))
    '0~2;4~7;9;11;13'
    >>> set_to_chanstr(set([7, 9, 11, 13]))
    '7~13^2'
    >>> set_to_chanstr(set([7, 9]))
    '7~9^2'
    >>> set_to_chanstr([0, 1, 2])
    '0~2'
    >>> set_to_chanstr([0, 1, 2], 3)
    '*'
    >>> set_to_chanstr([0, 1, 2, 6], 3)
    '*'
    >>> set_to_chanstr([0, 1, 2, 6])
    '0~2;6'
    >>> set_to_chanstr([1, 2, 4, 5, 6, 7, 8, 9, 10, 11], 12)
    '1~2;4~11'
    """
    if totnchan:
        mylist = [c for c in chanset if c < totnchan]
    else:
        mylist = list(chanset)

    if totnchan == len(mylist):
        return '*'

    mylist.sort()

    retstr = ''
    if len(mylist) > 1:
        # Check whether the same step can be used throughout.
        step = mylist[1] - mylist[0]
        samestep = True
        for i in xrange(2, len(mylist)):
            if mylist[i] - mylist[i - 1] != step:
                samestep = False
                break
        if samestep:
            retstr = str(mylist[0]) + '~' + str(mylist[-1])
            if step > 1:
                retstr += '^' + str(step)
        else:
            sc = mylist[0]
            oldc = sc
            retstr = str(sc)
            nc = len(mylist)
            for i in xrange(1, nc):
                cc = mylist[i]
                if (cc > oldc + 1) or (i == nc - 1):
                    if (i == nc - 1) and (cc == oldc + 1):
                        retstr += '~' + str(cc)
                    else:
                        if oldc != sc:
                            retstr += '~' + str(oldc)
                        retstr += ';' + str(cc)
                        sc = cc
                oldc = cc
    elif len(mylist) > 0:
        retstr = str(mylist[0])
    return retstr
        
def sets_to_spwchan(spwsets, nchans={}):
    """
    Returns a spw:chan selection string for a dict of sets of selected
    channels keyed by spectral window ID.

    nchans is a dict of the total number of channels keyed by spw, used to
    abbreviate the return string.

    Examples:
    >>> from update_spw import sets_to_spwchan
    >>> # Use nchans to get '1' instead of '1:0~3'.
    >>> sets_to_spwchan({1: [0, 1, 2, 3]}, {1: 4})
    '1'
    >>> sets_to_spwchan({1: set([1, 2, 3, 5, 7, 9]), 8: set([0])})
    '1:1~3;5;7;9,8:0'
    >>> sets_to_spwchan({0: set([4, 5, 6]), 1: [4, 5, 6], 2: [4, 5, 6]})
    '0~2:4~6'
    >>> sets_to_spwchan({0: [4], 1: [4], 3: [0, 1], 4: [0, 1], 7: [0, 1]}, {3: 2, 4: 2, 7: 2})
    '0~1:4,3~4,7'
    """
    # Make a list of spws for each channel selection.
    csd = {}
    for s in spwsets:
        # Convert the set of channels to a string.
        if spwsets[s]:
            cstr = set_to_chanstr(spwsets[s], nchans.get(s))

            if cstr:
                if not csd.has_key(cstr):
                    csd[cstr] = []
                csd[cstr].append(s)

    # Now convert those spw lists into strings, inverting as we go so the final
    # string can be sorted by spw:
    scd = {}
    while csd:
        cstr, slist = csd.popitem()
        slist.sort()
        startspw = slist[0]
        oldspw = startspw
        sstr = str(startspw)
        nselspw = len(slist)
        for sind in xrange(1, nselspw):
            currspw = slist[sind]
            if (currspw > oldspw + 1) or (sind == nselspw - 1):
                if currspw > oldspw + 1:
                    if oldspw != startspw:
                        sstr += '~' + str(oldspw)
                    sstr += ';' + str(currspw)
                    startspw = currspw
                else:               # The range has come to an end on the last spw.
                    sstr += '~' + str(currspw)
            oldspw = currspw
        scd[sstr] = cstr
    spwgrps = sorted(scd.keys())

    # Finally stitch together the final string.
    scstr = ''
    for sstr in spwgrps:
        scstr += sstr
        if scd[sstr] != '*':
            scstr += ':' + scd[sstr]
        scstr += ','
    return scstr.rstrip(',')

def update_spwchan(vis, sch0, sch1, truncate=False, widths={}):
    """
    Given an spw:chan selection string sch1, return what it must be changed to
    to get the same result if used with the output of split(vis, spw=sch0).

    '' is taken to mean '*' in the input but NOT the output!  For the output
    '' means sch0 and sch1 do not intersect.

    truncate: If True and sch0 only partially overlaps sch1, return the update
              of the intersection.
              If (False and sch0 does not cover sch1), OR
                 there is no intersection, raises a ValueError.

    widths is a dictionary of averaging widths (default 1) for each spw.

    Examples:
    >>> from update_spw import update_spwchan
    >>> newspw = update_spwchan('anything.ms', 'anything', 'anything')
    >>> newspw
    '*'
    >>> vis = casa['dirs']['data'] + '/regression/unittest/split/unordered_polspw.ms'
    >>> update_spwchan(vis, '0~1:1~3,5;7:10~20^2', '0~1:2~3,5;7:12~18^2')
    '0~1:1~2,2~3:1~4'
    >>> update_spwchan(vis, '7', '3')
    ValueError: '3' is not a subset of '7'.
    >>> update_spwchan(vis, '7:10~20^2', '7:12~18^3')
    ValueError: '7:12~18^3' is not a subset of '7:10~20^2'.
    >>> update_spwchan(vis, '7:10~20^2', '7:12~18^3', truncate=True)
    '0:1~4^3'
    >>> update_spwchan(vis, '7:10~20^2', '7:12~18^3', truncate=True, widths={7: 2})
    '0:0~2^2'
    """
    # Convert '' to 'select everything'.
    if not sch0:
        sch0 = '*'
    if not sch1:
        sch1 = '*'

    # Short circuits
    if sch1 == '*':
        return '*'
    elif sch1 in (sch0, '*'):
        return '*'

    sch0sets = spwchan_to_sets(vis, sch0)
    sch1sets = spwchan_to_sets(vis, sch1)

    outsets = {}
    outspw = 0
    s0spws = sorted(sch0sets.keys())
    s1spws = sorted(sch1sets.keys())
    ns0spw = len(s0spws)
    nchans = {}
    for s in s1spws:
        if s in s0spws:
            s0 = sch0sets[s]
            s1 = sch1sets[s]

            # Check for and handle (throw or dispose) channels in sch1 that aren't in
            # sch0.
            if s1.difference(s0):
                if truncate:
                    s1.intersection_update(s0)
                    if not s1:
                        raise ValueError, "'%s' does not overlap '%s'." % (sch1, sch0)
                else:
                    raise ValueError, "'%s' is not a subset of '%s'." % (sch1, sch0)

            # Adapt s1 for a post-s0 world.
            s0list = sorted(list(s0))
            s1list = sorted(list(s1))
            outchan = 0
            nc0 = len(s0list)
            for s1ind in xrange(len(s1list)):
                while (outchan < nc0) and (s0list[outchan] < s1list[s1ind]):
                    outchan += 1
                if outchan == nc0:  # Shouldn't happen
                    outchan -= 1
                s1list[s1ind] = outchan / widths.get(s, 1)

            # Determine outspw.
            while (outspw < ns0spw) and (s0spws[outspw] < s):
                outspw += 1
            if outspw == ns0spw:  # Shouldn't happen
                outspw -= 1

            outsets[outspw] = set(s1list)

            # Get the number of channels per spw that are selected by s0.
            nchans[outspw] = len(s0)
        elif not truncate:
            raise ValueError, str(s) + ' is not a selected spw of ' + sch0

    return sets_to_spwchan(outsets, nchans)

def expand_tilde(tstr, conv_multiranges=False):
    """
    Expands a string like '8~11' to [8, 9, 10, 11].
    Returns '*' if tstr is ''!

    conv_multiranges: If True, '*' will be returned if tstr contains ';'.
                      (split can't yet handle multiple channel ranges per spw.)

    Examples:
    >>> from update_spw import expand_tilde
    >>> expand_tilde('8~11')
    [8, 9, 10, 11]
    >>> expand_tilde(None)
    '*'
    >>> expand_tilde('3~7^2;9~11')
    [3, 5, 7, 9, 10, 11]
    >>> expand_tilde('3~7^2;9~11', True)
    '*'
    """
    tstr = str(tstr)  # Allows bare ints.
    if (not tstr) or (conv_multiranges and tstr.find(';') > -1):
        return '*'

    tstr = tstr.replace("'", '')  # Dequote
    tstr = tstr.replace('"', '')

    numset = set([])

    for numrang in tstr.split(';'):
        step = 1
        try:
            if numrang.find('~') > -1:
                if numrang.find('^') > -1:
                    numrang, step = numrang.split('^')
                    step = int(step)
                start, end = map(int, numrang.split('~'))
            else:
                start = int(numrang)
                end = start
        except:
            raise ValueError, 'numrang = ' + numrang + ', tstr = ' + tstr + ', conv_multiranges = ' + str(conv_multiranges)
        numset.update(range(start, end + 1, step))
    return sorted(list(numset))

def spw_to_dict(spw, spwdict={}, conv_multiranges=True):
    """
    Expand an spw:chan string to {s0: [s0chans], s1: [s1chans, ...], ...}
    where s0, s1, ... are integers for _each_ selected spw, and s0chans is a
    set of selected chans (as integers) for s0.  '' instead of a channel set
    means that all of the channels are selected.

    The spw:chan dict is unioned with spwdict.

    Returning an empty dict means everything should be selected (i.e. spw = '').
    (split can't yet handle multiple channel ranges per spw.)

    conv_multiranges: If True, any spw with > 1 channel range selected will
                      have ALL of its channels selected.
                      (split can't yet handle multiple channel ranges per spw.)

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
        spwrange = expand_tilde(spwg)
        if spwrange == '*':  # This shouldn't happen.
            return {}
        else:
            charange = expand_tilde(chag, conv_multiranges)
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

def join_spws(spw1, spw2, span_semicolon=True):
    """
    Returns the union of spw selection strings spw1 and spw2.

    span_semicolon (default True): If True, for any spws
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
    >>> join_spws('1~10:5~122,15~22:5~122', '0~21')
    '0~21,22:5~122'
    """
    if not spw1 or not spw2:
        return ''
        
    spwdict = spw_to_dict(spw1, {})
    spwdict = spw_to_dict(spw2, spwdict)

    res = ''
    # Convert channel sets to strings
    for s in spwdict:
        cstr = ''
        if isinstance(spwdict[s], set):
            cstr = set_to_chanstr(spwdict[s])
            if span_semicolon and ';' in cstr:
                cstr = ''
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
        res += ':' + spwdict[slist[-1]]
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
