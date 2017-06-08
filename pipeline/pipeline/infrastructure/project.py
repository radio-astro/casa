from __future__ import absolute_import
import abc

from . import logging

__all__ = [
    'LoggingModificationListener',
    'ModificationListener',
    'ModificationPublisher',
    'ProjectSummary',
    'ProjectStructure',
    'PerformanceParameters'
]

LOG = logging.get_logger(__name__)


# This class holds the project summary information

class ProjectSummary(object):
    def __init__(self, proposal_code='', proposal_title='undefined', piname='undefined',
                 observatory='ALMA Joint Observatory', telescope='ALMA'):

        self.proposal_code = proposal_code
        self.proposal_title = proposal_title
        self.piname = piname
        self.observatory = observatory
        self.telescope = telescope

    def __iter__(self):
        return vars(self).iteritems()

# This class holds the ALMA project structure information.

class ProjectStructure(object):
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
        ppr_file = '',
        recipe_name = 'undefined'):

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
        self.recipe_name = recipe_name

    def __iter__(self):
        return vars(self).iteritems()

# This class holds the ALMA OUS performance parameters information.

class PerformanceParameters(object):

    def __init__(self, desired_angular_resolution='0.0arcsec', min_angular_resolution='0.0arcsec',
        max_angular_resolution = '0.0arcsec',
        #desired_largest_scale = '0.0arcsec',
        #desired_spectral_resolution = '0.0MHz',
        #desired_sensitivity = '0.0Jy',
        #desired_dynamic_range = 1.0,

        # Note: Remove frequency and bandwidth settings?
        representative_source = '',
        representative_spwid = '',
        representative_frequency = '0.0GHz',
        representative_bandwidth = '0.0MHz',

        max_cube_size = -1.0,
        max_product_size = -1.0

        ):

        # QA goals
        self.desired_angular_resolution = desired_angular_resolution
        self.min_angular_resolution = min_angular_resolution
        self.max_angular_resolution = max_angular_resolution
        #self.desired_largest_scale = desired_largest_scale
        #self.desired_spectral_resolution = desired_spectral_resolution
        #self.desired_sensitivity = desired_sensitivity
        #self.desired_dynamic_range = desired_dynamic_range

        # Representative source parameters
        # Note: Remove frequency and bandwidth settings?
        self.representative_spwid = representative_spwid
        self.representative_source = representative_source
        self.representative_frequency = representative_frequency
        self.representative_bandwidth = representative_bandwidth

        # Product size parameters
        self.max_cube_size = max_cube_size
        self.max_product_size = max_product_size

    def __iter__(self):
        return vars(self).iteritems()


def get_state(o):
    # create a new vanilla instance so we can compare against the defaults
    defaults = o.__class__()
    modified = {k: v for k, v in o.__dict__.iteritems() if v != getattr(defaults, k)}
    cls_name = o.__class__.__name__
    return [(cls_name, k, v) for k, v in modified.iteritems()]


class ModificationListener(object):
    """
    Interface for listener classes that want to be notified when an object
    property changes.

    Notification will be received *after* the change has already occurred. It
    is not possible to veto a change using this class.

    Note: this function require two functions to be implemented (on_setattr
    and  on_delattr) rather than having one function switching on an
    enumeration to emphasise that this class is deliberately tied into the
    Python data model.
    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def on_delattr(self, modified_obj, attr):
        raise NotImplemented

    @abc.abstractmethod
    def on_setattr(self, modified_obj, attr, val):
        raise NotImplemented


class LoggingModificationListener(ModificationListener):
    """
    ModificationListener that logs INFO messages when the target is modified.
    """

    def on_delattr(self, modified_obj, attr):
        LOG.info('Deleting property: {!s}.{!s}'.format(modified_obj.__class__.__name__, attr))

    def on_setattr(self, modified_obj, attr, val):
        LOG.info('Modifying property: {!s}.{!s} = {!r}'.format(modified_obj.__class__.__name__, attr, val))


class ModificationPublisher(object):
    """
    Base class that publishes an event to registered listeners when public
    properties of an instance of this class are set or deleted.

    Notifications will only be published when the property points to a new
    value. Events for in-situ modifications (e.g., adding a value to an
    existing list) will not be received.

    Notifications will be sent *after* the change has already occurred. It
    is not possible to veto a change using this implementation.

    Background: see CAS-9497, which wants to log the PPR XML directives that
    would otherwise be missing from the casa_pipescript.py
    """

    def __init__(self):
        self._listeners = set()

    # def __iter__(self):
    #     public = {k: v for k, v in vars(self).iteritems() if not k.startswith('_')}
    #     return public.iteritems()

    def __setattr__(self, name, value):
        LOG.trace('Setting {!s}.{!s} = {!r}'.format(self.__class__.__name__, name, value))
        super(ModificationPublisher, self).__setattr__(name, value)

        # Only log changes to public properties
        if not name.startswith('_'):
            for listener in self._listeners:
                listener.on_setattr(self, name, value)

    def __delattr__(self, name):
        super(ModificationPublisher, self).__delattr__(name)

        # Only log changes to public properties
        if not name.startswith('_'):
            for listener in self._listeners:
                listener.on_delattr(self, name)

    def add_listener(self, listener):
        self._listeners.add(listener)

    def remove_listener(self, listener):
        self._listeners.remove(listener)

    def as_dict(o):
        state = o.__dict__.copy()
        del state['_listeners']
        return state

    def __getstate__(self):
        # do not persist listeners across pickles. This empty set will be
        # restored on deserialisation so we don't need to override
        # __setstate__ too.
        state = self.__dict__.copy()
        state['_listeners'] = set()
        return state
