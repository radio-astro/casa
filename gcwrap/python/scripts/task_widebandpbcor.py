################################################
# Task to do wideband PB-corrections on images produced by MS-MFS.
#
# v1.0: 2012.09.10, U.R.V.
#
################################################
import os
import numpy as np
import shutil
from scipy import linalg
from taskinit import *

im,cb,ms,tb,me,ia,po,sm,cl,cs,rg,sl,dc,vp,msmd,fi,fn,imd,sdms=gentools(['im','cb','ms','tb','me','ia','po','sm','cl','cs','rg','sl','dc','vp','msmd','fi','fn','imd','sdms'])
def widebandpbcor(vis='',
                  imagename='mfim',
                  nterms=2,
                  threshold='1mJy',
                  action='pbcor',
                  reffreq = '1.5GHz',
                  pbmin=0.001, 
                  field='',
                  spwlist=[0],
                  chanlist=[0],
                  weightlist=[1]):
   """
   Wide-Band PB-correction.  Specify a list of spwids and channel numbers at which
   to compute primary beams. Supply weights to use for each beam.  All three lists
   spwlist, chanlist, weightlist must be of the same length.  It is enough to
   make one PB per spw (middle channel). 
   """   
   casalog.origin('widebandpbcor')

   # Check nterms
   if nterms < 1:
       casalog.post('Please specify a valid number of Taylor-terms (>0)', 'SEVERE')
       return False

   # Check that all required input images exist on disk
   taylorlist=[]
   residuallist=[]
   for i in range(0,nterms):
	taylorlist.append(imagename+'.image.tt'+str(i));
	residuallist.append(imagename+'.residual.tt'+str(i));
        if(not os.path.exists(taylorlist[i])):
            casalog.post('Taylor-coeff Restored Image : ' + taylorlist[i] + ' not found.','SEVERE')
            return False
        if(not os.path.exists(residuallist[i])):
            casalog.post('Taylor-coeff Residual Image : ' + residuallist[i] + ' not found.','SEVERE')
            return False

   casalog.post('Using images ' + str(taylorlist) + ' and ' + str(residuallist), 'NORMAL')

   ## Read imsize, cellsize, reffreq, phasecenter from the input images.
   ia.open( taylorlist[0] )
   imsize = [ ia.shape()[0], ia.shape()[1] ]
   iminfo = ia.summary()
   ia.close()
   
   ## Get cell size
   cellx = str( abs (qa.convert( qa.quantity( iminfo['incr'][0] , iminfo['axisunits'][0] ) , 'arcsec' )['value'] ) ) + ' arcsec'
   celly = str( abs (qa.convert( qa.quantity( iminfo['incr'][1] , iminfo['axisunits'][1] ) , 'arcsec' )['value'] ) ) + ' arcsec'

   ## Get phasecenter
   pcra = qa.quantity( iminfo['refval'][0] , iminfo['axisunits'][0] )
   pcdec = qa.quantity( iminfo['refval'][1] , iminfo['axisunits'][1] )
   pcdir = me.direction('J2000', pcra, pcdec )

   phasecenter = 'J2000 ' + qa.formxxx(pcdir['m0'],'hms') + ' ' + qa.formxxx(pcdir['m1'],'dms')

   ## Get reffreq, if not specified.
   if len(reffreq)==0:
       rfreq = qa.convert( qa.quantity( iminfo['refval'][3] , iminfo['axisunits'][3] ) , 'GHz' )
       reffreq = str( rfreq['value'] ) + rfreq['unit']

   casalog.post('Using imsize : ' + str(imsize) + ' and cellsize : ' + str(cellx) + ', ' + str(celly) + ' with phasecenter : ' + phasecenter + ' and reffreq : ' + reffreq, 'NORMAL')

   if type(threshold) != str:
       casalog.post('Please specify imthreshold as a string with units','SEVERE')
       return False
   imthreshold = qa.convert( qa.quantity(threshold) , 'Jy' )['value']


   ret = False
   if action=='pbcor':

       # Extract thresholds
       pbthreshold = pbmin

       casalog.post('Using a pblimit of ' + str(pbthreshold))

       if len(chanlist) < nterms or len(spwlist) < nterms or len(weightlist) < nterms:
           casalog.post('Please specify channel/spw/weight lists of lengths >= nterms', 'SEVERE')
           return False

       # Make a list of PBs from all the specifield spw/chan pairs.
       if len(chanlist) != len(spwlist) :
           casalog.post("Spwlist and Chanlist must be the same length", 'SEVERE')
           return False
       if len(spwlist) != len(weightlist) :
           casalog.post("Spwlist and Weightlist must be the same length", 'SEVERE')
           return False

       casalog.post('Calculating PBs for spws : ' + str(spwlist) + '  weightlist : ' + str(weightlist) + '  chanlist : ' + str(chanlist) , 'NORMAL');

       pbdirname = imagename + '.pbcor.workdirectory'
       if not os.path.exists(pbdirname):
           os.system('mkdir '+pbdirname)

       pblist = _makePBList(msname=vis, pbprefix=pbdirname + '/' + imagename+'.pb.', field=field, spwlist=spwlist, chanlist=chanlist, imsize=imsize, cellx=cellx, celly=celly, phasecenter=phasecenter)
       if len(pblist)==0:
           return False

       pbcubename = pbdirname + '/' + imagename+'.pb.cube'

       casalog.post('Concatenating PBs at all specified frequencies into a cube','NORMAL')
       newia = ia.imageconcat(outfile=pbcubename,infiles=pblist,relax=True,overwrite=True)
       newia.close()

       # Delete individual pb images
       for pbim in pblist:
           shutil.rmtree(pbim)

       # Make lists of image names
       pblist=[]
       imlist=[]
       imlistpbcor=[]
       for i in range(0,nterms):
           imlist.append(imagename+'.image.tt'+str(i))
           pblist.append(pbdirname+'/'+imagename+'.pb.tt'+str(i));
           imlistpbcor.append(imagename+'.pbcor.image.tt'+str(i));
           
       # Calculate Taylor Coeffs from this cube
       ret = _calcTaylorFromCube(imtemplate=imagename+'.image.tt0',reffreq=reffreq,cubename=pbcubename,newtay=pblist, pbthreshold=pbthreshold, weightlist=weightlist);
       if ret==False:
           return False

       # Calculate PB alpha and beta ( just for information )
       pbalphaname = pbdirname+'/'+imagename+'.pb.alpha'
       _calcPBAlpha(pbtay=pblist, pbthreshold=pbthreshold,pbalphaname=pbalphaname)

       # Divide out the PB polynomial
       ret = _dividePBTaylor(imlist,pblist,imlistpbcor,pbthreshold);
       if ret==False:
           return False

   # Recalculate Alpha.
   if(ret==True or action=='calcalpha'):
       if nterms==1:
           casalog.post('Cannot compute spectral index with only one image', 'WARN')
           return True
       if ret==True:
           imname = imagename+'.pbcor'
       else:
           imname = imagename
       residuallist=[];
       imagelist=[];
       for ii in range(0,nterms):
           residuallist.append(imagename+'.residual.tt'+str(ii));
           imagelist.append(imname+'.image.tt'+str(ii));
       _compute_alpha_beta(imname, nterms, imagelist, residuallist, imthreshold, [], True);



