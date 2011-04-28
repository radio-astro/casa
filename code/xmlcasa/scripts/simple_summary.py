#import os
import math
#import random
import time
#import shutil
#import thread
#import commands
import numpy as np
import pylab as pl
#import plot_resource as prs

from taskinit import *
im,cb,ms,tb,fg,me,ia,po,sm,cl,cs,rg,sl,dc,vp=gentools()

class simple_summary:

    def __init__(self, vis):
        self._pol=[]
        self._nch=[]
        self._scan=[]
        self._trange=[]
        self._color=['b', 'g', 'r', 'c', 'm', 'k', 'y', '#eeefff',
                     '#202020', '#E07600', '#66D000', '#AC6600',
                     '#0091A0', '#10E050', '#6600E0', '#0066F0',
                     '#A868D8']

        self._polc=['0', 'I', 'Q', 'U', 'V', 'RR', 'RL', 'LR', 'LL', \
                    'XX', 'XY', 'YX', 'YY']
        self._fig=100
        self._vis=vis

    def timeconv(self, x, pos=0):
        #convert mjd to matplotlib: mjd+678576.0
        dt=pl.num2date(x+678576.0)
        return '%02d:%02d:%02d.%d' % \
          (dt.hour,dt.minute,dt.second,(int(dt.microsecond/1e5)))

    def timeconvsec(self, x, pos=0):
        #convert sec to matplotlib: sec/86400+678576.0
        dt=pl.num2date(x/86400+678576.0)
        return '%02d:%02d:%02d.%d' % \
          (dt.hour,dt.minute,dt.second,(int(dt.microsecond/1e5)))

    def conv(self, x):
        return '%d' % x

    def conv5(self, x):
        return '%(p).5f'%{'p':x}

    def find_subscan(self, tm, spw):
        #print tm, spw

        #print 'total items:', len(self._scan) 

        for i in range(len(self._scan)):
            itm=self._scan[i]
            #if len(itm['SpwIds']) > 1:
            #    print itm  
            #pass

            for j in range(len(itm['SpwIds'])):
                #print itm['SpwIds']
                if (itm['SpwIds'][j]>spw or (itm['SpwIds'][j]+1)<spw or
                    itm['BeginTime']>tm+3e-5 or itm['EndTime']<tm-3e-5):
                    continue
                scanInfo=[]
                scanInfo.append(itm['SpwIds'][j])
                scanInfo.append(self._pol[itm['SpwIds'][j]])
                scanInfo.append(itm['FieldName'])
                scanInfo.append(self.timeconv(itm['BeginTime']))
                #scanInfo.append(self.timeconv(itm['EndTime']))
                scanInfo.append(float('%.3f'%itm['IntegrationTime']))
                scanInfo.append(itm['scanId'])
                return scanInfo
        return []

    def on_move(self, event):
        x, y = event.x, event.y

        if event.inaxes:
            ss=self.find_subscan(event.xdata, event.ydata)
            if ss!=[]:
                print ss  

    def on_move_amptime(self, event):
        x, y = event.x, event.y

        if event.inaxes:
            print (event.xdata, event.ydata)

    def on_click(self, event):
        x, y = event.x, event.y

        if event.inaxes:
            ss=self.find_subscan(event.xdata, event.ydata)
            if ss!=[]:
                self.plot5(ss)

    def bl(self, n, a1, a2):
        return n*a1-(a1*(a1-1))/2+a2-a1 
               
    def plot5(self, ss):
        if self._vis==None or self._vis=='':
            return

        ###startProc=time.clock()
        ###startTime=time.time()

        ms.open(self._vis)
        print 'plot spw=%d scan=%d ...' % (ss[0], ss[5])
        ms.selectinit(datadescid=ss[0])
        ms.select({'scan_number':ss[5]})
        ms.selecttaql('ANTENNA1!=ANTENNA2')
        d=ms.getdata(['amplitude','antenna1','antenna2','uvdist','time'],
                     ifraxis=True)
        ms.done()
        
        if d=={}:
            return

        dt=d['amplitude']
        tm=d['time']
        a1=d['antenna1']
        a2=d['antenna2']
        uv=d['uvdist']

        ###endProc=time.clock()
        ###endTime=time.time()
        ###print 'ms.select process time %.2f sec.' % (endProc - startProc)
        ###print 'ms.select wall clock time %.2f sec.' % (endTime - startTime)


        fig=pl.figure(self._fig+1, figsize=(8, 6), 
                      dpi=80, facecolor='w', edgecolor='k')

        pl.ioff()
        pl.clf()
        #pl.title('Simple MS Summary')
        figman=pl.get_current_fig_manager()
        vs=os.path.abspath(self._vis)
        msname=vs[str.rfind(vs,'/')+1:]
        figman.set_window_title(msname+' scan:'+str(ss[5])+' field:'+ss[2]+
                                ' spw:'+str(ss[0])+' Simple Summary')
        
        pl.subplots_adjust(left=0.125,bottom=0.1,right=0.96,
                               top=0.96,wspace=0.25,hspace=0.25)

        shp=np.shape(dt)
        print '  ncorr=%s nchan=%s nbl=%s nt=%s' %(shp[0],shp[1],shp[2],shp[3])

        ###startProc=time.clock()
        ###startTime=time.time()

        print 'plot amplitude v.s. time ...'
        ax=pl.subplot(2, 2, 1)
        ax.cla()
        timeformatter=pl.FuncFormatter(self.timeconvsec)
        ax.xaxis.set_major_formatter(timeformatter)
        ax.xaxis.set_major_locator(pl.MaxNLocator(3))
        labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        ax.yaxis.set_major_formatter(labelFormatter)
        #ax.fmt_ydata=self.conv5
	pl.xlabel('time')
        pl.ylabel('amplitude')
        
        at=np.zeros((shp[3], shp[0]*2+1))
        for t in range(shp[3]):
            at[t][0]=tm[t]
        for p in range(shp[0]):
            c=np.reshape(dt[p], (shp[1]*shp[2], shp[3])).transpose()
            for t in range(shp[3]):
                at[t][2*p+1]=c[t].min()
                at[t][2*p+2]=c[t].max()
        
        #print at
        for p in range(shp[0]):
            i=2*p+1
            for j in range(shp[3]):
                pl.vlines(x=at[j][0], ymin=at[j][i], ymax=at[j][i+1], 
                          color=self._color[p])

        pl.xlim([tm.min(), tm.max()])
        pl.ylim(ymin=0)
        
        #print 'done amp v.s. time'
        ###endProc=time.clock()
        ###endTime=time.time()
        ###print 'plot amp vs. time  process time %.2f sec.' % \
        ###                    (endProc - startProc)
        ###print 'plot amp vs. time wall clock time %.2f sec.' % \
        ###                    (endTime - startTime)

        ###startProc=time.clock()
        ###startTime=time.time()

        print 'plot amplitude v.s. channel ...'
        ax=pl.subplot(2, 2, 2)
        ax.cla()
        labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        ax.xaxis.set_major_formatter(labelFormatter)
        ax.xaxis.set_major_locator(pl.MaxNLocator(3))
        ax.yaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
	pl.xlabel('channel')
        pl.ylabel('amplitude')

        shp=np.shape(dt)

        at=np.zeros((shp[1], shp[0]*2+1))
        for t in range(shp[1]):
            at[t][0]=t
        for p in range(shp[0]):
            for t in range(shp[1]):
                at[t][2*p+1]=dt[p][t].min()
                at[t][2*p+2]=dt[p][t].max()
        
        #print at
        for p in range(shp[0]):
            i=2*p+1
            for j in range(shp[1]):
                pl.vlines(x=at[j][0], ymin=at[j][i], ymax=at[j][i+1], 
                          color=self._color[p])

        pl.xlim([-0.5, shp[1]+0.5])
        pl.ylim(ymin=0)

        #print 'done amp v.s. channel'
        ###endProc=time.clock()
        ###endTime=time.time()
        ###print 'plot amp vs. channel  process time %.2f sec.' % \
        ###                    (endProc - startProc)
        ###print 'plot amp vs. channel wall clock time %.2f sec.' % \
        ###                    (endTime - startTime)

        ###startProc=time.clock()
        ###startTime=time.time()

        print 'plot amplitude v.s. antenna ...'
        ax=pl.subplot(2, 2, 3)
        ax.cla()
        labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        ax.xaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
        ax.xaxis.set_major_locator(pl.MaxNLocator(3))
        ax.yaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
	pl.xlabel('antenna')
        pl.ylabel('amplitude')

        mina=np.zeros((shp[2], shp[1]))
        maxa=np.zeros((shp[2], shp[1]))
        for p in range(shp[0]):
            ct=shp[2]*shp[3]
            s=np.reshape(dt[p], (shp[1], ct)).transpose()
            mina=np.zeros(ct)
            maxa=np.zeros(ct)
            for i in range(ct):
                mina[i]=s[i].min()
                maxa[i]=s[i].max()
            mina=np.reshape(mina, (shp[2], shp[3]))
            maxa=np.reshape(maxa, (shp[2], shp[3]))

            at=np.zeros((shp[2], 2))
            for i in range(shp[2]):
                at[i][0]=mina[i].min()
                at[i][1]=maxa[i].max()

            alen=int(round((1+math.sqrt(8*shp[2]))/2))-1
            bmp=np.zeros((alen, alen))
            cmp=np.zeros((alen, alen))
            for i in range(alen-1):
                for j in range(i+1, alen):
                    a=int(self.bl(alen, i, j))-1
                    #print a, i, j
                    bmp[i][j]=at[a][0]
                    bmp[j][i]=at[a][0]
                    cmp[i][j]=at[a][1]
                    cmp[j][i]=at[a][1]
            
            for i in range(alen):
                b=bmp[i].min()
                c=cmp[i].max()

            pl.vlines(x=range(alen), ymin=b, ymax=c, color=self._color[p])

        pl.xlim([-0.5, alen+0.5])
        pl.ylim(ymin=0)

        #endProc=time.clock()
        #endTime=time.time()
        #print 'plot amp vs. antenna  process time %.2f sec.' % \
        #                    (endProc - startProc)
        #print 'plot amp vs. antenna wall clock time %.2f sec.' % \
        #                    (endTime - startTime)
        ##print 'done amp v.s. antenna'

        ###startProc=time.clock()
        ###startTime=time.time()

        print 'plot amplitude v.s. uvdist ...'
        ##bl=N*a1-(a1*(a1-1))/2+a2-a1
        ax=pl.subplot(2, 2, 4)
        ax.cla()
        labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        ax.xaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
        ax.xaxis.set_major_locator(pl.MaxNLocator(3))
        ax.yaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
	pl.xlabel('uvdist')
        pl.ylabel('amplitude')

        at=np.zeros((shp[2], shp[0]*2+1))
        for t in range(shp[2]):
            at[t][0]=t
        mina=np.zeros((shp[2], shp[1]))
        maxa=np.zeros((shp[2], shp[1]))
        for p in range(shp[0]):
            ct=shp[2]*shp[3]
            s=np.reshape(dt[p], (shp[1], ct)).transpose()
            mina=np.zeros(ct)
            maxa=np.zeros(ct)
            for i in range(ct):
                mina[i]=s[i].min()
                maxa[i]=s[i].max()
            mina=np.reshape(mina, (shp[2], shp[3]))
            maxa=np.reshape(maxa, (shp[2], shp[3]))

            for j in range(shp[2]):
                for k in range(shp[3]):
                    pl.vlines(x=uv[j][k], ymin=mina[j][k], ymax=maxa[j][k], 
                          color=self._color[p])

        uvmax=uv.max()
        pl.xlim([0, uvmax+10])
        pl.ylim(ymin=0)

        #endProc=time.clock()
        #endTime=time.time()
        #print 'plot amp vs. uvdist process time %.2f sec.' % \
        #                    (endProc - startProc)
        #print 'plot amp vs. uvdist wall clock time %.2f sec.' % \
        #                    (endTime - startTime)
        ##print 'done amp v.s. uvdist'

        #fig.set_size_inches(8, 6)
        #pl.gcf().set_size_inches(18.5,10.5)
        #pl.rcParams['figure.figsize'] = 5, 10

        pl.ion()
        #figman.canvas.mpl_connect('motion_notify_event', 
        #        self.on_move_amptime )
        fig.show()

    def plot4(self, ss):
        #this function can be used only for small ms
        if self._vis==None or self._vis=='':
            return

        startProc=time.clock()
        startTime=time.time()

        ms.open(self._vis)
        #print 'plot spw=%d scan=%d' % (ss[0], ss[5])'
        ms.selectinit(datadescid=ss[0])
        ms.select({'scan_number':ss[5]})
        ms.selecttaql('ANTENNA1!=ANTENNA2')
        d=ms.getdata(['amplitude','antenna1','antenna2','uvdist','time'],
                     ifraxis=True)
        dt=d['amplitude']
        tm=d['time']
        a1=d['antenna1']
        a2=d['antenna2']
        uv=d['uvdist']

        endProc=time.clock()
        endTime=time.time()
        print 'ms.select process time %.2f sec.' % (endProc - startProc)
        print 'ms.select wall clock time %.2f sec.' % (endTime - startTime)


        fig=pl.figure(self._fig+1, figsize=(8, 6), 
                      dpi=80, facecolor='w', edgecolor='k')
        pl.ioff()
        pl.clf()
        #pl.title('Simple MS Summary')
        figman=pl.get_current_fig_manager()
        vs=os.path.abspath(self._vis)
        msname=vs[str.rfind(vs,'/')+1:]
        figman.set_window_title(msname+' scan:'+str(ss[5])+' field:'+ss[2]+
                                ' spw:'+str(ss[0])+' Simple Summary')
        
        pl.subplots_adjust(left=0.125,bottom=0.05,right=0.96,
                               top=0.96,wspace=0.25,hspace=0.25)
        #print amp vs. time
        startProc=time.clock()
        startTime=time.time()

        ax=pl.subplot(2, 2, 1)
        ax.cla()
        timeformatter=pl.FuncFormatter(self.timeconvsec)
        ax.xaxis.set_major_formatter(timeformatter)
        ax.xaxis.set_major_locator(pl.MaxNLocator(3))
        labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        ax.yaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
	pl.xlabel('time')
        pl.ylabel('amplitude')
        
        shp=np.shape(dt)
        #print shp
        for p in range(shp[0]):
            for c in range(shp[1]):
                for t in range(shp[3]):
                    #if fl[p][c][i]==0:
                    tmm=[]
                    tmm.append(tm[t])
                    pl.plot(tmm*shp[2], dt[p][c].transpose()[t], 
                        self._color[p]+'.')
        endProc=time.clock()
        endTime=time.time()
        print 'plot amp vs. time  process time %.2f sec.' % \
                            (endProc - startProc)
        print 'plot amp vs. time wall clock time %.2f sec.' % \
                            (endTime - startTime)
        #print 'done amp v.s. time'

        #amplitude vs. channel
        startProc=time.clock()
        startTime=time.time()
        ax=pl.subplot(2, 2, 2)
        ax.cla()
        labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        ax.xaxis.set_major_formatter(labelFormatter)
        ax.xaxis.set_major_locator(pl.MaxNLocator(3))
        ax.yaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
	pl.xlabel('channel')
        pl.ylabel('amplitude')

        shp=np.shape(dt)
        for p in range(shp[0]):
            for c in range(shp[1]):
                tmm=[]
                for b in range(shp[2]):
                    for t in range(shp[3]):
                        #if fl[p][c][i]==0:
                        tmm.append(dt[p][c][b][t])
                pl.plot([c]*len(tmm), tmm, self._color[p]+'.')
        pl.xlim([-0.5, shp[1]+0.5])
        endProc=time.clock()
        endTime=time.time()
        print 'plot amp vs. channel  process time %.2f sec.' % \
                            (endProc - startProc)
        print 'plot amp vs. channel wall clock time %.2f sec.' % \
                            (endTime - startTime)
        #print 'done amp v.s. channel'

        #amplitude vs. antenna
        startProc=time.clock()
        startTime=time.time()
        ax=pl.subplot(2, 2, 3)
        ax.cla()
        labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        ax.xaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
        ax.xaxis.set_major_locator(pl.MaxNLocator(3))
        ax.yaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
	pl.xlabel('antenna')
        pl.ylabel('amplitude')

        ants=np.array(list(set(a1.tolist()+a2.tolist())))
        for p in range(shp[0]):
            for c in range(shp[1]):
                amp=np.zeros(len(ants))
                for t in range(shp[3]):
                    #for a in range(shp[2]):
                    #    for k in range(len(ants)):
                    #        if a1[a]!=a2[a] and \
                    #             (a1[a]==ants[k] or a2[a]==ants[k]):
                    #            amp[k]=dt[p][c][a][t] 
                    #            break
                    #pl.plot(range(len(ants)), amp, self._color[p]+'.')

                    alen=int(round((1+math.sqrt(8*shp[2]))/2))-1
                    bmp=[]
                    for i in range(alen+1):
                        bmp.append([])
                    for i in range(alen):
                        for j in range(i+1, alen+1):
                            a=int(self.bl(alen, i, j))-1
                            #print a, i, j
                            bmp[i].append(dt[p][c][a][t])
                            bmp[j].append(dt[p][c][a][t])
                    for i in range(alen+1):
                        pl.plot(range(alen), bmp[i], self._color[p]+'.')
                            
        pl.xlim([-0.5, len(ants)+0.5])
        endProc=time.clock()
        endTime=time.time()
        print 'plot amp vs. antenna  process time %.2f sec.' % \
                            (endProc - startProc)
        print 'plot amp vs. antenna wall clock time %.2f sec.' % \
                            (endTime - startTime)
        #print 'done amp v.s. antenna'

        #amplitude vs. uvdist 
        startProc=time.clock()
        startTime=time.time()
        #bl=N*a1-(a1*(a1-1))/2+a2-a1
        ax=pl.subplot(2, 2, 4)
        ax.cla()
        labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        ax.xaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
        ax.xaxis.set_major_locator(pl.MaxNLocator(3))
        ax.yaxis.set_major_formatter(labelFormatter)
        ax.fmt_ydata=self.conv5
	pl.xlabel('uvdist')
        pl.ylabel('amplitude')


        for p in range(shp[0]):
            for c in range(shp[1]):
                for t in range(shp[3]):
                    pl.plot(uv.transpose()[t], dt[p][c].transpose()[t], 
                            self._color[p]+'.')
        uvmax=uv.max()
        pl.xlim([0, uvmax+100])
        endProc=time.clock()
        endTime=time.time()
        print 'plot amp vs. uvdist process time %.2f sec.' % \
                            (endProc - startProc)
        print 'plot amp vs. uvdist wall clock time %.2f sec.' % \
                            (endTime - startTime)
        #print 'done amp v.s. uvdist'

        #fig.set_size_inches(8, 6)
        #pl.gcf().set_size_inches(18.5,10.5)
        #pl.gcf().canvas.draw()
        #pl.rcParams['figure.figsize'] = 5, 10


        pl.ion()
        #figman.canvas.mpl_connect('motion_notify_event', 
        #        self.on_move_amptime )
        fig.show()


    def show(self):
        #print self._pol
        #print self._nch
        #print self._trange
        vs=os.path.abspath(self._vis)
        msname=vs[str.rfind(vs,'/')+1:]
        
        #startTime=time.time()
        #startProc=time.clock()

        fig=pl.figure(self._fig)
        pl.ioff()
        pl.clf()
        #pl.title('Simple MS Summary')
        figman=pl.get_current_fig_manager()
        figman.set_window_title(msname+' Simple Summary')
	pl.xlabel('time')
        pl.ylabel('spectral window')

        timeformatter=pl.FuncFormatter(self.timeconv)
        pl.gca().xaxis.set_major_formatter(timeformatter)
        pl.gca().xaxis.set_major_locator(pl.MaxNLocator(4))
        #labelFormatter=pl.ScalarFormatter(useOffset=False,useMathText=True)
        #pl.gca().yaxis.set_major_formatter(labelFormatter)
        pl.gca().yaxis.set_major_locator(pl.MaxNLocator(len(self._nch)))
        pl.gca().fmt_ydata=self.conv
        
        for i in range(len(self._scan)):
            item=self._scan[i]
            cl=self._color[item['FieldId']]
            #print item['BeginTime'], item['EndTime']
            #if len(item['SpwIds']) > 1:
            #    print item 
            for j in range(len(item['SpwIds'])):
                #any of the following three ways works good,
                #which one is quicker?

                xs, ys=pl.poly_between([item['BeginTime'], item['EndTime']],
                         item['SpwIds'][j], item['SpwIds'][j]+1)
                pl.fill(xs, ys, cl, ec=cl)

                #pl.fill([item['BeginTime'], item['EndTime'], 
                #         item['EndTime'], item['BeginTime']],
                #        [item['SpwIds'][j], item['SpwIds'][j],
                #         item['SpwIds'][j]+1, item['SpwIds'][j]+1],
                #        cl, ec=cl)

                #pl.broken_barh(
                #  [(item['BeginTime'], item['EndTime']-item['BeginTime'])], 
                #   (item['SpwIds'][j], 1), 
                #    facecolors=cl, edgecolors=cl)
        #pl.ylim([-0.2, len(item['SpwIds'])+0.2]) 
        pl.ion()
        figman.canvas.mpl_connect('motion_notify_event', self.on_move )
        figman.canvas.mpl_connect('button_press_event', self.on_click )
        fig.show()

        #endProc=time.clock()
        #endTime=time.time()
        #print 'ms.show process time %.2f sec.' % (endProc - startProc)
        #print 'ms.show wall clock time %.2f sec.' % (endTime - startTime)

        #print 'total items:', len(self._scan) 
        #for i in range(len(self._scan)):
        #    itm=self._scan[i]
        #    if len(itm['SpwIds']) > 1:
        #        print itm  
        #pass

    def summary(self):
        if self._vis==None or self._vis=='':
            return
        #vs=os.path.abspath(self._vis)

        print 'Read ms summary ...'
        tb.open(self._vis+'/SPECTRAL_WINDOW')
        self._nch=tb.getcol('NUM_CHAN')
        tb.done()

        tb.open(self._vis+'/DATA_DESCRIPTION')
        pol=tb.getcol('POLARIZATION_ID')
        tb.done()

        tb.open(self._vis+'/POLARIZATION')
        cor=tb.getvarcol('CORR_TYPE')
        tb.done()

        self._pol=[]
        pol.tolist()
        for i in xrange(len(pol)):
            b=[]
            a=cor['r'+str(pol[i]+1)].tolist()
            for j in range(len(a)):
                #b.append(a[j][0])
                b.append(self._polc[a[j][0]])
            self._pol.append(b)


        startTime=time.time()
        startProc=time.clock()

        ms.open(self._vis)
        head=ms.summary(verbose=True)['header']
        ms.done()
        self._trange.append(head['BeginTime'])
        self._trange.append(head['EndTime'])
        scans_=[]
        for i in head.keys():
            if i.startswith('scan_'):
                scans_.append(i)
        for i in scans_:
            scans=head[i]
            slen=len(scans.keys())
            for j in range(slen):
                self._scan.append(scans[str(j)])

        endProc=time.clock()
        endTime=time.time()
        #print 'ms.summary process time %.2f sec.' % (endProc - startProc)
        #print 'ms.summary wall clock time %.2f sec.' % (endTime - startTime)
        print 'Will take %d seconds plot summary ...'%(2*int(endTime-startTime))

        #print 'total items:', len(self._scan) 
        #for i in range(len(self._scan)):
        #    itm=self._scan[i]
        #    if len(itm['SpwIds']) > 1:
        #        print itm  
   

def plot_simple_summary(vis):
    #vis='jeff28-44.ms'
    #vis='X306/X306.ms'
    #vis='ngc5921.ms'
    sy=simple_summary(vis)
    smm=sy.summary()
    sy.show()

        

