from taskinit import *
import shutil

def fixvis(vis, outputvis='',field='', refcode='', reuse=True, phasecenter='', distances='', datacolumn='all'):
    """
    Input Parameters
    vis        -- Name of the input visibility set
    
    outputvis  -- Name of the output visibility set, default: same as vis

    field      -- field selection string

    refcode    -- Reference frame to convert to,
                  default: the refcode of PHASE_DIR in the FIELD table
                  example: 'B1950'
    
    reuse      -- base recalculation on existing UVW coordinates? default=True
                  ignored if parameter 'phasecenter' is set

    phasecenter  --  if set to a valid direction: change the phase center for the given field
                     to this value
                     example: 'J2000 9h25m00s 05d12m00s'
                     If given without the equinox, e.g. '0h01m00s 00d12m00s', the parameter
                     is interpreted as a pair of offsets in RA and DEC to the present phasecenter.

    distances -- (experimental) List of the distances (as quanta) of the fields selected by field
                 to be used for refocussing.
                 If empty, the distances of all fields are assumed to be infinity.
                 If not a list but just a single value is given, this is applied to
                 all fields.
                 default: []
                 examples: ['2E6km', '3E6km']   '15au'

    datacolumn -- when applying a phase center shift, modify visibilities only in this/these column(s)
                  default: 'all' (DATA, CORRECTED, and MODEL)
                  example: 'DATA,CORRECTED' (will not modify MODEL)

    Examples:

    fixvis('NGC3256.ms','NGC3256-fixed.ms')
          will recalculate the UVW coordinates for all fields based on the existing
          phase center information in the FIELD table.

    fixvis('0925+0512.ms','0925+0512-fixed.ms','0925+0512', '', 'J2000 9h25m00s 05d12m00s')
          will set the phase center for field '0925+0512' to the given direction and recalculate
          the UVW coordinates.
    """
    casalog.origin('fixvis')
    retval = True
    try:
        if(vis==outputvis or outputvis==''):
            casalog.post('Will overwrite original MS ...', 'NORMAL')
            outputvis = vis
        else:
            casalog.post('Copying original MS to outputvis ...', 'NORMAL')
            shutil.rmtree(outputvis, ignore_errors=True)
            shutil.copytree(vis, outputvis)

        # me and qa are also used, but not in a state-altering way.
        tbt, myms, myim = gentools(['tb', 'ms', 'im'])
        
        if(field==''):
            field='*'
            
        fields = myms.msseltoindex(vis=outputvis,field=field)['field']

        if(len(fields) == 0):
            casalog.post( "Field selection returned zero results.", 'WARN')
            return False

        thedistances = []

        if(distances==""):
            distances = []
        elif(distances!=[]):
            if(type(distances)==str and qa.isquantity(distances)):
                thedist = qa.canonical(distances)
                if(thedist['unit']=='m'): # a length
                    for f in fields: # put nfields copies into the list
                        thedistances.append(thedist['value'])
                else:
                    casalog.post("Parameter distances needs to contain quanta with units of length.", 'SEVERE')                
                    return False
            elif(type(distances)==list):
                if(len(fields)!=len(distances)):
                    casalog.post("You selected "+str(len(fields))+" fields but gave "+str(len(distances))+" distances,", 'SEVERE')
                    return False
                else:
                    for d in distances:
                        if(qa.isquantity(d)):
                            thedist = qa.canonical(d)
                            if(thedist['unit']=='m'): # a length
                                thedistances.append(thedist['value'])
                            else:
                                casalog.post("Parameter distances needs to contain quanta with units of length.", 'SEVERE')       
                                return False
            else:
                casalog.post("Invalid parameter distances.", 'SEVERE')       
                return False


        if(thedistances!=[]):
            casalog.post('Will refocus to the given distances: '+str(distances), 'NORMAL')
        
        #determine therefcode, the reference frame to be used for the output UVWs
        tbt.open(outputvis+"/FIELD")
        numfields = tbt.nrows()
        therefcode = 'J2000'
        ckwdict = tbt.getcolkeyword('PHASE_DIR', 'MEASINFO')
        tbt.close()
        if(refcode==''):
            if(ckwdict.has_key('TabRefTypes')): # we have a variable reference column
                therefcode = 'J2000' # always use "J2000"
            else: # not a variable reference column
                therefcode = ckwdict['Ref']
        else: # a refcode was given, enforce validity
            if not (type(refcode)==str):
                casalog.post('Invalid refcode '+str(refcode), 'SEVERE')
                return False                
            if(ckwdict.has_key('TabRefTypes')): # variable ref column
                refcodelist = ckwdict['TabRefTypes'].tolist()
                ref = 0
                if not (refcode in refcodelist):
                    casalog.post('Invalid refcode '+refcode, 'SEVERE')
                    return False
            else: # not a variable reference column
                if not (refcode in get_validcodes()):
                    casalog.post('Invalid refcode '+refcode, 'SEVERE')
                    return False
            #endif
            therefcode = refcode
        #end if

        if(phasecenter==''): # we are only modifying the UVW coordinates        
            casalog.post('Will leave phase centers unchanged.', 'NORMAL')
            casalog.post("Recalculating the UVW coordinates ...", 'NORMAL')

            fldids = []
            for i in xrange(numfields):
                if (i in fields):
                    fldids.append(i)

            # 
            myim.open(outputvis, usescratch=False)
            myim.calcuvw(fields=fldids, refcode=therefcode, reuse=reuse)
            myim.close()
        else: # we are modifying UVWs and visibilities
            ## if observation:
            ##     casalog.post('Modifying the phase tracking center(s) is imcompatible', 'SEVERE')
            ##     casalog.post('with operating on only a subset of the observation IDs', 'SEVERE')
            ##     return False
            
            if(type(phasecenter) != str):
                casalog.post("Invalid phase center.", 'SEVERE')
                return False

            try:
                theoldref, theoldrefstr, ckwdict, isvarref, flddict = get_oldref(outputvis, tbt)
            except ValueError:
                return False

            # for the case of a non-variable reference column and several selected fields 
            commonoldrefstr = ''

            # handle the datacolumn parameter
            if (not type(datacolumn)==str):
                casalog.post("Invalid parameter datacolumn", 'SEVERE')
            elif datacolumn=='' or datacolumn.lower()=='all':
                datacolumn='all'
                casalog.post("Will modify the visibilities in all columns", 'NORMAL')
            else:
                # need to check datacolumn before any part of the MS gets modified
                wantedcolumns = datacolumn.split(',')
                tbt.open(outputvis)
                thecolumns = tbt.colnames()
                tbt.close()
                for col in wantedcolumns:
                    if not (col.lower() in ['data','observed','corrected', 'corrected_data','model','model_data']):
                        casalog.post("Invalid datacolumn: \""+col+"\"", 'SEVERE')
                        return False
                    if (col.lower()=='observed'):
                        col = 'DATA'
                    if (col.lower()=='corrected'):
                        col = 'CORRECTED_DATA'
                    if (col.lower()=='model'):
                        col = 'MODEL_DATA'
                    if not col.upper() in thecolumns:
                        casalog.post("Datacolumn "+col+" not present", 'SEVERE')
                        return False
                casalog.post("Will only modify the visibilities in the columns "+datacolumn.upper(), 'NORMAL')

            for fld in fields:
                allselected = True
                for i in xrange(numfields):
                    if not (i in fields):
                        allselected = False
                        break

                commonoldrefstr = modify_fld_vis(fld, outputvis, tbt, myim,
                                                 commonoldrefstr, phasecenter,
                                                 therefcode, reuse, numfields,
                                                 ckwdict, theoldref, theoldrefstr,
                                                 isvarref, flddict, datacolumn,
                                                 allselected, thedistances)
                if commonoldrefstr == False:
                    return False
        #endif change phasecenter

        # Write history to output MS
        try:
            param_names = fixvis.func_code.co_varnames[:fixvis.func_code.co_argcount]
            param_vals = [eval(p) for p in param_names]   
            retval &= write_history(myms, outputvis, 'fixvis', param_names, param_vals,
                                    casalog)
        except Exception, instance:
            casalog.post("*** Error \'%s\' updating HISTORY" % (instance), 'WARN')        
    except Exception, instance:
        casalog.post("*** Error \'%s\' " % (instance), 'SEVERE')
        retval = False
        
    return retval

