from taskinit import *
import shutil

def fixplanets(vis, field, fixuvw=False, direction='', refant=0):
    """
    Fix FIELD, SOURCE, and UVW for given fields based on given direction or pointing
    table information

    This task's main purpose is to correct observations which were performed
    with correct pointing and correlation but for which incorrect direction
    information was entered in the FIELD and SOURCE table of the MS.
    If you actually want to change the phase center of the visibilties in an MS,
    you should use task fixvis.

    Input Parameters
    vis        -- Name of the input visibility set
    
    field      -- field selection string
    
    fixuvw     -- recalc uvw coordinates? (default: False)

    direction  -- if set, don't use pointing table but set direction to this value
               example: 'J2000 19h30m00 -40d00m00', default= '' (use pointing table)

    refant     -- if using pointing table information, use it from this antenna
                  default: 0 (antenna id 0)
                  examples: 'DV06' (antenna with name DV06)
                            3 (antenna id 3)

    Examples:

    fixplanets('uid___A002_X1c6e54_X223.ms', 'Titan', True)
          will look up the pointing direction for field 'Titan' in the POINTING table
          based on the median time in unflagged main table rows for this field,
          enter this direction in the FIELD and SOURCE tables, and then recalculate
          the UVW coordinates for this field.

    fixplanets('uid___A002_X1c6e54_X223.ms', 'Titan', False, 'J2000 12h30m15 -02d12m00')
          will set the directions for field 'Titan' in the FIELD and SOURCE table to the
          given direction and not recalculate the UVW coordinates.
          (This can be useful for several purposes, among them preparing a concatenation
          of datasets. Only fields with the same direction will be recognised as identical.
          fixplanets can then be run again after the concatenation using parameters as in
          the first example above.)

    """

    tbt = casac.homefinder.find_home_by_name('tableHome').create()

    casalog.origin('fixplanets')
    try:
        fields = ms.msseltoindex(vis=vis,field=field)['field']
        numfields = 0 
        if(len(fields) == 0):
            casalog.post( "Field selection returned zero results.", 'WARN')
            return False
        
        tbt.open(vis+"/FIELD")
        oldrefcol = []
        if('PhaseDir_Ref' in tbt.colnames()):
            oldrefcol = tbt.getcol('PhaseDir_Ref')
        tbt.close()
        
        for fld in fields:
            thenewra_rad = 0.
            thenewdec_rad = 0.
            thenewref = -1
            thenewrefstr = ''
            if(direction==''): # use information from the pointing table
                # find median timestamp for this field in the main table
                shutil.rmtree('fixplanetstemp', ignore_errors=True)
                tbt.open(vis)
                tbt.query('FIELD_ID=='+str(fld)+' AND FLAG_ROW==False', name='fixplanetstemp', columns='TIME')
                tbt.close()
                tbt.open('fixplanetstemp')
                thetime = tbt.getcell('TIME',tbt.nrows()/2)
                casalog.post( "TIME "+str(thetime), 'NORMAL')
                tbt.close()

                # determine reference antenna
                antids = ms.msseltoindex(vis=vis,baseline=refant)['antenna1']
                antid = -1
                if(len(antids) == 0):
                    casalog.post( "Antenna selection returned zero results.", 'WARN')
                    return False
                antid = int(antids[0])
                casalog.post('Using antenna id '+str(antid)+' as reference antenna.', 'NORMAL') 
                tbt.open(vis+'/ANTENNA')
                flgcol = tbt.getcol('FLAG_ROW')
                tbt.close()
                if(flgcol[antid]==True):
                    casalog.post('Antenna id '+str(antid)+' is flagged. Please select a different one.', 'SEVERE')
                    return False

                # get direction for the timestamp from pointing table
                tbt.open(vis+'/POINTING')
                ttb = tbt.query('TRACKING==True AND NEARABS(TIME,'+str(thetime)+',INTERVAL/2.) AND ANTENNA_ID=='
                                +str(antid),
                                name='fixplanetstemp2')
                nr = ttb.nrows()
                ttb.close()
                if(nr==0):
                    shutil.rmtree('fixplanetstemp2', ignore_errors=True)
                    ttb2 = tbt.query('TRACKING==True AND NEARABS(TIME,'+str(thetime)+',3.) AND ANTENNA_ID=='
                                     +str(antid), # search within 3 seconds
                                     name='fixplanetstemp2')
                    nr = ttb2.nrows()
                    ttb2.close()
                    if(nr==0):
                        casalog.post( "Cannot find any POINTING table rows for antenna "+str(antid)+" with TRACKING==True within 3 seconds of TIME "+str(thetime), 'NORMAL')
                        casalog.post( "Will try without requiring TRACKING==True ...", 'NORMAL')
                        shutil.rmtree('fixplanetstemp2', ignore_errors=True)
                        ttb3 = tbt.query('NEARABS(TIME,'+str(thetime)+',INTERVAL/2.) AND ANTENNA_ID=='
                                         +str(antid), 
                                         name='fixplanetstemp2')
                        nr = ttb3.nrows()
                        ttb3.close()
                        if(nr==0):
                            shutil.rmtree('fixplanetstemp2', ignore_errors=True)
                            ttb4 = tbt.query('NEARABS(TIME,'+str(thetime)+',3.) AND ANTENNA_ID=='
                                             +str(antid), # search within 3 seconds
                                             name='fixplanetstemp2')
                            nr = ttb4.nrows()
                            ttb4.close()
                            if(nr==0):
                                tbt.close()
                                casalog.post( "Cannot find any POINTING table rows for antenna "+str(antid)+" within 3 seconds of TIME "+str(thetime), 'SEVERE')
                                return False # give up
                tbt.close()
                tbt.open('fixplanetstemp2')
                thedir = tbt.getcell('DIRECTION',0)
                tbt.close()
                casalog.post( ' field id '+str(fld)+ ' AZ EL '+str(thedir[0])+" "+str(thedir[1]), 'NORMAL')
                thedirme = me.direction(rf='AZELGEO',v0=qa.quantity(thedir[0][0], 'rad'), v1=qa.quantity(thedir[1][0],'rad'))
                # convert AZEL to J2000 coordinates
                me.doframe(me.epoch(rf='UTC', v0=qa.quantity(thetime,'s')))
                tbt.open(vis+'/OBSERVATION')
                theobs = tbt.getcell('TELESCOPE_NAME',0)
                tbt.close()
                casalog.post( "Observatory is " + theobs, 'NORMAL')
                me.doframe(me.observatory(theobs))
                thedirmemod = me.measure(v=thedirme, rf='J2000')
                #print thedirmemod
                thenewra_rad = thedirmemod['m0']['value']
                thenewdec_rad = thedirmemod['m1']['value']
                me.done()
                shutil.rmtree('fixplanetstemp*', ignore_errors=True)
            else: # direction is not an empty string, use this instead of the pointing table information
                if(type(direction)==str):
                    dirstr = direction.split(' ')
                    codes = []
                    if len(dirstr)==2:
                        dirstr = ['J2000', dirstr[0], dirstr[1]]
                        casalog.post('Assuming reference frame J2000 for parameter \'direction\'', 'NORMAL')
                    elif not len(dirstr)==3:
                        casalog.post('Incorrect format in parameter \'direction\'', 'SEVERE')
                        return False
                    try:
                        thedir = me.direction(dirstr[0], dirstr[1], dirstr[2])
                        thenewra_rad = thedir['m0']['value']
                        thenewdec_rad = thedir['m1']['value']
                    except Exception, instance:
                        casalog.post("*** Error \'%s\' when interpreting parameter \'direction\': " % (instance), 'SEVERE')
                        return False
                    try:
                        tbt.open(vis+"/FIELD")
                        thenewrefindex = tbt.getcolkeyword('PHASE_DIR', 'MEASINFO')['TabRefTypes'].tolist().index(dirstr[0])
                        thenewref = tbt.getcolkeyword('PHASE_DIR', 'MEASINFO')['TabRefCodes'][thenewrefindex]
                        thenewrefstr = dirstr[0]
                        tbt.close()
                    except Exception, instance:
                        casalog.post("PHASE_DIR is not a variable reference column. Will leave reference as is.", 'WARN')
                        thenewref = -1
                    if(0<thenewref and thenewref<32):
                        casalog.post("*** Error when interpreting parameter \'direction\':\n presently only J2000 and solar system objects are supported.",
                                     'SEVERE')
                        return False
            #endif
            
            # modify FIELD table
            tbt.open(vis+'/FIELD', nomodify=False)
            numfields = tbt.nrows()
            theolddir = tbt.getcell('PHASE_DIR',fld)
            planetname = tbt.getcell('NAME',fld)
            casalog.post( 'object: '+planetname, 'NORMAL')
            casalog.post( 'old RA, DEC (rad) '+str(theolddir[0])+" " +str(theolddir[1]), 'NORMAL')
            casalog.post( 'new RA, DEC (rad) '+str(thenewra_rad)+" "+ str(thenewdec_rad), 'NORMAL')

            pcol = tbt.getcol('PHASE_DIR')
            pcol[0][0][fld] = thenewra_rad
            pcol[1][0][fld] = thenewdec_rad
            tbt.putcol('PHASE_DIR',pcol)

            pcol = tbt.getcol('DELAY_DIR')
            pcol[0][0][fld] = thenewra_rad
            pcol[1][0][fld] = thenewdec_rad
            tbt.putcol('DELAY_DIR',pcol)

            pcol = tbt.getcol('REFERENCE_DIR')
            pcol[0][0][fld] = thenewra_rad
            pcol[1][0][fld] = thenewdec_rad
            tbt.putcol('REFERENCE_DIR',pcol)
            casalog.post("FIELD table PHASE_DIR, DELAY_DIR, and REFERENCE_DIR columns changed for field "+str(fld)+".", 'NORMAL')

            if(thenewref!=-1):
                # modify reference of the direction columns permanently
                theoldref = -1
                theoldref2 = -1
                theoldref3 = -1
                try:
                    pcol = tbt.getcol('PhaseDir_Ref')
                    theoldref = pcol[fld]
                    pcol[fld] = thenewref
                    oldrefcol = pcol
                        
                    pcol2 = tbt.getcol('DelayDir_Ref')
                    theoldref2 = pcol2[fld]
                    pcol2[fld] = thenewref
                    
                    pcol3 = tbt.getcol('RefDir_Ref')
                    theoldref3 = pcol3[fld]
                    pcol3[fld] = thenewref

                    if not (theoldref==theoldref2 and theoldref==theoldref3):
                        casalog.post("The three FIELD table direction reference frame entries for field "+str(fld)
                                     +" are not identical in the input data: "
                                     +str(theoldref)+", "+str(theoldref2)+", "+str(theoldref3)
                                     +". Will try to continue ...", 'WARN')
                        theoldref=-1
                    else:
                        oldrefstrindex = tbt.getcolkeyword('PHASE_DIR', 'MEASINFO')['TabRefCodes'].tolist().index(theoldref)
                        oldrefstr = tbt.getcolkeyword('PHASE_DIR', 'MEASINFO')['TabRefTypes'][oldrefstrindex]
                        casalog.post("Original FIELD table direction reference frame entries for field "+str(fld)
                                     +" are all "+str(theoldref)+" ("+oldrefstr+")", 'NORMAL')
                    tbt.putcol('PhaseDir_Ref', pcol)
                    tbt.putcol('DelayDir_Ref', pcol2)
                    tbt.putcol('RefDir_Ref', pcol3)

                except Exception, instance:
                    casalog.post("*** Error \'%s\' when writing reference frames in FIELD table: " % (instance),
                                 'SEVERE')
                    return False
                if(theoldref != thenewref):
                    casalog.post("FIELD table direction reference frame entries for field "+str(fld)
                                 +" set to "+str(thenewref)+" ("+thenewrefstr+")", 'NORMAL')
                else:
                    casalog.post("FIELD table direction reference frame entries for field "+str(fld)
                                 +" unchanged.", 'NORMAL')                        
                

            if(fixuvw and (oldrefcol!=[]) and (thenewref>0)): 
                # modify reference of phase dir for fixuvw
                pcol = tbt.getcol('PhaseDir_Ref')
                pcol[fld] = 0 # J2000
                tbt.putcol('PhaseDir_Ref', pcol) # J2000

            tbt.close()

            #modify SOURCE table
            tbt.open(vis+'/SOURCE', nomodify=False)
            sdir = tbt.getcol('DIRECTION')
            newsdir = sdir
            sname = tbt.getcol('NAME')
            for i in xrange(0,tbt.nrows()):
                if(sname[i]==planetname):
                    #print 'i old dir ', i, " ", sdir[0][i], sdir[1][i]
                    newsdir[0][i] = thenewra_rad
                    newsdir[1][i] = thenewdec_rad
                    #print '  new dir ', newsdir[0][i], newsdir[1][i]
            tbt.putcol('DIRECTION', newsdir)
            tbt.close()
            casalog.post("SOURCE table DIRECTION column changed.", 'NORMAL')
                
        if(fixuvw):
            casalog.post("Fixing the UVW coordinates ...", 'NORMAL')

            # similar to fixvis

            fldids = []
            for i in xrange(numfields):
                if (i in fields):
                    fldids.append(i)

            im.open(vis, usescratch=True) # usescratch=True needed in order to have writable ms
            im.calcuvw(fldids, refcode='J2000', reuse=False)
            im.close()

            if((oldrefcol!=[]) and (thenewref>0)): 
                tbt.open(vis+'/FIELD', nomodify=False)
                tbt.putcol('PhaseDir_Ref', oldrefcol)
                tbt.close()

        else:
            casalog.post("UVW coordinates not changed.", 'NORMAL')

        return True

    except Exception, instance:
        casalog.post("*** Error \'%s\' " % (instance), 'SEVERE')
        return False

