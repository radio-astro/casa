# geodesy and pointing and other helper functions that are useful
# to be available outside of the simdata task
import casac
import os
import commands
import pdb
# all I really need is casalog, but how to get it:?
from taskinit import *
import pylab as pl
qatool = casac.homefinder.find_home_by_name('quantaHome')
qa = qatool.create()

class simutil:
    #def __init__(self, direction="", totaltime=qa.quantity("0h"), verbose=False):
    #    self.direction=direction
    #    self.verbose=verbose
    #    self.totaltime=totaltime

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

    def msg(self, s, origin=None, color=None):
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
        if color==None:
            clr="\x1b[32m"
        else:
            clr="\x1b["+color+"m"
        bw="\x1b[0m"
        if origin==None:
            origin="simutil"
        print clr+"["+origin+"] "+bw+s
        casalog.post("")
        casalog.post(s)


    def mul_scal_qa(self, scal, q):
        """
        Returns the product of a dimensionless number scal and quantity q.
        """
        return qa.quantity(scal * q['value'], q['unit'])
    

    def calc_pointings(self, spacing, imsize, cell, direction=None, relmargin = 0.5):
        """
        If direction is a list, simply returns direction and the number of
        pointings in it.
        
        Otherwise, returns a hexagonally packed list of pointings separated by
        spacing and fitting inside an image specified by direction, imsize and
        cell, and the number of pointings.  The hexagonal packing starts with a
        horizontal row centered on direction, and the other rows alternate
        being horizontally offset by a half spacing.  All of the pointings will
        be within a rectangle relmargin * spacing smaller than the image on all
        sides.
        """
        if type(imsize) != list:
            imsize=[imsize,imsize]
        if direction==None:
            # if no direction is specified, use the object's direction
            direction=self.direction
        else:
            # if one is specified, use that to set the object's direction
            # do we really want to do this?
            self.direction=direction
        if type(direction) == list:
            if len(direction) >1:
                return len(direction), direction
            else:
                direction=direction[0]
    
        epoch, centx, centy = self.direction_splitter()

        spacing  = qa.quantity(spacing)
        yspacing = self.mul_scal_qa(0.866025404, spacing)
    
        if type(cell) == list:
            cellx, celly = map(qa.quantity, cell)
        else:
            cellx = qa.quantity(cell)
            celly = cellx
        
        ysize = self.mul_scal_qa(imsize[1], celly)
        nrows = 1+ int(pl.floor(qa.convert(qa.div(ysize, yspacing), '')['value']
                                - 2.309401077 * relmargin))

        xsize = self.mul_scal_qa(imsize[0], cellx)
        availcols = 1 + qa.convert(qa.div(xsize, spacing),
                                   '')['value'] - 2.0 * relmargin
        ncols = int(pl.floor(availcols))

        # By making the even rows shifted spacing/2 ahead, and possibly shorter,
        # the top and bottom rows (nrows odd), are guaranteed to be short.
        if availcols - ncols >= 0.5:                                # O O O
            evencols = ncols                                    #  O O O
            ncolstomin = 0.5 * (ncols - 0.5)
        else:
            evencols = ncols - 1                                #  O O 
            ncolstomin = 0.5 * (ncols - 1)                      # O O O
        pointings = []

        # Start from the top because in the Southern hemisphere it sets first.
        y = qa.add(centy, self.mul_scal_qa(0.5 * (nrows - 1), yspacing))
        for row in xrange(0, nrows):         # xrange stops early.
            xspacing = self.mul_scal_qa(1.0 / pl.cos(qa.convert(y, 'rad')['value']),
                                   spacing)
            ystr = qa.formxxx(y, format='dms')
        
            if row % 2:                             # Odd
                xmin = qa.sub(centx, self.mul_scal_qa(ncolstomin, xspacing))
                stopcolp1 = ncols
            else:                                   # Even (including 0)
                xmin = qa.sub(centx, self.mul_scal_qa(ncolstomin - 0.5,
                                                 xspacing))
                stopcolp1 = evencols
            for col in xrange(0, stopcolp1):        # xrange stops early.
                x = qa.formxxx(qa.add(xmin, self.mul_scal_qa(col, xspacing)),
                               format='hms')
                pointings.append("%s%s %s" % (epoch, x, ystr))
            y = qa.sub(y, yspacing)
        ####could not fit pointings then single pointing
        if(len(pointings)==0):
            pointings.append(direction)
        self.msg("Using %i generated pointing(s)" % len(pointings))
        self.pointings=pointings
        return len(pointings), pointings



    def average_direction(self, directions=None):
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
        for drn in directions:
            epoch, x, y = self.direction_splitter(drn)
            x = x['value']
            y = y['value']
            x = self.wrapang(x, avgx, 360.0)
            offsets[:,i]=[x-avgx,y-avgy]
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
        if type(direction) == list:
            direction=self.average_direction(direction)[0]
        dirl = direction.split()
        if len(dirl) == 3:
            epoch = dirl[0] + ' '
        else:
            epoch = ''
        x, y = map(qa.toangle, dirl[-2:])
        return epoch, qa.convert(x, 'deg'), qa.convert(y, 'deg')


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
        if self.verbose: self.msg("subreflector diameter="+str(diam_subreflector))

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
        if self.verbose: self.msg("ruze phase efficiency for surface accuracy of "+str(epsilon)+"um = " + str(eta_p))

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
        else:
            if telescope=='EVLA':
                # these are T_Rx/epsilon so may be incorrect
                # http://www.vla.nrao.edu/astro/guides/vlas/current/node11.html
                # http://www.vla.nrao.edu/astro/guides/evlareturn/stress_tests/L-Band/
                f0=[0.33,1.47,4.89,8.44,22.5,33.5,43.3]
                t0=[500, 70,  60,  55,  100, 130, 350]
                flim=[0.305,50]
            else:
                if telescope=='VLA':
                    # http://www.vla.nrao.edu/genpub/overview/
                    # exclude P band for now
                    # f0=[0.0735,0.32, 1.5, 4.75, 8.4, 14.9, 23, 45 ]
                    # t0=[5000,  165,  56,  44,   34,  110, 110, 110]
                    f0=[0.32, 1.5, 4.75, 8.4, 14.9, 23, 45 ]
                    t0=[165,  56,  44,   34,  110, 110, 110]
                    flim=[0.305,50]
                else:
                    self.msg("I don't know about the "+telescope+" receivers, using 200K",color="31")
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
            self.msg("observing freqency is lower than expected for "+telescope,color="31")
            self.msg("proceeding with extrapolated receiver temp="+str(t_rx),color="31")
        if obsfreq>flim[1]:
            self.msg("observing freqency is higher than expected for "+telescope,color="31")
            self.msg("proceeding with extrapolated receiver temp="+str(t_rx),color="31")

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
        
