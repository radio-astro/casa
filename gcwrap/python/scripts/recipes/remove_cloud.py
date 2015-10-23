"""
Fit a constant tau component to the 4 ALMA WVR channels, 
then remove this from the 4 temperature measurements tsrc(0-3)

Based on rem_cloud.py by B. Dent version 12 Aug 2015, see ALMA CSV-3189
"""

from taskinit import *

import numpy as np
import pylab as pl
from matplotlib import pyplot
from scipy import stats
import scipy.optimize as opt
import os
import math
import time

def rmc_func1(x,a,b,c):    
    return a*(x-b)**2+c

def rmc_weighted_avg_and_std(values, weights):
    """
    Return the weighted average and standard deviation.

    values, weights -- Numpy ndarrays with the same shape.
    """
    average = np.average(values, weights=weights)
    variance = np.average((values-average)**2, weights=weights)
    return (average, math.sqrt(variance))

def rmc_approxCalc(tsrc0, tsrc1, tsrc2, tsrc3,
                   m_el, Tamb, verb=False):
    
    m_el=m_el/57.295   # convert to radians
    mean_pwt=0.0; raw_mean_pwt=0.0

    # correct for coupling to the sky, assuming T_loss=275K, eta_c=0.98
    eta_c=0.98
    T_loss=275.0

    tsrc0=(tsrc0-(1.0-eta_c)*T_loss)/eta_c
    tsrc1=(tsrc1-(1.0-eta_c)*T_loss)/eta_c
    tsrc2=(tsrc2-(1.0-eta_c)*T_loss)/eta_c
    tsrc3=(tsrc3-(1.0-eta_c)*T_loss)/eta_c

    #    m_az, m_el, m_ts= mc.actualAzEl()

    pw=[0.0,0.0,0.0,0.0];     pw_noc=[0.0,0.0,0.0,0.0]
    site="AOS"
    
    # set up fitting constants depending on the site:
    if site == "OSF":   # constants:
       tau0=[0.024,0.02,0.009,0.01]
       r=[1.205,0.402,0.177,0.116]
       # approximate physical temp of atmosphere:
       Tphys=267.0

    elif site == "AOS":  # constants:
       tau0=[0.027,0.02,0.010,0.01]
       r=[1.193,0.399,0.176,0.115]
       # approximate physical temp of atmosphere ,based on ambient temperature, Tamb, in centrigrade:
       Tphys=270.0 +Tamb

       if Tphys<tsrc0:
           Tphys=tsrc0+1.0
           if verb:
               casalog.post('  fixed physical temperature to be tsrc0', 'INFO')

    tsrcn=np.zeros(4)
    teln=np.zeros(4)

    tel=[0.0,0.0,0.0,0.0]
    tz=[0.0,0.0,0.0,0.0]
    wt=[0.0,0.0,0.0,0.0]

    # calculate transmissions:
    tel[3]=(1.0-tsrc3/Tphys)
    tel[2]=(1.0-tsrc2/Tphys)
    tel[1]=(1.0-tsrc1/Tphys)
    tel[0]=(1.0-tsrc0/Tphys)

    for i in range(4):
      if tel[i]<0.02:
         tel[i]=0.02
      wt[i]=1.0-(abs(tel[i]-0.5)/0.49)**2.0    # weights

    if verb:
        casalog.post( '  weights 0-3: '+str(wt), 'INFO')
        casalog.post( '  tsrc 0-3:    '+str(tsrc0)+', '+str(tsrc1)+', '+str(tsrc2)+', '+str(tsrc3), 'INFO')

    use=1
    for i in range(4):
           pw[i]=-(pl.log(tel[i])+tau0[i])/r[i]

    rat31_1=pw[3]/pw[1]
    pwm=np.mean(pw)

    if pwm>5.0: wt[0]=0.0

    if pwm>0.5:   # only look for a wet cloud component if pwv>0.5mm (bit arbitrary cutof but probably ok)
        pwt=np.zeros(4)

        # now set increment factor for tauc depending on dekta T
        dt=pw[3]-pw[2]

        tauc_r=np.zeros(5)
        std_pwt_r=np.zeros(5)
        # find std for tauc=0.0000
        for i1 in range(4):
                pwt[i1]=-(pl.log(tel[i1])+tau0[i1])/r[i1]

        mean_pwt,std_pwt_0=rmc_weighted_avg_and_std(pwt, wt)   # get std of 4 v

        #  tauc_r are the 5 ranges used to estimate the max value to be used in the array for fitting
        tauc_r[0]=0.05
        tauc_r[1]=0.1
        tauc_r[2]=0.5
        tauc_r[3]=1.0
        tauc_r[4]=3.0
    
        it_stop=0
        for it in range(5):
            for i1 in range(4):
                pwt[i1]=-((pl.log(tel[i1])+tau0[i1])+tauc_r[it])/r[i1]

            mean_pwt,std_pwt_r[it]=rmc_weighted_avg_and_std(pwt, wt)   # get std of 4 values, using weights (should weight down channels 0,1 a lot)
            
        if min(std_pwt_r)==std_pwt_r[4]:
            if verb:
                casalog.post('  tauc is too large - setting to max', 'INFO')
            tau_constant=3.0
        else:
            for itr in range(4,-1,-1):
                if std_pwt_r[itr]>std_pwt_0: it_stop=itr    # this is used to find the first value of it where the std is increaseing again

            it_stop=it_stop
            # now create an array of std's, up to it_stop (to avoid edges)
            tau_arr=np.zeros(300)
            std_arr=np.zeros(300)
            for it in range(300):
                tau_arr[it]=tauc_r[it_stop]*float(it)/300.0
                for i1 in range(4):
                    pwt[i1]=-((pl.log(tel[i1])+tau0[i1])+tau_arr[it])/r[i1]
                    if pwt[i1]<0.0:
                        iloop=False
                mean_pwt,std_arr[it]=rmc_weighted_avg_and_std(pwt, wt)

            # we now have the array. need to do a fit.
            # first check that 0'th element is not the smallest;
            # if it is, then no need to fit
            if std_arr[0]>(np.mean(std_arr[1:3])):
                x0=np.array([0.0,0.0,0.0])    # first guess
                # now find the minimum:
                mm=min(std_arr)
                nmin=[ii for ii,jj in enumerate(std_arr) if jj==mm][0]
                a2=tau_arr[nmin]
                if verb:
                    casalog.post('  First guess tauc, nmin: '+str(a2)+', '+str(nmin), 'INFO')
                # now do a fit +-50 elements around this point
                tau_arr_subset=[]; std_arr_subset=[]
                for isubset in range(-50,50):
                    i_arr=nmin+isubset
                    if i_arr>-1 and i_arr<300:
                        tau_arr_subset.append(tau_arr[i_arr])
                        std_arr_subset.append(std_arr[i_arr])
                try:
                    a1,a2,a3=opt.curve_fit(rmc_func1,tau_arr_subset,std_arr_subset,x0)[0]
                except:
                    a2=tau_arr[nmin]
                    casalog.post('  Fitting failed, using approximation (minimum)', 'INFO')
                   

                tau_constant=a2
            else:
                tau_constant=0.0
            
        # re-estimate pwv, after removing additional tau_constant component.
        # (could add extra factor 1/(1-tau) in measured line abs. 
        # because it's absorbed by the continuum atmopheric abs ... 
        # although maybe not
        # if they are colocated - this needs some radiative transfer...)

        for i in range(4):
           pw_noc[i]=(-(pl.log(tel[i])+tau0[i]+tau_constant)/r[i])  # *(1./(1-tau_constant))

        # reverse-calculate the effective tsrcn(0-3) based on the new pwvs:
        for i in range(4):
            teln[i]=math.exp(-(pw_noc[i]*r[i]+tau0[i]))
            tsrcn[i]=Tphys*(1.0-teln[i])

        if verb:
            casalog.post('  pw 0-3    : '+str(pw), 'INFO')
            casalog.post('  pw_noc 0-3: '+str(pw_noc), 'INFO')
            casalog.post('  tsrc 0-3  : '+str(tsrc0)+', '+str(tsrc1)+', '+str(tsrc2)+', '+str(tsrc3), 'INFO')
            casalog.post('  tsrcn 0-3 : '+str(tsrcn[0])+', '+str(tsrcn[1])+', '+str(tsrcn[2])+', '+str(tsrcn[3]), 'INFO')
 
        #  estimate weighted mean pwv, with and without cloud component:
        # first estimate
        ws=0.0
        for i in range(4):
            ws=ws+pw[i]*wt[i]
        pwv_los=ws/np.sum(wt)
        pwv_z=pwv_los*math.sin(m_el)

        # now remove moisture component
        ws=0.0
        for i in range(4):
            ws=ws+pw_noc[i]*wt[i]
        pwv_los_noc=ws/np.sum(wt)

        pwv_z_noc=pwv_los_noc*math.sin(m_el)

        # for i in range(4):
        #    pw[i]=pw[i]*math.sin(m_el)
    else: # pwv <= 0.5  
        tau_constant=0.0
        ws=0.0
        for i in range(4):
            ws=ws+pw[i]*wt[i]
        pwv_los=ws/np.sum(wt)
        pwv_z=pwv_los*math.sin(m_el)
        pwv_z_noc=pwv_z
        
    return pwv_z,pwv_z_noc,tau_constant,tsrcn


