import casac
from tw_utils import *
import os
from time import *
import numpy as numarray
import pylab

#from math import *
#from Scientific.Functions import LeastSquares
import tw_func


image = casac.homefinder.find_home_by_name('imageHome')
regman = casac.homefinder.find_home_by_name('regionmanagerHome')
quanta=casac.homefinder.find_home_by_name('quantaHome')

class ImageTest:
    def __init__(self, imageName, write=True,
                 resultDir='WEBPAGES/imageTest/',
                 imDir='IMAGES/'):

        self.imTool=image.create()
        self.imTool.open(imageName) #open('tables/squint_corr.restored')
        self.rgTool=regman.create()
        self.qaTool=quanta.create()
	self.getArr() #instead make explicit call to getArr()
	self.write=write
	self.imageName=imageName
        self.iterate=0 #for multiple cubeFit() tests

	if self.write:
         self.resultDir=resultDir+strftime('/%Y_%m_%d/')
         if os.access(self.resultDir,os.F_OK) is False:
          print self.resultDir+' directory DNE, so am making one!'
          os.mkdir(self.resultDir)
         else: 
          print self.resultDir+' directory exists; will add to it!'
	 self.imDir=imDir
	 if os.access(imDir,os.F_OK) is False:
	  print imDir+' directory DNE, so am making one!'
	  os.mkdir(imDir)
	 else: 
	  print imDir+' directory exists; will add to it!'

         t=localtime( time() )
         self.fname='Regression-%s-%s-%s-%s-%s-%s.html'%(t[0],t[1],t[2],t[3],t[4],t[5])
	 self.html=self.resultDir+self.fname
         self.body1=[]
         self.body2=[]
         self.htmlPub=htmlPub(self.html,'Image tests')
	else:
	 print 'stats-only mode; will not write to html file!'

    def changeImage(self, imageName):
        self.imTool.open(imageName) #open('tables/squint_corr.restored')
	self.getArr() #instead make explicit call to getArr()
	self.imageName=imageName

    def getArr(self):
	d=self.imTool.getchunk(axes=[],dropdeg=False)
	self.b=pylab.array(d)


    def simple_stats(self,sigma=10,plane=0):
        nchan=self.imTool.shape()[3]
        rmsmax=0
        rms1=0
        chan=0
        
        for k in range(0,nchan/2+1):
            rms1=pylab.rms_flat(self.b[:,:,plane,k])
            if(rms1 > rmsmax):
                rmsmax=rms1
                chan=k
        
	rms1=rmsmax
	min1,max1=self.min_max(self.b[:,:,plane,chan])
        self.show(self.b[:,:,plane,chan])
	if self.write:
	 header='Channel %d pol %d from image %s'%(chan,plane,self.imageName)
         body1=['The image generated with pylab:']
	 body2=['maximum: %f'%(max1),'minimum: %f'%(min1),'rms: %f'%(rms1)]
         saveDir=self.imDir+self.fname[11:-5]+'-channel%d-pol%d.png'%(chan,plane)
         pylab.savefig(saveDir)
         self.htmlPub.doBlk(body1, body2, saveDir,header)
        returnFlag= 1
        if(rms1 > 2*sigma): returnFlag=-1
        return rms1, max1, min1, returnFlag

    def min_max(self,image):
     s=numarray.shape(image)
     min=99
     max=0
     for x in range(s[0]):
      for y in range(s[1]):
       if image[x,y]>max: max=image[x,y]
       if image[x,y]<min: min=image[x,y]
     return min,max
			
    def p_min_max(self,n=5): #returns positions of n brightest pixels
        b=self.b
        s=numarray.shape(b)
        val=[]
        xp=[]
        yp=[]
        avoid=[]
        for z in range(n):
            v=0
            max=0
            x_p=0
            y_p=0
            for x in range(s[0]):
              for y in range(s[1]):
 	       if [x,y] not in avoid:
                v=b[x,y,0,0]
                if v>max:
                 max=v
                 x_p=x
                 y_p=y
            val.append(max)
            xp.append(x_p)
            yp.append(y_p)
            avoid.append([x_p,y_p]) #ignores previous bright peaks
        return val,xp,yp

    def done(self) :
	if self.write:
       	 self.htmlPub.doFooter()
	 print 'webpage construction successful!'
	 print 'images in '+os.path.abspath(self.imDir)
	 print 'webpage at '+os.path.abspath(self.html)
         return '%s'%(os.path.abspath(self.html))
        else: #return 0 if no writing of file is done
         return 'none'

    def model(self,xx=1024,yy=1024): #make model array/image
     c=[]
     for x in range(xx):
      c.append([])
      for y in range(yy):
       c[x].append([])
       c[x][y].append(0.0) #note the floating point: very important!
     self.m=pylab.array(c)

    def amodel(self,v,x,y): #alter selected 'pixels' of model
     self.m[x,y,0]=v


    def bmodel(self, XY=None, plane=0): 
        shp=self.imTool.shape()
        result=[]
        blc=[0,0,plane,0]
        trc=[shp[0]-1, shp[1]-1, plane, 0]
        reg=self.rgTool.box(blc=blc, trc=trc)
        dat=self.imTool.getchunk(blc=blc, trc=trc, dropdeg=True)
        self.show(dat)
        a={'return':{}}
        try:
            a=self.imTool.fitsky(region=reg)
        except:
            ###lets limit the region and try again
            blc=[int(0.45*shp[0]),int(0.45*shp[1]) ,plane,0]
            trc=[int(0.55*shp[0]),int(0.55*shp[1]) ,plane,0]
            reg=self.rgTool.box(blc=blc, trc=trc)
            dat=self.imTool.getchunk(blc=blc, trc=trc, dropdeg=True)
            self.show(dat)
            try:
                a=self.imTool.fitsky(region=reg)
            except:
                a={'return':{}, 'pixels':[]}
        body2=[]
        resid=pylab.array(a['pixels'])
        if(a['return'].has_key('component0')):
            ra = self.qaTool.time(a['return']['component0']['shape']['direction']['m0'])
            dec = self.qaTool.angle(a['return']['component0']['shape']['direction']['m1'])
            bmaj= self.qaTool.angle(a['return']['component0']['shape']['majoraxis'], form='dig2')
            bmin = self.qaTool.angle(a['return']['component0']['shape']['minoraxis'], form='dig2')
            bpa = self.qaTool.angle(a['return']['component0']['shape']['positionangle'])
            flux = str('%4.2f'%a['return']['component0']['flux']['value'][0])+a['return']['component0']['flux']['unit']
            result.append([ra, dec, bmaj, bmin, bpa, flux])
            ss='fit:\testimated center: %s  %s\n'%(ra,dec)+'\tMajAxis : %s  \tMinAxis: %s \tPosAng: %s'%(bmaj, bmin, bpa)+' flux= '+flux
            body2.append('<pre>%s</pre>'%ss)
        else:
            result.append(False)
            body2.append('<pre>Failed to converge in fitting</pre>')
        #write to web page
        if self.write:
            header='Image  of plane%d of  %s'%(plane,self.imageName)
            body1=['<pre>The image generated with pylab:</pre>']
        #body2=['maximum: %f'%(max1),'minimum: %f'%(min1),'rms: %f'%(rms)]
            saveDir=self.imDir+self.fname[11:-5]+'-model%d.png'%(plane)
            pylab.savefig(saveDir)
            self.htmlPub.doBlk(body1, body2, saveDir,header)
        rms=0.0
        if(result[0] != False):
            self.show(resid)
            rms=pylab.rms_flat(resid)
            min1,max1=self.min_max(resid)
            print 'rms of residual image: %f'%(rms)
            #write to web page
            if self.write:
                header='Residual from plane%d of image %s'%(plane,self.imageName)
                body1=['<pre>The image generated with pylab:</pre>']
                body2=['<pre>maximum: %f</pre>'%(max1),'<pre>minimum: %f</pre>'%(min1),'<pre>rms: %f</pre>'%(rms)]
                saveDir=self.imDir+self.fname[11:-5]+'-resid%d.png'%(plane)
                pylab.savefig(saveDir)
                self.htmlPub.doBlk(body1, body2, saveDir,header)
        return result, rms
	
    def bmodel_old(self,XY=None,plane=0):
     self.model()
     result=[]
     chi2=[]
     data=[]
     output=[] #to be returned: [x0,y0,FWHM_x,FWHM_y]
     body2=[]
     r,d=self.fitPeaks(XY,plane)
     for i in r:
      result.append(i[0])
      chi2.append(i[1])
     for i in range(len(d)):
      data.append([])
      for j in d[i]:
       data[i].append(j[0])
     #start computing models
     for i in range(len(result)):
      for point in data[i]:
       x=point[0]
       y=point[1]
       v=tw_func.gauss3d(result[i],[x,y])
       self.amodel(v,x,y)
     print 'done building model'
     print 'stats for fits:'
     #rms=pylab.rms_flat(self.m[:,:,0])
     #min1,max1=self.min_max(self.m[:,:,0])
     for i in range(len(r)): 
      self.show(self.m[:,:,0]) 
      sigmaX=pylab.sqrt(1/(2*r[i][0][4]))
      sigmaY=pylab.sqrt(1/(2*r[i][0][2]))
      if(XY==None):
          XY=[[0,0]]
      ss='fit #%d:\testimated center: %s  optimized center: [%.2f,%.2f]\n'%(i,XY[i],r[i][0][5],r[i][0][3])+'\tFWHM in x: %.3f pixels   FWHM in y: %.3f\n\tchi2: %f'%(2.355*sigmaX,2.355*sigmaY,r[i][1])
      print ss
      body2.append('<pre>%s</pre>'%ss)
      output.append([r[i][0][5],r[i][0][3],2.355*sigmaX,2.355*sigmaY])
     #write to web page
     if self.write:
      header='Model of plane%d of image %s'%(plane,self.imageName)
      body1=['<pre>The image generated with pylab:</pre>']
      #body2=['maximum: %f'%(max1),'minimum: %f'%(min1),'rms: %f'%(rms)]
      saveDir=self.imDir+self.fname[11:-5]+'-model%d.png'%(plane)
      pylab.savefig(saveDir)
      self.htmlPub.doBlk(body1, body2, saveDir,header)
     #return stuff
     return output

    def subtract(self,plane=0):
     resid=[]
     b=self.b[:,:,plane,0]
     m=self.m[:,:,0]
     s=numarray.shape(b)
     for x in range(s[0]):
      resid.append([])
      for y in range(s[1]):
       resid[x].append(0.0) #note the floating point: very important!
     self.resid=pylab.array(resid)
     for x in range(s[0]):
      for y in range(s[1]):
       self.resid[x,y]=b[x,y]-m[x,y]
     self.show(self.resid)
     rms=pylab.rms_flat(self.resid)
     min1,max1=self.min_max(self.resid)
     self.show(self.resid)
     print 'rms of residual image: %f'%(rms)
     #write to web page
     if self.write:
      header='Residual from plane%d of image %s'%(plane,self.imageName)
      body1=['<pre>The image generated with pylab:</pre>']
      body2=['<pre>maximum: %f</pre>'%(max1),'<pre>minimum: %f</pre>'%(min1),'<pre>rms: %f</pre>'%(rms)]
      saveDir=self.imDir+self.fname[11:-5]+'-resid%d.png'%(plane)
      pylab.savefig(saveDir)
      self.htmlPub.doBlk(body1, body2, saveDir,header)
     #return rms
     return rms

    def findPeaks(self,y,x,plane=0): #note inversion in x,y coord
 			     #use like findPeaks(x,y)
			     #inversion should be self-consistent if take output as [x,y]
     r=50 #search 'radius'
     rms=pylab.rms_flat(self.b[:,:,plane,0])
     thold=rms #limit of pixels to be considered

     val=[]
     xp=[]
     yp=[]
     avoid=[]
     data=[]
     if(x==None):
         x=[len(self.b[:,0,plane,0])/2]
         y=[len(self.b[0,:,plane,0])/2]
         r=min(x[0], y[0])-2
     for i in range(len(x)): #len(x)==len(y)!
      v=0
      max=0
      x_p=0
      y_p=0
      for j in range(x[i]-r,x[i]+r):
       for k in range(y[i]-r,y[i]+r):
        if [j,k] not in avoid:
         v=self.b[j,k,plane,0]
         if v>max:
          max=v
          x_p=j
          y_p=k
      val.append(max)
      xp.append(x_p)
      yp.append(y_p)
      avoid.append([x_p,y_p]) #ignores previous bright pixels
      #generate data sets
     for i in range(len(x)):
      vv=val[i]
      xx=xp[i]
      yy=yp[i]
      while vv>thold:
       xx+=1
       vv=self.b[xx,yp[i],plane,0]
      vv=val[i]
      while vv>thold:
       yy+=1
       vv=self.b[xp[i],yy,plane,0]
      dx=xx-xp[i]
      dy=yy-yp[i]
      r=(dx+dy)/2
      if(r >12):
          r=12
      print 'PEAK pos, val and r ', xp[i], yp[i],val[i],  r
      data.append([])
      for j in range(xp[i]-r,xp[i]+r):
       for k in range(yp[i]-r,yp[i]+r):
        if (pylab.sqrt((xp[i]-j)*(xp[i]-j)+(yp[i]-k)*(yp[i]-k)) <=r) & (self.b[j,k,plane,0]>0):
         data[i].append(([j,k],self.b[j,k,plane,0]))
     return data,xp,yp

    def fitPeaks(self,XY=None,plane=0):
     x=[]
     y=[]
     if(XY != None):
         for n in XY:    
             x.append(n[0])
             y.append(n[1])
     else:
         x=None
         y=None
     data,x0,y0=self.findPeaks(x,y,plane)
         
     tparam=[]
     result=[]
     for i in range(len(data)): #len(data) is number of fits to be made
      tparam.append([0.,1.,1.,x0[i],1.,y0[i]])
      #tparam.append([1.,1.,1.,y0[i],1.,x0[i]])
