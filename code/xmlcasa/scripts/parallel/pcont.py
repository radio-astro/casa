from taskinit import *
from parallel_go import *
from cleanhelper import *
import numpy as np
import time
import os
import pdb


def getchanimage(inimage, outimage, chan, nchan=1):
    """
    create a slice of channels image from cubeimage
    """
    #pdb.set_trace()
    ia.open(inimage)
    modshape=ia.shape()
    if (modshape[3]==1) or (chan > (modshape[3]-1)) :
        return False
    if((nchan+chan) < modshape[3]):
        endchan= chan+nchan-1
    else:
        endchan=modshape[3]-1
    blc=[0,0,modshape[2]-1,chan]
    trc=[modshape[0]-1,modshape[1]-1,modshape[2]-1,endchan]
    sbim=ia.subimage(outfile=outimage, region=rg.box(blc,trc), overwrite=True)
    sbim.close()
    ia.close()
    return True

def putchanimage(cubimage,inim,chan):
    """
    put channel image back to a pre-exisiting cubeimage
    """
    ia.open(inim)
    inimshape=ia.shape()
    imdata=ia.getchunk()
    immask=ia.getchunk(getmask=True)
    ia.close()
    ia.open(cubimage)
    cubeshape=ia.shape()
    blc=[0,0,inimshape[2]-1,chan]
    trc=[inimshape[0]-1,inimshape[1]-1,inimshape[2]-1,chan+inimshape[3]-1]
    if( not (cubeshape[3] > (chan+inimshape[3]-1))):
        return False

    rg0=ia.setboxregion(blc=blc,trc=trc)
    if inimshape[0:3]!=cubeshape[0:3]: 
        return False
    #ia.putchunk(pixels=imdata,blc=blc)
    ia.putregion(pixels=imdata,pixelmask=immask, region=rg0)
    ia.close()
    ia.removefile(inim)
    return True