#        # XXX cludge because ACA isn't in the data repo yet
#        if telescope=="ACA":
#            telescope="ALMA"
#        # XXX

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
        
        rise=me.measure(me.riseset(src)['rise']['utc'],'tai')
        set=me.measure(me.riseset(src)['set']['utc'],'tai')
        
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
        pl.ylim([0,90])

        if self.totaltime>0:
            etimeh=qa.convert(self.totaltime,'h')['value']
            pl.plot(pl.array([-0.5,0.5])*etimeh+(peak-starttime_float)*24,[80,80],'r')



















    #=========================================================================
    
    def readantenna(self, antab=None):
    ###Helper function to read 4 columns text antenna table X, Y, Z, Diam
        f=open(antab)
        line= '  '
        inx=[]
        iny=[]
        inz=[]
        ind=[]
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
                ### all coord systems should have x,y,z,diam, where xyz varies
                    #print line.split()
                    if(len(line.split()) >3):
                        splitline=line.split()
                        inx.append(float(splitline[0]))
                        iny.append(float(splitline[1]))
                        inz.append(float(splitline[2]))
                        ind.append(float(splitline[3]))
                        nant+=1                 
            except:
                break
        f.close()

        if not params.has_key("observatory"):
            self.msg("Must specify observatory in antenna file",origin="readantenna",color="31")
            return -1
        else:
            self.telescopename=params["observatory"]
            if self.verbose:
                self.msg("Using observatory= %s" % params["observatory"],origin="readantenna")

        if not params.has_key("coordsys"):
            self.msg("Must specify coordinate system #coorsys=XYZ|UTM|GEO in antenna file",origin="readantenna",color="31")
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
                self.msg("Antenna locations in UTM, expecting easting, northing, elevation in m",origin="readant") 
                if params.has_key("zone"):
                    zone=params["zone"]
                else:
                    self.msg("You must specify zone=NN in your antenna file",origin="readant",color="31")
                    return -1
                if params.has_key("datum"):
                    datum=params["datum"]
                else:
                    self.msg("You must specify datum in your antenna file",origin="readant",color="31")
                    return -1
                if params.has_key("hemisphere"):
                    nors=params["hemisphere"]
                    nors=nors[0].upper()
                else:
                    self.msg("You must specify hemisphere=N|S in your antenna file",origin="readant",color="31")
                    return -1
                
                if self.verbose: foo=self.getdatum(datum,verbose=True)
                for i in range(len(inx)):
                    x,y,z = self.utm2xyz(inx[i],iny[i],inz[i],int(zone),datum,nors)
                    stnx.append(x)
                    stny.append(y)
                    stnz.append(z)                
            else:
                if (params["coordsys"].upper()[0:3]=="LOC"):
                    # I'm pretty sure Rob's function only works with lat,lon in degrees;
