# sd task for imaging
import os
import numpy

from taskinit import casalog, gentools, qatool

import asap as sd
import sdutil

@sdutil.sdtask_decorator
def tsdimaging(infiles, outfile, overwrite, field, spw, antenna, scan, mode, nchan, start, step, veltype, outframe, gridfunction, convsupport, truncate, gwidth, jwidth, imsize, cell, phasecenter, ephemsrcname, pointingcolumn, restfreq, stokes, minweight):
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
        sdutil.assert_outfile_canoverwrite_or_nonexistent(self.outfile+'.weight',
                                                          'im',
                                                          self.overwrite)
        # check unit of start and step
        if self.mode == 'frequency':
            myunit = 'Hz'
        elif self.mode == 'velocity':
            myunit = 'km/s'
        else: # channel
            myunit = ''

        for name in ['start', 'step']:
            param = getattr(self, name)
            new_param = self.__format_quantum_unit(param, myunit)
            if new_param == False:
                raise ValueError, "Invalid unit for %s in mode %s: %s" % \
                      (name, self.mode, param)
            setattr(self, name, new_param)

        casalog.post("mode='%s': start=%s, step=%s, nchan=%d" % \
                     (self.mode, self.start, self.step, self.nchan))

        # check length of selection parameters
        if type(self.infiles) == str:
            nfile = 1
        else:
            nfile = len(self.infiles)

        for name in ['field', 'spw', 'antenna', 'scanno']:
            param = getattr(self, name)
            if not self.__check_selection_length(param, nfile):
                raise ValueError, "Length of %s != infiles." % (name)

    def __format_quantum_unit(self, data, unit):
        """
        Returns False if data has an unit which in not a variation of
        input unit.
        Otherwise, returns input data as a quantum string. The input
        unit is added to the return value if no unit is in data.
        """
        my_qa = qatool()
        if data == '' or my_qa.compare(qval, unit):
            return data
        if my_qa.getunit(data) == '':
            casalog.post("No unit specified. Using '%s'" % unit)
            return '%f%s' % (data, unit)
        return False

    def __check_selection_length(self, data, nfile):
        """
        Returns true if data is either a string, an array with length
        1 or nfile
        """
        if type(data) != str and len(data) not in [1, nfile]:
            return False
        return True

    def get_selection_param_for_ms(self, fileid, param):
        """
        Returns valid selection string for a certain ms

        Arguments
            fileid : file idx in infiles
            param : string (array) selection value
        """
        if type(param) == str:
            return param
        elif len(param) == 1:
            return param[0]
        else:
            return param[fileid]

    def get_selection_idx_for_ms(self, fileid):
        if fileid < len(self.infiles) and fileid > -1:
            vis = self.infiles[fileid]
            field = self.get_selection_param_for_ms(fileid, self.field)
            spw = self.get_selection_param_for_ms(fileid, self.spw)
            antenna = self.get_selection_param_for_ms(fileid, self.antenna)
            if antenna == -1: antenna = ''
            scan = self.get_selection_param_for_ms(fileid, self.scanno)
            my_ms = gentools(['ms'])[0]
            sel_ids = my_ms.msseltoindex(vis=vis, spw=spw, field=field,
                                         baseline=antenna, scan=scan)
            fieldid = list(sel_ids['field']) if len(sel_ids['field']) > 0 else -1
            spwid = list(sel_ids['spw']) if len(sel_ids['spw']) > 0 else -1
            baseline = self.format_ac_baseline(sel_ids['antenna1'])
            scanid = list(sel_ids['scan']) if len(sel_ids['scan']) > 0 else -1
            return {'field': fieldid, 'spw': spwid, 'baseline': baseline, 'scan': scanid}
        else:
            raise ValueError, ("Invalid file index, %d" % fileid)

    def format_ac_baseline(self, in_antenna):
        """ format auto-correlation baseline string from antenna idx list """
        # exact match string
        if type(in_antenna) == str:
            if (len(in_antenna) != 0) and (in_antenna.find('&') == -1) \
                   and (in_antenna.find(';')==-1):
                in_antenna =+ '&&&'
            return in_antenna
        # single integer -> list of int
        if type(in_antenna)==int:
            if in_antenna >=0:
                in_antenna = [in_antenna]
            else:
                return -1
        # format auto-corr string from antenna idices.
        baseline = ''
        for idx in in_antenna:
            if len(baseline) > 0: baseline += ';'
            if idx >= 0:
                baseline += (str(idx) + '&&&')
        return baseline

    def compile(self):
        # imaging mode
        self.imager_param['mode'] = self.mode

        # work on selection of the first table to get default restfreq and outframe
        selection_ids = self.get_selection_idx_for_ms(0)
        # field
        self.fieldid = selection_ids['field'][0] if type(selection_ids['field']) != int else selection_ids['field']
        self.sourceid=-1
        self.open_table(self.field_table)