def findchansel(msname='', spwids=[], numpartition=1, beginfreq=0.0, endfreq=1e12, continuum=True):
    numproc=numpartition
    spwsel=[]
    startsel=[]
    nchansel=[]
    for k in range(numproc):
        spwsel.append([])
        startsel.append([])
        nchansel.append([])
    
    tb.open(msname+"/SPECTRAL_WINDOW")
    channum=tb.getcol('NUM_CHAN')
    nspw=tb.nrows()
    if(len(spwids)==0):
        spwids=range(nspw)
    freqs=[]
    for k in range(nspw):
        freqs.append(tb.getcol('CHAN_FREQ', k, 1).flatten())
        
    allfreq=freqs[spwids[0]]
    for k in range(1, len(spwids)) :
        allfreq=np.append(allfreq, freqs[spwids[k]])
     ##number of channels in the ms
    allfreq=np.sort(allfreq)
    numchanperms=len(allfreq)
    print 'number of channels', numchanperms
    minfreq=np.min(allfreq)
    minfreq=minfreq if minfreq > beginfreq else beginfreq
    maxfreq=np.max(allfreq)
    maxfreq=maxfreq if maxfreq < endfreq else endfreq
    startallchan=0
    while (minfreq > allfreq[startallchan]):
        startallchan=startallchan+1
    if(startallchan > (allfreq.size -1) ):
        #return -1
        return -1, -1, -1
    endallchan=allfreq.size -1
    while((maxfreq < allfreq[endallchan]) and (endallchan > 0)):
        endallchan=endallchan-1
    if(endallchan < (startallchan+1)):
        ##fail
        return -1, -1, -1
    totchan=endallchan-startallchan+1
    bandperproc=(allfreq[endallchan]-allfreq[startallchan])/numproc
    chanperproc=totchan/numproc
    if((totchan%numproc) > 0):
        chanperproc=chanperproc+1
   
    startthissel=startallchan
    #endthissel=chanperproc+startthissel-1
    #if(endthissel > (len(allfreq))-1):
    #    enthissel=(len(allfreq))-1
    lowfreq=allfreq[startthissel]
    upfreq=lowfreq+bandperproc
    #pdb.set_trace()
    for k in range(numproc):
        donelow=False;
        doneup=False
        #while ((not donelow) or (not doneup)):
        for uu in range(1):
            #####################
            for j in range(nspw):
                #print lowfreq, upfreq, np.min(freqs[j]), np.max(freqs[j])
                ind=np.argsort(freqs[j])
                if((lowfreq > freqs[j][ind[channum[j]-1]]) or 
                        (upfreq < freqs[j][ind[0]])):
                    #this spectral window is not in
                    continue
                elif((lowfreq >= freqs[j][ind[0]]) and 
                   (upfreq <= freqs[j][ind[channum[j]-1]])):
                    ###whole selected band is in this spw
                    donelow=True
                    doneup=True
                    spwsel[k].append(j)
                    cc=0
                    if(freqs[j][channum[j]-1] > freqs[j][0]):
                        while(freqs[j][ind[cc]] < lowfreq):
                            cc=cc+1
                    else:
                        while(freqs[j][ind[cc]] > upfreq):
                            cc=cc+1
                    #cc= (cc-1) if cc > 0 else 0 
                    startsel[k].append(ind[cc])
                    nchansel[k].append(chanperproc)
                elif((lowfreq <= freqs[j][ind[0]]) and 
                     (upfreq >= freqs[j][ind[channum[j]-1]])):
                    ###this spw is within the whole range
                    spwsel[k].append(j)
                    startsel[k].append(0)
                    nchansel[k].append(channum[j])
                elif((lowfreq >= freqs[j][ind[0]]) and 
                     (lowfreq < freqs[j][ind[channum[j]-1]]) and
                     (upfreq > freqs[j][ind[channum[j]-1]])):
                    ###lowbound in this spw but not upbound
                    donelow=True
                    spwsel[k].append(j)
                    matchlow=0
                    while(lowfreq > freqs[j][ind[matchlow]]):
                        matchlow=matchlow+1
                    matchlow=matchlow-1 if matchlow > 0 else 0
                    if(freqs[j][channum[j]-1] > freqs[j][0]):
                        #positive inc
                        startsel[k].append(ind[matchlow])
                        nchansel[k].append(channum[j]-ind[matchlow])
                    else:
                        #neg inc
                        startsel[k].append(0)
                        nchansel[k].append(ind[matchlow])
                elif((upfreq > freqs[j][ind[0]]) and 
                     (upfreq <= freqs[j][ind[channum[j]-1]]) and
                     (lowfreq < freqs[j][ind[0]])):
                    ###upbound in this spw but not lowbound
                    doneup=True
                    spwsel[k].append(j)    
                    matchup=0
                    while(upfreq < freqs[j][ind[matchup]]):
                        matchup=matchup+1
                    matchup=matchup-1 if matchup > 0 else 0
                    if(freqs[j][channum[j]-1] > freqs[j][0]):
                        #positive inc
                        startsel[k].append(0)
                        nchansel[k].append(ind[matchup]+1)
                    else:
                        #neg inc
                        startsel[k].append(ind[matchup])
                        nchansel[k].append(channum[j]-ind[matchup]+1)
                else:
                    print 'Huh ?'
        #startthissel=endthissel
        #endthissel=chanperproc+startthissel-1
        #if(endthissel > (len(allfreq))-1):
        #    enthissel=(len(allfreq))-1
        lowfreq=upfreq
        upfreq=lowfreq+bandperproc
    if(continuum):
        #no need to send same data to different processors
        for k in range (numproc-1):
            for j in range(k+1, numproc):
                for kk in range(len(spwsel[k])):
                    spwtopop=[]
                    for jj in range(len(spwsel[j])):
                        if(spwsel[k][kk] == spwsel[j][jj]):
                            while(startsel[j][jj] < (startsel[k][kk]+nchansel[k][kk])):
                                startsel[j][jj]=startsel[j][jj]+1
                                nchansel[j][jj]=nchansel[j][jj]-1
                            if(nchansel[j][jj] <=0):
                                spwtopop.append(jj)
                    for jj in range(len(spwtopop)):
                        startsel[j].pop(spwtopop[jj])
                        spwsel[j].pop(spwtopop[jj])
                        nchansel[j].pop(spwtopop[jj])
                                
    return spwsel, startsel, nchansel

