from __future__ import absolute_import

import types
import os
import csv

import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


def export_flux_from_fit_result (results, context, filename, fieldids_with_spix=None):
    """
    Export flux densities from a set of results to a CSV file.
    Optional merge in spix values for fields in mergfields.
    """
    if fieldids_with_spix is None:
        fieldids_with_spix = []

    if type(results) is not types.ListType:
        results = [results, ]

    abspath = os.path.join(context.output_dir, filename)

    columns = ['ms', 'field', 'spw', 'I', 'Q', 'U', 'V', 'spix', 'comment']

    with open(abspath, 'w+') as f:
        writer = csv.writer(f)
        writer.writerow(columns)

        counter = 0
        for setjy_result in results:
            ms_name = setjy_result.vis
            ms_basename = os.path.basename(ms_name)
            for field_id, measurements in setjy_result.measurements.items():
                for m in measurements:
                    if field_id in fieldids_with_spix:
                        mcontext = find_flux_measurement (context, ms_basename, field_id, m.spw_id)
                        if mcontext:
                            spix = mcontext.spix
                            LOG.info('Assuming spix %s for field %s and spw %s' % ('%0.3f' % spix, str(field_id), str(m.spw_id)))
                            m.spix = spix
                        else:
                            spix = m.spix
                    else:
                        spix = m.spix
                    (I, Q, U, V) = m.casa_flux_density
                    ms = context.observing_run.get_ms(ms_basename)
                    field = ms.get_fields(field_id)[0]
                    comment = field.name + ' ' + 'intent=' + ','.join(sorted(field.intents))
                    writer.writerow((ms_basename, field_id, m.spw_id,
                                         I, Q, U, V, '%0.3f' % float(spix), comment))
                    counter += 1

        LOG.info('Exported %s flux measurements to %s' % (counter, abspath))


def find_flux_measurement(context, ms_basename, field_id, spw_id):
    """
    Find a flux measurement in the context which matched field_id and spw_id
    """

    measurement = None

    # Find the measurement set
    ms = context.observing_run.get_ms(ms_basename)
    fields = ms.get_fields(field_id)
    if not fields:
        return measurement

    # Loop over the fluxes
    field = fields[0]
    for flux in field.flux_densities:
        if str(flux.spw_id) != spw_id:
            continue
        measurement = flux
        break

    return measurement
