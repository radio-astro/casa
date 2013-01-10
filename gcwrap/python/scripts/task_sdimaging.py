# sd task for imaging
import os
import numpy
import pylab as pl
import asap as sd
from taskinit import *
import sdutil

@sdutil.sdtask_decorator
def sdimaging(infile, specunit, restfreq, scanlist, field, spw, antenna, stokes, gridfunction, convsupport, truncate, gwidth, jwidth, outfile, overwrite, imsize, cell, dochannelmap, nchan, start, step, phasecenter, ephemsrcname, pointingcolumn):
    with sdutil.sdtask_manager(sdimaging_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()    

class sdimaging_worker(sdutil.sdtask_template_imaging):
    def __init__(self, **kwargs):
        super(sdimaging_worker,self).__init__(**kwargs)
        self.imager_param = sdutil.parameter_registration(self)

    def __del__(self, base=sdutil.sdtask_template_imaging):
        super(sdimaging_worker,self).__del__()

    def __register(self, key, attr=None, arg_is_value=False):
        self.imager_param.register(key,attr,arg_is_value)

    def parameter_check(self):
        # outfile check
        sdutil.assert_outfile_canoverwrite_or_nonexistent(self.outfile,
                                                          'im',
                                                          self.overwrite)

    def compile(self):
        # imaging mode
        spunit = self.specunit.lower()
        if spunit == 'channel' or len(spunit) == 0:
            mode = 'channel'
        elif spunit == 'km/s':
            mode = 'velocity'
        else:
            mode = 'frequency'
        if not self.dochannelmap and mode != 'channel':
            casalog.post('Setting imaging mode as \'channel\'','INFO')
            mode = 'channel'
        self.__register('mode',mode)

        # scanlist

        # field
        self.fieldid=-1
        sourceid=-1
        self.open_table(self.field_table)
        field_names = self.table.getcol('NAME')
        source_ids = self.table.getcol('SOURCE_ID')
        self.close_table()
        if type(self.field)==str:
            try:
                self.fieldid = field_names.tolist().index(self.field)
            except:
                msg = 'field name '+field+' not found in FIELD table'
                raise ValueError, msg
        else:
            if self.field == -1:
                self.sourceid = source_ids[0]
            elif self.field < len(field_names):
                self.fieldid = self.field
                self.sourceid = source_ids[self.field]
            else:
                msg = 'field id %s does not exist' % (self.field)
                raise ValueError, msg

        # restfreq
        if self.restfreq=='' and self.source_table != '':
            self.open_table(self.source_table)
            source_ids = self.table.getcol('SOURCE_ID')
            for i in range(self.table.nrows()):
                if self.sourceid == source_ids[i] \
                       and self.table.iscelldefined('REST_FREQUENCY',i):
                    rf = self.table.getcell('REST_FREQUENCY',i)
                    if len(rf) > 0:
                        self.restfreq=self.table.getcell('REST_FREQUENCY',i)[0]
                        break
            self.close_table()
            casalog.post("restfreq set to %s"%self.restfreq, "INFO")
        self.__register('restfreq')
            
        # 
        # spw
        self.spwid=-1
        self.open_table(self.spw_table)
        nrows=self.table.nrows()
        if self.spw < nrows:
            self.spwid=self.spw
        else:
            self.close_table()
            msg='spw id %s does not exist' % (self.spw)
            raise ValueError, msg
        self.allchannels=self.table.getcell('NUM_CHAN',self.spwid)
        self.close_table()
        self.__register('spw','spwid')
        
        # antenna
        if type(self.antenna)==int:
            if self.antenna >= 0:
                self.antenna=str(self.antenna)+'&&&'
        else:
            if (len(self.antenna) != 0) and (self.antenna.find('&') == -1) \
                   and (self.antenna.find(';')==-1):
                self.antenna = self.antenna + '&&&'

        # stokes
        if self.stokes == '':
            self.stokes = 'I'
        self.__register('stokes')

        # gridfunction

        # outfile
        if os.path.exists(self.outfile) and self.overwrite:
            os.system('rm -rf %s'%(self.outfile))

        # imsize
        (nx,ny) = sdutil.get_nx_ny(self.imsize)
        self.__register('nx',nx)
        self.__register('ny',ny)

        # cell
        (cellx,celly) = sdutil.get_cellx_celly(self.cell,
                                               unit='arcmin')
        self.__register('cellx',cellx)
        self.__register('celly',celly)

        # channel map
        if self.dochannelmap:
            # start
            if mode == 'velocity':
                startval = ['LSRK', '%s%s'%(self.start,self.specunit)]
            elif mode == 'frequency':
                startval = '%s%s'%(self.start,self.specunit)
            else:
                startval = self.start

            # step
            if mode in ['velocity', 'frequency']:
                stepval = '%s%s'%(self.step,self.specunit)
            else:
                stepval = self.step
        else:
            startval = 0
            stepval = self.allchannels
            self.nchan = 1
        self.__register('start',startval)
        self.__register('step', stepval)
        self.__register('nchan')
                
        # phasecenter
        # if empty, it should be determined here...
        if len(self.phasecenter) == 0:
            self.open_table(self.pointing_table)
            dir = self.table.getcol('DIRECTION')
            dirinfo = self.table.getcolkeywords('DIRECTION')
            units = dirinfo['QuantumUnits'] if dirinfo.has_key('QuantumUnits') \
                    else ['rad', 'rad']
            mref = dirinfo['MEASINFO']['Ref'] if dirinfo.has_key('MEASINFO') \
                   else 'J2000'
            forms = ['dms','dms'] if mref.find('AZEL') != -1 else ['hms','dms']
            qx = qa.quantity(numpy.median(dir[0,:,:]),units[0])
            qy = qa.quantity(numpy.median(dir[1,:,:]),units[1])
            self.close_table()
            phasecenter = ' '.join([mref,
                                    qa.formxxx(qx,forms[0]),
                                    qa.formxxx(qy,forms[1])])
            self.__register('phasecenter',phasecenter,arg_is_value=True)
        else:
            self.__register('phasecenter')
        self.__register('movingsource', 'ephemsrcname')

    def execute(self):
        # imaging
        casalog.post("Start imaging...", "INFO")
        casalog.post("Using phasecenter \"%s\""%(self.imager_param['phasecenter']), "INFO")
        self.open_imager(self.infile)
        self.imager.selectvis(field=self.fieldid, spw=self.spwid, nchan=-1, start=0, step=1, baseline=self.antenna, scan=self.scanlist)
        #self.imager.selectvis(vis=infile, field=fieldid, spw=spwid, nchan=-1, start=0, step=1, baseline=antenna, scan=scanlist)
        self.imager.defineimage(**self.imager_param)#self.__get_param())
##         if self.dochannelmap:
##             self.imager.defineimage(mode=self.mode, nx=self.nx, ny=self.ny, cellx=self.cellx, celly=self.celly, nchan=self.nchan, start=self.startval, step=self.stepval, restfreq=self.restfreq, phasecenter=self.phasecenter, spw=self.spwid, stokes=self.stokes, movingsource=self.ephemsrcname)
##         else:
## ##             if self.mode != 'channel':
## ##                 casalog.post('Setting imaging mode as \'channel\'','INFO')
##             self.imager.defineimage(mode='channel', nx=self.nx, ny=self.ny, cellx=self.cellx, celly=self.celly, nchan=1, start=0, step=self.allchannels, phasecenter=self.phasecenter, spw=self.spwid, restfreq=self.restfreq, stokes=self.stokes, movingsource=self.ephemsrcname)
        self.imager.setoptions(ftmachine='sd', gridfunction=self.gridfunction)
        self.imager.setsdoptions(pointingcolumntouse=self.pointingcolumn, convsupport=self.convsupport, truncate=self.truncate, gwidth=self.gwidth, jwidth=self.jwidth)
        self.imager.makeimage(type='singledish', image=self.outfile)
        self.close_imager()
