def fixplanets(vis, field, fixuvw=False):
    """
    Fix FIELD, SOURCE, and UVW for given fields based on pointing table information

    Input Parameters
    vis    --- Name of the input visibility set
    
    field  --- field selection string
    
    fixuvw --- recalc uvw coordinate? (default: False)

    Example:

    fixplanets('uid___A002_X1c6e54_X223.ms', 'Titan', True)

    """

    mytb = casac.homefinder.find_home_by_name('tableHome').create()

    casalog.origin('fixplanets')
    try:
        fields = ms.msseltoindex(vis=vis,field=field)['field']
        numfields = 0 
        if(len(fields) == 0):
            casalog.post( "Field selection returned zero results.", 'WARN')
            return
        mytb.open(vis+"/FIELD")
        oldrefcol = []
        if('PhaseDir_Ref' in mytb.colnames()):
            oldrefcol = mytb.getcol('PhaseDir_Ref')
        mytb.close()
        for fld in fields:
            os.system('rm -rf fixplanetstemp')
            mytb.open(vis)
            mytb.query('FIELD_ID=='+str(fld)+' AND FLAG_ROW==False', name='fixplanetstemp', columns='TIME')
            mytb.close()
            mytb.open('fixplanetstemp')
            mytime = mytb.getcell('TIME',mytb.nrows()/2)
            casalog.post( "TIME "+str(mytime), 'NORMAL')
            mytb.close()

            mytb.open(vis+'/POINTING')
            ttb = mytb.query('TRACKING==True AND NEARABS(TIME,'+str(mytime)+',INTERVAL/2.)',
                             name='fixplanetstemp2')
            if(ttb.nrows()==0):
                ttb.close()
                os.system('rm -rf fixplanetstemp2')
                ttb2 = mytb.query('TRACKING==True AND NEARABS(TIME,'+str(mytime)+',3.)', # search within 3 seconds
                                  name='fixplanetstemp2')
                if(ttb2.nrows()==0):
                    ttb2.close()
                    casalog.post( "Cannot find any POINTING table rows with TRACKING==True within 3 seconds of TIME "+str(mytime), 'NORMAL')
                    casalog.post( "Will try without requiring TRACKING==True ...", 'NORMAL')
                    os.system('rm -rf fixplanetstemp2')
                    ttb3 = mytb.query('NEARABS(TIME,'+str(mytime)+',INTERVAL/2.)', 
                                      name='fixplanetstemp2')
                    if(ttb3.nrows()==0):
                        ttb3.close()
                        os.system('rm -rf fixplanetstemp2')
                        ttb4 = mytb.query('NEARABS(TIME,'+str(mytime)+',3.)', # search within 3 seconds
                                          name='fixplanetstemp2')
                        if(ttb4.nrows()==0):
                            ttb4.close()
                            mytb.close()
                            casalog.post( "Cannot find any POINTING table rows within 3 seconds of TIME "+str(mytime), 'SEVERE')
                            return # give up
            else:
                ttb.close()
            mytb.close()
            mytb.open('fixplanetstemp2')
            mydir = mytb.getcell('DIRECTION',0)
            mytb.close()
            casalog.post( ' field id '+str(fld)+ ' AZ EL '+str(mydir[0])+" "+str(mydir[1]), 'NORMAL')
            mydirme = me.direction(rf='AZELGEO',v0=qa.quantity(mydir[0][0], 'rad'), v1=qa.quantity(mydir[1][0],'rad'))
            #print mydirme
            me.doframe(me.epoch(rf='UTC', v0=qa.quantity(mytime,'s')))
            mytb.open(vis+'/OBSERVATION')
            myobs = mytb.getcell('TELESCOPE_NAME',0)
            mytb.close()
            casalog.post( "Observatory is " + myobs, 'NORMAL')
            me.doframe(me.observatory(myobs))
            mydirmemod = me.measure(v=mydirme, rf='J2000')
            #print mydirmemod
            mynewra_rad = mydirmemod['m0']['value']
            mynewdec_rad = mydirmemod['m1']['value']
            me.done()
            
            # modify FIELD table
            mytb.open(vis+'/FIELD', nomodify=False)
            numfields = mytb.nrows()
            myolddir = mytb.getcell('PHASE_DIR',fld)
            planetname = mytb.getcell('NAME',fld)
            casalog.post( 'object: '+planetname, 'NORMAL')
            casalog.post( 'old RA, DEC (rad) '+str(myolddir[0])+" " +str(myolddir[1]), 'NORMAL')
            casalog.post( 'new RA, DEC (rad) '+str(mynewra_rad)+" "+ str(mynewdec_rad), 'NORMAL')

            pcol = mytb.getcol('PHASE_DIR')
            pcol[0][0][fld] = mynewra_rad
            pcol[1][0][fld] = mynewdec_rad
            mytb.putcol('PHASE_DIR',pcol)

            pcol = mytb.getcol('DELAY_DIR')
            pcol[0][0][fld] = mynewra_rad
            pcol[1][0][fld] = mynewdec_rad
            mytb.putcol('DELAY_DIR',pcol)

            pcol = mytb.getcol('REFERENCE_DIR')
            pcol[0][0][fld] = mynewra_rad
            pcol[1][0][fld] = mynewdec_rad
            mytb.putcol('REFERENCE_DIR',pcol)

            # modify reference of phase dir for fixuvw
            if (fixuvw and not oldrefcol==[]): 
                pcol = mytb.getcol('PhaseDir_Ref')
                pcol[fld] = 0 # J2000
                mytb.putcol('PhaseDir_Ref', pcol) # J2000

            mytb.close()

            #modify SOURCE table
            mytb.open(vis+'/SOURCE', nomodify=False)
            sdir = mytb.getcol('DIRECTION')
            newsdir = sdir
            sname = mytb.getcol('NAME')
            for i in xrange(0,mytb.nrows()):
                if(sname[i]==planetname):
                    #print 'i old dir ', i, " ", sdir[0][i], sdir[1][i]
                    newsdir[0][i] = mynewra_rad
                    newsdir[1][i] = mynewdec_rad
                    #print '  new dir ', newsdir[0][i], newsdir[1][i]
            mytb.putcol('DIRECTION', newsdir)
            mytb.close()
                
        os.system('rm -rf fixplanetstemp*')

        if(fixuvw):
            casalog.post("Fixing the UVW coordinates ...", 'NORMAL')

            # similar to fixvis

            fldids = []
            for i in xrange(numfields):
                if (i in fields):
                    fldids.append(i)
                else:
                    fldids.append(-1)

            im.open(vis, usescratch=True) # usescratch=True needed in order to have writable ms
            im.calcuvw(fldids, refcode='J2000', reuse=False)
            im.close()

            if not oldrefcol==[]: 
                mytb.open(vis+'/FIELD', nomodify=False)
                mytb.putcol('PhaseDir_Ref', oldrefcol)
                mytb.close()

        else:
            casalog.post("UVW coordinates not fixed", 'NORMAL')

    except Exception, instance:
        casalog.post("*** Error \'%s\' " % (instance), 'SEVERE')
        return

