# sd task for imaging
import os
import numpy
import pylab as pl
import asap as sd
from taskinit import * 

def sdimaging(sdfile, specunit, restfreq, scanlist, field, spw, antenna, stokes, gridfunction, imagename, overwrite, imsize, cell, dochannelmap, nchan, start, step, phasecenter, ephemsrcname, pointingcolumn):

        casalog.origin('sdimaging')
        try:
            # file check
            sdfile=sdfile.rstrip('/')+'/'
            ftab=''
            spwtab=''
            srctab=''
            if os.path.isdir(sdfile):
                tb.open(sdfile)
                tbkeys=tb.getkeywords()
                ftab=tbkeys['FIELD'].split()[-1]
                spwtab=tbkeys['SPECTRAL_WINDOW'].split()[-1]
                srctab=tbkeys['SOURCE'].split()[-1]
                tb.close()
                if any(key=='MS_VERSION' for key in tbkeys):
                    casalog.post( 'MS format' )
                else:
                    msg='sdfile must be in MS format'
                    raise Exception, msg
            else:
                msg='sdfile must be in MS format'
                raise Exception, msg

            # spectral unit
            mode=''
            if specunit=='channel' or specunit=='':
                mode='channel'
            elif specunit=='km/s':
                mode='velocity'
            else:
                mode='frequency'

            # scanlist

            # field
            fieldid=-1
            sourceid=-1
            tb.open(ftab)
            fieldnames=tb.getcol('NAME')
            fsrcids=tb.getcol('SOURCE_ID')
            tb.close()
            if type(field)==str:
                for i in range(len(fieldnames)):
                    if field==fieldnames[i]:
                        fieldid=i
                        sourceid=fsrcids[i]
                        break
                if fieldid==-1:
                    msg='field name '+field+' not found in FIELD table'
                    raise Exception, msg
            else:
                if field < len(fieldnames):
                    fieldid=field
                    sourceid=fsrcids[field]
                else:
                    msg='field id %s does not exist' % (field)
                    raise Exception, msg
            
            # restfreq
            if restfreq=='':
                tb.open(srctab)
                rfcol=tb.getcol('REST_FREQUENCY').transpose()
                srcidcol=tb.getcol('SOURCE_ID')
                tb.close()
                for i in range(tb.nrows()):
                    if sourceid==srcidcol[i]:
                        if len(rfcol[i])==0:
                            restfreq=0.0
                        else:
                            restfreq=rfcol[i][0]
                        break
                casalog.post("restfreq set to %s"%restfreq, "INFO")

            # spw
            spwid=-1
            tb.open(spwtab)
            nrows=tb.nrows()
            tb.close()
            if spw < nrows:
                spwid=spw
            else:
                msg='spw id %s does not exist' % (spw)
                raise Exception, msg

            # antenna
            if type(antenna)==int:
                antenna=str(antenna)+'&&&'
            else:
                if (len(antenna)!=0) and (antenna.find('&')==-1) and (antenna.find(';')==-1):
                    antenna = antenna + '&&&'

            # stokes
            if stokes=='':
                stokes='I'

            # gridfunction

            # imagename
            if os.path.exists(imagename) and not overwrite:
                msg='file %s exists' % (imagename)
                raise Exception, msg

            # imsize
            nx=''
            ny=''
            if len(imsize)==1:
                nx=imsize[0]
                ny=imsize[0]
            else:
                nx=imsize[0]
                ny=imsize[1]

            # cell
            cellx=''
            celly=''
            if type(cell)==str:
                cellx=cell
                celly=cell
            elif len(cell)==1:
                cellx=cell[0]
                celly=cell[0]
            else:
                cellx=cell[0]
                celly=cell[1]

            # start
            startval=start
            if mode=='velocity':
                startval=str(start)
                startval=startval+specunit
                startval=['LSRK',startval]
            elif mode=='frequency':
                startval=str(start)
                startval=startval+specunit

            # step
            stepval=step
            if mode=='velocity':
                stepval=str(step)
                stepval=stepval+specunit
            elif mode=='frequency':
                stepval=str(step)
                stepval=stepval+specunit

            # phasecenter
            # if empty, it should be determined here...

            ###########
            # Imaging #
            ###########
            casalog.post("Start imaging...", "INFO")
            im.open(sdfile)
            im.selectvis(field=fieldid, spw=spwid, nchan=-1, start=0, step=1, baseline=antenna, scan=scanlist)
            if dochannelmap:
                im.defineimage(mode=mode, nx=nx, ny=ny, cellx=cellx, celly=celly, nchan=nchan, start=startval, step=stepval, restfreq=restfreq, phasecenter=phasecenter, spw=spwid, stokes=stokes, movingsource=ephemsrcname)
            else:
                im.defineimage(mode=mode, nx=nx, ny=ny, cellx=cellx, celly=celly, phasecenter=phasecenter, spw=spwid, restfreq=restfreq, stokes=stokes, movingsource=ephemsrcname)
            im.setoptions(ftmachine='sd', gridfunction=gridfunction)
            im.setsdoptions(pointingcolumntouse=pointingcolumn)
            im.makeimage(type='singledish', image=imagename)
            im.close()
            
        except Exception, instance:
            #print '***Error***',instance
            casalog.post( str(instance), priority = 'ERROR' )
            return
