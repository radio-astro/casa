# geodesy and pointing and other helper functions that are useful
# to be available outside of the simdata task
import casac
import os
import shutil
import commands
#import pdb
# all I really need is casalog, but how to get it:?
from taskinit import *
import pylab as pl

# qa doesn't hold state.
#qatool = casac.homefinder.find_home_by_name('quantaHome')
#qa = qatool.create()

im, cb, ms, tb, fg, me, ia, po, sm, cl, cs, rg, sl, dc, vp, msmd, fi, fn, imd = gentools()


# functions defined outside of the simutil class
def is_array_type(value):
    array_type = [list, tuple, pl.ndarray]
    if type(value) in array_type:
        return True
    else:
        return False




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
                 centerfreq=qa.quantity("245GHz"),
                 bandwidth=qa.quantity("1GHz"),
                 totaltime=qa.quantity("0h"),
                 verbose=False):
        self.direction=direction
        self.verbose=verbose
        self.centerfreq=centerfreq
        self.bandwidth=bandwidth
        self.totaltime=totaltime
        self.pmulti=0  # rows, cols, currsubplot


    def newfig(self,multi=0,show=True):  # new graphics window/file
        if show:
            pl.ion() # creates a fig if ness
        else:
            pl.ioff() 
        pl.clf()

        if multi!=0:
            if type(multi)!=type([]):
                self.msg("internal error setting multi-panel figure with multi="+str(multi),priority="warn")
            if len(multi)!=3:
                self.msg("internal error setting multi-panel figure with multi="+str(multi),priority="warn")
            self.pmulti=multi
            pl.subplot(multi[0],multi[1],multi[2])
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.09,top=0.95,hspace=0.2,wspace=0.2)


    ###########################################################

    def nextfig(self): # advance subwindow
        ax=pl.gca()
        l=ax.get_xticklabels()
        pl.setp(l,fontsize="x-small")
        l=ax.get_yticklabels()
        pl.setp(l,fontsize="x-small")
        if self.pmulti!=0:
            self.pmulti[2] += 1
            multi=self.pmulti
            if multi[2] <= multi[0]*multi[1]:
                pl.subplot(multi[0],multi[1],multi[2])
        # consider pl.draw() here - may be slow

    ###########################################################

    def endfig(self,show=True,filename=""): # set margins to smaller, save to file if required        
        ax=pl.gca()
        l=ax.get_xticklabels()
        pl.setp(l,fontsize="x-small")
        l=ax.get_yticklabels()
        pl.setp(l,fontsize="x-small")
        if show:
            pl.draw()
        if len(filename)>0:
            pl.savefig(filename)
        pl.ioff()
        

        
    ###########################################################

    def msg(self, s, origin=None, priority=None):
        # everything goes to logger with priority=priority
        # priority error: raise an exception, 
        # priority warn: change color to magenta, send to terminal
        # priority info: change color to green, send to terminal
        # priority none: not normally to terminal unless toterm=True
        # i.e. setting a priority makes it go to terminal

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
        
        clr=""
        if self.verbose:
            toterm=True     
        else:
            toterm=False
    
        if priority==None:
            priority="INFO"
        else:
            priority=priority.upper()
            toterm=True
            if priority=="INFO":
                clr="\x1b[32m"
            elif priority.count("WARN")>0:
                clr="\x1b[35m"                
                toterm=True
                priority="INFO" # otherwise casalog will spew to term also.
            elif priority=="ERROR":                
                clr="\x1b[31m"
                toterm=False  # casalog spews severe to term already
            else:
                if not (priority=="DEBUG" or priority[:-1]=="DEBUG"):
                    priority="INFO"
        bw="\x1b[0m"

        if toterm:
            if self.isreport():
                if origin:
                    self.report.write("["+origin+"] "+s+"\n")
                else:
                    self.report.write(s+"\n")

            if s.count("ERROR"):
                foo=s.split("ERROR")
                s=foo[0]+"\x1b[31mERROR\x1b[0m"+foo[1]
            if s.count("WARNING"):
                foo=s.split("WARNING")
                s=foo[0]+"\x1b[35mWARNING\x1b[0m"+foo[1]

            if origin:
                print clr+"["+origin+"] "+bw+s
            else:
                print s


        if priority=="ERROR":
            raise Exception, s
        else:            
            if origin==None:
                origin="simutil"
            casalog.post(s,priority=priority,origin=origin)


    ###########################################################

    def isreport(self):
        # is there an open report file?
        try:
            if self.report.name == self.reportfile:
                return True
            else:
                return False                
        except:
            return False
    

    def openreport(self):
        try:
            if os.path.exists(self.reportfile):
                self.report=open(self.reportfile,"a")
                #self.msg("Report file "+self.reportfile+"already exists - delete or change reportfile",priority="ERROR",origin="simutil")
            else:
                self.report=open(self.reportfile,"w")
        except:
            self.msg("Can't open reportfile because it's not defined",priority="ERROR",origin="simutil")
               

    def closereport(self):
        self.report.close()
        

    


    ###########################################################

    def isquantity(self,s,halt=True):
        if type(s)!=type([]):
            t=[s]
        else:
            t=s
        for t0 in t:
            if not (len(t0)>0 and qa.isquantity(t0)):
                if halt:
                    self.msg("can't interpret '"+str(t0)+"' as a CASA quantity",priority="error")
                return False
        return True

    ###########################################################

    def isdirection(self,s,halt=True):
        if type(s)==type([]):
            t=s[0]
        else:
            t=s
        try:
            x=self.direction_splitter(s)
            y=me.direction(x[0],x[1],x[2])
        except:
            if halt:
                self.msg("can't interpret '"+str(s)+"' as a direction",priority="error")
            return False
        if not me.measure(y,y['refer']):
            if halt:
                self.msg("can't interpret '"+str(s)+"' as a direction",priority="error")
            return False
        return True

    ###########################################################

    def ismstp(self,s,halt=False):
        try:
            istp = False
            # check if the ms is tp data or not.
            tb.open(s+'/ANTENNA')
            antname = tb.getcol('NAME')
            tb.close()
            if antname[0].find('TP') > -1:
                istp = True
            elif len(antname) == 1:
                istp = True
            else:
                # need complete testing of UVW
                tb.open(s)
                uvw = tb.getcol("UVW")
                tb.close()
                if uvw.all() == 0.:
                    istp = True 
        except:
            if halt:
                self.msg("can't understand the file '"+str(s)+"'",priority="error")
            return False
        if not istp: 
            if halt:
                self.msg("input file '"+str(s)+"' is not a totalpower ms",priority="error")
            return False
        return True
        


    ###########################################################
    # plot an image (optionally), and calculate its statistics

    def statim(self,image,plot=True,incell=None,disprange=None,bar=True,showstats=True):
        pix=self.cellsize(image)  # cell positive by convention
        pixarea=abs(qa.convert(pix[0],'arcsec')['value']*
                    qa.convert(pix[1],'arcsec')['value'])
        ia.open(image)       
        imunit=ia.brightnessunit()
        if imunit == 'Jy/beam':
            bm=ia.restoringbeam()
            if len(bm)>0:
                toJyarcsec=1./pixarea
            else:
                toJyarcsec=1.
            toJypix=toJyarcsec*pixarea
        elif imunit == 'Jy/pixel':
            toJyarcsec=1./pixarea
            toJypix=1.
        else:
            self.msg("%s: unknown units" % image,origin="statim")
            toJyarcsec=1.
            toJypix=1.
        stats=ia.statistics(robust=True,verbose=False,list=False)
        im_min=stats['min']*toJypix
        plarr=pl.zeros(1)
        badim=False
        if type(im_min)==type([]) or type(im_min)==type(plarr):
            if len(im_min)<1: 
                badim=True
                im_min=0.
        im_max=stats['max']*toJypix
        if type(im_max)==type([]) or type(im_max)==type(plarr):
            if len(im_max)<1: 
                badim=True
                im_max=1.
        imsize=ia.shape()[0:2]
        reg1=rg.box([0,0],[imsize[0]*.25,imsize[1]*.25])
        stats=ia.statistics(region=reg1,verbose=False,list=False)
        im_rms=stats['rms']*toJypix
        if type(im_rms)==type([]) or type(im_rms)==type(plarr):
            if len(im_rms)==0: 
                badim=True
                im_rms=0.
        data_array=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],[1],[],True,True,False)
        data_array=pl.array(data_array)
        tdata_array=pl.transpose(data_array)
        ttrans_array=tdata_array.tolist()
        ttrans_array.reverse()
        
        # get and apply mask
        mask_array=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],[1],[],True,True,True)
        mask_array=pl.array(mask_array)
        tmask_array=pl.transpose(mask_array)
        tmtrans_array=tmask_array.tolist()
        tmtrans_array.reverse()
        ttrans_array=pl.array(ttrans_array)
        z=pl.where(pl.array(tmtrans_array)==False)
        ttrans_array[z[0],z[1]]=0.

        if (plot):
            pixsize=[qa.convert(pix[0],'arcsec')['value'],qa.convert(pix[1],'arcsec')['value']]
            xextent=imsize[0]*abs(pixsize[0])*0.5
            yextent=imsize[1]*abs(pixsize[1])*0.5
            if self.verbose: 
                self.msg("plotting %fx%f\" im with %fx%f\" pix" % 
                         (xextent,yextent,pixsize[0],pixsize[1]),origin="statim")
            xextent=[xextent,-xextent]
            yextent=[-yextent,yextent]
        # remove top .5% of pixels:
        nbin=200
        if badim:
            highvalue=im_max
            lowvalue=im_min
        else:
            imhist=ia.histograms(cumu=True,nbins=nbin,list=False)#['histout']
            ii=0
            lowcounts=imhist['counts'][ii]
            while imhist['counts'][ii]<0.005*lowcounts and ii<nbin: 
                ii=ii+1
            lowvalue=imhist['values'][ii]
            ii=nbin-1
            highcounts=imhist['counts'][ii]
            while imhist['counts'][ii]>0.995*highcounts and ii>0 and 0.995*highcounts>lowcounts: 
                ii=ii-1
            highvalue=imhist['values'][ii]
        if disprange != None:
            if type(disprange)==type([]):
                if len(disprange)>0:
                    highvalue=disprange[-1]
                    if len(disprange)>1:
                        lowvalue=disprange[0]
                        if len(disprange)>2:
                            throw("internal error disprange="+str(disprange)+" has too many elements")
                else:  # if passed an empty list [], return low.high
                    disprange.append(lowvalue)
                    disprange.append(highvalue)
            else:
                highvalue=disprange  # assume if scalar passed its the max

        if plot:
            img=pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,vmax=highvalue,vmin=lowvalue)            
            ax=pl.gca()
            #l=ax.get_xticklabels()
            #pl.setp(l,fontsize="x-small")
            #l=ax.get_yticklabels()
            #pl.setp(l,fontsize="x-small")
            foo=image.split("/")
            if len(foo)==1:
                imagestrip=image
            else:
                imagestrip=foo[1]
            pl.title(imagestrip,fontsize="x-small")
            if showstats:
                pl.text(0.05,0.95,"min=%7.1e\nmax=%7.1e\nRMS=%7.1e\n%s" % (im_min,im_max,im_rms,imunit),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
            if bar:
                cb=pl.colorbar(pad=0)
                cl = pl.getp(cb.ax,'yticklabels')
                pl.setp(cl,fontsize='x-small')
        ia.done()
        return im_min,im_max,im_rms,imunit







    ###########################################################

    def calc_pointings2(self, spacing, size, maptype="hex", direction=None, relmargin=0.5, beam=0.):   
        """
        If direction is a list, simply returns direction and the number of
        pointings in it.
        
        Otherwise, returns a hexagonally packed list of pointings separated by
        spacing and fitting inside an area specified by direction and mapsize, 
        as well as the number of pointings.  The hexagonal packing starts with a
        horizontal row centered on direction, and the other rows alternate
        being horizontally offset by a half spacing.  
        """
        # make size 2-dimensional and ensure it is quantity
        if type(size) != type([]):
            size=[size,size]
        if len(size) <2:
            size=[size[0],size[0]]
        self.isquantity(size)

        # parse and check direction
        if direction==None:
            # if no direction is specified, use the object's direction
            direction=self.direction
        else:
            # if one is specified, use it to set the object's direction
            self.direction=direction
        self.isdirection(direction)

        # direction is always a list of strings (defined by .xml)
        if type(direction)==type([]):
            if len(direction) > 1:                
                if self.verbose: self.msg("you are inputing the precise pointings in 'direction' - if you want to calculate a mosaic, give a single direction string and set maptype",priority="warn")
                return direction
            else: direction=direction[0]


        # haveing eliminated other options, we need to calculate:
        epoch, centx, centy = self.direction_splitter()

        shorttype=str.upper(maptype[0:3])
#        if not shorttype=="HEX":
#            self.msg("can't calculate map of maptype "+maptype,priority="error")
        if shorttype == "HEX": 
            # this is hexagonal grid - Kana will add other types here
            self.isquantity(spacing)
            spacing  = qa.quantity(spacing)
            yspacing = qa.mul(0.866025404, spacing)
            
            xsize=qa.quantity(size[0])
            ysize=qa.quantity(size[1])

            nrows = 1+ int(pl.floor(qa.convert(qa.div(ysize, yspacing), '')['value']
                                    - 2.309401077 * relmargin))

            availcols = 1 + qa.convert(qa.div(xsize, spacing),
                                       '')['value'] - 2.0 * relmargin
            ncols = int(pl.floor(availcols))

            # By making the even rows shifted spacing/2 ahead, and possibly shorter,
            # the top and bottom rows (nrows odd), are guaranteed to be short.
            if availcols - ncols >= 0.5 and nrows>1:                            # O O O
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
                ystr = qa.formxxx(y, format='dms',prec=5)
                
                if row % 2:                             # Odd
                    xmin = qa.sub(centx, qa.mul(ncolstomin, xspacing))
                    stopcolp1 = ncols
                else:                                   # Even (including 0)
                    xmin = qa.sub(centx, qa.mul(ncolstomin - 0.5,
                                                xspacing))
                    stopcolp1 = evencols
                for col in xrange(0, stopcolp1):        # xrange stops early.
                    x = qa.formxxx(qa.add(xmin, qa.mul(col, xspacing)),
                                   format='hms',prec=5)
                    pointings.append("%s%s %s" % (epoch, x, ystr))
                y = qa.sub(y, yspacing)
        elif shorttype =="SQU":
            # lattice gridding
            self.isquantity(spacing)
            spacing  = qa.quantity(spacing)
            yspacing = spacing
    
            xsize=qa.quantity(size[0])
            ysize=qa.quantity(size[1])

            nrows = 1+ int(pl.floor(qa.convert(qa.div(ysize, yspacing), '')['value']
                                    - 2.0 * relmargin))

            availcols = 1 + qa.convert(qa.div(xsize, spacing), '')['value'] \
                        - 2.0 * relmargin
            ncols = int(pl.floor(availcols))


            ncolstomin = 0.5 * (ncols - 1)                           # O O O O
            pointings = []                                           # O O O O

            # Start from the top because in the Southern hemisphere it sets first.
            y = qa.add(centy, qa.mul(0.5 * (nrows - 1), yspacing))
            for row in xrange(0, nrows):         # xrange stops early.
                xspacing = qa.mul(1.0 / pl.cos(qa.convert(y, 'rad')['value']),spacing)
                ystr = qa.formxxx(y, format='dms',prec=5)

                xmin = qa.sub(centx, qa.mul(ncolstomin, xspacing))
                stopcolp1 = ncols
        
                for col in xrange(0, stopcolp1):        # xrange stops early.
                    x = qa.formxxx(qa.add(xmin, qa.mul(col, xspacing)),
                                   format='hms',prec=5)
                    pointings.append("%s%s %s" % (epoch, x, ystr))
                y = qa.sub(y, yspacing)
        if shorttype == "ALM": 
            # OT algorithm
            self.isquantity(spacing)
            spacing  = qa.quantity(spacing)
            xsize = qa.quantity(size[0])
            ysize = qa.quantity(size[1])

            spacing_asec=qa.convert(spacing,'arcsec')['value']
            xsize_asec=qa.convert(xsize,'arcsec')['value']
            ysize_asec=qa.convert(ysize,'arcsec')['value']
            angle = 0. # deg

            if str.upper(maptype[0:8]) == 'ALMA2012':
                x,y = self.getTrianglePoints(xsize_asec, ysize_asec, angle, spacing_asec)
            else:
                if beam<=0: 
                    beam=spacing_asec*pbcoeff*pl.sqrt(3) # ASSUMES ALMA default and arcsec
                x,y = self.getTriangularTiling(xsize_asec, ysize_asec, angle, spacing_asec, beam)

            pointings = []
            nx=len(x)
            for i in range(nx):
            # Start from the top because in the Southern hemisphere it sets first.
                y1=qa.add(centy, str(y[nx-i-1])+"arcsec")
                ycos=pl.cos(qa.convert(y1,"rad")['value'])
                ystr = qa.formxxx(y1, format='dms',prec=5)
                xstr = qa.formxxx(qa.add(centx, str(x[nx-i-1]/ycos)+"arcsec"), format='hms',prec=5)
                pointings.append("%s%s %s" % (epoch, xstr, ystr))
            

        # if could not fit any pointings, then return single pointing
        if(len(pointings)==0):
            pointings.append(direction)

        self.msg("using %i generated pointing(s)" % len(pointings),origin='calc_pointings')
        self.pointings=pointings
        return pointings










    ###########################################################

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
                        xstr = qa.formxxx(qa.quantity(ra0), format='hms',prec=5)
                        ystr = qa.formxxx(qa.quantity(de0), format='dms',prec=5)
                        pointings.append("%s %s %s" % (epoch,xstr,ystr))
            except:
                break
        f.close()

        # need an error check here if zero valid pointings were read
        if len(pointings) < 1:
            s="No valid lines found in pointing file"
            self.msg(s,priority="error")
        self.msg("read in %i pointing(s) from file" % len(pointings),origin="read_pointings")
        self.pointings=pointings
        #self.direction=pointings
                
        return len(pointings), pointings, time

    





    ###########################################################

    def write_pointings(self, filename,pointings,time=1.):
        """
        write pointing list to file containing epoch, ra, dec,
        and scan time (optional,in sec).
        
        Example of an output file:
        #Epoch     RA          DEC      TIME(optional)
        J2000 23h59m28.10 -019d52m12.35 10.0
        J2000 23h59m32.35 -019d52m12.35 10.0
        J2000 23h59m36.61 -019d52m12.35 60.0
        
        """
        f=open(filename,"write")
        f.write('#Epoch     RA          DEC      TIME[sec]\n')
        if type(pointings)!=type([]):
            pointings=[pointings]
        npos=len(pointings)
        if type(time)!=type([]):
            time=[time]
        if len(time)==1:
            time=list(time[0] for x in range(npos))

        for i in range(npos):
            #epoch,ra,dec=direction_splitter(pointings[i])
            #xstr = qa.formxxx(qa.quantity(ra), format='hms')
            #ystr = qa.formxxx(qa.quantity(dec), format='dms')
            #line = "%s %s %s" % (epoch,xstr,ystr)
            #self.isdirection(line)  # extra check
            #f.write(line+"  "+str(time[i])+"\n")
            f.write(pointings[i]+"  "+str(time[i])+"\n")

        f.close()
        return 
    


    ###########################################################

    def average_direction(self, directions=None):
        # RI TODO make deal with list of measures as well as list of strings
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
        avgx = qa.toangle('%17.12fdeg' % avgx)
        avgy = qa.toangle('%17.12fdeg' % avgy)
        avgx = qa.formxxx(avgx, format='hms',prec=5)
        avgy = qa.formxxx(avgy, format='dms',prec=5)
        return "%s%s %s" % (epoch0, avgx, avgy), offsets



    ###########################################################

    def median_direction(self, directions=None):
        # RI TODO make deal with list of measures as well as list of strings
        """
        Returns the median of directions as a string, and relative offsets
        """
        if directions==None:
            directions=self.direction
        epoch0, x, y = self.direction_splitter(directions[0])
        i = 1
        avgx = 0.0
        avgy = 0.0
        xx=[]
        yy=[]
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
            xx.append(x)
            yy.append(y)
            i += 1
        avgx = pl.median(xx)
        avgy = pl.median(yy)
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
        avgx = qa.toangle('%17.12fdeg' % avgx)
        avgy = qa.toangle('%17.12fdeg' % avgy)
        avgx = qa.formxxx(avgx, format='hms',prec=5)
        avgy = qa.formxxx(avgy, format='dms',prec=5)
        return "%s%s %s" % (epoch0, avgx, avgy), offsets



    ###########################################################

    def direction_splitter(self, direction=None):
        """
        Given a direction, return its epoch, x, and y parts.  Epoch will be ''
        if absent, or '%s ' % epoch if present.  x and y will be angle qa's in
        degrees.
        """
        import re
        if direction == None:
            direction=self.direction
        if type(direction) == type([]):
            direction=self.average_direction(direction)[0]
        dirl = direction.split()
        if len(dirl) == 3:
            epoch = dirl[0] + ' '
        else:
            epoch = ''
        # x, y = map(qa.toangle, dirl[-2:])
        x=qa.toangle(dirl[1])
        # qa is stupid when it comes to dms vs hms, and assumes hms with colons and dms for periods.  
        decstr=dirl[2]
        # parse with regex to get three numbers and reinstall them as dms
        q=re.compile('([+-]?\d+).(\d+).(\d+\.?\d*)')
        qq=q.match(decstr)
        z=qq.groups()
        decstr=z[0]+'d'
        if len(z)>1:
            decstr=decstr+z[1]+'m'
        if len(z)>2:
            decstr=decstr+z[2]+'s'
        y=qa.toangle(decstr)

        return epoch, qa.convert(x, 'deg'), qa.convert(y, 'deg')


    ###########################################################

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


    ###########################################################

    def dir_m2s(self, dir):
        """
        Given a direction as a measure, return it as astring 'refcode lon lat'.
        """
        if dir['type'] != 'direction':
            self.msg("converting direction measure",priority="error",origin="simutil.m2s")
            return False
        ystr = qa.formxxx(dir['m1'], format='dms',prec=5)
        xstr = qa.formxxx(dir['m0'], format='hms',prec=5)
        return "%s %s %s" % (dir['refer'], xstr, ystr)

    ###########################################################

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
    









    ###########################################################
    #========================== tsys ==========================

    def noisetemp(self, telescope=None, freq=None,
                  diam=None, epsilon=None):

        """
        Noise temperature and efficiencies for several telescopes:
              ALMA, ACA, EVLA, VLA, and SMA
        Input: telescope name, frequency as a quantity string "300GHz", 
               dish diameter (optional - knows diameters for arrays above)
               epsilon = rms surface accuracy in microns (also optional - 
                   this method contains the spec values for each telescope)
        Output: eta_p phase efficieny (from Ruze formula), 
                eta_s spill (main beam) efficiency,
                eta_b geometrical blockage efficiency,
                eta_t taper efficiency,
                eta_q correlator efficiency including quantization,
                t_rx  reciever temperature.
        antenna efficiency = eta_p * eta_s * eta_b * eta_t
        Notes: VLA correlator efficiency includes waveguide loss
               EVLA correlator efficiency is probably optimistic at 0.88
        """

        if telescope==None: telescope=self.telescopename
        telescope=str.upper(telescope)
        
        obs =['ALMASD','ALMA','ACA','EVLA','VLA','SMA']
        d   =[ 12.    ,12.   ,7.   ,25.   ,25.  , 6. ]
        ds  =[ 0.75   ,0.75  ,0.75 ,0.364 ,0.364,0.35] # subreflector size for ACA?
        eps =[ 25.    ,25.   ,20.  ,300   ,300  ,15. ] # antenna surface accuracy
        
        cq  =[ 0.845, 0.845,  0.88,  0.79, 0.86] # correlator eff
        # ALMA includes quantization eff of 0.96    
        # VLA includes additional waveguide loss from correlator loss of 0.809
        # EVLA is probably optimistic

        # things hardcoded in ALMA etimecalculator
        # t_ground=270.
        # t_cmb=2.73
        # eta_q*eta_corr = 0.88*.961
        # eta_ap = 0.72*eta_ruze
        
        if obs.count(telescope)>0:
            iobs=obs.index(telescope)
        else:
            if self.verbose: self.msg("I don't know much about "+telescope+" so I'm going to use ALMA specs")
            iobs=1 # ALMA is the default ;)
            
        if diam==None: diam=d[iobs]
        diam_subreflector=ds[iobs]
        if self.verbose: self.msg("subreflector diameter="+str(diam_subreflector),origin="noisetemp")

        # blockage efficiency.    
        eta_b = 1.-(diam_subreflector/diam)**2

        # spillover efficiency.    
        eta_s = 0.95 # these are ALMA values
        # taper efficiency.    
        #eta_t = 0.86 # these are ALMA values
        eta_t = 0.819 # 20100914 OT value
        eta_t = 0.72

        # Ruze phase efficiency.    
        if epsilon==None: epsilon = eps[iobs] # microns RMS
        if freq==None:
            freq_ghz=qa.convert(qa.quantity(self.centerfreq),'GHz')
            bw_ghz=qa.convert(qa.quantity(self.bandwidth),'GHz')
        else:            
            freq_ghz=qa.convert(qa.quantity(freq),'GHz')
        eta_p = pl.exp(-(4.0*3.1415926535*epsilon*freq_ghz.get("value")/2.99792458e5)**2)
        if self.verbose: self.msg("ruze phase efficiency for surface accuracy of "+str(epsilon)+"um = " + str(eta_p) + " at "+str(freq),origin="noisetemp")

        # antenna efficiency
        # eta_a = eta_p*eta_s*eta_b*eta_t

        # correlator quantization efficiency.    
        eta_q = cq[iobs]

        # Receiver radiation temperature in K.         
        if telescope=='ALMA' or telescope=='ACA' or telescope=='ALMASD':
            # ALMA-40.00.00.00-001-A-SPE.pdf
            # http://www.eso.org/sci/facilities/alma/system/frontend/

            # limits instead of centers, go to higher band in gaps
            f0=[31.3,45,84,116,163,211,275,373,500,720]
            # cycle 1 OT values 7/12
            t0=[ 17, 30, 45, 51, 65, 55, 75, 196, 100, 230]

            flim=[31.3,950]
            if self.verbose: self.msg("using ALMA/ACA Rx specs",origin="noisetemp")
        else:
            if telescope=='EVLA':
                # 201009114 from rick perley:
                # f0=[1.5,3,6,10,15,23,33,45]
                t0=[10.,15,12,15,10,12,15,28]
                # limits
                f0=[1,2,4,8,12,18,26.5,40,50]

                flim=[0.8,50]
                if self.verbose: self.msg("using EVLA Rx specs",origin="noisetemp")
            else:
                if telescope=='VLA':
                    # http://www.vla.nrao.edu/genpub/overview/
                    # f0=[0.0735,0.32, 1.5, 4.75, 8.4, 14.9, 23, 45 ]
                    # t0=[5000,  165,  56,  44,   34,  110, 110, 110]
                    # exclude P band for now
                    # f0=[0.32, 1.5, 4.75, 8.4, 14.9, 23, 45 ]
                    # limits
                    # http://www.vla.nrao.edu/genpub/overview/
                    f0=[0.30,0.34,1.73,5,8.8,15.4,24,50]
                    t0=[165,  56,  44,   34,  110, 110, 110]
                    flim=[0.305,50]
                    if self.verbose: self.msg("using old VLA Rx specs",origin="noisetemp")                    
                else:
                    if telescope=='SMA':
                        # f0=[212.,310.,383.,660.]
                        # limits
                        f0=[180,250,320,420,720]
                        t0=[67,  116, 134, 500]
                        flim=[180.,720]
                    else:
                        self.msg("I don't know about the "+telescope+" receivers, using 200K",priority="warn",origin="noisetemp")
                        f0=[10,900]
                        t0=[200,200]
                        flim=[0,5000]


        obsfreq=freq_ghz.get("value")        
        # z=pl.where(abs(obsfreq-pl.array(f0)) == min(abs(obsfreq-pl.array(f0))))
        # t_rx=t0[z[0]]
        z=0
        while(f0[z]<obsfreq and z<len(t0)):
            z+=1
        t_rx=t0[z-1]
        
        if obsfreq<flim[0]:
            self.msg("observing freqency is lower than expected for "+telescope,priority="warn",origin="noise")
            self.msg("proceeding with extrapolated receiver temp="+str(t_rx),priority="warn",origin="noise")
        if obsfreq>flim[1]:
            self.msg("observing freqency is higher than expected for "+telescope,priority="warn",origin="noise")
            self.msg("proceeding with extrapolated receiver temp="+str(t_rx),priority="warn",origin="noise")
        if obsfreq<=flim[1] and obsfreq>=flim[0]:
            self.msg("interpolated receiver temp="+str(t_rx),origin="noise")

        return eta_p, eta_s, eta_b, eta_t, eta_q, t_rx
    
    




    def sensitivity(self, freq, bandwidth, etime, elevation, pwv=None,
                   telescope=None, diam=None, nant=None,
                   antennalist=None,
                   doimnoise=None,
                   integration=None,debug=None,
                   method="tsys-atm",tau0=None,t_sky=None):
        
        import glob
        tmpname="tmp"+str(os.getpid())
        i=0
        while i<10 and len(glob.glob(tmpname+"*"))>0:
            tmpname="tmp"+str(os.getpid())+str(i)
            i=i+1
        if i>=9:
            xx=glob.glob(tmpname+"*")
            for k in range(len(xx)):
                if os.path.isdir(xx[k]):
                    cu.removetable(xx[k])
                else:
                    os.remove(xx[k])
 
        msfile=tmpname+".ms"
        sm.open(msfile)

        rxtype=0 # 2SB
        if antennalist==None:
            if telescope==None:
                self.msg("Telescope name has not been set.",priority="error")
                return False 
            if diam==None:
                self.msg("Antenna diameter has not been set.",priority="error")
                return False
            if nant==None:
                self.msg("Number of antennas has not been set.",priority="error")
                return False
               
            posobs=me.observatory(telescope)
            obs=me.measure(posobs,'WGS84')
            obslat=qa.convert(obs['m1'],'deg')['value']
            obslon=qa.convert(obs['m0'],'deg')['value']
            obsalt=qa.convert(obs['m2'],'m')['value']
            stnx,stny,stnz = self.locxyz2itrf(obslat,obslon,obsalt,0,0,0)
            antnames="A00"

        else:
            if str.upper(antennalist[0:4])=="ALMA":
                tail=antennalist[5:]
                if self.isquantity(tail,halt=False):
                    resl=qa.convert(tail,"arcsec")['value']
                    repodir=os.getenv("CASAPATH").split(' ')[0]+"/data/alma/simmos/"
                    if os.path.exists(repodir):
                        confnum=(2.867-pl.log10(resl*1000*qa.convert(freq,"GHz")['value']/672.))/0.0721
                        confnum=max(1,min(28,confnum))
                        conf=str(int(round(confnum)))
                        if len(conf)<2: conf='0'+conf
                        antennalist=repodir+"alma.out"+conf+".cfg"
                        self.msg("converted resolution to antennalist "+antennalist)

            if os.path.exists(antennalist):
                stnx, stny, stnz, stnd, padnames, nant, telescope = self.readantenna(antennalist)
            else:
                self.msg("antennalist "+antennalist+" not found",priority="error")
                return False

            # RI TODO average antenna instead of first?
            diam = stnd[0]
            antnames=padnames

            posobs=me.observatory(telescope)
            obs=me.measure(posobs,'WGS84')
            #obslat=qa.convert(obs['m1'],'deg')['value']
            #obslon=qa.convert(obs['m0'],'deg')['value']
            #obsalt=qa.convert(obs['m2'],'m')['value']

 
        if (telescope==None or diam==None):
            self.msg("Telescope name and antenna diameter have not been set.",priority="error")
            return False

        # copied from task_simdata:

        self.setcfg(sm, telescope, stnx, stny, stnz, diam,
                    padnames, posobs)
                
        model_nchan=1
        # RI TODO isquantity checks
        model_width=qa.quantity(bandwidth) # note: ATM uses band center

        # start is center of first channel.  for nch=1, that equals center
        model_start=qa.quantity(freq)

        stokes, feeds = self.polsettings(telescope)
        sm.setspwindow(spwname="band1", freq=qa.tos(model_start), 
                       deltafreq=qa.tos(model_width), 
                       freqresolution=qa.tos(model_width), 
                       nchannels=model_nchan, stokes=stokes)
        sm.setfeed(mode=feeds, pol=[''])

        sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
        sm.setauto(0.0)

        obslat=qa.convert(obs['m1'],'deg')
        dec=qa.add(obslat, qa.add(qa.quantity("90deg"),qa.mul(elevation,-1)))

        sm.setfield(sourcename="src1", 
                    sourcedirection="J2000 00:00:00.00 "+qa.angle(dec)[0],
                    calcode="OBJ", distance='0m')
        reftime = me.epoch('TAI', "2012/01/01/00:00:00")
        if integration==None:
            integration=qa.mul(etime,0.01)        
        self.msg("observing for "+qa.tos(etime)+" with integration="+qa.tos(integration))
        sm.settimes(integrationtime=integration, usehourangle=True, 
                    referencetime=reftime)

        sm.observe(sourcename="src1", spwname="band1",
                   starttime=qa.quantity(0, "s"),
                   stoptime=qa.quantity(etime));
        
        sm.setdata()
        sm.setvp()
        
        eta_p, eta_s, eta_b, eta_t, eta_q, t_rx = self.noisetemp(telescope=telescope,freq=freq)
        eta_a = eta_p * eta_s * eta_b * eta_t
        if self.verbose: 
            self.msg('antenna efficiency    = ' + str(eta_a),origin="noise")
            self.msg('spillover efficiency  = ' + str(eta_s),origin="noise")
            self.msg('correlator efficiency = ' + str(eta_q),origin="noise")
        
        if pwv==None:
            # RI TODO choose based on freq octile
            pwv=2.0

        # things hardcoded in ALMA etimecalculator, & defaults in simulator.xml
        t_ground=270.
        t_cmb=2.725
        # eta_q = 0.88
        # eta_a = 0.95*0.8*eta_s

        if telescope=='ALMA' and (qa.convert(freq,"GHz")['value'])>600.:
            rxtype=1 # DSB
        
        if method=="tsys-atm":
            sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                        antefficiency=eta_a,trx=t_rx,
                        tground=t_ground,tcmb=t_cmb,pwv=str(pwv)+"mm",
                        mode="tsys-atm",table=tmpname,rxtype=rxtype)
        else:
            if method=="tsys-manual":
                if not t_sky:
                    t_sky=200.
                    self.msg("Warning: Sky brightness temp not set, using 200K",origin="noise",priority="warn")
                sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                            antefficiency=eta_a,trx=t_rx,tatmos=t_sky,
                            tground=t_ground,tcmb=t_cmb,tau=tau0,
                            mode="tsys-manual",table=tmpname,rxtype=rxtype)
            else:
                self.msg("Unknown calculation method "+method,priority="error")
                return False
        
        if doimnoise:
            sm.corrupt()

        sm.done()

        
        if doimnoise:
            cellsize=qa.quantity(6.e3/250./qa.convert(model_start,"GHz")["value"],"arcsec")  # need better cell determination - 250m?!
            cellsize=[cellsize,cellsize]
            # very light clean - its an empty image!
            self.imclean(tmpname+".ms",tmpname,
                       "csclean",cellsize,[128,128],
                       "J2000 00:00:00.00 "+qa.angle(dec)[0],
                       False,100,"0.01mJy","natural",[],True,"I")

            ia.open(tmpname+".image")
            stats= ia.statistics(robust=True, verbose=False,list=False)
            ia.done()
            imnoise=(stats["rms"][0])
        else:
            imnoise=0.

        nint = qa.convert(etime,'s')['value'] / qa.convert(integration,'s')['value'] 
        nbase= 0.5*nant*(nant-1)
                
        if os.path.exists(tmpname+".T.cal"):
            tb.open(tmpname+".T.cal")
            gain=tb.getcol("CPARAM")
            # RI TODO average instead of first?
            tb.done()
            # gain is per ANT so square for per baseline;  
            # pick a gain from about the middle of the track
            noiseperbase=1./(gain[0][0][0.5*nint*nant].real)**2
        else:
            noiseperbase=0.

        theoreticalnoise=noiseperbase/pl.sqrt(nint*nbase*2) # assume 2-poln
        
        if debug==None:
            xx=glob.glob(tmpname+"*")
            for k in range(len(xx)):
                if os.path.isdir(xx[k]):
                    cu.removetable(xx[k])
                else:
                    os.remove(xx[k])

        if doimnoise:
            return theoreticalnoise , imnoise
        else:
            return theoreticalnoise 


    def setcfg(self, mysm, telescope, x, y, z, diam,
               padnames, posobs, mounttype=None):
        """
        Sets the antenna positions for the mysm sm instance, which should have
        already opened an MS, given
        telescope - telescope name
        x         - array of X positions, i.e. stnx from readantenna
        y         - array of Y positions, i.e. stny from readantenna
        z         - array of Z positions, i.e. stnz from readantenna
        diam      - numpy.array of antenna diameters, i.e. from readantenna
        padnames  - list of pad names
        posobs    - The observatory position as a measure.

        Optional:
        mounttype   (Defaults to a guess based on telescope.)

        Returns the mounttype that it uses.

        Closes mysm and throws a ValueError if it can't set the configuration.
        """
        if not mounttype:
            mounttype = 'alt-az'
            if telescope.upper() in ('ASKAP',  # Effectively, if not BIZARRE.
                                     'DRAO',
                                     'WSRT'):
                mounttype = 'EQUATORIAL'
            elif telescope.upper() in ('LOFAR', 'LWA', 'MOST'):
                # And other long wavelength arrays...like that one in WA that
                # has 3 orthogonal feeds so they can go to the horizon.
                #
                # The SKA should go here too once people accept
                # that it will have to be correlated as stations.
                mounttype = 'BIZARRE'  # Ideally it would be 'FIXED' or
                                       # 'GROUND'.

        if not mysm.setconfig(telescopename=telescope, x=x, y=y, z=z,
                              dishdiameter=diam.tolist(), 
                              mount=[mounttype], padname=padnames,
                              coordsystem='global', referencelocation=posobs):
            mysm.close()
            raise ValueError("Error setting the configuration")
        return mounttype




    ###########################################################
    #===================== ephemeris ==========================


    def ephemeris(self, date, usehourangle=True, direction=None, telescope=None, ms=None):

        if direction==None: direction=self.direction
        if telescope==None: telescope=self.telescopename

        import pdb
        
        # right now, simdata centers at the transit;  when that changes,
        # or when that becomes optional, then that option needs to be
        # stored in the simutil object and used here, to either
        # center the plot at transit or not.
        #
        # direction="J2000 18h00m00.03s -45d59m59.6s"
        # refdate="2012/06/21/03:25:00"

        # useHourAngle_p means simulate at transit
        # TODO: put in reftime parameter, parse 2012/05/21, 2012/05/21/transit,
        # and 2012/05/21/22:05:00 separately.
        
        ds=self.direction_splitter(direction)  # if list, returns average
        src=me.direction(ds[0],ds[1],ds[2])
    
        me.done()
        me.doframe(me.observatory(telescope))
    
        time=me.epoch('TAI',date)
        me.doframe(time)

        # what is HA of source at refdate? 
        offset_ha=qa.convert((me.measure(src,'hadec'))['m0'],'h')
        peak=me.epoch("TAI",qa.add(date,qa.mul(-1,offset_ha)))
        peaktime_float=peak['m0']['value']
        if usehourangle:
            # offset the reftime to be at transit:
            time=peak
            me.doframe(time)
                        
        reftime_float=time['m0']['value']
        reftime_floor=pl.floor(time['m0']['value'])
        refdate_str=qa.time(qa.totime(str(reftime_floor)+'d'),form='dmy')[0]

        timeinc='15min'  # for plotting
        timeinc=qa.convert(qa.time(timeinc)[0],'d')['value']
        ntime=int(1./timeinc)

        # check for circumpolar:
        rset = me.riseset(src)
        rise = rset['rise']
        if rise == 'above':
            rise = time
            rise['m0']['value'] = rise['m0']['value'] - 0.5
            settime = time
            settime['m0']['value'] = settime['m0']['value'] + 0.5
        elif rise == 'below':
            raise ValueError(direction + ' is not visible from ' + telescope)
        else:
            settime = rset['set']
            rise = me.measure(rise['utc'],'tai')
            settime = me.measure(settime['utc'],'tai')

        # where to start plotting?
        offset=-0.5
        if settime < time: offset-=0.5
        if rise > time: offset+=0.5
        time['m0']['value']+=offset

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
    
