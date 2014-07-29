import pylab as pl
from taskinit import *
import shutil
from setjy_helper import *

def predictSolarObjectCompList(objname, epoch, freqs, prefix):
    """
    predictcomp functionality using the new models
    set flux density of a solar system object using Bryan Butler's new
    python model calculation code.
    """
    retval = True
    cleanupcomps = False # leave genenerated cl files
    nfreqs=-1

    (myms, mytb, mycl,myme) = gentools(['ms','tb','cl','me'])
    #freqinc=freqs[0]*1e-6
    if len(freqs) == 1:
      freqinc=freqs[0]*1e-4
    else:
      freqinc=abs(freqs[1]-freqs[0])*1e-2
    freqlist=[]
    for freq in freqs:
      minf = freq-freqinc
      maxf = freq+freqinc
      freqlist.append([minf,maxf]) 
    #mepoch = myme.epoch('UTC', epoch)
    if epoch['m0']['value']==0.0:
        casalog.post('Invalid epoch, '+str(epoch['m0']['value'])+str(epoch['m0']['unit']),'SEVERE');
        raise Exception, "Error"
    epochv = epoch['m0']['value'] 


    # turn user input epoch to mjd

    #import solar_system_setjy as ss_setjy
    import solar_system_setjy as SSSetjy
    #print "sending objname=",objname, " epochv=",epochv, " freqlist=",freqlist
    observatory='ALMA'
    ss_setjy=SSSetjy.solar_system_setjy()
    (errcodes, fluxes, fluxerrs, sizes, dirs)=\
       ss_setjy.solar_system_fd(source_name=objname, MJDs=[epochv], frequencies=freqlist, observatory=observatory, casalog=casalog)
  
    #print "fluxes from ss_setjy=",fluxes
    #if errcodes[0][0] > 0:
    #    raise ValueError("cannot determined flux")
    
    reterr=testerrs(errcodes[0],objname) 
    if reterr == 2: 
        #raise Exception, "Error" 
        casalog.post("Flux densities cannot be determined","SEVERE")
        raise Exception

    dirstring = [dirs[0]['refer'],qa.tos(dirs[0]['m0']),qa.tos(dirs[0]['m1'])]
    # setup componentlists
    # need to set per dir
    # if scalebychan=F, len(freqs) corresponds to nspw selected

    #clpath='/tmp/'
    clpath='./'
    # use the first input frequency
    freqlabel = '%.3fGHz' % (freqs[0]/1.e9)
    tmlabel = '%.1fd' % epoch['m0']['value']
    clabel = objname+'_spw0_'+freqlabel+'_'+tmlabel
    clname = clabel+'.cl'

    if(os.path.exists(clname)):
        print "Removing previous cl file,", clname
        try:
            shutil.rmtree(clname)
        except:
            print "shutil.rmtree failed" 
    index= 2.0
    sptype = 'spectral index'
    #index= 0.0
    #sptype = 'constant'
    
    #print "fluxes=",fluxes
    #print "fluxerrs-=",fluxerrs
    #print "sizes=",sizes
    #print "dirs=",dirs 
    mycl.addcomponent(flux=fluxes[0][0],fluxunit='Jy', polarization="Stokes", dir=dirs[0],
         shape='disk', majoraxis=str(sizes[0][0])+'arcsec', minoraxis=str(sizes[0][1])+'arcsec',
         positionangle=str(sizes[0][2])+'arcsec', freq=['LSRK',str(freqs[0])+'Hz'],
         spectrumtype=sptype, index=index, label=clabel)
    # set tabular, use original input freqs
    if len(freqs)>1:
        mycl.setspectrum(which=0, type='tabular', tabularfreqs=freqs, tabularflux=fluxes[0],
                           tabularframe='TOPO')

    mycl.rename(clname)
    mycl.close(False)
    return (clname)
