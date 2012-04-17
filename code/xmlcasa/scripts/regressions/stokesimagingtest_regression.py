#######################################################
#                                                                                                                         #
#           Regression/Test Script for Stokes-Selection in Imager and Clean                #
#                                                                                                                         #
#######################################################
#                                                                                                                         
#  (1) Given a 'true' [I,Q,U,V] vector, 
#       simulate MSs for [RR,RL,LR,LL], [RR,LL], [RR],[LL], [XX,XY,YX,YY], [XX,YY], [XX], [YY].
#  (2) Test all possible options, some valid, some not : ['I','Q','U','V', 'IV', 'QU', 'IQ', 'UV', 
#          'IQU', 'IUV', 'IQUV', 'RR', 'LL', 'RL','LR', 'RRLL','RLLR','XX','YY','XY','YX','XXYY','XYYX']
#  (3) For runs that produce an output image, the resulting output (peak pixel value) 
#       is compared with the 'true' [I,Q,U,V,RR,LL,XX,YY]. 
#  (4) Counts of 'pass' and 'fail' are, in the end, compared and used to detect regression.
#       Note : This script tests both valid and invalid options.
#
#                                                                                                                           
######################################################
#                                                                                                                            
# More tests that will appear here in the future 
# (See JIRA CAS 2587 : https://bugs.nrao.edu/browse/CAS-2587 ) :                            
#                                                                               
# (1) Tests with partially flagged data
# (2) Tests with different minor-cycle algorithms
#                                                                              
######################################################

import time
import os

# Init printing info.
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='stokesimagingtest.'+datestring+'.log'
logfile=open(outfile,'w')

#####################################################
####  BEGIN : Helper function definition (tests are run after 'END')                 ####
#####################################################

################################
###  Simulate a small MS
################################
def makeMS(choice='RL_all',stokesvals=[1.0,1.0,0.0,0.0],parentpath='.'):
  dirname = parentpath+"/Point"
  choicelist = ['RL_all','RL_par','RR_only','LL_only','XY_all','XY_par','XX_only','YY_only'];
  if(not (choice in choicelist)):
    print >>logfile, 'Cannot find specs for ', choice;
    return;
  if(choice=='RL_all'):
    basename = dirname+"/point_linRL"
    msstokes='RR RL LR LL';
    feedtype='perfect R L';
  if(choice=='XY_all'):
    basename = dirname+"/point_linXY"
    msstokes='XX XY YX YY'; 
    feedtype='perfect X Y';
  if(choice=='RL_par'):
    basename = dirname+"/point_parRL"
    msstokes='RR LL';
    feedtype='perfect R L';
  if(choice=='XY_par'):
    basename = dirname+"/point_parXY"
    msstokes='XX YY'; 
    feedtype='perfect X Y';
  if(choice=='XX_only'):
    basename = dirname+"/point_onlyXX"
    msstokes='XX'; 
    feedtype='perfect X Y';
  if(choice=='YY_only'):
    basename = dirname+"/point_onlyYY"
    msstokes='YY'; 
    feedtype='perfect X Y';
  if(choice=='RR_only'):
    basename = dirname+"/point_onlyRR"
    msstokes='RR'; 
    feedtype='perfect R L';
  if(choice=='LL_only'):
    basename = dirname+"/point_onlyLL"
    msstokes='LL'; 
    feedtype='perfect R L';

  msname = basename + '.ms';
  clname = basename+'.cl';

  if(not os.path.exists(dirname)):
    cmd = 'mkdir ' + dirname;
    os.system(cmd);

  vx = [41.1100006,  134.110001,   268.309998,  439.410004,  644.210022]
  vy = [3.51999998, -39.8300018,  -102.480003, -182.149994, -277.589996]
  vz = [0.25,       -0.439999998, -1.46000004, -3.77999997, -5.9000001]
  d = [25.0,       25.0,         25.0,         25.0,       25.0]
  an = ['VLA1','VLA2','VLA3','VLA4','VLA5'];
  nn = len(vx);
  x = (vx - (sum(pl.array(vx))/(nn)));
  y = (vy - (sum(pl.array(vy))/(nn)));
  z = (vz - (sum(pl.array(vz))/(nn)));

  sm.open(ms=msname);
  sm.setconfig(telescopename='VLA',x=x.tolist(),y=y.tolist(),z=z.tolist(),dishdiameter=d,
	       mount=['alt-az'], antname=an,
	       coordsystem='local',referencelocation=me.observatory('VLA'));
  sm.setspwindow(spwname="LBand",freq="1.0GHz",deltafreq='500MHz',
		 freqresolution='2MHz',nchannels=3,stokes=msstokes);
  sm.setfeed(mode=feedtype,pol=['']);
  ra0="19:59:28.500";
  dec0="+40.44.01.50";
  sm.setfield( sourcename="fake",sourcedirection=me.direction(rf='J2000',v0=ra0,v1=dec0) );
  sm.setlimits(shadowlimit=0.01, elevationlimit='10deg');
  sm.setauto(autocorrwt=0.0);
  sm.settimes(integrationtime='2000s', usehourangle=True,
                       referencetime=me.epoch('UTC','2008/11/24/00:00:00'));
  sm.observe(sourcename="fake",spwname='LBand', starttime='-4.0h', stoptime='+4.0h');
  sm.close();


  if(choice in ['XY_all','XY_par','XX_only','YY_only']):
     tb.open(basename+'.ms/FEED',nomodify=False);
     ff = tb.getcol('POLARIZATION_TYPE');
     ff[0].fill('X');
     ff[1].fill('Y');
     tb.putcol('POLARIZATION_TYPE',ff);
     tb.close();

  os.system('rm -rf '+clname);
  ###cl.open();
  refRA = qa.unit(ra0);
  refDEC = qa.unit(dec0);
  cl.addcomponent(flux=stokesvals,fluxunit="Jy", 
                            dir=me.direction(rf='J2000', 
                            v0=qa.add(refRA,"0.0arcmin"),v1=qa.add(refDEC,"0.0arcmin")),
		            shape="point",freq='1.5GHz',
		            spectrumtype="spectral index",index=0.0);
  cl.rename(filename=clname);
  cl.close();

  print >>logfile, 'Predicting  : ', clname , ' onto ' , msname;
  ft(vis=msname,complist=clname,usescratch=True);

  tb.open(msname,nomodify=False);
  moddata = tb.getcol(columnname='MODEL_DATA');
  tb.putcol(columnname='DATA',value=moddata);
  tb.putcol(columnname='CORRECTED_DATA',value=moddata);
  moddata.fill(0.0);
  tb.putcol(columnname='MODEL_DATA',value=moddata);
  tb.close();


