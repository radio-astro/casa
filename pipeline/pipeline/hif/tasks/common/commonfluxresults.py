from __future__ import absolute_import
import collections
import os

import pipeline.domain as domain
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class FluxCalibrationResults(basetask.Results):
    """
    TODO: refactor this results structure. We're falling into the dictionaries
    of dictionaries trap again.. 
    """
    def __init__(self, vis, resantenna=None, measurements=None):
        super(FluxCalibrationResults, self).__init__()

        if measurements is None:
            measurements = collections.defaultdict(list)

        self.vis = vis
        self.resantenna=resantenna
        self.measurements = measurements
        self.uncertainties = collections.defaultdict(list)

    def merge_with_context(self, context):
        ms = context.observing_run.get_ms(self.vis)
        
        for foreign_field in self.measurements:
            # resolve the potentially foreign fields to native fields
            # contained in our target measurement set
            fields = ms.get_fields(foreign_field)
            if not fields:
                msg = ('Could not find field \'%s\' in %s' % 
                       (foreign_field, ms.basename))
                LOG.error(msg)
                continue

            # recreate the measurements on each iteration so that each field
            # has independent measurements objects. Otherwise, subsequently 
            # changing the flux value for one field would change it for all 
            # fields.
            measurements = self._adopt_measurements(ms, foreign_field)

            # TODO this prints twice - fix it
            LOG.info('Recording flux density measurements for {field} '
                     'in {vis}:\n\t{measurements}'.format(
                field=','.join([f.identifier for f in fields]),
                vis=ms.basename,
                measurements='\n\t'.join([str(m) for m in measurements])))

            # Start the measurement addition process by first collecting the
            # spw IDs to which these measurements apply..
            spw_ids = sorted([m.spw_id for m in measurements])

            # A single field identifier could map to multiple field objects,
            # but the flux should be the same for all, hence we iterate..
            for field in fields:
                # .. removing any existing measurements in these spws from
                # these native fields..
                map(field.flux_densities.remove,
                    [m for m in field.flux_densities if m.spw_id in spw_ids])    
                
                # .. and then updating with our new values
                field.flux_densities.update(measurements)

    def _adopt_measurements(self, ms, foreign_field):
        # when adding results to a resumed context, we should resolve each
        # potentially foreign target entity in the result to a native
        # entity within our target context
        
        native_measurements = []
        for fm in self.measurements[foreign_field]:
            # the measurements may refer to spectral windows in other
            # measurement sets, so we replace them with spectral windows from 
            # this measurement set, identified by the same ID
            spw_id = ms.get_spectral_window(fm.spw_id)
            m = domain.FluxMeasurement(spw_id, I=fm.I, Q=fm.Q, U=fm.U, V=fm.V)                
            native_measurements.append(m)
                
        return native_measurements

    def __repr__(self):
        s = 'Flux calibration results for %s:\n' % os.path.basename(self.vis)
        for field, flux_densities in self.measurements.items():
            flux_by_spw = sorted(flux_densities, key=lambda f: f.spw_id)                    
            # rather complicated string format to display something like:
            # 0841+708 spw #0: I=3.2899 Jy; Q=0 Jy; U=0 Jy; V=0 Jy
            lines = ['\t{field} spw #{spw}: I={I}; Q={Q}; U={U}; V={V}\n'.format(
                    field=field, spw=flux.spw_id, 
                    I=str(flux.I), Q=str(flux.Q), U=str(flux.U), V=str(flux.V)) 
                    for flux in flux_by_spw]
            s += ''.join(lines)

        return s


class File(file):
    def __init__(self, *args, **kwargs):
        super(File, self).__init__(*args, **kwargs)
        self.BLOCKSIZE = 4096

    def head(self, lines_2find=1):
        self.seek(0)                            #Rewind file
        return [super(File, self).next() for _ in xrange(lines_2find)]

    def tail(self, lines_2find=1):  
        self.seek(0, 2)                         #Go to end of file
        bytes_in_file = self.tell()
        lines_found, total_bytes_scanned = 0, 0
        while (lines_2find + 1 > lines_found and
               bytes_in_file > total_bytes_scanned): 
            byte_block = min(
                self.BLOCKSIZE,
                bytes_in_file - total_bytes_scanned)
            self.seek( -(byte_block + total_bytes_scanned), 2)
            total_bytes_scanned += byte_block
            lines_found += self.read(self.BLOCKSIZE).count('\n')
        self.seek(-total_bytes_scanned, 2)
        line_list = list(self.readlines())
        return line_list[-lines_2find:]

    def backward(self):
        self.seek(0, 2)                         #Go to end of file
        blocksize = self.BLOCKSIZE
        last_row = ''
        while self.tell() != 0:
            try:
                self.seek(-blocksize, 1)
            except IOError:
                blocksize = self.tell()
                self.seek(-blocksize, 1)
            block = self.read(blocksize)
            self.seek(-blocksize, 1)
            rows = block.split('\n')
            rows[-1] = rows[-1] + last_row
            while rows:
                last_row = rows.pop(-1)
                if rows and last_row:
                    yield last_row
        yield last_row
