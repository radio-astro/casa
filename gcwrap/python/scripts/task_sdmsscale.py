from taskinit import casalog, casac
import sdmsutil

@sdmsutil.sdtask_decorator
def sdmsscale(infile, antenna, field, spw, timerange, scan, pol, factor):
    with sdmsutil.sdtask_manager(sdmsscale_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
        

class sdmsscale_worker(sdmsutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdmsscale_worker,self).__init__(**kwargs)
        self.sdms = casac.sdms()

    def parameter_check(self):
        self.assert_no_channel_selection_in_spw(mode='error')

    def initialize_scan(self):
        self.sdms.open(self.infile)
        self.format_selection(auto_only=True, spwid_only=True)
        self.sdms.set_selection(**self.selection)

    def execute(self):
        # no scaling
        if self.factor == 1.0:
            casalog.post( "scaling factor is 1.0. No scaling" )
            return

        self.sdms.scale(self.factor)


    def save(self):
        self.sdms.close()

