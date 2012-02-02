import os
from taskinit import *
import sys
import string
import inspect
from odict import odict

import asap as sd
from asap._asap import Scantable
import pylab as pl
from sdaverage import sdaverage
from sdsmooth import sdsmooth
from sdbaseline import sdbaseline

#def sdcal(infile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, average, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, kernel, kwidth, blfunc, order, npiece, nwave, maxwavelength, clipthresh, clipniter, masklist, maskmode, thresh, avg_limit, edge, verifycal, verifysm, verifybl, verbosebl, outfile, outform, overwrite, plotlevel):
def sdcal(infile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, average, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, kernel, kwidth, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verifycal, verifysm, verifybl, verbosebl, showprogress, minnrow, outfile, outform, overwrite, plotlevel):

        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
          if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals

        saveinputs=myf['saveinputs']
        saveinputs('sdcal','sdcal.tmp')       

        casalog.origin('sdcal')

        ###
        ### Now the actual task code
        ###

        try:
            tmpfilelist=''
            sdaverageout=''
            sdsmoothout=''
            sdbaselineout=''
            if outfile=='':
                    outfile=infile.rstrip('/')+'_cal'
            outfilename = os.path.expandvars(outfile)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and  (outform!='ascii' and outform!='ASCII'):
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s


            if calmode != 'none' or average: 
              if kernel =='none' and blfunc=='none':
                sdaverageout = outfile
              else:
                sdaverageout = 'sdaverageout.tmp'
                tmpfilelist+=sdaverageout+' '
                if kernel !='none':
                  if blfunc !='none':
                    sdsmoothout = 'sdsmoothout.tmp'
                    sdbaselineout = outfile
                    tmpfilelist+=sdsmoothout+' '
                  else:
                    sdsmoothout = outfile
                else:
                  if blfunc != 'none':
                    sdbaselineout = outfile
            else:
              if kernel != 'none':
                sdaverageout = 'sdaverageout_noncal.tmp'
                tmpfilelist+=sdaverageout+' '
                if blfunc == 'none':
                  sdsmoothout = outfile
                else:
                  sdsmoothout = 'sdsmoothout.tmp'
                  tmpfilelist+=sdsmoothout+' '
                  sdbaselineout = outfile
              else:
                if blfunc != 'none':
                  sdbaselineout = outfile
                  sdaverageout = 'sdaverageout_noncal.tmp'
                  tmpfilelist+=sdaverageout+' '
                else:
                  sdaverageout = outfile 
            
            if average:
              if scanaverage == False and timeaverage == False and polaverage == False:
                 raise Exception, 'Specify type(s) of averaging'
              if timeaverage == True and tweight == 'none':
                 raise Exception, 'Specify weighting type of time average'
              if polaverage == True and pweight == 'none':
                 raise Exception, 'Specify weighting type of polarization average'
                   
            #print "*** sdaverage stage ***";
            casalog.post( "*** sdaverage stage ***" )
            if calmode != 'none':
              tmpoutfile = sdaverageout
              #sdaverage(infile, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, outfile=tmpoutfile, outform=outform, overwrite=True, plotlevel=plotlevel)
              sdaverage(infile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, verifycal, outfile=tmpoutfile, outform=outform, overwrite=True, plotlevel=plotlevel)
            else:
              plevel = plotlevel
              if not average:
                # still need to call sdaverage for unit conversion, etc.
                plevel = 0
                casalog.post( "Neither calibrated nor averaged..." )
              tmpoutfile = sdaverageout
              sdaverage(infile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, outfile=tmpoutfile, outform=outform, overwrite=True, plotlevel=plevel)

            #reset data selection
            tmpinfile=tmpoutfile
            if not os.path.exists(tmpinfile):
              m = "No output file found. Error occurred at sdaverage stage"
              raise Exception, m
            
            # scanlist, iflist needed to be reset since created scantable (calibration reduces scans,
            # also generated scantable renumbered scan numbers, etc.)
            #print ""
            #print "*** sdsmooth stage ***";
            casalog.post( "" )
            casalog.post( "*** sdsmooth stage ***" )
            if kernel != 'none':
              #sdsmooth.defaults()
              tmpoutfile = sdsmoothout 
              #sdsmooth(infile=tmpinfile, kernel=kernel, kwidth=kwidth, outfile=tmpoutfile, overwrite=True, plotlevel=plotlevel)
              sdsmooth(infile=tmpinfile, antenna=antenna, kernel=kernel, kwidth=kwidth, verify=verifysm, outfile=tmpoutfile, overwrite=True, plotlevel=plotlevel)
              tmpinfile = tmpoutfile
              #tmpfilelist+=tmpoutfile+' '
              if not os.path.exists(tmpinfile):
                m = "No output file found. Error occurred at sdsmooth stage"
                raise Exception, m
            else:
              #print "No smoothing was applied..."
              casalog.post( "No smoothing was applied..." )

            #print ""
            #print "*** sdbaseline stage ***";
            casalog.post( "" )
            casalog.post( "*** sdbaseline stage ***")
            if blfunc != 'none':
              tmpoutfile = sdbaselineout
              #sdbaseline.defaults()
              sdbaseline(infile=tmpinfile,antenna=antenna,masklist=masklist,maskmode=maskmode,thresh=thresh,avg_limit=avg_limit,edge=edge,blfunc=blfunc,order=order,npiece=npiece,applyfft=applyfft,fftmethod=fftmethod,fftthresh=fftthresh,addwn=addwn,rejwn=rejwn,clipthresh=clipthresh,clipniter=clipniter,verify=verifybl,verbose=verbosebl,showprogress=showprogress,minnrow=minnrow,outfile=tmpoutfile,outform=outform,overwrite=True,plotlevel=plotlevel)
            else:
              #print "No baseline subtraction was applied..."
              #print ""
              casalog.post( "No baseline subtraction was applied..." )
              casalog.post( "" )
            # to restore original input paramters
            _reset_inputs()
            # clean up tmp files
            if len(tmpfilelist)!=0:
              #print ""
              #print "Deleting the temporary files, %s ..." % tmpfilelist
              casalog.post( "" )
              casalog.post( "Deleting the temporary files, %s ..." % (tmpfilelist) )
              cmd='rm -rf '+tmpfilelist 
              os.system(cmd) 

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return




