from __future__ import absolute_import

import csv
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as acaltable
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

__all__ = [
    'Antpos',
    'AntposInputs',
    'AntposResults'
]

LOG = infrastructure.get_logger(__name__)


class AntposResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], antenna='', offsets=[]):
        super(AntposResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.antenna = antenna
        self.offsets = offsets

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No antenna position results to merge')
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


class AntposInputs(vdp.StandardInputs):
    """
    AntposInputs defines the inputs for the Antpos pipeline task.
    """
    antenna = vdp.VisDependentProperty(default='')
    antposfile = vdp.VisDependentProperty(default='antennapos.csv')
    hm_antpos = vdp.VisDependentProperty(default='manual')

    @vdp.VisDependentProperty
    def offsets(self):
        return []

    @vdp.VisDependentProperty
    def caltable(self):
        """
        Get the caltable argument for these inputs.

        If set to a table-naming heuristic, this should give a sensible name
        considering the current CASA task arguments.
        """
        namer = acaltable.AntposCaltable()
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    def __init__(self, context, output_dir=None, vis=None, caltable=None, hm_antpos=None, antposfile=None, antenna=None,
                 offsets=None):
        super(AntposInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.antenna = antenna
        self.antposfile = antposfile
        self.caltable = caltable

        # solution parameters
        self.hm_antpos = hm_antpos
        self.offsets = offsets

    def to_casa_args(self):
        # Get the antenna and offset lists.
        if self.hm_antpos == 'manual':
            antenna = self.antenna
            offsets = self.offsets
        elif self.hm_antpos == 'file':
            filename = os.path.join(self.output_dir, self.antposfile)           
            antenna, offsets = self._read_antpos_csvfile(
                filename, os.path.basename(self.vis))
        else:
            antenna = ''
            offsets = []

        return {'vis': self.vis,
                'caltable': self.caltable,
                'antenna': antenna,
                'parameter': offsets}

    def _read_antpos_txtfile(self, filename):
        """
        Read and return the contents of a file or list of files.
        """
        # If the input is a list of flagging command file names, call this
        # function recursively.  Otherwise, read in the file and return its
        # contents
        # FIXME: missing _add_file method
        if isinstance(filename, list):
            return ''.join([self._add_file(f) for f in filename])

        # This assumes a very simple antenna offsets file format
        #    Blank lines are skipped
        #    Comment lines start with # and are skipped
        #    Each line must contain at least 4 white spaced delimited fields
        #        containing the antenna name, x offset, y offset, and z offset
        # Rewrite this when we know the real format
        antennas = []
        parameters = []
        with open(filename, 'r') as datafile:
            for line in datafile:
                if not line.strip():
                    continue
                if line.startswith('#'):
                    continue
                fields = line.split()
                if len(fields) < 4:
                    continue
                antennas.append(fields[0])
                parameters.extend(
                    [float(fields[1]), float(fields[2]), float(fields[3])])

        # Convert the list to a string since CASA wants it that way?
        return ','.join(antennas), parameters

    @staticmethod
    def _read_antpos_csvfile(filename, msbasename):
        """
        Read and return the contents of a file or list of files.
        """

        # This assumes a very simple csv file format containing the following
        # columns
        #    ms
        #    antenna
        #    xoffset in meters
        #    yoffset in meters
        #    zoffset in meters
        #    comment
        # Rewrite this when we know the real format
        antennas = []
        parameters = []

        if not os.path.exists(filename):
            LOG.warn('Antenna position offsets file does not exist')
            return ','.join(antennas), parameters

        with open(filename, 'rt') as f:
            reader = csv.reader(f)

            # First row is header row
            reader.next()

            # Loop over the rows
            for row in reader:
                (ms_name, ant_name, xoffset, yoffset, zoffset, _) = row
                if ms_name != msbasename:
                    continue
                antennas.append(ant_name)
                parameters.extend(
                    [float(xoffset), float(yoffset), float(zoffset)])

        # Convert the list to a string since CASA wants it that way?
        return ','.join(antennas), parameters


@task_registry.set_equivalent_casa_task('hif_antpos')
class Antpos(basetask.StandardTaskTemplate):
    Inputs = AntposInputs    

    def prepare(self):
        inputs = self.inputs

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(caltype='antpos', **gencal_args)
        if inputs.hm_antpos == 'file' and gencal_args['antenna'] == '':
            LOG.info('No antenna position offsets are defined')
        else:
            self._executor.execute(gencal_job)

        calto = callibrary.CalTo(vis=inputs.vis)
        # careful now! Calling inputs.caltable mid-task will remove the
        # newly-created caltable, so we must look at the task arguments
        # instead
        calfrom = callibrary.CalFrom(gencal_args['caltable'],
                                     caltype='antpos',
                                     spwmap=[],
                                     interp='', calwt=False)

        calapp = callibrary.CalApplication(calto, calfrom)

        return AntposResults(pool=[calapp], antenna=gencal_args['antenna'],
                             offsets=gencal_args['parameter'])

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