#      r=LeastSquares.leastSquaresFit(tw_func.gauss3d,tparam[i],data[i])
      result.append(r)
     return result,data

    def show(self,image):
     pylab.clf()
     zmin=-2.0*pylab.rms_flat(image)
     zmax=5.0*pylab.rms_flat(image)
#    pylab.imshow(image,interpolation='bilinear', origin='lower', cmap=pylab.cm.Greys, vmin=zmin, vmax=zmax)
     pylab.imshow(image,interpolation='bilinear', origin='lower', cmap=pylab.cm.RdYlBu_r, vmin=zmin, vmax=zmax)
     pylab.colorbar()
    def ishow(self,plane=0): #show current image
     pylab.clf()
     image=self.b[:,:,plane,0]
     zmin=-2.0*pylab.rms_flat(image) 
     zmax=5.0*pylab.rms_flat(image)
     pylab.imshow(image,interpolation='bilinear', origin='lower', cmap=pylab.cm.Greys, vmin=zmin, vmax=zmax)
     pylab.colorbar()
 
    def mshow(self):  #show currentmodel
     pylab.clf()
     image=self.m[:,:,0]
     zmin=-2.0*pylab.rms_flat(image)
     zmax=5.0*pylab.rms_flat(image)
     pylab.imshow(image,interpolation='bilinear', origin='lower', cmap=pylab.cm.Greys, vmin=zmin, vmax=zmax)
     pylab.colorbar()
     
    def rshow(self): #show current residual
     pylab.clf()
     image=self.resid
     zmin=-2.0*pylab.rms_flat(image)
     zmax=5.0*pylab.rms_flat(image)
     pylab.imshow(image,interpolation='bilinear', origin='lower', cmap=pylab.cm.Greys, vmin=zmin, vmax=zmax)
     pylab.colorbar()

    def save(self,path='junk.png'):
     pylab.savefig(path)
     print 'image saved to %s'%(path)

    def ishift(self,image,x,y,plane=None,cube=None): #shift the image by x,y
                                 #x,y > 0 shift to right, up
                                 #x,y < 0 shift to left, down
     if (plane!=None) & (cube!=None): #choose both plane and cube, if ever applicable
      image=image[:,:,plane,cube]
     elif plane!=None: #select plane of image
      image=image[:,:,plane,0]
     elif cube!=None: #select section of image cube
      image=image[:,:,0,cube]
     s=numarray.shape(image)
     c=[]
     for i in range(s[0]):
      c.append([])
      for j in range(s[1]):
       c[i].append(0.0)
     c=pylab.array(c)
     for i in range(s[0]): 
      for j in range(s[1]):
       try:
        dx=i-x
        dy=j-y
        if (dx>=0) and (dy>=0):
         c[i,j]=image[dx,dy]
        else:
         c[i,j]=0
       except IndexError:
        c[i,j]=0
     self.disp(c)
     return c

    def ima(self,im1,im2): #multiply,add images
     s=numarray.shape(im1) #assume image shapes are same
     c=0
     for i in range(s[0]):
      for j in range(s[1]):
       c+=im1[i,j]*im2[i,j]
     return c

    def mima(self,im1,im2,XY1,XY2,p1=None,p2=None,c1=None,c2=None): #c1,c2 for image cube slices only
     x,y,x2,y2=[],[],[],[]
     for n in XY1:
      x.append(n[0])
      y.append(n[1])
     for n in XY2:
      x2.append(n[0])
      y2.append(n[1])
     result=[]
     for n in range(len(x)):
      c=self.ishift(im1,x[n],y[n],p1,c1)
      d=self.ishift(im2,x2[n],y2[n],p2,c2)
      result.append(self.ima(c,d))
     return result

    def disp(self,image,plane=None,cube=None,xlabel='',ylabel=''): #cube arg applies to image cube of shape (:,:,1:,n)
                                      #cube arg specifies nth slice of cube
     if (plane!=None) & (cube!=None): #choose both plane and cube, if ever applicable
      image=image[:,:,plane,cube]
     elif plane!=None: #select plane of image
      image=image[:,:,plane,0]
     elif cube!=None: #select section of image cube
      image=image[:,:,0,cube]
     zmin=-2.0*pylab.rms_flat(image)
     zmax=5.0*pylab.rms_flat(image)

     pylab.clf()
     pylab.xlabel(xlabel)
     pylab.ylabel(ylabel)
     pylab.imshow(image,interpolation='bilinear', origin='lower', cmap=pylab.cm.Greys, vmin=zmin, vmax=zmax)

    #utilties for image cubes
    def drill(self,image,x0,y0): #'drills' into cube and pulls all pixels having coord x,y
     s=numarray.shape(image)
     x,y=[],[]
     for n in range(s[3]):
      x.append(n)
      y.append(image[x0,y0,0,n])
     pylab.clf()
     return x,y
  
    def cubePeaks(self,image,x,y):
     dataMax=[]
     dataMin=[]
     optimumXY=[]
     xp=[0,0]
     minX=0
     minY=999999
     maxX=0
     maxY=0
     mean=pylab.mean(y)
     #find max point
     for n in x:
      if y[n]>maxY:
       maxX=n 
       maxY=y[n]
     xp[0]=xp[1]=maxX
     dataMax.append((xp[0],y[xp[0]]))
     #input data on each side of peak
     try: 
      while y[xp[0]]>mean: #mean->tHold #ensures ~1 point before max
       xp[0]-=1
       dataMax.append((xp[0],y[xp[0]]))
     except IndexError: pass
     try:
      while y[xp[1]]>mean: #mean->tHold #ensures ~1 point beyond max
       xp[1]+=1
       dataMax.append((xp[1],y[xp[1]]))
     except IndexError: pass
     #find min point
     for n in x:
      if y[n]<minY:
       minX=n
       minY=y[n]
     xp[0]=xp[1]=minX
     dataMin.append((xp[0],y[xp[0]]))
     #input data on each side of peak
     try: #try statements guard against guassian peak going out bounds
      while y[xp[0]]<mean:
       xp[0]-=1
       dataMin.append((xp[0],y[xp[0]]))
     except IndexError: pass
     try:
      while y[xp[1]]<mean:
       xp[1]+=1
       dataMin.append((xp[1],y[xp[1]]))
     except IndexError: pass
     optimumXY.append([minX,minY])
     optimumXY.append([maxX,maxY])
     s='minX: %d   minY: %f   maxX: %d   maxY: %f   mean: %f'%(minX,minY,maxX,maxY,mean)
     if self.write: self.body2.append('<pre>%s</pre>'%s)
     print s
     return dataMin,dataMax,optimumXY
    def auto_fitCube2(self,verbose=0):
        ib=self.imTool.moments(moments=7, outfile='Some_momtest_7.im', overwrite=True)
        a=ib.statistics()
        x=int(a['maxpos'][0])
        y=int(a['maxpos'][1])
        ib.done()
        XY,fwhm=self.fitCube2(x,y)
        return XY,fwhm
        
    def auto_fitCube(self,image,verbose=0):
     x,y=[],[]
     xlist,ylist=[],[]
     max_rms=0
     xp,yp=0,0
     diff=0
     max_diff=0
     s=numarray.shape(image)
     if s[0]>100:
