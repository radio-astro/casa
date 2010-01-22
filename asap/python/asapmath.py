from asap.scantable import scantable
from asap import rcParams
from asap import print_log
from asap import selector
from asap import asaplog
from asap import asaplotgui

def average_time(*args, **kwargs):
    """
    Return the (time) average of a scan or list of scans. [in channels only]
    The cursor of the output scan is set to 0
    Parameters:
        one scan or comma separated  scans or a list of scans
        mask:     an optional mask (only used for 'var' and 'tsys' weighting)
        scanav:   True averages each scan separately.
                  False (default) averages all scans together,
        weight:   Weighting scheme.
                    'none'     (mean no weight)
                    'var'      (1/var(spec) weighted)
                    'tsys'     (1/Tsys**2 weighted)
                    'tint'     (integration time weighted)
                    'tintsys'  (Tint/Tsys**2)
                    'median'   ( median averaging)
        align:    align the spectra in velocity before averaging. It takes
                  the time of the first spectrum in the first scantable
                  as reference time.
    Example:
        # return a time averaged scan from scana and scanb
        # without using a mask
        scanav = average_time(scana,scanb)
	# or equivalent
	# scanav = average_time([scana, scanb])
        # return the (time) averaged scan, i.e. the average of
        # all correlator cycles
        scanav = average_time(scan, scanav=True)
    """
    scanav = False
    if kwargs.has_key('scanav'):
       scanav = kwargs.get('scanav')
    weight = 'tint'
    if kwargs.has_key('weight'):
       weight = kwargs.get('weight')
    mask = ()
    if kwargs.has_key('mask'):
        mask = kwargs.get('mask')
    align = False
    if kwargs.has_key('align'):
        align = kwargs.get('align')
    compel = False
    if kwargs.has_key('compel'):
        compel = kwargs.get('compel')
    varlist = vars()
    if isinstance(args[0],list):
        lst = args[0]
    elif isinstance(args[0],tuple):
        lst = list(args[0])
    else:
        lst = list(args)

    del varlist["kwargs"]
    varlist["args"] = "%d scantables" % len(lst)
    # need special formatting here for history...

    from asap._asap import stmath
    stm = stmath()
    for s in lst:
        if not isinstance(s,scantable):
            msg = "Please give a list of scantables"
            if rcParams['verbose']:
                #print msg
                asaplog.push(msg)
                print_log('ERROR')
                return
            else:
                raise TypeError(msg)
    if scanav: scanav = "SCAN"
    else: scanav = "NONE"
    alignedlst = []
    if align:
        refepoch = lst[0].get_time(0)
        for scan in lst:
            alignedlst.append(scan.freq_align(refepoch,insitu=False))
    else:
        alignedlst = lst
    if weight.upper() == 'MEDIAN':
        # median doesn't support list of scantables - merge first
        merged = None
        if len(alignedlst) > 1:
            merged = merge(alignedlst)
        else:
            merged = alignedlst[0]
        s = scantable(stm._averagechannel(merged, 'MEDIAN', scanav))
        del merged
    else:
        #s = scantable(stm._average(alignedlst, mask, weight.upper(), scanav))
        s = scantable(stm._new_average(alignedlst, compel, mask, weight.upper(), scanav))
    s._add_history("average_time",varlist)
    print_log()
    return s

def quotient(source, reference, preserve=True):
    """
    Return the quotient of a 'source' (signal) scan and a 'reference' scan.
    The reference can have just one scan, even if the signal has many. Otherwise
    they must have the same number of scans.
    The cursor of the output scan is set to 0
    Parameters:
        source:        the 'on' scan
        reference:     the 'off' scan
        preserve:      you can preserve (default) the continuum or
                       remove it.  The equations used are
                       preserve:  Output = Toff * (on/off) - Toff
                       remove:    Output = Toff * (on/off) - Ton
    """
    varlist = vars()
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(False)
    s = scantable(stm._quotient(source, reference, preserve))
    s._add_history("quotient",varlist)
    print_log()
    return s

