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
        self.pmulti=0  # rows, cols, currsubplot


    def newfig(self,multi=0,show=True):  # new graphics window/file
        if show:
            pl.ion() # creates a fig if ness
        pl.clf() 
        pl.ioff() # just in case

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
        

        
    ###########################################################

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
                    toterm=False  # casalog spews severe to term already
                else:
                    if not (priority=="DEBUG" or priority[:-1]=="DEBUG"):
                        priority="INFO"
        bw="\x1b[0m"
        if origin==None:
            origin="simdata"            
        if toterm:
            print clr+"["+origin+"] "+bw+s
        casalog.post(s,priority=priority,origin=origin)
        if priority=="ERROR":
            return False



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
        if not me.measure(y):
            if halt:
                self.msg("can't interpret '"+str(s)+"' as a direction",priority="error")
            return False
        return True

    ###########################################################

    def ismstp(self,s,halt=False):
        try:
            istp=False
            # check if the ms is tp data or not.
            tb.open(s+'/ANTENNA')
            antname=tb.getcol('NAME')
            tb.close()
            if antname[0].find('TP') > -1: istp=True
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

    def statim(self,image,plot=True,incell=None,disprange=None,bar=True):
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
        if type(im_min)==type([]) or type(im_min)==type(plarr):
            if len(im_min)<1: im_min=0.
        im_max=stats['max']*toJypix
        if type(im_max)==type([]) or type(im_min)==type(plarr):
            if len(im_max)<1: im_max=1.
        imsize=ia.shape()[0:2]
        reg1=rg.box([0,0],[imsize[0]*.25,imsize[1]*.25])
        stats=ia.statistics(region=reg1,verbose=False,list=False)
        im_rms=stats['rms']*toJypix
        if type(im_rms)==type([]) or type(im_min)==type(plarr):
            if len(im_rms)==0: im_rms=0.
        data_array=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],[1],[],True,True,False)
        data_array=pl.array(data_array)
        tdata_array=pl.transpose(data_array)
        ttrans_array=tdata_array.tolist()
        ttrans_array.reverse()
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
        imhist=ia.histograms(cumu=True,nbins=nbin,list=False)['histout']
        ii=0
        lowcounts=imhist['counts'][ii]
        while imhist['counts'][ii]<0.005*lowcounts and ii<nbin: 
            ii=ii+1
        lowvalue=imhist['values'][ii]
        ii=nbin-1
        highcounts=imhist['counts'][ii]
        while imhist['counts'][ii]>0.995*highcounts and ii>0: 
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
            pl.title(image,fontsize="x-small")
            pl.text(0.05,0.95,"min=%7.1e\nmax=%7.1e\nRMS=%7.1e" % (im_min,im_max,im_rms),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
            if bar:
                cb=pl.colorbar(pad=0)
                cl = pl.getp(cb.ax,'yticklabels')
                pl.setp(cl,fontsize='x-small')
        ia.done()
        return im_min,im_max,im_rms







    ###########################################################

    # WARNING:  this will dissapear in favor of calc_pointings2

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
        ####could not fit pointings then single pointing
        if(len(pointings)==0):
            pointings.append(direction)
        self.msg("using %i generated pointing(s)" % len(pointings))
        self.pointings=pointings
        return len(pointings), pointings, [0.]*len(pointings)





    ###########################################################
    # new version - in simdata2, we don't need the file reading here

    def calc_pointings2(self, spacing, size, maptype="hex", direction=None, relmargin=0.5):
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
        

        # if could not fit any pointings, then return single pointing
        if(len(pointings)==0):
            pointings.append(direction)

        self.msg("using %i generated pointing(s)" % len(pointings))
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
            s="No valid point is found in input file"
            self.msg(s,priority="error")
        self.msg("read in %i pointing(s) from file" % len(pointings))
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
        f.write('#Epoch     RA          DEC      TIME\n')
        if type(pointings)!=type([]):
            pointings=[pointings]
        npos=len(pointings)
        if type(time)!=type([]):
            time=[time]
        if len(time)!=npos:
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
    
    











    ###########################################################
    #===================== ephemeris ==========================


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



















    ###########################################################
    #==========================================================
    
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














    ###########################################################

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



















    ##################################################################
    # fit modelimage into a 4 coordinate image defined by the parameters

    # WARNING: image4d will be removed in favor of modifymodel

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
            
            model_refdir='J2000 '+qa.formxxx(ra,format='hms',prec=5)+" "+qa.formxxx(dec,format='dms',prec=5)
            axmap[0]=0 # direction in first two pixel axes
            axmap[1]=1
            axassigned[0]=0  # coordinate corresponding to first 2 pixel axes
            axassigned[1]=0
             
        else:  # get model_refdir from CoordSys:
            if not in_dir['return']:
                self.msg("You don't have direction coordinates that I can understand, so either edit the header or set ignorecoord=True",priority="error")
                return False            
            ra,dec = in_csys.referencevalue(type="direction")['numeric']
            model_refdir= in_csys.referencecode(type="direction")+" "+qa.formxxx(str(ra)+"rad",format='hms',prec=5)+" "+qa.formxxx(str(dec)+"rad",format='dms',prec=5)
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
        if not spc['return']: return (0,0)
        model_width=str(cs.increment(type="spectral")['numeric'][0])+cs.units(type="spectral")
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
        if not (dir['return'] and spc['return'] and stk['return']): return False
        if dir['pixel'].__len__() != 2: return False
        if type(spc['pixel']) == type([]): return False
        if type(stk['pixel']) == type([]): return False
        cs.done()
        return True

    ##################################################################
    # fit modelimage into a 4 coordinate image defined by the parameters
    
    # TODO spectral extrapolation and regridding using innchan ****

    def modifymodel(self, inimage, outimage, 
                modifymodel,inbright,
                direction,incell,incenter,inwidth,innchan,
                flatimage=False):  # if nonzero, create mom -1 image 

        # new ia tool
        in_ia=ia.newimagefromfile(inimage)            
        in_shape=in_ia.shape()
        in_csys=in_ia.coordsys()

        # cell size:  from incell param, or from image
        model_cell=qa.quantity('0arcsec')
        if modifymodel:
            if type(incell) == type([]):
                if len(incell)>0:
                    model_cell =  map(qa.convert,incell,['arcsec','arcsec'])
            else:                
                if len(incell)>0:
                    model_cell = qa.abs(qa.convert(incell,'arcsec'))
            model_cell = [model_cell,model_cell]

            # in modifymodel ONLY, model_cell[0]<0 for RA increasing left
            if qa.gt(qa.convert(model_cell[0],'arcsec'),"0arcsec"):
                model_cell[0]=qa.mul(model_cell[0],-1)
            if qa.lt(qa.convert(model_cell[1],'arcsec'),"0arcsec"):
                model_cell[1]=qa.abs(model_cell[1])

        # if incell fails to convert to a sensible model_cell, or if we're not 
        # modifying the model, we need to get model_cell from the image coordsys
        if (not modifymodel) or (model_cell[1]['value']<=0):
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

        if self.verbose:
            self.msg("model image shape= %s" % in_shape,origin="setup model")
            self.msg("model pixel = %8.2e x %8.2e arcsec" % (model_cell[0]['value'],model_cell[1]['value']),origin="setup model")



        # brightness scaling 
        # we can in principal change inbright even if modifymodel=F
        if (inbright=="unchanged") or (inbright==""):
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

        if outimage!=inimage:
            if self.verbose: self.msg("rearranging input data (may take some time for large cubes)")
            arr=in_ia.getchunk()
        else:
            arr=pl.zeros(in_shape)
        axmap=[-1,-1,-1,-1]
        axassigned=[-1,-1,-1,-1]

        # first check number of pixel axes and raise to 4 if required
        in_nax=arr.shape.__len__()
        if in_nax<2:
            self.msg("Your input model has fewer than 2 dimensions.  Can't proceed",priority="error")
            return False
        if in_nax==2:            
            arr=arr.reshape([arr.shape[0],arr.shape[1],1])
            in_shape=arr.shape
            in_nax=in_shape.__len__() # which should be 3
        if in_nax==3:
            arr=arr.reshape([arr.shape[0],arr.shape[1],arr.shape[2],1])
            in_shape=arr.shape
            in_nax=in_shape.__len__() # which should be 3
        if in_nax>4:
            self.msg("model image has more than 4 dimensions.  Not sure how to proceed",priority="error")
            return False


        # now parse coordsys:
        model_refdir=""
        #model_refpix=[]
        # look for direction coordinate, with two pixel axes:
        if in_dir['return']:
            # ra,dec = in_csys.referencevalue(type="direction")['numeric']
            # model_refpix = in_csys.referencepixel(type="direction")['numeric'].tolist()            
            # model_refdir= in_csys.referencecode(type="direction")+" "+qa.formxxx(str(ra)+"rad",format='hms',prec=5)+" "+qa.formxxx(str(dec)+"rad",format='dms',prec=5)
            # ra=qa.quantity(str(ra)+"rad")
            # dec=qa.quantity(str(dec)+"rad")
            in_ndir = in_dir['pixel'].__len__() 
            if in_ndir != 2:
                if modifymodel:
                    self.msg("Mal-formed direction coordinates in modelimage. Discarding and using first two pixel axes for RA and Dec.")
                    axmap[0]=0 # direction in first two pixel axes
                    axmap[1]=1
                    axassigned[0]=0  # coordinate corresponding to first 2 pixel axes
                    axassigned[1]=0
                else:
                    self.msg("I can't understand your direction coordinates, so either edit the header or set modifymodel=True",priority="error")
                    return False
            else:
                # here, it doesn't matter if we're modifying or not, we have found some 
                # direction axes, and may change their increments and direction or not.
                dirax=in_dir['pixel']
                axmap[0]=dirax[0]
                axmap[1]=dirax[1]                    
                axassigned[dirax[0]]=0
                axassigned[dirax[1]]=0
                if self.verbose: self.msg("Direction coordinate (%i,%i) parsed" % (axmap[0],axmap[1]),origin="setup model")

            # move model_refdir to center of image
            model_refpix=[0.5*in_shape[axmap[0]],0.5*in_shape[axmap[1]]]
            ra,dec = in_ia.toworld(model_refpix)['numeric'][0:2]
            ra=qa.quantity(str(ra)+"rad")
            dec=qa.quantity(str(dec)+"rad")
            model_refdir= in_csys.referencecode(type="direction")+" "+qa.formxxx(ra,format='hms',prec=5)+" "+qa.formxxx(dec,format='dms',prec=5)

        else:
            axmap[0]=0 # direction in first two pixel axes
            axmap[1]=1
            axassigned[0]=0  # coordinate corresponding to first 2 pixel axes
            axassigned[1]=0

        if modifymodel:
            # try to parse direction using splitter function and set model_refdir if success
            if type(direction)==type([]):
                if len(direction) > 1:
                    self.msg("error parsing direction "+str(direction)+" -- should be a single direction string")
                    return False
                else:
                    direction=direction[0]
            if self.isdirection(direction,halt=False):
                epoch, ra, dec = self.direction_splitter(direction)

                #if self.verbose: self.msg("setting model image direction to ra="+qa.angle(qa.div(ra,"15"))+" dec="+qa.angle(dec),origin="setup model")            
                model_refdir='J2000 '+qa.formxxx(ra,format='hms',prec=5)+" "+qa.formxxx(dec,format='dms',prec=5)
                model_refpix=[0.5*in_shape[axmap[0]],0.5*in_shape[axmap[1]]]
        if model_refdir=="":
            self.msg("Model ref direction undefined.  Either set direction and modifymodel=T, or edit the image header",priority="error")
            return False        

        # we've now found or assigned two direction axes.

        model_center=""
        model_width=""
        # look for a spectral axis:
        if in_spc['return']:
            if type(in_spc['pixel']) == type(1) :
                foo=in_spc['pixel']
            else:
                foo=in_spc['pixel'][0]
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
            model_center=model_start+0.5*model_nchan*model_width
            model_width=str(model_width)+in_csys.units(type="spectral")
            model_start=str(model_start)+in_csys.units(type="spectral")
            model_center=str(model_center)+in_csys.units(type="spectral")
            add_spectral_coord=False
            if self.verbose: self.msg("Spectral Coordinate %i parsed" % axmap[3],origin="setup model")                
        else:
            # need to add one to the coordsys 
            add_spectral_coord=True 

        if modifymodel:
            if self.isquantity(incenter,halt=False):
                foo=qa.quantity(incenter)
                if foo['value']>=0:
                    model_center=incenter
                    model_restfreq=model_center
            if self.isquantity(inwidth,halt=False):
                model_width=inwidth

        if model_width=="" or model_center=="":
            self.msg("Model width or center undefined.  Either set incenter, inwidth and modifymodel=T, or edit the image header",priority="error")
            return False        


        model_stokes=""
        # look for a stokes axis
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
                self.msg("I can't find an unused axis to make Stokes [%i %i %i %i] " % (axassigned[0],axassigned[1],axassigned[2],axassigned[3]),priority="error",origin="setup model")
                return False
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


        # now we should have 4 assigned pixel axes, and model_cell, model_refdir, model_nchan, 
        # model_stokes all set either from modifymodel or from input image
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
        raq=dirm['m0']        
        deq=dirm['m1']        
        modelcsys.setreferencevalue(
            [qa.convert(raq,modelcsys.units()[0])['value'],
             qa.convert(deq,modelcsys.units()[1])['value']],
            type="direction")
        modelcsys.setreferencepixel(
            model_refpix,
#            [0.5*in_shape[axmap[0]],0.5*in_shape[axmap[1]]],
            "direction")
        if self.verbose: 
            self.msg("sky model image direction = "+model_refdir)
            self.msg("sky model image increment = "+str(model_cell))

        modelcsys.setspectral(refcode="LSRK",restfreq=model_restfreq)
        modelcsys.setreferencevalue(qa.convert(model_center,modelcsys.units()[3])['value'],type="spectral")
        modelcsys.setreferencepixel(0.5*model_nchan,type="spectral") # default is middle chan
        modelcsys.setincrement(qa.convert(model_width,modelcsys.units()[3])['value'],type="spectral")
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


        # there's got to be a better way to remove NaNs: :)
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
                msg("your model is large - predicting visibilities may take a while.",priority="warn")

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

        return model_refdir,model_cell,model_size,model_nchan,model_center,model_width,model_stokes













    ##################################################################
    # image/clean subtask

    def image(self,mstoimage,image,
              cleanmode,cell,imsize,imcenter,niter,threshold,weighting,
              outertaper,stokes,sourcefieldlist="",modelimage=""):
        from clean import clean

        # determine channelization from (first) ms:
        if type(mstoimage)==type([]):
            ms0=mstoimage[0]
        else:
            ms0=mstoimage
        
        tb.open(ms0+"/SPECTRAL_WINDOW")
        if tb.nrows() > 1:
            self.msg("determining output cube parameters from FIRST of several SPW in MS "+ms0)
        freq=tb.getvarcol("CHAN_FREQ")['r1'][0]
        nchan=len(freq)
        tb.done()

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

        
        # print clean inputs no matter what, so user can use them.
        # and write a clean.last file
        cleanlast=open("clean.last","write")
        cleanlast.write('taskname            = "clean"\n')

        self.msg("clean inputs:")
        cleanstr="clean(vis='"+', '.join(mstoimage)+"',imagename='"+image+"'"
        cleanlast.write('vis                 = "'+', '.join(mstoimage)+'"\n')
        cleanlast.write('imagename           = "'+image+'"\n')
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
        cleanlast.write('interpolation           = "nearest"\n')
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
        cleanlast.write('interactive             = False\n')
        cleanlast.write('mask                    = []\n')
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
            
        if outertaper != []:
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
            cleanstr=cleanstr+",modelimage="+str(modelimage)
            cleanlast.write('modelimage              = "'+str(modelimage)+'"\n');
        else:
            cleanlast.write('modelimage              = ""\n');
        cleanlast.write("restoringbeam           = ['']\n");
        cleanlast.write("pbcor                   = False\n");
        cleanlast.write("minpb                   = 0.1\n");
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
        self.msg(cleanstr,priority="warn")
        cleanlast.write("#"+cleanstr+"\n")
        cleanlast.close()
        
        clean(vis=', '.join(mstoimage), imagename=image, mode=chanmode, nchan=nchan,
                  niter=niter, threshold=threshold, selectdata=False,
                  psfmode=psfmode, imagermode=imagermode, ftmachine=ftmachine, 
                  imsize=imsize, cell=map(qa.tos,cell), phasecenter=imcenter,
                  stokes=stokes, weighting=weighting, robust=robust,
                  uvtaper=uvtaper,outertaper=outertaper)








    ###################################################

    def flatimage(self,image,complist="",verbose=False):
        # flat output 
        ia.open(image)
        imsize=ia.shape()
        imcsys=ia.coordsys()
        ia.done()
        spectax=imcsys.findcoordinate('spectral')['pixel']
        nchan=imsize[spectax]
        stokesax=imcsys.findcoordinate('stokes')['pixel']
        nstokes=imsize[stokesax]

        flat=image+".flat"
        if nchan>1:
            if verbose: self.msg("creating moment zero image "+flat,origin="analysis")
            ia.open(image)
            ia.moments(moments=[-1],outfile=flat,overwrite=True)
            ia.done()
        else:
            if verbose: self.msg("removing degenerate image axes in "+flat,origin="analysis")
            # just remove degenerate axes from image
            ia.newimagefromimage(infile=image,outfile=flat,dropdeg=True,overwrite=True)
            # seems no way to just drop the spectral and keep the stokes. 
            if nstokes<=1:
                os.rename(flat,flat+".tmp")
                ia.open(flat+".tmp")
                ia.adddegaxes(outfile=flat,stokes='I',overwrite=True)
                ia.done()
                shutil.rmtree(flat+".tmp")
        if nstokes>1:
            os.rename(flat,flat+".tmp")
            po.open(flat+".tmp")
            foo=po.stokesi(outfile=flat,stokes='I')
            foo.done()
            po.done()
            shutil.rmtree(flat+".tmp")

        imcsys.done()
        del imcsys

        # add components 
        if len(complist)>0:
            ia.open(flat)
            if not os.path.exists(complist):
                self.msg("sky component list "+str(complist)+" not found in flatimge",priority="error")
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
        ia.regrid(outfile=modelregrid+'.tmp', overwrite=True,
                  csys=outflatcs.torecord(),shape=outflatshape)
        # im.regrid assumes a surface brightness, or more accurately doesnt
        # pay attention to units at all, so we now have to scale 
        # by the pixel size to have the right values in jy/pixel, 

        # get pixel size from model image coordsys
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
            self.msg("scaling model by pixel area ratio %g" % factor)

        # add unresolved components in Jy/pix
        # don't do this if you've already done it in flatimage()!
        if (os.path.exists(complist)):
            #pdb.set_trace()
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
        ia.convolve2d(convolved,major=beam['major'],minor=beam['minor'],
                      pa=beam['positionangle'],overwrite=True)
        ia.done()
        ia.open(convolved)
        ia.setbrightnessunit("Jy/beam")
        ia.setrestoringbeam(beam=beam)
        ia.done()


