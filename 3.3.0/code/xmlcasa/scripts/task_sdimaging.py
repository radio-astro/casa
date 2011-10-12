# sd task for imaging
import os
import numpy
import pylab as pl
import asap as sd
from taskinit import * 

def sdimaging(infile, specunit, restfreq, scanlist, field, spw, antenna, stokes, gridfunction, outfile, overwrite, imsize, cell, dochannelmap, nchan, start, step, phasecenter, ephemsrcname, pointingcolumn):

        casalog.origin('sdimaging')
        try:
            # file check
            infile=infile.rstrip('/')+'/'
            ftab=''
            spwtab=''
            srctab=''
            if os.path.isdir(infile):
                tb.open(infile)
                tbkeys=tb.getkeywords()
                ftab=tbkeys['FIELD'].split()[-1]
                spwtab=tbkeys['SPECTRAL_WINDOW'].split()[-1]
                if tbkeys.has_key('SOURCE'):
                    srctab=tbkeys['SOURCE'].split()[-1]
                else:
                    srctab = ''
                tb.close()
                if any(key=='MS_VERSION' for key in tbkeys):
                    casalog.post( 'MS format' )
                else:
                    msg='infile must be in MS format'
                    raise Exception, msg
            else:
                msg='infile must be in MS format'
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
                if field == -1:
                    sourceid=fsrcids[0]
                if field < len(fieldnames):
                    fieldid=field
                    sourceid=fsrcids[field]
                else:
                    msg='field id %s does not exist' % (field)
                    raise Exception, msg
            
            # restfreq
            if restfreq=='' and srctab != '':
                tb.open(srctab)
                srcidcol=tb.getcol('SOURCE_ID')
                for i in range(tb.nrows()):
                    if sourceid==srcidcol[i] and tb.iscelldefined('REST_FREQUENCY',i):
                        rf = tb.getcell('REST_FREQUENCY',i)
                        if len(rf) > 0:
                            restfreq=tb.getcell('REST_FREQUENCY',i)[0]
                            break
                tb.close()
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
                if antenna == -1:
                    pass
                else:
                    antenna=str(antenna)+'&&&'
            else:
                if (len(antenna)!=0) and (antenna.find('&')==-1) and (antenna.find(';')==-1):
                    antenna = antenna + '&&&'

            # stokes
            if stokes=='':
                stokes='I'

            # gridfunction

            # outfile
            if os.path.exists(outfile):
                if overwrite:
                    os.system( 'rm -rf %s'%(outfile) )
                else:
                    msg='file %s exists' % (outfile)
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
            im.open(infile)
            im.selectvis(field=fieldid, spw=spwid, nchan=-1, start=0, step=1, baseline=antenna, scan=scanlist)
            if dochannelmap:
                im.defineimage(mode=mode, nx=nx, ny=ny, cellx=cellx, celly=celly, nchan=nchan, start=startval, step=stepval, restfreq=restfreq, phasecenter=phasecenter, spw=spwid, stokes=stokes, movingsource=ephemsrcname)
            else:
                if mode!='channel':
                    casalog.post('Setting imaging mode as \'channel\'','INFO')
                im.defineimage(mode='channel', nx=nx, ny=ny, cellx=cellx, celly=celly, phasecenter=phasecenter, spw=spwid, restfreq=restfreq, stokes=stokes, movingsource=ephemsrcname)
            im.setoptions(ftmachine='sd', gridfunction=gridfunction)
            im.setsdoptions(pointingcolumntouse=pointingcolumn)
            im.makeimage(type='singledish', image=outfile)
            im.close()
            
        except Exception, instance:
            #print '***Error***',instance
            casalog.post( str(instance), priority = 'ERROR' )
            return
