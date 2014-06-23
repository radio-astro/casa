from taskinit import *
import shutil
from parallel.parallel_task_helper import ParallelTaskHelper
import recipes.ephemerides.JPLephem_reader2 as jplreader

def fixplanets(vis, field, fixuvw=False, direction='', refant=0, reftime='first'):
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

    direction  -- if set, don't use pointing table but set direction to this value.
                  The direction can either be given explicitly or as the path
                  to a JPL Horizons ephemeris (for an example of the format,
                  see directory data/ephemerides/JPL-Horizons/).
                  Alternatively, the ephemeris table can also be provided as mime format file,
                  i.e. a saved email as obtained via the commands (for example):
                  import recipes.ephemerides.request as jplreq
                  jplreq.request_from_JPL(objnam='Mars',startdate='2012-01-01',enddate='2014-12-31',
                       date_incr='0.1 d', get_axis_orientation=False, get_axis_ang_orientation=True,
                       get_sub_long=True, use_apparent=False, get_sep=False,
                       return_address='YOUR_EMAIL_ADDESS', mailserver='YOUR_MAIL_SERVER_ADDRESS')
                  example: 'J2000 19h30m00 -40d00m00', default= '' (use pointing table)

    refant     -- if using pointing table information, use it from this antenna
                  default: 0 (antenna id 0)
                  examples: 'DV06' (antenna with name DV06)
                            3 (antenna id 3)

    reftime    -- if using pointing table information, use it from this timestamp
                  default: 'first'
                  examples: 'median' will use the median timestamp for the given field
		              using only the unflagged maintable rows 
                            '2012/07/11/08:41:32' will use the given timestamp (must be
                            within the observaton time)
    """
    
    casalog.origin('fixplanets')

    mst = mstool()
    tbt = tbtool()
    imt = None

    try:
        fields = mst.msseltoindex(vis=vis,field=field)['field']
        numfields = 0 
        if(len(fields) == 0):
            casalog.post( "Field selection returned zero results.", 'WARN')
            return True

        tbt.open(vis+"/FIELD")
        oldrefcol = []
        if('PhaseDir_Ref' in tbt.colnames()):
            oldrefcol = tbt.getcol('PhaseDir_Ref')
        tbt.close()

        fixplanetstemp = 'fixplanetstemp-'+os.path.basename(vis) # temp file name
        fixplanetstemp2 = 'fixplanetstemp2-'+os.path.basename(vis) # temp file name
        
        for fld in fields:
            thenewra_rad = 0.
            thenewdec_rad = 0.
            thenewref = -1
            thenewrefstr = ''
            theephemeris = ''
            
            if(direction==''): # use information from the pointing table
                # find median timestamp for this field in the main table
                shutil.rmtree(fixplanetstemp, ignore_errors=True)
                thetime = 0
                if(reftime.lower()=='median'):
                    tbt.open(vis)
                    tttb = tbt.query('FIELD_ID=='+str(fld)+' AND FLAG_ROW==False',
                              name=fixplanetstemp, columns='TIME')
                    tttb.close()
                    tttb = None
                    tbt.close()
                    tbt.open(fixplanetstemp)
                    if(tbt.nrows()>0):
                        thetime = tbt.getcell('TIME',tbt.nrows()/2)
                        casalog.post( "MEDIAN TIME "+str(thetime), 'NORMAL')
                        tbt.close()
                    else:
                        casalog.post( "No pointing table rows for field "+field, 'NORMAL')
                        tbt.close()
                        shutil.rmtree(fixplanetstemp, ignore_errors=True)
                        return True
                elif(reftime.lower()=='first'):
                    tbt.open(vis)
                    tttb = tbt.query('FIELD_ID=='+str(fld), name=fixplanetstemp, columns='TIME')
                    tttb.close()
                    tttb = None
                    tbt.close()
                    tbt.open(fixplanetstemp)
                    if(tbt.nrows()>0):
                        thetime = tbt.getcell('TIME',0)
                        casalog.post( "FIRST TIME "+str(thetime), 'NORMAL')
                        tbt.close()
                    else:
                        casalog.post( "No pointing table rows for field "+field, 'NORMAL')
                        tbt.close()
                        shutil.rmtree(fixplanetstemp, ignore_errors=True)
                        return True
                else:
                    try:
                        myqa = qa.quantity(reftime)
                        thetime = qa.convert(myqa,'s')['value']
                    except Exception, instance:
                        raise TypeError, "reftime parameter is not a valid date (e.g. YYYY/MM/DD/hh:mm:ss)" %reftime
                    tbt.open(vis)
                    tttb = tbt.query('FIELD_ID=='+str(fld), name=fixplanetstemp, columns='TIME')
                    tttb.close()
                    tttb = None
                    tbt.close()
                    tbt.open(fixplanetstemp)
                    if(tbt.nrows()>0):
                        thefirsttime = tbt.getcell('TIME',0)
                        thelasttime = tbt.getcell('TIME',tbt.nrows()-1)
                        tbt.close()
                    else:
                        casalog.post( "No pointing table rows for field "+field, 'NORMAL')
                        tbt.close()
                        shutil.rmtree(fixplanetstemp, ignore_errors=True)
                        return True        
                    shutil.rmtree(fixplanetstemp, ignore_errors=True)
                    if (thefirsttime<=thetime and thetime<=thelasttime):
                        casalog.post( "GIVEN TIME "+reftime+" == "+str(thetime), 'NORMAL')
                    else:
                        casalog.post( "GIVEN TIME "+reftime+" == "+str(thetime)+" is not within the observation time ("
                                      +str(thefirsttime)+'s'+" - "
                                      +str(thelasttime)+'s'+")", 'SEVERE')
                        raise TypeError
                shutil.rmtree(fixplanetstemp, ignore_errors=True)

                # determine reference antenna
                antids = mst.msseltoindex(vis=vis,baseline=refant)['antenna1']
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
                shutil.rmtree(fixplanetstemp2, ignore_errors=True)
                tbt.open(vis+'/POINTING')
                ttb = tbt.query('TRACKING==True AND NEARABS(TIME,'+str(thetime)+',INTERVAL/2.) AND ANTENNA_ID=='
                                +str(antid),
                                name=fixplanetstemp2)
                nr = ttb.nrows()
                ttb.close()
                ttb = None
                if(nr==0):
                    shutil.rmtree(fixplanetstemp2, ignore_errors=True)
                    ttb2 = tbt.query('TRACKING==True AND NEARABS(TIME,'+str(thetime)+',3.) AND ANTENNA_ID=='
                                     +str(antid), # search within 3 seconds
                                     name=fixplanetstemp2)
                    nr = ttb2.nrows()
                    ttb2.close()
                    ttb2 = None
                    if(nr==0):
                        casalog.post( "Cannot find any POINTING table rows for antenna "+str(antid)
                                      +" with TRACKING==True within 3 seconds of TIME "+str(thetime), 'NORMAL')
                        casalog.post( "Will try without requiring TRACKING==True ...", 'NORMAL')
                        shutil.rmtree(fixplanetstemp2, ignore_errors=True)
                        ttb3 = tbt.query('NEARABS(TIME,'+str(thetime)+',INTERVAL/2.) AND ANTENNA_ID=='
                                         +str(antid), 
                                         name=fixplanetstemp2)
                        nr = ttb3.nrows()
                        ttb3.close()
                        ttb3 = None
                        if(nr==0):
                            shutil.rmtree(fixplanetstemp2, ignore_errors=True)
                            ttb4 = tbt.query('NEARABS(TIME,'+str(thetime)+',3.) AND ANTENNA_ID=='
                                             +str(antid), # search within 3 seconds
                                             name=fixplanetstemp2)
                            nr = ttb4.nrows()
                            ttb4.close()
                            ttb4 = None
                            if(nr==0):
                                tbt.close()
                                casalog.post( "Cannot find any POINTING table rows for antenna "+str(antid)
                                              +" within 3 seconds of TIME "+str(thetime), 'SEVERE')
                                return False # give up
                tbt.close()
                tbt.open(fixplanetstemp2)
                thedir = tbt.getcell('DIRECTION',0)
                tbt.close()
                casalog.post( ' field id '+str(fld)+ ' AZ EL '+str(thedir[0])+" "+str(thedir[1]), 'NORMAL')
                thedirme = me.direction(rf='AZELGEO',v0=qa.quantity(thedir[0][0], 'rad'),
                                        v1=qa.quantity(thedir[1][0],'rad'))
                # convert AZEL to J2000 coordinates
                me.doframe(me.epoch(rf='UTC', v0=qa.quantity(thetime,'s')))

                tbt.open(vis+'/ANTENNA')
                thepos = tbt.getcell('POSITION',antid)
                theposref = tbt.getcolkeyword('POSITION', 'MEASINFO')['Ref']
                theposunits =  tbt.getcolkeyword('POSITION', 'QuantumUnits')
                tbt.close()
                casalog.post( "Ref. antenna position is "+str(thepos)+' ('+theposunits[0]+', '+theposunits[1]+', '
                              +theposunits[2]+')('+theposref+')', 'NORMAL')
                me.doframe(me.position(theposref,
                                       qa.quantity(thepos[0],theposunits[0]),
                                       qa.quantity(thepos[1],theposunits[1]),
                                       qa.quantity(thepos[2],theposunits[2]))
                           )
                thedirmemod = me.measure(v=thedirme, rf='J2000')
                #print thedirmemod
                thenewra_rad = thedirmemod['m0']['value']
                thenewdec_rad = thedirmemod['m1']['value']
                me.done()
                shutil.rmtree(fixplanetstemp2, ignore_errors=True)

            else: # direction is not an empty string, use this instead of the pointing table information
                if(type(direction)==str):
                    dirstr = direction.split(' ')
                    if len(dirstr)==1: # an ephemeris table was given
                        if(os.path.exists(dirstr[0])):
                            if os.path.isfile(dirstr[0]): # it is a file, i.e. not a CASA table
                                try: # a mime file maybe?
                                    outdict=jplreader.readJPLephem(dirstr[0], '1.0')
                                    if not jplreader.ephem_dict_to_table(outdict, dirstr[0]+".tab"):
                                        raise ValueError, "Error converting dictionary to ephem table"
                                except: # no it is not a mime file either
                                    casalog.post("*** Error when interpreting parameter \'direction\':\n File given is not a valid JPL email mime format file.",
                                                 'SEVERE')
                                    return False
                                else:
                                    theephemeris = dirstr[0]+".tab"
                                    casalog.post('Successfully converted mime format ephemeris to table '+theephemeris+'.\n Will use it with offset (0,0)', 'NORMAL')
                            else: # not a file, assume it is a CASA table
                                theephemeris = dirstr[0]
                                casalog.post('Will use ephemeris table '+theephemeris+' with offset (0,0)', 'NORMAL')
                            
                            thenewra_rad = 0.
                            thenewdec_rad = 0.

                        else:
                            casalog.post("*** Error when interpreting parameter \'direction\':\n string is neither a direction nor an existing file or table.",
                                         'SEVERE')
                            return False
                    else:
                        if len(dirstr)==2: # a direction without ref frame was given
                            dirstr = ['J2000', dirstr[0], dirstr[1]]
                            casalog.post('Assuming reference frame J2000 for parameter \'direction\'', 'NORMAL')
                        elif not len(dirstr)==3:
                            casalog.post('Incorrect format in parameter \'direction\'', 'SEVERE')
                            return False
                        # process direction and refframe
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
            if(theephemeris==''):
                casalog.post( 'new RA, DEC (rad) '+str(thenewra_rad)+" "+ str(thenewdec_rad), 'NORMAL')
            else:
                casalog.post( 'new RA, DEC to be taken from ephemeris table '+theephemeris
                              +' with offsets (rad) '+str(thenewra_rad)+" "+ str(thenewdec_rad), 'NORMAL')

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

            if(theephemeris==''):
                
                if ('EPHEMERIS_ID' in tbt.colnames()) and (tbt.getcell('EPHEMERIS_ID',fld)>=0): # originally an ephemeris was used
                    eidc = tbt.getcol('EPHEMERIS_ID')
                    eidc[fld] = -1
                    tbt.putcol('EPHEMERIS_ID', eidc) # remove the reference to it
                    casalog.post("FIELD table EPHEMERIS_ID column reset to -1 for field "+str(fld)+".", 'NORMAL')

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
            else: # we are working with an ephemeris
                try:
                    mst.open(vis, nomodify=False)
                    mst.addephemeris(-1, theephemeris, planetname, fld) # -1 = take the next free ID
                    mst.close()
                except Exception, instance:
                    casalog.post("*** Error \'%s\' when attaching ephemeris: " % (instance),
                                     'SEVERE')
                    return False

            if(fixuvw and (oldrefcol!=[]) and (thenewref>0)): 
                # modify reference of phase dir for fixuvw
                pcol = tbt.getcol('PhaseDir_Ref')
                pcol[fld] = 0 # J2000
                tbt.putcol('PhaseDir_Ref', pcol) # J2000

            tbt.close()

            #modify SOURCE table
            newsra_rad = thenewra_rad
            newsdec_rad = thenewdec_rad
            if(theephemeris!=''): # get the nominal position from the ephemeris
                mst.open(vis)
                trec = mst.getfielddirmeas('PHASE_DIR',fld)
                newsra_rad = trec['m0']['value']
                newsdec_rad = trec['m1']['value']
                mst.close()
            tbt.open(vis+'/SOURCE', nomodify=False)
            sdir = tbt.getcol('DIRECTION')
            newsdir = sdir
            sname = tbt.getcol('NAME')
                
            for i in xrange(0,tbt.nrows()):
                if(sname[i]==planetname):
                    #print 'i old dir ', i, " ", sdir[0][i], sdir[1][i]
                    newsdir[0][i] = newsra_rad
                    newsdir[1][i] = newsdec_rad
                    #print '  new dir ', newsdir[0][i], newsdir[1][i]
            tbt.putcol('DIRECTION', newsdir)
            tbt.close()
            casalog.post("SOURCE table DIRECTION column changed.", 'NORMAL')

        # end for

        if(fixuvw):
            casalog.post("Fixing the UVW coordinates ...", 'NORMAL')

            # similar to fixvis

            fldids = []
            for i in xrange(numfields):
                if (i in fields):
                    fldids.append(i)
                    
            imt = imtool()
            imt.open(vis, usescratch=False)
            imt.calcuvw(fldids, refcode='J2000', reuse=False)
            imt.close()
            imt = None

            if((oldrefcol!=[]) and (thenewref>0)): 
                tbt.open(vis+'/FIELD', nomodify=False)
                tbt.putcol('PhaseDir_Ref', oldrefcol)
                tbt.close()

        else:
            casalog.post("UVW coordinates not changed.", 'NORMAL')

        if (ParallelTaskHelper.isParallelMS(vis)):
            casalog.post("Tidying up the MMS subtables ...", 'NORMAL')
            ParallelTaskHelper.restoreSubtableAgreement(vis)

        mst = None
        tbt = None

        return True

    except Exception, instance:
        mst = None
        tbt = None
        imt = None
        casalog.post("*** Error \'%s\' " % (instance), 'SEVERE')
        return False

