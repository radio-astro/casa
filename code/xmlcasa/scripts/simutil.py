# geodesy and pointing and other helper functions that are useful
# to be available outside of the simdata task
import casac
import os
import shutil
import commands
import pdb
# all I really need is casalog, but how to get it:?
from taskinit import *
import pylab as pl
qatool = casac.homefinder.find_home_by_name('quantaHome')
qa = qatool.create()

class compositenumber:
    def __init__(self, maxval=100):
        self.generate(maxval)
    def generate(self,maxval):
        n2 = int(log(float(maxval))/log(2.0) + 1) +1
        n3 = int(log(float(maxval))/log(3.0) + 1) +1
        n5 = int(log(float(maxval))/log(5.0) + 1) +1
        itsnumbers=pl.zeros(n2*n3*n5)
        n = 0
        for i2 in range(n2):
            for i3 in range(n3):
                for i5 in range(n5):
                    composite=( 2.**i2 * 3.**i3 * 5.**i5 )
                    itsnumbers[n] = composite
                    #print i2,i3,i5,composite
                    n=n+1
        itsnumbers.sort()
        maxi=0
        while maxi<(n2*n3*n5) and itsnumbers[maxi]<=maxval: maxi=maxi+1
        self.itsnumbers=pl.int64(itsnumbers[0:maxi])
    def list(self):
        print self.itsnumbers
    def nextlarger(self,x):
        if x>max(self.itsnumbers): self.generate(2*x)
        xi=0
        n=self.itsnumbers.__len__()
        while xi<n and self.itsnumbers[xi]<x: xi=xi+1
        return self.itsnumbers[xi]



class simutil:
    def __init__(self, direction="",
                 startfreq=qa.quantity("245GHz"),
                 bandwidth=qa.quantity("1GHz"),
                 totaltime=qa.quantity("0h"),
                 verbose=False):
        self.direction=direction
        self.verbose=verbose
        self.startfreq=startfreq
        self.bandwidth=bandwidth
        self.totaltime=totaltime
        self.nplots=1
        self.iplot=0

#    def newplot(self,nplots=1):  #set up nplots, subwindow etc
#    def nextplot: # advamce subwindow
#    def endplot: # margins etc


    def msg(self, s, origin=None, priority=None):
        # ansi color codes:
        # Foreground colors
        # 30    Black
        # 31    Red
        # 32    Green
        # 33    Yellow
        # 34    Blue
        # 35    Magenta
        # 36    Cyan
        # 37    White
        toterm=False
        if priority==None:
            clr="\x1b[32m"
            priority="INFO"
        else:
            priority=priority.upper()
            if priority=="WARN":
                clr="\x1b[35m"                
                toterm=True
                priority="INFO" # otherwise casalog will spew to term also.
            else:
                if priority=="ERROR":
                    clr="\x1b[31m"
                    toterm=True
                else:
                    if not (priority=="DEBUG" or priority[:-1]=="DEBUG"):
                        priority="INFO"
        bw="\x1b[0m"
        if origin==None:
            origin="simdata"            
        if toterm:
            print clr+"["+origin+"] "+bw+s
        casalog.post(s,priority=priority,origin=origin)


