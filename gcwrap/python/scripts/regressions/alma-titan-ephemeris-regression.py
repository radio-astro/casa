#############################################################################
# $Id:$
# Test Name:                                                                #
# alma-titan-ephemeris-regression.py                                        #
#                                                                           # 
# Rationale for Inclusion:                                                  #
#    Need test of analysis of MS with ephemeris attached                    #
#                                                                           # 
# Input data:                                                               #
#     two MSs                                                               #
#                                                                           #
# Author: DP, Sept. 2013                                                    #
#                                                                           #
#############################################################################


step_title = { 0 : 'fixplanets with ephemeris',
	       1 : 'concat',
               2 : 'selfcal',
	       3 : 'cvel to outframe SOURCE',
               4 : 'imaging',
               5 : 'verification of the regression results'
               }

# global defs
basename=['X19d','X346']
makeplots=False

#############################

# Some infrastructure to make repeating individual parts
#   of this workflow more convenient.

thesteps = []

try:
    print 'List of steps to be executed ...', mysteps
    thesteps = mysteps
except:
    print 'global variable mysteps not set.'
if (thesteps==[]):
    thesteps = range(0,len(step_title))
    print 'mysteps empty. Executing all steps: ', thesteps

# The Python variable 'mysteps' will control which steps
# are executed when you start the script using
#   execfile('alma-titan-ephemeris-regression.py')
# e.g. setting
#   mysteps = [2,3,4]
# before starting the script will make the script execute
# only steps 2, 3, and 4
# Setting mysteps = [] will make it execute all steps.

totaltime = 0
inittime = time.time()
ttime = inittime
steptime = []

def timing():
    global totaltime
    global inittime
    global ttime
    global steptime
    global step_title
    global mystep
    global thesteps
    thetime = time.time()
    dtime = thetime - ttime
    steptime.append(dtime)
    totaltime += dtime
    ttime = thetime
    casalog.origin('TIMING')
    casalog.post( 'Step '+str(mystep)+': '+step_title[mystep], 'WARN')
    casalog.post( 'Time now: '+str(ttime), 'WARN')
    casalog.post( 'Time used this step: '+str(dtime), 'WARN')
    casalog.post( 'Total time used so far: ' + str(totaltime), 'WARN')
    casalog.post( 'Step  Time used (s)     Fraction of total time (percent) [description]', 'WARN')
    for i in range(0, len(steptime)):
        casalog.post( '  '+str(thesteps[i])+'   '+str(steptime[i])+'  '+str(steptime[i]/totaltime*100.)
                      +' ['+step_title[thesteps[i]]+']', 'WARN')
        

# default ASDM dataset name
myasdm_dataset_name = "X19d.ms.split.cal.titan.spw0"
myasdm_dataset2_name = "X346.ms.split.cal.titan.spw0"
myephemeris = 'Titan_55197-59214dUTC_J2000.tab'

# get the dataset name from the wrapper if possible
mydict = locals()
if mydict.has_key("asdm_dataset_name"):
    if(myasdm_dataset_name != mydict["asdm_dataset_name"]):
        raise("Wrong input file 1")
if mydict.has_key("asdm_dataset2_name"):
    if(myasdm_dataset2_name != mydict["asdm_dataset2_name"]):
        raise("Wrong input file 2")        
if mydict.has_key("tsys_table"):
    if(myephemeris != mydict["ephemeris"]):
        raise("Wrong input file 3")                

# ephemeris attachment
mystep = 0
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        fixplanets(vis=name+'.ms.split.cal.titan.spw0',
                   field = 'Titan',
                   direction='Titan_55197-59214dUTC_J2000.tab',
                   fixuvw=True)

        if(makeplots):
            plotms(vis=name+'.ms.split.cal.titan.spw0', xaxis='time', yaxis='elevation', 
		       averagedata=T, avgchannel='4000')
            
    timing()


# concat
mystep = 1
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    myvis = []
    for name in basename:
        myvis.append(name+'.ms.split.cal.titan.spw0')

    os.system('rm -rf titan.ms')
    concat(vis = myvis, concatvis = 'titan.ms')

    timing()

# selfcal
mystep = 2
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    setjy(vis="titan.ms",field="0",spw="0",reffreq="1GHz",standard="Butler-JPL-Horizons 2010")

    gaincal(vis="titan.ms",caltable="titan.model.selfcal_p",field="0",spw="0:0~1400;1600~3839",solint="inf",
            combine="",preavg=-1.0,refant="DV04",minblperant=4,minsnr=3.0,solnorm=False,
            gaintype="G",smodel=[],calmode="p")

    applycal(vis="titan.ms",field="0",gaintable="titan.model.selfcal_p")