#         field_names = self.table.getcol('NAME')
        source_ids = self.table.getcol('SOURCE_ID')
        self.close_table()
#         if type(self.field)==str:
#             try:
#                 self.fieldid = field_names.tolist().index(self.field)
#             except:
#                 msg = 'field name '+field+' not found in FIELD table of the first MS'
#                 raise ValueError, msg
#         else:
#             if self.field == -1:
#                 self.sourceid = source_ids[0]
#             elif self.field < len(field_names):
#                 self.fieldid = self.field
#                 self.sourceid = source_ids[self.field]
#             else:
#                 msg = 'field id %s does not exist in the first MS' % (self.field)
#                 raise ValueError, msg
        if self.fied == '':
            self.sourceid = source_ids[0]
        elif self.fieldid > 0 and self.fieldid < len(source_ids):
            self.sourceid = source_ids[self.fieldid]

        # restfreq
        if self.restfreq=='' and self.source_table != '':
            self.open_table(self.source_table)
            source_ids = self.table.getcol('SOURCE_ID')
            for i in range(self.table.nrows()):
                if self.sourceid == source_ids[i] \
                       and self.table.iscelldefined('REST_FREQUENCY',i) \
                       and (selection_ids['spw'] == -1 or \
                            self.table.getcell('SPECTRAL_WINDOW_ID', i) in selection_ids['spw']):
                    rf = self.table.getcell('REST_FREQUENCY',i)
                    if len(rf) > 0:
                        self.restfreq=self.table.getcell('REST_FREQUENCY',i)[0]
                        break
            self.close_table()
            casalog.post("restfreq set to %s"%self.restfreq, "INFO")
        self.imager_param['restfreq'] = self.restfreq
        
        # 
        # spw (define representative spw id = spwid_ref)
        spwid_ref = selection_ids['spw'][0] if type(selection_ids['spw']) != int else selection_ids['spw']
        # Invalid spw selection should have handled at msselectiontoindex().
        # -1 means all spw are selected.
        self.open_table(self.spw_table)
        if spwid_ref < 0:
            for id in self.table.nrows():
                if self.table.getcell('NUM_CHAN',id) > 0:
                    spwid_ref = id
                    break
            if spwid_ref < 0:
                self.close_table()
                msg='No valid spw id exists in the first table'
                raise ValueError, msg
        self.allchannels=self.table.getcell('NUM_CHAN',spwid_ref)
        self.close_table()
        self.imager_param['spw'] = -1 #spwid_ref

        # outframe (force using the current frame)
        #self.imager_param['outframe'] = ''
        self.imager_param['outframe'] = self.outframe
        if self.outframe == '':
            if len(self.infiles) > 1:
                # The default will be 'LSRK'
                casalog.post("Multiple MS inputs. The default outframe is set to 'LSRK'")
                self.imager_param['outframe'] = 'LSRK'
            else:
                # get from MS
                my_ms = gentools(['ms'])[0]
                my_ms.open(self.infiles[0])
                spwinfo = my_ms.getspectralwindowinfo()
                my_ms.close()
                del my_ms
                for key, spwval in spwinfo.items():
                    if spwval['SpectralWindowId'] == spwid_ref:
                        self.imager_param['outframe'] = spwval['Frame']
                        casalog.post("Using frequency frame of MS, '%s'" % self.imager_param['outframe'])
                        break
            if self.imager_param['outframe'] == '':
                raise Exception, "Internal error of getting frequency frame of spw=%d." % spwid_ref
        else:
            casalog.post("Using frequency frame defined by user, '%s'" % self.imager_param['outframe'])
        