#      xlist=range(.25*s[0],.75*s[0])
      xlist=range(s[0]/4,3*s[0]/4)
     else:
      xlist=range(s[0])
     if s[1]>100:
#      ylist=range(.25*s[1],.75*s[1],4) #later add the 4th pixel thing
      ylist=range(s[1]/4,3*s[1]/4,4)
     else:
      ylist=range(s[1])
     for i in xlist:
      for j in ylist:
       x,y=self.drill(image,i,j)
       rms=pylab.rms_flat(y)
       diff=abs(pylab.max(y)-rms)
       if(diff > max_diff):
           max_diff,xp,yp=diff,i,j 
 #      if rms>max_rms:
 #          max_rms,xp,yp=rms,i,j
 #          if verbose: print i,j,rms
     print 'optimum fit to [%d,%d]   rms=%.6f'%(xp,yp,max_rms)
     XY,fwhm=self.fitCube2(xp,yp)
     return XY,fwhm
     
    def fitCube(self,image,x0,y0,fit=True): #fits all layers of cube at pixel x0,y0
 	                           #fits min/max peaks w/ gaussian                            
     x,y=self.drill(image,x0,y0)
     result=[]
     if fit:
      maxFitted=0 #boolean flag
      minFitted=0
      s1,s2,t1,t2=[],[],[],[]
      s='fit stats (numbered in order of fits)'
      if self.write: self.body2.append('<pre>%s</pre>'%s) #assemble body2 for html
      print s
      dataMin,dataMax,optimumXY=self.cubePeaks(image,x,y)
      tparam0=[1.,-1.,1.,optimumXY[0][0]]
      tparam1=[1.,1.,1.,optimumXY[1][0]]
      try: #new min fit: reduce dataset size whenever overflow error occurs
          dataMin.sort()
          print 'tparam0', tparam0
          print dataMin
          dataMin.sort()