def get_validcodes(code=None):
    """Returns a list of valid refcodes."""
    if not get_validcodes._hc:    # Is it not already cached?
        # Making it once per session is often enough.
        validcodes = me.listcodes(me.direction('J2000', '0','0'))
        get_validcodes._normal = validcodes['normal'].tolist()
        get_validcodes._extra  = validcodes['extra'].tolist()
    if code == 'extra':
        return get_validcodes._extra
    elif code == 'normal':
        return get_validcodes._normal
    else:
        return get_validcodes._normal + get_validcodes._extra
get_validcodes._hc = False

def is_valid_refcode(refcode):
    """Is refcode usable?"""
    return refcode in ['J2000', 'B1950', 'B1950_VLA', 'HADEC', 'ICRS'] \
            + get_validcodes('extra')

def log_phasecenter(oldnew, refstr, ra, dec):
    """Post a phase center to the logger, along with whether it is old or new."""
    casalog.post(oldnew + ' phasecenter RA, DEC ' + refstr + ' '
                 + qa.time(qa.quantity(ra, 'rad'), 10)[0] # 10 digits precision
                 + " " + qa.angle(qa.quantity(dec, 'rad'), 10)[0], 'NORMAL')
    casalog.post('          RA, DEC (rad) ' + refstr + ' '
                 + str(ra) + " " + str(dec), 'NORMAL')

