"""
Created on 5 Sep 2014

@author: sjw
"""
import collections
import operator
import os
import shutil

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
from pipeline.domain.measures import FrequencyUnits

LOG = logging.get_logger(__name__)


class T2_4MDetailsImportDataRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='importdata.mako', 
                 description='Register measurement sets with the pipeline', 
                 always_rerender=False):
        super(T2_4MDetailsImportDataRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, result):
        weblog_dir = os.path.join(pipeline_context.report_dir,
                                  'stage%s' % result.stage_number)

        setjy_results = []
        for r in result:
            setjy_results.extend(r.setjy_results)

        measurements = []        
        for r in setjy_results:
            measurements.extend(r.measurements)

        num_mses = reduce(operator.add, [len(r.mses) for r in result])

        flux_table_rows = make_flux_table(pipeline_context, setjy_results)

        repsource_table_rows, repsource_name_is_none = make_repsource_table(pipeline_context, result)
        repsource_defined = any('N/A' not in td for tr in repsource_table_rows for td in tr[1:])

        fluxcsv_files = {}
        for r in result:

            # copy flux.csv file across to weblog directory
            fluxcsv_filename = 'flux.csv'
            ms = pipeline_context.observing_run.get_ms(os.path.basename(r.inputs['vis'].rstrip('/')))
            if os.path.exists(fluxcsv_filename):
                LOG.trace('Copying %s to %s' % (fluxcsv_filename, weblog_dir))
                shutil.copy(fluxcsv_filename, weblog_dir)

            fluxcsv_files[ms.basename] = os.path.join('stage%s' % result.stage_number, fluxcsv_filename)

        mako_context.update({
            'flux_imported': True if measurements else False,
            'flux_table_rows': flux_table_rows,
            'repsource_defined': repsource_defined,
            'repsource_name_is_none': repsource_name_is_none,
            'repsource_table_rows': repsource_table_rows,
            'num_mses': num_mses,
            'fluxcsv_files': fluxcsv_files,
            'weblog_dir': weblog_dir
        })


FluxTR = collections.namedtuple('FluxTR', 'vis field spw i q u v spix ageNMP')


def make_flux_table(context, results):
    # will hold all the flux stat table rows for the results
    rows = []

    for single_result in results:
        ms_for_result = context.observing_run.get_ms(single_result.vis)
        vis_cell = os.path.basename(single_result.vis)

        # measurements will be empty if fluxscale derivation failed
        if len(single_result.measurements) is 0:
            continue
            
        for field_arg, measurements in single_result.measurements.items():
            field = ms_for_result.get_fields(field_arg)[0]
            field_cell = '%s (#%s)' % (field.name, field.id)

            for measurement in sorted(measurements, key=lambda m: int(m.spw_id)):
                fluxes = collections.defaultdict(lambda: 'N/A')
                for stokes in ['I', 'Q', 'U', 'V']:
                    try:                        
                        flux = getattr(measurement, stokes)
                        fluxes[stokes] = '%s' % flux
                    except:
                        pass

                if measurement.age:
                    age = measurement.age
                else:
                    age = 'N/A'

                tr = FluxTR(vis_cell, field_cell, measurement.spw_id,
                            fluxes['I'], fluxes['Q'], fluxes['U'], fluxes['V'],
                            measurement.spix, age)
                rows.append(tr)

    return utils.merge_td_columns(rows)


RepsourceTR = collections.namedtuple('RepsourceTR', 'vis source rfreq rbwidth spwid bwidth')


def make_repsource_table(context, results):
    # will hold all the representative source table rows for the results

    qa = casatools.quanta

    rows = []

    repsource_name_is_none = False
    for r in results:
        for ms in r.mses:

            # Skip if not ALMA data
            #    What about single dish
            if ms.antenna_array.name != 'ALMA':
                continue

            # ASDM
            vis = ms.basename

            # If either the representative frequency or bandwidth is undefined then
            # the representatve target is undefined
            representative_target = ms.representative_target
            rep_target_defined = not(None in representative_target or 'None' in representative_target or 'none' in representative_target)
            repsource_name_is_none = representative_target[0] == 'none'
            if not rep_target_defined:
                rows.append(RepsourceTR(vis, 'N/A', 'N/A', 'N/A', 'N/A', 'N/A'))
                continue

            # Is the representative source in the context or not
            if not context.project_performance_parameters.representative_source:
                source_name = None
            else:
                source_name = context.project_performance_parameters.representative_source

            # Is the representative spw in the context or not
            if not context.project_performance_parameters.representative_spwid:
                source_spwid = None
            else:
                source_spwid = context.project_performance_parameters.representative_spwid

            # Determine the representative source name and spwid for the ms
            repsource_name, repsource_spwid = ms.get_representative_source_spw(source_name=source_name,
                                                                               source_spwid=source_spwid)

            # Populate the table rows
            # No source
            if not repsource_name: 
                if not ms.representative_target[0]:
                    tr = RepsourceTR(vis, 'Unknown', 'Unknown', 'Unknown', 'Unknown', 'Unknown')
                else:
                    tr = RepsourceTR(vis, ms.representative_target[0], 'Unknown', 'Unknown', 'Unknown', 'Unknown')
                rows.append(tr)
                continue

            # No spwid
            if not repsource_spwid:
                tr = RepsourceTR(vis, repsource_name, qa.tos(ms.representative_target[1], 5),
                                 qa.tos(ms.representative_target[2], 5), 'Unknown', 'Unknown')
                rows.append(tr)
                continue

            # Get center frequency and channel width for representative spw id
            repsource_spw = ms.get_spectral_window(repsource_spwid)
            repsource_chanwidth = qa.quantity(
                float(repsource_spw.channels[0].getWidth().to_units(FrequencyUnits.MEGAHERTZ)), 'MHz')

            tr = RepsourceTR(vis, repsource_name, qa.tos(ms.representative_target[1], 5),
                             qa.tos(ms.representative_target[2], 5), str(repsource_spwid),
                             qa.tos(repsource_chanwidth, 5))
            rows.append(tr)

    return utils.merge_td_columns(rows), repsource_name_is_none