#        self.msg(" ref="+date,origin='ephemeris')
        self.msg("rise="+qa.time(rise['m0'],form='dmy')[0],origin='ephemeris')
        self.msg(" set="+qa.time(settime['m0'],form='dmy')[0],origin='ephemeris')
    
        pl.plot((pl.array(times)-reftime_floor)*24,el)
#        peak=(rise['m0']['value']+settime['m0']['value'])/2        
#        self.msg("peak="+qa.time('%fd' % peak,form='dmy'),origin='ephemeris')
        self.msg("peak="+qa.time('%fd' % reftime_float,form='dmy')[0],origin='ephemeris')

        relpeak=peaktime_float-reftime_floor
        pl.plot(pl.array([1,1])*24*relpeak,[0,90])

        # if theres an ms, figure out the entire range of observation
        if ms:
            tb.open(ms+"/OBSERVATION")
            timerange=tb.getcol("TIME_RANGE")
            tb.done()
            obsstart=min(timerange.flat)
            obsend=max(timerange.flat)
            relstart=me.epoch("UTC",str(obsstart)+"s")['m0']['value']-reftime_floor
            relend=me.epoch("UTC",str(obsend)+"s")['m0']['value']-reftime_floor
            pl.plot([relstart*24,relend*24],[89,89],'r',linewidth=3)
        else:
            if self.totaltime>0:
                etimeh=qa.convert(self.totaltime,'h')['value']
                pl.plot(pl.array([-0.5,0.5])*etimeh+relpeak*24,[80,80],'r')

        pl.xlabel("hours relative to "+refdate_str,fontsize='x-small')
        pl.ylabel("elevation",fontsize='x-small')
        ax=pl.gca()
        l=ax.get_xticklabels()
        pl.setp(l,fontsize="x-small")
        l=ax.get_yticklabels()
        pl.setp(l,fontsize="x-small")


        pl.ylim([0,90])
        pl.xlim(pl.array([-12,12])+24*(reftime_float-reftime_floor))



















    ###########################################################
    #==========================================================
    
    def readantenna(self, antab=None):
    ###Helper function to read 4 columns text antenna table X, Y, Z, Diam
        f=open(antab)
        self.msg("Reading antenna positions from '%s'" % antab,origin="readantenna")
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
                
                vsave=self.verbose
                for i in range(len(inx)):
                    x,y,z = self.utm2xyz(inx[i],iny[i],inz[i],int(zone),datum,nors)
                    if i==1: 
                        self.verbose=False
                    stnx.append(x)
                    stny.append(y)
                    stnz.append(z)
                self.verbose=vsave
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
                        self.msg("converting local tangent plane coordinates to ITRF using observatory position = %d %d " % (obslat,obslon),origin="readantenna")
                        #foo=self.getdatum(datum,verbose=True)
                    for i in range(len(inx)):
                        x,y,z = self.locxyz2itrf(obslat,obslon,obsalt,inx[i],iny[i],inz[i])
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



















    ###########################################################
    #==================== geodesy =============================


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
            self.msg(" longitude, latitude = %s %s; conv,kp = %f,%f" % (qa.angle(qa.quantity(lon,"rad"),prec=8)[0],qa.angle(qa.quantity(lat,"rad"),prec=8)[0],conv,kp),origin="utm2long")
        
        return lon,lat



    
    def long2xyz(self,long,lat,elevation,datum):
        
        dx,dy,dz,er,rf = self.getdatum(datum,verbose=False)

        f=1./rf
        esq=2*f-f**2    
        nu=er/pl.sqrt(1.-esq*(pl.sin(lat))**2)
        
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



    def locxyz2itrf(self, lat, longitude, alt, locx=0.0, locy=0.0, locz=0.0):
        """
        Returns the nominal ITRF (X, Y, Z) coordinates (m) for a point at "local"
        (x, y, z) (m) measured at geodetic latitude lat and longitude longitude
        (degrees) and altitude of the reference point of alt.  
        The ITRF frame used is not the official ITRF, just a right
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
        Nploczcphimlocysphi = (N + locz+alt) * pl.cos(phi) - locy * sphi
    
        clmb = pl.cos(lmbda)
        slmb = pl.sin(lmbda)
    
        x = Nploczcphimlocysphi * clmb - locx * slmb
        y = Nploczcphimlocysphi * slmb + locx * clmb
        z = (N * (b / a)**2 + locz+alt) * sphi + locy * pl.cos(phi)
    
        return x, y, z




    def itrf2loc(self, x,y,z, cx,cy,cz):
        """
        itrf xyz and COFA cx,cy,cz -> lat lon el WGS84
        """
        clon,clat = self.xyz2long(cx,cy,cz,'WGS84')
        ccoslon=pl.cos(clon)
        csinlon=pl.sin(clon)        
        csinlat=pl.sin(clat)
        ccoslat=pl.cos(clat)
        import types
        if isinstance(x,types.FloatType): # weak
            x=[x]
            y=[y]
            z=[z]
        n=x.__len__()
        lat=pl.zeros(n)
        lon=pl.zeros(n)
        el=pl.zeros(n)

        # do like MsPlotConvert
        for i in range(n):
            # translate w/o rotating:
            xtrans=x[i]-cx
            ytrans=y[i]-cy
            ztrans=z[i]-cz
            # rotate
            lat[i] = (-csinlon*xtrans) + (ccoslon*ytrans)
            lon[i] = (-csinlat*ccoslon*xtrans) - (csinlat*csinlon*ytrans) + ccoslat*ztrans
            el[i] = (ccoslat*ccoslon*xtrans) + (ccoslat*csinlon*ytrans) + csinlat*ztrans
                
        return lat,lon,el





    def xyz2loc(self, x,y,z, obsname):
        """
        itrf xyz and array name -> lat lon el WGS84
        """
        cofa=me.measure(me.observatory(obsname),'WGS84')
        cx,cy,cz=self.long2xyz(cofa['m0']['value'],cofa['m1']['value'],cofa['m2']['value'],cofa['refer'])

        return self.itrf2loc(x,y,z,cx,cy,cz)













    ###########################################################

    def plotants(self,x,y,z,d,name):
        # given globals
        
        #stnx, stny, stnz, stnd, nant, telescopename = util.readantenna(antennalist)
        cx=pl.mean(x)
        cy=pl.mean(y)
        cz=pl.mean(z)
        lat,lon,el = self.itrf2loc(x,y,z,cx,cy,cz)
        n=lat.__len__()
        
        dolam=0
        # TODO convert to klam: (d too)
        ###
                
        rg=max(lat)-min(lat)
        r2=max(lon)-min(lon)
        if r2>rg:
            rg=r2
        if max(d)>0.01*rg:
            pl.plot(lat,lon,',')            
            #print max(d),ra
            for i in range(n):
                pl.gca().add_patch(pl.Circle((lat[i],lon[i]),radius=0.5*d[i],fc="#dddd66"))
                if n<10:
                    pl.text(lat[i],lon[i],name[i],horizontalalignment='center',verticalalignment='center')
        else:
            pl.plot(lat,lon,'o',c="#dddd66")
            if n<10: 
                for i in range(n):
                    pl.text(lat[i],lon[i],name[i],horizontalalignment='center',fontsize=8)

        pl.axis('equal')
        #if dolam:
        #    pl.xlabel("kilolamda")
        #    pl.ylabel("kilolamda")



















    ######################################################
    # helper function to get the pixel size from an image  (positive increments)

    def cellsize(self,image):
        ia.open(image)
        mycs=ia.coordsys()
        ia.done()
        increments=mycs.increment(type="direction")['numeric']
        cellx=qa.quantity(abs(increments[0]),mycs.units(type="direction")[0])
        celly=qa.quantity(abs(increments[1]),mycs.units(type="direction")[1])
        xform=mycs.lineartransform(type="direction")
        offdiag=max(abs(xform[0,1]),abs(xform[1,0]))
        if offdiag > 1e-4:
            self.msg("Your image is rotated with respect to Lat/Lon.  I can't cope with that yet",priority="error")
        cellx=qa.mul(cellx,abs(xform[0,0]))
        celly=qa.mul(celly,abs(xform[1,1]))
        return [qa.tos(cellx),qa.tos(celly)]

    ######################################################
    # helper function to get the spectral size from an image

    def spectral(self,image):
        ia.open(image)
        cs=ia.coordsys()
        sh=ia.shape()
        ia.done()
        spc=cs.findcoordinate("spectral")
        if not spc[0]: return (0,0)
        model_width=str(cs.increment(type="spectral")['numeric'][0])+cs.units(type="spectral")[0]
        model_nchan=sh[spc['pixel']]
        return model_nchan,model_width

    ######################################################

    def is4d(self, image):
        ia.open(image)
        s=ia.shape()
        if len(s)!=4: return False
        cs=ia.coordsys()
        ia.done()
        dir=cs.findcoordinate("direction")
        spc=cs.findcoordinate("spectral")
        stk=cs.findcoordinate("stokes")
        if not (dir[0] and spc[0] and stk[0]): return False
        if dir[1].__len__() != 2: return False
        if spc[1].__len__() != 1: return False
        if stk[1].__len__() != 1: return False
        # they have to be in the correct order too
        if stk[1]!=2: return False
        if spc[1]!=3: return False
        if dir[1][0]!=0: return False
        if dir[1][1]!=1: return False
        cs.done()
        return True

    ##################################################################
    # fit modelimage into a 4 coordinate image defined by the parameters
    
    # TODO spectral extrapolation and regridding using innchan ****

    def modifymodel(self, inimage, outimage, 
                inbright,indirection,incell,incenter,inwidth,innchan,
                flatimage=False):  # if nonzero, create mom -1 image 

        # new ia tool
        in_ia=ia.newimagefromfile(inimage)
        in_shape=in_ia.shape()
        in_csys=in_ia.coordsys()

        # pull data first, since ia.stats doesn't work w/o a CS:
        if outimage!=inimage:
            if self.verbose: self.msg("rearranging input data (may take some time for large cubes)",origin="setup model")
            arr=in_ia.getchunk()
        else:
            # TODO move rearrange to inside ia tool, and at least don't do this:
            arr=pl.zeros(in_shape)
        axmap=[-1,-1,-1,-1]
        axassigned=[-1,-1,-1,-1]



        # brightness scaling 
        if (inbright=="unchanged") or (inbright==""):
            scalefactor=1.
        else:
            if self.isquantity(inbright,halt=False):
                qinb=qa.quantity(inbright)
                if qinb['unit']!='':
                    # qa doesn't deal universally well with pixels and beams
                    # so this may fail:
                    try:
                        inb=qa.convert(qinb,"Jy/pixel")['value']
                    except:
                        inb=qinb['value']
                        self.msg("assuming inbright="+str(inbright)+" means "+str(inb)+" Jy/pixel",priority="warn")
                    inbright=inb
            try:
                scalefactor=float(inbright)/pl.nanmax(arr)
            except Exception, e:
                in_ia.close()
                raise Exception, e

        # check shape characteristics of the input;
        # add degenerate axes as neeed:

        in_dir=in_csys.findcoordinate("direction")
        in_spc=in_csys.findcoordinate("spectral")
        in_stk=in_csys.findcoordinate("stokes")


        # first check number of pixel axes and raise to 4 if required
        in_nax=arr.shape.__len__()
        if in_nax<2:
            in_ia.close()
            self.msg("Your input model has fewer than 2 dimensions.  Can't proceed",priority="error")
            return False
        if in_nax==2:            
            arr=arr.reshape([arr.shape[0],arr.shape[1],1])
            in_shape=arr.shape
            in_nax=in_shape.__len__() # which should be 3
        if in_nax==3:
            arr=arr.reshape([arr.shape[0],arr.shape[1],arr.shape[2],1])
            in_shape=arr.shape
            in_nax=in_shape.__len__() # which should be 4
        if in_nax>4:
            in_ia.close()
            self.msg("model image has more than 4 dimensions.  Not sure how to proceed",priority="error")
            return False


        # make incell a list if not already
        try:
            if type(incell) == type([]):
                incell =  map(qa.convert,incell,['arcsec','arcsec'])
            else:
                incell = qa.abs(qa.convert(incell,'arcsec'))
                # incell[0]<0 for RA increasing left
                incell = [qa.mul(incell,-1),incell]
        except Exception, e:
            # invalid incell
            in_ia.close()
            raise Exception, e
        # later, we can test validity with qa.compare()


        # now parse coordsys:
        model_refdir=""
        model_cell=""
        # look for direction coordinate, with two pixel axes:
        if in_dir[0]:
            in_ndir = in_dir[1].__len__() 
            if in_ndir != 2:
                self.msg("Mal-formed direction coordinates in modelimage. Discarding and using first two pixel axes for RA and Dec.")
                axmap[0]=0 # direction in first two pixel axes
                axmap[1]=1
                axassigned[0]=0  # coordinate corresponding to first 2 pixel axes
                axassigned[1]=0
            else:
                # we've found direction axes, and may change their increments and direction or not.
                dirax=in_dir[1]
                axmap[0]=dirax[0]
                axmap[1]=dirax[1]
                axassigned[dirax[0]]=0
                axassigned[dirax[1]]=0
                if self.verbose: self.msg("Direction coordinate (%i,%i) parsed" % (axmap[0],axmap[1]),origin="setup model")
            # move model_refdir to center of image
            model_refpix=[0.5*in_shape[axmap[0]],0.5*in_shape[axmap[1]]]
            ra = in_ia.toworld(model_refpix,'q')['quantity']['*'+str(axmap[0]+1)]
            dec = in_ia.toworld(model_refpix,'q')['quantity']['*'+str(axmap[1]+1)]
            model_refdir= in_csys.referencecode(type="direction",list=False)[0]+" "+qa.formxxx(ra,format='hms',prec=5)+" "+qa.formxxx(dec,format='dms',prec=5)
            model_projection=in_csys.projection()['type']
            model_projpars=in_csys.projection()['parameters']

            # cell size from image
            increments=in_csys.increment(type="direction")['numeric']
            incellx=qa.quantity(increments[0],in_csys.units(type="direction")[0])
            incelly=qa.quantity(increments[1],in_csys.units(type="direction")[1])
            xform=in_csys.lineartransform(type="direction")
            offdiag=max(abs(xform[0,1]),abs(xform[1,0]))
            if offdiag > 1e-4:
                self.msg("Your image is rotated with respect to Lat/Lon.  I can't cope with that yet",priority="error")
            incellx=qa.mul(incellx,xform[0,0])
            incelly=qa.mul(incelly,xform[1,1])

            # preserve sign in case input image is backwards (RA increasing right)
            model_cell = [qa.convert(incellx,'arcsec'),qa.convert(incelly,'arcsec')]

        else: # no valid direction coordinate
            axmap[0]=0 # assign direction to first two pixel axes
            axmap[1]=1
            axassigned[0]=0  # assign coordinate corresponding to first 2 pixel axes
            axassigned[1]=0


        # try to parse direction using splitter function and override model_refdir 
        if type(indirection)==type([]):
            if len(indirection) > 1:
                in_ia.close()
                self.msg("error parsing indirection "+str(indirection)+" -- should be a single direction string")
                return False
            else:
                indirection=indirection[0]
        if self.isdirection(indirection,halt=False):
            # lon/lat = ra/dec if J2000, =glon/glat if galactic
            epoch, lon, lat = self.direction_splitter(indirection)

            model_refdir=epoch+qa.formxxx(lon,format='hms',prec=5)+" "+qa.formxxx(lat,format='dms',prec=5)
            model_refpix=[0.5*in_shape[axmap[0]],0.5*in_shape[axmap[1]]]
            model_projection="SIN" # for indirection we default to SIN.
            model_projpars=pl.array([0.,0])
            if self.verbose: self.msg("setting model image direction to indirection = "+model_refdir,origin="setup model")
        else:
            # indirection is not set - is there a direction in the model already?
            if not self.isdirection(model_refdir,halt=False):
                in_ia.close()
                self.msg("Cannot determine reference direction in model image.  Valid 'indirection' parameter must be provided.",priority="error")
                return False
        

        # override model_cell?
        cell_replaced=False
        if self.isquantity(incell[0],halt=False):
            if qa.compare(incell[0],"1arcsec"): 
                model_cell=incell
                cell_replaced=True
                if self.verbose: self.msg("replacing existing model cell size with incell",origin="setup model")
        valid_modcell=False
        if not cell_replaced:
            if self.isquantity(model_cell[0],halt=False):
                if qa.compare(model_cell[0],"1arcsec"):
                    valid_modcell=True
            if not valid_modcell:
                in_ia.close()
                self.msg("Unable to determine model cell size.  Valid 'incell' parameter must be provided.",priority="error")
                return False


     
        if self.verbose:
            self.msg("model image shape="+str(in_shape),origin="setup model")
            self.msg("model pixel = %8.2e x %8.2e arcsec" % (model_cell[0]['value'],model_cell[1]['value']),origin="setup model")







        # we've now found or assigned two direction axes, and changed direction and cell if required
        # next, work on spectral axis:

        model_center=""
        model_width=""
        # look for a spectral axis:
        if in_spc[0]:
            #if type(in_spc[1]) == type(1) :
            #    # should not come here after SWIG switch over
            #    foo=in_spc[1]
            #else:
            foo=in_spc[1][0]
            if in_spc[1].__len__() > 1:
                self.msg("you seem to have two spectral axes",priority="warn")
            model_nchan=arr.shape[foo]
            axmap[3]=foo
            axassigned[foo]=3
            model_restfreq=in_csys.restfrequency()
            in_startpix=in_csys.referencepixel(type="spectral")['numeric'][0]
            model_width=in_csys.increment(type="spectral")['numeric'][0]
            model_start=in_csys.referencevalue(type="spectral")['numeric'][0]-in_startpix*model_width
            # this maybe can be done more accurately - for nonregular
            # grids it may trip things up
            # start is center of first channel.  for nch=1, that equals center
            model_center=model_start+0.5*(model_nchan-1)*model_width
            model_width=str(model_width)+in_csys.units(type="spectral")[0]
            model_start=str(model_start)+in_csys.units(type="spectral")[0]
            model_center=str(model_center)+in_csys.units(type="spectral")[0]
            add_spectral_coord=False
            if self.verbose: self.msg("Spectral Coordinate %i parsed" % axmap[3],origin="setup model")                
        else:
            # need to add one to the coordsys 
            add_spectral_coord=True 


        # override incenter?
        center_replaced=False
        if self.isquantity(incenter,halt=False):
            if qa.compare(incenter,"1Hz"): 
                if (qa.quantity(incenter))['value']>=0:
                    model_center=incenter
                    model_restfreq=model_center
                    center_replaced=True
                    if self.verbose: self.msg("setting central frequency to "+incenter,origin="setup model")
        valid_modcenter=False
        if not center_replaced:
            if self.isquantity(model_center,halt=False):
                if qa.compare(model_center,"1Hz"):
                    valid_modcenter=True
            if not valid_modcenter:
                in_ia.close()
                self.msg("Unable to determine model frequency.  Valid 'incenter' parameter must be provided.",priority="error")
                return False

        # override inwidth?
        width_replaced=False
        if self.isquantity(inwidth,halt=False):
            if qa.compare(inwidth,"1Hz"): 
                if (qa.quantity(inwidth))['value']>=0:
                    model_width=inwidth
                    width_replaced=True
                    if self.verbose: self.msg("setting channel width to "+inwidth,origin="setup model")
        valid_modwidth=False
        if not width_replaced:
            if self.isquantity(model_width,halt=False):
                if qa.compare(model_width,"1Hz"):
                    valid_modwidth=True
            if not valid_modwidth:
                in_ia.close()
                self.msg("Unable to determine model channel or bandwidth.  Valid 'inwidth' parameter must be provided.",priority="error")
                return False





        model_stokes=""
        # look for a stokes axis
        if in_stk[0]:
            model_stokes=in_csys.stokes()
            foo=model_stokes[0]
            out_nstk=model_stokes.__len__()
            for i in range(out_nstk-1):
                foo=foo+model_stokes[i+1]
            model_stokes=foo
            #if type(in_stk[1]) == type(1):
            #    # should not come here after SWIG switch over
            #    foo=in_stk[1]
            #else:
            foo=in_stk[1][0]
            if in_stk[1].__len__() > 1:
                self.msg("you seem to have two stokes axes",priority="warn")                
            axmap[2]=foo
            axassigned[foo]=2
            if in_shape[foo]>4:
                in_ia.close()
                self.msg("you appear to have more than 4 Stokes components - please edit your header and/or parameters",priority="error")
                return False                        
            add_stokes_coord=False
            if self.verbose: self.msg("Stokes Coordinate %i parsed" % axmap[2],origin="setup model")
        else:
            # need to add one to the coordsys 
            add_stokes_coord=True 



        if add_spectral_coord:                        
            # find first unused axis - probably at end, but just in case its not:
            i=0
            extra_axis=-1
            while extra_axis<0 and i<4:
                if axassigned[i]<0: extra_axis=i
                i+=1
            if extra_axis<0:
                in_ia.close()
                self.msg("I can't find an unused axis to make Spectral [%i %i %i %i] " % (axassigned[0],axassigned[1],axassigned[2],axassigned[3]),priority="error",origin="setup model")
                return False

            axmap[3]=extra_axis
            axassigned[extra_axis]=3
            model_nchan=arr.shape[extra_axis]


        if add_stokes_coord:
            # find first unused axis - probably at end, but just in case its not:
            i=0
            extra_axis=-1
            while extra_axis<0 and i<4:
                if axassigned[i]<0: extra_axis=i
                i+=1
            if extra_axis<0:
                in_ia.close()
                self.msg("I can't find an unused axis to make Stokes [%i %i %i %i] " % (axassigned[0],axassigned[1],axassigned[2],axassigned[3]),priority="error",origin="setup model")
                return False
            axmap[2]=extra_axis
            axassigned[extra_axis]=2                            

            if arr.shape[extra_axis]>4:
                in_ia.close()
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




        # ========================================
        # now we should have 4 assigned pixel axes, and model_cell, model_refdir, model_nchan, 
        # model_stokes all set 
        out_nstk=len(model_stokes)
        if self.verbose:
            self.msg("axis map for model image = %i %i %i %i" %
                     (axmap[0],axmap[1],axmap[2],axmap[3]),origin="setup model")

        modelshape=[in_shape[axmap[0]], in_shape[axmap[1]],out_nstk,model_nchan]
        if outimage!=inimage:
            ia.fromshape(outimage,modelshape,overwrite=True)
            modelcsys=ia.coordsys()        
        else:
            modelcsys=in_ia.coordsys()        
        modelcsys.setunits(['rad','rad','','Hz'])
        modelcsys.setincrement([qa.convert(model_cell[0],modelcsys.units()[0])['value'],    # already -1
                                qa.convert(model_cell[1],modelcsys.units()[1])['value']],
                                type="direction")

        dirm=self.dir_s2m(model_refdir)
        lonq=dirm['m0'] 
        latq=dirm['m1'] 
        modelcsys.setreferencecode(dirm['refer'],type="direction")
        if len(model_projpars)>0:
            modelcsys.setprojection(parameters=model_projpars.tolist(),type=model_projection)
        else:
            modelcsys.setprojection(type=model_projection)
        modelcsys.setreferencevalue(
            [qa.convert(lonq,modelcsys.units()[0])['value'],
             qa.convert(latq,modelcsys.units()[1])['value']],
            type="direction")
        modelcsys.setreferencepixel(model_refpix,"direction")
        if self.verbose: 
            self.msg("sky model image direction = "+model_refdir,origin="setup model")
            self.msg("sky model image increment = "+str(model_cell),origin="setup model")

        modelcsys.setspectral(refcode="LSRK",restfreq=model_restfreq)
        modelcsys.setreferencevalue(qa.convert(model_center,modelcsys.units()[3])['value'],type="spectral")
#        modelcsys.setreferencepixel(0.5*model_nchan,type="spectral") # default is middle chan
        modelcsys.setreferencepixel(0.5*(model_nchan-1),type="spectral") # but not half-pixel
        modelcsys.setincrement(qa.convert(model_width,modelcsys.units()[3])['value'],type="spectral")


        # first assure that the csys has the expected order 
        expected=['Direction', 'Direction', 'Stokes', 'Spectral']
        if modelcsys.axiscoordinatetypes() != expected:
            in_ia.close()
            self.msg("internal error with coordinate axis order created by Imager",priority="error")
            self.msg(modelcsys.axiscoordinatetypes().__str__(),priority="error")
            return False

        # more checks:
        foo=pl.array(modelshape)
        if not (pl.array(arr.shape) == pl.array(foo.take(axmap).tolist())).all():
            in_ia.close()
            self.msg("internal error: I'm confused about the shape if your model data cube",priority="error")
            self.msg("have "+foo.take(axmap).__str__()+", want "+in_shape.__str__(),priority="error")
            return False

        if outimage!=inimage:
            ia.setcoordsys(modelcsys.torecord())
            ia.done()
            ia.open(outimage)
        
        # now rearrange the pixel axes if ness.
        for ax in range(4):
            if axmap[ax] != ax:
                if self.verbose: self.msg("swapping input axes %i with %i" % (ax,axmap[ax]),origin="setup model")
                arr=arr.swapaxes(ax,axmap[ax])                        
                tmp=axmap[ax]
                axmap[ax]=ax
                axmap[tmp]=tmp                

        
        # there's got to be a better way to remove NaNs:
        if outimage!=inimage:
            for i0 in range(arr.shape[0]):
                for i1 in range(arr.shape[1]):
                    for i2 in range(arr.shape[2]):
                        for i3 in range(arr.shape[3]):
                            foo=arr[i0,i1,i2,i3]
                            if foo!=foo: arr[i0,i1,i2,i3]=0.0

        if self.verbose and outimage!=inimage:
            self.msg("model array minmax= %e %e" % (arr.min(),arr.max()),origin="setup model")        
            self.msg("scaling model brightness by a factor of %f" % scalefactor,origin="setup model")
            self.msg("image channel width = %8.2e GHz" % qa.convert(model_width,'GHz')['value'],origin="setup model")
            if arr.nbytes > 5e7:
                self.msg("your model is large - predicting visibilities may take a while.",priority="warn")

        if outimage!=inimage:
            ia.putchunk(arr*scalefactor)
            ia.setbrightnessunit("Jy/pixel")
            ia.close()
        in_ia.close()
        # outimage should now have correct Coordsys and shape

        # make a moment 0 image
        if flatimage and outimage!=inimage:
            self.flatimage(outimage,verbose=self.verbose)

        # returning to the outside world we'll return a positive cell:
        model_cell=[qa.abs(model_cell[0]),qa.abs(model_cell[1])]
        model_size=[qa.mul(modelshape[0],model_cell[0]),
                    qa.mul(modelshape[1],model_cell[1])]

        if self.verbose: self.msg(" ") # add a line after my spewage

        return model_refdir,model_cell,model_size,model_nchan,model_center,model_width,model_stokes





    ##################################################################
    # image/clean subtask

    def imclean(self,mstoimage,imagename,
                cleanmode,cell,imsize,imcenter,interactive,niter,threshold,weighting,
                outertaper,pbcor,stokes,sourcefieldlist="",modelimage="",mask=[],dryrun=False):
        from clean import clean

        from simutil import is_array_type

        # determine channelization from (first) ms:
        if is_array_type(mstoimage):
            ms0=mstoimage[0]
            if len(mstoimage)==1:
                mstoimage=mstoimage[0]
        else:
            ms0=mstoimage
        
        if os.path.exists(ms0):
            tb.open(ms0+"/SPECTRAL_WINDOW")
            if tb.nrows() > 1:
                self.msg("determining output cube parameters from FIRST of several SPW in MS "+ms0)
            freq=tb.getvarcol("CHAN_FREQ")['r1']
            nchan=freq.size
            tb.done()
        elif dryrun:
            nchan=1 # May be wrong

        if nchan==1:
            chanmode="mfs"
        else:
            chanmode="channel"
        
        psfmode="clark"
        ftmachine="ft"

        if cleanmode=="csclean":
            imagermode='csclean'
        #if cleanmode=="clark":
        #    imagermode=""
        if cleanmode=="mosaic":
            imagermode="mosaic"
            ftmachine="mosaic" 

        # in 3.4 clean doesn't accept just any imsize
        from cleanhelper import cleanhelper
        optsize=[0,0]
        optsize[0]=cleanhelper.getOptimumSize(imsize[0])
        nksize=len(imsize)
        if nksize==1: # imsize can be a single element or array
            optsize[1]=optsize[0]
        else:
            optsize[1]=cleanhelper.getOptimumSize(imsize[1])
        if((optsize[0] != imsize[0]) or (nksize!=1 and optsize[1] != imsize[1])):
            self.msg(str(imsize)+' is not an acceptable imagesize, will use '+str(optsize)+" instead",priority="warn")
            imsize=optsize
                
        if not interactive:
            interactive=False
        # print clean inputs no matter what, so user can use them.
        # and write a clean.last file
        cleanlast=open(imagename+".clean.last","write")
        cleanlast.write('taskname            = "clean"\n')

        #self.msg("clean inputs:")        
        if self.verbose: self.msg(" ")
        if type(mstoimage)==type([]):
            cleanstr="clean(vis="+str(mstoimage)+",imagename='"+imagename+"'"
            cleanlast.write('vis                 = '+str(mstoimage)+'\n')
        else:
            cleanstr="clean(vis='"+str(mstoimage)+"',imagename='"+imagename+"'"
            cleanlast.write('vis                 = "'+str(mstoimage)+'"\n')
        cleanlast.write('imagename           = "'+imagename+'"\n')
        cleanlast.write('outlierfile         = ""\n')
        cleanlast.write('field               = "'+sourcefieldlist+'"\n')
        cleanlast.write('spw                 = ""\n')
        cleanlast.write('selectdata          = False\n')
        cleanlast.write('timerange           = ""\n')
        cleanlast.write('uvrange             = ""\n')
        cleanlast.write('antenna             = ""\n')
        cleanlast.write('scan                = ""\n')
        if nchan>1:
            cleanstr=cleanstr+",mode='"+chanmode+"',nchan="+str(nchan)
            cleanlast.write('mode                = "'+chanmode+'"\n')
            cleanlast.write('nchan               = "'+str(nchan)+'"\n')
        else:
            cleanlast.write('mode                = "mfs"\n')
            cleanlast.write('nchan               = -1\n')
        cleanlast.write('gridmode                = ""\n')
        cleanlast.write('wprojplanes             = 1\n')
        cleanlast.write('facets                  = 1\n')
        cleanlast.write('cfcache                 = "cfcache.dir"\n')
        cleanlast.write('painc                   = 360.0\n')
        cleanlast.write('epjtable                = ""\n')
        #cleanstr=cleanstr+",interpolation='nearest'"  # default change 20100518
        cleanlast.write('interpolation           = "linear"\n')
        cleanstr=cleanstr+",niter="+str(niter)
        cleanlast.write('niter                   = '+str(niter)+'\n')
        cleanlast.write('gain                    = 0.1\n')
        cleanstr=cleanstr+",threshold='"+str(threshold)+"'"
        cleanlast.write('threshold               = "'+str(threshold)+'"\n')
        cleanstr=cleanstr+",psfmode='"+psfmode+"'"
        cleanlast.write('psfmode                 = "'+psfmode+'"\n')
        if imagermode != "":
            cleanstr=cleanstr+",imagermode='"+imagermode+"'"
        cleanlast.write('imagermode              = "'+imagermode+'"\n')
        cleanstr=cleanstr+",ftmachine='"+ftmachine+"'"
        cleanlast.write('ftmachine               = "'+ftmachine+'"\n')
        cleanlast.write('mosweight               = False\n')
        cleanlast.write('scaletype               = "SAULT"\n')
        cleanlast.write('multiscale              = []\n')
        cleanlast.write('negcomponent            = -1\n')
        cleanlast.write('smallscalebias          = 0.6\n')
        cleanlast.write('interactive             = '+str(interactive)+'\n')
        if interactive:
            cleanstr=cleanstr+",interactive=True"
        if type(mask)==type(" "):
            cleanlast.write('mask                    = "'+mask+'"\n')
            cleanstr=cleanstr+",mask='"+mask+"'"
        else:
            cleanlast.write('mask                    = '+str(mask)+'\n')
            cleanstr=cleanstr+",mask="+str(mask)
        cleanlast.write('start                   = 0\n')
        cleanlast.write('width                   = 1\n')
        cleanlast.write('outframe                = ""\n')
        cleanlast.write('veltype                 = "radio"\n')
        cleanstr=cleanstr+",imsize="+str(imsize)+",cell="+str(map(qa.tos,cell))+",phasecenter='"+str(imcenter)+"'"
        cleanlast.write('imsize                  = '+str(imsize)+'\n');
        cleanlast.write('cell                    = '+str(map(qa.tos,cell))+'\n');
        cleanlast.write('phasecenter             = "'+str(imcenter)+'"\n');
        cleanlast.write('restfreq                = ""\n');
        if stokes != "I":
            cleanstr=cleanstr+",stokes='"+stokes+"'"
        cleanlast.write('stokes                  = "'+stokes+'"\n');
        cleanlast.write('weighting               = "'+weighting+'"\n');
        cleanstr=cleanstr+",weighting='"+weighting+"'"
        if weighting == "briggs":
            cleanstr=cleanstr+",robust=0.5"
            cleanlast.write('robust                  = 0.5\n');
            robust=0.5
        else:
            cleanlast.write('robust                  = 0.0\n');
            robust=0.
            
        taper=False
        if len(outertaper) >0:            
            taper=True
            if type(outertaper) == type([]):
                if len(outertaper[0])==0:
                    taper=False
        if taper:
            uvtaper=True
            cleanlast.write('uvtaper                 = True\n');
            cleanlast.write('outertaper              = "'+str(outertaper)+'"\n');
            cleanstr=cleanstr+",uvtaper=True,outertaper="+str(outertaper)+",innertaper=[]"
        else:
            uvtaper=False            
            cleanlast.write('uvtaper                 = False\n');
            cleanlast.write('outertaper              = []\n');
            cleanstr=cleanstr+",uvtaper=False"
        cleanlast.write('innertaper              = []\n');
        if os.path.exists(modelimage):
            cleanstr=cleanstr+",modelimage='"+str(modelimage)+"'"
            cleanlast.write('modelimage              = "'+str(modelimage)+'"\n');
        else:
            cleanlast.write('modelimage              = ""\n');
        cleanlast.write("restoringbeam           = ['']\n");
        cleanstr=cleanstr+",pbcor="+str(pbcor)
        cleanlast.write("pbcor                   = "+str(pbcor)+"\n");
        cleanlast.write("minpb                   = 0.2\n");
        cleanlast.write("calready                = True\n");
        cleanlast.write('noise                   = ""\n');
        cleanlast.write('npixels                 = 0\n');
        cleanlast.write('npercycle               = 100\n');
        cleanlast.write('cyclefactor             = 1.5\n');
        cleanlast.write('cyclespeedup            = -1\n');
        cleanlast.write('nterms                  = 1\n');
        cleanlast.write('reffreq                 = ""\n');
        cleanlast.write('chaniter                = False\n');
        cleanstr=cleanstr+")"        
        if self.verbose:
            # RI TODO assumed origin is simanalyze
            self.msg(cleanstr,priority="warn",origin="simanalyze")
        else:
            self.msg(cleanstr,priority="info",origin="simanalyze")
        cleanlast.write("#"+cleanstr+"\n")
        cleanlast.close()
        
        if not dryrun:
            clean(vis=mstoimage, imagename=imagename, mode=chanmode, 
              niter=niter, threshold=threshold, selectdata=False, nchan=nchan,
              psfmode=psfmode, imagermode=imagermode, ftmachine=ftmachine, 
              imsize=imsize, cell=map(qa.tos,cell), phasecenter=imcenter,
              stokes=stokes, weighting=weighting, robust=robust,
              interactive=interactive,
              uvtaper=uvtaper,outertaper=outertaper, pbcor=True, mask=mask,
              modelimage=modelimage)

            del freq,nchan # something is holding onto the ms in table cache







    ###################################################

    def flatimage(self,image,complist="",verbose=False):
        # flat output 
        ia.open(image)
        imsize=ia.shape()
        imcsys=ia.coordsys()
        ia.done()
        spectax=imcsys.findcoordinate('spectral')[1]
        nchan=imsize[spectax]
        stokesax=imcsys.findcoordinate('stokes')[1]
        nstokes=imsize[stokesax]

        flat=image+".flat"
        if nchan>1:
            if verbose: self.msg("creating moment zero image "+flat,origin="flatimage")
            ia.open(image)
            flat_ia = ia.moments(moments=[-1],outfile=flat,overwrite=True)
            ia.done()
            flat_ia.close()
            del flat_ia
        else:
            if verbose: self.msg("removing degenerate image axes in "+flat,origin="flatimage")
            # just remove degenerate axes from image
            flat_ia = ia.newimagefromimage(infile=image,outfile=flat,dropdeg=True,overwrite=True)
            flat_ia.close()

            # seems no way to just drop the spectral and keep the stokes. 
            if nstokes<=1:
                os.rename(flat,flat+".tmp")
                ia.open(flat+".tmp")
                flat_ia = ia.adddegaxes(outfile=flat,stokes='I',overwrite=True)
                ia.done()
                flat_ia.close()
                shutil.rmtree(flat+".tmp")
            del flat_ia

        if nstokes>1:
            os.rename(flat,flat+".tmp")
            po.open(flat+".tmp")
            foo=po.stokesi(outfile=flat)
            foo.done()
            po.done()
            shutil.rmtree(flat+".tmp")

        imcsys.done()
        del imcsys

        # add components 
        if len(complist)>0:
            ia.open(flat)
            if not os.path.exists(complist):
                self.msg("sky component list "+str(complist)+" not found in flatimage",priority="error")
            cl.open(complist)
            ia.modify(cl.torecord(),subtract=False) 
            cl.done()
            ia.done()




    ###################################################

    def convimage(self,modelflat,outflat,complist=""):
        # regrid flat input to flat output shape and convolve
        modelregrid = modelflat+".regrid"
        # get outflatcoordsys from outflat
        ia.open(outflat)
        outflatcs=ia.coordsys()
        outflatshape=ia.shape()
        # and beam TODO is beam the same in flat as a cube?
        beam=ia.restoringbeam()
        ia.done()            

        ia.open(modelflat)
        modelflatcs=ia.coordsys()
        modelflatshape=ia.shape()
        tmpxx=ia.regrid(outfile=modelregrid+'.tmp', overwrite=True,
                  csys=outflatcs.torecord(),shape=outflatshape, asvelocity=False)
        # im.regrid assumes a surface brightness, or more accurately doesnt
        # pay attention to units at all, so we now have to scale 
        # by the pixel size to have the right values in jy/pixel, 

        # get pixel size from model image coordsys
        tmpxx.done()
        increments=outflatcs.increment(type="direction")['numeric']
        incellx=qa.quantity(abs(increments[0]),outflatcs.units(type="direction")[0])
        incelly=qa.quantity(abs(increments[1]),outflatcs.units(type="direction")[1])
        xform=outflatcs.lineartransform(type="direction")
        offdiag=max(abs(xform[0,1]),abs(xform[1,0]))
        if offdiag > 1e-4:
            self.msg("Your image is rotated with respect to Lat/Lon.  I can't cope with that yet",priority="error")
        incellx=qa.mul(incellx,abs(xform[0,0]))
        incelly=qa.mul(incelly,abs(xform[1,1]))
        model_cell = [qa.convert(incellx,'arcsec'),qa.convert(incelly,'arcsec')]

        # and from outflat (the clean image)
        increments=outflatcs.increment(type="direction")['numeric']
        incellx=qa.quantity(abs(increments[0]),outflatcs.units(type="direction")[0])
        incelly=qa.quantity(abs(increments[1]),outflatcs.units(type="direction")[1])
        xform=outflatcs.lineartransform(type="direction")
        offdiag=max(abs(xform[0,1]),abs(xform[1,0]))
        if offdiag > 1e-4:
            self.msg("Your image is rotated with respect to Lat/Lon.  I can't cope with that yet",priority="error")
        incellx=qa.mul(incellx,abs(xform[0,0]))
        incelly=qa.mul(incelly,abs(xform[1,1]))
        cell = [qa.convert(incellx,'arcsec'),qa.convert(incelly,'arcsec')]
        
        # image scaling
        factor  = (qa.convert(cell[0],"arcsec")['value'])
        factor *= (qa.convert(cell[1],"arcsec")['value']) 
        factor /= (qa.convert(model_cell[0],"arcsec")['value']) 
        factor /= (qa.convert(model_cell[1],"arcsec")['value'])         
        imrr = ia.imagecalc(modelregrid, 
                            "'%s'*%g" % (modelregrid+'.tmp',factor), 
                            overwrite = True)
        shutil.rmtree(modelregrid+".tmp")
        if self.verbose:
            self.msg("scaling model by pixel area ratio %g" % factor,origin='convimage')

        # add unresolved components in Jy/pix
        # don't do this if you've already done it in flatimage()!
        if (os.path.exists(complist)):            
            cl.open(complist)
            imrr.modify(cl.torecord(),subtract=False)
            cl.done()
            
        imrr.done()    
        ia.done()
        del imrr

        # Convolve model with beam.
        convolved = modelregrid + '.conv'
        ia.open(modelregrid)
        ia.setbrightnessunit("Jy/pixel")
        tmpcnv=ia.convolve2d(convolved,major=beam['major'],minor=beam['minor'],
                      pa=beam['positionangle'],overwrite=True)
        ia.done()
        
        #tmpcnv.open(convolved)
        tmpcnv.setbrightnessunit("Jy/beam")
        tmpcnv.setrestoringbeam(beam=beam)
        tmpcnv.done()


    def bandname(self, freq):
        """
        Given freq in GHz, return the silly and in some cases deliberately
        confusing band name that some people insist on using.
        """
        # TODO: add a trivia argument to optionally provide the historical
        #       radar band info from Wikipedia.
        band = ''
        if freq > 90:   # Use the ALMA system.
            band = 'band%.0f' % (0.01 * freq)   # () are mandatory!
        # Now switch to radar band names.  Above 40 GHz different groups used
        # different names.  Wikipedia uses ones from Baytron, a now defunct company
        # that made test equipment.
        elif freq > 75:    # used as a visual sensor for experimental autonomous vehicles
            band = 'W' 
        elif freq > 50:    # Very strongly absorbed by atmospheric O2,
            band = 'V'     # which resonates at 60 GHz.
        elif freq >= 40:
            band = 'Q'
        elif freq >= 26.5: # mapping, short range, airport surveillance;
            band = 'Ka'    # frequency just above K band (hence 'a')
                           # Photo radar operates at 34.300 +/- 0.100 GHz.
        elif freq >= 18:
            band = 'K'     # from German kurz, meaning 'short'; limited use due to
                           # absorption by water vapor, so Ku and Ka were used
                           # instead for surveillance. Used for detecting clouds
                           # and at 24.150 +/- 0.100 GHz for speeding motorists.
        elif freq >= 12:
            band = 'U'     # or Ku
        elif freq >= 8:    # Missile guidance, weather, medium-resolution mapping and ground
            band = 'X'     # surveillance; in the USA the narrow range 10.525 GHz +/- 25 MHz
                           # is used for airport radar and short range tracking.  Named X
                           # band because the frequency was a secret during WW2.
        elif freq >= 4:
            band = 'C'     # Satellite transponders; a compromise (hence 'C') between X
                           # and S bands; weather; long range tracking
        elif freq >= 2:
            band = 'S'     # Moderate range surveillance, air traffic control,
                           # long-range weather; 'S' for 'short'
        elif freq >= 1:
            band = 'L'     # Long range air traffic control and surveillance; 'L' for 'long'
        elif freq >= 0.3:
            band = 'UHF'
        else:
            band = 'P'     # for 'previous', applied retrospectively to early radar systems
                           # Includes HF and VHF.  Worse, it leaves no space for me
                           # to put in rock band easter eggs.
        return band


    def polsettings(self, telescope, poltype=None, listall=False):
        """
        Returns stokes parameters (for use as stokes in sm.setspwindow)
        and feed type (for use as mode in sm.setfeed).

        If poltype is not specified or recognized, a guess is made using
        telescope.  Defaults to 'XX YY', 'perfect X Y'

        If listall is True, return the options instead.
        """
        psets = {'circular': ('RR LL', 'perfect R L'),
                 'linear':   ('XX YY', 'perfect X Y'),
                 'RR':       ('RR',    'perfect R L')}
        if listall:
            return psets
        if poltype not in psets:
            poltype = 'linear'
            for circ in ('VLA', 'DRAO'):       # Done this way to
                if circ in telescope.upper():  # catch EVLA.
                    poltype = 'circular'
        return psets[poltype]

#######################################
# ALMA calcpointings
        
    def applyRotate(self, x, y, tcos, tsin):     
        return tcos*x-tsin*y, tsin*x+tcos*y
    
    def isEven(self, num):
        return (num % 2) == 0

    # this was the only algorithm in Cycle 0 - for Cycle 1 it was 
    # recast as BaseTriangularTiling.getPointings(), the width>height 
    # case statement was removed, and BaseTriangularTiling was supplemented by
    # ShiftTriangularTiling.getPointings()
    def getTrianglePoints(self, width, height, angle, spacing):        
        tcos = pl.cos(angle*pl.pi/180)
        tsin = pl.sin(angle*pl.pi/180)

        xx=[]
        yy=[]

        if (width >= height):
            wSpacing = spacing
            hSpacing = (pl.sqrt(3.) / 2) * spacing
      
            nwEven = int(pl.floor((width / 2) / wSpacing))
            nwOdd  = int(pl.floor((width / 2) / wSpacing + 0.5))
            nh     = int(pl.floor((height / 2) / hSpacing))

            for ih in pl.array(range(nh*2+1))-nh:
                if (self.isEven(ih)):
                    for iw in pl.array(range(nwEven*2+1))-nwEven:
                        x,y = self.applyRotate(iw*wSpacing, ih*hSpacing, tcos, tsin)
                        xx.append(x)
                        yy.append(y)          
                else:
                    for iw in pl.array(range(nwOdd*2+1))-nwOdd:
                        x,y = self.applyRotate((iw+0.5)*wSpacing, ih*hSpacing, tcos, tsin)
                        xx.append(x)
                        yy.append(y)
        else:
            wSpacing = (pl.sqrt(3) / 2) * spacing
            hSpacing = spacing
      
            nw     = int(pl.floor((width / 2) / wSpacing))
            nhEven = int(pl.floor((height / 2) / hSpacing))
            nhOdd  = int(pl.floor((height / 2) / hSpacing + 0.5))

            for iw in pl.array(range(nw*2+1))-nw:                
                if (self.isEven(iw)):
                    for ih in pl.array(range(nhEven*2+1))-nhEven:
                        x,y = self.applyRotate(iw*wSpacing, ih*hSpacing, tcos, tsin)
                        xx.append(x)
                        yy.append(y)          
                else:                    
                    for ih in pl.array(range(nhOdd*2+1))-nhOdd:
                        x,y = self.applyRotate(iw*wSpacing, (ih+0.5)*hSpacing, tcos, tsin)
                        xx.append(x)
                        yy.append(y)          
        return xx,yy



    def getTriangularTiling(self, longlength, latlength, angle, spacing, pb):

        # OT if isLandscape, shortside=Latlength
        # else isLandscape=false, shortside=Longlength

        if longlength > latlength: # OT isLandscape=True (OT uses >= )
            width=longlength # arcsec
            height=latlength # arcsec
            shortside=height
        else:
            width=latlength
            height=longlength
            shortside=height
            angle=angle+90
        
        # which tiling? Base or Shifted (OT getTiling)
        vSpacing = (pl.sqrt(3) / 2) * spacing
        n = pl.ceil(shortside / vSpacing)

        if n % 2 == 0:
            return self.getShiftTriangularTiling(width, height, angle, spacing, pb)
        else:
            return self.getBaseTriangularTiling(width, height, angle, spacing, pb)

    def needsFiller(self, length, spacing, bs, npoints):
        if length > spacing * (npoints - 1) + bs:
            return 1
        else:
            return 0

    def getBaseTriangularTiling(self, width, height, angle, spacing, pb):
        tcos = pl.cos(angle*pl.pi/180)
        tsin = pl.sin(angle*pl.pi/180)

        xx=[]
        yy=[]

        wSpacing = spacing
        hSpacing = (pl.sqrt(3.) / 2) * spacing
      
        nwEven = int(pl.floor((width / 2) / wSpacing))
        nwEven += self.needsFiller(width, wSpacing, pb, nwEven*2+1)

        nwOdd  = int(pl.floor((width / 2) / wSpacing + 0.5))
        nwOdd += self.needsFiller(width, wSpacing, pb, nwOdd*2)

        nh     = int(pl.floor((height / 2) / hSpacing))
        nh += self.needsFiller(height, hSpacing, pb, nh*2+1)

        for ih in pl.array(range(nh*2+1))-nh:
            if (self.isEven(ih)):
                for iw in pl.array(range(nwEven*2+1))-nwEven:
                    x,y = self.applyRotate(iw*wSpacing, ih*hSpacing, tcos, tsin)
                    xx.append(x)
                    yy.append(-y) # will require additional testing @ angle>0
            else:
                for iw in pl.array(range(nwOdd*2))-nwOdd:
                    x,y = self.applyRotate((iw+0.5)*wSpacing, ih*hSpacing, tcos, tsin)
                    xx.append(x)
                    yy.append(-y)
        
        return xx,yy




    def getShiftTriangularTiling(self, width, height, angle, spacing, pb):
        tcos = pl.cos(angle*pl.pi/180)
        tsin = pl.sin(angle*pl.pi/180)

        xx=[]
        yy=[]

        wSpacing = spacing
        hSpacing = (pl.sqrt(3.) / 2) * spacing
      
        nwEven = int(pl.floor((width / 2) / wSpacing + 0.5))
        nwEven += self.needsFiller(width, wSpacing, pb, nwEven*2)

        nwOdd  = int(pl.floor((width / 2) / wSpacing ))
        nwOdd += self.needsFiller(width, wSpacing, pb, nwOdd*2+1)

        nh     = int(pl.floor((height - hSpacing) / 2 / hSpacing +1))
        nh += self.needsFiller(height, hSpacing, pb, nh*2)

        for ih in pl.array(range(nh*2))-nh:
            if (self.isEven(ih)):
                for iw in pl.array(range(nwEven*2))-nwEven:
                    x,y = self.applyRotate((iw+0.5)*wSpacing, (ih+0.5)*hSpacing, tcos, tsin)
                    xx.append(x)
                    yy.append(-y)          
            else:
                for iw in pl.array(range(nwOdd*2+1))-nwOdd:
                    x,y = self.applyRotate(iw*wSpacing, (ih+0.5)*hSpacing, tcos, tsin)
                    xx.append(x)
                    yy.append(-y)
        
        return xx,yy




######################################
    # adapted from aU.getBaselineStats
    def baselineLengths(self, configfile):
        stnx, stny, stnz, stnd, padnames, nAntennas, telescopename = self.readantenna(configfile)
        
        cx=pl.mean(stnx)
        cy=pl.mean(stny)
        cz=pl.mean(stnz)
        lat,lon,el = self.itrf2loc(stnx,stny,stnz,cx,cy,cz)
        
        #l = {}
        #neighborlist = []
        maxlength = 0
        minlength = 1e9
        #mylengths = pl.zeros([nAntennas,nAntennas])
        mylengths=pl.zeros(nAntennas*(nAntennas-1)/2)
        k=0

        for i in range(nAntennas):
            for j in range(i+1,nAntennas):
                x = lat[i]-lat[j]
                y = lon[i]-lon[j]
                z =  el[i]- el[j]
                #mylengths[i][j] = (x**2 + y**2 + z**2)**0.5
                mylengths[k]=(x**2 + y**2 + z**2)**0.5
                k=k+1
        
        return mylengths


######################
    def approxBeam(self,configfile,freq):
        # freq must be in GHz
        mylengths=self.baselineLengths(configfile)
        rmslength = pl.sqrt(pl.mean(mylengths.flatten()**2))
        from scipy.stats import scoreatpercentile
        ninety = scoreatpercentile(mylengths, 90)

        return 0.2997924/freq/ninety*3600.*180/pl.pi # lambda/b converted to arcsec

        
######################
    def sfBeam1d(self,beam="", cell="", convsupport=-1, sampling=""):
        """
        Calculates PSF of image generated by gridfunction 'SF'.
        Migrated codes from gjinc.sfBeam() in analysisUtil module
        by Todd Hunter.
        Note: this is simplified version of the function.
        
        Paramters:
           beam        : Antenna primary beam (quantity)
           cell        : Cell size of image (quantity)
           convsupport : convolution support used for imaging (integer)
           sampling    : pointing spacing of observation (quantity).
                         If not defined, comvolution of sampling kernel is
                         skipped with warning.
        Returns:
           Estimated PSF of image (quantity).
        """
        import scipy.special as spspec
        import scipy.signal as spsig
        import scipy.interpolate as spintrp
        
        if not qa.compare(beam, "rad"):
            raise ValueError, "beam should be a quantity of antenna primary beam size (angle)"
        if not qa.compare(cell, "rad"):
            raise ValueError, "cell should be a quantity of image pixel size (angle)"
        if len(sampling) > 0 and not qa.compare(sampling, "rad"):
            raise ValueError, "sampling should be a quantity of pointing spacing (angle)"
        if (convsupport < -1):
            convsupport = 3

        supportwidth = (convsupport*2 + 1)
        c = supportwidth * pl.pi/2.
        pb_asec = qa.getvalue(qa.convert(beam, "arcsec"))
        cell_asec = qa.getvalue(qa.convert(cell, "arcsec"))
        samp_asec = -1.
        if len(sampling) > 0:
            samp_asec = qa.getvalue(qa.convert(sampling, "arcsec"))
        # Define kernel array
        myxaxis = pl.arange(-3*pb_asec,3*pb_asec+0.1,0.2)
        # Gaussian func of PB
        scale = 0.5 / ( (pb_asec/2.3548201)**2 )
        mygaussian = pl.exp(- scale * myxaxis**2 ) ### exp(x**2/(2*sigma**2))
        # Spheroidal kernel func
        sfaxis = myxaxis/float((supportwidth-1)*cell_asec/2.0)
        indices = pl.where(abs(sfaxis)<1)[0]
        centralRegion = sfaxis[indices]
        centralRegionY = spspec.pro_ang1_cv(0, 0, c, spspec.pro_cv(0,0,c),
                                            centralRegion)[0]
        mysf = pl.zeros(len(myxaxis))
        mysf[indices] += centralRegionY/max(centralRegionY)
        # Convolution of Gaussian PB and SF
        result = spsig.convolve(mysf, mygaussian, mode='same')
        del mygaussian, sfaxis, indices, centralRegion, centralRegionY
        # Sampling function
        if samp_asec > 0:
            myboxcar = pl.zeros(len(myxaxis))
            indices = pl.where(abs(myxaxis)<samp_asec/2.)[0]
            myboxcar[indices] = 1
            result = spsig.convolve(result, myboxcar, mode='same')
        else:
            self.msg("Pointing spacing is not specified. Calculated PSF could be less accurate.", priority="warn", origin="sfBeam1d")
        # Calculate FWHM
        result /= max(result)
        halfmax = max(result)*0.5
        spline = spintrp.UnivariateSpline(myxaxis, result-halfmax, s=0)
        x0, x1 = spline.roots()
        del result, myxaxis, spline
        return qa.quantity(abs(x1-x0), "arcsec")