def _reset_inputs(param=None):
        '''
        internal function to recover inputs of sdcal (containing other tasks) with global task parameter settin
g
        '''
        arg_names=['infile','antenna','fluxunit','telescopeparm','specunit','frame','doppler','calmode','scanlist','field','iflist','pollist','channelrange','average','scanaverage','timeaverage','tweight','averageall','polaverage','pweight','tau','kernel','kwidth','blfunc','order','npiece','applyfft','fftmethod','fftthresh','addwn','rejwn','clipthresh','clipniter','masklist','maskmode','thresh','avg_limit','edge','verifycal','verifysm','verifybl','verbosebl','showprogress','minnrow','outfile','outform','overwrite','plotlevel']
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
          if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
        a=odict()
        paramfile = 'sdcal' + '.tmp'
        f=open(paramfile)
        while 1:
                try:
                        line=f.readline()
                        if (line.find('#') != 0):
                          splitline=line.split('\n')[0]
                          splitline2=splitline.split('=')
                          pname = splitline2[0].rstrip()
                          pvalstr = splitline2[1].lstrip()
                          pval = eval(pvalstr)
                          for key in arg_names:
                            if pname== key:
                              a[key]=pval
                              break

                except:
                        break
        f.close()
        if(param == None):
          myf['__set_default_parameters'](a)
        elif(param == 'paramkeys'):
          return a.keys()
        else:
          if(a.has_key(param)):
            return a[param]

