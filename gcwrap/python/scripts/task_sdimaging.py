# sd task for imaging
import os
import numpy
import pylab as pl
import asap as sd
from taskinit import casalog, gentools, qatool
import sdutil

@sdutil.sdtask_decorator
def sdimaging(infile, specunit, restfreq, scanlist, field, spw, antenna, stokes, gridfunction, convsupport, truncate, gwidth, jwidth, outfile, overwrite, imsize, cell, dochannelmap, nchan, start, step, outframe, phasecenter, ephemsrcname, pointingcolumn):
    with sdutil.sdtask_manager(sdimaging_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()    

class sdimaging_worker(sdutil.sdtask_template_imaging):
    def __init__(self, **kwargs):
        super(sdimaging_worker,self).__init__(**kwargs)
        self.imager_param = {}

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
        self.imager_param['mode'] = mode

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
        self.imager_param['restfreq'] = self.restfreq
        
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
        self.imager_param['spw'] = self.spwid

        # outframe (force using the current frame)
        #self.imager_param['outframe'] = ''
        self.imager_param['outframe'] = self.outframe
        if self.outframe == '':
            # get from MS
            my_ms = gentools(['ms'])[0]
            my_ms.open(self.infile)
            spwinfo = my_ms.getspectralwindowinfo()
            my_ms.close()
            del my_ms
            for key, spwval in spwinfo.items():
                if spwval['SpectralWindowId'] == self.imager_param['spw']:
                    self.imager_param['outframe'] = spwval['Frame']
                    casalog.post("Using frequency frame of MS, '%s'" % self.imager_param['outframe'])
                    break
            if self.imager_param['outframe'] == '':
                raise Exception, "Internal error of getting frequency frame of spw=%d." % self.imager_param['spw']
        else:
            casalog.post("Using frequency frame defined by user, '%s'" % self.imager_param['outframe'])
        
        # antenna
        in_antenna = self.antenna # backup for future use
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
        self.imager_param['stokes'] = self.stokes

        # gridfunction

        # outfile
        if os.path.exists(self.outfile) and self.overwrite:
            os.system('rm -rf %s'%(self.outfile))


        # cell
        cell = self.cell
        if cell == '' or cell[0] == '':
            # Calc PB
            grid_factor = 3.
            qPB = self._calc_PB(in_antenna)
            casalog.post("Cell size is calculated using PB size")
            cell = '%f%s' % (qPB['value']/grid_factor, qPB['unit'])
            casalog.post("Using cell size = PB/%4.2F = %s" % (grid_factor, cell))

        (cellx,celly) = sdutil.get_cellx_celly(cell, unit='arcmin')
        self.imager_param['cellx'] = cellx
        self.imager_param['celly'] = celly

        # imsize
        imsize = self.imsize
        if imsize == [] or imsize[0] < 1:
            imsize = self._get_imsize_from_map_extent(cellx, celly)
            if imsize[0] > 1024 or imsize[1] > 1024:
                casalog.post("The calculated image pixel number is larger than 1024. It would take time to generate the image. Please wait...", priority='warn')

        (nx,ny) = sdutil.get_nx_ny(imsize)
        self.imager_param['nx'] = nx
        self.imager_param['ny'] = ny

        # channel map
        if self.dochannelmap:
            # start
            if mode == 'velocity':
                #startval = ['LSRK', '%s%s'%(self.start,self.specunit)]
                startval = [self.imager_param['outframe'], '%s%s'%(self.start,self.specunit)]
            elif mode == 'frequency':
                #startval = '%s%s'%(self.start,self.specunit)
                startval = [self.imager_param['outframe'], '%s%s'%(self.start,self.specunit)]
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
        self.imager_param['start'] = startval
        self.imager_param['step'] = stepval
        self.imager_param['nchan'] = self.nchan
                
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
            # CAS-5410 Use private tools inside task scripts
            my_qa = qatool()
            qx = my_qa.quantity(numpy.median(dir[0,:,:]),units[0])
            qy = my_qa.quantity(numpy.median(dir[1,:,:]),units[1])
            self.close_table()
            phasecenter = ' '.join([mref,
                                    my_qa.formxxx(qx,forms[0]),
                                    my_qa.formxxx(qy,forms[1])])
            self.imager_param['phasecenter'] = phasecenter
        else:
            self.imager_param['phasecenter'] = self.phasecenter
        self.imager_param['movingsource'] = self.ephemsrcname

        ### WORKAROUND for image unit ###
        self.open_table(self.infile)
        datacol_lookup = ['FLOAT_DATA', 'DATA']
        valid_cols = self.table.colnames()
        datacol = ''
        for colname in datacol_lookup:
            if colname in valid_cols:
                datacol = colname
                break

        if len(datacol) == 0:
            self.close_table()
            raise Exception, "Could not find a column that stores data."

        datakw = self.table.getcolkeywords(datacol)
        self.tb_fluxunit = ''
        if datakw.has_key('UNIT'):
            self.tb_fluxunit = datakw['UNIT']

        self.close_table()
        ###

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

        if not os.path.exists(self.outfile):
            raise RuntimeError, "Failed to generate output image '%s'" % self.outfile
        # Convert output images to proper output frame
        my_ia = gentools(['ia'])[0]
        my_ia.open(self.outfile)
        csys = my_ia.coordsys()
        csys.setconversiontype(spectral=csys.referencecode('spectra')[0])
        my_ia.setcoordsys(csys.torecord())

        ### WORKAROUND to fix image unit
        if self.tb_fluxunit == 'K' and my_ia.brightnessunit() != 'K':
            my_ia.setbrightnessunit(self.tb_fluxunit)

        my_ia.close()

    def _calc_PB(self, antenna):
        # CAS-5410 Use private tools inside task scripts
        my_qa = qatool()
        
        pb_factor = 1.175
        self.open_table(self.antenna_table)
        antid = -1
        casalog.post("Calculating Pirimary beam size:")
        try:
            if type(antenna) == int and antenna < self.table.nrows():
                antid = antenna
            elif type(antenna) == str and len(antenna) > 0:
                for idx in range(self.table.nrows()):
                    if (antenna.upper() == self.table.getcell('NAME', idx)):
                        antid = idx
                        break
            antdiam_unit = self.table.getcolkeyword('DISH_DIAMETER', 'QuantumUnits')[0]
            if antid > 0:
                antdiam_ave = my_qa.quantity(self.table.getcell('DISH_DIAMETER'),antdiam_unit)
            else:
                diams = self.table.getcol('DISH_DIAMETER')
                antdiam_ave = my_qa.quantity(diams.mean(), antdiam_unit)
        finally:
            self.close_table()
        
        rest_frequency = self.restfreq
        if type(rest_frequency) in [float, numpy.float64]:
            rest_frequency = my_qa.tos(my_qa.quantity(rest_frequency, 'Hz'))
        if not my_qa.compare(rest_frequency, 'Hz'):
            raise Exception, "Invalid rest frequency, %s" % str(rest_frequency)
        wave_length = 0.2997924 / my_qa.convert(my_qa.quantity(rest_frequency),'GHz')['value']
        D_m = my_qa.convert(antdiam_ave, 'm')['value']
        lambda_D = wave_length / D_m * 3600. * 180 / numpy.pi
        PB = my_qa.quantity(pb_factor*lambda_D, 'arcsec')
        # Summary
        casalog.post("- Antenna diameter: %s m" % D_m)
        casalog.post("- Reference Frequency: %s" % rest_frequency)
        casalog.post("PB size = %5.3f * lambda/D = %s" % (pb_factor, my_qa.tos(PB)))
        return PB


    def _get_imsize_from_map_extent(self, dx, dy):
        ### MS selection is ignored. This is not quite right.
        casalog.post("Calculating image pixel from map extent.")
        colname = self.pointingcolumn.upper()
        self.open_table(self.pointing_table)
        try:
            dirinfo = self.table.getcolkeywords(colname)
            dir_unit = dirinfo['QuantumUnits'] if dirinfo.has_key('QuantumUnits') \
                    else ['rad', 'rad']
            pointing = self.table.getcol(colname)
        finally:
            self.close_table()

        # CAS-5410 Use private tools inside task scripts
        my_qa = qatool()
        ymax = pointing[1][0].max()
        ymin = pointing[1][0].min()
        qheight = my_qa.quantity(ymax - ymin, dir_unit[1])
        qcenty = my_qa.quantity(0.5*(ymin + ymax), dir_unit[1])

        x_to_rad = my_qa.convert(my_qa.quantity(1., dir_unit[0]), 'rad')['value']
        xrad = pointing[0][0] * x_to_rad
        del pointing
        width = self._get_x_extent(xrad) * \
                numpy.cos(my_qa.convert(qcenty, 'rad')['value'])
        qwidth = my_qa.quantity(width, dir_unit[0])

        ny = numpy.ceil( ( my_qa.convert(qheight, my_qa.getunit(dy))['value'] /  \
                           my_qa.getvalue(dy) ) )
        nx = numpy.ceil( ( my_qa.convert(qwidth, my_qa.getunit(dx))['value'] /  \
                           my_qa.getvalue(dx) ) )
        casalog.post("- Pointing extent: [%s, %s]" % (my_qa.tos(qwidth), my_qa.tos(qheight)))
        casalog.post("- Cell size: [%s, %s]" % (dx, dy))
        casalog.post("Image pixel numbers to cover pointings: [%d, %d]" % (nx+1, ny+1))
        return (int(nx+1), int(ny+1))


    def _get_x_extent(self, x):
        # assumed the x is in unit of rad.
        pi2 = 2. * numpy.pi
        x = (x % pi2)
        npart = 4
        dlon = pi2/float(npart)
        pos = [int(v/dlon) for v in x]
        voids = [False for dummy in range(npart)]
        for ipos in range(npart):
            try: dummy = pos.index(ipos)
            except: voids[ipos] = True
        if not any(voids):
            raise Exception, "Failed to find pointing gap. The algorithm requires at least 2PI/%d of pointing gap" % npart
        rot_pos = []
        if (not voids[0]) and (not voids[npart-1]):
            gmax = -1
            for idx in range(npart-2, 0, -1):
                if voids[idx]:
                    gmax = idx
                    break
            if gmax < 0:
                raise Exception, "Failed to detect gap max"
            rot_pos = range(gmax+1, npart)
        for idx in xrange(len(x)):
            x[idx] = (x[idx] - pi2) if pos[idx] in rot_pos else x[idx]

        return (x.max() - x.min())