###############################################

def _calcPBAlpha(pbtay=[], pbthreshold=0.1,pbalphaname='pbalpha.im'):
    nterms = len(pbtay)
    if nterms<2:
        return False
    
    if(os.path.exists(pbalphaname)):
       rmcmd = 'rm -rf ' + pbalphaname
       os.system(rmcmd)
    if(not os.path.exists(pbalphaname)):
       cpcmd = 'cp -r ' + pbtay[0] + ' ' + pbalphaname
       os.system(cpcmd)

    ia.open(pbalphaname)
    alpha = ia.getchunk()
    ia.close()

    ptay=[]
    ia.open(pbtay[0])
    ptay.append(ia.getchunk())
    ia.close()
    ia.open(pbtay[1])
    ptay.append(ia.getchunk())
    ia.close()

    ptay[0][ ptay[0] < pbthreshold  ] = 1.0
    ptay[1][ ptay[0] < pbthreshold  ] = 0.0

    alpha = ptay[1]/ptay[0]

    ia.open(pbalphaname)
    ia.putchunk(alpha)
    ia.calcmask(mask='"'+pbtay[0]+'"'+'>'+str(pbthreshold));
    ia.close()


#################################################
def _makePBList(msname='',pbprefix='',field='',spwlist=[],chanlist=[], imsize=[], cellx='10.0arcsec', celly='10.0arcsec',phasecenter=''):
   #print 'Making PB List from the following spw,chan pairs';
   pblist = []
   try:
     for aspw in range(0,len(spwlist)):
         #print 'spw=', spwlist[aspw], ' chan=',chanlist[aspw]
         im.open(msname);
         sspw = str(spwlist[aspw])+':'+str(chanlist[aspw])
         ret = im.selectvis(field=field, spw=sspw);
         if(ret==False):
             casalog.post('Error in constructing PBs at the specified spw:chan of '+ sspw ,'SEVERE')
             return []
         im.defineimage(nx=imsize[0],ny=imsize[1], cellx=cellx,celly=celly, 
                             nchan=1,start=chanlist[aspw], stokes='I',
                             mode='channel',spw=[spwlist[aspw]],phasecenter=phasecenter);
         im.setvp(dovp=True,usedefaultvp=True,telescope='EVLA');
         pbname = pbprefix + str(spwlist[aspw])+'.'+str(chanlist[aspw])
         im.makeimage(type='pb', image=pbname, compleximage="", verbose=False);
         pblist.append(pbname)
         im.close();
   except:
       casalog.post('Error in constructing PBs at the specified frequencies. Please check spwlist/chanlist','SEVERE')
       return []
   return pblist
    