#####################################


### Run clean
def runstokes(msname="Point/point_linXY.ms", imname='sctest',stokes='IQ'):
   cmd = 'rm -rf ' + imname + '*';
   os.system(cmd);
   print >>logfile, '### Running clean with stokes : ', stokes, ' on MS : ', msname;
   clean( vis                  =  msname,
             imagename     =  imname,
             niter              =  100,
             gain               =  0.5,
             psfmode            =  "clark",
             imagermode         =  '',
             multiscale       = [],
             imsize             =  [100],
             cell               =  ['10.0arcsec', '10.0arcsec'],
             stokes             =  stokes,
             weighting          =  "natural"
           );
   return checkoutput(msname,imname,stokes);
################################

### Check output images
def checkoutput(msname='Point/point_linXY.ms', imname='sctest', stokes='IQ'):
    ## check output stokes coordinates.
    resultvals={'ms':msname, 'userstokes':stokes};
    if( not os.path.exists( imname+'.image' ) ):
        print >>logfile, "### No output file found";
        resultvals['answer']='no output image';
        return resultvals;
    ia.open(imname+'.image');
    csys = ia.coordsys();
    outstokes = csys.stokes();
    if( type(outstokes)==str ):
       outstokes = [outstokes];
    midpix = (ia.shape())[0]/2;
    rvals={};
    for st in range(0,len(outstokes)):
       print >>logfile, '### Output ', outstokes[st], ":",  (ia.pixelvalue([midpix,midpix,st,0]))['value'];
       rvals[outstokes[st]]=(ia.pixelvalue([midpix,midpix,st,0]))['value'];
    ia.close();
    resultvals['answer']=rvals;
    return resultvals;
###################################

###################################

##########################
### Convert IQUV to all stokes dictionary
##########################
def convertToStokes(stokesvals=[]):
   slist = {};
   slist['I'] = stokesvals[0];
   slist['Q']= stokesvals[1];
   slist['U'] = stokesvals[2];
   slist['V']= stokesvals[3];
   slist['RR']= ( slist['I'] + slist['V'] );
   slist['LL']= ( slist['I'] - slist['V'] );
   slist['RL']= ( slist['Q'] + complex(0,slist['U']) );
   slist['LR']= ( slist['Q'] - complex(0,slist['U']) );
   slist['XX']= ( slist['I'] + slist['Q'] );
   slist['YY']= ( slist['I'] - slist['Q'] );
   slist['XY']= ( slist['U'] + complex(0,slist['V']) );
   slist['YX']= ( slist['U'] - complex(0,slist['V']) );
   return slist;   

##############################


