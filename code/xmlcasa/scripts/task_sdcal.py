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

#def sdcal(sdfile, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, average, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, kernel, kwidth, blmode, blpoly, interactive, masklist, thresh, avg_limit, edge, outfile, outform, overwrite, plotlevel):
#def sdcal(sdfile, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, average, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, kernel, kwidth, blmode, blpoly, verify, masklist, thresh, avg_limit, edge, outfile, outform, overwrite, plotlevel):
def sdcal(sdfile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, average, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, kernel, kwidth, blmode, blpoly, verifycal,verifysm,verifybl, masklist, thresh, avg_limit, edge, outfile, outform, overwrite, plotlevel):

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
                    outfile=sdfile.rstrip('/')+'_cal'
            outfilename = os.path.expandvars(outfile)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and  (outform!='ascii' and outform!='ASCII'):
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s


            if calmode != 'none': 
              if kernel =='none' and blmode=='none':
                sdaverageout = outfile
              else:
                sdaverageout = 'sdaverageout.tmp'
                tmpfilelist+=sdaverageout+' '
                if kernel !='none':
                  if blmode !='none':
                    sdsmoothout = 'sdsmoothout.tmp'
                    sdbaselineout = outfile
                    tmpfilelist+=sdsmoothout+' '
                  else:
                    sdsmoothout = outfile
                else:
                  if blmode != 'none':
                    sdbaselineout = outfile
            else:
              if kernel != 'none':
                sdaverageout = 'sdaverageout_noncal.tmp'
                tmpfilelist+=sdaverageout+' '
                if blmode == 'none':
                  sdsmoothout = outfile
                else:
                  sdsmoothout = 'sdsmoothout.tmp'
                  tmpfilelist+=sdsmoothout+' '
              else:
                if blmode != 'none':
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
              #sdaverage(sdfile, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, outfile=tmpoutfile, outform=outform, overwrite=True, plotlevel=plotlevel)
              sdaverage(sdfile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, verifycal, outfile=tmpoutfile, outform=outform, overwrite=True, plotlevel=plotlevel)
            else:
            #print "Skipping calibration..."
              tmpoutfile = sdaverageout
              sdaverage(sdfile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, outfile=tmpoutfile, outform=outform, overwrite=True, plotlevel=plotlevel)
                
            #reset data selection
            tmpsdfile=tmpoutfile
            if not os.path.exists(tmpsdfile):
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
              #sdsmooth(sdfile=tmpsdfile, kernel=kernel, kwidth=kwidth, outfile=tmpoutfile, overwrite=True, plotlevel=plotlevel)
              sdsmooth(sdfile=tmpsdfile, antenna=antenna, kernel=kernel, kwidth=kwidth, verify=verifysm, outfile=tmpoutfile, overwrite=True, plotlevel=plotlevel)
              tmpsdfile = tmpoutfile
              #tmpfilelist+=tmpoutfile+' '
              if not os.path.exists(tmpsdfile):
                m = "No output file found. Error occurred at sdsmooth stage"
                raise Exception, m
            else:
              #print "No smoothing was applied..."
              casalog.post( "No smoothing was applied..." )

            #print ""
            #print "*** sdbaseline stage ***";
            casalog.post( "" )
            casalog.post( "*** sdbaseline stage ***")
            if blmode != 'none':
              tmpoutfile = sdbaselineout
              #sdbaseline.defaults()
#              sdbaseline(sdfile=tmpsdfile, blmode=blmode,blpoly=blpoly,interactive=interactive,masklist=masklist, thresh=thresh, avg_limit=avg_limit, edge=edge, outfile=tmpoutfile, outform=outform, overwrite=True, plotlevel=plotlevel)
              sdbaseline(sdfile=tmpsdfile, antenna=antenna, blmode=blmode,blpoly=blpoly,verify=verifybl,masklist=masklist, thresh=thresh, avg_limit=avg_limit, edge=edge, outfile=tmpoutfile, outform=outform, overwrite=True, plotlevel=plotlevel)
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
#        arg_names=['sdfile','fluxunit','telescopeparm','specunit','frame','doppler','calmode','scanlist','field','iflist','pollist', 'scanaverage','timeaverage','tweight', 'polaverage','pweight', 'kernel','kwidth','tau','blmode','blpoly','interactive','masklist','outfile','outform','overwrite','plotlevel','thresh','avg_limit','edge']
        arg_names=['sdfile','fluxunit','telescopeparm','specunit','frame','doppler','calmode','scanlist','field','iflist','pollist', 'scanaverage','timeaverage','tweight', 'polaverage','pweight', 'kernel','kwidth','tau','blmode','blpoly','verify','masklist','outfile','outform','overwrite','plotlevel','thresh','avg_limit','edge']
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