##############################################################################
def _calcTaylorFromCube(imtemplate="",reffreq='1.42GHz',cubename="sim.pb",newtay=[],pbthreshold=0.0001,weightlist=[]):
   for tay in range(0,len(newtay)):
     if(os.path.exists(newtay[tay])):
       rmcmd = 'rm -rf '+newtay[tay]
       os.system(rmcmd)
     if(not os.path.exists(newtay[tay])):
       cpcmd = 'cp -r ' + imtemplate + ' ' + newtay[tay]
       os.system(cpcmd)

   ia.open(cubename);
   pbcube = ia.getchunk();
   csys = ia.coordsys();
   shp = ia.shape();
   ia.close();

   if(csys.axiscoordinatetypes()[3] == 'Spectral'):
       restfreq = csys.restfrequency()['value'][0]/1e+09; # convert more generally..
       freqincrement = csys.increment()['numeric'][3]/1e+09;
       freqlist = [];
       for chan in range(0,shp[3]): 
             freqlist.append(restfreq + chan * freqincrement);
   elif(csys.axiscoordinatetypes()[3] == 'Tabular'):
       freqlist = (csys.torecord()['tabular2']['worldvalues'])/1e+09;
   else:
       casalog.post('Unknown frequency axis. Exiting.','SEVERE');
       return False;

   reffreqGHz = qa.convert(qa.quantity(reffreq), 'GHz')['value']
   freqs = (np.array(freqlist,'f')-reffreqGHz)/reffreqGHz;

   casalog.post("Using PBs at " + str(freqlist) + " GHz, to compute Taylor coefficients about " + str(reffreqGHz) + " GHz", 'NORMAL')
   #print freqs;

   nfreqs = len(freqlist)
   if len(weightlist)==0:
     weightarr = np.ones( freqs.shape )
   else:
     if len(weightlist) != nfreqs:
       casalog.post('Weight list must be the same length as nFreq : ' + nfreqs , 'SEVERE')
       return False
     else:
       weightarr = np.array(weightlist)

   nterms = len(newtay);
   if(nterms>5):
     casalog.post('Cannot handle more than 5 terms for PB computation','SEVERE')
     return False;
   ptays=[];
   if(nterms>0):
     ia.open(newtay[0]);
     ptays.append( ia.getchunk() );
     ptays[0].fill(0.0);
     ia.close();
     pstart=[0.0];
   if(nterms>1):
     ia.open(newtay[1]);
     ptays.append( ia.getchunk() );
     ptays[1].fill(0.0);
     ia.close();
     pstart=[0.0,0.0];
   if(nterms>2):
     ia.open(newtay[2]);
     ptays.append( ia.getchunk() );
     ptays[2].fill(0.0);
     ia.close();
     pstart=[0.0,0.0,0.0];
   if(nterms>3):
     ia.open(newtay[3]);
     ptays.append( ia.getchunk() );
     ptays[3].fill(0.0);
     ia.close();
     pstart=[0.0,0.0,0.0,0.0];
   if(nterms>4):
     ia.open(newtay[4]);
     ptays.append( ia.getchunk() );
     ptays[4].fill(0.0);
     ia.close();
     pstart=[0.0,0.0,0.0,0.0,0.0];
   
   ##### Fit a nterms-term polynomial to each point !!!
  
   # Linear fit
   ptays=_linfit(ptays, freqs, pbcube[:,:,0,:], weightarr, pbthreshold);

   # Set all values below the pbthreshold, to zero
   for tt in range(0,nterms):
       ptays[tt][ ptays[0]<pbthreshold  ] = 0.0

   # Write to disk.
   if(nterms>0):
     ia.open(newtay[0]);
     ia.putchunk(ptays[0]);
     ia.close();
   if(nterms>1):
     ia.open(newtay[1]);
     ia.putchunk(ptays[1]);
     ia.close();
   if(nterms>2):
     ia.open(newtay[2]);
     ia.putchunk(ptays[2]);
     ia.close();
   if(nterms>3):
     ia.open(newtay[3]);
     ia.putchunk(ptays[3]);
     ia.close();
   if(nterms>4):
     ia.open(newtay[4]);
     ia.putchunk(ptays[4]);
     ia.close();

 ####################################################
