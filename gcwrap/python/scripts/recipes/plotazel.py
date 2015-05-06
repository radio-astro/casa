from taskinit import *
from getazel import *
import pylab as pl
import os
import datetime
from matplotlib.dates import DateFormatter
# plotting azimuth and elevation of source(s)

def plotsrcazel(planet='', srcfile='', date='', obs='ALMA', plotsun=False, plottype='', saveplot='', elimit='3.0deg'):
    '''
    plot Azimuth/elevation track of a source (or list of sources)
    on given date. For solar system objects known to Measures(this excludes minor planets, Jovian moons, etc), 
    name is sufficient but for other sources
    RAs and Decs are required. 
     (requires getazel.py)
    
    Input:
    planet - a planet (optional)
    srcfile - (optional) An ascii file with source_name coordinates (J2000) seperated by
              a white space. A line with '#' will be skipped.
              The solar system can also be include without coodinates. 

              0423-013 4h23m0s -1d20m0s
              #3c273 12h29m0s 2d3m0s
              3c279 12h56m0s -5d47m0s
              Mars
             
    date - 'YYYY/MM/DD' if not specified it will be prompted to enter later
    obs  - observatory name (ALMA,EVLA,etc)
    plotsun - True will plot the Sun also
    plottype - Az, El, or both (if left blank it will ask to enter later)
    saveplot - save to the plot to ps file with the name specified in this paramter
    elimit - draw horizontal line to indicate elevation limit. 

    Examples:
        plotsrcazel('Uranus')  # enter date, plottype when prompted
        plotsrcazel(planet='Uranus', date='2012/06/01', plottype='both') 
        plotsrcazel(planet='Uranus', srcfile='calandtarget.txt')    #plot Uranus and sources defined
                                                                    # in calandtarget.txt

    version 2015.05.06  TT -Minor fixes to be able run on current casa 
    version 2012.04.20  TT
    '''


    #def plotazel(date=''):
    # date = '2010/02/01'
    inpd =""
    if date=='':
      inpd=raw_input('date? YYYY/MM/DD or hit return to use today\'s date:')
      if inpd == "":
	date=qa.time('today', form=['ymd','no_time'])[0]
      else:    
	date=inpd
    else:
      date=date
    print "Use date:", date

    intz=raw_input('Show in UTC, CLT,or LST? ')
    tz='UTC' 
    if intz!='':
      check_intz = [intz.upper()==tl for tl in ['UTC','CLT','LST']]
      if not any(check_intz):
         raise Exception, "Input error: should be 'UTC','CLT',or 'LST'"
      tz=intz.upper()

    # read source list from a file 
    if srcfile!="":
       if not os.path.exists(srcfile):
          raise IOError, "%s does not exist!" % srcfile
       readfromfile = True
    else:
       readfromfile = False
    insrc = True 
    # plot planets
    knownsrcs=['SUN', 'MOON', 'MERCURY','VENUS','MARS','JUPITER','SATURN','URANUS','NEPTUNE','PLUTO']
    plotplanets=False
    if planet!='':
      for ksrc in knownsrcs:
        if planet.upper() == ksrc:
          plotplanets= True
    #plotplanets = True

    if plottype=='':
        # default plottype
        #plottype='az'
        plottype='both'

        inptype=raw_input('plot type? (el, az, or both):')

        if inptype=='el' or inptype=='az':
	    plottype=inptype


    #observatory
    # should work any observatory name in 
    # me.obslist()
    #obs='ALMA'
    #obs='VLA'
    #obs='NRO'
    if obs.upper()=='ALMA':
        elimit_hard=3.0
    else:
        elimit_hard=0.0
    #user specified EL limit (draws horizontal line)        
    elimitv=qa.quantity(elimit)['value']

    # source list from a file or use hardcoded one in here
    # or can be read from source in casa database
    # (but maybe they are mostly northern hemisphere soruces???)

    # read calibrator list as a file
    # format 
    # name ra dec 
    # 0522-364 5h23m0s -36d28m0s
    #
    #
    srclist={}
    srclist['name']=[]
    srclist['ra']=[]
    srclist['dec']=[]

    if readfromfile:
	#f = open('calibrators.list','r')
	f = open(srcfile,'r')
	while f:
	    ln = f.readline()
	    if len(ln) ==0:
		break
	    if ln.rfind('#')==0:
	       # skip comment 
	       pass
	    else:
	       #(name,ra,dec) = ln.split()
               src = ln.split()
               if len(src)==3:
	         srclist['name'].append(src[0])
	         srclist['ra'].append(src[1])
	         srclist['dec'].append(src[2])
               elif len(src)==1:
                 # this part still does not work (need to modify getazel) 
                 knownEphemSrcs=me.listcodes(me.direction())['extra'].tolist()
                 if any(i == src[0].upper() for i in knownEphemSrcs):
                   srclist['name'].append(src[0])
                   srclist['ra'].append(src[0])
                   srclist['dec'].append(' ')
    if insrc:
       inpos = raw_input("Enter name xxhxxmxx.xs +/-xxdxxmxx.xs: (return to skip)")
       if inpos=='':
          if planet=='' and srcfile=='' and (not plotsun):
             raise Exception, "No source is specified!!!"
	  pass
       else:
	  inpos.replace("\'","")
	  srcpos = inpos.split()
	  srclist['name'].append(srcpos[0])
	  srclist['ra'].append(srcpos[1])
	  srclist['dec'].append(srcpos[2])

    if plotsun:
       # add sun
       srclist['name'].append('SUN')
       srclist['ra'].append('SUN')
       srclist['dec'].append('')

    if plotplanets:
       srclist['name'].append(planet)
       srclist['ra'].append(planet)
       srclist['dec'].append('')
       
    #
    last = False
    fig = pl.figure()
    fig.clf()
    tmfmt = DateFormatter('%H')
    seq = range(len(srclist['name']))
    print "Number of sources to be plotted:", len(srclist['name'])
    for i in seq:
	if i == seq[-1] :
	   last = True 
	srcname=srclist['name'][i]
	srcradec=srclist['ra'][i]+' '+srclist['dec'][i]
	
	srcCoord='J2000 '+srcradec
        casalog.post('Plotting for '+srcname)
	(az,el,tm,tutc) = getazel(obs,srcname,srcCoord,date,tz)

	eld = el*180./pi
	azd = az*180./pi
        #elevation limit
        import numpy
        for iel in range(len(eld)):
          if eld[iel] < elimit_hard:
            eld[iel]=-1.0
            azd[iel]=360.

	#print "max(eld)=",max(eld)
	#print "max(azd)=",max(azd)

	tx = tm
        #for t in tx:
        #   tq = qa.time(str(t)+'d',form='ymd')
        #   tqymd=tq.split('/')
        #   tqhms=tqymd[-1].split(':')
        #   dt=datetime.datetime(int(tqymd[0]),int(tqymd[1]),int(tqymd[2]),int(tqhms[0]),int(tqhms[1]),int(tqhms[2])) 
        #   print "date", dt.year, dt.month, dt.day
        #   print "time", dt.hour

	#toff = int(min(tx))
        #if tz=='LST':
        #  tlab = tutc
          #tofflab = int(min(tlab))
          #tx = tutc
        #  toff = int(min(tx))
        #  tofflab = toff
        #  print "tx=",tx
        #  print "toff=",toff
        #  print "tofflab=",tofflab
        #else:
	#  tofflab = toff

	#tx -= toff
	#tx = tx*24.
        #print "tx[0]=", tx[0], " tx[-1]=",tx[-1]
        #for itm in range(len(tx)):
        #   if tx[itm] >24.0:
        #     tx[itm]-=24.0
        #print "tx[0]=", tx[0], " tx[-1]=",tx[-1]
	#timlab = qa.time(qa.quantity(tofflab,'d'), form=['ymd', 'no_time'])
	#timlab += ' %s' % obs 
	timlab = qa.time(qa.quantity(int(min(tx)),'d'), form=['ymd', 'no_time'])[0]
	timlab += ' %s' % obs 
       
	defaultcolors=['b','g','r','c','m','y','k']
	cindx = fmod(i,len(defaultcolors))

	if plottype=='both' or plottype=='el':
	    #print "plotting El vs. time.."
	    if i==0:
                if plottype=='both': 
		    ax=fig.add_subplot(2,1,1)
	        if plottype=='el': 
		    #pl.subplot(1,1,1)
		    ax=fig.add_subplot(1,1,1)
                ax.xaxis.set_major_formatter(tmfmt)
	    if last:
		#ax.xlabel(tz)
		#ax.ylabel('EL(deg.)')
		ax.set_xlabel(tz)
		ax.set_ylabel('EL(deg.)')
	    #pl.xlim(min(tx),max(tx))
	    ax.plot_date(tx,eld,marker='.',markersize=2.0,color=defaultcolors[cindx])   
            if last:
                if elimitv>elimit_hard:
                  ax.axhline(elimitv, linestyle='dashed')
	    # get max for label
	    maxel = max(eld)
	    maxidx = argmax(eld)
	    if last:
                #if tz=='LST': 
		#  pl.xlim(tx[0],tx[-1])
                #else:
		#  pl.xlim(0,24.)
		ax.set_ylim(elimit_hard,90.)
		ax.set_title(timlab)
	    ax.text(tx[maxidx],maxel*1.01,srcname,color=defaultcolors[cindx])

	if plottype=='both' or plottype=='az':
	    #print "plotting Az vs. time.."
	    if i==0: 
                if plottype=='both':
		    ax2=fig.add_subplot(2,1,2)
	        if plottype=='az':
		    ax2=fig.add_subplot(1,1,1)
                ax2.xaxis.set_major_formatter(tmfmt)
	    ax2.plot_date(tx,azd,marker='.',markersize=2.0,color=defaultcolors[cindx])
	    maxaz = max(azd)
	    maxazidx = argmax(azd)
	    if last:
	        ax2.set_xlabel(tz)
	        ax2.set_ylabel('Az(deg.)')
                #if tz=='LST':
                #    pl.xlim(tx[0],tx[-1])
                #else:
                #    pl.xlim(0,24.)
	        #pl.xlim(0,24.)
	        ax2.set_ylim(-180.,180.)
	    #ax.text(tx[maxidx],*1.01,srcname,color=defaultcolors[cindx])
            print srcname," max EL:", maxel

    #pl.draw()
    if saveplot!='':
        fig.savefig(saveplot)
