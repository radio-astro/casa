def _vanvleck_flag(scan, level=5.0):
    """
    Find level crossings and flag all data before (and after it).
    This rectifies the problem caused by the MOPS bandpass being negative.
    Use levels greater > 0.0 to get a better result.
    """
    sel = selector()
    saved = scan.get_selection()
    for i in xrange(scan.nrow()):
        sel.reset()
        sel.set_scans(scan.getscan(i))
        sel.set_beams(scan.getbeam(i))
        sel.set_ifs(scan.getif(i))
        sel.set_polarisations(scan.getpol(i))
        sel.set_cycles(scan.getcycle(i))
        scan.set_selection(sel)
        spec = scan._getspectrum()
        msk = mask_not(scan._getmask())
        n = len(msk)
        modified = False
        for j in xrange(n):
            if j < n and spec[j] <= level:
                msk[j] = 1
                modified = True
        if modified:
            scan.flag(msk)
        scan.set_selection(saved)

if not "vanvleck_flag" in dir(scantable):
    scantable.vanvleck_flag = _vanvleck_flag
    del _vanvleck_flag
