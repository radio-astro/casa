# sd task for imaging
from taskinit import casalog

import asap as sd
import sdutil

@sdutil.sdtask_decorator
def tsdgrid(infiles, antenna, spw, scan, pol, gridfunction, convsupport, truncate, gwidth, jwidth, weight, clipminmax, outfile, overwrite, npix, cell, center, plot):
    with sdutil.sdtask_manager(sdgrid_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
        

class sdgrid_worker(sdutil.sdtask_interface):
    def __init__(self, **kwargs):
        super(sdgrid_worker,self).__init__(**kwargs)
        self.suffix = '.grid'
        self.gridder = None

    def initialize(self):
        self.parameter_check()
        self.__summarize_raw_inputs()
        self.__compile()
        self.__summarize_compiled_inputs()

        self.gridder = sd.asapgrid(infile=self.infiles)

    def parameter_check(self):
        self.assert_no_channel_selection_in_spw('warn')

        if self.gridfunction.upper() == 'PB':
            msg='Sorry. PB gridding is not implemented yet.'
            raise Exception, msg
        elif self.gridfunction.upper() == 'BOX':
            casalog.post('convsupport is automatically set to -1 for BOX gridding.', priority='WARN')
            self.convsupport = -1

    def execute(self):
        self.gridder.setPolList(self.pols)
        self.gridder.setScanList(self.scans)
        if self.ifno >= 0:
            self.gridder.setIF(self.ifno)
        if self.clipminmax:
            self.gridder.enableClip()
        else:
            self.gridder.disableClip()
        self.gridder.setWeight(self.weight)
        self.gridder.defineImage(nx=self.nx, ny=self.ny,
                                 cellx=self.cellx, celly=self.celly,
                                 center=self.mapcenter)
        self.gridder.setFunc(func=self.gridfunction,
                             convsupport=self.convsupport,
                             truncate=str(self.truncate),
                             gwidth=str(self.gwidth),
                             jwidth=str(self.jwidth))
        self.gridder.grid()

    def finalize(self):
        self.gridder.save(outfile=self.outname)

        if self.plot:
            self.gridder.plot()
            
    def __compile(self):
        # infiles
        if isinstance(self.infiles, str):
            self.infiles = [self.infiles]

        # scantable for temporary use
        tmpst = sd.scantable(self.infiles[0], False)

        # scanlist
        #self.scans = sdutil._to_list(self.scanlist, int)
        self.scans = tmpst.parse_idx_selection("SCAN", self.scanno)

        # pollist
        #self.pols = sdutil._to_list(self.pollist, int)
        self.pols = tmpst.parse_idx_selection("POL", self.polno)

        # spw
        if (self.spw.strip() == '-1'):
            self.ifno = tmpst.getif(0)
        else:
            masklist = tmpst.parse_spw_selection(self.spw)
            if len(masklist) == 0:
                raise ValueError, "Invalid spectral window selection. Selection contains no data."
            self.ifno = masklist.keys()[0]
        
        # outfile
        self.outname = sdutil.get_default_outfile_name(self.infiles[0],
                                                       self.outfile,
                                                       self.suffix)
        sdutil.assert_outfile_canoverwrite_or_nonexistent(self.outname,
                                                          'ASAP',
                                                          self.overwrite)
        
        # nx and ny
        (self.nx, self.ny) = sdutil.get_nx_ny(self.npix)

        # cellx and celly
        (self.cellx, self.celly) = sdutil.get_cellx_celly(self.cell)

        # map center
        self.mapcenter = sdutil.get_map_center(self.center)

        del tmpst

    def __summarize_raw_inputs(self):
        params = ['infiles', 'antenna', 'scanno', 'spw',
                  'polno', 'gridfunction', 'convsupport',
                  'truncate', 'gwidth', 'jwidth', 'weight',
                  'clipminmax', 'outfile', 'overwrite',
                  'npix', 'cell', 'center', 'plot']
        summary = self.__generate_summary(header='Input Parameter Summary',
                                          params=params)
        casalog.post(summary, priority='DEBUG')

    def __summarize_compiled_inputs(self):
        params = ['infiles', 'ifno', 'scans', 'pols',
                  'gridfunction', 'convsupport',
                  'truncate', 'gwidth', 'jwidth', 'weight',
                  'clipminmax', 'outname', 'overwrite',
                  'nx', 'ny', 'cellx', 'celly',
                  'mapcenter', 'plot']
        summary = self.__generate_summary(header='Grid Parameter Summary',
                                          params=params)
        casalog.post(summary, priority='DEBUG')
        
    def __generate_summary(self, header, params):
        summary = header+':\n'
        for p in params:
            summary += '   %12s = %s\n'%(p,getattr(self,p))
        return summary

    def assert_no_channel_selection_in_spw(self, mode='warn'):
        """
        Assert 'spw' does not have channel selection
        Returns True if spw string does not have channel selecton
        Returns False or raises an error if spw has channel selection

        Available modes are
            'result' : just returns the result (true or false)
            'warn'   : warn user if channel selection is set
            'error'  : raise an error if channel seledtion is set
        """
        if not hasattr(self, 'spw'): return True
        # find pattern spw = 'spwID:channelRange'
        has_chan = (self.spw.find(':') > -1)
        ## TODO: also need to do something with "###Hz" and "###km/s"?
        #quantap = re.compile('[a-z]', re.IGNORECASE)
        #has_chan = has_chan or len(quantap.findall(self.spw))
        if has_chan:
            if mode.upper().startswith('E'):
                raise ValueError, "spw parameter should not contain channel selection."
            elif mode.upper().startswith('W'):
                casalog.post("Channel selection found in spw parameter. It would be ignored", priority='WARN')
        
        return has_chan

