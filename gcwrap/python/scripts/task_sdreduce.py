import os
from taskinit import *
import sys
import string
import inspect
from odict import odict

import sdutil
import asap as sd
from asap._asap import Scantable
import pylab as pl
from sdcal import sdcal
from sdsmooth import sdsmooth
from sdbaseline import sdbaseline
import task_sdcal
import task_sdsmooth
import task_sdbaseline

def sdreduce(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, calmode, fraction, noff, width, elongated, markonly, plotpointings, scanlist, field, iflist, pollist, channelrange, average, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, kernel, kwidth, chanwidth, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verifycal, verifysm, verifybl, verbosebl, bloutput, blformat, showprogress, minnrow, outfile, outform, overwrite, plotlevel):

        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
          if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals

        restorer = None

        saveinputs=myf['saveinputs']
        saveinputs('sdreduce','sdreduce.tmp')       

        casalog.origin('sdreduce')

        ###
        ### Now the actual task code
        ###

        try:
            tmpfilelist=''
            sdcalout=''
            sdsmoothout=''
            sdbaselineout=''
            project = sdutil.get_default_outfile_name(infile,
                                                            outfile,
                                                            '_cal')
            sdutil.assert_outfile_canoverwrite_or_nonexistent(project,
                                                              outform,
                                                              overwrite)

            # instantiate scantable
            s = sd.scantable(infile, average=False, antenna=antenna)

            # restorer
            restorer = sdutil.scantable_restore_factory(s,
                                                        infile,
                                                        fluxunit,
                                                        specunit,
                                                        frame,
                                                        doppler,
                                                        restfreq)
            
            # apply input parameters to scantable
            sdutil.set_spectral_unit(s, specunit)
            sdutil.set_doppler(s, doppler)
            sdutil.set_freqframe(s, frame)
            
            # apply data selection to scantable
            sel = sdutil.get_selector(scanlist, iflist, pollist,
                                      field)
            s.set_selection(sel)
            del sel

            # calibration stage
            casalog.post( "*** sdcal stage ***" )
            task_sdcal.prior_plot(s, plotlevel)
            scal = task_sdcal.docalibration(s, calmode, fraction,
                                            noff, width, elongated,
                                            markonly, plotpointings,
                                            verifycal)

            # convert flux
            sdutil.set_fluxunit(scal, fluxunit, telescopeparm, insitu=True)

            # delete original scantable instance
            s.set_selection()
            del s


            # opacity correction
            sdutil.doopacity(scal, tau)

            # channel splitting
            sdutil.dochannelrange(scal, channelrange)

            # averaging stage
            if average:
                    sout = sdutil.doaverage(scal, scanaverage, timeaverage,
                                            tweight, polaverage, pweight,
                                            averageall)
            else:
                    casalog.post( "No averaging was applied..." )
                    sout = scal
            task_sdcal.posterior_plot(sout, project, plotlevel)


            # smoothing stage
            casalog.post( "" )
            casalog.post( "*** sdsmooth stage ***" )
            if kernel != 'none':
                    task_sdsmooth.prior_plot(sout, project, plotlevel)
                    task_sdsmooth.dosmooth(sout, kernel, kwidth, chanwidth,
                                           verifysm)
                    task_sdsmooth.posterior_plot(sout, project, plotlevel)
            else:
                    casalog.post( "No smoothing was applied..." )

            # baseline stage
            casalog.post( "" )
            casalog.post( "*** sdbaseline stage ***")
            if blfunc != 'none':
                    task_sdbaseline.prior_plot(sout, plotlevel)
                    blfile = task_sdbaseline.init_blfile(sout, infile, project, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, bloutput, blformat)
                    task_sdbaseline.dobaseline(sout, blfile, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verifybl, verbosebl, blformat, showprogress, minnrow)
                    task_sdbaseline.posterior_plot(sout, project, plotlevel)
            else:
                    casalog.post( "No baseline subtraction was applied..." )
                    
            # write result on disk
            sdutil.save(sout, project, outform, overwrite)
            del scal
            del sout

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                raise Exception, instance
                return
        finally:
                # restore
                if restorer is not None:
                        restorer.restore()
                        del restorer




def _reset_inputs(param=None):
        '''
        internal function to recover inputs of sdreduce (containing other tasks) with global task parameter settin
g
        '''
        arg_names=['infile','antenna','fluxunit','telescopeparm','specunit','restfreq','frame','doppler','calmode','scanlist','field','iflist','pollist','channelrange','average','scanaverage','timeaverage','tweight','averageall','polaverage','pweight','tau','kernel','kwidth','blfunc','order','npiece','applyfft','fftmethod','fftthresh','addwn','rejwn','clipthresh','clipniter','masklist','maskmode','thresh','avg_limit','edge','verifycal','verifysm','verifybl','verbosebl','bloutput','blformat','showprogress','minnrow','outfile','outform','overwrite','plotlevel']
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
          if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
        a=odict()
        paramfile = 'sdreduce' + '.tmp'
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