def _linfit(ptays, freqs, pcube, wts, pbthresh):
  #print 'Calculating PB Taylor Coefficients by applying Inv Hessian to Taylor-weighted sums'
  nterms=len(ptays);
  hess = np.zeros( (nterms,nterms) );
  rhs = np.zeros( (nterms,1) );
  soln = np.zeros( (nterms,1) );
  shp = ptays[0].shape;
  if(len(freqs) != pcube.shape[2]):
      casalog.post('Mismatch in frequency axes : '+ len(freqs)+ ' and ' + pcube.shape[2] , 'SEVERE');
      return ptays;
  if(len(freqs) != len(wts)):
      casalog.post('Mismatch in lengths of freqs : '+ len(freqs)+ ' and wts : '+ len(wts) , 'SEVERE');
      return ptays;
  
  for ii in range(0,nterms):
    for jj in range(0,nterms):
       hess[ii,jj]= np.mean( freqs**(ii+jj) * wts);

  normval = hess[0,0]
  hess = hess/normval;

  invhess = linalg.inv(hess);
  casalog.post('Hessian : ' + str(hess) , 'NORMAL')
  casalog.post('Inv Hess : ' + str(invhess), 'NORMAL')

  casalog.post('Calculating Taylor-coefficients for the PB spectrum', 'NORMAL')
  
  for x in range(0,shp[0]):
    for y in range(0,shp[1]):
       if pcube[x,y,0] > pbthresh: # Calculate coeffs only where the largest beam is above thresh
           for ii in range(0,nterms):
               rhs[ii,0]=np.mean( (freqs**(ii)) * pcube[x,y,:] * wts)/normval;
           soln = np.dot(invhess,rhs);
           for ii in range(0,nterms):
               ptays[ii][x,y]=soln[ii,0];
    if(x%100 == 0):
       casalog.post('--- finished rows '+str(x)+ ' to '+ str(x+99), 'NORMAL');

  return ptays;  

#############