### Run a bunch of tests -- try all combinations allowed at the clean task level.
####  I, IV, IQ, QU, UV, IQUV, RR, LL, RRLL, XX,YY, XXYY
def doAllChecks(stokesvals=[],parentpath='.'):
 soptions = ['I','Q','U','V', 'IV', 'QU', 'IQ', 'UV', 'IQU', 'IUV', 'IQUV', 'RR', 'LL', 'RL','LR', 'RRLL','RLLR','XX','YY','XY','YX','XXYY','XYYX'];
 choicelist = ['RL_all','RL_par','RR_only','LL_only','XY_all','XY_par','XX_only','YY_only'];

 msnames = {'RL_all':'Point/point_linRL.ms' , 'RL_par':'Point/point_parRL.ms' , 'RR_only':'Point/point_onlyRR.ms' ,'LL_only':'Point/point_onlyLL.ms' ,'XY_all':'Point/point_linXY.ms' ,'XY_par':'Point/point_parXY.ms' ,'XX_only':'Point/point_onlyXX.ms' ,'YY_only':'Point/point_onlyYY.ms' }

 ## Select what tests to run. 
 #soptions = ['I','IV','IQU','IQUV'];
 #choicelist=['XY_all'];

 resultlist=[];
 for mss in choicelist:
   for sto in soptions:
      print >>logfile, '### ---------------------------------------------------------------- ';
      if( not os.path.exists( parentpath+'/'+msnames[mss] ) ):
          print >>logfile, '### Did not find MS. Making ', parentpath+'/'+msnames[mss];
          makeMS(mss, stokesvals,parentpath);
      else:
          print >>logfile, '### Found MS ', parentpath+'/'+msnames[mss];
      resultlist.append( runstokes(msname=parentpath+'/'+msnames[mss],stokes=sto) );
 return resultlist;
###############################################

#####################################################
####  END : Helper function definition                                                            ####
#####################################################

## Now, run the tests.

# Data : Simulated in the test directory.
pathname=os.environ.get('CASAPATH').split()[0]

# Initialize status flag
regstate = True;

# Start timers
startTime=time.time()
startProc=time.clock()

# Run All Tests
parentpath='.';
stokesvals = [1.0,2.0,3.0,4.0];
resultlist = doAllChecks(stokesvals=stokesvals,parentpath=parentpath);

# Stop timers
endProc=time.clock()
endTime=time.time()


# Perform the checks
print >>logfile,''
print >>logfile,'*********************** Comparison of results **********************************'

truestokes=convertToStokes(stokesvals);

countpass=0;
countfail=0;
countwrongnumbers=0;
totalcount=0;
for stok in resultlist:
     slist = stok['answer'];
     if(slist == 'no output image'):
         print >>logfile, 'FAIL : ',stok;
         countfail=countfail+1;
     else:
        stat=True;
        for sout in slist:
           if( abs( slist[sout]['value'] - truestokes[sout] ) > 1e-03 ):
               stat=False;
        if(stat==True):
               print >>logfile, 'PASS : ', stok;
               countpass=countpass+1;
        else:
               print >>logfile, 'FAIL : WRONG NUMBERS : ', stok;
               countfail=countfail+1;
               countwrongnumbers=countwrongnumbers+1;
     totalcount=totalcount+1;
################################

# Truth for initial set of tests
# Feb 18 2011 : This tests clark clean (allows only npol=1,2,4), and stokes=[1.0,2.0,3.0,4.0]
true_totalcount = 184;
true_countpass = 40;
true_countfail = 144;
true_countwrongnumbers = 14;

print >>logfile,  '*********************************************************************************';
print >>logfile,  'OUTPUT -- Total count : ', totalcount , ' --- Passed : ', countpass , ' --- Failed : ', countfail , ' (', countwrongnumbers, ' wrong numbers) ';
print >>logfile, 'TRUTH    -- Total count : ', true_totalcount , ' --- Passed : ', true_countpass , ' --- Failed : ', true_countfail , ' (', true_countwrongnumbers, ' wrong numbers) ';
print >>logfile,  '*********************************************************************************';

if( not (totalcount==true_totalcount) or not (countpass==true_countpass) or not (countfail==true_countfail) or not (countwrongnumbers==true_countwrongnumbers) ):
     regstate=False;
################################

# Final verdict
if(regstate):
   print >>logfile,'PASSED regression test for stokes selection in imager'
   print ''
   print 'Regression PASSED'
   print ''
else:
   print >>logfile,'FAILED regression test for stokes selection in imager'
   print ''
   print 'Regression FAILED'
   print ''

print >>logfile,''

# Print timing info
print >>logfile,'********************************************************************************'
print >>logfile,'**                         Benchmarking                                       **'
print >>logfile,'********************************************************************************'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(278./(endTime - startTime))
print >>logfile,'*                                                                              *'
print >>logfile,'********************************************************************************'

logfile.close()


