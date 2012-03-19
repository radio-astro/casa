from parallel.pimager import pimager
import pdb
from simple_cluster import simple_cluster
import string
import commands
from taskinit import *
import shutil
import os

def pclean(vis=None,
           imagename=None,
           imsize=None,
           cell=None,
           phasecenter=None,
           stokes=None,
           mask=None,
           field=None,
           spw=None,
           ftmachine=None,
           alg=None,
           scales=None,
           majorcycles=None,
           niter=None,
           gain=None,
           threshold=None,
           weighting=None,
           robust=None,
           npixels=None,
           mode=None,
           start=None,
           nchan=None,
           width=None,
           restfreq=None,
           interactive=None,
           wprojplanes=None,
           facets=None,          
           overwrite=None,
           uvtaper=None, 
           outertaper=None,
           selectdata=None,
           timerange=None,
           uvrange=None, antenna=None, scan=None, observation=None
           ):
              

    """ Invert and deconvolve images with parallel engines

        Form images from visibilities. Handles continuum and spectral line 
        cubes using module pcont and pcube respectively.
    """
    #Python script    

####checking section
    if (mode=='cube') and (nchan <2) :
        raise ValueError, 'Not going to handle cube with 1 channel; use continuum'
  
    
    if((type(cell)==list) and (len(cell)==1)):
        cell.append(cell[0])
    elif ((type(cell)==str) or (type(cell)==int) or (type(cell)==float)):
        cell=[cell, cell]
    elif (type(cell) != list):
        raise TypeError, "parameter cell %s is not understood" % str(cell)
    cellx=qa.quantity(cell[0], 'arcsec')
    celly=qa.quantity(cell[1], 'arcsec')
    if(cellx['unit']==''):
            #string with no units given
        cellx['unit']='arcsec'
    if(celly['unit']==''):
        #string with no units given
        celly['unit']='arcsec'
    if((type(imsize)==list) and (len(imsize)==1)):
        imsize.append(imsize[0])
    elif(type(imsize)==int):
        imsize=[imsize, imsize]
    elif(type(imsize) != list):
        raise TypeError, "parameter imsize %s is not understood" % str(imsize)

    ###make sure units are consistent
    cubemode='channel'
    if (qa.quantity(start)['unit'].find('Hz') > 0):
        cubemode='frequency'
        if (qa.quantity(width)['unit'].find('Hz') < 0):
            raise TypeError, "start and width parameter should be in the same units" 
    if (qa.quantity(start)['unit'].find('m/s') > 0):
        cubemode='velocity'
        if (qa.quantity(width)['unit'].find('m/s') < 0):
            raise TypeError, "start and width parameter should be in the same units" 

    if(type(phasecenter)==str):
            ### blank means take field[0]
        if (phasecenter==''):
            fieldoo=field
            if(fieldoo==''):
                fieldoo='0'
            phasecenter=int(ms.msseltoindex(vis,field=fieldoo)['field'][0])
        else:
            tmppc=phasecenter
            try:
                if(len(ms.msseltoindex(vis, field=phasecenter)['field']) > 0):
                    tmppc = int(ms.msseltoindex(vis,
                                                field=phasecenter)['field'][0])
                    ##succesful must be string like '0' or 'NGC*'
            except Exception, instance:
                    ##failed must be a string 'J2000 18h00m00 10d00m00'
                tmppc = phasecenter
            phasecenter = tmppc
                

####checking done
    #I'll assume this machine is representative in memory
    arch=os.uname()[0].lower()
    totmem=8.0e9
    if(arch=='linux'):
        totmem=string.atof(commands.getoutput('cat /proc/meminfo | grep -i memtotal').split()[1])*1024.0
    elif(arch=='darwin'):
        totmem=string.atof(commands.getoutput('sysctl hw.memsize').split()[1])

    
    cluster=simple_cluster.getCluster()._cluster
    numproc=len(cluster.get_engines())
    numprocperhost=len(cluster.get_engines())/len(cluster.get_nodes()) if (len(cluster.get_nodes()) >0 ) else 1

    pim=pimager(cluster)
    #pdb.set_trace()

    if spw=='':
       spw='*'

    if imagename=='':
       imagename=vis

    if mode=='continuum':
        pim.pcont(msname=vis, imagename=imagename, 
              imsize=imsize, pixsize=[cellx, celly], 
              phasecenter=phasecenter, 
              field=field, spw=spw, 
              ftmachine=ftmachine, alg=alg, 
              hostnames='', numcpuperhost=-1, 
              majorcycles=majorcycles, niter=niter, gain=gain,
              threshold=threshold, weight=weighting, robust=robust, scales=scales,
              wprojplanes=wprojplanes,facets=facets,  stokes=stokes,
              contclean=(not overwrite), visinmem=False, maskimage=mask, interactive=interactive, numthreads=1)
    else:
        ##need to calculate chanchunk
        memperproc=totmem/float(numprocperhost)/2.0
        estmem=14.0*float(imsize[0]*imsize[1])*4
        chanchunk=int(memperproc/estmem)
        while((chanchunk*numproc) > nchan):
            chanchunk=chanchunk-1
        if(chanchunk <1):
            chanchunk=1
        pim.pcube(msname=vis, imagename=imagename, 
              imsize=imsize, pixsize=[cellx, celly], 
              phasecenter=phasecenter, 
              field=field, spw=spw, 
              ftmachine=ftmachine, alg=alg,
              hostnames='', numcpuperhost=-1, 
              majorcycles=majorcycles, niter=niter, gain=gain,
              threshold=threshold, weight=weighting, robust=robust, scales=scales,
              mode=cubemode, 
              wprojplanes=wprojplanes,facets=facets, 
              start=start, nchan=nchan, step=width, restfreq=restfreq,stokes=stokes, 
              imagetilevol=1000000, chanchunk=chanchunk, maskimage=mask,  
              contclean=(not overwrite), visinmem=False, numthreads=1)

#parallel_clean=pclean
    

