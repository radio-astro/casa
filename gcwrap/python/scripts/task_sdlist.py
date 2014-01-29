import asap as sd
import sdutil

@sdutil.sdtask_decorator
def sdlist(infile, antenna, outfile, overwrite):
    with sdutil.sdtask_manager(sdlist_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()


class sdlist_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdlist_worker,self).__init__(**kwargs)

    def initialize_scan(self):
        self.scan = sd.scantable(self.infile, average=False, antenna=self.antenna)

    def execute(self):
        self.scan._summary(self.outfile)

   
