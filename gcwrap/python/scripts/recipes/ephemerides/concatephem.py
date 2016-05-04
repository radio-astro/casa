from taskinit import *

# Concatenation of ephemeris tables
#
# Example:
#      import recipes.ephemerides.concatephem as cce
#      cce.concatephem(ephems=['Neptune1.tab', 'Neptune2.tab', 'Neptune3.tab'],
#                              outputephem='NeptuneAll.tab')


def concatephem(ephems=[], outputephem=''):
    """
    concatephem

    Concatenate the given ephemeris tables into a single output table
    filling gaps with dummy rows and removing overlap rows.
    Before concatenation, test that basic conditions are fulfilled:
    same time grid, list of input ephemerides is ordered in time.

    ephems - List of the ephemeris tables to be concatenated
            default: 
    outputephem - Name of the output ephemeris to be created from the concatenation
            If empty, concatephem will only perform a dryrun.
            default: ''
    
    """
    mytb = tbtool()
    starts = []
    ends = []
    stepsizes = []
    hasoverlap_with_previous = []
    gap_to_previous = [] # negative entry means overlap   
    shouldconcat_with_previous = []
    canconcat_with_previous = []

    for myephem in ephems:
        mytb.open(myephem)
        mynrows = mytb.nrows()
        if mynrows==0:
            mytb.close()
            casalog.post('Ephemeris '+myephem+' has no rows.', 'SEVERE')
            return
        mystart = mytb.getcell('MJD',0)
        myend = mytb.getcell('MJD', mynrows-1)
        if mynrows<2:
            mystepsize = 0
            casalog.post('Ephemeris '+myephem+' has only one row.', 'WARN')
        else:
            mystepsize =  mytb.getcell('MJD',1) - mystart

        mytb.close()

        starts.append(mystart)
        ends.append(myend)
        stepsizes.append(mystepsize)

    casalog.post('Ephem no., startMJD, endMJD, step size (d)', 'INFO')

    for i in range(0,len(starts)):
        casalog.post(str(i)+', '+str(starts[i])+', '+str(ends[i])+', '+str(stepsizes[i]), 'INFO')
        shouldconcat_with_previous.append(False)
        canconcat_with_previous.append(False)
        hasoverlap_with_previous.append(False)
        gap_to_previous.append(0)
        if i>0: 
            if stepsizes[i]==stepsizes[i-1]:
                print 'Ephemerides '+str(i-1)+' and '+str(i)+' have same step size.'
                if starts[i-1] < starts[i]:
                    print 'Ephemeris '+str(i-1)+' begins before '+str(i)
                    if ends[i-1] < ends[i]: 
                        print 'Ephemeris '+str(i-1)+' ends before '+str(i)
                        shouldconcat_with_previous[i] = True
                        numsteps_to_add = (starts[i]-ends[i-1])/stepsizes[i] - 1
                        gap_to_previous[i] = int(round(numsteps_to_add))
                        if abs(round(numsteps_to_add) - numsteps_to_add)<1E-6:
                            print 'Gap between ephemerides '+str(i-1)+' and '+str(i)+' is '+str(gap_to_previous[i])+' steps'
                            canconcat_with_previous[i] = True
                            if numsteps_to_add<0.:
                                print  'Ephemerides '+str(i-1)+' and '+str(i)+' have overlap of '+str(-gap_to_previous[i])+' steps'
                                hasoverlap_with_previous[i]=True
                        else:
                            print 'Gap between ephemerides '+str(i-1)+' and '+str(i)+' is not an integer number of steps:'
                            print str(round(numsteps_to_add) - numsteps_to_add)
                            canconcat_with_previous[i] = False
                    else:
                        print 'Ephemeris '+str(i-1)+' does not end before '+str(i)
                        shouldconcat_with_previous[i] = False
                        canconcat_with_previous[i] = False
                else:
                    print 'Ephemeris '+str(i-1)+' begins before '+str(i)
                    casalog.post('Ephemerides are in wrong order.', 'SEVERE')
                    return
            else:
                print 'Ephemerides '+str(i-1)+' and '+str(i)+' do not have same step size.'
                casalog.post('Ephemerides have inhomogeneous steps sizes.', 'SEVERE')
                return
        #end if

    if outputephem=='' or len(starts)<2:
        return
    else:
        print 'Creating output ephemeris ...'
        if os.path.exists(outputephem):
            casalog.post('Output ephemeris table '+outputephem+' exists already.', 'SEVERE')
            return

        os.system('cp -R '+ephems[0]+' '+outputephem)
        
        mytb2 = tbtool()
        
        try:
            mytb.open(outputephem, nomodify=False)
        except:
            casalog.post('Error opening table '+outputepehem+' for writing.', 'SEVERE')
            return


        for i in range(1,len(starts)):

            mynrows = mytb.nrows()

            mytb2.open(ephems[i])
            mynrows2 = mytb2.nrows()
            startrow = 0
            if(hasoverlap_with_previous[i]):
                startrow = -gap_to_previous[i]
            elif(gap_to_previous[i]>0): # fill gap with dummy rows
                mytb.addrows(gap_to_previous[i])
                startmjd = mytb.getcell('MJD', mynrows-1)
                for j in range(mynrows, mynrows+gap_to_previous[i]):
                    newmjd = startmjd+stepsizes[i]*(j-mynrows+1)
                    mytb.putcell('MJD', j, newmjd)
                    mytb.putcell('RA', j, 0.)
                    mytb.putcell('DEC', j, 0.)
                    mytb.putcell('Rho', j, 0.)
                    mytb.putcell('RadVel', j, 0.)
                    mytb.putcell('diskLong', j, 0.)
                    mytb.putcell('diskLat', j, 0.)

                print str(gap_to_previous[i])+' dummy rows appended to fill gap'

            mynrows = mytb.nrows()

            mytb.addrows(mynrows2-startrow)
            for j in range(mynrows, mynrows+mynrows2-startrow):
                fromrow = j - mynrows + startrow
                mytb.putcell('MJD', j, mytb2.getcell('MJD', fromrow))
                mytb.putcell('RA', j, mytb2.getcell('RA', fromrow))
                mytb.putcell('DEC', j, mytb2.getcell('DEC', fromrow))
                mytb.putcell('Rho', j, mytb2.getcell('Rho', fromrow))
                mytb.putcell('RadVel', j, mytb2.getcell('RadVel', fromrow))
                mytb.putcell('diskLong', j, mytb2.getcell('diskLong', fromrow))
                mytb.putcell('diskLat', j, mytb2.getcell('diskLat', fromrow))

            print str(mynrows2-startrow)+' rows copied over'

    return


                
        

    