def copyimage(inimage='', outimage='', init=False, initval=0.0):
    ia.fromimage(outfile=outimage, infile=inimage, overwrite=True)
    ia.open(outimage)
    if(init):
        ia.set(initval)
    else:
        ib=iatool.create()
        ib.open(inimage)
        arr=ib.getchunk()
        ib.done()
        ia.putchuck(arr)
    ia.done()

def pcont(msname=None, imagename=None, imsize=[1000, 1000], 
          pixsize=['1arcsec', '1arcsec'], phasecenter='', 
          field='', spw='*', ftmachine='ft', wprojplanes=128, facets=1, 
          hostnames='', 
          numcpuperhost=1, majorcycles=1, niter=1000, alg='clark', 
          contclean=False, visinmem=False):

    """
    msname= measurementset
    imagename = image
    imsize = list of 2 numbers  [nx,ny] defining image size in x and y
    pixsize = list of 2 quantities   ['sizex', 'sizey'] defining the pixel size e.g  ['1arcsec', '1arcsec']
    phasecenter = an integer or a direction string   integer is fieldindex or direction e.g  'J2000 19h30m00 -30d00m00'
    field = field selection string ...msselection style
    spw = spw selection string ...msselection style
    ftmachine= the ftmachine to use ...'ft', 'wproject' etc
    wprojplanes is an interger that is valid only of ftmachine is 'wproject', 
    facets= integer do split image facet, 
    hostnames= list of strings ..empty string mean localhost
    numcpuperhos = integer ...number of processes to launch on each host
    majorcycles= integer number of CS major cycles to do, 
    niter= integer ...total number of clean iteration 
    alg= string  possibilities are 'clark', 'hogbom', 'msclean'
    contclean = boolean ...if False the imagename.model is deleted if its on 
    disk otherwise clean will continue from previous run
    visinmem = load visibility in memory for major cycles...make sure totalmemory  available to all processes is more than the MS size
    """


    niterpercycle=niter/majorcycles
    if(niterpercycle == 0):
        niterpercycle=niter
        majorcycles=1
    spwids=ms.msseltoindex(vis=msname, spw=spw)['spw']
    c=cluster()
    hostname=os.getenv('HOSTNAME')
    wd=os.getcwd()
    owd=wd
    timesplit=0
    timeimage=0
    model=imagename+'.model' if (len(imagename) != 0) else 'elmodel'
    os.system('rm -rf '+'tempmodel')
    if(not contclean):
        os.system('rm -rf '+model)
        os.system('rm -rf '+imagename+'.image')
    ###num of cpu per node
    numcpu=numcpuperhost
    if((hostnames==[]) or (hostnames=='')): 
        hostnames=[hostname]
    print 'Hosts ', hostnames
    time1=time.time()
    print 'output will be in directory', owd
    for hostname in hostnames:
        c.start_engine(hostname,numcpu,owd)
    numcpu=numcpu*len(hostnames)
    ###spw and channel selection
    spwsel,startsel,nchansel=findchansel(msname, spwids, numcpu)

    out=range(numcpu)  
    c.pgc('from  parallel.parallel_cont import *')
    spwlaunch='"'+spw+'"' if (type(spw)==str) else str(spw)
    fieldlaunch='"'+field+'"' if (type(field) == str) else str(field)
    pslaunch='"'+phasecenter+'"' if (type(phasecenter) == str) else str(phasecenter)
    launchcomm='a=imagecont(ftmachine='+'"'+ftmachine+'",'+'wprojplanes='+str(wprojplanes)+',facets='+str(facets)+',pixels='+str(imsize)+',cell='+str(pixsize)+', spw='+spwlaunch +',field='+fieldlaunch+',phasecenter='+pslaunch+')'
    print 'launch command', launchcomm
    c.pgc(launchcomm)
    c.pgc('a.visInMem='+str(visinmem))
    tb.open(msname+"/SPECTRAL_WINDOW")
    freqs=tb.getcol('CHAN_FREQ')
    allfreq=freqs[:, spwids[0]]
    for k in range(1, len(spwids)) :
        allfreq=np.append(allfreq, freqs[:,spwids[k]])
    ##number of channels in the ms
    numchanperms=len(allfreq)
    print 'number of channels', numchanperms
    minfreq=np.min(allfreq)
    band=np.max(allfreq)-minfreq
    minfreq=minfreq-(band/2.0);
    ###need to get the data selection for each process here
    ## this algorithm is a poor first try
    if(minfreq <0 ):
        minfreq=0.0
    band=band*1.5;
    freq='"%s"'%(str(minfreq)+'Hz')
    band='"%s"'%(str(band)+'Hz')
    tb.done()
    ###define image names
    imlist=[]
    for k in range(numcpu):
        substr='spw'
        for u in range(len(spwsel[k])):
            if(u==0):
                substr=substr+'_'+str(spwsel[k][u])+'_chan_'+str(startsel[k][u])
            else:
                substr=substr+'_spw_'+str(spwsel[k][u])+'_chan_'+str(startsel[k][u])
        imlist.append(substr)
    #####
    ##continue clean or not
    if(contclean and os.path.exists(model)):
        for k in range(numcpu):
            copyimage(inimage=model, outimage=imlist[k]+'.model', init=False)
    
    for maj in range(majorcycles):
        for k in range(numcpu):
            imnam='"%s"'%(imlist[k])
            runcomm='a.imagecont(msname='+'"'+msname+'", start='+str(startsel[k])+', numchan='+str(nchansel[k])+', field="'+str(field)+'", spw='+str(spwsel[k])+', freq='+freq+', band='+band+', imname='+imnam+')'
            print 'command is ', runcomm
            out[k]=c.odo(runcomm,k)
        over=False
        while(not over):
            time.sleep(5)
            overone=True
            for k in range(numcpu):
                overone=(overone and c.check_job(out[k],False)) 
            over=overone
        residual=imlist[0]+'.residual'
        psf=imlist[0]+'.psf'
        if(maj==0):
            copyimage(inimage=residual, outimage='lala.mask', init=True, initval=1.0)
            if(not contclean or (not os.path.exists(model))):
                copyimage(inimage=residual, outimage=model, 
                          init=True, initval=0.0)    
        ia.open(residual)
        for k in range(1, len(imlist)) :
            ia.calc('"'+residual+'" +  "'+imlist[k]+'.residual"')
        ia.calc('"'+residual+'"'+'/'+str(len(imlist)))
        ia.done()
        #incremental clean...get rid of tempmodel
        os.system('rm -rf '+'tempmodel')
        rundecon='a.cleancont(alg="'+str(alg)+'", niter='+str(niterpercycle)+',psf="'+psf+'", dirty="'+residual+'", model="'+'tempmodel'+'", mask='+'"lala.mask")'
        print 'Deconvolution command', rundecon
        out[0]=c.odo(rundecon,0)
        over=False
        while(not over):
            time.sleep(5)
            over=c.check_job(out[0],False)
        ###incremental added to total 
        ia.open(model)
        ia.calc('"'+model+'" +  "tempmodel"')
        ia.done()
        ia.open(model)
        arr=ia.getchunk()
        print 'min max of model', np.min(arr), np.max(arr)
        ia.done()
        for k in range(len(imlist)):
            ia.open(imlist[k]+'.model')
            ia.putchunk(arr)
            ia.done()
    restored=imlist[0]+'.image'
    ia.open(restored)
    for k in range(1, len(imlist)) :
        ia.calc('"'+restored+'" +  "'+imlist[k]+'.image"')
    ia.calc('"'+restored+'"'+'/'+str(len(imlist)))
    ia.done()
    os.system('rm -rf '+imagename+'.image')
    os.system('mv '+restored+'  '+imagename+'.image')
    time2=time.time()
    
    print 'Time to image is ', (time2-time1)/60.0, 'mins'
    c.stop_cluster()