# cvel
mystep = 3
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    name = "titan.ms"
    os.system('rm -rf cvel_'+name)
    cvel(vis=name,
         outputvis='cvel_'+name,
         outframe='SOURCE',
         mode = 'velocity', width = '0.3km/s',
         restfreq = '354.50547GHz') # HCN(4-3)v=0

    timing()



# imaging
mystep = 4
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf titanline.*')
    clean(vis="cvel_titan.ms",
          imagename="titanline",
          field="0",spw="0",
          mode="channel",width=1,
          niter=1000,gain=0.1,threshold="20.0mJy",
          psfmode="clark",imagermode="",
          mask="circle[[108pix,108pix],8pix]",
          restfreq='354.50547GHz',
          imsize=[216,216],cell="0.11arcsec",
          stokes="I",weighting="briggs",robust=0.5,
          interactive=False)
    

    timing()


# verification
mystep = 5
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    passed = True

    # verify MS frame
    mytb = tbtool()
    mytb.open('cvel_titan.ms/SPECTRAL_WINDOW')
    theframe = mytb.getcell('MEAS_FREQ_REF', 0)
    mytb.close()
    if (theframe!=0):
        print "ERROR: reference frame of SPW 0 in cvel_titan.ms should be REST==0 but is ", theframe
        passed = False

    # verify image properties 

    # expectation values CASA 4.3
    #exppeakm = 3.36615992 
    #exprmsm = 1.13404071 
    #exppeakmb = 0.36783999
    #exprmsmb = 0.01991641
    #exppeakchan = 258

    # expectation values CASA 4.4
    exppeakm = 3.3441593647
    exprmsm = 1.13052916527
    exppeakmb = 0.363690376282
    exprmsmb = 0.0196652077138
    exppeakchan = 259

    os.system('rm -rf collapsed')

    calstat=imstat(imagename='titanline.image', region="circle[[108pix,108pix],8pix]")
    resrmsm=(calstat['rms'][0])
    respeakm=(calstat['max'][0])

    imcollapse(imagename='titanline.image', function='sum', axes=[0,1], region="circle[[108pix,108pix],8pix]", outfile='collapsed')
    calstatx=imstat(imagename='collapsed')
    respeakchan=(calstatx['maxpos'][3])
    
    calstatb=imstat(imagename='titanline.image', region="circle[[93pix,108pix],8pix]")
    resrmsmb=(calstatb['rms'][0])
    respeakmb=(calstatb['max'][0])

    casalog.post( ' rms in Titan: '+str(resrmsm))
    casalog.post( ' Peak in Titan: '+str(respeakm))
    casalog.post( ' Dynamic range in Titan: '+str(respeakm/resrmsm))
    casalog.post( ' rms off-axis: '+str(resrmsmb))
    casalog.post( ' peak off-axis: '+str(respeakmb))


    timing()

    # print results to logger
    casalog.origin('SUMMARY')

    casalog.post( "\n***** Peak and RMS of the image *****")

    casalog.post( "Titan: Peak (expectation), RMS (expectation)")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post( str(respeakm)+ "("+str(exppeakm)+"),"+ str(resrmsm)+ "("+str(exprmsm)+")")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post( "Off-axis: Peak (expectation), RMS (expectation)")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post( str(respeakmb)+ "("+str(exppeakmb)+"),"+ str(resrmsmb)+ "("+str(exprmsmb)+")")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post( "Peak channel (expectation)")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post( str(respeakchan)+ "("+str(exppeakchan)+")")


    if (respeakchan!=exppeakchan):
        casalog.post( 'ERROR: Peak flux in titan cube should be at channel '+str(exppeakchan)+' but is at '+str(respeakchan),'WARN')
        passed = False

    peakmdev = abs(respeakm-exppeakm)/exppeakm*100.
    if (peakmdev > 0.5):
        casalog.post( 'ERROR: Peak in Titan image deviates from expectation by '+str(peakmdev)+' percent.','WARN')
        passed = False

    rmsmdev = abs(resrmsm-exprmsm)/exprmsm*100.
    if (rmsmdev > 0.5):
        casalog.post( 'ERROR: RMS in Titan image deviates from expectation by '+str(rmsmdev)+' percent.','WARN')
        passed = False

    peakmdevb = abs(respeakmb-exppeakmb)/exppeakmb*100.
    if (peakmdevb > 0.5):
        casalog.post( 'ERROR: Peak offaxis deviates from expectation by '+str(peakmdevb)+' percent.','WARN')
        passed = False

    rmsmdevb = abs(resrmsmb-exprmsmb)/exprmsmb*100.
    if (rmsmdevb > 0.5):
        casalog.post( 'ERROR: RMS in Titan image deviates from expectation by '+str(rmsmdevb)+' percent.','WARN')
        passed = False

    if not passed:
        raise Exception, 'Results are different from expectations by more than 0.5 percent.'

    casalog.post( "\nAll peak and RMS values within 0.5 percent of the expectation.")
    

print 'Done.'
