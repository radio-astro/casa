###############################################
## To plot stuff in weather tables, saved to MSname+.plotWX.png
## and estimate zenith opacity per spw, returned as a list named myTau
##
##
## J. Marvil 2.6.12
## revised 4.27.12 to add support for missing/empty weather table
## revised 11.05.12 to address CASA 4.0 changes
###############################################


import casac
from taskinit import *
import pylab as pl
from math import pi,floor
import os.path as osp


###############
## hides the extreme Y-axis ticks, helps stack plots close together without labels overlaping
def jm_clip_Yticks():
    xa=pl.gca()
    nlabels=0
    for label in xa.yaxis.get_ticklabels(): 
        nlabels+=1
    thislabel=0
    if nlabels>3:
        for label in xa.yaxis.get_ticklabels(): 
            if thislabel==0: label.set_alpha(0)            
            if thislabel==nlabels-1: label.set_alpha(0)
            thislabel+=1

##############
##  sets the position of the y-axis label to the right side of the plot, can also move up/down
def jm_set_Ylabel_pos(pos=(0.5,0.5)):
    ax=pl.gca();
    ax.yaxis.set_label_position('right')
    ax.yaxis.label.set_rotation(270)
    ax.yaxis.label.set_position(pos)


###############
## fixed y-ticks, from myMin to myMax
def jm_set_Ylim_ticks(myMin=-1,myMax=1):
    myYlocs=pl.linspace(round(myMin,1),round(myMax,1),5)
    myLocator = pl.FixedLocator(myYlocs)
    ax=pl.gca()
    ax.yaxis.set_major_locator( myLocator )
    pl.ylim(myMin,myMax)
    jm_clip_Yticks()

###############
## variable y-ticks, but not more than 1+ this argument
def jm_set_Yvar_ticks(myScale=4):
    xa=pl.gca()
    xa.yaxis.set_major_locator(pl.MaxNLocator(myScale))
    jm_clip_Yticks()

###############
## calculates K-band zenith opacity from temperature and dewpoint
def Tau_K_Calc(D,T,day, weights=(.5,.5)): 
    P = pl.exp(1.81+(17.27*D)/(D+237.3)) # water vapor partial pressure 
    h = 324.7*P/(T+273.15) # PWV in mm 
    tau_w = 3.8 + 0.23*h + 0.065*h**2 # tau from weather, in %, at 22GHz
    if day > 199: day = day - 365. 
    m = day + 165. # modified day of the year 
    tau_d = 22.1 - 0.178*m + 0.00044*m**2 # tau from seaonal model, in % 
    tau_k = weights[0]*tau_w + weights[1]*tau_d # the average, with equal weights (as in the AIPS default) 
    return tau_k, h

################
## calculates elevation of the sun
def jm_sunEL(mytime):
    me.doframe(me.observatory('VLA'))
    me.doframe(me.epoch('utc',mytime))
    mysun=me.measure(me.direction('SUN'),'AZELGEO')
    return mysun['m1']['value']