def pcube(msname=None, imagename='elimage', imsize=[1000, 1000], 
          pixsize=['1arcsec', '1arcsec'], phasecenter='', 
          field='', spw='*', ftmachine='ft', wprojplanes=128, facets=1, 
          hostnames='', 
          numcpuperhost=1, majorcycles=1, niter=1000, alg='clark',
          mode='channel', start=0, nchan=1, step=1, weight='natural', 
          imagetilevol=1000000,
          contclean=False, chanchunk=1, visinmem=False, painc=360., pblimit=0.1, dopbcorr=True, applyoffsets=False, cfcache='cfcache.dir'):

    """
    msname= measurementset
    imagename = image
    imsize = list of 2 numbers  [nx,ny] defining image size in x and y
    pixsize = list of 2 quantities   ['sizex', 'sizey'] defining the pixel size e.g  ['1arcsec', '1arcsec']
    phasecenter = an integer or a direction string   integer is fieldindex or direction e.g  'J2000 19h30m00 -30d00m00'
    field = field selection string ...msselection style
    spw = spw selection string ...msselection style
    ftmachine= the ftmachine to use ...'ft', 'wproject' etc
    wprojplanes is an interger that is valid only of ftmachine is 'wproject', 
    facets= integer do split image facet, 
    hostnames= list of strings ..empty string mean localhost
    numcpuperhos = integer ...number of processes to launch on each host
    majorcycles= integer number of CS major cycles to do, 
    niter= integer ...total number of clean iteration 
    alg= string  possibilities are 'clark', 'hogbom', 'multiscale' and their 'mf'
    weight= type of weight to apply
    contclean = boolean ...if False the imagename.model is deleted if its on 
    disk otherwise clean will continue from previous run
    chanchunk = number of channel to process at a go per process...careful not to 
   go above total memory available
   visinmem = load visibility in memory for major cycles...make sure totalmemory  available to all processes is more than the MS size
=====parameters for pbwproject

    """
    spwids=ms.msseltoindex(vis=msname, spw=spw)['spw']
    c=cluster()
    hostname=os.getenv('HOSTNAME')
    wd=os.getcwd()
    owd=wd
    model=imagename+'.model' 
    if(not contclean or (not os.path.exists(model))):
        os.system('rm -rf '+model)
        os.system('rm -rf '+imagename+'.image')
        ##create the cube
        im.selectvis(vis=msname, spw=spw, field=field)
        im.defineimage(nx=imsize[0], ny=imsize[1], cellx=pixsize[0], celly=pixsize[1], 
                       phasecenter='', mode=mode, spw=spwids.tolist(), nchan=nchan, step=step, start=start)
        im.setoptions(imagetilevol=imagetilevol) 
        im.make(model)
        im.done()
    ia.open(model)
    fstart=ia.toworld([0,0,0,0],'n')['numeric'][3]
    fstep=ia.toworld([0,0,0,1],'n')['numeric'][3]-fstart
    fend=fstep*(nchan+1)+fstart
    ia.done()
    os.system('rm -rf '+imagename+'.image')
    os.system('cp -r '+model+'  '+imagename+'.image')
    os.system('cp -r '+model+'  '+imagename+'.residual')
    ###num of cpu per node
    numcpu=numcpuperhost
    if((hostnames==[]) or (hostnames=='')): 
        hostnames=[hostname]
    print 'Hosts ', hostnames
    time1=time.time()
    print 'output will be in directory', owd
    for hostname in hostnames:
        c.start_engine(hostname,numcpu,owd)
    numcpu=numcpu*len(hostnames)

    out=range(numcpu)  
    c.pgc('from  parallel.parallel_cont import *')
    spwlaunch='"'+spw+'"' if (type(spw)==str) else str(spw)
    fieldlaunch='"'+field+'"' if (type(field) == str) else str(field)
    pslaunch='"'+phasecenter+'"' if (type(phasecenter) == str) else str(phasecenter)
    launchcomm='a=imagecont(ftmachine='+'"'+ftmachine+'",'+'wprojplanes='+str(wprojplanes)+',facets='+str(facets)+',pixels='+str(imsize)+',cell='+str(pixsize)+', spw='+spwlaunch +',field='+fieldlaunch+',phasecenter='+pslaunch+',weight="'+weight+'")'
    print 'launch command', launchcomm
    c.pgc(launchcomm)
    ###set some common parameters
    c.pgc('a.imagetilevol='+str(imagetilevol))
    c.pgc('a.visInMem='+str(visinmem))
    c.pgc('a.painc='+str(painc))
    c.pgc('a.cfcache='+'"'+str(cfcache)+'"')