def get_oldref(outputvis, tbt):
    """Returns the original reference code, string, ckwdict, and isvarref."""
    theoldref = -1
    theoldrefstr = ''    
    tbt.open(outputvis + "/FIELD")
    ckwdict = tbt.getcolkeyword('PHASE_DIR', 'MEASINFO')
    flddict = {}
    colstoget = ['PHASE_DIR', 'NAME']
    if ckwdict.has_key('TabRefTypes') and ckwdict.has_key('TabRefCodes'):
        colstoget.append('PhaseDir_Ref')
    for c in colstoget:
        flddict[c] = tbt.getcol(c)
    if flddict['PHASE_DIR'].shape[1] > 1:
        casalog.post('The input PHASE_DIR column is of order '
                     + str(flddict['PHASE_DIR'].shape[1] - 1), 'WARN')
        casalog.post('Orders > 0 are poorly tested.', 'WARN')
    flddict['PHASE_DIR'] = flddict['PHASE_DIR'].transpose((2, 0, 1))
    tbt.close()
    if(ckwdict.has_key('TabRefTypes') and ckwdict.has_key('TabRefCodes')):
        isvarref = True
    else:
        isvarref = False
        theoldrefstr = ckwdict['Ref']
    return theoldref, theoldrefstr, ckwdict, isvarref, flddict

