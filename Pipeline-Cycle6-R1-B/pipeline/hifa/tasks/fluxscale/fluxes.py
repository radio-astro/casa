from __future__ import absolute_import

import csv
import os
import types

import pipeline.infrastructure as infrastructure
from ..importdata.dbfluxes import ORIGIN_DB
from ....h.tasks.importdata.fluxes import ORIGIN_XML

LOG = infrastructure.get_logger(__name__)


def export_flux_from_fit_result(results, context, filename, fieldids_with_spix=None):
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
            ms = context.observing_run.get_ms(setjy_result.vis)

            for field_id, measurements in setjy_result.measurements.iteritems():
                fields = ms.get_fields(field_id)
                assert(len(fields) == 1)
                field = fields[0]

                for m in measurements:
                    if field_id in fieldids_with_spix:
                        catalogue_measurements = [fd for fd in field.flux_densities
                                                  if int(m.spw_id) == fd.spw_id
                                                  and fd.origin in (ORIGIN_DB, ORIGIN_XML)]
                        if catalogue_measurements:
                            assert(len(catalogue_measurements) == 1)
                            spix = catalogue_measurements[0].spix
                            LOG.info('Assuming spix {:.3f} for field {} ({}) spw {}'.format(spix, field.name, field.id, m.spw_id))
                            m.spix = spix
                    (I, Q, U, V) = m.casa_flux_density
                    ms = context.observing_run.get_ms(ms.basename)
                    comment = '# {} intent={}'.format(field.clean_name, ','.join(sorted(field.intents)))
                    writer.writerow((ms.basename, field_id, m.spw_id, I, Q, U, V, '%0.3f' % float(m.spix), comment))
                    counter += 1

        LOG.info('Exported %s flux measurements to %s' % (counter, abspath))
