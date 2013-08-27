import os
import re

from taskinit import casalog

import asap as sd
import sdutil

@sdutil.sdtask_decorator
def sdmath(expr, varlist, antenna, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, outfile, outform, overwrite):
    with sdutil.sdtask_manager(sdmath_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
        

class sdmath_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdmath_worker,self).__init__(**kwargs)

    def initialize_scan(self):
        pass
    
    def initialize(self):
        # completely override initialize

        # check expr
        if self.expr=='':
            raise Exception, 'expr is undefined'

        # check outfile
        sdutil.assert_outfile_canoverwrite_or_nonexistent(self.outfile,
                                                          self.outform,
                                                          self.overwrite)

        # save current insitu parameter and overwrite it
        self.insitu_saved = sd.rcParams['insitu']
        if self.is_disk_storage:
            sd.rcParams['insitu'] = False

    def execute(self):
        # insert varlist into expr
        varlist = self.varlist
        for key in varlist.keys():
            regex = re.compile( key )
            if isinstance( varlist[key], str ):
                self.expr = regex.sub( '\"%s\"' % varlist[key], self.expr )
            else:
                self.expr = regex.sub( "varlist['%s']" % key, self.expr )

        # default flux unit
        fluxunit_now = self.fluxunit
        
        # set filename list
        self.__parse()

        # selector
        sel = self.get_selector()

        # actual operation
        scanlist = {}
        for i in range(len(self.filenames)):
            skey='s'+str(i)
            isfactor = None
            # file type check
            if os.path.isdir( self.filenames[i] ):
                isfactor = False
            else:
                f = open( self.filenames[i] )
                line = f.readline().rstrip('\n')
                f.close()
                del f
                try:
                    isfactor = True
                    vtmp = float( line[0] )
                    del line
                except ValueError, e:
                    isfactor = False
                    del line
                
            if isfactor:
                # variable
                scanlist[skey] = sdutil.read_factor_file(self.filenames[i])
            else:
                # scantable
                thisscan=sd.scantable(self.filenames[i],average=False,antenna=self.antenna)

                # Apply the selection
                thisscan.set_selection(sel)
                if fluxunit_now == '':
                    fluxunit_now = thisscan.get_fluxunit()
                # copy scantable since convert_flux overwrites spectral data
                if self.is_disk_storage:
                    casalog.post('copy data to keep original one')
                    s = thisscan.copy()
                else:
                    s = thisscan
                sdutil.set_fluxunit(s, self.fluxunit, self.telescopeparm, True)
                scanlist[skey] = s

            #regex=re.compile('[\',\"]')
            regex=re.compile('[\',\"]%s[\',\"]' % self.filenames[i])
            #expr=regex.sub('',expr)
            self.expr=regex.sub("scanlist['%s']" % skey ,self.expr)
        self.expr="tmpout="+self.expr
        exec(self.expr)
        
        # set flux unit
        if tmpout.get_fluxunit() != fluxunit_now:
            tmpout.set_fluxunit(fluxunit_now)

        self.scan = tmpout

    def save(self):
        # avoid to call set_fluxunit
        del self.fluxunit
        
        self.set_to_scan()

        sdutil.save(self.scan, self.outfile, self.outform, self.overwrite)

    def cleanup(self):
        # restore insitu parameter
        if hasattr(self,'insitu_saved'):
            sd.rcParams['insitu'] = self.insitu_saved

    def __parse(self):
        self.filenames=[]
        #p=re.compile(r'[\',\"]\w+[\',\"]')
        #p=re.compile(r'[\',\"]\w+[\.,\-,\w+]*[\',\"]')
        p=re.compile(r'(?!varlist\[)[\',\"]\w+[\.,\-,/,\w+]*[\',\"](?!\])')
        fnames=p.findall(self.expr)
        p=re.compile('[\',\"]')
        for fname in fnames:
           fnamestr=p.sub('',fname)
           self.filenames.append(fnamestr)
           