################
## gets and plots data from the weather table of the given MS
def plotweather(vis='', seasonal_weight=0.5, doPlot=True, plotName = ''):
    myMS=vis
    if plotName == '': plotName = myMS+'.plotweather.png'

    # check for weather table

    if osp.isdir(myMS+'/WEATHER'):
    
        try:
            tb.open(myMS+'/WEATHER')
            firstTime = tb.getcol('TIME')[0]
            tb.close()
            WEATHER_table_exists = True

        except:
            print 'could not open weather table, using seasonal model only and turning off plots'
            WEATHER_table_exists = False
            doPlot=False
            seasonal_weight = 1.0
    else:
        print 'could not find a weather table, using seasonal model only and turning off plots'
        WEATHER_table_exists = False
        doPlot=False
        seasonal_weight = 1.0



    ##retrieve center frequency for each sub-band
    tb.open(myMS+'/SPECTRAL_WINDOW')
    spwFreqs=tb.getcol('REF_FREQUENCY') * 1e-9   
    tb.close()

    ##retrieve stuff from weather table, if exists
    
    if WEATHER_table_exists:
        tb.open(myMS+'/WEATHER')
        mytime=tb.getcol('TIME')
        mytemp=tb.getcol('TEMPERATURE') - 273.15
        mydew=tb.getcol('DEW_POINT') - 273.15
        mywinds=tb.getcol('WIND_SPEED')
        mywindd=tb.getcol('WIND_DIRECTION')*(180.0/pi) - 90
        mypres=tb.getcol('PRESSURE')
        myhum=tb.getcol('REL_HUMIDITY')
        tb.close()

    else:
        ms.open(myMS)
        mytime_range = ms.range(["time"])
        mytime = [mytime_range['time'][0]]


    ##calculate the elevation of the sun
    sunEL=[]
    for time in mytime:
        t1= qa.quantity(time,'s')
        myday=qa.convert(t1,'d')
        sunEL1=jm_sunEL(myday)        
        sunEL.append(sunEL1)

    ##convert time to a string of date/time
    myTimestr = []
    myTimestr2=[]
    for time in mytime:
        q1=qa.quantity(time,'s')
        time1=qa.time(q1,form='ymd')[0]
        time2=qa.time(q1,form='local')[0]
        myTimestr.append(time1)
        myTimestr2.append(time2)
 
    ##convert time to a decimal
    numtime=pl.datestr2num(myTimestr)    

    ##### calculate opacity as in EVLA memo 143
    thisday= 30*(float(myTimestr[0][5:7])-1)+float(myTimestr[0][8:10]) 
    thisday=thisday + 5 * (thisday / 365.)


    if WEATHER_table_exists:
        # get 22 GHz zenith opacity and pwv estimate from weatherstation (myPWV)
        myTauZ, myPWV1 = Tau_K_Calc(mydew,mytemp,thisday)
        myTauZ1, myPWV = Tau_K_Calc(mydew,mytemp,thisday, weights=(0,1.0))
        myTauZ2, myPWV = Tau_K_Calc(mydew,mytemp,thisday, weights=(1.0,0))

        # estimate pwv from seasonal model zenith opacity    
        myPWV2 = -1.71 + 1.3647*myTauZ1
        myPWV = (1-seasonal_weight)*myPWV1 + seasonal_weight*myPWV2
 
    else:
        day = thisday*1.0
        if day > 199: day = day - 365. 
        m = day + 165. # modified day of the year 
        myTauZ = 22.1 - 0.178*m + 0.00044*m**2 # tau from seaonal model, in % 
        myPWV = -1.71 + 1.3647*myTauZ
        myPWV1, myPWV2 = myPWV, myPWV
        myTauZ1, myTauZ2 = myTauZ, myTauZ
 
    tmp = qa.quantity(270.0,'K')
    pre = qa.quantity(790.0,'mbar')
    alt = qa.quantity(2125,'m')
    h0 = qa.quantity(2.0,'km')
    wvl = qa.quantity(-5.6, 'K/km')
    mxA = qa.quantity(48,'km')
    dpr = qa.quantity(10.0,'mbar')
    dpm = 1.2
    att = 1
    nb = 1

    fC=qa.quantity(25.0,'GHz')
    fW=qa.quantity(50.,'GHz')
    fR=qa.quantity(0.25,'GHz')

    at=casac.atmosphere()
    hum=20.0

    myatm=at.initAtmProfile(alt,tmp,pre,mxA,hum,wvl,dpr,dpm,h0,att)

    at.initSpectralWindow(nb,fC,fW,fR)
    sg=at.getSpectralWindow()
    mysg = sg['value']

    nstep = 20
    pwv = []  
    opac = pl.zeros((len(mysg),nstep))
    
    for i in range(nstep):
        hum = 20.0*(i+1)
        myatm = at.initAtmProfile(alt,tmp,pre,mxA,hum,wvl,dpr,dpm,h0,att)
        w=at.getGroundWH2O()
        pwv.append(w['value'][0])
        at.initSpectralWindow(nb,fC,fW,fR)
        at.setUserWH2O(w)
        sg=at.getSpectralWindow()
        mysg = sg['value']
        sdry=at.getDryOpacitySpec()
        swet=at.getWetOpacitySpec()
        sd=sdry[1]
        sw=swet[1]['value']
        stot = pl.array(sd)+pl.array(sw)
        opac[:,i]=stot

    pwv_coef=pl.zeros((len(mysg),2))
    for i in range(len(mysg)):
        myfit=pl.polyfit(pwv,opac[i,:],1)
        pwv_coef[i,:]=myfit
    
    freqs=pl.array(mysg)/1e9
    coef0=pwv_coef[:,1]/1e-3
    coef1=pwv_coef[:,0]/1e-3


    #interpolate between nearest table entries for each spw center frequency
    meanTau=[]

    for i in range(len(spwFreqs)):
        mysearch=(pl.array(freqs)-spwFreqs[i])**2
        hits=pl.find(mysearch == min(mysearch))
        if len(hits) > 1: hits=hits[0]
        tau_interp = (pl.array(coef0[hits-2:hits+2])+pl.array(coef1[hits-2:hits+2])*pl.mean(myPWV)) * 1e-1  #percent
        tau_F = pl.interp(spwFreqs[i],freqs[hits-2:hits+2],tau_interp)
        meanTau.append(pl.mean(tau_F*.01))  #nepers


    tau_allF = (pl.array(coef0) + pl.array(coef1)*pl.mean(myPWV)) * 1e-1  #percent
    tau_allF1 = (pl.array(coef0) + pl.array(coef1)*pl.mean(myPWV1)) *1e-1  
    tau_allF2 = (pl.array(coef0) + pl.array(coef1)*pl.mean(myPWV2)) *1e-1  

    casalog.post('SPW : Frequency (GHz) : Zenith opacity (nepers)')
    for i in range(len(meanTau)):
        myStr = str(i).rjust(3) + '  :  ' 
        myStr2 = '%.3f'%(spwFreqs[i])
        myStr += myStr2.rjust(7) + '  :  ' +str(round(meanTau[i], 3))
        casalog.post(myStr)



    ##make the plots

    if doPlot==False:
        return meanTau

    pl.ioff()
    myColor2='#A6A6A6'
    myColorW='#92B5F2'
    myColor1='#4D4DFF'  
    myOrangeColor='#FF6600'
    myYellowColor='#FFCC00'
    myWeirdColor='#006666'
    myLightBrown='#996633'
    myDarkGreay='#333333'

    thisfig=pl.figure(1)
    thisfig.clf()
    thisfig.set_size_inches(8.5,10)

    Xrange=numtime[-1]-numtime[0]
    Yrange=max(mywinds)-min(mywinds)
    Xtextoffset=-Xrange*.01
    Ytextoffset=-Yrange*.08
    Xplotpad=Xrange*.03
    Yplotpad=Yrange*.03

    sp1=thisfig.add_axes([.13,.8,.8,.15])
    pl.ylabel('solar el')
    nsuns=30
    myj=pl.array(pl.linspace(0,len(sunEL)-1,nsuns),dtype='int')
    for i in myj:
        if sunEL[i]<0: pl.plot([numtime[i],numtime[i]],[(180/pi)*sunEL[i],(180/pi)*sunEL[i]],'kH')
        else: pl.plot([numtime[i],numtime[i]],[(180/pi)*sunEL[i],(180/pi)*sunEL[i]],'H',color=myYellowColor)
    pl.plot([numtime[0],numtime[-1]],[0,0],'-',color='brown')
    xa=pl.gca(); xa.set_xticklabels('')
    jm_set_Ylim_ticks(myMin=-90,myMax=90)
    jm_set_Ylabel_pos(pos=(0,.5))
    pl.title('Weather Summary for '+myMS)
    pl.xlim(numtime[0]-Xplotpad,numtime[-1]+Xplotpad)
    xa.set_xticks(pl.linspace(min(numtime),max(numtime),3))

    sp2=thisfig.add_axes([.13,.65,.8,.15])
    pl.ylabel('wind (m/s)')
    nwind=60
    myj=pl.array(pl.linspace(0,len(mywinds)-1,nwind),dtype='int')
    for i in myj:
        pl.text(numtime[i]+Xtextoffset,Ytextoffset+mywinds[i],'-->',rotation=mywindd[i], alpha=1,color='purple',fontsize=12) 

    pl.plot(numtime, .3+mywinds,'.', color='black', ms=2, alpha=0)
    jm_set_Ylabel_pos(pos=(0,.5))
    jm_set_Yvar_ticks(5)
    xa=pl.gca(); xa.set_xticklabels('')
    pl.xlim(numtime[0]-Xplotpad,numtime[-1]+Xplotpad)
    pl.ylim(min(mywinds)-Yplotpad,max(mywinds)+Yplotpad)
    xa.set_xticks(pl.linspace(min(numtime),max(numtime),3))

    sp4=thisfig.add_axes([.13,.5,.8,.15])
    pl.plot(numtime, mytemp,'-', color=myOrangeColor,lw=2)      
    pl.plot(numtime, mydew,'-', color=myWeirdColor,lw=2)      
    pl.ylabel('temp,dew')
    jm_set_Ylabel_pos(pos=(0, .5))
    xa=pl.gca(); xa.set_xticklabels('')
    jm_set_Yvar_ticks(5)
    pl.xlim(numtime[0]-Xplotpad,numtime[-1]+Xplotpad)
    xa.set_xticks(pl.linspace(min(numtime),max(numtime),3))

    sp7=thisfig.add_axes([.13,.35,.8,.15])
    pl.ylabel('PWV (mm)')
    pl.plot(numtime, myPWV2, color=myColor2, lw=2, label='seasonal model')
    pl.plot(numtime, myPWV1, color=myColor1, lw=2, label='weather station')
    pl.plot(numtime, myPWV, color=myColorW,lw=2, label='weighted')

    thismin=min([min(myPWV),min(myPWV1),min(myPWV2)])
    thismax=max([max(myPWV),max(myPWV1),max(myPWV2)])
    pl.ylim(.8*thismin,1.2*thismax)
    jm_set_Ylabel_pos(pos=(0,.5))
    jm_set_Yvar_ticks(5)
    xa=pl.gca(); xa.set_xticklabels('')
    pl.xlim(numtime[0]-Xplotpad,numtime[-1]+Xplotpad)

    middletimei=int(floor(len(myTimestr)/2.))
    middletimes=str(myTimestr[middletimei])[11:]
    endtimes=myTimestr[-1][11:]                
    ax=pl.gca()
    axt=ax.get_xticks()
    ax.set_xticks(pl.linspace(min(numtime),max(numtime),3))
    ax.set_xticklabels([myTimestr[0],middletimes,endtimes ])

    sp8=thisfig.add_axes([.13,.1,.8,.2])
    pl.plot(freqs,.01*tau_allF2,'-', color=myColor2, lw=2, label='seasonal model')
    pl.plot(freqs,.01*tau_allF1,'-', color=myColor1, lw=2, label='weather station')
    pl.plot(freqs,.01*tau_allF,'-', color=myColorW, lw=2,label='weighted')

  
    sp8.legend(loc=2, borderaxespad=0)
    pl.ylabel('Tau_Z (nepers)')
    pl.xlabel('Frequency (GHz)')
    pl.ylim(0,.25)
    jm_set_Yvar_ticks(6)
    jm_set_Ylabel_pos(pos=(0,.5))
    pl.savefig( plotName, dpi=150)
    pl.close()

    casalog.post('wrote weather figure: '+plotName)
    return meanTau


