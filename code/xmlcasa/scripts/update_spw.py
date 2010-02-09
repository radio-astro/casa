import re

def update_spw(spw):
    """Given an spw:chan selection string, return what it should be after
       the spws have been remapped (i.e. by split)."""

    # teststr = '0~3,5;6:1~7;11~13,7~9:0~3,11,7~8:6~8'
    # should -> '0~3,4,5:1~7;11~13,6~8:0~3,9,6~7:6~8'

    # A list of [spw, chan] pairs.  The chan parts will not be changed.
    spwchans = [] 

    # Because ; means different things when it separates spws and channel
    # ranges, I can't think of a better way to construct spwchans than an
    # explicit state machine.  (But $spws_alone =~ s/:[^,]+//g;)
    inspw = True    # Must start with an spw.
    spwgrp = ''
    chagrp = ''
    spws = set([])
    for c in spw:
        if c == ',' or (inspw and c == ';'):  # Start new [spw, chan] pair.
            # Store old one.
            spwchans.append([spwgrp, chagrp])
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
    if spwgrp.find('~') > -1:
        start, end = map(int, spwgrp.split('~'))
        spws.update(range(start, end + 1))
    else:
        spws.add(int(spwgrp))

    # print "spwchans =", spwchans
    # print "spws =", spws
        
    # Update spw (+ fitspw)
    smap = {}
    i = 0
    for s in sorted(spws):
        smap[str(s)] = str(i)
        i += 1
    outstr = ''
    for sc in spwchans:
        if sc[0].find('~') > -1:
            start, end = sc[0].split('~')
            outstr += smap[start] + '~' + smap[end]
        else:
            outstr += smap[sc[0]]
        if sc[1]:
            outstr += ':' + sc[1]
        outstr += ','

    return outstr[:-1] # discard final comma.