#                    if params["observatory"]=="ACA":
#                        ## cludge because ACA isn't in the data repo yet
#                        obs=me.measure(me.observatory("ALMA"),'WGS84')
#                    else:
                    obs=me.measure(me.observatory(params["observatory"]),'WGS84')
                    obslat=qa.convert(obs['m1'],'deg')['value']
                    obslon=qa.convert(obs['m0'],'deg')['value']
                    obsalt=qa.convert(obs['m2'],'m')['value']
                    if self.verbose:
                        self.msg("converting local tangent plane coordinates to ITRF using observatory position = %d %d " % (obslat,obslon))
                        #foo=self.getdatum(datum,verbose=True)
                    for i in range(len(inx)):
                        x,y,z = self.locxyz2itrf(inx[i],iny[i],inz[i]+obsalt,obslat,obslon)
                        stnx.append(x)
                        stny.append(y)
                        stnz.append(z)                
                else:
                    if (params["coordsys"].upper()[0:3]=="GEO"):
                        if params.has_key("datum"):
                            datum=params["datum"]
                        else:
                            self.msg("You must specify zone=NN in your antenna file",origin="readant",color="31")
                            return -1
                        if (datum.upper() != "WGS84"):
                            self.msg("Unfortunately I only can deal with WGS84 right now",origin="readant",color="31")
                            return -1
                        self.msg("geodetic coordinates not implemented yet :(",color="31")
                    
        return (stnx, stny, stnz, pl.array(ind), nant, params["observatory"])



















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
            self.msg("I can't figure out what datum %s is" % datumcode,color="31")
            return -1
        
        datum=datums[datumcode]
        ellipsoid=datum[3]
        
        if not ellipsoids.has_key(ellipsoid):
            self.msg("I can't figure out what ellipsoid %s is" % ellipsoid,color="31")
            return -1
        
        if self.verbose:
            self.msg("Using %s datum with %s ellipsoid" % (datum[4],ellipsoids[ellipsoid][2]))
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
        
        offx,offy,offz,er,rf = self.getdatum(datum)

        f=1./rf
        esq=(2*f-f*f)
        
        # find the central meridian if the zone number is less than 30
        
        if zone < 30 :
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
        
        if self.verbose:
            self.msg(" longitude, latitude = %s %s; conv,kp = %f,%f" % (qa.angle(qa.quantity(lon,"rad"),prec=8),qa.angle(qa.quantity(lat,"rad"),prec=8),conv,kp),origin="utm2long")
        
        return lon,lat



    
    def long2xyz(self,long,lat,elevation,datum):
        
        dx,dy,dz,er,rf = self.getdatum(datum)
        
        f=1./rf
        esq=2*f-f**2
        nu=er/(1.-esq*(pl.sin(lat))**2)
        
        x=(nu+elevation)*pl.cos(lat)*pl.cos(long) +dx
        y=(nu+elevation)*pl.cos(lat)*pl.sin(long) +dy
        z=((1.-esq)*nu+elevation)*pl.sin(lat)  +dz
        
        return x,y,z
    
    
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