def dototalpower(calon, caloff, tcalval=0.0):
    """
    Do calibration for CAL on,off signals.
    Adopted from GBTIDL dototalpower
    Parameters:
        calon:         the 'cal on' subintegration
        caloff:        the 'cal off' subintegration
        tcalval:       user supplied Tcal value
    """
    varlist = vars()
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(False)
    s = scantable(stm._dototalpower(calon, caloff, tcalval))
    s._add_history("dototalpower",varlist)
    print_log()
    return s

def dosigref(sig, ref, smooth, tsysval=0.0, tauval=0.0):
    """
    Calculate a quotient (sig-ref/ref * Tsys)
    Adopted from GBTIDL dosigref
    Parameters:
        sig:         on source data
        ref:         reference data
        smooth:      width of box car smoothing for reference
        tsysval:     user specified Tsys (scalar only)
        tauval:      user specified Tau (required if tsysval is set)
    """
    varlist = vars()
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(False)
    s = scantable(stm._dosigref(sig, ref, smooth, tsysval, tauval))
    s._add_history("dosigref",varlist)
    print_log()
    return s

def calps(scantab, scannos, smooth=1, tsysval=0.0, tauval=0.0, tcalval=0.0, verify=False):
    """
    Calibrate GBT position switched data
    Adopted from GBTIDL getps
    Currently calps identify the scans as position switched data if they
    contain '_ps' in the source name. The data must contains 'CAL' signal
    on/off in each integration. To identify 'CAL' on state, the word, 'calon'
    need to be present in the source name field.
    (GBT MS data reading process to scantable automatically append these
    id names to the source names)

    Parameters:
        scantab:       scantable
        scannos:       list of scan numbers
        smooth:        optional box smoothing order for the reference
                       (default is 1 = no smoothing)
        tsysval:       optional user specified Tsys (default is 0.0,
                       use Tsys in the data)
        tauval:        optional user specified Tau
        tcalval:       optional user specified Tcal (default is 0.0,
                       use Tcal value in the data)
    """
    varlist = vars()
    # check for the appropriate data
    s = scantab.get_scan('*_ps*')
    if s is None:
        msg = "The input data appear to contain no position-switch mode data."
        if rcParams['verbose']:
            #print msg
            asaplog.push(msg)
            print_log('ERROR')
            return
        else:
            raise TypeError(msg)
    ssub = s.get_scan(scannos)
    if ssub is None:
        msg = "No data was found with given scan numbers!"
        if rcParams['verbose']:
            #print msg
            asaplog.push(msg)
            print_log('ERROR')
            return
        else:
            raise TypeError(msg)
    ssubon = ssub.get_scan('*calon')
    ssuboff = ssub.get_scan('*[^calon]')
    if ssubon.nrow() != ssuboff.nrow():
        msg = "mismatch in numbers of CAL on/off scans. Cannot calibrate. Check the scan numbers."
        if rcParams['verbose']:
            #print msg
            asaplog.push(msg)
            print_log('ERROR')
            return
        else:
            raise TypeError(msg)
    cals = dototalpower(ssubon, ssuboff, tcalval)
    sig = cals.get_scan('*ps')
    ref = cals.get_scan('*psr')
    if sig.nscan() != ref.nscan():
        msg = "mismatch in numbers of on/off scans. Cannot calibrate. Check the scan numbers."
        if rcParams['verbose']:
            #print msg
            asaplog.push(msg)
            print_log('ERROR')
            return
        else:
            raise TypeError(msg)

    #for user supplied Tsys
    if tsysval>0.0:
        if tauval<=0.0:
            msg = "Need to supply a valid tau to use the supplied Tsys"
            if rcParams['verbose']:
                #print msg
                asaplog.push(msg)
                print_log('ERROR')
                return
            else:
                raise TypeError(msg)
        else:
            sig.recalc_azel()
            ref.recalc_azel()
            #msg = "Use of user specified Tsys is not fully implemented yet."
            #if rcParams['verbose']:
            #    print msg
            #    return
            #else:
            #    raise TypeError(msg)
            # use get_elevation to get elevation and
            # calculate a scaling factor using the formula
            # -> tsys use to dosigref

    #ress = dosigref(sig, ref, smooth, tsysval)
    ress = dosigref(sig, ref, smooth, tsysval, tauval)
    ###
    if verify:
        # get data
        import numpy
        precal={}
        postcal=[]
        keys=['ps','ps_calon','psr','psr_calon']
        ifnos=list(ssub.getifnos())
        polnos=list(ssub.getpolnos())
        sel=selector()
        for i in range(2):
            ss=ssuboff.get_scan('*'+keys[2*i])
            ll=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    try:
                        ss.set_selection(sel)
                    except:
                        continue
                    ll.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    ss.set_selection()
            precal[keys[2*i]]=ll
            del ss
            ss=ssubon.get_scan('*'+keys[2*i+1])
            ll=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    try:
                        ss.set_selection(sel)
                    except:
                        continue
                    ll.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    ss.set_selection()
            precal[keys[2*i+1]]=ll
            del ss
        for j in range(len(ifnos)):
            for k in range(len(polnos)):
                sel.set_ifs(ifnos[j])
                sel.set_polarizations(polnos[k])
                try:
                    ress.set_selection(sel)
                except:
                    continue
                postcal.append(numpy.array(ress._getspectrum(0)))
                sel.reset()
                ress.set_selection()
        del sel
        # plot
        print_log()
        asaplog.push('Plot only first spectrum for each [if,pol] pairs to verify calibration.')
        print_log('WARN')
        p=asaplotgui.asaplotgui()
        #nr=min(6,len(ifnos)*len(polnos))
        nr=len(ifnos)*len(polnos)
        titles=[]
        btics=[]
        if nr<4:
            p.set_panels(rows=nr,cols=2,nplots=2*nr,ganged=False)
            for i in range(2*nr):
                b=False
                if i >= 2*nr-2:
                    b=True
                btics.append(b)
        elif nr==4:
            p.set_panels(rows=2,cols=4,nplots=8,ganged=False)
            for i in range(2*nr):
                b=False
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
        elif nr<7:
            p.set_panels(rows=3,cols=4,nplots=2*nr,ganged=False)
            for i in range(2*nr):
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
        else:
            print_log()
            asaplog.push('Only first 6 [if,pol] pairs are plotted.')
            print_log('WARN')
            nr=6
            for i in range(2*nr):
                b=False
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
            p.set_panels(rows=3,cols=4,nplots=2*nr,ganged=False)
        for i in range(nr):
            p.subplot(2*i)
            p.color=0
            title='raw data IF%s POL%s' % (ifnos[int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title,fontsize=40)
            ymin=1.0e100
            ymax=-1.0e100
            nchan=s.nchan()
            edge=int(nchan*0.01)
            for j in range(4):
                spmin=min(precal[keys[j]][i][edge:nchan-edge])
                spmax=max(precal[keys[j]][i][edge:nchan-edge])
                ymin=min(ymin,spmin)
                ymax=max(ymax,spmax)
            for j in range(4):
                if i==0:
                    p.set_line(label=keys[j])
                else:
                    p.legend()
                p.plot(precal[keys[j]][i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[2*i]:
                p.axes.set_xticks([])
            p.subplot(2*i+1)
            p.color=0
            title='cal data IF%s POL%s' % (ifnos[int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title)
            p.legend()
            ymin=postcal[i][edge:nchan-edge].min()
            ymax=postcal[i][edge:nchan-edge].max()
            p.plot(postcal[i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[2*i+1]:
                p.axes.set_xticks([])
        for i in range(2*nr):
            p.subplot(i)
            p.set_axes('title',titles[i],fontsize='medium')
        x=raw_input('Accept calibration ([y]/n): ' )
        if x.upper() == 'N':
            p.unmap()
            del p
            return scabtab
        p.unmap()
        del p
    ###
    ress._add_history("calps", varlist)
    print_log()
    return ress

def calnod(scantab, scannos=[], smooth=1, tsysval=0.0, tauval=0.0, tcalval=0.0, verify=False):
    """
    Do full (but a pair of scans at time) processing of GBT Nod data
    calibration.
    Adopted from  GBTIDL's getnod
    Parameters:
        scantab:     scantable
        scannos:     a pair of scan numbers, or the first scan number of the pair
        smooth:      box car smoothing order
        tsysval:     optional user specified Tsys value
        tauval:      optional user specified tau value (not implemented yet)
        tcalval:     optional user specified Tcal value
    """
    varlist = vars()
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(False)

    # check for the appropriate data
    s = scantab.get_scan('*_nod*')
    if s is None:
        msg = "The input data appear to contain no Nod observing mode data."
        if rcParams['verbose']:
            #print msg
            asaplog.push(msg)
            print_log('ERROR')
            return
        else:
            raise TypeError(msg)

    # need check correspondance of each beam with sig-ref ...
    # check for timestamps, scan numbers, subscan id (not available in
    # ASAP data format...). Assume 1st scan of the pair (beam 0 - sig
    # and beam 1 - ref...)
    # First scan number of paired scans or list of pairs of
    # scan numbers (has to have even number of pairs.)

    #data splitting
    scan1no = scan2no = 0

    if len(scannos)==1:
        scan1no = scannos[0]
        scan2no = scannos[0]+1
        pairScans = [scan1no, scan2no]
    else:
        #if len(scannos)>2:
        #    msg = "calnod can only process a pair of nod scans at time."
        #    if rcParams['verbose']:
        #        print msg
        #        return
        #    else:
        #        raise TypeError(msg)
        #
        #if len(scannos)==2:
        #    scan1no = scannos[0]
        #    scan2no = scannos[1]
        pairScans = list(scannos)

    if tsysval>0.0:
        if tauval<=0.0:
            msg = "Need to supply a valid tau to use the supplied Tsys"
            if rcParams['verbose']:
                #print msg
                asaplog.push(msg)
                print_log('ERROR')
                return
            else:
                raise TypeError(msg)
        else:
            scantab.recalc_azel()
    resspec = scantable(stm._donod(scantab, pairScans, smooth, tsysval,tauval,tcalval))
    ###
    if verify:
        # get data
        import numpy
        precal={}
        postcal=[]
        keys=['nod','nod_calon']
        ifnos=list(scantab.getifnos())
        polnos=list(scantab.getpolnos())
        sel=selector()
        for i in range(2):
            ss=scantab.get_scan('*'+keys[i])
            ll=[]
            ll2=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    sel.set_scans(pairScans[0])
                    try:
                        ss.set_selection(sel)
                    except:
                        continue
                    ll.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    ss.set_selection()
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    sel.set_scans(pairScans[1])
                    try:
                        ss.set_selection(sel)
                    except:
                        ll.pop()
                        continue
                    ll2.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    ss.set_selection()
            key='%s%s' %(pairScans[0],keys[i].lstrip('nod'))
            precal[key]=ll
            key='%s%s' %(pairScans[1],keys[i].lstrip('nod'))
            precal[key]=ll2
            del ss
        keys=precal.keys()
        for j in range(len(ifnos)):
            for k in range(len(polnos)):
                sel.set_ifs(ifnos[j])
                sel.set_polarizations(polnos[k])
                sel.set_scans(pairScans[0])
                try:
                    resspec.set_selection(sel)
                except:
                    continue
                postcal.append(numpy.array(resspec._getspectrum(0)))
                sel.reset()
                resspec.set_selection()
        del sel
        # plot
        print_log()
        asaplog.push('Plot only first spectrum for each [if,pol] pairs to verify calibration.')
        print_log('WARN')
        p=asaplotgui.asaplotgui()
        #nr=min(6,len(ifnos)*len(polnos))
        nr=len(ifnos)*len(polnos)
        titles=[]
        btics=[]
        if nr<4:
            p.set_panels(rows=nr,cols=2,nplots=2*nr,ganged=False)
            for i in range(2*nr):
                b=False
                if i >= 2*nr-2:
                    b=True
                btics.append(b)
        elif nr==4:
            p.set_panels(rows=2,cols=4,nplots=8,ganged=False)
            for i in range(2*nr):
                b=False
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
        elif nr<7:
            p.set_panels(rows=3,cols=4,nplots=2*nr,ganged=False)
            for i in range(2*nr):
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
        else:
            print_log()
            asaplog.push('Only first 6 [if,pol] pairs are plotted.')
            print_log('WARN')
            nr=6
            for i in range(2*nr):
                b=False
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
            p.set_panels(rows=3,cols=4,nplots=2*nr,ganged=False)
        for i in range(nr):
            p.subplot(2*i)
            p.color=0
            title='raw data IF%s POL%s' % (ifnos[int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title,fontsize=40)
            ymin=1.0e100
            ymax=-1.0e100
            nchan=scantab.nchan()
            edge=int(nchan*0.01)
            for j in range(4):
                spmin=min(precal[keys[j]][i][edge:nchan-edge])
                spmax=max(precal[keys[j]][i][edge:nchan-edge])
                ymin=min(ymin,spmin)
                ymax=max(ymax,spmax)
            for j in range(4):
                if i==0:
                    p.set_line(label=keys[j])
                else:
                    p.legend()
                p.plot(precal[keys[j]][i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[2*i]:
                p.axes.set_xticks([])
            p.subplot(2*i+1)
            p.color=0
            title='cal data IF%s POL%s' % (ifnos[int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title)
            p.legend()
            ymin=postcal[i][edge:nchan-edge].min()
            ymax=postcal[i][edge:nchan-edge].max()
            p.plot(postcal[i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[2*i+1]:
                p.axes.set_xticks([])
        for i in range(2*nr):
            p.subplot(i)
            p.set_axes('title',titles[i],fontsize='medium')
        x=raw_input('Accept calibration ([y]/n): ' )
        if x.upper() == 'N':
            p.unmap()
            del p
            return scabtab
        p.unmap()
        del p
    ###
    resspec._add_history("calnod",varlist)
    print_log()
    return resspec

def calfs(scantab, scannos=[], smooth=1, tsysval=0.0, tauval=0.0, tcalval=0.0, verify=False):
    """
    Calibrate GBT frequency switched data.
    Adopted from GBTIDL getfs.
    Currently calfs identify the scans as frequency switched data if they
    contain '_fs' in the source name. The data must contains 'CAL' signal
    on/off in each integration. To identify 'CAL' on state, the word, 'calon'
    need to be present in the source name field.
    (GBT MS data reading via scantable automatically append these
    id names to the source names)

    Parameters:
        scantab:       scantable
        scannos:       list of scan numbers
        smooth:        optional box smoothing order for the reference
                       (default is 1 = no smoothing)
        tsysval:       optional user specified Tsys (default is 0.0,
                       use Tsys in the data)
        tauval:        optional user specified Tau
    """
    varlist = vars()
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(False)

#    check = scantab.get_scan('*_fs*')
#    if check is None:
#        msg = "The input data appear to contain no Nod observing mode data."
#        if rcParams['verbose']:
#            print msg
#            return
#        else:
#            raise TypeError(msg)
    s = scantab.get_scan(scannos)
    del scantab

    resspec = scantable(stm._dofs(s, scannos, smooth, tsysval,tauval,tcalval))
    ###
    if verify:
        # get data
        ssub = s.get_scan(scannos)
        ssubon = ssub.get_scan('*calon')
        ssuboff = ssub.get_scan('*[^calon]')
        import numpy
        precal={}
        postcal=[]
        keys=['fs','fs_calon','fsr','fsr_calon']
        ifnos=list(ssub.getifnos())
        polnos=list(ssub.getpolnos())
        sel=selector()
        for i in range(2):
            ss=ssuboff.get_scan('*'+keys[2*i])
            ll=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    try:
                        ss.set_selection(sel)
                    except:
                        continue
                    ll.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    ss.set_selection()
            precal[keys[2*i]]=ll
            del ss
            ss=ssubon.get_scan('*'+keys[2*i+1])
            ll=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    try:
                        ss.set_selection(sel)
                    except:
                        continue
                    ll.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    ss.set_selection()
            precal[keys[2*i+1]]=ll
            del ss
        sig=resspec.get_scan('*_fs')
        ref=resspec.get_scan('*_fsr')
        for k in range(len(polnos)):
            for j in range(len(ifnos)):
                sel.set_ifs(ifnos[j])
                sel.set_polarizations(polnos[k])
                try:
                    sig.set_selection(sel)
                    postcal.append(numpy.array(sig._getspectrum(0)))
                except:
                    ref.set_selection(sel)
                    postcal.append(numpy.array(ref._getspectrum(0)))
                sel.reset()
                resspec.set_selection()
        del sel
        # plot
        print_log()
        asaplog.push('Plot only first spectrum for each [if,pol] pairs to verify calibration.')
        print_log('WARN')
        p=asaplotgui.asaplotgui()
        #nr=min(6,len(ifnos)*len(polnos))
        nr=len(ifnos)/2*len(polnos)
        titles=[]
        btics=[]
        if nr>3:
            print_log()
            asaplog.push('Only first 3 [if,pol] pairs are plotted.')
            print_log('WARN')
            nr=3
        p.set_panels(rows=nr,cols=3,nplots=3*nr,ganged=False)
        for i in range(3*nr):
            b=False
            if i >= 3*nr-3:
                b=True
            btics.append(b)
        for i in range(nr):
            p.subplot(3*i)
            p.color=0
            title='raw data IF%s,%s POL%s' % (ifnos[2*int(i/len(polnos))],ifnos[2*int(i/len(polnos))+1],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title,fontsize=40)
            ymin=1.0e100
            ymax=-1.0e100
            nchan=s.nchan()
            edge=int(nchan*0.01)
            for j in range(4):
                spmin=min(precal[keys[j]][i][edge:nchan-edge])
                spmax=max(precal[keys[j]][i][edge:nchan-edge])
                ymin=min(ymin,spmin)
                ymax=max(ymax,spmax)
            for j in range(4):
                if i==0:
                    p.set_line(label=keys[j])
                else:
                    p.legend()
                p.plot(precal[keys[j]][i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[3*i]:
                p.axes.set_xticks([])
            p.subplot(3*i+1)
            p.color=0
            title='sig data IF%s POL%s' % (ifnos[2*int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title)
            p.legend()
            ymin=postcal[2*i][edge:nchan-edge].min()
            ymax=postcal[2*i][edge:nchan-edge].max()
            p.plot(postcal[2*i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[3*i+1]:
                p.axes.set_xticks([])
            p.subplot(3*i+2)
            p.color=0
            title='ref data IF%s POL%s' % (ifnos[2*int(i/len(polnos))+1],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title)
            p.legend()
            ymin=postcal[2*i+1][edge:nchan-edge].min()
            ymax=postcal[2*i+1][edge:nchan-edge].max()
            p.plot(postcal[2*i+1])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[3*i+2]:
                p.axes.set_xticks([])
        for i in range(3*nr):
            p.subplot(i)
            p.set_axes('title',titles[i],fontsize='medium')
        x=raw_input('Accept calibration ([y]/n): ' )
        if x.upper() == 'N':
            p.unmap()
            del p
            return scabtab
        p.unmap()
        del p
    ###
    resspec._add_history("calfs",varlist)
    print_log()
    return resspec

def simple_math(left, right, op='add', tsys=True):
    """
    Apply simple mathematical binary operations to two
    scan tables,  returning the result in a new scan table.
    The operation is applied to both the correlations and the TSys data
    The cursor of the output scan is set to 0
    Parameters:
        left:          the 'left' scan
        right:         the 'right' scan
        op:            the operation: 'add' (default), 'sub', 'mul', 'div'
        tsys:          if True (default) then apply the operation to Tsys
                       as well as the data
    """
    #print "simple_math is deprecated use +=/* instead."
    asaplog.push( "simple_math is deprecated use +=/* instead." )
    print_log('WARN')

def merge(*args):
    """
    Merge a list of scanatables, or comma-sperated scantables into one
    scnatble.
    Parameters:
        A list [scan1, scan2] or scan1, scan2.
    Example:
        myscans = [scan1, scan2]
	allscans = merge(myscans)
	# or equivalent
	sameallscans = merge(scan1, scan2)
    """
    varlist = vars()
    if isinstance(args[0],list):
        lst = tuple(args[0])
    elif isinstance(args[0],tuple):
        lst = args[0]
    else:
        lst = tuple(args)
    varlist["args"] = "%d scantables" % len(lst)
    # need special formatting her for history...
    from asap._asap import stmath
    stm = stmath()
    for s in lst:
        if not isinstance(s,scantable):
            msg = "Please give a list of scantables"
            if rcParams['verbose']:
                #print msg
                asaplog.push(msg)
                print_log('ERROR')
                return
            else:
                raise TypeError(msg)
    s = scantable(stm._merge(lst))
    s._add_history("merge", varlist)
    print_log()
    return s

def calibrate( scantab, scannos=[], calmode='none', verify=None ):
    """
    Calibrate data.
    
    Parameters:
        scantab:       scantable
        scannos:       list of scan number
        calmode:       calibration mode
        verify:        verify calibration     
    """
    antname = scantab.get_antennaname()
    if ( calmode == 'nod' ):
        asaplog.push( 'Calibrating nod data.' )
        print_log()
        scal = calnod( scantab, scannos=scannos, verify=verify )
    elif ( calmode == 'quotient' ):
        asaplog.push( 'Calibrating using quotient.' )
        print_log()
        scal = scantab.auto_quotient( verify=verify )
    elif ( calmode == 'ps' ):
        asaplog.push( 'Calibrating %s position-switched data.' % antname )
        print_log()
        if ( antname.find( 'APEX' ) != -1 ):
            scal = apexcal( scantab, scannos, calmode, verify )
        elif ( antname.find( 'ALMA' ) != -1 or antname.find( 'OSF' ) != -1 ):
            scal = almacal( scantab, scannos, calmode, verify )
        else:
            scal = calps( scantab, scannos=scannos, verify=verify )
    elif ( calmode == 'fs' or calmode == 'fsotf' ):
        asaplog.push( 'Calibrating %s frequency-switched data.' % antname )
        print_log()
        if ( antname.find( 'APEX' ) != -1 ):
            scal = apexcal( scantab, scannos, calmode, verify )
        elif ( antname.find( 'ALMA' ) != -1 or antname.find( 'OSF' ) != -1 ):
            scal = almacal( scantab, scannos, calmode, verify )
        else:
            scal = calfs( scantab, scannos=scannos, verify=verify )
    elif ( calmode == 'otf' ):
        asaplog.push( 'Calibrating %s On-The-Fly data.' % antname )
        print_log()
        scal = almacal( scantab, scannos, calmode, verify )
    else:
        asaplog.push( 'No calibration.' )
        scal = scantab.copy()

    return scal 

def apexcal( scantab, scannos=[], calmode='none', verify=False ):
    """
    Calibrate APEX data

    Parameters:
        scantab:       scantable
        scannos:       list of scan number
        calmode:       calibration mode

        verify:        verify calibration     
    """
    from asap._asap import stmath
    stm = stmath()
    antname = scantab.get_antennaname()
    ssub = scantab.get_scan( scannos )
    scal = scantable( stm.cwcal( ssub, calmode, antname ) )
    return scal

def almacal( scantab, scannos=[], calmode='none', verify=False ):
    """
    Calibrate ALMA data

    Parameters:
        scantab:       scantable
        scannos:       list of scan number
        calmode:       calibration mode

        verify:        verify calibration     
    """
    from asap._asap import stmath
    stm = stmath()
    ssub = scantab.get_scan( scannos )
    scal = scantable( stm.almacal( ssub, calmode ) )
    return scal

def splitant(filename, outprefix='',overwrite=False):
    """
    Split Measurement set by antenna name, save data as a scantables,
    and return a list of filename.
    Notice this method can only be available from CASA. 
    Prameter
       filename:    the name of Measurement set to be read. 
       outprefix:   the prefix of output scantable name.
                    the names of output scantable will be
                    outprefix.antenna1, outprefix.antenna2, ....
                    If not specified, outprefix = filename is assumed.
       overwrite    If the file should be overwritten if it exists.
                    The default False is to return with warning
                    without writing the output. USE WITH CARE.
                    
    """
    # Import the table toolkit from CASA
    try:
        import casac
    except ImportError:
        if rcParams['verbose']:
            #print "failed to load casa"
            print_log()
            asaplog.push("failed to load casa")
            print_log('ERROR')
        else: raise
        return False
    try:
        tbtool = casac.homefinder.find_home_by_name('tableHome')
        tb = tbtool.create()
        tb2 = tbtool.create()
    except:
        if rcParams['verbose']:
            #print "failed to load a table tool:\n", e
            print_log()
            asaplog.push("failed to load table tool")
            print_log('ERROR')
        else: raise
        return False
    # Check the input filename
    if isinstance(filename, str):
        import os.path
        filename = os.path.expandvars(filename)
        filename = os.path.expanduser(filename)
        if not os.path.exists(filename):
            s = "File '%s' not found." % (filename)
            if rcParams['verbose']:
                print_log()
                asaplog.push(s)
                print_log('ERROR')
                return
            raise IOError(s)
        # check if input file is MS
        if not os.path.isdir(filename) \
               or not os.path.exists(filename+'/ANTENNA') \
               or not os.path.exists(filename+'/table.f1'): 
            s = "File '%s' is not a Measurement set." % (filename)
            if rcParams['verbose']:
                print_log()
                asaplog.push(s)
                print_log('ERROR')
                return
            raise IOError(s)
    else:
        s = "The filename should be string. "
        if rcParams['verbose']:
            print_log()
            asaplog.push(s)
            print_log('ERROR')
            return
        raise TypeError(s)
    # Check out put file name
    outname=''
    if len(outprefix) > 0: prefix=outprefix+'.'
    else:
        prefix=filename
    # Now do the actual splitting.
    outfiles=[]
    tmpms="temp_antsplit.ms"
    if os.path.exists(tmpms):
        ans=raw_input('Temporal file '+tmpms+' exists. Delete it and continue? [y/N]: ')
        if ans.upper() == 'Y':
            os.system('rm -rf '+tmpms)
            asaplog.push('The file '+tmpms+' deleted.')
        else:
            asaplog.push('Exit without splitting.')
            return
    tb.open(tablename=filename+'/ANTENNA',nomodify=True)
    nant=tb.nrows()
    antnames=tb.getcol('NAME',0,nant,1)
    antpos=tb.getcol('POSITION',0,nant,1).transpose()
    tb.close()
    tb.open(tablename=filename,nomodify=True)
    ant1=tb.getcol('ANTENNA1',0,-1,1)
    for antid in set(ant1):
        qstr="ANTENNA1 == "+str(antid)
        stab = tb.queryC(qstr)
        ctab = stab.copy(tmpms,deep=True)
        stab.close()
        ctab.close()
        scan=scantable(tmpms,average=False,getpt=True)
        outname=prefix+antnames[antid]+'.asap'
        scan.save(outname,format='ASAP',overwrite=overwrite)
        # Modify scantable header
        tb2.open(tablename=outname,nomodify=False)
        tb2.putkeyword(keyword='AntennaName',value=antnames[antid])
        tb2.putkeyword(keyword='AntennaPosition',value=antpos[antid])
        tb2.flush()
        tb2.close()
        del scan, ctab, stab
        outfiles.append(outname)
    tb.close()
    del tb, tb2
    os.system('rm -rf '+tmpms)
    return outfiles
