from taskinit import *
from parallel_go import *
from cleanhelper import *
from parallel.parallel_cont import imagecont
from odict import *
import numpy as np
import random
import string
import time
import os
import shutil
import pdb
class pimager():
    def __init__(self):
        self.msinfo=odict()
        self.spw=''
        self.field=''
        self.phasecenter=''
        self.ftmachine='ft'
        self.wprojplanes=32
        self.facets=1
        self.imsize=[512, 512]
        self.cell=['1arcsec', '1arcsec']
        self.weight='natural'
        self.visinmem=False

    @staticmethod
    def averimages(outimage='outimage', inimages=[]):
        if((type(inimages)==list) and (len (inimages)==0)):
            return False
        if(os.path.exists(outimage)):
            shutil.rmtree(outimage)
        if(type(inimages)==list):
            shutil.copytree(inimages[0], outimage)
            ia.open(outimage)        
            for k in range(1, len(inimages)) :
                ia.calc('"'+outimage+'" +  "'+inimages[k]+'"')
            ia.calc('"'+outimage+'"'+'/'+str(len(inimages)))
            ia.done()
        elif(type(inimages)==str):
            shutil.copytree(inimages, outimage)
        else:
            return False
        return True

    @staticmethod
    def findchanselcont(msname='', spwids=[], numpartition=1, beginfreq=0.0, endfreq=1e12, freqrange=[0, 1e12]):
        numproc=numpartition
        spwsel=[]
        startsel=[]
        nchansel=[]
        for k in range(numproc):
            spwsel.append([])
            startsel.append([])
            nchansel.append({})

        tb.open(msname+"/SPECTRAL_WINDOW")
        channum=tb.getcol('NUM_CHAN')
        nspw=tb.nrows()
        if(len(spwids)==0):
            spwids=range(nspw)
        freqs=[]
        for k in range(nspw):
            freqs.append(tb.getcol('CHAN_FREQ', k, 1).flatten())

        allfreq=freqs[spwids[0]]
        flatspw=np.ndarray((channum[spwids[0]]), int)
        flatspw[:]=spwids[0]
        flatchan=np.array(range(channum[spwids[0]]))
        for k in range(1, len(spwids)) :
            allfreq=np.append(allfreq, freqs[spwids[k]])
            tpflatspw=np.ndarray((channum[spwids[k]]), int)
            tpflatspw[:]=spwids[k]
            flatspw=np.append(flatspw, tpflatspw)
            flatchan=np.append(flatchan, np.array(range(channum[spwids[k]])))
         ##number of channels in the ms
        #pdb.set_trace()
        sortind=np.argsort(allfreq)
        allfreq=np.sort(allfreq)
        numchanperms=len(allfreq)
        #print 'number of channels', numchanperms
        minfreq=np.min(allfreq)
        if(minfreq > endfreq):
            return -1, -1, -1
        minfreq=minfreq if minfreq > beginfreq else beginfreq
        maxfreq=np.max(allfreq)
        if(maxfreq < beginfreq):
            return -1, -1, -1
        maxfreq=maxfreq if maxfreq < endfreq else endfreq
        ###modify the beginfreq and endfreq
        freqrange[0]=minfreq
        freqrange[1]=maxfreq
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
        extrachan=0
        if((totchan%numproc) > 0):
            chanperproc=chanperproc+1
            extrachan=1
        actualchan=0

        spwcounter=0
        for k in range(numproc):
            spwsel[k].append(flatspw[sortind[actualchan]])
            startsel[k].append(flatchan[sortind[actualchan]])
            for j in range(chanperproc):
                if(((actualchan%chanperproc) > 0) and (flatspw[sortind[actualchan]] !=flatspw[sortind[actualchan-1]])):
                    nchansel[k].update({flatspw[sortind[actualchan-1]]:(flatchan[sortind[actualchan-1]]-startsel[k][spwcounter]+1)})
                    if(not np.any(np.array(spwsel[k])==flatspw[sortind[actualchan]])):
                        spwcounter=spwcounter+1
                        spwsel[k].append(flatspw[sortind[actualchan]])
                        startsel[k].append(flatchan[sortind[actualchan]])
                    else:
                        ###we are back on a spw that is already selected
                        nchansel[k].pop(flatspw[sortind[actualchan]])
                        spwcounter=0
                        while (spwsel[k][spwcounter] != flatspw[sortind[actualchan]]):
                            spwcounter+=1
                actualchan=actualchan+1
                if(actualchan==totchan):
                    break
            #pdb.set_trace()
            if(len(nchansel[k]) != len(spwsel[k])):
                nchansel[k].update({spwsel[k][spwcounter]:(flatchan[sortind[actualchan-1]]-startsel[k][spwcounter]+1)})
            if(actualchan==totchan):
                break
            spwcounter=0
        retchan=[]
        for k in range(numproc):
            retchan.append([])
            for j in range(len(nchansel[k])):
                retchan[k].append(nchansel[k][spwsel[k][j]])

        return spwsel, startsel, retchan

    @staticmethod
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
        #print 'number of channels', numchanperms
        minfreq=np.min(allfreq)
        if(minfreq > endfreq):
            return -1, -1, -1
        minfreq=minfreq if minfreq > beginfreq else beginfreq
        maxfreq=np.max(allfreq)
        if(maxfreq < beginfreq):
            return -1, -1, -1
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
        extrachan=0
        if((totchan%numproc) > 0):
            chanperproc=chanperproc+1
            extrachan=1
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
                        cc= (cc-extrachan) if (cc-extrachan) > 0 else 0 
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
                        while(upfreq > freqs[j][ind[matchup]]):
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
            startthissel=startthissel+chanperproc-extrachan
            lowfreq=upfreq
            #lowfreq=upfreq if (upfreq < allfreq[startthissel]) else allfreq[startthissel]
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

    @staticmethod
    def copyimage(inimage='', outimage='', init=False, initval=0.0):
        ia.fromimage(outfile=outimage, infile=inimage, overwrite=True)
        ia.open(outimage)
        if(init):
            ia.set(initval)
        else:
            ####ib=iatool.create()
            ####ib.open(inimage)
            ####arr=ib.getchunk()
            ####ib.done()
            ia.insert(inimage, locate=[0,0,0,0])
        ia.done()

    def pcont(self, msname=None, imagename=None, imsize=[1000, 1000], 
              pixsize=['1arcsec', '1arcsec'], phasecenter='', 
              field='', spw='*', ftmachine='ft', wprojplanes=128, facets=1, 
              hostnames='', 
              numcpuperhost=1, majorcycles=1, niter=1000, threshold='0.0mJy', alg='clark', scales=[0], weight='natural',
              contclean=False, visinmem=False, interactive=False,
              painc=360., pblimit=0.1, dopbcorr=True, applyoffsets=False, cfcache='cfcache.dir',
              epjtablename=''):

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
        threshold=quantity ...residual peak at which to stop deconvolving
        alg= string  possibilities are 'clark', 'hogbom', 'msclean'
        scales = scales to use when using alg='msclean'
        contclean = boolean ...if False the imagename.model is deleted if its on 
        disk otherwise clean will continue from previous run
        visinmem = load visibility in memory for major cycles...make sure totalmemory  available to all processes is more than the MS size
        painc = Parallactic angle increment in degrees after which a new convolution function is computed (default=360.0deg)
        cfcache = The disk cache directory for convolution functions
        pblimit = The fraction of the peak of the PB to which the PB corrections are applied (default=0.1)
        dopbcorr = If true, correct for PB in the major cycles as well
        applyoffsets = If true, apply antenna pointing offsets from the pointing table given by epjtablename 
        epjtablename = Table containing antenna pointing offsets
        """


        niterpercycle=niter/majorcycles
        if(niterpercycle == 0):
            niterpercycle=niter
            majorcycles=1
        if(spw==''):
            spw='*'
        if(field==''):
            field='*'
        spwids=ms.msseltoindex(vis=msname, spw=spw)['spw']
        c=cluster()
        if (len(c.get_ids()) > 0 or len(c.get_nodes()) > 0 or 
            len(c.get_engines())):
            c.stop_cluster()
            time.sleep(1)
        hostname=os.getenv('HOSTNAME')
        wd=os.getcwd()
        owd=wd
        timesplit=0
        timeimage=0
        model=imagename+'.model' if (len(imagename) != 0) else 'elmodel'
        shutil.rmtree('tempmodel', True)
        if(not contclean):
            print "Removing ", model, 'and', imagename+'.image'
            shutil.rmtree(model, True)
            shutil.rmtree(imagename+'.image', True)
            shutil.rmtree(imagename+'.residual', True)

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
        spwsel,startsel,nchansel=self.findchanselcont(msname, spwids, numcpu)

        print 'SPWSEL ', spwsel, startsel, nchansel 

        out=range(numcpu)  
        c.pgc('casalog.filter()')
        c.pgc('from  parallel.parallel_cont import *')
        spwlaunch='"'+spw+'"' if (type(spw)==str) else str(spw)
        fieldlaunch='"'+field+'"' if (type(field) == str) else str(field)
        pslaunch='"'+phasecenter+'"' if (type(phasecenter) == str) else str(phasecenter)
        launchcomm='a=imagecont(ftmachine='+'"'+ftmachine+'",'+'wprojplanes='+str(wprojplanes)+',facets='+str(facets)+',pixels='+str(imsize)+',cell='+str(pixsize)+', spw='+spwlaunch +',field='+fieldlaunch+',phasecenter='+pslaunch+',weight="'+weight+'")'
        print 'launch command', launchcomm
        c.pgc(launchcomm);
        c.pgc('a.visInMem='+str(visinmem));
        #c.pgc('a.painc='+str(painc));
        #c.pgc('a.cfcache='+'"'+str(cfcache)+'"');
        #c.pgc('a.pblimit='+str(pblimit));
        #c.pgc('a.dopbcorr='+str(dopbcorr));
        #c.pgc('a.applyoffsets='+str(applyoffsets));
        #c.pgc('a.epjtablename='+'"'+str(epjtablename)+'"');


        tb.open(msname+"/SPECTRAL_WINDOW")
        #freqs=tb.getcol('CHAN_FREQ')
        allfreq=tb.getcol('CHAN_FREQ', spwids[0],1)
        for k in range(1, len(spwids)) :
            allfreq=np.append(allfreq, tb.getcol('CHAN_FREQ', spwids[k],1))
        ##number of channels in the ms
        numchanperms=len(allfreq)
        #print 'number of channels', numchanperms
        minfreq=np.min(allfreq)
        band=np.max(allfreq)-minfreq
        ##minfreq=minfreq-(band/2.0);
        ###need to get the data selection for each process here
        ## this algorithm is a poor first try
        if(minfreq <0 ):
            minfreq=0.0
        freq='"%s"'%(str((minfreq+band/2.0))+'Hz')
        band='"%s"'%(str((band*1.1))+'Hz')
        tb.done()
        ###define image names
        imlist=[]
        char_set = string.ascii_letters
        cfcachelist=[]
        for k in range(numcpu):
            #substr='spw'
            #for u in range(len(spwsel[k])):
            #    if(u==0):
            #       substr=substr+'_'+str(spwsel[k][u])+'_chan_'+str(startsel[k][u])
            #   else:
            #        substr=substr+'_spw_'+str(spwsel[k][u])+'_chan_'+str(startsel[k][u])
            ####new version to keep filename small
            substr='Temp_'+str(k)+'_'+string.join(random.sample(char_set,8), sep='')
            while (os.path.exists(substr+'.model')):
                substr='Temp_'+str(k)+'_'+string.join(random.sample(char_set,8), sep='')
            ###############
            imlist.append(substr)
            cfcachelist.append(cfcache+'_'+str(k));
        #####
        ##continue clean or not
        if(contclean and os.path.exists(model)):
            for k in range(numcpu):
                self.copyimage(inimage=model, outimage=imlist[k]+'.model', init=False)
        else:
            for k in range(len(imlist)):
                shutil.rmtree(imlist[k]+'.model', True)
        intmask=0
        for maj in range(majorcycles):
            for k in range(numcpu):
                imnam='"%s"'%(imlist[k])
                c.odo('a.cfcache='+'"'+str(cfcachelist[k])+'"',k);
                c.odo('a.painc='+str(painc),k);
                c.odo('a.pblimit='+str(pblimit),k);
                c.odo('a.dopbcorr='+str(dopbcorr),k);
                c.odo('a.applyoffsets='+str(applyoffsets),k);
                c.odo('a.epjtablename='+'"'+str(epjtablename)+'"',k);
                runcomm='a.imagecont(msname='+'"'+msname+'", start='+str(startsel[k])+', numchan='+str(nchansel[k])+', field="'+str(field)+'", spw='+str(spwsel[k])+', freq='+freq+', band='+band+', imname='+imnam+')'
                print 'command is ', runcomm,cfcachelist[k];
                out[k]=c.odo(runcomm,k)
            over=False
            printkounter=0
            while(not over):
                time.sleep(5)
                #printkounter +=1
                overone=True
                for k in range(numcpu):
                    #print 'k', k, out[k]
                    overone=(overone and c.check_job(out[k],False))
                    #if((printkounter==10) and not(c.check_job(out[k],False))):
                    #    print 'job ', k, 'is waiting'
                    #    printkounter=0
                over=overone
            residual=imagename+'.residual'
            psf=imagename+'.psf'
            psfs=range(len(imlist))
            residuals=range(len(imlist))
            restoreds=range(len(imlist))
            for k in range (len(imlist)):
                psfs[k]=imlist[k]+'.psf'
                residuals[k]=imlist[k]+'.residual'
                restoreds[k]=imlist[k]+'.image'
            self.averimages(residual, residuals)
            if (interactive and (intmask==0)):
                if(maj==0):
                    ia.removefile('lala.mask')
                intmask=im.drawmask(residual,'lala.mask');
                print 'intmask', intmask
                if(intmask==1):
                    im.done()
                if(intmask==2):
                    break;
            else:
                if (maj==0):
                    self.copyimage(inimage=residual, outimage='lala.mask', init=True, initval=1.0);
            if(maj==0):
    #            copyimage(inimage=residual, outimage='lala.mask', init=True, initval=1.0)
                if(not contclean or (not os.path.exists(model))):
                    self.copyimage(inimage=residual, outimage=model, 
                              init=True, initval=0.0)    
            #ia.open(residual)
            #for k in range(1, len(imlist)) :
            #   ia.open(residual)
            #    ia.calc('"'+residual+'" +  "'+imlist[k]+'.residual"')
            #   ia.done()
                ############
            #    ia.open(imlist[k]+'.residual')
            #    print 'residual of ', imlist[k], ia.statistics()['min'], ia.statistics()['max']
            #    ia.done()
                ##############
            #ia.open(residual)
            #print 'calc dvision', '"'+residual+'"'+'/'+str(len(imlist))
            #ia.calc('"'+residual+'"'+'/'+str(len(imlist)))
            #print 'Residual', ia.statistics() 
            #ia.done()
            if(maj==0):
                self.averimages(psf, psfs)
            #incremental clean...get rid of tempmodel
            shutil.rmtree('tempmodel', True)
            rundecon='a.cleancont(alg="'+str(alg)+'", thr="'+str(threshold)+'", scales='+ str(scales)+', niter='+str(niterpercycle)+',psf="'+psf+'", dirty="'+residual+'", model="'+'tempmodel'+'", mask='+'"lala.mask")'
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
            ia.open('tempmodel')
            #arr=ia.getchunk()
            print 'min max of incrmodel', ia.statistics()['min'], ia.statistics()['max']
            ia.done()
            ia.open(model)
            #arr=ia.getchunk()
            print 'min max of model', ia.statistics()['min'], ia.statistics()['max']
            ia.done()
            for k in range(len(imlist)):
                ia.open(imlist[k]+'.model')
                #ia.putchunk(arr)
                ia.insert(infile=model, locate=[0,0,0,0])
                ia.done()
        restored=imagename+'.image'
        #ia.open(restored)
        #for k in range(1, len(imlist)) :
        #    ia.calc('"'+restored+'" +  "'+imlist[k]+'.image"')
        #ia.calc('"'+restored+'"'+'/'+str(len(imlist)))
        #ia.done()
        shutil.rmtree(imagename+'.image', True)
        self.averimages(restored, restoreds)
        #shutil.move(restored,  imagename+'.image')
        time2=time.time()
        ###Clean up
        for k in range(len(imlist)):
            shutil.rmtree(imlist[k]+'.model', True)
            shutil.rmtree(imlist[k]+'.psf', True)
            shutil.rmtree(imlist[k]+'.residual', True)
            shutil.rmtree(imlist[k]+'.image', True)
        print 'Time to image is ', (time2-time1)/60.0, 'mins'
        c.stop_cluster()

    def pcube(self, msname=None, imagename='elimage', imsize=[1000, 1000], 
              pixsize=['1arcsec', '1arcsec'], phasecenter='', 
              field='', spw='*', ftmachine='ft', wprojplanes=128, facets=1, 
              hostnames='', 
              numcpuperhost=1, majorcycles=1, niter=1000, threshold='0.0mJy', alg='clark', scales=[0],
              mode='channel', start=0, nchan=1, step=1, weight='natural', 
              imagetilevol=100000,
              contclean=False, chanchunk=1, visinmem=False, 
              painc=360., pblimit=0.1, dopbcorr=True, applyoffsets=False, cfcache='cfcache.dir',
              epjtablename=''): 

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
        threshold=quantity string ...residual peak at which to stop deconvolving
        alg= string  possibilities are 'clark', 'hogbom', 'multiscale' and their 'mf'
        scales= list of scales in pixel for multiscale clean e.g [0, 3, 10]
        weight= type of weight to apply
        contclean = boolean ...if False the imagename.model is deleted if its on 
        disk otherwise clean will continue from previous run
        chanchunk = number of channel to process at a go per process...careful not to 
       go above total memory available
       visinmem = load visibility in memory for major cycles...make sure totalmemory  available to all processes is more than the MS size
        painc = Parallactic angle increment in degrees after which a new convolution function is computed (default=360.0deg)
        cfcache = The disk cache directory for convolution functions
        pblimit = The fraction of the peak of the PB to which the PB corrections are applied (default=0.1)
        dopbcorr = If true, correct for PB in the major cycles as well
        applyoffsets = If true, apply antenna pointing offsets from the pointing table given by epjtablename 
        epjtablename = Table containing antenna pointing offsets
        """

        if(spw==''):
            spw='*'
        if(field==''):
            field='*'
        spwids=ms.msseltoindex(vis=msname, spw=spw)['spw']
        c=cluster()
        if (len(c.get_ids()) > 0 or len(c.get_nodes()) > 0 or 
            len(c.get_engines())):
            c.stop_cluster()
            time.sleep(1)
        myhostname=os.getenv('HOSTNAME')
        wd=os.getcwd()
        owd=wd
       ########################3
        ###num of cpu per node
        numcpu=numcpuperhost
        if((hostnames==[]) or (hostnames=='')): 
            hostnames=[myhostname]
        print 'Hosts ', hostnames
        time1=time.time()
        print 'output will be in directory', owd
        for hostname in hostnames:
            c.start_engine(hostname,numcpu,owd)
        numcpu=numcpu*len(hostnames)
        ##Start an slave for my async use for cleaning up etc here
        c.start_engine(myhostname, 3, owd)
        buddy_id=[numcpu, numcpu+1, numcpu+2]
        c.push(numcpu=numcpu, targets=buddy_id) 
        #####################
        model=imagename+'.model' 
        if(not contclean or (not os.path.exists(model))):
            shutil.rmtree(model, True)
            shutil.rmtree(imagename+'.image', True)
            ##create the cube
            im.selectvis(vis=msname, spw=spw, field=field)
            im.defineimage(nx=imsize[0], ny=imsize[1], cellx=pixsize[0], celly=pixsize[1], 
                           phasecenter=phasecenter, mode=mode, spw=spwids.tolist(), nchan=nchan, step=step, start=start)
            im.setoptions(imagetilevol=imagetilevol) 
            print 'making model image (', model, ') ...'
            im.make(model)
            print 'model image (', model, ') made'
            im.done()
        print 'LOCKS ', tb.listlocks()
        ia.open(model)
        csys=ia.coordsys()
        ###as image will have conversion to LSRK...need to get original stuff
        originsptype=csys.getconversiontype('spectral', showconversion=False)
        csys.setconversiontype(spectral=originsptype)
        fstart=csys.toworld([0,0,0,0],'n')['numeric'][3]
        fstep=csys.toworld([0,0,0,1],'n')['numeric'][3]-fstart
        fend=fstep*(nchan-1)+fstart
        ia.done()
        print 'LOCKS2 ', tb.listlocks()
        imepoch=csys.epoch()
        imobservatory=csys.telescope()
        shutil.rmtree(imagename+'.image', True)
        shutil.rmtree(imagename+'.residual', True)
        shutil.copytree(model, imagename+'.image')
        shutil.copytree(model, imagename+'.residual')

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
        c.pgc('a.pblimit='+str(pblimit));
        c.pgc('a.dopbcorr='+str(dopbcorr));
        c.pgc('a.applyoffsets='+str(applyoffsets));
        c.pgc('a.epjtablename='+'"'+str(epjtablename)+'"');

        tb.clearlocks()
        print 'LOCKS3', tb.listlocks()
        chancounter=0
        nchanchunk=nchan/chanchunk if (nchan%chanchunk) ==0 else nchan/chanchunk+1
        ###spw and channel selection
        spwsel,startsel,nchansel=self.findchansel(msname, spwids, nchanchunk, beginfreq=fstart, endfreq=fend, continuum=True)
        #print 'spwsel', spwsel
        #print 'startsel', startsel
        #print  'nchansel', nchansel
        imnam='"%s"'%(imagename)
        donegetchan=np.array(range(nchanchunk),dtype=bool)
        doneputchan=np.array(range(nchanchunk),dtype=bool)
        readyputchan=np.array(range(nchanchunk), dtype=bool)
        cpudoing=np.array(range(nchanchunk), dtype=int)
        donegetchan.setfield(False,bool)
        doneputchan.setfield(False,bool)
        readyputchan.setfield(False, bool)
        chanind=np.array(range(numcpu), dtype=int)
        c.push(readyputchan=readyputchan, targets=buddy_id)
        #c.push(doneputchan=doneputchan, targets=buddy_id)
        buddy_is_ready=[True, True, True]
        buddy_ref=[False, False, False]
        cleanupcomm=['', '', '']
        cleanupcomm[2]='a.cleanupmodelimages(readyputchan=readyputchan,  imagename='+imnam+', nchanchunk='+str(nchanchunk)+', chanchunk='+str(chanchunk)+')'
        cleanupcomm[1]='a.cleanupresidualimages(readyputchan=readyputchan,  imagename='+imnam+', nchanchunk='+str(nchanchunk)+', chanchunk='+str(chanchunk)+')'
        cleanupcomm[0]='a.cleanuprestoredimages(readyputchan=readyputchan,  imagename='+imnam+', nchanchunk='+str(nchanchunk)+', chanchunk='+str(chanchunk)+')'
        def gen_command(ccounter):
            return 'a.imagechan_new(msname='+'"'+msname+'", start='+str(startsel[ccounter])+', numchan='+str(nchansel[ccounter])+', field="'+str(field)+'", spw='+str(spwsel[ccounter])+', cubeim='+imnam+', imroot='+imnam+',imchan='+str(ccounter)+',chanchunk='+str(chanchunk)+',niter='+str(niter)+',alg="'+alg+'", scales='+str(scales)+', majcycle='+str(majorcycles)+', thr="'+str(threshold)+'")'

        #while(chancounter < nchanchunk):
        chanind.setfield(-1, int)
        for k in range(numcpu):
            if(chancounter < nchanchunk):
                chanind[k]=chancounter
                    #if(not donegetchan[chancounter]):
                    #    imagecont.getchanimage(model, imagename+str(chancounter)+'.model', 
                    #                 chancounter*chanchunk, chanchunk)
                    #    donegetchan[chancounter]=True

                    #runcomm='a.imagechan_new(msname='+'"'+msname+'", start='+str(startsel[chancounter])+', numchan='+str(nchansel[chancounter])+', field="'+str(field)+'", spw='+str(spwsel[chancounter])+', cubeim='+imnam+', imroot='+imnam+',imchan='+str(chancounter)+',chanchunk='+str(chanchunk)+',niter='+str(niter)+',alg="'+alg+'", scales='+str(scales)+', majcycle='+str(majorcycles)+', thr="'+str(threshold)+'")'
                runcomm=gen_command(chancounter)
                    #runcomm='a.imagechan(msname='+'"'+msname+'", start='+str(startsel[chancounter])+', numchan='+str(nchansel[chancounter])+', field="'+str(field)+'", spw='+str(spwsel[chancounter])+', imroot='+imnam+',imchan='+str(chancounter)+',niter='+str(niter)+',alg="'+alg+'", scales='+str(scales)+', majcycle='+str(majorcycles)+', thr="'+str(threshold)+'")'
                print 'command is ', runcomm
                out[k]=c.odo(runcomm,k)
                chancounter=chancounter+1
        while(chancounter < nchanchunk):
                over=False
                while(not over):
                #############loop waiting for a chunk of work
                    time.sleep(1)
                    for bud in range(3):
                        if(buddy_is_ready[bud]):
                            #print 'SENDING ', cleanupcomm[bud]
                            c.push(readyputchan=readyputchan, targets=buddy_id[bud])
                        #c.push(doneputchan=doneputchan, targets=buddy_id)
                            buddy_ref[bud]=c.odo(cleanupcomm[bud], buddy_id[bud])
                        buddy_is_ready[bud]=c.check_job(buddy_ref[bud], False)
                        #print 'buddy_ready', bud, buddy_is_ready[bud]
                #if(buddy_is_ready):
                #    doneputchan=c.pull('doneputchan', buddy_id)[buddy_id]
                    overone=True
                    for k in range(numcpu):
                        overone=(overone and c.check_job(out[k],False))
                        if((chanind[k] > -1) and c.check_job(out[k],False) and 
                           (not readyputchan[chanind[k]])):
                            readyputchan[chanind[k]]=True      
                            if(chancounter < nchanchunk):
                                chanind[k]=chancounter
                                runcomm=gen_command(chancounter)
                                print 'command is ', runcomm
                                print 'processor ', k
                                out[k]=c.odo(runcomm,k)
                                chancounter+=1
                            overone=(overone and c.check_job(out[k],False))
                    over=overone
               ############
        time2=time.time()
        print 'Time to image is ', (time2-time1)/60.0, 'mins'
        ##sweep the remainder channels in case they are missed
        for bud in range(3):
            while(not buddy_is_ready[bud]):
                buddy_is_ready[bud]=c.check_job(buddy_ref[bud], False)
            #doneputchan=c.pull('doneputchan', buddy_id)[buddy_id] 
            c.push(readyputchan=readyputchan, targets=buddy_id[bud])
            buddy_ref[bud]=c.odo(cleanupcomm[bud], buddy_id[bud])
        for bud in range(3):
            while(not buddy_is_ready[bud]):
                buddy_is_ready[bud]=c.check_job(buddy_ref[bud], False)
        #c.stop_engine(buddy_id)
        #for k in range(nchanchunk):
        #   if(not doneputchan[k]):
        #        imagecont.putchanimage(model, imagename+str(k)+'.model', k*chanchunk, False)
        #        imagecont.putchanimage(imagename+'.residual', imagename+str(k)+'.residual', k*chanchunk, False)
        #        imagecont.putchanimage(imagename+'.image', imagename+str(k)+'.image', k*chanchunk, False)
        #        doneputchan[k]=True
        time2=time.time()
        print 'Time to image after cleaning is ', (time2-time1)/60.0, 'mins'
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


    def pcontmultims(self, msnames=[], workdirs=[], imagename=None, imsize=[1000, 1000], 
                     pixsize=['1arcsec', '1arcsec'], phasecenter='', 
                     field='', spw='*', freqrange=['', ''],  ftmachine='ft', wprojplanes=128, facets=1, 
                     hostnames='', 
                     numcpuperhost=1, majorcycles=1, niter=1000, threshold='0.0mJy', alg='clark', weight='natural',
                     contclean=False, visinmem=False,
                     painc=360., pblimit=0.1, dopbcorr=True, applyoffsets=False, cfcache='cfcache.dir',
                     epjtablename=''):
        """
        msnames=  list containing measurementset names
        imagename = image
        imsize = list of 2 numbers  [nx,ny] defining image size in x and y
        pixsize = list of 2 quantities   ['sizex', 'sizey'] defining the pixel size e.g  ['1arcsec', '1arcsec']
        phasecenter = an integer or a direction string   integer is fieldindex or direction e.g  'J2000 19h30m00 -30d00m00'
        field = field selection string ...msselection style
        spw = spw selection string ...msselection style
        freqrange= continuum image frequency bound e.g ['1GHz', '1.23GHz'] . 
        ftmachine= the ftmachine to use ...'ft', 'wproject' etc
        wprojplanes is an interger that is valid only of ftmachine is 'wproject', 
        facets= integer do split image facet, 
        hostnames= list of strings ..empty string mean localhost
        numcpuperhos = integer ...number of processes to launch on each host
        majorcycles= integer number of CS major cycles to do, 
        niter= integer ...total number of clean iteration 
        threshold=quantity ...residual peak at which to stop deconvolving
        alg= string  possibilities are 'clark', 'hogbom', 'msclean'
        scales = scales to use when using alg='msclean'
        contclean = boolean ...if False the imagename.model is deleted if its on 
        disk otherwise clean will continue from previous run
        visinmem = load visibility in memory for major cycles...make sure totalmemory  available to all processes is more than the MS size
        painc = Parallactic angle increment in degrees after which a new convolution function is computed (default=360.0deg)
        cfcache = The disk cache directory for convolution functions
        pblimit = The fraction of the peak of the PB to which the PB corrections are applied (default=0.1)
        dopbcorr = If true, correct for PB in the major cycles as well
        applyoffsets = If true, apply antenna pointing offsets from the pointing table given by epjtablename 
        epjtablename = Table containing antenna pointing offsets
        """
        time1=time.time()
        if len(msnames)==0:
            return    
        niterpercycle=niter/majorcycles
        if(niterpercycle == 0):
            niterpercycle=niter
            majorcycles=1
        num_ext_procs=0
        self.spw=spw
        self.field=field
        self.phasecenter=phasecenter
        self.ftmachine=ftmachine
        self.wprojplanes=wprojplanes
        self.facets=facets
        self.imsize=imsize
        self.cell=pixsize
        self.weight=weight
        self.visinmem=visinmem
        
        self.setupcluster(hostnames,numcpuperhost, num_ext_procs)
        model=imagename+'.model' if (len(imagename) != 0) else 'elmodel'
        if(not contclean):
            print "Removing ", model, 'and', imagename+'.image'
            shutil.rmtree(model, True)
            shutil.rmtree(imagename+'.image', True)
            shutil.rmtree(imagename+'.residual', True)
        shutil.rmtree('tempmodel', True)
        if(len(freqrange) < 2):
            freqrange=['','']
        if(freqrange[0]==''):
            freqrange[0]=0.0
        if(freqrange[1]==''):
            freqrange[1]=1e24
        if(type(freqrange[0])==str):
            freqrange[0]=qa.convert(qa.quantity(freqrange[0], 'Hz'), 'Hz')['value']
        if(type(freqrange[1])==str):
            freqrange[1]=qa.convert(qa.quantity(freqrange[1], 'Hz'), 'Hz')['value']
        numms=len(msnames) if (type(msnames)==list) else 1
        if(type(msnames) == str):
            msnames=[msnames]
        freqrange=self.findfreqranges(msnames=msnames, spw=spw, freqmin=freqrange[0], freqmax=freqrange[1])
        ## the above will return the freqmin, freqmax in the data if freqrange is default
        freq=(freqrange[0]+freqrange[1])/2.0
        band=abs(freqrange[1]-freqrange[0])
        self.makeconttempimages(imagename, contclean)
        def gen_comm(msname, startsel, nchansel, field, spw, freq, band, imname):
            spwsel=str(self.msinfo[msname]['spwids'].tolist())
            freqsel='"%fHz"'%freq
            bandsel='"%fHz"'%band
            return 'a.imagecont(msname='+'"'+msname+'", start='+str(startsel)+', numchan='+str(nchansel)+', field="'+str(field)+'", spw='+spwsel+', freq='+freqsel+', band='+ bandsel+', imname="'+imname+'")'
        ###major cycle
        out=range(self.numcpu)  
        mscpuindex=range(self.numcpu) 
        for maj in range(majorcycles):
            ###set the processed flags off
            processing=odict()
            processed=odict()
            mscounter=0
            for k in range(len(msnames)):
                processing[msnames[k]]=False
                processed[msnames[k]]=False
            myrec=self.msinfo
            ##initial bunch of launches
            for k in range(self.numcpu):
                msname=myrec.keys()[k]
                mscpuindex[k]=msname
                if(mscounter < len(msnames)):
                    runcomm=gen_comm(msname=msname, startsel=myrec[msname]['startsel'], nchansel=myrec[msname]['nchansel'],
                                    field=self.field, spw=myrec[msname]['spwsel'], freq=freq, band=band, imname=myrec[msname]['imname'])
                    print 'command is ', runcomm
                    out[k]=self.c.odo(runcomm,k)
                    mscounter +=1
                    processing[msname]=True
            while (not np.alltrue(processed.values())):
                overone=True
                time.sleep(1)
                for k in range(self.numcpu):
                    if(processing[mscpuindex[k]] and self.c.check_job(out[k],False) and (not processed[mscpuindex[k]])):
                        processed[mscpuindex[k]]=True
                        if(mscounter < len(msnames)):
                            msname=myrec.keys()[mscounter]
                            runcomm=gen_comm(msname=msname, startsel=myrec[msname]['startsel'], nchansel=myrec[msname]['nchansel'],
                                    field=self.field, spw=myrec[msname]['spwsel'], freq=freq, band=band, imname=myrec[msname]['imname'])
                            print 'command is ', runcomm
                            out[k]=self.c.odo(runcomm,k)
                            mscpuindex[k]=msname
                            mscounter +=1
                            processing[msname]=True
            residual=imagename+'.residual'
            psf=imagename+'.psf'
            psfs=range(len(msnames))
            residuals=range(len(msnames))
            restoreds=range(len(msnames))
            for k in range (len(msnames)):
                psfs[k]=self.msinfo[msnames[k]]['imname']+'.psf'
                residuals[k]=self.msinfo[msnames[k]]['imname']+'.residual'
                restoreds[k]=self.msinfo[msnames[k]]['imname']+'.image'
            self.averimages(residual, residuals)
            if(maj==0):
                self.averimages(psf, psfs)
                self.copyimage(inimage=residual, outimage='lala.mask', init=True, initval=1.0)
                if(not contclean or (not os.path.exists(model))):
                    self.copyimage(inimage=residual, outimage=model, 
                                   init=True, initval=0.0)
            self.incrementaldecon(alg=alg, residual=residual, model=model, niter=niterpercycle, psf=psf, mask='lala.mask', thr=threshold, cpuid=0)
        #######
        restored=imagename+'.image'
        self.averimages(restored, restoreds)
        ###close major cycle loop
        #shutil.rmtree(imagename+'.image', True)
        #shutil.move(restored,  imagename+'.image')
        time2=time.time()
        ###Clean up
        #for k in range(len(imlist)):
        #   shutil.rmtree(imlist[k]+'.model', True)
        #  shutil.rmtree(imlist[k]+'.residual', True)
        # shutil.rmtree(imlist[k]+'.image', True)
        print 'Time to image is ', (time2-time1)/60.0, 'mins'
        self.c.stop_cluster()

    def incrementaldecon(self, alg, residual, model, niter, psf,  mask, thr, cpuid):
        ##############
            ia.open(residual)
            print 'Residual', ia.statistics() 
            ia.done()
            ########
            #incremental clean...get rid of tempmodel
            shutil.rmtree('tempmodel', True)
            rundecon='a.cleancont(alg="'+str(alg)+'", niter='+str(niter)+',psf="'+psf+'", dirty="'+residual+'", model="'+'tempmodel'+'", mask='+'"'+mask+'", thr="'+str(thr)+'")'
            print 'Deconvolution command', rundecon
            out=self.c.odo(rundecon,cpuid)
            over=False
            while(not over):
                time.sleep(1)
                over=self.c.check_job(out,False)
            ###incremental added to total 
            ia.open(model)
            ia.calc('"'+model+'" +  "tempmodel"')
            ia.done()
            ia.open('tempmodel')
            #arr=ia.getchunk()
            print 'min max of incrmodel', ia.statistics()['min'], ia.statistics()['max']
            ia.done()
            ia.open(model)
            #arr=ia.getchunk()
            print 'min max of model', ia.statistics()['min'], ia.statistics()['max']
            ia.done()
            msnames=self.msinfo.keys()
            for k in range(len(msnames)):
                imlist=self.msinfo[msnames[k]]['imname']
                ia.open(imlist+'.model')
                #ia.putchunk(arr)
                ia.insert(infile=model, locate=[0,0,0,0])
                ia.done()
        #######
    def makeconttempimages(self, imname,contclean=False): 
        myrec=self.msinfo
        msnames=myrec.keys()
        char_set=string.ascii_letters
        for k in range(len(msnames)):
            substr='Tmp_'+msnames[k]+'_'+string.join(random.sample(char_set,8), sep='')
            while (os.path.exists(substr+'.model')):
                substr='Tmp_'+msnames[k]+'_'+string.join(random.sample(char_set,8), sep='')
            myrec[msnames[k]]['imname']=substr
            model=imname+'.model'
            if(contclean and os.path.exists(model)):
                self.copyimage(inimage=model, outimage=myrec[msnames[k]]['imname']+'.model', init=False)
            else:
                shutil.rmtree(myrec[msnames[k]]['imname']+'.model', True)
    def findfreqranges(self, msnames=[], spw=['*'], freqmin=0.0,  freqmax=1.e24):
        if(type(spw)==str):
            spw=[spw]
        if (len(spw)==1):
            for k in range(1,len(msnames)) :
                spw.append(spw[0])
        retfreqmin=1.0e24
        retfreqmax=-1.0e24
        for k in range(len(msnames)):
            self.msinfo[msnames[k]]=odict()
            myrec=self.msinfo[msnames[k]]
            freqrange=[0,0]
            myrec['spwids']=ms.msseltoindex(vis=msnames[k], spw=spw[k])['spw']
            spws, starts, nchans=self.findchanselcont(
                msname=msnames[k], spwids=myrec['spwids'], 
                numpartition=1, beginfreq=freqmin, 
                endfreq=freqmax, freqrange=freqrange)
            myrec['spwsel']=spws[0]
            myrec['startsel']=starts[0]
            myrec['nchansel']=nchans[0]
            retfreqmax=freqrange[1] if (freqrange[1] > retfreqmax) else retfreqmax
            retfreqmin=freqrange[0] if (freqrange[0] <  retfreqmin) else retfreqmin
        freqmin=retfreqmin
        freqmax=retfreqmax
        return [freqmin, freqmax]

    



    def setupcluster(self, hostnames='', numcpuperhost=1, num_ext_procs=0, workingdir=''):
        """
        This sets up the cluster with numcpuperhost on each of the hostnames
        working dir by default is  pwd or it can be a vector of 1 or nhostnames
        if  1 
        num_ext_procs get to be set on master host (i.e this host) and working dir for them is pwd
        num_ext_procs is basically to help the master process do stuff asynchronously
        """
        self.c=cluster()
        hostname=os.getenv('HOSTNAME')
        wd=os.getcwd()
        #pdb.set_trace()
        if((workingdir=='') or (workingdir==['']) or (workingdir==[])):
            workingdir=[wd]
        if(type(workingdir)==str):
            workingdir=[workingdir]
        self.workingdirs=workingdir
        if((hostnames==[]) or (hostnames=='')): 
            self.hostnames=[hostname]
        else:
            self.hostnames=hostnames
        if(len(self.workingdirs)==1):
            for k in range(1, len(hostnames)):
                self.workingdirs.append(self.workingdirs[0])
        if(len(hostnames) != len(self.workingdirs)):
            raise ValueError, "Length of hostnames and workingdirs do not match"

        for k in range(len(hostnames)):
            hostname=hostnames[k]
            print 'starting ', numcpuperhost, 'on', hostname, 'with wd', self.workingdirs[k]
            self.c.start_engine(hostname,numcpuperhost,self.workingdirs[k])
        self.numcpu=len(hostnames)*numcpuperhost
        self.numextraprocs=num_ext_procs
        if(num_ext_procs >0):
            hostname=os.getenv('HOSTNAME')
            self.c.start_engine(hostname,num_ext_procs,wd)
        ###do the common stuff to all child
        self.c.pgc('casalog.filter()')
        self.c.pgc('from  parallel.parallel_cont import *')
        spwlaunch='"'+self.spw+'"' if (type(self.spw)==str) else str(self.spw)
        fieldlaunch='"'+self.field+'"' if (type(self.field) == str) else str(self.field)
        pslaunch='"'+self.phasecenter+'"' if (type(self.phasecenter) == str) else str(self.phasecenter)
        launchcomm='a=imagecont(ftmachine='+'"'+self.ftmachine+'",'+'wprojplanes='+str(self.wprojplanes)+',facets='+str(self.facets)+',pixels='+str(self.imsize)+',cell='+str(self.cell)+', spw='+spwlaunch +',field='+fieldlaunch+',phasecenter='+pslaunch+',weight="'+self.weight+'")'
        print 'launch command', launchcomm
        self.c.pgc(launchcomm);
        self.c.pgc('a.visInMem='+str(self.visinmem));
        #c.pgc('a.painc='+str(self.painc));
        #c.pgc('a.cfcache='+'"'+str(self.cfcache)+'"');
        #c.pgc('a.pblimit='+str(self.pblimit));
        #c.pgc('a.dopbcorr='+str(self.dopbcorr));
        #c.pgc('a.applyoffsets='+str(self.applyoffsets));
        #c.pgc('a.epjtablename='+'"'+str(self.epjtablename)+'"');

    def pcontmultidiskms(self, msnames=[], workdirs=[], imagename=None, imsize=[1000, 1000], 
                     pixsize=['1arcsec', '1arcsec'], phasecenter='', 
                     field='', spw='*', ftmachine='ft', wprojplanes=128, facets=1, 
                     hostnames='', 
                     numcpuperhost=1, majorcycles=1, niter=1000, threshold='0.0mJy', alg='clark', weight='natural',
                     contclean=False, visinmem=False,
                     painc=360., pblimit=0.1, dopbcorr=True, applyoffsets=False, cfcache='cfcache.dir',
                     epjtablename=''):
        if len(msnames)==0:
            return                    
        niterpercycle=niter/majorcycles
        if(niterpercycle == 0):
            niterpercycle=niter
            majorcycles=1
        c=cluster()
        hostname=os.getenv('HOSTNAME')
        wd=os.getcwd()
        owd=wd
        timesplit=0
        timeimage=0
        model=imagename+'.model' if (len(imagename) != 0) else 'elmodel'
        shutil.rmtree('tempmodel', True)
        if(not contclean):
            print "Removing ", model, 'and', imagename+'.image'
            shutil.rmtree(model, True)
            shutil.rmtree(imagename+'.image', True)
        ###num of cpu per node
        numcpu=numcpuperhost
        if((hostnames==[]) or (hostnames=='')): 
            hostnames=[hostname]
        if (len(msnames) != len(hostnames)):
            raise 'Number of MSs and hosts has to match for now' 
            return
        if (len(msnames) != len(workdirs)):
            raise 'Number of Working Directiories and hosts has to match for now' 
            return
        print 'Hosts ', hostnames
        time1=time.time()
        print 'output will be in directory', owd
        hostdata=odict()
        for k in range(len(hostnames)):
            hostname=hostnames[k]
            myrec=odict()
            myrec['msname']=workdirs[k]+'/'+msnames[k]
            myrec['spwids']=ms.msseltoindex(vis=myrec['msname'], spw=spw)['spw']
            myrec['spwsel'],myrec['startsel'],myrec['nchansel']=self.findchansel(msnames[k], myrec['spwids'], numcpuperhost)
            c.start_engine(hostname,numcpuperhost,workdirs[k])
            hostdata[hostname]=myrec
        ##start an engine locally for deconvolution
        c.start_engine(os.getenv('HOSTNAME'),1,owd)
        numcpu=numcpu*len(hostnames)

        print 'SPWSEL ', hostdata 

        out=range(numcpu)  
        c.pgc('casalog.filter()')
        c.pgc('from  parallel.parallel_cont import *')
        spwlaunch='"'+spw+'"' if (type(spw)==str) else str(spw)
        fieldlaunch='"'+field+'"' if (type(field) == str) else str(field)
        pslaunch='"'+phasecenter+'"' if (type(phasecenter) == str) else str(phasecenter)
        launchcomm='a=imagecont(ftmachine='+'"'+ftmachine+'",'+'wprojplanes='+str(wprojplanes)+',facets='+str(facets)+',pixels='+str(imsize)+',cell='+str(pixsize)+', spw='+spwlaunch +',field='+fieldlaunch+',phasecenter='+pslaunch+',weight="'+weight+'")'
        print 'launch command', launchcomm
        c.pgc(launchcomm);
        #c.pgc('a.visInMem='+str(visinmem));
        #c.pgc('a.painc='+str(painc));
        #c.pgc('a.cfcache='+'"'+str(cfcache)+'"');
        #c.pgc('a.pblimit='+str(pblimit));
        #c.pgc('a.dopbcorr='+str(dopbcorr));
        #c.pgc('a.applyoffsets='+str(applyoffsets));
        #c.pgc('a.epjtablename='+'"'+str(epjtablename)+'"');

        allfreq=np.array([])
        for msid in range(len(hostnames)): 
            msname=msnames[msid]
            myrec=hostdata.values()[msid]
            tb.open(msname+"/SPECTRAL_WINDOW")
            #freqs=tb.getcol('CHAN_FREQ')
            for k in range(len(myrec['spwids'])) :
                allfreq=np.append(allfreq, tb.getcol('CHAN_FREQ',myrec['spwids'][k],1))
            tb.done()
        ##number of channels in the ms
        numchanperms=len(allfreq)
        #print 'number of channels', numchanperms
        minfreq=np.min(allfreq)
        band=np.max(allfreq)-minfreq
        ##minfreq=minfreq-(band/2.0);
        ###need to get the data selection for each process here
        ## this algorithm is a poor first try
        if(minfreq <0 ):
            minfreq=0.0
        freq='"%s"'%(str((minfreq+band/2.0))+'Hz')
        band='"%s"'%(str((band*1.1))+'Hz')
        ###define image names
        imlist=[]
        cfcachelist=[]
        for kk in range(len(hostnames)):
            myrec=hostdata.values()[kk]
            for jj in range(numcpuperhost) :
                k=kk*numcpuperhost+jj
                substr=msnames[kk]+'_spw'
                for u in range(len(myrec['spwsel'][jj])):
                    if(u==0):
                        substr=substr+'_'+str(myrec['spwsel'][jj][u])+'_chan_'+str(myrec['startsel'][jj][u])
                    else:
                        substr=substr+'_spw_'+str(myrec['spwsel'][jj][u])+'_chan_'+str(myrec['startsel'][jj][u])
                imlist.append(workdirs[kk]+'/'+substr)
                cfcachelist.append(cfcache+'_'+str(k));
        #####
        ##continue clean or not
        if(contclean and os.path.exists(model)):
            for k in range(numcpu):
                self.copyimage(inimage=model, outimage=imlist[k]+'.model', init=False)
        else:
            for k in range(len(imlist)):
                shutil.rmtree(imlist[k]+'.model', True)
        for maj in range(majorcycles):
            for jj in range(len(hostnames)):
                msname=msnames[jj]
                for kk in range(numcpuperhost):
                    spwsel=hostdata[hostnames[jj]]['spwsel'][kk]
                    startsel=hostdata[hostnames[jj]]['startsel'][kk]
                    nchansel=hostdata[hostnames[jj]]['nchansel'][kk]
                    k=jj*numcpuperhost+kk
                    imnam='"%s"'%(imlist[k])
                    #c.odo('a.cfcache='+'"'+str(cfcachelist[k])+'"',k)
                    runcomm='a.imagecont(msname='+'"'+msname+'", start='+str(startsel)+', numchan='+str(nchansel)+', field="'+str(field)+'", spw='+str(spwsel)+', freq='+freq+', band='+band+', imname='+imnam+')'
                    print 'command is ', runcomm
                    out[k]=c.odo(runcomm,k)
            over=False
            while(not over):
                time.sleep(5)
                overone=True
                for k in range(numcpu):
                    #print 'k', k, out[k]
                    overone=(overone and c.check_job(out[k],False)) 
                    #print 'overone', overone 
                over=overone
            residual=imagename+'.residual'
            psf=imagename+'.psf'
            psfs=range(len(imlist))
            residuals=range(len(imlist))
            restoreds=range(len(imlist))
            for k in range (len(imlist)):
                psfs[k]=imlist[k]+'.psf'
                residuals[k]=imlist[k]+'.residual'
                restoreds[k]=imlist[k]+'.image'
            self.averimages(residual, residuals)
            if(maj==0):
                    self.averimages(psf, psfs)
                    self.copyimage(inimage=residual, outimage='lala.mask', init=True, initval=1.0)
                    if(not contclean or (not os.path.exists(model))):
                            self.copyimage(inimage=residual, outimage=model, 
                                    init=True, initval=0.0)    
            #ia.open(residual)
            #for k in range(1, len(imlist)) :
            #    ia.open(residual)
            #    ia.calc('"'+residual+'" +  "'+imlist[k]+'.residual"')
            #ia.done()
            #############
            #   ia.open(imlist[k]+'.residual')
            #   print 'residual of ', imlist[k], ia.statistics()['min'], ia.statistics()['max']
            #   ia.done()
            ##############
            ia.open(residual)
            print 'Residual', ia.statistics() 
            ia.done()
            ########
            #incremental clean...get rid of tempmodel
            shutil.rmtree('tempmodel', True)
            rundecon='a.cleancont(alg="'+str(alg)+'", niter='+str(niterpercycle)+',psf="'+psf+'", dirty="'+residual+'", model="'+'tempmodel'+'", mask='+'"lala.mask", thr="'+str(threshold)+'")'
            print 'Deconvolution command', rundecon
            out[0]=c.odo(rundecon,numcpu)
            over=False
            while(not over):
                time.sleep(5)
                over=c.check_job(out[0],False)
            ###incremental added to total 
            ia.open(model)
            ia.calc('"'+model+'" +  "tempmodel"')
            ia.done()
            ia.open('tempmodel')
            #arr=ia.getchunk()
            print 'min max of incrmodel', ia.statistics()['min'], ia.statistics()['max']
            ia.done()
            ia.open(model)
            #arr=ia.getchunk()
            print 'min max of model', ia.statistics()['min'], ia.statistics()['max']
            ia.done()
            for k in range(len(imlist)):
                ia.open(imlist[k]+'.model')
                #ia.putchunk(arr)
                ia.insert(infile=model, locate=[0,0,0,0])
                ia.done()
        #######
        restored=imagename+'.image'
        self.averimages(restored, restoreds)
        ###close major cycle loop
        #shutil.rmtree(imagename+'.image', True)
        #shutil.move(restored,  imagename+'.image')
        time2=time.time()
        ###Clean up
        #for k in range(len(imlist)):
        #   shutil.rmtree(imlist[k]+'.model', True)
        #  shutil.rmtree(imlist[k]+'.residual', True)
        # shutil.rmtree(imlist[k]+'.image', True)
        print 'Time to image is ', (time2-time1)/60.0, 'mins'
        c.stop_cluster()


        return