#          result.append(LeastSquares.leastSquaresFit(tw_func.gauss,tparam0,list(dataMin)))
          minFitted=1
      except:
          print '!\n overFlowError in fitting max peak\n will reduce dataset size'
#          print ' initial dataset size: %d'%len(dataMin)
#          shrink=0
#          while (shrink<25) & (minFitted==0):
#              dataMin=dataMin[:-1]
#              try:
#                  result.append(leastSquaresFit(model=tw_func.gauss,parameters=tparam0,data=dataMin, stopping_limit=0.1))
#                  minFitted=1
#                  shrink+=1
#              except: pass
#          print ' final dataset size: %d\n!'%len(dataMin)
#      except:
#          print 'error in finding a negative peak'
      try: #new max fit: reduce dataset size when things don't work right
          dataMax.sort()
          print tparam1
#          result.append(LeastSquares.leastSquaresFit(tw_func.gauss,tparam1,list(dataMax)))
          maxFitted=1
      except:
          print 'error in fitting a positive peak'
 #         print '!\n overFlowError in fitting max peak\n will reduce dataset size'
 #         print ' initial dataset size: %d'%len(dataMax)
 #         shrink=0
 #         while (shrink<25) & (maxFitted==0):
 #             dataMax=dataMax[:-1]
 #             try:
 #                 result.append(leastSquaresFit(tw_func.gauss,tparam1,dataMax))
 #                 maxFitted=1
 #                 shrink+=1
 #             except: pass
 #         print ' final dataset size: %d\n!'%len(dataMax)
      #resume old
      for i in range(len(result)):
       print 'result ', result[i]
       sigma=pylab.sqrt(1/(2*result[i][0][2]))
       s='fit #%d:\toptimized coord: [%.6f,%.6f]\n\tFWHM: %f pixels\n\tchi2: %f'%(i,result[i][0][3],result[i][0][0]+result[i][0][1],2.355*sigma,result[i][1])
       print s
       if self.write: self.body2.append('<pre>%s</pre>'%s)
      #draw nice guassian curves on chart
      if minFitted:
       tmax=0
       tmin=999999
       for j in dataMin: #->better gaussian chart
        s1.append(j[0])
       for j in s1:
        if j>tmax: tmax=j
        if j<tmin: tmin=j
       s1=pylab.arange(tmin,tmax,0.1)
       for j in s1: 
        t1.append(tw_func.gauss(result[0][0],j))
       pylab.plot(s1,t1,'k.-.') #plot for min peak
      if maxFitted:
       tmax=0
       tmin=999999
       #print dataMax
       for j in dataMax: #->better gaussian chart
        s2.append(j[0])
       for j in s2:
        if j>tmax: tmax=j
        if j<tmin: tmin=j
       #print tmin,tmax
       s2=pylab.arange(tmin,tmax,0.1)
       for j in s2:
        if minFitted:
         t2.append(tw_func.gauss(result[1][0],j))
        else:
         t2.append(tw_func.gauss(result[0][0],j))
       pylab.plot(s2,t2,'k.-.') #plot for max peak
     pylab.xlabel('Layer of image cube @ pixel [%d,%d]'%(x0,y0))
     pylab.ylabel('Intensity')
     #pylab.plot(x,y,lw=0.4)
     pylab.plot(x,y,'b.-.',lw=0.4)
     #webpage write out
     if self.write:
      header='Image cube: %s'%(self.imageName)
      self.body1=['<pre>Plot of layers @ pixel [%d,%d]:</pre>'%(x0,y0)]
      saveDir=self.imDir+self.fname[11:-5]+'-cube-min_max%d.png'%self.iterate
      pylab.savefig(saveDir)
      self.htmlPub.doBlk(self.body1, self.body2, saveDir,header)
      self.iterate+=1
     #now return stuff: center coord, FWHM values
     XY=[]
     fwhm=[]
     if(len(result)==0):
         XY.append([-1., -1.])
         XY.append([-1., -1.])
         fwhm.append(-1)
         fwhm.append(-1)
     
     for i in range(len(result)):
         sigma=pylab.sqrt(1/(2*result[i][0][2]))
         fwhm.append(2.355*sigma)
         XY.append([result[i][0][3],result[i][0][0]])
     
         return XY,fwhm

    def fitCube2(self,x0,y0):
        result=[]
        reg=self.rgTool.box(blc=[x0,y0], trc=[x0,y0])
        retval=self.imTool.fitprofile(region=reg, ngauss=1, fit=True)
        result.append(retval['return']['elements']['*1']['parameters'])
        result.append(retval['return']['elements']['*1']['errors'])
        s='fit at [%d,%d]\n\tFWHM: %f \n\peak: %f \t with errors: %f, %f '%(x0,y0, result[0][2], result[0][0], result[1][2], result[1][0]) 
        print s
        if self.write: self.body2.append('<pre>%s</pre>'%s)
        data=self.imTool.getchunk(blc=[x0,y0], trc=[x0,y0], dropdeg=True)
        theFit=retval['values']
        theResid=retval['resid']
        pylab.clf()
        pylab.plot(data,'r-')
        pylab.plot(theFit, 'bo')
        pylab.plot(theResid,'g.-.')
        pylab.xlabel('Layer of image cube @ pixel [%d,%d]'%(x0,y0))
        pylab.ylabel('Intensity')
        pylab.title('Fit on Image '+self.imageName)
        if self.write:
            header='Image cube Fit: %s'%self.imageName
            self.body1=['<pre>Plot of layers @ pixel [%d,%d]:</pre>'%(x0,y0)]
            saveDir=self.imDir+self.fname[11:-5]+'-cube-min_max%d.png'%self.iterate
            pylab.savefig(saveDir)
            self.htmlPub.doBlk(self.body1, self.body2, saveDir,header)
            self.iterate+=1
        XY=[]
        fwhm=[]
        for i in range(len(result)):
            fwhm.append(result[i][2])
            XY.append([result[i][0],result[i][1]])
             
        return XY, fwhm