def modify_fld_vis(fld, outputvis, tbt, myim, commonoldrefstr, phasecenter,
                   therefcode, reuse, numfields, ckwdict, theoldref,
                   theoldrefstr, isvarref, flddict, datacol, allselected,
                   thedistances):
    """Modify the UVW and visibilities of field fld."""
    viaoffset = False
    thenewra_rad = 0.
    thenewdec_rad = 0.
    thenewref = -1
    theolddir = flddict['PHASE_DIR'][fld]
    fieldname = flddict['NAME'][fld]
    if(ckwdict.has_key('TabRefTypes') and ckwdict.has_key('TabRefCodes')):
        # determine string name of the phase dir reference frame
        theoldref = flddict['PhaseDir_Ref'][fld]
        refcodestrlist = ckwdict['TabRefTypes'].tolist()
        refcodelist = ckwdict['TabRefCodes'].tolist()
        if not (theoldref in refcodelist):
            casalog.post('Invalid refcode in FIELD column PhaseDir_Ref: ' +
                         str(theoldref), 'SEVERE')
            return False
        theoldrefstr = refcodestrlist[refcodelist.index(theoldref)]

    if not isvarref:
        if not (commonoldrefstr == ''):
            theoldrefstr = commonoldrefstr
        else:
            commonoldrefstr = theoldrefstr

    
    theoldphasecenter = theoldrefstr + ' ' + \
                        qa.time(qa.quantity(theolddir[0], 'rad'), 14)[0] + ' ' + \
                        qa.angle(qa.quantity(theolddir[1],'rad'), 14)[0]

    if not is_valid_refcode(theoldrefstr):
        casalog.post('Refcode for FIELD column PHASE_DIR is valid but not yet supported: '
                     + theoldrefstr, 'WARN')
        casalog.post('Output MS may not be valid.', 'WARN')

    casalog.post('field: ' + fieldname, 'NORMAL')
    log_phasecenter('old', theoldrefstr, theolddir[0], theolddir[1])
            
    if(therefcode != 'J2000'):
        casalog.post(
                 "When changing phase center, can only write new UVW coordinates in J2000.",
                 'WARN')
        therefcode = 'J2000'
    if reuse:
        casalog.post("When changing phase center, UVW coordinates will be recalculated.",
                     'NORMAL')
        reuse = False

    dirstr = parse_phasecenter(phasecenter, isvarref, theoldref, theoldrefstr, theolddir)
    if not dirstr:
        return False

    if(isvarref):
        thenewrefindex = ckwdict['TabRefTypes'].tolist().index(dirstr[0])
        thenewref = ckwdict['TabRefCodes'][thenewrefindex]
        thenewrefstr = dirstr[0]
    else: # not a variable ref col
        validcodes = get_validcodes()
        if dirstr[0] in validcodes:
            thenewref = validcodes.index(dirstr[0])
            thenewrefstr = dirstr[0]
        else:
            casalog.post('Invalid refcode ' + dirstr[0], 'SEVERE')
            return False
        if(dirstr[0] != ckwdict['Ref']):
            if numfields > 1 and not allselected:
                        casalog.post("You have not selected all " + str(numfields)
                      + " fields and PHASE_DIR is not a variable reference column.\n"
                      + " Please use split or provide phase dir in " + ckwdict['Ref']
                                     + ".", 'SEVERE')
                        return False
            else:
                casalog.post(
            "The direction column reference frame in the FIELD table will be changed from "
                             + ckwdict['Ref'] + " to " + dirstr[0], 'NORMAL')
    #endif isvarref

    try:
        thedir = me.direction(thenewrefstr, dirstr[1], dirstr[2])
        thenewra_rad = thedir['m0']['value']
        thenewdec_rad = thedir['m1']['value']
    except Exception, instance:
        casalog.post("*** Error \'%s\' when interpreting parameter \'phasecenter\': "
                     % (instance), 'SEVERE')
        return False 

    if not is_valid_refcode(thenewrefstr):
        casalog.post('Refcode for the new phase center is valid but not yet supported: '
                     + thenewrefstr, 'WARN')
        casalog.post('Output MS may not be valid.', 'WARN')

    if(theolddir[0] >= qa.constants('pi')['value']): # old RA uses range 0 ... 2 pi, not -pi ... pi
        while (thenewra_rad < 0.): # keep RA positive in order not to confuse the user
            thenewra_rad += 2. * qa.constants('pi')['value']
                
    log_phasecenter('new', thenewrefstr, thenewra_rad, thenewdec_rad)

    # modify FIELD table                
    tbt.open(outputvis + '/FIELD', nomodify=False)
    pcol = tbt.getcol('PHASE_DIR')
    pcol[0][0][fld] = thenewra_rad
    pcol[1][0][fld] = thenewdec_rad
    tbt.putcol('PHASE_DIR', pcol)
        
    casalog.post("FIELD table PHASE_DIR column changed for field " + str(fld) + ".",
                 'NORMAL')

    if(thenewref != -1):
        # modify reference of the phase dir column; check also the
        # other direction columns
        theoldref2 = -1
        theoldref3 = -1
        if(isvarref):
            pcol = tbt.getcol('PhaseDir_Ref')
            #theoldref was already determined further above
            #theoldref = pcol[fld]
            pcol[fld] = thenewref
            
            pcol2 = tbt.getcol('DelayDir_Ref')
            theoldref2 = pcol2[fld]
            
            pcol3 = tbt.getcol('RefDir_Ref')
            theoldref3 = pcol3[fld]
                
            if(theoldref != thenewref):
                tbt.putcol('PhaseDir_Ref', pcol)
                casalog.post(
                    "FIELD table phase center direction reference frame for field "
                    + str(fld) + " set to " + str(thenewref) + " ("
                    + thenewrefstr + ")", 'NORMAL')
                if not (thenewref == theoldref2 and thenewref == theoldref3):
                    casalog.post(
            "*** The three FIELD table direction reference frame entries for field "
                                 + str(fld)
                                 + " will not be identical in the output data: "
                                 + str(thenewref) + ", " + str(theoldref2) + ", "
                                 + str(theoldref3), 'NORMAL')
                    if not (theoldref == theoldref2 and theoldref == theoldref3):
                        casalog.post(
            "*** The three FIELD table direction reference frame entries for field "
                                     + str(fld)
                                     + " were not identical in the input data either: "
                                     + str(theoldref) + ", " + str(theoldref2)
                                     + ", " + str(theoldref3), 'NORMAL')
            else:
                casalog.post(
            "FIELD table direction reference frame entries for field  " + str(fld)
                             + " unchanged.", 'NORMAL')

        else: # not a variable reference column
            tmprec = tbt.getcolkeyword('PHASE_DIR', 'MEASINFO')
            if(theoldrefstr != thenewrefstr):
                tmprec['Ref'] = thenewrefstr
                tbt.putcolkeyword('PHASE_DIR', 'MEASINFO', tmprec) 
                casalog.post(
                    "FIELD table phase center direction reference frame changed from "
                             + theoldrefstr + " to " + thenewrefstr, 'NORMAL')
    tbt.close()
    
    fldids = []
    phdirs = []
    for i in xrange(numfields):
        if (i==fld):
            fldids.append(i)
            phdirs.append(theoldphasecenter)

    if thedistances==[]:
        thedistances = 0. # the default value

    # 
    myim.open(outputvis, usescratch=False)
    myim.fixvis(fields=fldids, phasedirs=phdirs, refcode=therefcode, datacolumn=datacol, distances=thedistances)
    myim.close()
    return commonoldrefstr

