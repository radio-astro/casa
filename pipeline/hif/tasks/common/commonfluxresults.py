from __future__ import absolute_import
import collections
import os

import pipeline.infrastructure.api as api
import pipeline.domain.fluxmeasurement as fluxmeasurement
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)


class FluxCalibrationResults(api.Results):
    """
    TODO: refactor this results structure. We're falling into the dictionaries
    of dictionaries trap again.. 
    """
    def __init__(self, vis=None, fields={}, fields_setjy={}):
        self.results = collections.defaultdict(dict)
        self.setjy_settings = collections.defaultdict(dict)
        if vis:
            self.results[vis] = fields
            self.setjy_settings[vis] = fields_setjy

    def set_result(self, vis, fields):
        if vis in self.results:
            current = self.results[vis]
            for field, measurements in fields.items():
                current[field] = measurements
        else:
            self.results[vis] = fields

    def set_setjy_result(self, vis, fields):
        if vis in self.setjy_settings:
            current = self.setjy_settings[vis]
            for field, measurements in fields.items():
                current[field] = measurements
        else:
            self.setjy_settings[vis] = fields

    def merge_with_context(self, context, replace=False):
        for vis, fields in self.results.items():
            ms = context.observing_run.get_ms(vis)
            for field_name, foreign_measurements in fields.items():
                # the measurements may refer to spectral windows in other
                # measurement sets, so we replace them with spectral 
                # windows from this measurement set with the same ID
                measurements = []
                for m in foreign_measurements:
                    ms_spw = ms.get_spectral_window(m.spw.id)
                    clone = fluxmeasurement.FluxMeasurement(ms_spw,
                      I=m.I, Q=m.Q, U=m.U, V=m.V)                
                    measurements.append(clone)
                
                spw_ids = sorted([m.spw.id for m in measurements])

                # one field name may resolve to multiple fields
                fields = ms.get_fields(task_arg=field_name)
                for field in fields:
                    # remove any existing measurements in these spws
                    existing = [m for m in field.flux_densities 
                                if m.spw.id in spw_ids]
                    if existing:
                        map(field.flux_densities.remove, existing)
    
                    LOG.info('Setting flux density measurements for {field} '
                             'in {vis}:\n\t{measurements}'.format(
                        field=field.identifier, 
                        vis=ms.name,
                        measurements='\n\t'.join([str(m) for m in measurements])))
                    field.flux_densities.update(measurements)

    def __repr__(self):
        s = 'Flux calibration results:\n'
        for vis, fields in self.results.items():
            for field, field_fluxes in fields.items():
                flux_by_spw = sorted(field_fluxes, key=lambda g: g.spw.id)                    
                # rather complicated string format to display something like:
                # 0841+708 spw #0: I=3.2899 Jy; Q=0 Jy; U=0 Jy; V=0 Jy
                lines = ['{vis} {field} spw #{spw}: '
                         'I={I}; Q={Q}; U={U}; V={V}\n'.format(
                    vis=os.path.basename(vis), field=field, spw=flux.spw.id, 
                    I=str(flux.I), Q=str(flux.Q), U=str(flux.U), 
                    V=str(flux.V)) for flux in flux_by_spw]
                for line in lines:
                    s += line

        return s

    def merge_result(self, toAdd):
        for vis, fields in toAdd.results.items():
            for field, field_fluxes in fields.items():
                self.results[vis][field] = field_fluxes[:]