#############
def _dividePB(nterms,pbcoeffs,targetpbs):
   if(len(pbcoeffs) != nterms or len(targetpbs) != nterms):
        casalog.post("To divide out the PB spectrum, PB coeffs and target images must have same nterms", 'SEVERE')
        return [];
   correctedpbs=[];

   if(nterms==1):
       det = pbcoeffs[0]
       det[abs(det)==0.0]=1.0;
       correctedpbs.append( targetpbs[0] / det);

   if(nterms==2):
       det = pbcoeffs[0]**2;
       det[abs(det)==0.0]=1.0;
       correctedpbs.append( pbcoeffs[0] * targetpbs[0] / det);
       correctedpbs.append( (-1*pbcoeffs[1]*targetpbs[0] + pbcoeffs[0] * targetpbs[1])/det );
       
   if(nterms==3):
       det = pbcoeffs[0]**3;
       det[abs(det)==0.0]=1.0;
       correctedpbs.append( (pbcoeffs[0]**2) * targetpbs[0] / det);
       correctedpbs.append( ( -1*pbcoeffs[0]*pbcoeffs[1]*targetpbs[0] + (pbcoeffs[0]**2)*targetpbs[1]   )/det );
       correctedpbs.append( ( (pbcoeffs[1]**2 - pbcoeffs[0]*pbcoeffs[2])*targetpbs[0] + (-1*pbcoeffs[0]*pbcoeffs[1])*targetpbs[1] + (pbcoeffs[0]**2)*targetpbs[2]   )/det);

   return correctedpbs;

#####
##############################################################################
def _dividePBTaylor(imlist=[],pblist=[],imlistpbcor=[],pbthreshold=0.1):
   casalog.post("Dividing the Image polynomial by the PB polynomial",'NORMAL')

   if len(imlist) != len(pblist):
       casalog.post("Image list and PB list must be of the same length",'SEVERE')
       return False
   if len(imlist) != len(imlistpbcor):
       casalog.post("Image list and imlistpbcor must be of the same length",'SEVERE')
       return False

   nterms = len(imlist)

   # Read PB coefficient images   
   pbcoeffs=[]
   for tay in range(0,nterms):
      if(not os.path.exists(pblist[tay])):
           casalog.post("PB Coeff " + pblist[tay] + " does not exist ", 'SEVERE');
           return False;
      ia.open(pblist[tay]);
      pbcoeffs.append(ia.getchunk());
      ia.close();

   # Read Images to normalize   
   inpimages=[];   
   for tay in range(0,nterms):
      if(not os.path.exists(imlist[tay])):
           casalog.post("Image Coeff " + imlist[tay] + " does not exist ", 'SEVERE');
           return False;
      ia.open(imlist[tay]);
      inpimages.append(ia.getchunk());
      ia.close();

   # Divide the two polynomials.
   normedims = _dividePB(nterms,pbcoeffs,inpimages);
 
   if(len(normedims)==0):
      casalog.post("Could not divide the beam",'SEVERE');
      return False;    

   for tay in range(0,nterms):
      imtemp = imlist[0]
      normname =  imlistpbcor[tay]
      casalog.post("Writing PB-corrected images " + normname);
      if os.path.exists(normname):
          rmcmd = 'rm -rf '+normname
          os.system(rmcmd)
      cpcmd = 'cp -r '+imtemp + ' ' + normname;
      os.system(cpcmd);
      ia.open(normname);
      ia.putchunk(normedims[tay]);
      ia.calcmask(mask='"'+pblist[0]+'"'+'>'+str(pbthreshold));
      ia.close();

   return True;
###################################################

