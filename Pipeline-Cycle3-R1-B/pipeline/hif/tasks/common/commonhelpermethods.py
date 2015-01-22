from __future__ import absolute_import
import pipeline.infrastructure as infrastructure
LOG = infrastructure.get_logger(__name__)

def get_antenna_names(ms):
    """Get antenna names.
    """
    antenna_ids = [antenna.id for antenna in ms.antennas]
    antenna_ids.sort()
    antenna_name = {}
    for antenna_id in antenna_ids:
        antenna_name[antenna_id] = [antenna.name for antenna in ms.antennas
          if antenna.id==antenna_id][0]

    return antenna_name, antenna_ids

def get_corr_products(ms, spwid):
    """Get names of corr products stored in ms.
    """
    # get names of correlation products
    datadescs = [dd for dd in ms.data_descriptions if dd.spw.id==spwid]
    polarization = ms.polarizations[datadescs[0].pol_id]
    corr_type = polarization.corr_type_string

    return corr_type

