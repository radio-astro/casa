from __future__ import absolute_import

from . import logging

LOG = logging.get_logger(__name__)


# This class holds the project summary information

class ProjectSummary (object):
    def __init__ (self,
        proposal_code = '',
        proposal_title = 'undefined',
        piname = 'undefined',
        observatory = 'ALMA Joint Observatory',
        telescope = 'ALMA'):

        self.proposal_code = proposal_code
        self.proposal_title = proposal_title
        self.piname = piname
        self.observatory = observatory
        self.telescope = telescope

    def __iter__(self):
        return vars(self).iteritems()

# This class holds the ALMA project structure information.

class ProjectStructure (object):
    def __init__ (self,
        ous_entity_type = 'ObsProject',
        ous_entity_id = 'unknown',
        ous_part_id ='unknown',
        ous_title = 'undefined',
        ous_type = 'Member',
        ps_entity_type = 'ProjectStatus',
        ps_entity_id = 'unknown',
        ousstatus_type = 'OUSStatus',
        ousstatus_entity_id = 'unknown',
        ppr_type = 'SciPipeRequest',
        ppr_entity_id = 'unknown',
        ppr_file = ''):

        self.ous_entity_type = ous_entity_type
        self.ous_entity_id = ous_entity_id
        self.ous_part_id = ous_part_id
        self.ous_title = ous_title
        self.ous_type = ous_type
        self.ps_entity_type = ps_entity_type
        self.ps_entity_id = ps_entity_id
        self.ousstatus_type = ousstatus_type
        self.ousstatus_entity_id = ousstatus_entity_id
        self.ppr_type = ppr_type
        self.ppr_entity_id = ppr_entity_id
        self.ppr_file = ppr_file

    def __iter__(self):
        return vars(self).iteritems()

# This class holds the ALMA OUS performance parameters information.

class PerformanceParameters (object):

    def __init__ (self,
        desired_angular_resolution = '0.0arcsec',
        desired_largest_scale = '0.0arcsec',
        desired_spectral_resolution = '0.0MHz',
        desired_sensitivity = '0.0Jy',
        desired_dynamic_range = 1.0
        ):

        self.desired_angular_resolution = desired_angular_resolution
        self.desired_largest_scale = desired_largest_scale
        self.desired_spectral_resolution = desired_spectral_resolution
        self.desired_sensitivity = desired_sensitivity
        self.desired_dynamic_range = desired_dynamic_range

    def __iter__(self):
        return vars(self).iteritems()
