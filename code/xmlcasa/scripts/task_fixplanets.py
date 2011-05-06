from taskinit import *

# Fix FIELD, SOURCE, and UVW for given fields based on pointing table information
# example: fixplanets('uid___A002_X1c6e54_X223.ms', 'Titan', True)

def fixplanets(vis, field, fixuvw=False):
    casalog.origin('fixplanets')
    try:
        fields = ms.msseltoindex(vis=vis,field=field)['field']
        numfields = 0 
        if(len(fields) == 0):
            casalog.post( "Field selection returned zero results.", 'WARN')
            return
        tb.open(vis+"/FIELD")
        oldrefcol = []
        if('PhaseDir_Ref' in tb.colnames()):
            oldrefcol = tb.getcol('PhaseDir_Ref')
        tb.close()
        for fld in fields:
            os.system('rm -rf fixplanetstemp')
            tb.open(vis)
            tb.query('FIELD_ID=='+str(fld)+' AND FLAG_ROW==False', name='fixplanetstemp', columns='TIME')
            tb.close()
            tb.open('fixplanetstemp')
            mytime = tb.getcell('TIME',tb.nrows()/2)
            casalog.post( "TIME "+str(mytime), 'NORMAL')
            tb.close()

            tb.open(vis+'/POINTING')
            ttb = tb.query('TRACKING==True AND NEARABS(TIME,'+str(mytime)+',INTERVAL/2.)',
                           name='fixplanetstemp2')
            if(ttb.nrows()==0):
                del ttb
                os.system('rm -rf fixplanetstemp2')
                ttb = tb.query('TRACKING==True AND NEARABS(TIME,'+str(mytime)+',3.)', # search within 3 seconds
                           name='fixplanetstemp2')
                if(ttb.nrows()==0):
                    del ttb
                    casalog.post( "Cannot find any POINTING table rows with TRACKING==True within 3 seconds of TIME "+str(mytime), 'WARN')
                    casalog.post( "Will try without requiring TRACKING==True ...", 'WARN')
                    os.system('rm -rf fixplanetstemp2')
                    ttb = tb.query('NEARABS(TIME,'+str(mytime)+',INTERVAL/2.)', 
                                   name='fixplanetstemp2')
                    if(ttb.nrows()==0):
                        del ttb
                        os.system('rm -rf fixplanetstemp2')
                        ttb = tb.query('NEARABS(TIME,'+str(mytime)+',3.)', # search within 3 seconds
                                       name='fixplanetstemp2')
                        if(ttb.nrows()==0):
                            del ttb
                            tb.close()
                            casalog.post( "Cannot find any POINTING table rows within 3 seconds of TIME "+str(mytime), 'SEVERE')
                            return # give up
            del ttb
            tb.close()
            tb.open('fixplanetstemp2')
            mydir = tb.getcell('DIRECTION',0)
            tb.close()
            casalog.post( ' field id '+str(fld)+ ' AZ EL '+str(mydir[0])+" "+str(mydir[1]), 'NORMAL')
            mydirme = me.direction(rf='AZELGEO',v0=qa.quantity(mydir[0][0], 'rad'), v1=qa.quantity(mydir[1][0],'rad'))
            #print mydirme
            me.doframe(me.epoch(rf='UTC', v0=qa.quantity(mytime,'s')))
            tb.open(vis+'/OBSERVATION')
            myobs = tb.getcell('TELESCOPE_NAME',0)
            tb.close()
            casalog.post( "Observatory is " + myobs, 'NORMAL')
            me.doframe(me.observatory(myobs))
            mydirmemod = me.measure(v=mydirme, rf='J2000')
            #print mydirmemod
            mynewra_rad = mydirmemod['m0']['value']
            mynewdec_rad = mydirmemod['m1']['value']
            me.done()
            
            # modify FIELD table
            tb.open(vis+'/FIELD', nomodify=False)
            numfields = tb.nrows()
            myolddir = tb.getcell('PHASE_DIR',fld)
            planetname = tb.getcell('NAME',fld)
            casalog.post( 'object: '+planetname, 'NORMAL')
            casalog.post( 'old RA, DEC (rad) '+str(myolddir[0])+" " +str(myolddir[1]), 'NORMAL')
            casalog.post( 'new RA, DEC (rad) '+str(mynewra_rad)+" "+ str(mynewdec_rad), 'NORMAL')
            mynewdir = myolddir
            mynewdir[0][0] = mynewra_rad
            mynewdir[1][0] = mynewdec_rad
            tb.putcell('PHASE_DIR',fld, mynewdir)
            if('PhaseDir_Ref' in tb.colnames()):
                tb.putcell('PhaseDir_Ref',fld, 0) # J2000
            tb.close()
            #modify SOURCE table
            tb.open(vis+'/SOURCE', nomodify=False)
            sdir = tb.getcol('DIRECTION')
            newsdir = sdir
            sname = tb.getcol('NAME')
            for i in xrange(0,tb.nrows()):
                if(sname[i]==planetname):
                    #print 'i old dir ', i, " ", sdir[0][i], sdir[1][i]
                    newsdir[0][i] = mynewra_rad
                    newsdir[1][i] = mynewdec_rad
                    #print '  new dir ', newsdir[0][i], newsdir[1][i]
            tb.putcol('DIRECTION', newsdir)
            tb.close()
                
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
        else:
            casalog.post("UVW coordinates not fixed", 'NORMAL')

        if not oldrefcol==[]: 
            tb.open(vis+'/FIELD', nomodify=False)
            tb.putcol('PhaseDir_Ref', oldrefcol)
            tb.close()

    except Exception, instance:
        casalog.post("*** Error \'%s\' " % (instance), 'SEVERE')
        return