# helper function to plot an image (optionally), and calculate its statistics
# we could move this to the utility object (should, to facilitate "restart" of fidelity etc calculation after ms creation"

    def statim(self,image,plot=True,incell=None,disprange=None):
        ia.open(image)       
        imunit=ia.summary()['header']['unit']            
        if imunit == 'Jy/beam':
            # stupid for dirty image:
            if len(ia.restoringbeam())>0:
                bm=ia.summary()['header']['restoringbeam']['restoringbeam']
                toJyarcsec=1./(qa.convert(bm['major'],'arcsec')['value']*
                               qa.convert(bm['minor'],'arcsec')['value']*pl.pi/4)
            else:
                toJyarcsec=1.
            pix=ia.summary()['header']['incr']
            toJypix=toJyarcsec*abs(pix[0]*pix[1])*206265.0**2
        elif imunit == 'Jy/pixel':
            pix=ia.summary()['header']['incr']
            toJyarcsec=1./abs(pix[0]*pix[1])/206265.0**2
            toJypix=1.
        else:
            self.msg("%s: unknown units" % image,origin="statim")
            toJyarcsec=1.
            toJypix=1.
        stats=ia.statistics(robust=True,verbose=False,list=False)
        im_min=stats['min']*toJypix
        if type(im_min)==type([]):
            if len(im_min)<1: im_min=0.
        im_max=stats['max']*toJypix
        if type(im_max)==type([]):
            if len(im_max)<1: im_max=1.
        imsize=ia.shape()[0:2]
        reg1=rg.box([0,0],[imsize[0]*.25,imsize[1]*.25])
        stats=ia.statistics(region=reg1,verbose=False,list=False)
        #im_rms=stats['rms']*toJyarcsec
        im_rms=stats['rms']*toJypix
        if type(im_rms)==type([]):
            if len(im_rms)==0: im_rms=0.
        data_array=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],[1],[],True,True,False)
        data_array=pl.array(data_array)
        tdata_array=pl.transpose(data_array)
        ttrans_array=tdata_array.tolist()
        ttrans_array.reverse()
        if (plot):
            csys=ia.coordsys()            
            xpix=qa.quantity(csys.increment(type="direction")["numeric"][0],csys.units(type="direction")[0])
            ypix=qa.quantity(csys.increment(type="direction")["numeric"][1],csys.units(type="direction")[1])
            xpix=qa.convert(xpix,'arcsec')['value']
            ypix=qa.convert(ypix,'arcsec')['value']
            xform=csys.lineartransform(type="direction")
            offdiag=max(abs(xform[0,1]),abs(xform[1,0]))
            if offdiag > 1e-4:
                self.msg("Your image is rotated with respect to Lat/Lon.  I can't cope with that yet",origin="statim",priority="error")
            factor=pl.sqrt(abs(pl.det(xform)))
            xpix=abs(xpix*factor)
            ypix=abs(ypix*factor)
            #if abs(xpix-ypix)/(xpix+ypix) < 1e-4:
            #    self.msg("WARN: image %s doesn't have square pixels" % image,origin="statim")
            pixsize=[xpix,ypix]
            if incell != None:
                if type(incell)==type(""):
                    incell=qa.quantity(incell)
                if type(incell)==type([]): incell=qa.sqrt(qa.mul(incell[0],incell[1]))
                #print incell
                pixsize=qa.convert(incell,'arcsec')['value']+pl.zeros(2)
                xpix=pixsize[0]
                ypix=pixsize[1]
            if self.verbose: self.msg("plotting image with pixel size %f x %f arcsec" % (xpix,ypix),origin="statim")
            xextent=imsize[0]*abs(pixsize[0])*0.5
            yextent=imsize[1]*abs(pixsize[1])*0.5
            if self.verbose: self.msg("plotting image with field size %f x %f arcsec" % (xextent,yextent),origin="statim")
            xextent=[xextent,-xextent]
            yextent=[-yextent,yextent]
        # remove top .5% of pixels:
        nbin=200
        imhist=ia.histograms(cumu=True,nbins=nbin)['histout']
        ii=nbin-1
        highcounts=imhist['counts'][ii]
        while imhist['counts'][ii]>0.995*highcounts and ii>0: 
            ii=ii-1
        highvalue=imhist['values'][ii]
        if disprange != None:
            if type(disprange)==type([]):
                n=len(disprange)
                if n>0:
                    highvalue=disprange[n-1]
                else:
                    disprange.append(highvalue)  # return highvalue
            else:
                highvalue=disprange
            #
        if (plot):
            pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,vmax=highvalue)
            ax=pl.gca()
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
            pl.title(image,fontsize="x-small")
            # from matplotlib.font_manager import fontManager, FontProperties
            # font= FontProperties(size='x-small');
            # pl.legend(("min=%7.1e" % im_min,"max=%7.1e" % im_max,"RMS=%7.1e" % im_rms),pad=0.15,prop=font)
            pl.text(0.05,0.95,"min=%7.1e\nmax=%7.1e\nRMS=%7.1e" % (im_min,im_max,im_rms),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
        ia.done()
        return im_min,im_max,im_rms





    def calc_pointings(self, spacing, imsize, direction=None, relmargin=0.33):
        """
        If direction is a list, simply returns direction and the number of
        pointings in it.
        
        Otherwise, returns a hexagonally packed list of pointings separated by
        spacing and fitting inside an area specified by direction and imsize, 
        as well as the number of pointings.  The hexagonal packing starts with a
        horizontal row centered on direction, and the other rows alternate
        being horizontally offset by a half spacing.  
        """
        # make imsize 2-dimensional:
        if type(imsize) != type([]):
            imsize=[imsize,imsize]
        if len(imsize) <2:
            imsize=[imsize[0],imsize[0]]
        if direction==None:
            # if no direction is specified, use the object's direction
            direction=self.direction
        else:
            # if one is specified, use that to set the object's direction
            # do we really want to do this?
            self.direction=direction

        # direction is always a list of strings (defined by .xml)
        if type(direction)==type([]):
            if len(direction) > 1:
                if self.verbose: self.msg("You are inputing the precise pointings in 'direction' - if you want me to calculate a mosaic, give a single direction",priority="warn")
                return len(direction), direction, [0.]*len(direction) #etime at end
            else: direction=direction[0]        


        # now its either a filename or a single direction:
        # do we need this string check?  maybe it could be a quantity?
        #if type(direction) == str:
        # Assume direction as a filename and read lines if it exists
        filename=os.path.expanduser(os.path.expandvars(direction))
        if os.path.exists(filename):
            self.msg('Reading direction information from the file, %s' % filename)
            return self.read_pointings(filename)
        
        # haveing elimiated other options, we need to calculate:
        epoch, centx, centy = self.direction_splitter()

        spacing  = qa.quantity(spacing)
        yspacing = qa.mul(0.866025404, spacing)
    
        xsize=qa.quantity(imsize[0])
        ysize=qa.quantity(imsize[1])

        nrows = 1+ int(pl.floor(qa.convert(qa.div(ysize, yspacing), '')['value']
                                - 2.309401077 * relmargin))

        availcols = 1 + qa.convert(qa.div(xsize, spacing),
                                   '')['value'] - 2.0 * relmargin
        ncols = int(pl.floor(availcols))

        # By making the even rows shifted spacing/2 ahead, and possibly shorter,
        # the top and bottom rows (nrows odd), are guaranteed to be short.
        if availcols - ncols >= 0.5:                            # O O O
            evencols = ncols                                    #  O O O
            ncolstomin = 0.5 * (ncols - 0.5)
        else:
            evencols = ncols - 1                                #  O O 
            ncolstomin = 0.5 * (ncols - 1)                      # O O O
        pointings = []

        # Start from the top because in the Southern hemisphere it sets first.
        y = qa.add(centy, qa.mul(0.5 * (nrows - 1), yspacing))
        for row in xrange(0, nrows):         # xrange stops early.
            xspacing = qa.mul(1.0 / pl.cos(qa.convert(y, 'rad')['value']),spacing)
            ystr = qa.formxxx(y, format='dms')
        
            if row % 2:                             # Odd
                xmin = qa.sub(centx, qa.mul(ncolstomin, xspacing))
                stopcolp1 = ncols
            else:                                   # Even (including 0)
                xmin = qa.sub(centx, qa.mul(ncolstomin - 0.5,
                                                 xspacing))
                stopcolp1 = evencols
            for col in xrange(0, stopcolp1):        # xrange stops early.
                x = qa.formxxx(qa.add(xmin, qa.mul(col, xspacing)),
                               format='hms')
                pointings.append("%s%s %s" % (epoch, x, ystr))
            y = qa.sub(y, yspacing)
        ####could not fit pointings then single pointing
        if(len(pointings)==0):
            pointings.append(direction)
        self.msg("using %i generated pointing(s)" % len(pointings))
        self.pointings=pointings
        return len(pointings), pointings, [0.]*len(pointings)



    def read_pointings(self, filename):
        """
        read pointing list from file containing epoch, ra, dec,
        and scan time (optional,in sec).
        Parameter:
             filename:  (str) the name of input file
       
        The input file (ASCII) should contain at least 3 fields separated
        by a space which specify positions with epoch, ra and dec (in dms
        or hms).
        The optional field, time, shoud be a list of decimal numbers
        which specifies integration time at each position in second.
        The lines which start with '#' is ignored and can be used
        as comment lines. 
        
        Example of an input file:
        #Epoch     RA          DEC      TIME(optional)
        J2000 23h59m28.10 -019d52m12.35 10.0
        J2000 23h59m32.35 -019d52m12.35 10.0
        J2000 23h59m36.61 -019d52m12.35 60.0
        
        """
        f=open(filename)
        line= '  '
        time=[]
        pointings=[]

        # add option of different epoch in a header line like read_antenna?

        while (len(line)>0):
            try: 
                line=f.readline()
                if not line.startswith('#'):
                ### ignoring line that has less than 3 elements
                     if(len(line.split()) >2):
                        splitline=line.split()
                        epoch=splitline[0]
                        ra0=splitline[1]
                        de0=splitline[2]
                        if len(splitline)>3:
                            time.append(float(splitline[3]))
                        else:
                            time.append(0.)
                        xstr = qa.formxxx(qa.quantity(ra0), format='hms')
                        ystr = qa.formxxx(qa.quantity(de0), format='dms')
                        pointings.append("%s %s %s" % (epoch,xstr,ystr))
            except:
                break
        f.close()

        # need an error check here if zero valid pointings were read
        if len(pointings) < 1:
            s="No valid point is found in input file"
            self.msg(s,priority="error")
        self.msg("read in %i pointing(s) from file" % len(pointings))
        self.pointings=pointings
        #self.direction=pointings
                
        return len(pointings), pointings, time

    

    def average_direction(self, directions=None):
        # RI TODP make deal with list of measures as well as list of strings
        """
        Returns the average of directions as a string, and relative offsets
        """
        if directions==None:
            directions=self.direction
        epoch0, x, y = self.direction_splitter(directions[0])
        i = 1
        avgx = 0.0
        avgy = 0.0
        for drn in directions:
            epoch, x, y = self.direction_splitter(drn)
            # in principle direction_splitter returns directions in degrees,
            # but can we be sure?
            x=qa.convert(x,'deg')
            y=qa.convert(y,'deg')
            x = x['value']
            y = y['value']
            if epoch != epoch0:                     # Paranoia
                print "[simutil] WARN: precession not handled by average_direction()"
            x = self.wrapang(x, avgx, 360.0)
            avgx += (x - avgx) / i
            avgy += (y - avgy) / i
            i += 1
        offsets=pl.zeros([2,i-1])
        i=0
        cosdec=pl.cos(avgy*pl.pi/180.)
        for drn in directions:
            epoch, x, y = self.direction_splitter(drn)
            x=qa.convert(x,'deg')
            y=qa.convert(y,'deg')
            x = x['value']
            y = y['value']
            x = self.wrapang(x, avgx, 360.0)
            offsets[:,i]=[(x-avgx)*cosdec,y-avgy]  # apply cosdec to make offsets on sky
            i+=1
        avgx = qa.toangle('%fdeg' % avgx)
        avgy = qa.toangle('%fdeg' % avgy)
        avgx = qa.formxxx(avgx, format='hms')
        avgy = qa.formxxx(avgy, format='dms')
        return "%s%s %s" % (epoch0, avgx, avgy), offsets


    def direction_splitter(self, direction=None):
        """
        Given a direction, return its epoch, x, and y parts.  Epoch will be ''
        if absent, or '%s ' % epoch if present.  x and y will be angle qa's in
        degrees.
        """
        if direction == None:
            direction=self.direction
        if type(direction) == type([]):
            direction=self.average_direction(direction)[0]
        dirl = direction.split()
        if len(dirl) == 3:
            epoch = dirl[0] + ' '
        else:
            epoch = ''
        x, y = map(qa.toangle, dirl[-2:])
        return epoch, qa.convert(x, 'deg'), qa.convert(y, 'deg')


    def dir_s2m(self, direction=None):
        """
        Given a direction as a string 'refcode lon lat', return it as qa measure.
        """
        if direction == None:
            direction=self.direction
        if type(direction) == type([]):
            direction=self.average_direction(direction)[0]            
        dirl = direction.split()
        if len(dirl) == 3:
            refcode = dirl[0] + ' '
        else:
            refcode = 'J2000'
            if self.verbose: self.msg("assuming J2000 for "+direction,origin="simutil.s2m")
        x, y = map(qa.quantity, dirl[-2:])
        if x['unit'] == '': x['unit']='deg'
        if y['unit'] == '': y['unit']='deg'
        return me.direction(refcode,qa.toangle(x),qa.toangle(y))


    def dir_m2s(self, dir):
        """
        Given a direction as a measure, return it as astring 'refcode lon lat'.
        """
        if dir['type'] != 'direction':
            self.msg("converting direction measure",priority="error",origin="simutil.m2s")
            return False
        ystr = qa.formxxx(dir['m1'], format='dms')
        xstr = qa.formxxx(dir['m0'], format='hms')
        return "%s %s %s" % (dir['refer'], xstr, ystr)


    def wrapang(self, ang, target, period = 360.0):
        """
        Returns ang wrapped so that it is within +-period/2 of target.
        """
        dang       = ang - target
        period     = pl.absolute(period)
        halfperiod = 0.5 * period
        if pl.absolute(dang) > halfperiod:
            nwraps = pl.floor(0.5 + float(dang) / period)
            ang -= nwraps * period
        return ang
    


    #==================================== tsys ==========================

    def noisetemp(self, telescope=None, freq=None,
                  diam=None, epsilon=None):
        
        if telescope==None: telescope=self.telescopename
        telescope=str.upper(telescope)
        
        obs =['ALMA','ACA','EVLA','VLA']
        d   =[ 12.   ,7.,   25.  , 25. ]
        ds  =[ 0.75,  0.75, 0.364, 0.364] # what is subreflector size for ACA?!
        eps =[ 25.,   25.,  300,   300 ]  # antenna surface accuracy
        
        cq  =[ 0.95, 0.95,  0.91,  0.79]  # correlator quantization eff
        # VLA includes additional waveguide loss from correlator loss of 0.809
        
        if obs.count(telescope)>0:
            iobs=obs.index(telescope)
        else:
            if self.verbose: self.msg("I don't know much about "+telescope+" so I'm going to use ALMA specs")
            iobs=0 # ALMA is the default ;)
            
        if diam==None: diam=d[iobs]
        diam_subreflector=ds[iobs]
        if self.verbose: self.msg("subreflector diameter="+str(diam_subreflector),origin="noisetemp")

        # blockage efficiency.    
        eta_b = 1.-(diam_subreflector/diam)**2

        # spillover efficiency.    
        eta_s = 0.96 # these are ALMA values
        # taper efficiency.    
        eta_t = 0.86 # these are ALMA values

        # Ruze phase efficiency.    
        if epsilon==None: epsilon = eps[iobs] # microns RMS
        if freq==None:            
            startfreq_ghz=qa.convert(qa.quantity(self.startfreq),'GHz')
            bw_ghz=qa.convert(qa.quantity(self.bandwidth),'GHz')
            freq_ghz=qa.add(startfreq_ghz,qa.mul(qa.quantity(0.5),bw_ghz))
        else:            
            freq_ghz=qa.convert(qa.quantity(freq),'GHz')
        eta_p = pl.exp(-(4.0*3.1415926535*epsilon*freq_ghz.get("value")/2.99792458e5)**2)
        if self.verbose: self.msg("ruze phase efficiency for surface accuracy of "+str(epsilon)+"um = " + str(eta_p),origin="noisetemp")

        # antenna efficiency
        # eta_a = eta_p*eta_s*eta_b*eta_t

        # correlator quantization efficiency.    
        eta_q = cq[iobs]

        # Receiver radiation temperature in K.         
        if telescope=='ALMA' or telescope=='ACA':
            # ALMA-40.00.00.00-001-A-SPE.pdf
            f0=[ 35, 75,110,145,185,230,345,409,675,867]
            t0=[ 17, 30, 37, 51, 65, 83,147,196,175,230]
            flim=[31.3,950]
            if self.verbose: self.msg("using ALMA/ACA Rx specs",origin="noisetemp")
        else:
            if telescope=='EVLA':
                # these are T_Rx/epsilon so may be incorrect
                # http://www.vla.nrao.edu/astro/guides/vlas/current/node11.html
                # http://www.vla.nrao.edu/astro/guides/evlareturn/stress_tests/L-Band/
                f0=[0.33,1.47,4.89,8.44,22.5,33.5,43.3]
                t0=[500, 70,  60,  55,  100, 130, 350]
                flim=[0.305,50]
                if self.verbose: self.msg("using EVLA Rx specs",origin="noisetemp")
            else:
                if telescope=='VLA':
                    # http://www.vla.nrao.edu/genpub/overview/
                    # exclude P band for now
                    # f0=[0.0735,0.32, 1.5, 4.75, 8.4, 14.9, 23, 45 ]
                    # t0=[5000,  165,  56,  44,   34,  110, 110, 110]
                    f0=[0.32, 1.5, 4.75, 8.4, 14.9, 23, 45 ]
                    t0=[165,  56,  44,   34,  110, 110, 110]
                    flim=[0.305,50]
                    if self.verbose: self.msg("using old VLA Rx specs",origin="noisetemp")
                else:
                    self.msg("I don't know about the "+telescope+" receivers, using 200K",priority="warn",origin="noisetemp")
                    f0=[10,900]
                    t0=[200,200]
                    flim=[0,5000]


        obsfreq=freq_ghz.get("value")
        t_rx = pl.interp([obsfreq],f0,t0)[0]
        if obsfreq>f0[-1] or obsfreq<f0[0]:
            t_rx = pl.polyval(pl.polyfit(f0,t0,2),obsfreq)
        # too jumpy
        # sp=spline(f0,t0)
        # t_rx = sp(obsfreq)[0]
        
        if obsfreq<flim[0]:
            self.msg("observing freqency is lower than expected for "+telescope,priority="warn",origin="noise")
            self.msg("proceeding with extrapolated receiver temp="+str(t_rx),priority="warn",origin="noise")
        if obsfreq>flim[1]:
            self.msg("observing freqency is higher than expected for "+telescope,priority="warn",origin="noise")
            self.msg("proceeding with extrapolated receiver temp="+str(t_rx),priority="warn",origin="noise")
        if obsfreq<=flim[1] and obsfreq>=flim[0]:
            self.msg("interpolated receiver temp="+str(t_rx),origin="noise")

        return eta_p, eta_s, eta_b, eta_t, eta_q, t_rx
    
#        # NewMSSimulator needs 2-temp formula not just t_atm
#        sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
#                    antefficiency=eta_a,trx=t_rx,
#                    tau=tau0,tatmos=t_atm,tcmb=t_cmb,
#                    mode="calculate")

    












    #==================================== ephemeris ==========================


    def ephemeris(self, date, direction=None, telescope=None):
        if direction==None: direction=self.direction
        if telescope==None: telescope=self.telescopename
        
        # right now, simdata centers at the transit;  when that changes,
        # or when that becomes optional, then that option needs to be
        # stored in the simutil object and used here, to either
        # center the plot at transit or not.
        #
        # direction="J2000 18h00m00.03s -45d59m59.6s"
        # refdate="2012/06/21/03:25:00"
        
        ds=self.direction_splitter(direction)  # if list, returns average
        src=me.direction(ds[0],ds[1],ds[2])
    
        me.done()
        me.doframe(me.observatory(telescope))
    
        time=me.epoch('TAI',date)
        me.doframe(time)
        reftime_float=time['m0']['value']

        timeinc='15min'  # for plotting
        timeinc=qa.convert(qa.time(timeinc),'d')['value']
        ntime=int(1./timeinc)

        # check for circumpolar:
        rise=me.riseset(src)['rise']
        if rise == 'above':
            rise = time
            rise['m0']['value'] = rise['m0']['value'] - 0.5
        else:
            rise=me.measure(rise['utc'],'tai')

        set=me.riseset(src)['set']
        if set == 'above':
            set = time
            set['m0']['value'] = set['m0']['value'] + 0.5
        else:
            set=me.measure(set['utc'],'tai')

        offset=-0.5
        if set < time: offset-=0.5
        if rise > time: offset+=0.5
        time['m0']['value']+=offset
        starttime_float=round(time['m0']['value'])
        startdate_str=qa.time(qa.totime(str(round(time['m0']['value']))+'d'),form='dmy')
        times=[]
        az=[]
        el=[]
    
        for i in range(ntime):
            times.append(time['m0']['value'])
            me.doframe(time)
            azel=me.measure(src,'azel')
            az.append(qa.convert(azel['m0'],'deg')['value'])
            el.append(qa.convert(azel['m1'],'deg')['value'])
            time['m0']['value']+=timeinc
    
        self.msg(" ref="+date,origin='ephemeris')
        self.msg("rise="+qa.time(rise['m0'],form='dmy'),origin='ephemeris')
        self.msg(" set="+qa.time(set['m0'],form='dmy'),origin='ephemeris')
    
        pl.plot((pl.array(times)-starttime_float)*24,el)
        peak=(rise['m0']['value']+set['m0']['value'])/2
        self.msg("peak="+qa.time('%fd' % peak,form='dmy'),origin='ephemeris')

        relpeak=reftime_float-starttime_float
        pl.plot(pl.array([1,1])*24*relpeak,[0,90])
        pl.xlabel("hours relative to "+startdate_str,fontsize='x-small')
        pl.ylabel("elevation",fontsize='x-small')
        ax=pl.gca()
        l=ax.get_xticklabels()
        pl.setp(l,fontsize="x-small")
        l=ax.get_yticklabels()
        pl.setp(l,fontsize="x-small")

        if self.totaltime>0:
            etimeh=qa.convert(self.totaltime,'h')['value']
            pl.plot(pl.array([-0.5,0.5])*etimeh+(peak-starttime_float)*24,[80,80],'r')

        pl.ylim([0,90])



















    #=========================================================================
    
    def readantenna(self, antab=None):
    ###Helper function to read 4 columns text antenna table X, Y, Z, Diam
        f=open(antab)
        line= '  '
        inx=[]
        iny=[]
        inz=[]
        ind=[]
        id=[] # pad id
        nant=0
        line='    '
        params={}
        while (len(line)>0):
            try: 
                line=f.readline()
                if line.startswith('#'):
                    line=line[1:]
                    paramlist=line.split('=')
                ### if the line is a parameter line like coordsys=utms, then it stores that
                    if (paramlist.__len__() == 2):
                        if params==None:
                            params={paramlist[0].strip():paramlist[1].strip()}
                        else:
                            params[paramlist[0].strip()]=paramlist[1].strip()
                else:
                ### ignoring line that has less than 4 elements
                ### all coord systems should have x,y,z,diam,id, where xyz varies
                    #print line.split()
                    if(len(line.split()) >3):
                        splitline=line.split()
                        inx.append(float(splitline[0]))
                        iny.append(float(splitline[1]))
                        inz.append(float(splitline[2]))
                        ind.append(float(splitline[3]))
                        if len(splitline)>4:
                            id.append(splitline[4])
                        else:
                            id.append('A%02d'%nant)                            
                        nant+=1                 
            except:
                break
        f.close()

        if not params.has_key("observatory"):
            self.msg("Must specify observatory in antenna file",origin="readantenna",priority="error")
            return -1
        else:
            self.telescopename=params["observatory"]
            if self.verbose:
                self.msg("Using observatory= %s" % params["observatory"],origin="readantenna")

        if not params.has_key("coordsys"):
            self.msg("Must specify coordinate system #coorsys=XYZ|UTM|GEO in antenna file",origin="readantenna",priority="error")
            return -1
        else:
            self.coordsys=params["coordsys"]

        if (params["coordsys"].upper()=="XYZ"):
        ### earth-centered XYZ i.e. ITRF in casa
            stnx=inx
            stny=iny
            stnz=inz
        else:
            stnx=[]
            stny=[]
            stnz=[]
            if (params["coordsys"].upper()=="UTM"):
        ### expect easting, northing, elevation in m
                self.msg("Antenna locations in UTM; will read from file easting, northing, elevation in m",origin="readantenna") 
                if params.has_key("zone"):
                    zone=params["zone"]
                else:
                    self.msg("You must specify zone=NN in your antenna file",origin="readantenna",priority="error")
                    return -1
                if params.has_key("datum"):
                    datum=params["datum"]
                else:
                    self.msg("You must specify datum in your antenna file",origin="readantenna",priority="error")
                    return -1
                if params.has_key("hemisphere"):
                    nors=params["hemisphere"]
                    nors=nors[0].upper()
                else:
                    self.msg("You must specify hemisphere=N|S in your antenna file",origin="readantenna",priority="error")
                    return -1
                
                # if self.verbose: foo=self.getdatum(datum,verbose=True)
                for i in range(len(inx)):
                    x,y,z = self.utm2xyz(inx[i],iny[i],inz[i],int(zone),datum,nors)
                    stnx.append(x)
                    stny.append(y)
                    stnz.append(z)                
            else:
                if (params["coordsys"].upper()[0:3]=="LOC"):
                    # I'm pretty sure Rob's function only works with lat,lon in degrees;
                    meobs=me.observatory(params["observatory"])
                    if (meobs.__len__()<=1):
                        self.msg("You need to add "+params["observatory"]+" to the Observatories table in your installation to proceed.",priority="error")
                        return False,False,False,False,False,params["observatory"]
                    obs=me.measure(meobs,'WGS84')
                    obslat=qa.convert(obs['m1'],'deg')['value']
                    obslon=qa.convert(obs['m0'],'deg')['value']
                    obsalt=qa.convert(obs['m2'],'m')['value']
                    if self.verbose:
                        self.msg("converting local tangent plane coordinates to ITRF using observatory position = %d %d " % (obslat,obslon))
                        #foo=self.getdatum(datum,verbose=True)
                    for i in range(len(inx)):
                        x,y,z = self.locxyz2itrf(obslat,obslon,inx[i],iny[i],inz[i]+obsalt)
                        stnx.append(x)
                        stny.append(y)
                        stnz.append(z)                
                else:
                    if (params["coordsys"].upper()[0:3]=="GEO"):
                        if params.has_key("datum"):
                            datum=params["datum"]
                        else:
                            self.msg("You must specify zone=NN in your antenna file",origin="readantenna",priority="error")
                            return -1
                        if (datum.upper() != "WGS84"):
                            self.msg("Unfortunately I only can deal with WGS84 right now",origin="readantenna",priority="error")
                            return -1
                        self.msg("geodetic coordinates not implemented yet",priority="error")
                    
        return (stnx, stny, stnz, pl.array(ind), id, nant, params["observatory"])



















    #==================================== geodesy =============================


    def tmgeod(self,n,e,eps,cm,fe,sf,so,r,v0,v2,v4,v6,fn,er,esq):
        """
        Transverse Mercator Projection
        conversion of grid coords n,e to geodetic coords
        revised subroutine of t. vincenty  feb. 25, 1985
        converted from Fortran R Indebetouw Jan 2009
        ********** symbols and definitions ***********************
        latitude positive north, longitude positive west.
        all angles are in radian measure.
        
        input:
        n,e       are northing and easting coordinates respectively
        er        is the semi-major axis of the ellipsoid
        esq       is the square of the 1st eccentricity
        cm        is the central meridian of the projection zone
        fe        is the false easting value at the cm
        eps       is the square of the 2nd eccentricity
        sf        is the scale factor at the cm
        so        is the meridional distance (times the sf) from the
        equator to southernmost parallel of lat. for the zone
        r         is the radius of the rectifying sphere
        u0,u2,u4,u6,v0,v2,v4,v6 are precomputed constants for
        determination of meridianal dist. from latitude
        output:
        lat,lon   are lat. and long. respectively
        conv      is convergence
        kp        point scale factor
        
        the formula used in this subroutine gives geodetic accuracy
        within zones of 7 degrees in east-west extent.  within state
        transverse mercator projection zones, several minor terms of
        the equations may be omitted (see a separate ngs publication).
        if programmed in full, the subroutine can be used for
        computations in surveys extending over two zones.        
        """
        
        om=(n-fn+so)/(r*sf)  # (northing - flag_north + distance_from_equator)
        cosom=pl.cos(om)
        foot=om+pl.sin(om)*cosom*(v0+v2*cosom*cosom+v4*cosom**4+v6*cosom**6)
        sinf=pl.sin(foot)
        cosf=pl.cos(foot)
        tn=sinf/cosf
        ts=tn*tn
        ets=eps*cosf*cosf
        rn=er*sf/pl.sqrt(1.-esq*sinf*sinf)
        q=(e-fe)/rn
        qs=q*q
        b2=-tn*(1.+ets)/2.
        b4=-(5.+3.*ts+ets*(1.-9.*ts)-4.*ets*ets)/12.
        b6=(61.+45.*ts*(2.+ts)+ets*(46.-252.*ts-60.*ts*ts))/360.
        b1=1.
        b3=-(1.+ts+ts+ets)/6.
        b5=(5.+ts*(28.+24.*ts)+ets*(6.+8.*ts))/120.
        b7=-(61.+662.*ts+1320.*ts*ts+720.*ts**3)/5040.
        lat=foot+b2*qs*(1.+qs*(b4+b6*qs))
        l=b1*q*(1.+qs*(b3+qs*(b5+b7*qs)))
        lon=-l/cosf+cm
        
        # compute scale factor
        fi=lat
        lam = lon
        sinfi=pl.sin(fi)
        cosfi=pl.cos(fi)
        l1=(lam-cm)*cosfi
        ls=l1*l1
        
        tn=sinfi/cosfi
        ts=tn*tn
        
        # convergence
        c1=-tn
        c3=(1.+3.*ets+2.*ets**2)/3.
        c5=(2.-ts)/15.
        conv=c1*l1*(1.+ls*(c3+c5*ls))
        
        # point scale factor
        f2=(1.+ets)/2.
        f4=(5.-4.*ts+ets*( 9.-24.*ts))/12.
        kp=sf*(1.+f2*ls*(1.+f4*ls))
        
        return lat,lon,conv,kp




    def tconpc(self,sf,orlim,er,esq,rf):
        
        """
        transverse mercator projection               ***
        conversion of grid coords to geodetic coords
        revised subroutine of t. vincenty  feb. 25, 1985
        converted from fortran r. indebetouw jan 2009
        ********** symbols and definitions ***********************
        input:
        rf is the reciprocal flattening of ellipsoid
        esq = e squared (eccentricity?)    
        er is the semi-major axis for grs-80
        sf is the scale factor at the cm
        orlim is the southernmost parallel of latitude for which the
        northing coord is zero at the cm
        
        output:
        eps
        so is the meridional distance (times the sf) from the
        equator to southernmost parallel of lat. for the zone
        r is the radius of the rectifying sphere
        u0,u2,u4,u6,v0,v2,v4,v6 are precomputed constants for
        determination of meridional dist. from latitude
        **********************************************************
        """
        
        f=1./rf
        eps=esq/(1.-esq)
        pr=(1.-f)*er
        en=(er-pr)/(er+pr)
        en2=en*en
        en3=en*en*en
        en4=en2*en2
        
        c2=-3.*en/2.+9.*en3/16.
        c4=15.*en2/16.-15.*en4/32.
        c6=-35.*en3/48.
        c8=315.*en4/512.
        u0=2.*(c2-2.*c4+3.*c6-4.*c8)
        u2=8.*(c4-4.*c6+10.*c8)
        u4=32.*(c6-6.*c8)
        u6=128.*c8
        
        c2=3.*en/2.-27.*en3/32.
        c4=21.*en2/16.-55.*en4/32.
        c6=151.*en3/96.
        c8=1097.*en4/512.
        v0=2.*(c2-2.*c4+3.*c6-4.*c8)
        v2=8.*(c4-4.*c6+10.*c8)
        v4=32.*(c6-6.*c8)
        v6=128.*c8
        
        r=er*(1.-en)*(1.-en*en)*(1.+2.25*en*en+(225./64.)*en4)
        cosor=pl.cos(orlim)
        omo=orlim+pl.sin(orlim)*cosor*(u0+u2*cosor*cosor+u4*cosor**4+u6*cosor**6)
        so=sf*r*omo
        
        return eps,r,so,v0,v2,v4,v6
    
    
    
    
    
    def getdatum(self,datumcode,verbose=False):
        """
        local datums and ellipsoids;
        take local earth-centered xyz coordinates, add dx,dy,dz to get wgs84 earth-centered
        """
        
        # set equatorial radius and inverse flattening
        
        ellipsoids={'AW':[6377563.396,299.3249647  ,'Airy 1830'                     ],
                    'AM':[6377340.189,299.3249647  ,'Modified Airy'                 ],
                    'AN':[6378160.0  ,298.25       ,'Australian National'           ],
                    'BR':[6377397.155,299.1528128  ,'Bessel 1841'                   ],
                    'CC':[6378206.4  ,294.9786982  ,'Clarke 1866'                   ],
                    'CD':[6378249.145,293.465      ,'Clarke 1880'                   ],
                    'EA':[6377276.345,300.8017     ,'Everest (India 1830)'          ],
                    'RF':[6378137.0  ,298.257222101,'Geodetic Reference System 1980'],
                    'HE':[6378200.0  ,298.30       ,'Helmert 1906'                  ],
                    'HO':[6378270.0  ,297.00       ,'Hough 1960'                    ],
                    'IN':[6378388.0  ,297.00       ,'International 1924'            ],
                    'SA':[6378160.0  ,298.25       ,'South American 1969'           ],
                    'WD':[6378135.0  ,298.26       ,'World Geodetic System 1972'    ],
                    'WE':[6378137.0  ,298.257223563,'World Geodetic System 1984'    ]}
        
        datums={
            'AGD66' :[-133, -48, 148,'AN','Australian Geodetic Datum 1966'], 
            'AGD84' :[-134, -48, 149,'AN','Australian Geodetic Datum 1984'], 
            'ASTRO' :[-104,-129, 239,'IN','Camp Area Astro (Antarctica)'  ], 
            'CAPE'  :[-136,-108,-292,'CD','CAPE (South Africa)'           ], 
            'ED50'  :[ -87, -98,-121,'IN','European 1950'                 ], 
            'ED79'  :[ -86, -98,-119,'IN','European 1979'                 ], 
            'GRB36' :[ 375,-111, 431,'AW','Great Britain 1936'            ], 
            'HAWAII':[  89,-279,-183,'IN','Hawaiian Hawaii (Old)'         ],
            'KAUAI' :[  45,-290,-172,'IN','Hawaiian Kauai (Old)'          ],
            'MAUI'  :[  65,-290,-190,'IN','Hawaiian Maui (Old)'           ],
            'OAHU'  :[  56,-284,-181,'IN','Hawaiian Oahu (Old)'           ],
            'INDIA' :[ 289, 734, 257,'EA','Indian'                        ],
            'NAD83' :[   0,   0,   0,'RF','N. American 1983'              ],
            'CANADA':[ -10, 158, 187,'CC','N. American Canada 1927'       ], 
            'ALASKA':[  -5, 135, 172,'CC','N. American Alaska 1927'       ],
            'NAD27' :[  -8, 160, 176,'CC','N. American Conus 1927'        ],
            'CARIBB':[  -7, 152, 178,'CC','N. American Caribbean'         ],
            'MEXICO':[ -12, 130, 190,'CC','N. American Mexico'            ],
            'CAMER' :[   0, 125, 194,'CC','N. American Central America'   ],
            'SAM56' :[-288, 175,-376,'IN','South American (Provisional1956)'],
            'SAM69' :[ -57, 1  , -41,'SA','South American 1969'           ],
            'CAMPO' :[-148, 136,  90,'IN','S. American Campo Inchauspe (Argentina)'],
            'WGS72' :[   0, 0  , 4.5,'WD','World Geodetic System - 72'    ],
            'WGS84' :[   0, 0  ,   0,'WE','World Geodetic System - 84'    ]}
        
        if not datums.has_key(datumcode):
            self.msg("unknown datum %s" % datumcode,priority="error")
            return -1
        
        datum=datums[datumcode]
        ellipsoid=datum[3]
        
        if not ellipsoids.has_key(ellipsoid):
            self.msg("unknown ellipsoid %s" % ellipsoid,priority="error")
            return -1
        
        if verbose:
            self.msg("Using %s datum with %s ellipsoid" % (datum[4],ellipsoids[ellipsoid][2]),origin="getdatum")
        return datum[0],datum[1],datum[2],ellipsoids[ellipsoid][0],ellipsoids[ellipsoid][1]
    
    



    def utm2long(self,east,north,zone,datum,nors):
        """
        this program converts universal transverse meractor coordinates to gps
        converted from fortran by r. indebetouw jan 2009.
        ri also added other datums and ellipsoids in a helper function
        
        header from original UTMS fortran program:
        *     this program was originally written by e. carlson
        *     subroutines tmgrid, tconst, tmgeod, tconpc,
        *     were written by t. vincenty, ngs, in july 1984 .
        *     the orginal program was written in september of 1988.
        *
        *     this program was updated on febuary 16, 1989.  the update was
        *     having the option of saving and *81* record file.
        *
        *
        *     this program was updated on april 3, 1990.  the following update
        *     were made:
        *      1. change from just a choice of nad27 of nad83 reference
        *         ellipsoids to; clarke 1866, grs80/wgs84, international, and
        *         allow for user defined other.
        *      2. allow use of latitudes in southern hemisphere and longitudes
        *         up to 360 degrees west.
        *
        *     this program was updated on december 1, 1993. the following update
        *     was made:
        *      1. the northings would compute the right latitude in the southern
        *         hemishpere.
        *      2. the computed latitude on longidutes would be either  in e or w.
        *
        *****************************************************************     *
        *                  disclaimer                                         *
        *                                                                     *
        *   this program and supporting information is furnished by the       *
        * government of the united states of america, and is accepted and     *
        * used by the recipient with the understanding that the united states *
        * government makes no warranties, express or implied, concerning the  *
        * accuracy, completeness, reliability, or suitability of this         *
        * program, of its constituent parts, or of any supporting data.       *
        *                                                                     *
        *   the government of the united states of america shall be under no  *
        * liability whatsoever resulting from any use of this program.  this  *
        * program should not be relied upon as the sole basis for solving a   *
        * problem whose incorrect solution could result in injury to person   *
        * or property.                                                        *
        *                                                                     *
        *   this program is property of the government of the united states   *
        * of america.  therefore, the recipient further agrees not to assert  *
        * proprietary rights therein and not to represent this program to     *
        * anyone as being other than a government program.                    *
        *                                                                     *
        ***********************************************************************
        
        this is the driver program to compute latitudes and longitudes from
        the utms for each zone
        
        input:
        northing, easting
        zone, datum
        nors=N/S
        
        determined according to datum:
        er = equatorial radius of the ellipsoid (semi-major axis)
        rf = reciprocal of flatting of the ellipsod
        f  =
        esq= e squared
        
        calculated according to longitude and zone:
        rad = radian conversion factor
        cm = central meridian ( computed using the longitude)
        sf = scale factor of central meridian ( always .9996 for utm)
        orlim = southernmost parallel of latitude ( always zero for utm)
        r, a, b, c, u, v, w = ellipsoid constants used for computing
        meridional distance from latitude
        so = meridional distance (multiplied by scale factor )
        from the equator to the southernmost parallel of latitude
        ( always zero for utm)
        
        """
        
        rad=180./pl.pi
        
        offx,offy,offz,er,rf = self.getdatum(datum,verbose=self.verbose)

        f=1./rf
        esq=(2*f-f*f)
        
        # find the central meridian if the zone number is less than 30
        
        if zone < 30 :   # ie W long - this code uses W=positive lon
            iz=zone
            icm=(183-(6*iz))
            cm=float(icm)/rad
            ucm=(icm+3)/rad
            lcm=(icm-3)/rad
        else:
            iz=zone
            icm=(543 - (6*iz))
            cm= float(icm)/rad
            ucm=(icm+3)/rad
            lcm=(icm-3)/rad
            
        tol=(5./60.)/rad
        
        if nors == 'S':
            fn= 10000000.
        else:
            fn=0.
    
        fe=500000.0
        sf=0.9996
        orlim=0.0
        
        found=0

        # precompute parameters for this zone:
        eps,r,so,v0,v2,v4,v6 = self.tconpc(sf,orlim,er,esq,rf)
        
        # compute the latitudes and longitudes:
        lat,lon,conv,kp = self.tmgeod(north,east,eps,cm,fe,sf,so,r,v0,v2,v4,v6,fn,er,esq)
        
        # do the test to see if the longitude is within 5 minutes
        # of the boundaries for the zone and if so	compute	the
        # north and easting for the adjacent zone
        
        # if abs(ucm-lam) < tol:
        #     cm=float(icm+6)/rad
        #     iz=iz-1
        #     if iz == 0:
        #         iz=60
        #     found=found+1
        #     lat,lon,conv,kp = tmgeod(n,e,eps,cm,fe,sf,so,r,v0,v2,v4,v6,fn,er,esq)
        # 
        # if abs(lcm-lam) < tol:
        #     cm=float(icm-6)/rad
        #     iz=iz+1
        #     if iz == 61:
        #         iz=1
        #     lat,lon,conv,kp = tmgeod(n,e,eps,cm,fe,sf,so,r,v0,v2,v4,v6,fn,er,esq)
        #     found=found+1
        
        # *** convert to more usual convention of negative lon = W
        lon=-lon

        if self.verbose:
            self.msg(" longitude, latitude = %s %s; conv,kp = %f,%f" % (qa.angle(qa.quantity(lon,"rad"),prec=8),qa.angle(qa.quantity(lat,"rad"),prec=8),conv,kp),origin="utm2long")
        
        return lon,lat



    
    def long2xyz(self,long,lat,elevation,datum):
        
        dx,dy,dz,er,rf = self.getdatum(datum,verbose=False)

        f=1./rf
        esq=2*f-f**2    
        nu=er/(1.-esq*(pl.sin(lat))**2)
        
        x=(nu+elevation)*pl.cos(lat)*pl.cos(long) +dx
        y=(nu+elevation)*pl.cos(lat)*pl.sin(long) +dy
        z=((1.-esq)*nu+elevation)*pl.sin(lat)  +dz
        
        return x,y,z
    
    
    def xyz2long(self,x,y,z,datum):
        
        dx,dy,dz,er,rf = self.getdatum(datum,verbose=False)
        
        f=1./rf

        b= ((x-dx)**2 + (y-dy)**2) / er**2
        c= (z-dx)**2 / er**2

        esq=2*f-f**2 # (a2-b2)/a2

        a0=c*(1-esq)
        a1=2*a0
        efth=esq**2
        a2=a0+b-efth
        a3=-2.*efth
        a4=-efth

        b0=4.*a0
        b1=3.*a1
        b2=2.*a2
        b3=a3

        # refine/calculate esq
        nlqk=esq
        for i in range(3):
            nlqks = nlqk * nlqk
            nlqkc = nlqk * nlqks
            nlf = a0*nlqks*nlqks + a1*nlqkc + a2*nlqks + a3*nlqk + a4
            nlfprm = b0*nlqkc + b1*nlqks + b2*nlqk + b3
            nlqk = nlqk - (nlf / nlfprm)

        y0 = (1.+nlqk)*(z-dz)
        x0 = pl.sqrt((x-dx)**2 + (y-dy)**2)
        lat=pl.arctan2(y0,x0)
        lon=pl.arctan2(y-dy,x-dx)
        #print x-dx,y-dy,z-dz,x0,y0
                
        return lon,lat


    def utm2xyz(self,easting,northing,elevation,zone,datum,nors):

        lon,lat = self.utm2long(easting,northing,zone,datum,nors)
        x,y,z = self.long2xyz(lon,lat,elevation,datum)

        return x,y,z



    def locxyz2itrf(self, lat, longitude, locx=0.0, locy=0.0, locz=0.0):
        """
        Returns the nominal ITRF (X, Y, Z) coordinates (m) for a point at "local"
        (x, y, z) (m) measured at geodetic latitude lat and longitude longitude
        (degrees).  The ITRF frame used is not the official ITRF, just a right
        handed Cartesian system with X going through 0 latitude and 0 longitude,
        and Z going through the north pole.  The "local" (x, y, z) are measured
        relative to the closest point to (lat, longitude) on the WGS84 reference
        ellipsoid, with z normal to the ellipsoid and y pointing north.
        """
        # from Rob Reid;  need to generalize to use any datum...
        phi, lmbda = map(pl.radians, (lat, longitude))
        sphi = pl.sin(phi)
        a = 6378137.0      # WGS84 equatorial semimajor axis
        b = 6356752.3142   # WGS84 polar semimajor axis
        ae = pl.arccos(b / a)
        N = a / pl.sqrt(1.0 - (pl.sin(ae) * sphi)**2)
    
        # Now you see the connection between the Old Ones and Antarctica...
        Nploczcphimlocysphi = (N + locz) * pl.cos(phi) - locy * sphi
    
        clmb = pl.cos(lmbda)
        slmb = pl.sin(lmbda)
    
        x = Nploczcphimlocysphi * clmb - locx * slmb
        y = Nploczcphimlocysphi * slmb + locx * clmb
        z = (N * (b / a)**2 + locz) * sphi + locy * pl.cos(phi)
    
        return x, y, z




    def irtf2loc(self, x,y,z, cx,cy,cz):
        """
        itrf xyz and COFA cx,cy,cz -> latlon WGS84
        """
        clon,clat = self.xyz2long(cx,cy,cz,'WGS84')
        ccoslon=pl.cos(clon)
        csinlon=pl.sin(clon)        
        csinlat=pl.sin(clat)
        n=x.__len__()
        lat=pl.zeros(n)
        lon=pl.zeros(n)

        # do like MsPlotConvert
        for i in range(n):
            # translate w/o rotating:
            xtrans=x[i]-cx
            ytrans=y[i]-cy
            ztrans=z[i]-cz
            # rotate
            lat[i] = (-csinlon*xtrans) + (ccoslon*ytrans)
            lon[i] = (-csinlat*ccoslon*xtrans) - (csinlat*csinlon*ytrans) + ztrans
                
        return lat,lon



    def plotants(self,x,y,z,d,name):
        # given globals
        
        #stnx, stny, stnz, stnd, nant, telescopename = util.readantenna(antennalist)
        cx=pl.mean(x)
        cy=pl.mean(y)
        cz=pl.mean(z)
        lat,lon = self.irtf2loc(x,y,z,cx,cy,cz)
        n=lat.__len__()
        
        dolam=0
        # TODO convert to klam: (d too)
        ###
                
        ra=max(lat)-min(lat)
        r2=max(lon)-min(lon)
        if r2>range:
            ra=r2
        if max(d)>0.01*ra:
            pl.plot(lat,lon,',')            
            #print max(d),ra
            for i in range(n):
                pl.gca().add_patch(pl.Circle((lat[i],lon[i]),radius=0.5*d[i],fc="#dddd66"))
                pl.text(lat[i],lon[i],name[i],horizontalalignment='center',verticalalignment='center')
        else:
            pl.plot(lat,lon,'o',c="#dddd66")
            for i in range(n):
                pl.text(lat[i],lon[i],name[i],horizontalalignment='center',fontsize=8)

        #if dolam:
        #    pl.xlabel("kilolamda")
        #    pl.ylabel("kilolamda")

























    ##################################################################
    # fit modelimage into a 4 coordinate image defined by the parameters
    def image4d(self, inimage, outimage, 
                inbright,ignorecoord,
                ra,dec,cell,startfreq,chanwidth, # only used if ignorecoord
                flatimage=""):  # if nonzero, create mom -1 image named this

        # *** ra,dec,cell,freq,width are expected to be quantities

        in_ia=ia.newimagefromfile(inimage)            
        in_shape=in_ia.shape()
        in_csys=in_ia.coordsys()

        if ignorecoord:
            if type(cell) == type([]):
                model_cell =  map(qa.convert,cell,['arcsec','arcsec'])
            else:
                model_cell = qa.convert(cell,'arcsec')            
                model_cell = [model_cell,model_cell]
        else:
            # get pixel size from model image CoordSys
            increments=in_csys.increment(type="direction")['numeric']
            incellx=qa.quantity(abs(increments[0]),in_csys.units(type="direction")[0])
            incelly=qa.quantity(abs(increments[1]),in_csys.units(type="direction")[1])
            xform=in_csys.lineartransform(type="direction")
            offdiag=max(abs(xform[0,1]),abs(xform[1,0]))
            if offdiag > 1e-4:
                self.msg("Your image is rotated with respect to Lat/Lon.  I can't cope with that yet",priority="error")
            incellx=qa.mul(incellx,abs(xform[0,0]))
            incelly=qa.mul(incelly,abs(xform[1,1]))

#            # warn if input pixels are not square
#            if (abs(incellx['value'])-abs(incelly['value']))/abs(incellx['value']) > 0.001 and not ignorecoord:
#                self.msg("input pixels are not square!",priority="warn",origin="setup model")
#                if self.verbose:
#                    self.msg("using cell = %s (not %s)" % (str(incellx),str(incelly)),origin="setup model")
#                else:
#                    self.msg("using cell = %s" % str(incellx),origin="setup model")
            model_cell = [qa.convert(incellx,'arcsec'),qa.convert(incelly,'arcsec')]

        if self.verbose:
            self.msg("model image shape= %s" % in_shape,origin="setup model")
            self.msg("model pixel size = %8.2e x %8.2e arcsec" % (model_cell[0]['value'],model_cell[1]['value']),origin="setup model")



        # deal with brightness scaling
        if (inbright=="unchanged") or (inbright=="default"):
            scalefactor=1.
        else:
            stats=in_ia.statistics(verbose=False,list=False)
            highvalue=stats['max']
            scalefactor=float(inbright)/highvalue.max()


        # check shape characteristics of the input;
        # add degenerate axes as neeed:

        in_dir=in_csys.findcoordinate("direction")
        in_spc=in_csys.findcoordinate("spectral")
        in_stk=in_csys.findcoordinate("stokes")


        if self.verbose: self.msg("rearranging input data (may take some time for large cubes)")
        arr=in_ia.getchunk()
        axmap=[-1,-1,-1,-1]
        axassigned=[-1,-1,-1,-1]

        in_nax=arr.shape.__len__()
        if in_nax<2:
            self.msg("Your input model has fewer than 2 dimensions.  Can't proceed",priority="error")
            return False



        # we have at least two axes:

        # set model_refdir and model_cell according to ignorecoord
        if ignorecoord:
            if self.verbose: self.msg("setting model image direction to ra="+qa.angle(qa.div(ra,"15"))+" dec="+qa.angle(dec),origin="setup model")
            
            model_refdir='J2000 '+qa.formxxx(ra,format='hms')+" "+qa.formxxx(dec,format='dms')
            axmap[0]=0 # direction in first two pixel axes
            axmap[1]=1
            axassigned[0]=0  # coordinate corresponding to first 2 pixel axes
            axassigned[1]=0
             
        else:  # get model_refdir from CoordSys:
            if not in_dir['return']:
                self.msg("You don't have direction coordinates that I can understand, so either edit the header or set ignorecoord=True",priority="error")
                return False            
            ra,dec = in_csys.referencevalue(type="direction")['numeric']
            model_refdir= in_csys.referencecode(type="direction")+" "+qa.formxxx(str(ra)+"rad",format='hms')+" "+qa.formxxx(str(dec)+"rad",format='dms')
            ra=qa.quantity(str(ra)+"rad")
            dec=qa.quantity(str(dec)+"rad")
            if in_dir['pixel'].__len__() != 2:
                self.msg("I can't understand your direction coordinates, so either edit the header or set ignorecoord=True",priority="error")
                return False            
            dirax=in_dir['pixel']
            axmap[0]=dirax[0]
            axmap[1]=dirax[1]                    
            axassigned[dirax[0]]=0
            axassigned[dirax[1]]=0
            if self.verbose: self.msg("Direction coordinate (%i,%i) parsed" % (axmap[0],axmap[1]),origin="setup model")

        # if we only have 2d to start with:
        if in_nax==2:            
            nchan=1
            # add an extra axis to be Spectral:
            arr=arr.reshape([arr.shape[0],arr.shape[1],1])
            in_shape=arr.shape
            in_nax=in_shape.__len__() # which should be 3
            if self.verbose: self.msg("Adding degenerate spectral axis",origin="setup model")

        # we now have at least 3 axes, either by design or by addition:
        if ignorecoord:
            add_spectral_coord=True
            extra_axis=2
        else:
            if in_spc['return']:
                if type(in_spc['pixel']) == type(1) :
                    foo=in_spc['pixel']
                else:
                    foo=in_spc['pixel'][0]
                    self.msg("you seem to have two spectral axes",priority="warn")
                nchan=arr.shape[foo]                
                axmap[3]=foo
                axassigned[foo]=3
                model_restfreq=in_csys.restfrequency()
                in_startpix=in_csys.referencepixel(type="spectral")['numeric'][0]
                model_step=in_csys.increment(type="spectral")['numeric'][0]
                model_start=in_csys.referencevalue(type="spectral")['numeric'][0]-in_startpix*model_step
                model_step=str(model_step)+in_csys.units(type="spectral")
                model_start=str(model_start)+in_csys.units(type="spectral")
                add_spectral_coord=False
                if self.verbose: self.msg("Spectral Coordinate %i parsed" % axmap[3],origin="setup model")                
            else:
                # we're not ignoreing coord, but we have at least one extra axis
                # that isn't a spectral axis.                
                if in_stk['return']:
                    # we have a valid stokes axis:
                    axassigned[in_stk['pixel']]=2
                    axmap[2]=in_stk['pixel']
                    # AND, if we only had 3 axes (this was the only extra one), 
                    # we need to add a degenerate spectral:
                    if in_nax<4:
                        nchan=1
                        arr=arr.reshape([arr.shape[0],arr.shape[1],arr.shape[2],1])
                        in_shape=arr.shape
                        in_nax=in_shape.__len__() # which should be 4
                        if self.verbose: self.msg("Adding degenerate spectral axis",origin="setup model")
                        
                # find first unused axis - probably at end, but just in case its not:
                i=0
                extra_axis=-1
                while extra_axis<0 and i<4:
                    if axassigned[i]<0: extra_axis=i
                    i+=1
                if extra_axis<0:                    
                    self.msg("I can't find an unused axis to make Spectral [%i %i %i %i] " % (axassigned[0],axassigned[1],axassigned[2],axassigned[3]),priority="error",origin="setup model")
                    return False
                add_spectral_coord=True
                
        if add_spectral_coord:
            axmap[3]=extra_axis
            axassigned[extra_axis]=3
            nchan=arr.shape[extra_axis]
            model_restfreq=startfreq
            model_start=startfreq
            model_step=chanwidth  # user responsibility to have chanwidth=BW
            if self.verbose: self.msg("Adding Spectral Coordinate",origin="setup model")



        # if we only have three axes, add one to be Stokes:
        if in_nax==3:
            arr=arr.reshape([arr.shape[0],arr.shape[1],arr.shape[2],1])
            in_shape=arr.shape
            in_nax=in_shape.__len__() # which should be 4
            add_stokes_coord=True
            extra_axis=3
            if self.verbose: self.msg("Adding degenerate Stokes axis",origin="setup model")
            
        # we have at least 3 axes, either by design or by addition:
        if ignorecoord:
            add_stokes_coord=True
            extra_axis=3
        else:
            if in_stk['return']:
                model_stokes=in_csys.stokes()
                foo=model_stokes[0]
                out_nstk=model_stokes.__len__()
                for i in range(out_nstk-1):
                    foo=foo+model_stokes[i+1]
                model_stokes=foo
                if type(in_stk['pixel']) == type(1):
                    foo=in_stk['pixel']
                else:
                    foo=in_stk['pixel'][0]
                    self.msg("you seem to have two stokes axes",priority="warn")                
                axmap[2]=foo
                axassigned[foo]=2
                if in_shape[foo]>4:
                    self.msg("you appear to have more than 4 Stokes components - please edit your header and/or parameters",priority="error")
                    return False                        
                add_stokes_coord=False
                if self.verbose: self.msg("Stokes Coordinate %i parsed" % axmap[2],origin="setup model")
            else:
                # find the unused axis:
                i=0
                extra_axis=-1
                while extra_axis<0 and i<4:
                    if axassigned[i]<0: extra_axis=i
                    i+=1
                if extra_axis<0:
                    self.msg("I can't find an unused axis to make Stokes [%i %i %i %i] " % (axassigned[0],axassigned[1],axassigned[2],axassigned[3]),priority="error",origin="setup model")
                    return False
                add_stokes_coord=True
                            

        if add_stokes_coord:
            axmap[2]=extra_axis
            axassigned[extra_axis]=2
            if arr.shape[extra_axis]>4:
                self.msg("you have %i Stokes parameters in your potential Stokes axis %i.  something is wrong." % (arr.shape[extra_axis],extra_axis),priority="error")
                return False
            if self.verbose: self.msg("Adding Stokes Coordinate",origin="setup model")
            if arr.shape[extra_axis]==4:                    
                model_stokes="IQUV"
            if arr.shape[extra_axis]==3:                    
                model_stokes="IQV"
                self.msg("setting IQV Stokes parameters from the 4th axis of you model.  If that's not what you want, then edit the header",origin="setup model",priority="warn")
            if arr.shape[extra_axis]==2:                    
                model_stokes="IQ"
                self.msg("setting IQ Stokes parameters from the 4th axis of you model.  If that's not what you want, then edit the header",origin="setup model",priority="warn")
            if arr.shape[extra_axis]<=1:                    
                model_stokes="I"
            out_nstk=len(model_stokes)

        if self.verbose:
            self.msg("axis map for model image = %i %i %i %i" %
                     (axmap[0],axmap[1],axmap[2],axmap[3]),origin="setup model")

        modelshape=[in_shape[axmap[0]], in_shape[axmap[1]],out_nstk,nchan]
        ia.fromshape(outimage,modelshape,overwrite=True)
        modelcsys=ia.coordsys()        
        modelcsys.setunits(['rad','rad','','Hz'])
        modelcsys.setincrement([-1*qa.convert(model_cell[0],modelcsys.units()[0])['value'],
                                qa.convert(model_cell[1],modelcsys.units()[1])['value']],
                                type="direction")
        # setting both increment and lintransform does bad things.
        #modelcsys.setlineartransform("direction",
        #                             pl.array([[-1*qa.convert(model_cell,modelcsys.units()[0])['value'],0.],
        #                                       [0.,qa.convert(model_cell,modelcsys.units()[1])['value']]]))
        dirm=self.dir_s2m(model_refdir)
        raq=dirm['m0']        
        deq=dirm['m1']        
        modelcsys.setreferencevalue(
            [qa.convert(raq,modelcsys.units()[0])['value'],
             qa.convert(deq,modelcsys.units()[1])['value']],
            type="direction")
        modelcsys.setreferencepixel(
            [0.5*in_shape[axmap[0]],0.5*in_shape[axmap[1]]],
            "direction")
        modelcsys.setspectral(refcode="LSRK",restfreq=model_restfreq)
        modelcsys.setreferencevalue(model_start,type="spectral")
        modelcsys.setreferencepixel(0,type="spectral") # default is middle chan
        modelcsys.setincrement(qa.convert(model_step,modelcsys.units()[3])['value'],type="spectral")
        #modelcsys.summary()

        # first assure that the csys has the expected order 
        expected=['Direction', 'Direction', 'Stokes', 'Spectral']
        if modelcsys.axiscoordinatetypes() != expected:
            self.msg("internal error with coordinate axis order created by Imager",priority="error")
            self.msg(modelcsys.axiscoordinatetypes().__str__(),priority="error")
            return False

        # more checks:
        foo=pl.array(modelshape)
        if not (pl.array(arr.shape) == pl.array(foo.take(axmap).tolist())).all():
            self.msg("internal error: I'm confused about the shape if your model data cube",priority="error")
            self.msg("have "+foo.take(axmap).__str__()+", want "+in_shape.__str__(),priority="error")
            return False

        ia.setcoordsys(modelcsys.torecord())
        ia.done()
        ia.open(outimage)


        for ax in range(4):
            if axmap[ax] != ax:
                if self.verbose: self.msg("swapping input axes %i with %i" % (ax,axmap[ax]),origin="setup model")
                arr=arr.swapaxes(ax,axmap[ax])                        
                tmp=axmap[ax]
                axmap[ax]=ax
                axmap[tmp]=tmp                


        # there's got to be a better way to remove NaNs: :)
        for i0 in range(arr.shape[0]):
            for i1 in range(arr.shape[1]):
                for i2 in range(arr.shape[2]):
                    for i3 in range(arr.shape[3]):
                        foo=arr[i0,i1,i2,i3]
                        if foo!=foo: arr[i0,i1,i2,i3]=0.0

        if self.verbose:
            self.msg("model array minmax= %e %e" % (arr.min(),arr.max()),origin="setup model")        
            self.msg("scaling model brightness by a factor of %f" % scalefactor,origin="setup model")
            self.msg("image channel width = %8.2e GHz" % qa.convert(model_step,'GHz')['value'],origin="setup model")
            if arr.nbytes > 5e7:
                msg("your model is large - predicting visibilities may take a while.",priority="warn")


        ia.putchunk(arr*scalefactor)
        ia.close()
        in_ia.close()

        # coord image should now have correct Coordsys and shape


        #####################################################################
        # make a moment 0 image
        if flatimage != "":
            
            inspectax=modelcsys.findcoordinate('spectral')['pixel']
            innchan=modelshape[inspectax]
            
            stokesax=modelcsys.findcoordinate('stokes')['pixel']
            innstokes=modelshape[stokesax]

            if innchan>1:
                if self.verbose: self.msg("creating moment zero input image",origin="setup model")
                # actually run ia.moments
                ia.open(outimage)
                ia.moments(moments=[-1],outfile=flatimage,overwrite=True)
                ia.done()
            else:            
                if self.verbose: self.msg("removing degenerate input image axes",origin="setup model")
                # just remove degenerate axes from modelimage4d
                ia.newimagefromimage(infile=outimage,outfile=flatimage,dropdeg=True,overwrite=True)
                if innstokes<=1:
                    os.rename(flatimage,flatimage+".tmp")
                    ia.open(flatimage+".tmp")
                    ia.adddegaxes(outfile=flatimage,stokes='I',overwrite=True)
                    ia.done()
                    shutil.rmtree(flatimage+".tmp")
            if innstokes>1:
                os.rename(flatimage,flatimage+".tmp")
                po.open(flatimage+".tmp")
                foo=po.stokesi(outfile=flatimage,stokes='I')
                foo.done()
                po.done()
                shutil.rmtree(flatimage+".tmp")

        return ra,dec,model_cell,nchan,model_start,model_step,model_stokes