####################################################
def  _compute_alpha_beta(imagename, nterms, taylorlist, residuallist, threshold, beamshape, calcerror):
   imtemplate = imagename+'.image.tt0';
   nameintensity = imagename+'.image.tt0';
   namealpha = imagename+'.image.alpha';
   nameerror = namealpha+'.error';
   namebeta = imagename+'.image.beta';

   #if(os.path.exists(nameintensity)):
   #      rmcmd = 'rm -rf ' + nameintensity;
   #      os.system(rmcmd);
   #print 'Creating new image : ', nameintensity   
   #cpcmd = 'cp -r ' + imtemplate + ' ' + nameintensity;
   #os.system(cpcmd);

   if(os.path.exists(namealpha)):
         rmcmd = 'rm -rf ' + namealpha;
         os.system(rmcmd);
   casalog.post( 'Creating new image : ' + namealpha, 'NORMAL')   
   cpcmd = 'cp -r ' + imtemplate + ' ' + namealpha;
   os.system(cpcmd);

   if(calcerror==True):
       if(os.path.exists(nameerror)):
             rmcmd = 'rm -rf ' + nameerror;
             os.system(rmcmd);
       casalog.post( 'Creating new image : ' + nameerror , 'NORMAL' )   
       cpcmd = 'cp -r ' + imtemplate + ' ' + nameerror;
       os.system(cpcmd);

   if(nterms>2):
     if(os.path.exists(namebeta)):
            rmcmd = 'rm -rf ' + namebeta;
            os.system(rmcmd);
     casalog.post( 'Creating new image : ' +  namebeta, 'NORMAL') 
     cpcmd = 'cp -r ' + imtemplate + ' ' + namebeta;
     os.system(cpcmd);

   ## Open and read the images to compute alpha/beta with
   ptay=[];
   for i in range(0,nterms):
       ia.open(taylorlist[i]);
       ptay.append(ia.getchunk());
       ia.close();

   ## If calc error, open residual images too
   if(calcerror==True):
       pres=[];
       for i in range(0,nterms):
          ia.open(residuallist[i]);
          pres.append(ia.getchunk());
          ia.close();

   ## Initialize arrays of empty images
   #ia.open(nameintensity);
   #intensity = ia.getchunk();
   #intensity.fill(0.0);
   #ia.close();
   ia.open(namealpha);
   alpha = ia.getchunk();
   alpha.fill(0.0);
   ia.close();
   if(calcerror):
     ia.open(nameerror);
     aerror = ia.getchunk();
     aerror.fill(0.0);
     ia.close();
   if(nterms>2):
     ia.open(namebeta);
     beta = ia.getchunk();
     beta.fill(0.0);
     ia.close();


   ## Calc alpha,beta from ptay0,ptay1,ptay2
   #intensity = ptay[0];
   #ia.open(nameintensity);
   #ia.putchunk(intensity);
   #ia.close();

   ptay[0][ptay[0]<threshold]=1.0;
   ptay[1][ptay[0]<threshold]=0.0;
   if(nterms>2):
      ptay[2][ptay[0]<threshold]=0.0;

   alpha = ptay[1]/ptay[0];

   if(nterms>2):
      beta = (ptay[2]/ptay[0]) - 0.5*alpha*(alpha-1);

   ia.open(namealpha);
   ia.putchunk(alpha);
   ia.calcmask(mask='"'+nameintensity+'"'+'>'+str(threshold));
   ia.close();
   if(nterms>2):
     ia.open(namebeta);
     ia.putchunk(beta);
     ia.calcmask(mask='"'+nameintensity+'"'+'>'+str(threshold));
     ia.close();

   # calc error
   if(calcerror):
      aerror =  np.abs(alpha) * np.sqrt( (pres[0]/ptay[0])**2 + (pres[1]/ptay[1])**2 );
      ia.open(nameerror);
      ia.putchunk(aerror);
      ia.calcmask(mask='"'+nameintensity+'"'+'>'+str(threshold));
      ia.close();


   # Set the new restoring beam, if beamshape was used
   if(beamshape != []):
      if(  _set_clean_beam(nameintensity,beamshape) == False ):
           return False;
      if(  _set_clean_beam(namealpha,beamshape) == False):
           return False;
      if(nterms>2):
           if( _set_clean_beam(namebeta,beamshape) == False):
                return False;
      if(calcerror==True):
           if( _set_clean_beam(nameerror,beamshape) == False):
                return False;

####################################################
# Set the restoring beam to the new one.
def _set_clean_beam(imname,beamshape):
      ia.open(imname);
      try:
          ia.setrestoringbeam(major=beamshape[0],minor=beamshape[1],pa=beamshape[2]);
      except Exception, e:
          casalog.post( "Error setting restoring beam : " + e , 'WARN');
          ia.close();
          return False;
      ia.close();
      return True;

####################################################