#         # antenna
#         in_antenna = self.antenna # backup for future use
#         if type(self.antenna)==int:
#             if self.antenna >= 0:
#                 self.antenna=str(self.antenna)+'&&&'
#         else:
#             if (len(self.antenna) != 0) and (self.antenna.find('&') == -1) \
#                    and (self.antenna.find(';')==-1):
#                 self.antenna = self.antenna + '&&&'

        # stokes
        if self.stokes == '':
            self.stokes = 'I'
        self.imager_param['stokes'] = self.stokes

        # gridfunction

        # outfile
        if os.path.exists(self.outfile) and self.overwrite:
            os.system('rm -rf %s'%(self.outfile))
        if os.path.exists(self.outfile+'.weight') and self.overwrite:
            os.system('rm -rf %s'%(self.outfile+'.weight'))


        # cell
        cell = self.cell
        if cell == '' or cell[0] == '':
            # Calc PB
            grid_factor = 3.
            casalog.post("The cell size will be calculated using PB size")
            qPB = self._calc_PB(in_antenna)
            cell = '%f%s' % (qPB['value']/grid_factor, qPB['unit'])
            casalog.post("Using cell size = PB/%4.2F = %s" % (grid_factor, cell))

        (cellx,celly) = sdutil.get_cellx_celly(cell, unit='arcmin')
        self.imager_param['cellx'] = cellx
        self.imager_param['celly'] = celly

        # Calculate Pointing center and extent (if necessary)
        # return a dictionary with keys 'center', 'width', 'height'
        imsize = self.imsize
        phasecenter = self.phasecenter
        if self.phasecenter == "" or \
               len(self.imsize) == 0 or self.imsize[0] < 1:
            map_param = self._get_pointing_extent()
            # imsize
            if len(imsize) == 0 or imsize[0] < 1:
                imsize = self._get_imsize(map_param['width'], map_param['height'], cellx, celly)
                if self.phasecenter != "":
                    casalog.post("You defined phasecenter but not imsize. The image will cover as wide area as pointing in MS extends, but be centered at phasecenter. This could result in a strange image if your phasecenter is a part from the center of pointings", priority='warn')
                if imsize[0] > 1024 or imsize[1] > 1024:
                    casalog.post("The calculated image pixel number is larger than 1024. It could take time to generate the image depending on your computer resource. Please wait...", priority='warn')

            # phasecenter
            # if empty, it should be determined here...
            if self.phasecenter == "":
                phasecenter = map_param['center']

        # imsize
        (nx,ny) = sdutil.get_nx_ny(imsize)
        self.imager_param['nx'] = nx
        self.imager_param['ny'] = ny

        # phasecenter
        self.imager_param['phasecenter'] = phasecenter

        self.imager_param['movingsource'] = self.ephemsrcname

        # channel map
        # start
        if self.mode == 'velocity':
            #startval = ['LSRK', '%s%s'%(self.start,self.specunit)]
            if self.start == '':
                spwid_ref
            startval = [self.imager_param['outframe'], '%s%s'%(self.start,self.specunit)]
        elif self.mode == 'frequency':
            #startval = '%s%s'%(self.start,self.specunit)
            startval = [self.imager_param['outframe'], '%s%s'%(self.start,self.specunit)]
        else: #channel
            startval = self.start

        # step
        if mode in ['velocity', 'frequency']:
            stepval = '%s%s'%(self.step,self.specunit)
        else:
            stepval = self.step

        #startval = 0
        #stepval = self.allchannels
        #self.nchan = 1
        self.imager_param['start'] = startval
        self.imager_param['step'] = stepval
        self.imager_param['nchan'] = self.nchan
        

    def execute(self):
        # imaging
        casalog.post("Start imaging...", "INFO")
        casalog.post("Using phasecenter \"%s\""%(self.imager_param['phasecenter']), "INFO")
        if len(self.infiles) == 1:
            self.open_imager(self.infiles[0])
            selection_ids = self.get_selection_idx_for_ms(0)
            self.imager.selectvis(field=selection_ids['field'],\
                                  spw=selection_ids['spw'],\
                                  nchan=-1, start=0, step=1,\
                                  baseline=selection_ids['baseline'],\
                                  scan=selection_ids['scan'])
        else:
            self.close_imager()
            for idx in (len(self.infiles)):
                name = self.infiles[idx]
                selection_ids = self.get_selection_idx_for_ms(idx)
                self.imager.selectvis(vis=name, field=selection_ids['field'],\
                                      spw=selection_ids['spw'],\
                                      nchan=-1, start=0, step=1,\
                                      baseline=selection_ids['baseline'],\
                                      scan=selection_ids['scan'])
                # need to do this
                self.is_imager_opened = True
        self.imager.defineimage(**self.imager_param)#self.__get_param())
        self.imager.setoptions(ftmachine='sd', gridfunction=self.gridfunction)
        self.imager.setsdoptions(pointingcolumntouse=self.pointingcolumn, convsupport=self.convsupport, truncate=self.truncate, gwidth=self.gwidth, jwidth=self.jwidth, minweight = 0.)
        self.imager.makeimage(type='singledish', image=self.outfile)
        weightfile = self.outfile+".weight"
        self.imager.makeimage(type='coverage', image=weightfile)
        self.close_imager()

        if not os.path.exists(self.outfile):
            raise RuntimeError, "Failed to generate output image '%s'" % self.outfile
        if not os.path.exists(weightfile):
            raise RuntimeError, "Failed to generate weight image '%s'" % weightfile
        # Convert output images to proper output frame
        my_ia = gentools(['ia'])[0]
        my_ia.open(self.outfile)
        csys = my_ia.coordsys()
        csys.setconversiontype(spectral=csys.referencecode('spectra')[0])
        my_ia.setcoordsys(csys.torecord())

        my_ia.close()

        # Mask image pixels whose weight are smaller than minweight.
        # Weight image should have 0 weight for pixels below < minweight
        casalog.post("Start masking the map using minweight = %f" % \
                     self.minweight, "INFO")
        my_ia.open(weightfile)
        weight_val = my_ia.getchunk()
        valid_pixels = numpy.where(weight_val > 0.0)
        if len(valid_pixels[0]) == 0:
            my_ia.close()
            casalog.post("All pixels weight zero. This indicates no data in MS is in image area. Mask will not be set. Please check your image parameters.","WARN")
            return
        median_weight = numpy.median(weight_val[valid_pixels])
        casalog.post("Median of weight in the map is %f" % median_weight, \
                     "INFO")
        casalog.post("Pixels in map with weight <= median(weight)*minweight = %f will be masked." % \
                     (median_weight*self.minweight),"INFO")
        mask_pixels = numpy.where(weight_val <= median_weight*self.minweight)
        #weight_val[mask_pixels] = 0.
        #my_ia.putchunk(weight_val)
        my_ia.close()
        # Modify default mask
        my_ia.open(self.outfile)
        my_ia.calcmask("'%s'>%f" % (weightfile,self.minweight), asdefault=True)
        my_ia.close()
        masked_fraction = 100.*(1. - (weight_val.size - len(mask_pixels[0])) / float(len(valid_pixels[0])) )
        casalog.post("This amounts to %5.1f %% of the area with nonzero weight." % \
                    ( masked_fraction ),"INFO")
        casalog.post("The weight image '%s' is returned by this task, if the user wishes to assess the results in detail." \
                     % (weightfile), "INFO")
        del weight_val, mask_pixels, valid_pixels

    def _calc_PB(self, antenna):
        """
        Calculate the primary beam size of antenna,
        using dish diamenter and rest frequency
        """
        casalog.post("Calculating Pirimary beam size:")
        # CAS-5410 Use private tools inside task scripts
        my_qa = qatool()
        
        pb_factor = 1.175
        # Reference frequency
        ref_freq = self.restfreq
        if type(ref_freq) in [float, numpy.float64]:
            ref_freq = my_qa.tos(my_qa.quantity(ref_freq, 'Hz'))
        if not my_qa.compare(ref_freq, 'Hz'):
            msg = "Could not get the reference frequency. " + \
                  "Your data does not seem to have valid one in selected field.\n" + \
                  "PB is not calculated.\n" + \
                  "Please set restreq or cell manually to generate an image."
            raise Exception, msg
        # Antenna diameter
        self.open_table(self.antenna_table)
        antid = -1
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
        # Calculate PB
        wave_length = 0.2997924 / my_qa.convert(my_qa.quantity(ref_freq),'GHz')['value']
        D_m = my_qa.convert(antdiam_ave, 'm')['value']
        lambda_D = wave_length / D_m * 3600. * 180 / numpy.pi
        PB = my_qa.quantity(pb_factor*lambda_D, 'arcsec')
        # Summary
        casalog.post("- Antenna diameter: %s m" % D_m)
        casalog.post("- Reference Frequency: %s" % ref_freq)
        casalog.post("PB size = %5.3f * lambda/D = %s" % (pb_factor, my_qa.tos(PB)))
        return PB


    def _get_imsize(self, width, height, dx, dy):
        casalog.post("Calculating pixel size.")
        # CAS-5410 Use private tools inside task scripts
        my_qa = qatool()
        ny = numpy.ceil( ( my_qa.convert(height, my_qa.getunit(dy))['value'] /  \
                           my_qa.getvalue(dy) ) )
        nx = numpy.ceil( ( my_qa.convert(width, my_qa.getunit(dx))['value'] /  \
                           my_qa.getvalue(dx) ) )
        casalog.post("- Map extent: [%s, %s]" % (my_qa.tos(width), my_qa.tos(height)))
        casalog.post("- Cell size: [%s, %s]" % (my_qa.tos(dx), my_qa.tos(dy)))
        casalog.post("Image pixel numbers to cover the extent: [%d, %d] (projected)" % \
                     (nx+1, ny+1))
        return (int(nx+1), int(ny+1))


    def _get_pointing_extent(self):
        ### MS selection is ignored. This is not quite right.
        casalog.post("Calculating map extent from pointings.")
        # CAS-5410 Use private tools inside task scripts
        my_qa = qatool()
        ret_dict = {}
        
        colname = self.pointingcolumn.upper()
        subname = os.path.basename(self.pointing_table)
        self.open_table(self.pointing_table)
        try:
            dirinfo = self.table.getcolkeywords(colname)
            pointing = self.table.getcol(colname)
        finally:
            self.close_table()

        base_unit = dirinfo['QuantumUnits'] if dirinfo.has_key('QuantumUnits') \
                    else ['rad', 'rad']
        base_mref = dirinfo['MEASINFO']['Ref'] if dirinfo.has_key('MEASINFO') \
                    else 'J2000'

        if type(self.phasecenter) == str and len(self.phasecenter) > 0:
            rf = self.phasecenter.split()[0]
            if rf != '' and rf != base_mref:
                msg = "You are attempting to convert spatial coordinate frame. " +\
                      "Pointing extent may not accrate in that case"
                casalog.post(msg, priority='warn')

        ymax_g = my_qa.convert('-90deg', base_unit[1])
        ymin_g = my_qa.convert('90deg', base_unit[1])
        xmax_g = my_qa.convert('-360deg', base_unit[0])
        xmin_g = my_qa.convert('360deg', base_unit[0])
        for name in self.infiles:
            ptgname = name + "/" + subname
            if not os.path.exists(ptgname):
                raise Exception, "Could not find POINTING subtable, %s " % ptgname

            self.open_table(ptgname)
            try:
                dirinfo = self.table.getcolkeywords(colname)
                pointing = self.table.getcol(colname)
            finally:
                self.close_table()

            dir_unit = dirinfo['QuantumUnits'] if dirinfo.has_key('QuantumUnits') \
                       else ['rad', 'rad']
            mref = dirinfo['MEASINFO']['Ref'] if dirinfo.has_key('MEASINFO') \
                   else 'J2000'

            if mref.upper() != base_mref.upper():
                msg = "Can not calculate map extent. Coordinate references are not the same in all MSes."
                raise Exception, msg
            
            # Y-extent
            qymax_loc = my_qa.convert(my_qa.quantity(pointing[1][0].max(), dir_unit[1]), base_unit[1])
            qymin_loc = my_qa.convert(my_qa.quantity(pointing[1][0].min(), dir_unit[1]), base_unit[1])
            # X-extent
            x_to_rad = my_qa.convert(my_qa.quantity(1., dir_unit[0]), 'rad')['value']
            xrad = pointing[0][0] * x_to_rad
            del pointing
            (xmin_rad, xmax_rad) = self._get_x_minmax(xrad)
            qxmax_loc = my_qa.convert(my_qa.quantity(xmax_rad, 'rad'), base_unit[0])
            qxmin_loc = my_qa.convert(my_qa.quantity(xmin_rad, 'rad'), base_unit[0])
            # Global limit
            xmax_g = my_qa.quantity(max(xmax_g['value'], qxmax_loc['value']), base_unit[0])
            xmin_g = my_qa.quantity(min(xmin_g['value'], qxmin_loc['value']), base_unit[0])
            ymax_g = my_qa.quantity(max(ymax_g['value'], qymax_loc['value']), base_unit[1])
            ymin_g = my_qa.quantity(min(ymin_g['value'], qymin_loc['value']), base_unit[1])
        # End of infiles loop

        # POINTING center
        qcentx = my_qa.quantity( 0.5*(xmax_g['value']+xmin_g['value']), base_unit[0] )
        qcenty = my_qa.quantity( 0.5*(ymax_g['value']+ymin_g['value']), base_unit[1] )
        # POINTING extent
        qheight = my_qa.sub(ymax_g, ymin_g)
        width_rad = my_qa.convert(my_qa.sub(xmax_g, xmin_g), 'rad')['value'] * \
                    numpy.cos(my_qa.convert(qcenty, 'rad')['value'])
        qwidth = my_qa.convert(my_qa.quantity(width_rad, 'rad'), base_unit[0])
        scenter = "%s %s %s" % (base_mref, my_qa.formxxx(qcentx, "hms"), \
                  my_qa.formxxx(qcenty, "dms"))

        casalog.post("- Pointing center: %s" % scenter)
        casalog.post("- Pointing extent: [%s, %s] (projected)" % (my_qa.tos(qwidth), \
                                                                  my_qa.tos(qheight)))
        ret_dict['center'] = scenter
        ret_dict['width'] = qwidth
        ret_dict['height'] = qheight
        return ret_dict


    def _get_x_minmax(self, x):
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
            raise Exception, "Failed to find global pointing gap. The algorithm requires at least 2PI/%d of pointing gap" % npart
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

        return (x.min(), x.max())