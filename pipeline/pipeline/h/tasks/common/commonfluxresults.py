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
    def __init__(self, vis, resantenna=None, uvrange=None, measurements=None):
        super(FluxCalibrationResults, self).__init__()

        if resantenna is None:
            resantenna = ''

        if uvrange is None:
            uvrange = ''

        if measurements is None:
            measurements = collections.defaultdict(list)

        self.vis = vis
        self.resantenna=resantenna
        self.uvrange=uvrange
        self.measurements = measurements

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

            # A single field identifier could map to multiple field objects,
            # but the flux should be the same for all, hence we iterate..
            for field in fields:
                # .. removing any existing measurements of this origin
                for new_m in measurements:
                    to_replace = [m for m in field.flux_densities
                                  if m.origin == new_m.origin and m.spw_id == new_m.spw_id]
                    map(field.flux_densities.remove, to_replace)

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
            m = domain.FluxMeasurement(fm.spw_id, fm.I, Q=fm.Q, U=fm.U, V=fm.V, spix=fm.spix, origin=fm.origin,
                                       age=fm.age, queried_at=fm.queried_at)
            native_measurements.append(m)
                
        return native_measurements

    def __repr__(self):
        s = 'Flux calibration results for %s:\n' % os.path.basename(self.vis)
        for field, flux_densities in self.measurements.items():
            flux_by_spw = sorted(flux_densities, key=lambda f: f.spw_id)                    
            # rather complicated string format to display something like:
            # 0841+708 spw #0: I=3.2899 Jy; Q=0 Jy; U=0 Jy; V=0 Jy
            lines = ['\t{field} spw #{spw}: I={I}; Q={Q}; U={U}; V={V}; spix={spix}\n'.format(
                    field=field, spw=flux.spw_id, 
                    I=str(flux.I), Q=str(flux.Q), U=str(flux.U), V=str(flux.V), spix=str(flux.spix)) 
                    for flux in flux_by_spw]
            s += ''.join(lines)

        return s
