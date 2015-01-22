from __future__ import absolute_import
import os
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.callibrary as callibrary

from pipeline.hif.heuristics import caltable as acaltable

LOG = infrastructure.get_logger(__name__)


class AntposResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(AntposResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):
        s = 'AntposResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s


class AntposInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None,
                 hm_antpos=None, antposfile=None, antenna=None, 
                 offsets=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def antenna(self):
        return self._antenna

    @antenna.setter
    def antenna(self, value):
        if value is None:
            value = ''
        self._antenna = value

    @property
    def antposfile(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('antposfile')

        if self._antposfile is None:
            vis_root = os.path.splitext(self.vis)[0]
            return vis_root + '_antposlist.txt'
        return self._antposfile

    @antposfile.setter
    def antposfile(self, value):
        self._antposfile = value

    @property
    def caltable(self):
        """
        Get the caltable argument for these inputs.
        
        If set to a table-naming heuristic, this should give a sensible name
        considering the current CASA task arguments.
        """ 
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')
        
        if callable(self._caltable):
            casa_args = self._get_task_args(ignore=('caltable',))
            return self._caltable(output_dir=self.output_dir,
                                  stage=self.context.stage,
                                  **casa_args)
        return self._caltable
        
    @caltable.setter
    def caltable(self, value):
        if value is None:
            value = acaltable.AntposCaltable()
        self._caltable = value

    @property
    def caltype(self):
        return 'antpos'

    @property
    def hm_antpos(self):
        return self._hm_antpos

    @hm_antpos.setter
    def hm_antpos(self, value):
        if value is None:
            value = 'manual'
        self._hm_antpos = value

    @property
    def offsets(self):
        return self._offsets

    @offsets.setter
    def offsets(self, value):
        if value is None:
            value = []
        self._offsets = value

    def to_casa_args(self):
        # Get the antenna and offset lists.
        if self.hm_antpos == 'manual':
            antenna = self.antenna
            offsets = self.offsets
        elif self.hm_antpos == 'file':
            filename = os.path.join(self.output_dir, self.antposfile)            
            antenna, offsets = self._read_antpos_file(filename)
        else:
            antenna = ''
            offsets = []

        return {'vis'       : self.vis,
                'caltable'  : self.caltable,
                'caltype'   : self.caltype,
                'antenna'   : antenna,
                'parameter' : offsets       }

    def _read_antpos_file(self, filename):
        """
        Read and return the contents of a file or list of files.
        """
        # If the input is a list of flagging command file names, call this
        # function recursively.  Otherwise, read in the file and return its
        # contents
        if type(filename) is types.ListType:
            return ''.join([self._add_file(f) for f in filename])

        # This assumes a very simple antenna offsets file format
        #    Blank lines are skipped
        #    Comment lines start with # and are skipped
        #    Each line must contain at least 4 white spaced delimited fields
        #        containing the antenna name, x offset, y offset, and z offset
        # Rewrite this when we know the real format
        antennas = []
        parameters = []
        with open (filename, 'r') as datafile:
            for line in datafile:
                if not line.strip():
                    continue
                if line.startswith('#'):
                    continue
                fields = line.split()
                if len(fields) < 4:
                    continue
                antennas.append(fields[0])
                parameters.extend([float(fields[1]), float(fields[2]), float(fields[3])])

        # Convert the list to a string since CASA wants it that way?
        return ','.join(antennas), parameters


class Antpos(basetask.StandardTaskTemplate):
    Inputs = AntposInputs    

    def prepare(self):
        inputs = self.inputs

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)

        calto = callibrary.CalTo(vis=inputs.vis)
#        calto = callibrary.CalTo(vis=inputs.vis),
#                                 antenna=gencal_args['antenna'])
        # careful now! Calling inputs.caltable mid-task will remove the
        # newly-created caltable, so we must look at the task arguments
        # instead
        calfrom = callibrary.CalFrom(gencal_args['caltable'],
                                     caltype='antpos',
				     spwmap=[],
				     interp='')

        calapp = callibrary.CalApplication(calto, calfrom)

        return AntposResults(pool=[calapp])

    def analyse(self, result):
        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result 

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk
        
        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]        
        result.error.clear()
        result.error.update(missing)

        return result