def remove_cloud(vis=None, correct_ms=False, offsetsfile='', verbose=False, doplot=False):
    """
    Parameters:
       vis - MS with WVR data included (imported ALMA data)
       correct_ms - do the corrections to the wvr data in the MS (default False)
       offsetsfile - store processing results (Temp offsets) in this filename (default '' = don't store) 
       verbose - control terminal output (default False) 
       doplot - generate diagnostic plots in subdirectory vis+'_remove_cloud_plots' (default False)
    Example:
       remove_cloud(vis='uid___A002_X....', True, 'myoffsets.txt')
    """

    casalog.post('*** Starting remove_cloud ***', 'INFO')

    if vis==None or type(vis)!=str:
        casalog.post('Invalid parameter vis.', 'SEVERE')
        return False

    if correct_ms:
        mst = mstool()
        mst.open(vis)
        myref = mst.asdmref()
        mst.close()
        if not myref=='':
            casalog.post('MS '+vis
                         +' was imported with option lazy=True, i.e. its DATA column is read-only.'
                         +'\nCannot proceed.', 'SEVERE')
            return False

    if not type(offsetsfile)==str: 
        casalog.post('Invalid parameter offsetsfile.', 'SEVERE')
        return False

    if offsetsfile!='' and  os.path.exists(offsetsfile):
        casalog.post('File '+offsetsfile+' exits.', 'SEVERE')
        return False

    if correct_ms:   # either correct or dont correct ms file - need to set in advance
        casalog.post(' Will apply corrections to WVR data. MS will be modified.', 'INFO')

    plotdir=''
    if doplot:
        plotdir = vis+'_remove_cloud_plots'
        casalog.post(' Will (re-)create directory '+plotdir+' to store plots.', 'INFO')
        os.system('rm -rf '+plotdir+'; mkdir '+plotdir)

    mytb = tbtool()
    
    # get basic info
        
    mytb.open(vis+'/ANTENNA')
    nant=mytb.nrows()
    antnames=mytb.getcol('NAME')
    mytb.close()

    mytb.open(vis+'/PROCESSOR')
    nprocs=mytb.nrows()
    procs=mytb.getcol('SUB_TYPE')
    mytb.close()

    proc_id=-1
    for ipp in range(nprocs):
        if procs[ipp]=='ALMA_RADIOMETER':
            proc_id=ipp
            break
    if proc_id<0:
        casalog.post('MS contains no WVR data.', 'SEVERE')
        return False

    mytb.open(vis+'/WEATHER')
    Tamb=np.median(mytb.getcol('TEMPERATURE'))-273.1
    mytb.close()

    mytb.open(vis+'/POINTING')
    if(mytb.nrows()==0):
        mytb.close()
        casalog.post('Empty POINTING table. Please run on MS with intact POINTING table.', 'SEVERE')
        return False
    m_el=360.0*(np.median(mytb.getcol('DIRECTION')[1]))/(2.0*3.14)
    mytb.close()

    tbo = None
    dooffsets=False
    if offsetsfile!='':
        os.system('echo "0 0 0 0 0" > mydummy.txt')
        ok = mytb.fromascii(offsetsfile, sep=" ", columnnames=['ANTENNA','OFFSETS'], datatypes=['S', 'R4'], 
                           asciifile='mydummy.txt')
        mytb.close()
        if not ok:
            casalog.post('Error creating table '+offsetsfile, 'SEVERE')
            return False
        tbo = tbtool()
        tbo.open(offsetsfile, nomodify=False)
        tbo.removerows([0])
        os.system('rm mydummy.txt')
        dooffsets=True

    if correct_ms:
	mytb.open(vis,nomodify=False)
    else:
	mytb.open(vis,nomodify=True)    # for testing, dont modify asdm


    tsrcn=np.zeros(4)
    # values for each ant
    pwv_ant=np.zeros(nant)
    pwv_std_ant=np.zeros(nant)
    tauc_ant=np.zeros(nant)
    tauc_std_ant=np.zeros(nant)

    for iant in range(nant):
        casalog.post('- Processing antenna#'+str(iant)+' ('+antnames[iant]+') ...', 'INFO')
        tb1=mytb.query("ANTENNA1==%d && PROCESSOR_ID==%d" % (iant,proc_id), sortlist='TIME')
        temp=tb1.getcol('DATA')
        nsamples=len(temp[0][0])
        pwvna=np.zeros(nsamples)
        pwvn_noca=np.zeros(nsamples)
        tau_con=np.zeros(nsamples)

        offsets=None
        if dooffsets:
            offsets=np.zeros((4,nsamples))

        for isam in range(nsamples):
            tsrc=[(temp[0][0][isam]).real, (temp[0][1][isam]).real, (temp[0][2][isam]).real, (temp[0][3][isam]).real]

            # got temps, now convert to pwv

            pwvna[isam],pwvn_noca[isam],tau_con[isam],tsrcn=rmc_approxCalc(tsrc[0], tsrc[1], tsrc[2], tsrc[3], m_el, Tamb, 
                                                                           verbose)
            if dooffsets:
                for it in range(4):
                    offsets[it][isam]=tsrc[it] - tsrcn[it] # = old WVR value minus newly calculated one

            if correct_ms:
                # put the new tsrcn values for this sample & antenna into temp[0][0-3][isam]
                for it in range(4):
                    temp[0][it][isam]=tsrcn[it]

        if dooffsets:
            casalog.post('   Writing the offset values for antenna '+str(iant)+' to '+offsetsfile, 'INFO')
            startrow=tbo.nrows()
            tbo.addrows(nsamples)
            tbo.putcol('OFFSETS', offsets, startrow)
            ants = np.empty(nsamples)
            ants.fill(iant)
            tbo.putcol('ANTENNA', ants, startrow)

        if correct_ms:
            casalog.post('   Writing new values for antenna '+str(iant)+' to Main table of '+vis, 'INFO')
            tb1.putcol('DATA',temp)
        
        tb1.close()

        # now outputs to the screen the medians of the samples
        # this just removes crazy numbers, for rms estimate
        pwvna_m=np.median(pwvna)
        pwvn_noca_m=np.median(pwvn_noca)
        tau_constant_m=np.median(tau_con)
        for i in range(len(pwvna)):
            if abs(pwvna[i]-pwvna_m)>1.0:
                pwvna[i]=np.nan
            if abs(pwvn_noca[i]-pwvn_noca_m)>1.0:
                pwvn_noca[i]=np.nan
            if abs(tau_con[i]-tau_constant_m)>0.2:
                tau_con[i]=np.nan

        casalog.post('   Result for '+antnames[iant]+':', 'INFO')
        casalog.post('      PWV    : before, after '+str(stats.nanmedian(pwvna))+', '+str(stats.nanmedian(pwvn_noca)), 'INFO')
        casalog.post('      PWV rms: before, after '+str(stats.nanstd(pwvna))+','+str(stats.nanstd(pwvn_noca)), 'INFO')
        casalog.post('      tau_constant '+str(tau_constant_m)+'  rms '+str(stats.nanstd(tau_con)), 'INFO')

        pwv_ant[iant]=stats.nanmedian(pwvn_noca)
        pwv_std_ant[iant]=stats.nanstd(pwvn_noca)
        tauc_ant[iant]=tau_constant_m
        tauc_std_ant[iant]=stats.nanstd(tau_con)

        if doplot:
            # plot before and after results
            tau_con_scaled=10*tau_con
            pl.ion()
            pyplot.clf()
            pyplot.plot(pwvna, color='blue')
            pyplot.plot(pwvn_noca, color='red')
            pyplot.plot(tau_con_scaled, color='green')
            pyplot.title(antnames[iant]+'  ('+str(iant)+')  '+vis)
            pyplot.xlabel('Measurement Number')
            pyplot.ylabel('blue=PWV_before (mm), red=PWV_after (mm), green=10*tau_con')
            pyplot.draw()
            plotfil=plotdir+'/'+antnames[iant]+'.png'
            pyplot.savefig(plotfil)
            casalog.post('    Generated '+plotfil, 'INFO')
    #end for

    mytb.close()

    if dooffsets:
        pwv_noca_all=stats.nanmedian(pwv_ant)
        pwv_std_all=stats.nanmedian(pwv_std_ant)
        tauc_all=stats.nanmedian(tauc_ant)
        tauc_std_all=stats.nanmedian(tauc_std_ant)

        tbo.putkeyword('REFMS', vis)
        tbo.putkeyword('CREATION_UTC', time.asctime(time.gmtime()))
        tbo.putkeyword('PWV', pwv_noca_all)
        tbo.putkeyword('PWV_STDEV', pwv_std_all)
        tbo.putkeyword('TAUC', tauc_all)
        tbo.putkeyword('TAUC_STDEV', tauc_std_all)

        tbo.close()
        
        casalog.post(' Saved remove_cloud results to '+offsetsfile, 'INFO')


    return True

  
