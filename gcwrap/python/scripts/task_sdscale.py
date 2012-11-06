import os
import sys
from taskinit import *
import sdutil
import asap as sd
from asap.scantable import is_scantable, is_ms

def sdscale(infile, antenna, factor, scaletsys, outfile, overwrite):
    
    casalog.origin('sdscale')
    
    try:
        worker = sdscale_worker(**locals())
        worker.initialize()
        worker.execute()
        worker.finalize()
        
    except Exception, instance:
        sdutil.process_exception(instance)
        raise Exception, instance

class sdscale_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdscale_worker,self).__init__(**kwargs)
        self.suffix = '_scaled%s'%(self.factor)
        self.outform = self.__get_outform()

    def parameter_check(self):
        if isinstance( self.factor, str ):
            casalog.post( 'read factor from \'%s\'' %self.factor )
            f = open( self.factor )
            lines = f.readlines()
            f.close()
            del f
            for i in range( len(lines) ):
                lines[i] = lines[i].rstrip('\n')
                lines[i] = lines[i].split()
                for j in range( len(lines[i]) ):
                    lines[i][j] = float( lines[i][j] )
            self.factor = lines        

    def initialize_scan(self):
        self.scan = sd.scantable(self.infile, average=False, antenna=self.antenna)

    def execute(self):
        # no scaling
        if self.factor == 1.0:
            casalog.post( "scaling factor is 1.0. No scaling" )
            return

        s2 = self.scan.scale(self.factor, self.scaletsys, False)
        casalog.post( "Scaled spectra and Tsys by "+str(self.factor) )

        if self.scaletsys:
            oldtsys=self.scan._row_callback(self.scan._gettsys, "Original Tsys")
            newtsys=s2._row_callback(s2._gettsys, "Scaled Tsys")
        else:
            oldtsys=s2._row_callback(s2._gettsys, "Tsys (not scaled)")
        self.scan = s2

    def save(self):
        sdutil.save(self.scan, self.outfile, self.outform, self.overwrite)
        casalog.post( "Wrote scaled data to %s file, %s " % (self.outform, self.outfile) )

    def __get_outform(self):
        if is_scantable(self.infile):
            outform = 'ASAP'
        elif is_ms(infile):
            outform = 'MS2'
        else:
            outform = 'SDFITS'
        return outform