#        self.pblimit=0.1
#        self.dopbcorr=True
#        self.applyoffsets=False


    chancounter=0
    nchanchunk=nchan/chanchunk if (nchan%chanchunk) ==0 else nchan/chanchunk+1
    ###spw and channel selection
    spwsel,startsel,nchansel=findchansel(msname, spwids, nchanchunk, beginfreq=fstart, endfreq=fend, continuum=False)
    imnam='"%s"'%(imagename)
    donegetchan=np.array(range(nchanchunk),dtype=bool)
    doneputchan=np.array(range(nchanchunk),dtype=bool)
    readyputchan=np.array(range(nchanchunk), dtype=bool)
    donegetchan.setfield(False,bool)
    doneputchan.setfield(False,bool)
    readyputchan.setfield(False, bool)
    chanind=np.array(range(numcpu), dtype=int)
    
    while(chancounter < nchanchunk):
        chanind.setfield(-1, int)
        for k in range(numcpu):
            if(chancounter < nchanchunk):
                chanind[k]=chancounter
                if(not donegetchan[chancounter]):
                    getchanimage(model, imagename+str(chancounter)+'.model', 
                                 chancounter*chanchunk, chanchunk)
                    donegetchan[chancounter]=True
                runcomm='a.imagechan(msname='+'"'+msname+'", start='+str(startsel[chancounter])+', numchan='+str(nchansel[chancounter])+', field="'+str(field)+'", spw='+str(spwsel[chancounter])+', imroot='+imnam+',imchan='+str(chancounter)+',niter='+str(niter)+',alg="'+alg+'", majcycle='+str(majorcycles)+')'
                print 'command is ', runcomm
                out[k]=c.odo(runcomm,k)
                chancounter=chancounter+1
        over=False
        while(not over):
            time.sleep(5)
            for k in range(nchanchunk):
                ###split the remaining channel image while the master is waiting
                if(not donegetchan[k]):
                    getchanimage(model, imagename+str(k)+'.model', k*chanchunk, chanchunk)
                    donegetchan[k]=True
                if(readyputchan[k] and (not doneputchan[k])):
                    putchanimage(model, imagename+str(k)+'.model', k*chanchunk)
                    putchanimage(imagename+'.residual', imagename+str(k)+'.residual', k*chanchunk)
                    putchanimage(imagename+'.image', imagename+str(k)+'.image', k*chanchunk)
                    doneputchan[k]=True
            overone=True
            for k in range(numcpu):
                overone=(overone and c.check_job(out[k],False))
                if((chanind[k] > -1) and c.check_job(out[k],False) and 
                   (not readyputchan[chanind[k]])):
                    readyputchan[chanind[k]]=True
            over=overone
    ##sweep the remainder channels in case they are missed
    for k in range(nchanchunk):
         if(not doneputchan[k]):
             putchanimage(model, imagename+str(k)+'.model', k*chanchunk)
             putchanimage(imagename+'.residual', imagename+str(k)+'.residual', k*chanchunk)
             putchanimage(imagename+'.image', imagename+str(k)+'.image', k*chanchunk)
             doneputchan[k]=True
    time2=time.time()
    print 'Time to image is ', (time2-time1)/60.0, 'mins'
    c.stop_cluster()


    def pcubemultims(msnames=[], imagename='elimage', imsize=[1000, 1000], 
          pixsize=['1arcsec', '1arcsec'], phasecenter='', 
          field='', spw='*', ftmachine='ft', wprojplanes=128, facets=1, 
          hostnames=[], 
          numcpuperhost=1, majorcycles=1, niter=1000, alg='clark',
          mode='channel', start=0, nchan=1, step=1, weight='natural', 
          imagetilevol=1000000,
          contclean=False):
        if len(msnames)==0:
            return
        if (len(msnames) != len(hostnames)):
            raise 'Number of MSs and hosts has to match for now' 
            return
        