def parse_phasecenter(phasecenter, isvarref, ref, refstr, theolddir):
    dirstr = phasecenter.split(' ')
    if len(dirstr) == 2: # interpret phasecenter as an offset
        casalog.post("No equinox given in parameter \'phasecenter\': "
                     + phasecenter, 'NORMAL')         
        casalog.post("Interpreting it as pair of offsets in (RA,DEC) ...",
                     'NORMAL')

        if(isvarref and ref > 31):
            casalog.post('*** Refcode in FIELD column PhaseDir_Ref is a solar system object: '
                         + refstr, 'NORMAL')
            casalog.post(
        '*** Will use the nominal entry in the PHASE_DIR column to calculate new phase center',
                         'NORMAL')
                    
        qra = qa.quantity(theolddir[0], 'rad') 
        qdec = qa.quantity(theolddir[1], 'rad')
        qraoffset = qa.quantity(dirstr[0])
        qdecoffset = qa.quantity(dirstr[1])
        if not qa.isangle(qdecoffset):
            casalog.post("Invalid phasecenter parameter. DEC offset must be an angle.",
                         'SEVERE')
            return False
        qnewdec = qa.add(qdec,qdecoffset)
        qnewra = qra
        ishms = (qa.canonical(qraoffset)['unit'] == 'rad') and (('h' in dirstr[0] and 'm' in dirstr[0] and 's' in dirstr[0]) or (dirstr[0].count(':')==2))
        if (qa.canonical(qraoffset)['unit'] == 'rad') and not ishms:
            casalog.post(
            "RA offset is an angle (not a time). Will divide by cos(DEC) to compute time offset.")
            if(qa.cos(qnewdec)['value'] == 0.):
                casalog.post(
                   "Resulting DEC is at celestial pole. Will ignore RA offset.", 'WARN')
            else:
                qraoffset = qa.div(qraoffset, qa.cos(qnewdec))
                qnewra = qa.add(qnewra, qraoffset)
        else:
            if not ((qa.canonical(qraoffset)['unit'] == 's') or ishms):
                casalog.post("Invalid phasecenter parameter. RA offset must be an angle or a time.",
                             'SEVERE')
                return False
            # RA offset was given as a time; apply as is
            qraoffset = qa.convert(qraoffset, 'deg')
            qnewra = qa.add(qnewra, qraoffset)

        dirstr = [refstr, qa.time(qnewra,12)[0], qa.angle(qnewdec,12)[0]]

    elif not len(dirstr) == 3:
        casalog.post('Incorrectly formatted parameter \'phasecenter\': '
                     + phasecenter, 'SEVERE')
        return False
    return dirstr
