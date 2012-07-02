import re

def update_spw(spw, spwmap=None):
    """Given an spw:chan selection string, return what it should be after
       the spws have been remapped (i.e. by split).  It does not change spw.

       If given, spwmap will be used as a dictionary from (string) input spw to
       (string) output spws.  Otherwise it will be freshly calculated.
       Supplying spwmap doesn't just save work: it is also necessary for
       chaining update_spw() calls when the first selection includes more spws
       than the subsequent one(s).
       Example:
       myfitspw, spws = update_spw('0~3,5;6:1~7;11~13', None)
       # myfitspw == '0~3,4,5:1~7;11~1'
       myspw = update_spw('1,5;6:8~10', spws)[0]
       # myspw == '1,4,5:8~10', not '0,1,2:8~10'
       """

    # teststr = '0~3,5;6:1~7;11~13,7~9:0~3,11,7~8:6~8'
    # should -> '0~3,4,5:1~7;11~13,6~8:0~3,9,6~7:6~8'

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
