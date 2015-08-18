# sd task for imaging
import os
import re
import numpy

from taskinit import casalog, gentools, qatool

import asap as sd
import sdutil
import sdbeamutil
from cleanhelper import cleanhelper

@sdutil.sdtask_decorator
def sdimaging(infiles, outfile, overwrite, field, spw, antenna, scan, intent, mode, nchan, start, width, veltype, outframe, gridfunction, convsupport, truncate, gwidth, jwidth, imsize, cell, phasecenter, ephemsrcname, pointingcolumn, restfreq, stokes, minweight):
    with sdutil.sdtask_manager(sdimaging_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()    

def is_string_type(val):
    """
    Returns True if the argument is string type.
    """
    return type(val) in [str, numpy.string_]

class sdimaging_worker(sdutil.sdtask_template_imaging):
    def __init__(self, **kwargs):
        super(sdimaging_worker,self).__init__(**kwargs)
        self.imager_param = {}
        self.sorted_idx = []

    def parameter_check(self):
        # outfile check
        sdutil.assert_outfile_canoverwrite_or_nonexistent(self.outfile,
                                                          'im',
                                                          self.overwrite)
        sdutil.assert_outfile_canoverwrite_or_nonexistent(self.outfile+'.weight',
                                                          'im',
                                                          self.overwrite)
        # fix spw
        if type(self.spw) == str:
            self.spw = self.__format_spw_string(self.spw)
        # check unit of start and width
        # fix default
        if self.mode == 'channel':
            if self.start == '': self.start = 0
            if self.width == '': self.width = 1
        else:
            if self.start == 0: self.start = ''
            if self.width == 1: self.width = ''
        # fix unit
        if self.mode == 'frequency':
            myunit = 'Hz'
        elif self.mode == 'velocity':
            myunit = 'km/s'
        else: # channel
            myunit = ''

        for name in ['start', 'width']:
            param = getattr(self, name)
            new_param = self.__format_quantum_unit(param, myunit)
            if new_param == None:
                raise ValueError, "Invalid unit for %s in mode %s: %s" % \
                      (name, self.mode, param)
            setattr(self, name, new_param)

        casalog.post("mode='%s': start=%s, width=%s, nchan=%d" % \
                     (self.mode, self.start, self.width, self.nchan))
        

        # check length of selection parameters
        if is_string_type(self.infiles):
            nfile = 1
            self.infiles = [ self.infiles ]
        else:
            nfile = len(self.infiles)

        for name in ['field', 'spw', 'antenna', 'scanno']:
            param = getattr(self, name)
            if not self.__check_selection_length(param, nfile):
                raise ValueError, "Length of %s != infiles." % (name)

    def __format_spw_string(self, spw):
        """
        Returns formatted spw selection string which is accepted by imager.
        """
        if type(spw) != str:
            raise ValueError, "The parameter should be string."
        if spw.strip() == '*': spw = ''
        # WORKAROUND for CAS-6422, i.e., ":X~Y" fails while "*:X~Y" works.
        if spw.startswith(":"): spw = '*' + spw
        return spw        

    def __format_quantum_unit(self, data, unit):
        """
        Returns False if data has an unit which in not a variation of
        input unit.
        Otherwise, returns input data as a quantum string. The input
        unit is added to the return value if no unit is in data.
        """
        my_qa = qatool()
        if data == '' or my_qa.compare(data, unit):
            return data
        if my_qa.getunit(data) == '':
            casalog.post("No unit specified. Using '%s'" % unit)
            return '%f%s' % (data, unit)
        return None

    def __check_selection_length(self, data, nfile):
        """
        Returns true if data is either a string, an array with length
        1 or nfile
        """
        if not is_string_type(data) and len(data) not in [1, nfile]:
            return False
        return True

    def get_selection_param_for_ms(self, fileid, param):
        """
        Returns valid selection string for a certain ms

        Arguments
            fileid : file idx in infiles list
            param : string (array) selection value
        """
        if is_string_type(param):
            return param
        elif len(param) == 1:
            return param[0]
        else:
            return param[fileid]

    def get_selection_idx_for_ms(self, file_idx):
        """
        Returns a dictionary of selection indices for i-th MS in infiles

        Argument: file idx in infiles list
        """
        if file_idx < len(self.infiles) and file_idx > -1:
            vis = self.infiles[file_idx]
            field = self.get_selection_param_for_ms(file_idx, self.field)
            spw = self.get_selection_param_for_ms(file_idx, self.spw)
            spw = self.__format_spw_string(spw)
            antenna = self.get_selection_param_for_ms(file_idx, self.antenna)
            if antenna == -1: antenna = ''
            scan = self.get_selection_param_for_ms(file_idx, self.scanno)
            intent = self.get_selection_param_for_ms(file_idx, self.intent) 
            my_ms = gentools(['ms'])[0]
            sel_ids = my_ms.msseltoindex(vis=vis, spw=spw, field=field,
                                         baseline=antenna, scan=scan)
            fieldid = list(sel_ids['field']) if len(sel_ids['field']) > 0 else -1
            baseline = self.format_ac_baseline(sel_ids['antenna1'])
            scanid = list(sel_ids['scan']) if len(sel_ids['scan']) > 0 else ""
            # SPW (need to get a list of valid spws instead of -1)
            if len(sel_ids['channel']) > 0:
                spwid = [ chanarr[0] for chanarr in sel_ids['channel'] ]
            elif spw=="": # No spw selection
                my_ms.open(vis)
                try: spwinfo =  my_ms.getspectralwindowinfo()
                except: raise
                finally: my_ms.close()
                
                spwid = [int(idx) for idx in spwinfo.keys()]
            else:
                raise RuntimeError("Invalid spw selction, %s ,for MS %d" (str(spw), file_idx))
            
            return {'field': fieldid, 'spw': spwid, 'baseline': baseline, 'scan': scanid, 'intent': intent, 'antenna1': sel_ids['antenna1']} 
        else:
            raise ValueError, ("Invalid file index, %d" % file_idx)

    def format_ac_baseline(self, in_antenna):
        """ format auto-correlation baseline string from antenna idx list """
        # exact match string
        if  is_string_type(in_antenna):
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

        # Work on selection of the first table in sorted list
        # to get default restfreq and outframe
        imhelper = cleanhelper(self.imager, self.infiles, casalog=casalog)
        imhelper.sortvislist(self.spw, self.mode, self.width)
        self.sorted_idx = imhelper.sortedvisindx
        selection_ids = self.get_selection_idx_for_ms(self.sorted_idx[0])
        self.__update_subtable_name(self.infiles[self.sorted_idx[0]])
        # field
        fieldid = selection_ids['field'][0] if type(selection_ids['field']) != int else selection_ids['field']
        sourceid=-1
        self.open_table(self.field_table)
        source_ids = self.table.getcol('SOURCE_ID')
        self.close_table()
        if self.field == '' or fieldid ==-1:
            sourceid = source_ids[0]
        elif fieldid >= 0 and fieldid < len(source_ids):
            sourceid = source_ids[fieldid]
        else:
            raise ValueError, "No valid field in the first MS."

        # restfreq
        if self.restfreq=='' and self.source_table != '':
            self.open_table(self.source_table)
            source_ids = self.table.getcol('SOURCE_ID')
            for i in range(self.table.nrows()):
                if sourceid == source_ids[i] \
                       and self.table.iscelldefined('REST_FREQUENCY',i) \
                       and (selection_ids['spw'] == -1 or \
                            self.table.getcell('SPECTRAL_WINDOW_ID', i) in selection_ids['spw']):
                    rf = self.table.getcell('REST_FREQUENCY',i)
                    if len(rf) > 0:
                        self.restfreq=self.table.getcell('REST_FREQUENCY',i)[0]
                        break
            self.close_table()
            casalog.post("restfreq set to %s" % self.restfreq, "INFO")
        # REST_FREQUENCY column is optional (need retry if not exists)
        self.imager_param['restfreq'] = self.restfreq
    
        # 
        # spw (define representative spw id = spwid_ref)
        spwid_ref = selection_ids['spw'][0] if type(selection_ids['spw']) != int else selection_ids['spw']
        # Invalid spw selection should have handled at msselectiontoindex().
        # -1 means all spw are selected.
        self.open_table(self.spw_table)
        if spwid_ref < 0:
            for id in range(self.table.nrows()):
                if self.table.getcell('NUM_CHAN',id) > 0:
                    spwid_ref = id
                    break
            if spwid_ref < 0:
                self.close_table()
                msg='No valid spw id exists in the first table'
                raise ValueError, msg
        self.allchannels = self.table.getcell('NUM_CHAN',spwid_ref)
        freq_chan0 = self.table.getcell('CHAN_FREQ',spwid_ref)[0]
        freq_inc0 = self.table.getcell('CHAN_WIDTH',spwid_ref)[0]
        # in case rest frequency is not defined yet.
        if self.restfreq=='':
            self.restfreq = '%fHz' % self.table.getcell('CHAN_FREQ',spwid_ref).mean()
            self.imager_param['restfreq'] = self.restfreq
            casalog.post("Using mean freq of spw %d as restfreq: %s" %
                         (spwid_ref, self.restfreq), "INFO")
        self.close_table()
        self.imager_param['spw'] = -1 #spwid_ref

        # outframe (force using the current frame)
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

    def _configure_map_property(self):
        selection_ids = self.get_selection_idx_for_ms(self.sorted_idx[0])

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
            casalog.post("The cell size will be calculated using PB size of antennas in the first MS")
            qPB = self._calc_PB(selection_ids['antenna1'])
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
        imhelper = cleanhelper(self.imager, self.infiles, casalog=casalog)
        imhelper.sortvislist(self.spw, self.mode, self.width)
        spwsel = str(',').join([str(spwid) for spwid in selection_ids['spw']])
        srestf = self.imager_param['restfreq'] if is_string_type(self.imager_param['restfreq']) else "%fHz" % self.imager_param['restfreq']
        (imnchan, imstart, imwidth) = imhelper.setChannelizeDefault(self.mode, spwsel, self.field, self.nchan, self.start, self.width, self.imager_param['outframe'], self.veltype,self.imager_param['phasecenter'], srestf)
        del imhelper
        
        # start and width
        if self.mode == 'velocity':
            startval = [self.imager_param['outframe'], imstart]
            widthval = imwidth
        elif self.mode == 'frequency':
            startval = [self.imager_param['outframe'], imstart]
            widthval = imwidth
        else: #self.mode==channel
            startval = int(self.start)
            widthval = int(self.width)

        if self.nchan < 0: self.nchan = self.allchannels
        self.imager_param['start'] = startval
        self.imager_param['step'] = widthval
        self.imager_param['nchan'] = imnchan #self.nchan

        

    def execute(self):
        # imaging
        casalog.post("Start imaging...", "INFO")
        if len(self.infiles) == 1:
            self.open_imager(self.infiles[0])
            selection_ids = self.get_selection_idx_for_ms(0)
            spwsel = self.get_selection_param_for_ms(0, self.spw)
            if spwsel.strip() in ['', '*']: spwsel = selection_ids['spw']
            ### TODO: channel selection based on spw
            self.imager.selectvis(field=selection_ids['field'],
                                  #spw=selection_ids['spw'],
                                  spw=spwsel,
                                  nchan=-1, start=0, step=1,
                                  baseline=selection_ids['baseline'],
                                  scan=selection_ids['scan'],
                                  intent=selection_ids['intent']) 
        else:
            self.close_imager()
            self.sorted_idx.reverse()
#             for idx in (len(self.infiles)):
            for idx in self.sorted_idx:
                name = self.infiles[idx]
                selection_ids = self.get_selection_idx_for_ms(idx)
                spwsel = self.get_selection_param_for_ms(idx, self.spw)
                if spwsel.strip() in ['', '*']: spwsel = selection_ids['spw']
                ### TODO: channel selection based on spw
                self.imager.selectvis(vis=name, field=selection_ids['field'],
                                      #spw=selection_ids['spw'],
                                      spw = spwsel,
                                      nchan=-1, start=0, step=1,
                                      baseline=selection_ids['baseline'],
                                      scan=selection_ids['scan'],
                                      intent=selection_ids['intent'])
                # need to do this
                self.is_imager_opened = True
                
        # it should be called after infiles are registered to imager
        self._configure_map_property()
        
        casalog.post("Using phasecenter \"%s\""%(self.imager_param['phasecenter']), "INFO")

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
        try:
            stat=my_ia.statistics(mask="'"+weightfile+"' > 0.0", robust=True)
            valid_pixels=stat['npts']
        except RuntimeError, e:
            if e.message.find('No valid data found.') >= 0:
                valid_pixels = [0]
            else:
                raise e
        if len(valid_pixels) == 0 or valid_pixels[0] == 0:
            my_ia.close()
            casalog.post("All pixels weight zero. This indicates no data in MS is in image area. Mask will not be set. Please check your image parameters.","WARN")
            return
        median_weight = stat['median'][0]
        casalog.post("Median of weight in the map is %f" % median_weight, \
                     "INFO")
        casalog.post("Pixels in map with weight <= median(weight)*minweight = %f will be masked." % \
                     (median_weight*self.minweight),"INFO")
        ###Leaving the original logic to calculate the number of masked pixels via
        ###product of median of and min_weight (which i don't understand the logic)
        ### if one wanted to find how many pixel were masked one could easily count the
        ### number of pixels set to false 
        ### e.g  after masking self.outfile below one could just do this 
        ### nmasked_pixels=tb.calc('[select from "'+self.outfile+'"/mask0'+'"  giving [nfalse(PagedArray )]]')
        my_tb = gentools(['tb'])[0]
        nmask_pixels=0
        nchan=stat['trc'][3]+1
        casalog.filter('ERROR') ### hide the useless message of tb.calc
    
       
        ### doing it by channel to make sure it does not go out of memory
        ####tab.calc try to load the whole chunk in ram 
        for k in range(nchan):
            nmask_pixels += my_tb.calc('[select from "'+weightfile+'"  giving [ntrue(map[,,,'+str(k)+'] <='+str(median_weight*self.minweight)+')]]')['0'][0]
        casalog.filter()  ####set logging back to normal
        
        casalog.filter()  ####set logging back to normal
        imsize=numpy.product(my_ia.shape())
        my_ia.close()
        # Modify default mask
        my_ia.open(self.outfile)
        my_ia.calcmask("'%s'>%f" % (weightfile,self.minweight), asdefault=True)
        my_ia.close()
        masked_fraction = 100.*(1. - (imsize - nmask_pixels) / float(valid_pixels[0]) )
        casalog.post("This amounts to %5.1f %% of the area with nonzero weight." % \
                    ( masked_fraction ),"INFO")
        casalog.post("The weight image '%s' is returned by this task, if the user wishes to assess the results in detail." \
                     % (weightfile), "INFO")
        
        # Calculate theoretical beam size
        casalog.post("Calculating image beam size.")
        if self.gridfunction.upper() not in  ['SF']:
            casalog.post("Beam size definition for '%s' kernel is experimental." % self.gridfunction, priority='WARN')
            casalog.post("You may want to take careful look at the restoring beam in the image.",priority='WARN')
        my_msmd = gentools(['msmd'])[0]
        # antenna diameter and blockage
        ref_ms_idx = self.sorted_idx[0]
        ref_ms_name = self.infiles[ref_ms_idx]
        selection_ids = self.get_selection_idx_for_ms(ref_ms_idx)
        ant_idx = selection_ids['antenna1']
        diameter = self._get_average_antenna_diameter(ant_idx)
        my_msmd.open(ref_ms_name)
        ant_name = my_msmd.antennanames(ant_idx)
        my_msmd.close()
        is_alma = False
        for name in ant_name:
            if name[0:2] in ["PM", "DV", "DA", "CM"]:
                is_alma = True
                break
        blockage = "0.75m" if is_alma else "0.0m" # unknown blockage diameter
        # output reference code
        my_ia.open(self.outfile)
        csys = my_ia.coordsys()
        my_ia.close()
        outref = csys.referencecode('direction')[0]
        cell = list(csys.increment(type='direction',format='s')['string'])
        # pointing sampling
        ref_ms_spw = self.get_selection_param_for_ms(ref_ms_idx,self.spw)
        ref_ms_field = self.get_selection_param_for_ms(ref_ms_idx,self.field)
        ref_ms_scan = self.get_selection_param_for_ms(ref_ms_idx,self.scanno)
        xSampling, ySampling, angle = sdutil.get_ms_sampling_arcsec(ref_ms_name, spw=ref_ms_spw,
                                                                    antenna=selection_ids['baseline'],
                                                                    field=ref_ms_field,
                                                                    scan=ref_ms_scan,#timerange='',
                                                                    outref=outref)
        casalog.post("Detected raster sampling = [%f, %f] arcsec" %
                     (xSampling, ySampling))
        # handling of failed sampling detection
        valid_sampling = True
        sampling = [xSampling, ySampling]
        if abs(xSampling) < 1.0e-3 or numpy.isnan(xSampling):
            casalog.post("Invalid sampling=%s arcsec. Using the value of orthogonal direction=%s arcsec" % (xSampling, ySampling), priority="WARN")
            sampling = [ ySampling ]
            valid_sampling = False
        if abs(ySampling) < 1.0e-3 or numpy.isnan(ySampling):
            if valid_sampling:
                casalog.post("Invalid sampling=%s arcsec. Using the value of orthogonal direction=%s arcsec" % (ySampling, xSampling), priority="WARN")
                sampling = [ xSampling ]
                valid_sampling = True
        # reduce sampling and cell if it's possible
        if len(sampling)>1 and sampling[0]==sampling[1]:
            sampling = [sampling[0]]
            if cell[0]==cell[1]: cell = [cell[0]]
        if valid_sampling:
            # actual calculation of beam size
            bu = sdbeamutil.TheoreticalBeam()
            bu.set_antenna(diameter,blockage)
            bu.set_sampling(sampling, "%fdeg" % angle)
            bu.set_image_param(cell, self.restfreq, self.gridfunction,
                               self.convsupport, self.truncate, self.gwidth,
                               self.jwidth,is_alma)
            bu.summary()
            imbeam_dict = bu.get_beamsize_image()
            casalog.post("Setting image beam: major=%s, minor=%s, pa=%s" %
                         (imbeam_dict['major'], imbeam_dict['minor'],
                          imbeam_dict['pa'],))
            # set beam size to image
            my_ia.open(self.outfile)
            my_ia.setrestoringbeam(**imbeam_dict)
            my_ia.close()
        else:
            #BOTH sampling was invalid
            casalog.post("Could not detect valid raster sampling. Exitting without setting beam size to image", priority='WARN')


    def _calc_PB(self, antenna):
        """
        Calculate the primary beam size of antenna, using dish diamenter
        and rest frequency
        Average antenna diamter and reference frequency are adopted for
        calculation.
        The input argument should be a list of antenna IDs.
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
        antdiam_ave = self._get_average_antenna_diameter(antenna)
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

        # MSs should be registered to imager
        if not self.is_imager_opened:
            raise RuntimeError('Internal error: imager should be opened here.')
        
        if self.phasecenter == "":
            # defaut is J2000
            base_mref = 'J2000'
        elif isinstance(self.phasecenter, int) or self.phasecenter.isdigit():
            # may be field id
            self.open_table(self.field_table)
            base_mref = self.table.getcolkeyword('PHASE_DIR', 'MEASINFO')['Ref']
            self.close_table()
        else:
            # may be phasecenter is explicitly specified
            pattern = '^([\-\+]?[0-9.]+([eE]?-?[0-9])?)|([\-\+]?[0-9][:h][0-9][:m][0-9.]s?)|([\-\+]?[0-9][.d][0-9][.d][0-9.]s?)$'
            items = self.phasecenter.split()
            base_mref = 'J2000'
            for i in items:
                s = i.strip()
                if re.match(pattern, s) is None:
                    base_mref = s
                    break

        mapextent = self.imager.mapextent(ref=base_mref, movingsource=self.ephemsrcname, 
                                          pointingcolumntouse=colname)
        if mapextent['status'] is True:
            qheight = my_qa.quantity(mapextent['extent'][1], 'rad')
            qwidth = my_qa.quantity(mapextent['extent'][0], 'rad')
            qcent0 = my_qa.quantity(mapextent['center'][0], 'rad')
            qcent1 = my_qa.quantity(mapextent['center'][1], 'rad')
            scenter = '%s %s %s'%(base_mref, my_qa.formxxx(qcent0, 'hms'), 
                                  my_qa.formxxx(qcent1, 'dms'))

            casalog.post("- Pointing center: %s" % scenter)
            casalog.post("- Pointing extent: [%s, %s] (projected)" % (my_qa.tos(qwidth), \
                                                                  my_qa.tos(qheight)))
            ret_dict['center'] = scenter
            ret_dict['width'] = qwidth
            ret_dict['height'] = qheight
        else:
            casalog.post('Failed to derive map extent from the MSs registered to the imager probably due to mising valid data.', priority='SEVERE')
            ret_dict['center'] = ''
            ret_dict['width'] = my_qa.quantity(0.0, 'rad')
            ret_dict['height'] = my_qa.quantity(0.0, 'rad')
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

    def __update_subtable_name(self, msname):
        self.open_table(msname)
        keys = self.table.getkeywords()
        self.close_table()
        self.field_table = sdutil.get_subtable_name(keys['FIELD'])
        self.spw_table = sdutil.get_subtable_name(keys['SPECTRAL_WINDOW'])
        self.source_table = sdutil.get_subtable_name(keys['SOURCE'])
        self.antenna_table = sdutil.get_subtable_name(keys['ANTENNA'])
        self.polarization_table = sdutil.get_subtable_name(keys['POLARIZATION'])
        self.observation_table = sdutil.get_subtable_name(keys['OBSERVATION'])
        self.pointing_table = sdutil.get_subtable_name(keys['POINTING'])
        self.data_desc_table = sdutil.get_subtable_name(keys['DATA_DESCRIPTION'])
        self.pointing_table = sdutil.get_subtable_name(keys['POINTING'])        

    def _get_average_antenna_diameter(self, antenna):
        my_qa = qatool()
        self.open_table(self.antenna_table)
        try:
            antdiam_unit = self.table.getcolkeyword('DISH_DIAMETER', 'QuantumUnits')[0]
            diams = self.table.getcol('DISH_DIAMETER')
        finally:
            self.close_table()

        if len(antenna) == 0:
            antdiam_ave = my_qa.quantity(diams.mean(), antdiam_unit)
        else:
            d_ave = sum([diams[idx] for idx in antenna])/float(len(antenna))
            antdiam_ave = my_qa.quantity(d_ave, antdiam_unit)
        return antdiam_ave
