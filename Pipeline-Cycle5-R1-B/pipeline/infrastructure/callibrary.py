from __future__ import absolute_import
import collections
import copy
import datetime
import functools
import itertools
import operator
import os
import string
import types
import uuid
import weakref

import pipeline.extern
import intervaltree

from . import logging
from . import utils

from . import casatools

LOG = logging.get_logger(__name__)


CalToArgs = collections.namedtuple('CalToArgs',
                                   ['vis', 'spw', 'field', 'intent', 'antenna'])

# struct used to link calapplication to the task and inputs that created it
CalAppOrigin = collections.namedtuple('CalAppOrigin', ['task', 'inputs'])

# observations before this date are considered Cycle 0 observations
CYCLE_0_END_DATE = datetime.datetime(2013, 1, 21)


class CalApplication(object):
    """
    CalApplication maps calibration tables and their application arguments to
    a target data selection, encapsulated as |CalFrom| and |CalTo| objects 
    respectively.

    .. py:attribute:: calto

        the |CalTo| representing the data selection to which the calibration
        should apply.

    .. py:attribute:: calfrom

        the |CalFrom| representing the calibration and application parameters

    .. py:attribute:: origin

        the |CalAppOrigin| marking how this calibration was created

.. |CalTo| replace:: :class:`CalTo`
.. |CalFrom| replace:: :class:`CalFrom`
.. |CalAppOrigin| replace:: :class:`CalAppOrigin`
    """

    def __init__(self, calto, calfrom, origin=None):
        self.calto = calto

        if not isinstance(calfrom, list):
            calfrom = [calfrom]
        self.calfrom = calfrom

        if not isinstance(origin, list):
            origin = [origin]
        self.origin = origin

    @staticmethod
    def from_export(s):
        """
        Unmarshal a CalApplication from a string.

        :rtype: the unmarshalled :class:`CalApplication` object
        """
        d = eval(string.replace(s, 'applycal(', 'dict('))
        calto = CalTo(vis=d['vis'], field=d['field'], spw=d['spw'], 
                      antenna=d['antenna'], intent=d['intent'])
        
        # wrap these values in a list if they are single valued, 
        # eg. 'm31' -> ['m31']
        for key in ('gainfield', 'gaintable', 'interp'):
            if type(d[key]) is types.StringType:
                d[key] = [d[key]]
        for key in ('calwt',):
            if type(d[key]) is types.BooleanType:
                d[key] = [d[key]]

        # do the same for spwmap. A bit more complicated, as a single valued
        # spwmap is a list of integers, or may not have any values at all.                
        try:            
            if type(d['spwmap'][0]) is not types.ListType:
                d['spwmap'] = [d['spwmap']]
        except IndexError, _:
            d['spwmap'] = [d['spwmap']]

        zipped = zip(d['gaintable'], d['gainfield'], d['interp'], d['spwmap'],
                     d['calwt'])

        calfroms = []
        for (gaintable, gainfield, interp, spwmap, calwt) in zipped:
            with casatools.TableReader(gaintable) as caltable:
                viscal = caltable.getkeyword('VisCal')
            
            caltype = CalFrom.get_caltype_for_viscal(viscal) 
            calfrom = CalFrom(gaintable, gainfield=gainfield, interp=interp, 
                              spwmap=spwmap, calwt=calwt, caltype=caltype)
            LOG.trace('Marking caltable \'%s\' as caltype \'%s\''
                      '' % (gaintable, calfrom.caltype))

            calfroms.append(calfrom)
        
        return CalApplication(calto, calfroms)

    def as_applycal(self):
        """
        Get a representation of this object as a CASA applycal call.

        :rtype: string
        """
        args = {
            'vis': self.vis,
            'field': self.field,
            'intent': self.intent,
            'spw': self.spw,
            'antenna': self.antenna,
            'gaintable': self.gaintable,
            'gainfield': self.gainfield,
            'spwmap': self.spwmap,
            'interp': self.interp,
            'calwt': self.calwt
        }
        
        for key in ('gaintable', 'gainfield', 'spwmap', 'interp', 'calwt'):
            if type(args[key]) is types.StringType:
                args[key] = '\'%s\'' % args[key]
        
        return ('applycal(vis=\'{vis}\', field=\'{field}\', '
                'intent=\'{intent}\', spw=\'{spw}\', antenna=\'{antenna}\', '
                'gaintable={gaintable}, gainfield={gainfield}, '
                'spwmap={spwmap}, interp={interp}, calwt={calwt})'
                ''.format(**args))

    @property
    def antenna(self):
        """
        The antennas to which the calibrations apply.

        :rtype: string
        """ 
        return self.calto.antenna

    @property
    def calwt(self):
        """
        The calwt parameters to be used when applying these calibrations.
        
        :rtype: a scalar string if representing 1 calibration, otherwise a
                list of strings
        """
        l = [cf.calwt for cf in self.calfrom]
        return l[0] if len(l) is 1 else l
    
    def exists(self):
        """
        Test whether all calibration tables referred to by this application exist.
        
        :rtype: boolean
        """ 
        for cf in self.calfrom: 
            if not os.path.exists(cf.gaintable):
                return False
        return True

    @property
    def field(self):
        """
        The fields to which the calibrations apply.

        :rtype: string
        """ 
        return self.calto.field

    @property
    def gainfield(self):
        """
        The gainfield parameters to be used when applying these calibrations.
        
        :rtype: a scalar string if representing 1 calibration, otherwise a
                list of strings
        """
        l = [cf.gainfield for cf in self.calfrom]
        return l[0] if len(l) is 1 else l

    @property
    def gaintable(self):
        """
        The gaintable parameters to be used when applying these calibrations.
        
        :rtype: a scalar string if representing 1 calibration, otherwise a
                list of strings
        """
        l = [cf.gaintable for cf in self.calfrom]
        return l[0] if len(l) is 1 else l
    
    @property
    def intent(self):
        """
        The observing intents to which the calibrations apply.

        :rtype: string
        """ 
        return self.calto.intent

    @property
    def interp(self):
        """
        The interp parameters to be used when applying these calibrations.
        
        :rtype: a scalar string if representing 1 calibration, otherwise a
                list of strings
        """
        l = [cf.interp for cf in self.calfrom]
        return l[0] if len(l) is 1 else l
        
    @property
    def spw(self):
        """
        The spectral windows to which the calibrations apply.

        :rtype: string
        """ 
        return self.calto.spw
    
    @property
    def spwmap(self):
        """
        The spwmap parameters to be used when applying these calibrations.
        
        :rtype: a scalar string if representing 1 calibration, otherwise a
                list of strings
        """
        # convert tuples back into lists for the CASA argument
        l = [list(cf.spwmap) for cf in self.calfrom]
        return l[0] if len(l) is 1 else l 

    @property
    def vis(self):
        """
        The name of the measurement set to which the calibrations apply.

        :rtype: string
        """ 
        return self.calto.vis

    def __str__(self):
        return self.as_applycal()
    
    def __repr__(self):
        return 'CalApplication(%s, %s)' % (self.calto, self.calfrom)


class CalTo(object):
    """
    CalTo represents a target data selection to which a calibration can be 
    applied.
    """

    __slots__ = ('_antenna', '_intent', '_field', '_spw', '_vis')
      
    def __getstate__(self):
        return self._antenna, self._intent, self._field, self._spw, self._vis
 
    def __setstate__(self, state):
        self._antenna, self._intent, self._field, self._spw, self._vis = state

    def __init__(self, vis=None, field='', spw='', antenna='', intent=''):
        self.vis = vis
        self.field = field
        self.spw = spw
        self.antenna = antenna
        self.intent = intent

    @property
    def antenna(self):
        return self._antenna
    
    @antenna.setter
    def antenna(self, value):
        if value is None:
            value = ''
        self._antenna = utils.find_ranges(str(value))

    @property
    def field(self):
        return self._field
    
    @field.setter
    def field(self, value):
        if value is None:
            value = ''
        self._field = str(value)

    @property
    def intent(self):
        return self._intent
    
    @intent.setter
    def intent(self, value):
        if value is None:
            value = ''
        self._intent = str(value)

    @property
    def spw(self):
        return self._spw
    
    @spw.setter
    def spw(self, value):
        if value is None:
            value = ''
        self._spw = utils.find_ranges(str(value))

    @property
    def vis(self):
        return self._vis
    
    @vis.setter
    def vis(self, value=None):
        self._vis = str(value)

    def __repr__(self):
        return ('CalTo(vis=\'%s\', field=\'%s\', spw=\'%s\', antenna=\'%s\','
                'intent=\'%s\')' % (self.vis, self.field, self.spw, self.antenna, 
                                    self.intent))


class CalFrom(object):
    """
    CalFrom represents a calibration table and the CASA arguments that should
    be used when applying that calibration table.

    .. py:attribute:: CALTYPES

        an enumeration of calibration table types identified by this code. 
        
    .. py:attribute:: CALTYPE_TO_VISCAL
    
        mapping of calibration type to caltable identifier as store in the table
        header

    .. py:attribute:: VISCAL
    
        mapping of calibration table header information to a description of
        that table type
    """

    CALTYPES = {
        'unknown': 0,
        'gaincal': 1,
        'bandpass': 2,
        'tsys': 3,
        'wvr': 4,
        'polarization': 5,
        'antpos': 6,
        'gc': 7,
        'opac': 8,
        'rq': 9,
        'swpow': 10,
        'finalcal': 11,
        'uvcont': 12,
        'amp': 13,
        'ps': 14,
        'otfraster': 15,
        'tecim': 16,
        'kcross': 17,
        'otf': 18,
    }

    CALTYPE_TO_VISCAL = {
        'gaincal': ('G JONES', 'GSPLINE', 'T JONES'),
        'bandpass': ('B JONES', 'BPOLY'),
        'tsys': ('B TSYS',),
        'antpos': ('KANTPOS JONES',),
        'uvcont': ('A MUELLER',),
        #'amp': ('G JONES',),
        'ps': ('SDSKY_PS',),
        'otfraster': ('SDSKY_RASTER',),
        'otf': ('SDSKY_OTF',),
    }

    VISCAL = {
        'P JONES': 'P Jones (parallactic angle phase)',
        'T JONES': 'T Jones (polarization-independent troposphere)',
        'TF JONES': 'Tf Jones (frequency-dependent atmospheric complex gain)',
        'G JONES': 'G Jones (electronic Gain)',
        'B JONES': 'B Jones (bandpass)',
        'DGEN JONES': 'Dgen Jones (instrumental polarization)',
        'DFGEN JONES': 'Dfgen Jones (frequency-dependent instrumental polarization)',
        'D JONES': 'D Jones (instrumental polarization)',
        'DF JONES': 'Df Jones (frequency-dependent instrumental polarization)',
        'J JONES': 'J Jones (generic polarized gain)',
        'M MUELLER': 'M Mueller (baseline-based)',
        'MF MUELLER': 'Mf Mueller (closure bandpass)',
        'TOPAC': 'TOpac (Opacity corrections in amplitude)',
        'TFOPAC': 'TfOpac (frequency-dependent opacity)',
        'X MUELLER': 'X Mueller (baseline-based)',
        'X JONES': 'X Jones (antenna-based)',
        'XF JONES': 'Xf Jones (antenna-based)',
        'GLINXPH JONES': 'GlinXph Jones (X-Y phase)',
        'B TSYS': 'B TSYS (freq-dep Tsys)',
        'BPOLY': 'B Jones Poly (bandpass)',
        'GSPLINE': 'G Jones SPLINE (elec. gain)',
        'KANTPOS JONES': 'KAntPos Jones (antenna position errors)',
        'A MUELLER': 'A Mueller (baseline-based)',
    }

    # Hundreds of thousands of CalFroms can be created and stored in a context.
    # To save memory, CalFrom uses a Flyweight pattern, caching objects in 
    # _CalFromPool and returning a shared immutable instance for CalFroms
    # constructed with the same arguments.
    _CalFromPool = weakref.WeakValueDictionary()

    @staticmethod
    def _calc_hash(gaintable, gainfield, interp, spwmap, calwt):
        """
        Generate a hash code unique to the given arguments.
        
        :rtype: integer
        """ 
        result = 17
        result = 37*result + hash(gaintable)
        result = 37*result + hash(gainfield)
        result = 37*result + hash(interp)
        result = 37*result + hash(spwmap)
        result = 37*result + hash(calwt)
        return result
    
    def __new__(cls, gaintable=None, gainfield='', interp='linear,linear', 
                spwmap=None, caltype='unknown', calwt=True):
        if spwmap is None:
            spwmap = []
        
        if gaintable is None:
            raise ValueError, 'gaintable must be specified. Got None'
        
        if type(gainfield) is not types.StringType:
            raise ValueError, 'gainfield must be a string. Got %s' % str(gainfield)

        if type(interp) is not types.StringType:
            raise ValueError, 'interp must be a string. Got %s' % str(interp)

        if type(spwmap) is types.TupleType:
            spwmap = [spw for spw in spwmap]

        if not isinstance(spwmap, list):
            raise ValueError, 'spwmap must be a list. Got %s' % str(spwmap)
        # Flyweight instances should be immutable, so convert spwmap to a
        # tuple. This also makes spwmap hashable for our hash function.
        spwmap = tuple([o for o in spwmap])
                
        caltype = string.lower(caltype)
        assert caltype in CalFrom.CALTYPES

        arg_hash = CalFrom._calc_hash(gaintable, gainfield, interp, spwmap, 
                                      calwt)
        
        obj = CalFrom._CalFromPool.get(arg_hash, None)
        if not obj:
            LOG.trace('Creating new CalFrom(gaintable=\'%s\', '
                      'gainfield=\'%s\', interp=\'%s\', spwmap=%s, '
                      'caltype=\'%s\', calwt=%s)' % 
                (gaintable, gainfield, interp, spwmap, caltype, calwt))
            obj = object.__new__(cls)
            obj.__gaintable = gaintable
            obj.__gainfield = gainfield
            obj.__interp = interp
            obj.__spwmap = spwmap
            obj.__caltype = caltype
            obj.__calwt = calwt

            LOG.debug('Adding new CalFrom to pool: %s' % obj)
            CalFrom._CalFromPool[arg_hash] = obj
            LOG.trace('New pool contents: %s' % CalFrom._CalFromPool.items())
        else:
            LOG.trace('Reusing existing CalFrom(gaintable=\'%s\', '
                      'gainfield=\'%s\', interp=\'%s\', spwmap=\'%s\', '
                      'caltype=\'%s\', calwt=%s)' % 
                (gaintable, gainfield, interp, spwmap, caltype, calwt))
                        
        return obj

    __slots__ = ('__caltype', '__calwt', '__gainfield', '__gaintable', 
                 '__interp', '__spwmap', '__weakref__')
          
    def __getstate__(self):
        return (self.__caltype, self.__calwt, self.__gainfield, 
                self.__gaintable, self.__interp, self.__spwmap)
     
    def __setstate__(self, state):
        # a misguided attempt to clear stale CalFroms when loading from a
        # pickle. I don't think this should be done here.
#         # prevent exception with pickle format #1 by calling hash on properties
#         # rather than the object
#         (_, calwt, gainfield, gaintable, interp, spwmap) = state
#         old_hash = CalFrom._calc_hash(gaintable, gainfield, interp, spwmap, calwt)
#         if old_hash in CalFrom._CalFromPool:
#             del CalFrom._CalFromPool[old_hash]

        (self.__caltype, self.__calwt, self.__gainfield, self.__gaintable, 
         self.__interp, self.__spwmap) = state

    def __getnewargs__(self):
        return (self.gaintable, self.gainfield, self.interp, self.spwmap, 
                self.caltype, self.calwt)
    
    def __init__(self, *args, **kw):
        pass

    @property
    def caltype(self):
        return self.__caltype

    @property
    def calwt(self):
        return self.__calwt
    
    @property
    def gainfield(self):
        return self.__gainfield
    
    @property
    def gaintable(self):
        return self.__gaintable
    
    @staticmethod
    def get_caltype_for_viscal(viscal):
        s = string.upper(viscal)
        for caltype, viscals in CalFrom.CALTYPE_TO_VISCAL.items():
            if s in viscals:
                return caltype
        return 'unknown'

    @property
    def interp(self):
        return self.__interp
    
    @property
    def spwmap(self):
        return self.__spwmap
    
#     def __eq__(self, other):
#         return (self.gaintable == other.gaintable and
#                 self.gainfield == other.gainfield and
#                 self.interp    == other.interp    and
#                 self.spwmap    == other.spwmap    and
#                 self.calwt     == other.calwt)
    
    def __hash__(self):
        return CalFrom._calc_hash(self.gaintable, self.gainfield, self.interp,
                                  self.spwmap, self.calwt)

    def __repr__(self):
        return ('CalFrom(\'%s\', gainfield=\'%s\', interp=\'%s\', spwmap=%s, '
                'caltype=\'%s\', calwt=%s)' % 
                (self.gaintable, self.gainfield, self.interp, self.spwmap, 
                 self.caltype, self.calwt))


class CalToIdAdapter(object):
    def __init__(self, context, calto):
        self._context = context
        self._calto = calto

    @property
    def antenna(self):
        return [a.id for a in self.ms.get_antenna(self._calto.antenna)]

    @property
    def field(self):
        fields = [f for f in self.ms.get_fields(task_arg=self._calto.field)]
        # if the field names are unique, we can return field names. Otherwise,
        # we fall back to field IDs.
        all_field_names = [f.name for f in self.ms.get_fields()]
        ### Activate the following line for NRO ###
#         return [f.id for f in fields]
        if len(set(all_field_names)) == len(all_field_names):
            return [f.name for f in fields]
        else:
            return [f.id for f in fields]

    @property
    def intent(self):
        # return the intents present in the CalTo
        return self._calto.intent

    def get_field_intents(self, field_id, spw_id):
        field = self._get_field(field_id)
        field_intents = field.intents

        spw = self._get_spw(spw_id)
        spw_intents = spw.intents
        
        user_intents = frozenset(self._calto.intent.split(','))
        if self._calto.intent == '':
            user_intents = field.intents

        return user_intents & field_intents & spw_intents

    @property
    def ms(self):
        return self._context.observing_run.get_ms(self._calto.vis)

    @property
    def spw(self):
        return [spw.id for spw in self.ms.get_spectral_windows(
                self._calto.spw, science_windows_only=False)]

    def _get_field(self, field_id):
        fields = self.ms.get_fields(task_arg=field_id)
        if len(fields) != 1:
            msg = 'Illegal field ID \'%s\' for vis \'%s\'' % (field_id, 
                                                              self._calto.vis)
            LOG.error(msg)
            raise ValueError, msg        
        return fields[0]

    def _get_spw(self, spw_id):
        spws = self.ms.get_spectral_windows(spw_id, 
                                            science_windows_only=False)
        if len(spws) != 1:
            msg = 'Illegal spw ID \'%s\' for vis \'%s\'' % (spw_id, 
                                                            self._calto.vis)
            LOG.error(msg)
            raise ValueError, msg        
        return spws[0]

    def __repr__(self):
        return ('CalToIdAdapter(ms=\'%s\', field=\'%s\', intent=\'%s\', ' 
                'spw=%s, antenna=%s)' % (self.ms.name, self.field,
                                         self.intent, self.spw, self.antenna))


# CalState extends defaultdict. For defaultdicts to be pickleable, their
# default factories must be defined at the module level.
def _antenna_dim(): return []
def _intent_dim(): return collections.defaultdict(_antenna_dim)
def _field_dim(): return collections.defaultdict(_intent_dim)
def _spw_dim(): return collections.defaultdict(_field_dim)
def _ms_dim(): return collections.defaultdict(_spw_dim)


class DictCalState(collections.defaultdict):
    """
    DictCalState is a data structure used to map calibrations for all data
    registered with the pipeline.
    
    It is implemented as a multi-dimensional array indexed by data selection
    parameters (ms, spw, field, intent, antenna), with the end value being a 
    list of CalFroms, representing the calibrations to be applied to that data
    selection.
    """

    def __init__(self, default_factory=_ms_dim):
        super(DictCalState, self).__init__(default_factory)
        self._removed = set()

    def __reduce__(self):  # optional, for pickle support
        super_state = super(DictCalState, self).__reduce__()
        return self.__class__, super_state[1], self._removed, super_state[3], super_state[4]

    def __setstate__(self, state):
        self._removed = state

    def global_remove(self, calfrom):
        """
        Mark a CalFrom as being removed from the calibration state. Rather than
        iterating through the registered calibrations, this adds the CalFrom to
        a set of object to be ignored. When the calibrations are subsequently
        inspected, CalFroms marked as removed will be bypassed.

        :param calfrom: the CalFrom to remove
        :return:
        """
        self._removed.add(calfrom)

    def global_reactivate(self, calfroms):
        """
        Reactivate a CalFrom that was marked as ignored through a call to
        global_remove.

        This will reactivate the CalFrom entry, making it appear at whatever
        index in the CalApplications that it was originally registered, e.g.
        if a CalFrom was 'deleted' via a call to global_remove and 3 more
        CalFroms were added to the CalState, when the CalFrom is reactivated
        it will appear in the original position - that is, before the 3
        subsequent CalFroms, rather than appearing at the end of the list.

        :param calfroms: the CalFroms to reactivate
        :type calfroms: a set of CalFrom objects
        :return: None
        """
        LOG.trace('Globally reactivating %s CalFroms: %s',
                  len(calfroms), calfroms)
        self._removed -= calfroms

    def get_caltable(self, caltypes=None):
        """
        Get the names of all caltables registered with this CalState. 
        
        If an optional caltypes argument is given, only caltables of the
        requested type will be returned.

        :param caltypes: Caltypes should be one or/a list of table
        types known in CalFrom.CALTYPES.
            
        :rtype: set of strings
        """ 
        if caltypes is None:
            caltypes = CalFrom.CALTYPES.keys()

        if type(caltypes) is types.StringType:
            caltypes = (caltypes,)
            
        for c in caltypes:
            assert c in CalFrom.CALTYPES

        calfroms = (itertools.chain(*self.merged().values()))
        return set([cf.gaintable for cf in calfroms
                    if cf.caltype in caltypes])
        
    @staticmethod
    def dictify(dd):
        """
        Get a standard dictionary of the items in the tree.
        """
        return dict([(k, (DictCalState.dictify(v) if isinstance(v, dict) else v))
                     for (k, v) in dd.items()])

    def merged(self, hide_empty=False):
        hashes = {}
        flattened = self._flattened(hide_empty=hide_empty)
        for (calto_tup, calfrom) in flattened:
            # create a tuple, as lists are not hashable
            calfrom_hash = tuple([hash(cf) for cf in calfrom])
            if calfrom_hash not in hashes:
                LOG.trace('Creating new CalFrom hash for %s', calfrom)
                calto_args = CalToArgs(*[[x,] for x in calto_tup])
                hashes[calfrom_hash] = (calto_args, calfrom)
            else:
                calto_args = hashes[calfrom_hash][0]

                for old_key, new_key in zip(calto_args, calto_tup):
                    if new_key not in old_key:
                        old_key.append(new_key)

        for calto_tup, _ in hashes.values():
            for l in calto_tup:
                l.sort()

        result = {}
        for calto_args, calfrom in hashes.values():            
            for vis in calto_args.vis:
                calto = CalTo(vis=vis,
                              spw=self._commafy(calto_args.spw), 
                              field=self._commafy(calto_args.field),
                              intent=self._commafy(calto_args.intent), 
                              antenna=self._commafy(calto_args.antenna))
                result[calto] = calfrom

        return result

    def _commafy(self, l=[]):
        return ','.join([str(i) for i in l])

    def _flattened(self, hide_empty=True):
        active = ((ct_tuple, [cf for cf in cf_list if cf not in self._removed])
                  for (ct_tuple, cf_list) in utils.flatten_dict(self))

        if hide_empty:
            return ((ct_tuple, cf_list) for ct_tuple, cf_list in active
                    if len(cf_list) is not 0)

        return active

    def as_applycal(self):
        calapps = [CalApplication(k,v) 
                   for k,v in self.merged(hide_empty=True).items()]
        return '\n'.join([str(c) for c in calapps])

    def __str__(self):
        return self.as_applycal()

    def __repr__(self):
        return self.as_applycal()
#        return 'CalState(%s)' % repr(CalState.dictify(self.merged))


class DictCalLibrary(object):
    """
    CalLibrary is the root object for the pipeline calibration state.
    """
    def __init__(self, context):
        self._context = context
        self._active = DictCalState()
        self._applied = DictCalState()

    def clear(self):
        self._active = DictCalState()
        self._applied = DictCalState()

    def _add(self, calto, calfroms, calstate):
        if type(calfroms) is not types.ListType:
            calfroms = [calfroms]

        calto = CalToIdAdapter(self._context, calto)
        ms_name = calto.ms.name

        for spw_id in calto.spw:
            for field_id in calto.field:
                for intent in calto.get_field_intents(field_id, spw_id):
                    for antenna_id in calto.antenna:
                        for cf in calfroms:
                            # now that we use immutable CalFroms, we don't
                            # need to deepcopy the object we are appending
                            calstate[ms_name][spw_id][field_id][intent][antenna_id].append(cf)

        LOG.trace('Calstate after _add:\n%s', calstate.as_applycal())

    def _calc_filename(self, filename=None):
        if filename in ('', None):
            filename = os.path.join(self._context.output_dir,
                                    self._context.name + '.calstate')
        return filename

    def _export(self, calstate, filename=None):
        filename = self._calc_filename(filename)

        calapps = [CalApplication(k,v) for k,v in calstate.merged().items()]

        with open(filename, 'w') as export_file:
            for ca in calapps:
                export_file.write(ca.as_applycal())
                export_file.write('\n')

    def _remove(self, calstate, calfrom, calto=None):
        # If this is a global removal, as signified by the lack of a CalTo to
        # give any target data selection, we can simply mark the CalFrom as
        # removed
        if calto is None:
            calstate.global_remove(calfrom)

        # But if this is a partial removal, go through the dictionary
        # dimensions and remove it from the data selection specified by the
        # CalTo
        else:
            if type(calfrom) is not types.ListType:
                calfrom = [calfrom]

            calto = CalToIdAdapter(self._context, calto)
            ms_name = calto.ms.name

            for spw_id in calto.spw:
                for field_id in calto.field:
                    for intent in calto.get_field_intents(field_id, spw_id):
                        for antenna_id in calto.antenna:
                            current = calstate[ms_name][spw_id][field_id][intent][antenna_id]
                            for c in calfrom:
                                try:
                                    current.remove(c)
                                except ValueError, _:
                                    LOG.debug('%s not found in calstate', c)


        LOG.trace('Calstate after _remove:\n%s', calstate.as_applycal())

    def add(self, calto, calfroms):
        # If we are adding a previously removed CalFrom back into a
        # CalState, we assume that the user really want the previous
        # CalFrom not to be ignored in future runs rather than adding
        # a second entry for this CalFrom into the CalState.
        if not isinstance(calfroms, collections.Iterable):
            calfroms = [calfroms]

        calfroms_to_reactivate = self._active._removed.intersection(set(calfroms))
        self._active.global_reactivate(calfroms_to_reactivate)

        calfroms_to_add = [cf for cf in calfroms if cf not in calfroms_to_reactivate]
        if calfroms_to_add:
            self._add(calto, calfroms_to_add, self._active)

    @property
    def active(self):
        """
        CalState holding CalApplications to be (pre-)applied to the MS.
        """
        return self._active

    @property
    def applied(self):
        """
        CalState holding CalApplications that have been applied to the MS via
        the pipeline applycal task.
        """
        return self._applied

    def export(self, filename=None):
        """
        Export the pre-apply calibration state to disk.

        The pre-apply calibrations held in the 'active' CalState will be
        written to disk as a set of equivalent applycal calls.
        """
        filename = self._calc_filename(filename)
        LOG.info('Exporting current calibration state to %s', filename)
        self._export(self._active, filename)

    def export_applied(self, filename=None):
        """
        Export the applied calibration state to disk.

        The calibrations held in the 'applied' CalState will be written to
        disk as a set of equivalent applycal calls.
        """
        filename = self._calc_filename(filename)
        LOG.info('Exporting applied calibration state to %s', filename)
        self._export(self._applied, filename)

    def get_calstate(self, calto, hide_null=True, ignore=None):
        """
        Get the calibration state for a target data selection.
        """
        if ignore is None:
            ignore = []

        # wrap the text-only CalTo in a CalToIdAdapter, which will parse the
        # CalTo properties and give us the appropriate subtable IDs to iterate
        # over
        id_resolver = CalToIdAdapter(self._context, calto)
        ms_name = id_resolver.ms.name

        result = DictCalState()
        for spw_id in id_resolver.spw:
            for field_id in id_resolver.field:
                for intent in id_resolver.get_field_intents(field_id, spw_id):
                    for antenna_id in id_resolver.antenna:
                        calfroms = self._active[ms_name][spw_id][field_id][intent][antenna_id]

                        # Make the hash function ignore the ignored properties
                        # by setting their value to the default (and equal)
                        # value.
                        calfrom_copies = [self._copy_calfrom(cf, ignore)
                                          for cf in calfroms
                                          if cf not in self._active._removed]

                        result[ms_name][spw_id][field_id][intent][antenna_id] = calfrom_copies

        return result

    def _copy_calfrom(self, to_copy, ignore=None):
        if ignore is None:
            ignore = []

        calfrom_properties = ['caltype', 'calwt', 'gainfield', 'gaintable',
                              'interp', 'spwmap']

        copied = {k: getattr(to_copy, k) for k in calfrom_properties
                  if k not in ignore}

        return CalFrom(**copied)

    def import_state(self, filename=None, append=False):
        filename = self._calc_filename(filename)

        LOG.info('Importing calibration state from %s' % filename)
        calapps = []
        with open(filename, 'r') as import_file:
            for line in [l for l in import_file if l.startswith('applycal(')]:
                calapp = CalApplication.from_export(line)
                calapps.append(calapp)

        if not append:
            self._active = DictCalState()

        for calapp in calapps:
            LOG.debug('Adding %s' % calapp)
            self.add(calapp.calto, calapp.calfrom)

        LOG.info('Calibration state after import:\n'
                 '%s' % self.active.as_applycal())

    def mark_as_applied(self, calto, calfrom):
        self._remove(self._active, calfrom, calto)
        self._add(calto, calfrom, self._applied)

        LOG.debug('New calibration state:\n'
                 '%s' % self.active.as_applycal())
        LOG.debug('Applied calibration state:\n'
                  '%s' % self.applied.as_applycal())


# CalLibrary based on interval trees -----------------------------------------


def unit(x):
    return x


def contiguous_sequences(l):
    """
    Group a sequence of numbers into contiguous groups

    :param l: a sequence
    :return: list of Intervals
    """
    s = sorted([int(d) for d in l])

    for _, g in itertools.groupby(enumerate(s), lambda (i, x): i - x):
        rng = map(operator.itemgetter(1), g)
        yield rng


# intervals are not inclusive of the upper bound, hence the +1 on the right bound
sequence_to_range = lambda l: (l[0], l[-1]+1)


def sequence_to_casa_range(seq):
    def as_casa_range(seq):
        size = len(seq)
        if size is 0:
            return ''
        elif size is 1:
            return '{}'.format(seq[0])
        else:
            return '{}~{}'.format(seq[0], seq[-1])

    return (as_casa_range(seq) for seq in contiguous_sequences(seq))


class CalToIntervalAdapter(object):
    def __init__(self, context, calto):
        self._context = context
        self._calto = calto

        ms = context.observing_run.get_ms(calto.vis)
        self.ms = ms

        antenna_ids = (a.id for a in ms.get_antenna(calto.antenna))
        self.antenna = [sequence_to_range(seq) for seq in contiguous_sequences(antenna_ids)]

        field_ids = (f.id for f in ms.get_fields(task_arg=calto.field))
        self.field = [sequence_to_range(seq) for seq in contiguous_sequences(field_ids)]

        spw_ids = (spw.id for spw in ms.get_spectral_windows(self._calto.spw, science_windows_only=False))
        self.spw = [sequence_to_range(seq) for seq in contiguous_sequences(spw_ids)]

        id_to_intent = get_intent_id_map(ms)
        intent_to_id = {v: i for i, v in id_to_intent.iteritems()}

        if self._calto.intent == '':
            self.intent = [(0, len(intent_to_id))]
        else:
            str_intents = self._calto.intent.split(',')
            # the conditional check for intent is required as task parameters may
            # specify an intent that is not in the MS, such as CHECK.
            intent_ids = (intent_to_id[intent] for intent in str_intents
                          if intent in intent_to_id)
            self.intent = [sequence_to_range(seq) for seq in contiguous_sequences(intent_ids)]

    def __str__(self):
        return ('CalToIntervalAdapter(ms={!r}, field={!r}, intent={!r}, spw={!r}, antenna={!r})'.format(
            os.path.basename(self._calto.vis), self.field, self.intent, self.spw, self.antenna))

    def __repr__(self):
        return ('CalToIntervalAdapter({!s}, {!s})'.format(self._context, self._calto))


def create_data_reducer(join):
    """
    Return a function that creates a new TimestampedData object containing the
    result of executing the given operation on two TimestampedData objects.

    The use case for this function is actually quite simple: perform an
    operation on two TimestampedData objects (add, subtract, etc.) and put the
    result in a new TimestampedData object.

    The resulting TimestampedData object has a creation time equal to that of
    the oldest input object.

    :param join: the function to call on the two input objects
    :return:
    """
    def m(td1, td2, join=join):
        oldest = min(td1, td2)
        newest = max(td1, td2)
        return TimestampedData(oldest.time, join(oldest, newest))
    return m


def merge_lists(join_fn=operator.add):
    """
    Return a function that merge two lists by calling the input operation
    on the two input arguments.

    :param join_fn:
    :return:
    """
    def m(oldest, newest):
        return join_fn(oldest.data, newest.data)
    return m


def merge_intervaltrees(on_intersect):
    """
    Return a function that merges two IntervalTrees, executing a function
    on the intersecting Interval ranges in the resulting merged IntervalTree.

    :param on_intersect: the function to call on overlapping Intervals
    :return: function
    """
    def m(oldest, newest):
        union = oldest.data | newest.data
        union.split_overlaps()
        union.merge_equals(data_reducer=on_intersect)
        return union
    return m


def ranges(lst):
    pos = (j - i for i, j in enumerate(lst))
    t = 0
    for i, els in itertools.groupby(pos):
        l = len(list(els))
        el = lst[t]
        t += l
        yield (el, el + l)


def safe_join(vals, separator=','):
    return separator.join((str(o) for o in vals))


def merge_contiguous_intervals(tree):
    """
    Merge contiguous Intervals with the same value into one Interval.

    :param tree: an IntervalTree
    :return: new IntervalTree with merged Intervals
    """
    merged_tree = intervaltree.IntervalTree()

    # sort the tree by the list values. This is a prerequisite of using the
    # itertools.groupby function
    data = sorted(tree, key=tsd_accessor)

    # create groups of Intervals that have the same list values. These are the
    # Intervals we can merge.
    for l, g in itertools.groupby(data, tsd_accessor):
        # create a tree for these Intervals with the same list values
        candidate_tree = intervaltree.IntervalTree(g)
        # expand the Interval ranges to a list of integers (e.g. 1,3,6,7,8),
        # then find the contiguous ranges
        sequence = list(itertools.chain(*[range(i.begin, i.end) for i in sorted(candidate_tree)]))
        for begin, end in ranges(sequence):
            vals = sorted(candidate_tree[begin:end], key=tsd_accessor)
            # create a new Interval for the contiguous range but using an
            # existing value, thus reusing the timestamp
            merged_tree.add(intervaltree.Interval(begin, end, vals[0].data))

    return merged_tree


# function to access the value of a TimestampedData inside an Interval
tsd_accessor = operator.attrgetter('data.data')


def defrag_interval_tree(tree):
    """
    Condense an IntervalTree by consolidating fragmented entries with the same
    value into contiguous Intervals.

    :param tree:
    :return:
    """
    # if the intervals in this tree do not contain IntervalTrees, we're at the final
    # branch - the branch with the list of CalApplications. The intervals in this
    # branch can be merged
    leaf_values = [tsd_accessor(interval) for interval in tree]
    if not all((isinstance(v, intervaltree.IntervalTree) for v in leaf_values)):
        return merge_contiguous_intervals(tree)

    # otherwise call recursively
    merged_tree = intervaltree.IntervalTree()
    for interval in tree:
        new_leaf = intervaltree.Interval(interval.begin,
                                         interval.end,
                                         defrag_interval_tree(tsd_accessor(interval)))
        merged_tree.add(new_leaf)

    return merged_tree


# this chain of functions defines how to add overlapping Intervals when adding
# IntervalTrees
intent_add = create_data_reducer(join=merge_lists(join_fn=operator.add))
field_add = create_data_reducer(join=merge_intervaltrees(intent_add))
spw_add = create_data_reducer(join=merge_intervaltrees(field_add))
ant_add = create_data_reducer(join=merge_intervaltrees(spw_add))

# this chain of functions defines how to subtract overlapping Intervals when
# subtracting IntervalTrees
intent_sub = create_data_reducer(join=merge_lists(join_fn=lambda x, y: [item for item in x if item not in y]))
field_sub = create_data_reducer(join=merge_intervaltrees(intent_sub))
spw_sub = create_data_reducer(join=merge_intervaltrees(field_sub))
ant_sub = create_data_reducer(join=merge_intervaltrees(spw_sub))


def interval_to_set(interval):
    """
    Get the all the indexes covered by an Interval.

    :param interval:
    :return:
    """
    return set(range(interval.begin, interval.end))


def get_id_to_intent_fn(id_to_intent):
    """
    Return a function that can convert intent IDs to a string intent.

    Takes a dict of dicts, first key mapping measurement set name and second
    key mapping numeric intent ID to string intent for that MS, e.g.

    {'a.ms': {0: 'PHASE', 1: 'BANDPASS'}

    :param id_to_intent: dict of vis : intent ID : string intent
    :return: set of intents
    """
    def f(vis, intent_ids):
        assert vis in id_to_intent

        mapping = id_to_intent[vis]

        # if the intent range spans all intents for this measurement set,
        # transform it back to '' to indicate all intents
        if all((i in intent_ids for i in mapping)):
            return set('')

        return set((mapping[i] for i in intent_ids))

    return f


def get_id_to_field_fn(id_to_field):
    """
    Return a function that can convert field IDs to a field name.

    Takes a dict of dicts, first key mapping measurement set name and second
    key mapping numeric field ID to field name, eg.

    {'a.ms': {0: 'field 1', 1: 'field 2'}

    :param id_to_field: dict of vis : field ID : field name
    :return: set of field names (or field IDs if names are not unique)
    """
    def f(vis, field_ids):
        assert vis in id_to_field

        mapping = id_to_field[vis]

        # if the field range spans all fields for this measurement set,
        # transform it back to '' to indicate all fields
        if all((i in field_ids for i in mapping)):
            return set('')

        all_field_names = mapping.values()
        names_are_unique = len(all_field_names) is len(set(all_field_names))

        if names_are_unique:
            return set((mapping[i] for i in field_ids))
        else:
            return field_ids

    return f


def expand_interval(interval, calto_args, calto_fn):
    """
    Convert an Interval into the equivalent list of (CalTo, [CalFrom..])
    2-tuples.

    This function is the partner function to expand_intervaltree. See the
    documention for expand_intervaltree for more details on the argument
    format for this function.

    :param interval: the Interval to convert
    :param calto_args: the list of (argument name, conversion function) 2-tuples
     for the remaining dimensions
    :param calto_fn: the partial CalToArgs application
    :return:  a list of (CalTo, [CalFrom..]) tuples
    """
    data = tsd_accessor(interval)
    numeric_ids = interval_to_set(interval)

    arg_name, conversion_fn = calto_args[0]
    processed = conversion_fn(numeric_ids)
    kwargs = {arg_name: processed}

    calto_fn = functools.partial(calto_fn, **kwargs)

    if isinstance(data, intervaltree.IntervalTree):
        return expand_intervaltree(data, calto_args[1:], calto_fn)
    else:
        # the return type is an iterable of 2-tuples
        return ((calto_fn(), data),)


def expand_intervaltree(tree, convert_fns, calto_fn):
    """
    Convert an IntervalTree into the equivalent list of (CalTo, [CalFrom..])
    2-tuples.

    The second argument for this function is a list of 2-tuples of (CalToArgs
    constructor argument for this dimension, value conversion function for
    this dimension). The conversion function takes in a set of integer indexes
    and converts it to a suitable (probably more human-readable value) for that
    dimension, e.g. a conversion from field ID to field name. So, for a
    dimension that supplies the 'antenna' argument to CalToArgs and should
    prefix 'DV' to each antenna index, the tuple for that dimension could be
    ('antenna', lambda id: {'DV%s' % i for i in field_ids}).

    The third argument is the partially-applied CalToArgs constructor. A
    CalToArgs needs a number of arguments (vis, field, spw, etc.), each of
    which corresponds to a dimension of the IntervalTree and which must be
    supplied at CalToArgs creation time. To achieve this while iterating
    through the dimensions (when the constructor arguments are not fully
    known), object creation is delayed by performing just a partial
    application, adding the keyword for the current dimension to the partial
    application. At the final leaf node, when all constructor arguments have
    been partially applied, we can call the partial function and get the
    CalToArgs.

    :param tree: the IntervalTree to convert
    :param convert_fns: the list of (argument name, conversion function) 2-tuples
     for the remaining dimensions
    :param calto_fn: the partial CalToArgs application
    :return: a list of (CalTo, [CalFrom..]) tuples
    """
    return (x
            for interval in tree
            for x in expand_interval(interval, convert_fns, calto_fn))


def expand_calstate_to_calapps(calstate):
    """
    Convert an IntervalCalState into a list of (CalTo, [CalFrom..]) tuples.

    :param calstate: the IntervalCalState to convert
    :return: a list of 2-tuples, first element a Calto, second element a list
    of CalFroms
    """
    # get functions to map from integer IDs to field and intent for this MS
    id_to_field_fn = get_id_to_field_fn(calstate.id_to_field)
    id_to_intent_fn = get_id_to_intent_fn(calstate.id_to_intent)

    calapps = []

    for vis in calstate:
        # Set the vis argument for the CalToArgs constructor through partial
        # application. The subsequent calls will set the other arguments for
        # CalToArgs (field, intent, spw, etc.)
        caltoarg_fn = functools.partial(CalToArgs, vis={vis})

        # partially apply vis so that callees can call id-to-X functions
        # directly rather than having to push the vis arg through the layers
        intent_fn = functools.partial(id_to_intent_fn, vis)
        field_fn = functools.partial(id_to_field_fn, vis)

        # maps dimension order to the CalToArgs argument and value processing
        # function for that dimension. We have to process values at this point
        # while vis is still atomic, as once it's a set for the CalTo (as it
        # justly needs to be to handle sessions) we can't determine vis to
        # perform the mapping.
        caltoarg_dimension = (('antenna', unit),
                              ('spw', unit),
                              ('field', field_fn),
                              ('intent', intent_fn))

        vis_tree = calstate[vis]
        vis_calapps = expand_intervaltree(vis_tree, caltoarg_dimension, caltoarg_fn)
        calapps.extend(vis_calapps)

    return calapps


def consolidate_calibrations(calapps):
    """
    Consolidate a list of (CalTo, [CalFrom..]) 2-tuples into a smaller set of
    equivalent applications by consolidating their data selection arguments.

    This function works by merging the data selections of CalTo objects that
    have the same calibration application, as determined by the values and
    data selection present in the CalFroms.

    :param calapps: an iterable of (CalTo, [CalFrom..]) 2-tuples
    :return: a list of (CalTo, [CalFrom..]) tuples
    """

    # dict mapping an object hash to the object itself:
    #     hash([CalFrom, ...]): [CalFrom, ...]
    hash_to_calfroms = {}
    # dict mapping from object hash to corresponding list of CalToArgs
    hash_to_calto_args = collections.defaultdict(list)

    # create our maps of hashes, which we need to test for overlapping data
    # selections
    for calto_args, calfroms in calapps:
        # create a tuple, as lists are not hashable
        calfrom_hash = tuple([hash(cf) for cf in calfroms])
        hash_to_calto_args[calfrom_hash].append(calto_args)

        if calfrom_hash not in hash_to_calfroms:
            hash_to_calfroms[calfrom_hash] = calfroms

    # dict that maps holds accepted data selections and their CalFroms
    accepted = {}
    for calfrom_hash, calto_args in hash_to_calto_args.iteritems():
        # assemble the other data selections (the other CalToArgs) which we
        # will use to search for conflicting data selections
        other_data_selections = []
        for v in [v for k, v in hash_to_calto_args.iteritems() if k != calfrom_hash]:
            other_data_selections.extend(v)

        for to_merge in calto_args:
            if calfrom_hash not in accepted:
                # first time round for this calibration application, therefore it can always be added
                # as there will be nothing to merge
                accepted[calfrom_hash] = [(copy.deepcopy(to_merge), hash_to_calfroms[calfrom_hash])]
                continue

            for idx, (existing_calto, calfroms) in enumerate(accepted[calfrom_hash]):
                proposed_calto = CalToArgs(*copy.deepcopy(existing_calto))

                for proposed_values, to_merge_values in zip(proposed_calto, to_merge):
                    proposed_values.update(to_merge_values)

                # if the merged data selection does not conflict with any of
                # the explicitly registered data selections that require a
                # different calibration application, then it is safe to add
                # the merged data selection and discard the unmerged data
                # selection
                if not any((data_selection_contains(proposed_calto, other) for other in other_data_selections)):
                    LOG.trace('No conflicting data selection detected')
                    LOG.trace('Accepting merged data selection: {!s}'.format(proposed_calto))
                    LOG.trace('Discarding unmerged data selection: {!s}'.format(to_merge))
                    accepted[calfrom_hash][idx] = (proposed_calto, hash_to_calfroms[calfrom_hash])
                    break

            else:
                # we get here if all of the proposed merged data selections
                # conflict with the data selection in hand. In this case, it
                # should be added as it stands, completely unaltered.
                LOG.trace('Merged data selection conflicts with other registrations')
                LOG.trace('Abandoning proposed data selection: {!s}'.format(proposed_calto))
                LOG.trace('Appending new unmerged data selection: {!s}'.format(to_merge))
                unmergeable = (to_merge, hash_to_calfroms[calfrom_hash])
                accepted[calfrom_hash].append(unmergeable)

    # dict values are lists, which we need to flatten into a single list
    result = []
    for l in accepted.values():
        result.extend(l)
    return result


def data_selection_contains(proposed, calto_args):
    """
    Return True if one data selection is contained within another.

    :param proposed: data selection 1
    :type proposed: CalToArgs
    :param calto_args: data selection 2
    :type calto_args: CalToArgs
    :return: True if data selection 2 is contained within data selection 1
    """
    return all([not proposed.vis.isdisjoint(calto_args.vis),
                not proposed.antenna.isdisjoint(calto_args.antenna),
                not proposed.field.isdisjoint(calto_args.field),
                not proposed.spw.isdisjoint(calto_args.spw),
                not proposed.intent.isdisjoint(calto_args.intent)])


def expand_calstate(calstate):
    """
    Convert an IntervalCalState into the equivalent consolidated list of
    (CalTo, [CalFrom..]) 2-tuples.

    This function is is the top-level entry point for converting a calibration
    state to 2-tuples. It consolidates data selections and converts numeric
    data selection IDs to friendly equivalents through downstream processing,

    :param calstate: the IntervalCalState to convert
    :return: a list of (CalTo, [CalFrom..]) tuples
    """
    # step 1: convert to [(CalTo, [CalFrom..]), ..]
    unmerged = expand_calstate_to_calapps(calstate)

    # step 2: consolidate entries with identical calibrations
    consolidated = consolidate_calibrations(unmerged)

    # step 3: take the list of (CalToArgs, [CalFrom]) tuples, taking any
    # CalToArgs whose vis property targets multiple MSes and dividing them
    # into n entries each targeting a single MS. This keeps the export data
    # format more readable as each entry targets a single measurement set.
    per_ms = [(CalToArgs({vis}, *cta[1:]), calfroms)
              for cta, calfroms in consolidated for vis in cta.vis]

    # step 4: convert integer ranges in data selection to friendlier CASA range
    # syntax, e.g.  [1,2,3,4,6,8] => ['1~4','6','8']
    casa_format = [(CalToArgs(vis=calto_args.vis,
                              antenna=sequence_to_casa_range(calto_args.antenna),
                              spw=sequence_to_casa_range(calto_args.spw),
                              field=calto_args.field,
                              intent=calto_args.intent), calfroms)
                   for calto_args, calfroms in per_ms]

    # step 5: convert each iterable argument to a comma-separated string
    return [(CalToArgs(*[safe_join(arg) for arg in calto_args]), calfroms)
            for calto_args, calfroms in casa_format]


def get_min_max(l, keyfunc=None):
    if keyfunc:
        l = map(keyfunc, l)
    # this function is used to specify Interval ranges, which are not
    # inclusive of the upper bound - hence the +1.
    return min(l), max(l) + 1


def create_interval_tree(a):
    """
    Create an IntervalTree containing a set of Intervals.

    The input argument used to create the Intervals is an iterable of
    3-tuples, each 3-tuple defined as:

    (interval start, interval end, function giving value for that interval).

    :param a: the iterable of argument tuples
    :return: IntervalTree
    """
    intervals = (intervaltree.Interval(begin, end, data_fn())
                 for begin, end, data_fn in a)
    return intervaltree.IntervalTree(intervals)


def create_interval_tree_nd(intervals, value_fn):
    """
    Create a multidimensional IntervalTree. Each Interval within the
    IntervalTree points to the next dimension, with the final Interval
    containing the value given by calling value_fn.

    :param intervals: a list of Interval lists, with range of the final
    (deepest) first, ending with the range of the root dimension
    :param value_fn: function that returns value for the final dimension
    :return: an IntervalTree
    """
    # wrapper to create TimestampedData objects with a fixed timestamp of now
    tsd_now = functools.partial(TimestampedData, datetime.datetime.now())

    # Intervals have to point to the next dimension, so we must create the
    # dimensions in reverse order, starting with the deepest dimension.
    final_tree = create_interval_tree([(begin, end, lambda: tsd_now(value_fn()))
                                       for begin, end in intervals[0]])
    root = final_tree

    # the parent dimensions just link to their child dimensions, similar to a
    # linked list
    for current_dim_intervals in intervals[1:]:
        dim_args = [(begin, end, lambda: tsd_now(root))
                    for begin, end in current_dim_intervals]
        root = create_interval_tree(dim_args)

    return root


def create_interval_tree_for_ms(ms):
    """
    Create a new IntervalTree fitted to the dimensions of a measurement set.

    This function creates a new IntervalTree with the size of the antenna,
    spw, field and intent dimensions fitted to envelop of the input measurement
    set.

    :param ms:
    :return: an IntervalTree
    """
    tree_intervals = [
        [(0, len(ms.intents))],
        [get_min_max(ms.fields, keyfunc=operator.attrgetter('id'))],
        [get_min_max(ms.spectral_windows, keyfunc=operator.attrgetter('id'))],
        [get_min_max(ms.antennas, keyfunc=operator.attrgetter('id'))]
    ]

    return create_interval_tree_nd(tree_intervals, list)


def trim(tree, ranges):
    """
    Return an IntervalTree trimmed to the specified ranges.

    Ranges are specified as tuples of (begin, end).

    :param tree: the IntervalTree to trim
    :param ranges: a list of range tuples
    :return: the trimmed IntervalTree
    """
    insertions = set()

    for begin, end in ranges:
        # locate Intervals overlapping the range, not just those completely
        # contained within the range
        overlapping = tree.search(begin, end, strict=False)

        # truncate the Intervals to the range boundaries
        truncated = {intervaltree.Interval(max(iv.begin, begin),
                                           min(iv.end, end),
                                           iv.data)
                     for iv in overlapping}
        insertions.update(truncated)

    return intervaltree.IntervalTree(insertions)


def trim_nd(tree, selection):
    """
    Return an IntervalTree with each dimension trimmed to the specified
    set of ranges.

    The data selection for each dimension is specified as a sequence of
    (begin, end) tuples; the data selection for the tree as a whole is a
    sequence of these dimension sequences. For example, the data selection

        [ [(1, 3)], [(0, 5), (7, 8)] ]

    would select 1-3 from the first dimension and 0-5, and 7 from the
    second dimension.

    :param tree: the IntervalTree to trim
    :param selection: the sequence of data selections for each dimension
    :return:
    """
    # print('tree={}\nselection={}'.format(tree, selection))
    root = trim(tree, selection[0])

    if len(selection) > 1:
        # TimestampedData objects are immutable namedtuples, so to change the
        # data they point to we must replace the whole Interval. These
        # replacement Intervals are identical to those they replace except for
        # the TimestampedData.data property, which is trimmed to the next set
        # of dimensions
        replacements = {
            intervaltree.Interval(iv.begin,
                                  iv.end,
                                  TimestampedData(iv.data.time,
                                                  trim_nd(tsd_accessor(iv), selection[1:])))
            for iv in root
        }
        # now remove the untrimmed Intervals and replace them with our trimmed
        # versions
        root.clear()
        root.update(replacements)

    return root


def get_intent_id_map(ms):
    """
    Get the mapping of intent ID to string intent for a measurement set.

    :param ms: the measurement set to analyse
    :return: a dict of intent ID: intent
    """
    # intents are sorted to ensure consistent ordering
    return dict(enumerate(sorted(ms.intents)))


class IntervalCalState(object):
    """
    CalState is a data structure used to map calibrations for all data
    registered with the pipeline.

    It is implemented as a multi-dimensional array indexed by data selection
    parameters (ms, spw, field, intent, antenna), with the end value being a
    list of CalFroms, representing the calibrations to be applied to that data
    selection.
    """
    def __init__(self):
        self.data = {}
        self.id_to_intent = {}
        self.id_to_field = {}
        self.shape = {}

    @staticmethod
    def from_calapplication(context, calto, calfroms):
        if type(calfroms) is not types.ListType:
            calfroms = [calfroms]

        adapted = CalToIntervalAdapter(context, calto)

        selection_intervals = [
            adapted.intent,
            adapted.field,
            adapted.spw,
            adapted.antenna
        ]
        selection = create_interval_tree_nd(selection_intervals, lambda: calfroms)

        calstate = IntervalCalState.create_from_context(context)
        ms = context.observing_run.get_ms(calto.vis)
        calstate.data[ms.name] = selection

        calstate.data[ms.name] = trim_to_valid_data_selection(calstate, ms.name)[ms.name]

        return calstate

    @staticmethod
    def create_from_context(context):
        # holds a mapping of numeric intent ID to string intent for each ms.
        id_to_intent = {ms.name: get_intent_id_map(ms)
                        for ms in context.observing_run.measurement_sets}

        # holds a mapping of numeric ID to field name for each ms.
        id_to_field = {ms.name: {field.id: field.name for field in ms.fields}
                       for ms in context.observing_run.measurement_sets}

        calstate = IntervalCalState()
        calstate.id_to_intent.update(id_to_intent)
        calstate.id_to_field.update(id_to_field)

        interval_trees = {ms.name: create_interval_tree_for_ms(ms)
                          for ms in context.observing_run.measurement_sets}
        calstate.data.update(interval_trees)

        # create the shape of valid data selections for future trimming
        # See CAS-9415: CalLibrary needs a way to filter out calibration
        # applications for missing data selections
        for vis, antenna_tree in interval_trees.iteritems():
            # create map of observing intent to intent ID by inverting
            # existing map
            intent_to_id = {v: k for k, v in calstate.id_to_intent[vis].iteritems()}

            ms = context.observing_run.get_ms(vis)
            spw_shape = {}
            for spw in ms.spectral_windows:
                intents_for_field = {}
                for field in ms.fields:
                    if spw in field.valid_spws:
                        # construct the list of observed intent IDs for this field
                        observed_intent_ids = (intent_to_id[i] for i in field.intents)
                        # convert the intent IDs to an IntervalTree-friendly range
                        # and record it against the field ID
                        intents_for_field[field.id] = tuple(
                            sequence_to_range(seq) for seq in contiguous_sequences(observed_intent_ids)
                        )
                # merge adjacent field intervals that have identical values
                spw_shape[spw.id] = _merge_intervals(intents_for_field)
            # merge adjacent spw intervals that have identical values
            spw_shape = _merge_intervals(spw_shape)

            # assume that spws are observed by all antennas. Note the trailing comma to make it a tuple!
            # the inner tuple is needed to convert the generator comprehension to objects
            antenna_shape = (tuple((((interval.begin, interval.end),), spw_shape) for interval in antenna_tree),)

            calstate.shape[vis] = antenna_shape

        calstate.data = trim_to_valid_data_selection(calstate)

        return calstate

    def clear(self):
        for calstate in self.data.itervalues():
            calstate.clear()
        # do NOT clear the id mapping dicts as without access to the context
        # we have no way to repopulate them.
        # self.id_to_intent.clear()
        # self.id_to_field.clear()

    def trimmed(self, context, calto):
        """
        Return a copy of this IntervalCalState trimmed to the specified CalTo data selection.
        :param calto:
        :param selection_intervals:
        :return:
        """
        # wrap the text-only CalTo in a CalToIntervalAdapter, which will parse
        # the CalTo properties and give us the appropriate subtable IDs to
        # iterate over
        adapted = CalToIntervalAdapter(context, calto)

        # get the data selection as numeric IDs
        selection_intervals = [
            adapted.antenna,
            adapted.spw,
            adapted.field,
            adapted.intent
        ]

        # get a copy of this calstate trimmed to the CalTo data selection
        vis = adapted.ms.name

        # if the data has not been registered with the CalLibrary, create a
        # new and empty calibration application and register it, thereby
        # creating all the IntervalTrees necessary for the MS.
        if vis not in self.data:
            ms = context.observing_run.get_ms(vis)
            calto = CalTo(vis=ms.name)
            to_add = IntervalCalState.from_calapplication(context, calto, [])
            self.__iadd__(to_add)

        copied = copy.deepcopy(self.data[vis])
        trimmed = trim_nd(copied, selection_intervals)

        calstate = IntervalCalState()
        calstate.data[vis] = trimmed
        calstate.id_to_intent[vis] = self.id_to_intent[vis]
        calstate.id_to_field[vis] = self.id_to_field[vis]
        calstate.shape = self.shape

        return calstate

    def get_caltable(self, caltypes=None):
        """
        Get the names of all caltables registered with this CalState.

        If an optional caltypes argument is given, only caltables of the
        requested type will be returned.

        :param caltypes: Caltypes should be one or/a list of table
        types known in CalFrom.CALTYPES.

        :rtype: set of strings
        """
        if caltypes is None:
            caltypes = CalFrom.CALTYPES.keys()

        if type(caltypes) is types.StringType:
            caltypes = (caltypes,)

        for c in caltypes:
            assert c in CalFrom.CALTYPES

        return {calfrom.gaintable for calfroms in self.merged().itervalues()
                for calfrom in calfroms
                if calfrom.caltype in caltypes}

    def merged(self, hide_empty=False):
        calapps = expand_calstate(self)

        if hide_empty:
            calapps = filter(lambda (_, cf): len(cf) > 0, calapps)

        # TODO dict is unnecessary. refactor all usages of this class to use
        # the tuple
        return dict(calapps)

    def as_applycal(self):
        calapps = (CalApplication(calto, calfroms)
                   for calto, calfroms in self.merged(hide_empty=True).iteritems())

        return '\n'.join([str(c) for c in calapps])

    def __str__(self):
        return self.as_applycal()

    def _combine(self, other, combine_fn):
        """
        Get the union of this object combined with another IntervalCalState,
        applying a function to any Intervals that overlap.

        :param other: the other IntervalCalState
        :param combine_fn: the combining function to apply
        :return: IntervalCalState
        """
        calstate = IntervalCalState()

        # ensure that the other calstate is not considered equal to this
        # calstate, even if they the values they hold are identical. This step
        # is required so that all entries are added to in the union
        # (my_root | other_root) operation, and ensures that arithmetic like
        # 'calstate_x - calstate_x = 0' holds true.
        marker = uuid.uuid4()
        other_marked = set_calstate_marker(other, marker)

        # copy the ID mapping and shape data across.
        calstate.id_to_intent = self.id_to_intent
        calstate.id_to_field = self.id_to_field
        calstate.shape = self.shape

        for vis, my_root in self.data.iteritems():
            # adopt IntervalTrees present in just this object
            if vis not in other_marked.data:
                # TODO think: does this need to be a deep copy?
                calstate.data[vis] = copy.deepcopy(self.data[vis])
                continue

            # get the union of IntervalTrees for MSes present in both objects
            other_root = other_marked.data[vis]
            union = my_root | other_root
            union.split_overlaps()
            union.merge_equals(data_reducer=combine_fn)

            calstate.data[vis] = union

        calstate.data = trim_to_valid_data_selection(calstate)

        # Unmark the result calstate, thus eliminating any residual uuids
        unmarked = set_calstate_marker(calstate, None)

        return unmarked

    def __add__(self, other):
        calstate = self._combine(other, ant_add)

        # also adopt IntervalTrees only present in the other object
        for vis, other_root in other.data.iteritems():
            if vis not in self.data:
                calstate[vis] = other_root
                calstate.id_to_intent[vis] = other.id_to_intent[vis]
                calstate.id_to_field[vis] = other.id_to_field[vis]
                calstate.shape[vis] = other.shape[vis]

        return calstate

    def __iadd__(self, other):
        sum_state = self + other

        # adopt all properties from the added states
        self.data = sum_state.data
        self.id_to_field = sum_state.id_to_field
        self.id_to_intent = sum_state.id_to_intent
        self.shape = sum_state.shape

        return self

    def __sub__(self, other):
        return self._combine(other, ant_sub)

    def __getitem__(self, key):
        return self.data[key]

    def __setitem__(self, key, value):
        self.data[key] = value

    def __delitem__(self, key):
        del self.data[key]

    def __contains__(self, item):
        return item in self.data

    def __iter__(self):
        return iter(self.data)


def fix_cycle0_data_selection(context, merged):
    results = {}

    # We can't trust Cycle 0 data intents. If this is Cycle 0 data we need
    # to resolve the intents to fields and add them to the CalTo data
    # selection to ensure that the correct data is selected.
    for calto, calfroms in merged.items():
        vis = calto.vis
        ms = context.observing_run.get_ms(vis)
        if utils.get_epoch_as_datetime(ms.start_time) > CYCLE_0_END_DATE:
            results[calto] = calfroms
            continue

        if calto.intent is not '':
            fields_with_intent = ms.get_fields(task_arg=calto.field, intent=calto.intent)

            field_names = {f.name for f in fields_with_intent}
            if len(field_names) is len(fields_with_intent):
                new_field_arg = ','.join(field_names)
            else:
                new_field_arg = ','.join([str(field.id) for field in fields_with_intent])

            if new_field_arg != calto.field:
                LOG.info('Rewriting data selection to work around mislabeled Cycle 0 data intents. '
                         'Old field selection: %r; new field selection: %r', calto.field, new_field_arg)
                calto = CalTo(vis=calto.vis,
                              field=new_field_arg,
                              spw=calto.spw,
                              antenna=calto.antenna,
                              intent=calto.intent)

        results[calto] = calfroms

    return results


class IntervalCalLibrary(object):
    """
    CalLibrary is the root object for the pipeline calibration state.
    """
    def __init__(self, context):
        self._context = context
        self._active = IntervalCalState.create_from_context(context)
        self._applied = IntervalCalState.create_from_context(context)

    def clear(self):
        self._active.clear()
        self._applied.clear()

    def _calc_filename(self, filename=None):
        if filename in ('', None):
            filename = os.path.join(self._context.output_dir,
                                    self._context.name + '.calstate')
        return filename

    def _export(self, calstate, filename=None):
        filename = self._calc_filename(filename)

        calapps = [CalApplication(k, v) for k, v in calstate.merged().items()]

        with open(filename, 'w') as export_file:
            for ca in calapps:
                export_file.write(ca.as_applycal())
                export_file.write('\n')

    def add(self, calto, calfroms):
        to_add = IntervalCalState.from_calapplication(self._context, calto, calfroms)
        self._active += to_add

        LOG.trace('Calstate after _add:\n%s', self._active.as_applycal())

    @property
    def active(self):
        """
        CalState holding CalApplications to be (pre-)applied to the MS.
        """
        return self._active

    @property
    def applied(self):
        """
        CalState holding CalApplications that have been applied to the MS via
        the pipeline applycal task.
        """
        return self._applied

    def export(self, filename=None):
        """
        Export the pre-apply calibration state to disk.

        The pre-apply calibrations held in the 'active' CalState will be
        written to disk as a set of equivalent applycal calls.
        """
        filename = self._calc_filename(filename)
        LOG.info('Exporting current calibration state to %s', filename)
        self._export(self._active, filename)

    def export_applied(self, filename=None):
        """
        Export the applied calibration state to disk.

        The calibrations held in the 'applied' CalState will be written to
        disk as a set of equivalent applycal calls.
        """
        filename = self._calc_filename(filename)
        LOG.info('Exporting applied calibration state to %s', filename)
        self._export(self._applied, filename)

    def get_calstate(self, calto, ignore=None):
        """
        Get the active calibration state for a target data selection.

        :param calto: the data selection
        :param ignore:
        :return:
        """
        if ignore is None:
            ignore = []

        trimmed = self.active.trimmed(self._context, calto)

        # TODO replace with something like defrag_tree implementation
        for tree in trimmed.data.values():
            for antenna_interval in tree:
                spw_tree = tsd_accessor(antenna_interval)
                for spw_interval in spw_tree:
                    field_tree = tsd_accessor(spw_interval)
                    for field_interval in field_tree:
                        intent_tree = tsd_accessor(field_interval)
                        for intent_interval in intent_tree:
                            old_vals = tsd_accessor(intent_interval)
                            old_vals[:] = [self._copy_calfrom(cf, ignore)
                                           for cf in old_vals]

        return trimmed

    def _copy_calfrom(self, to_copy, ignore=None):
        if ignore is None:
            ignore = []

        calfrom_properties = ['caltype', 'calwt', 'gainfield', 'gaintable',
                              'interp', 'spwmap']

        copied = {k: getattr(to_copy, k) for k in calfrom_properties
                  if k not in ignore}

        return CalFrom(**copied)

    def import_state(self, filename=None, append=False):
        filename = self._calc_filename(filename)

        LOG.info('Importing calibration state from %s' % filename)
        calapps = []
        with open(filename, 'r') as import_file:
            for line in [l for l in import_file if l.startswith('applycal(')]:
                calapp = CalApplication.from_export(line)
                calapps.append(calapp)

        if not append:
            for _, calstate in self._active.data.iteritems():
                calstate.clear()

        for calapp in calapps:
            LOG.debug('Adding %s', calapp)
            self.add(calapp.calto, calapp.calfrom)

        LOG.info('Calibration state after import:\n%s', self.active.as_applycal())

    def mark_as_applied(self, calto, calfrom):
        application = IntervalCalState.from_calapplication(self._context, calto, calfrom)
        self._active -= application
        self._applied += application

        LOG.debug('New calibration state:\n%s', self.active.as_applycal())
        LOG.debug('Applied calibration state:\n%s', self.applied.as_applycal())


# CalState = DictCalState
# CalLibrary = DictCalLibrary
CalState = IntervalCalState
CalLibrary = IntervalCalLibrary


class TimestampedData(collections.namedtuple('TimestampedDataBase', ['time', 'data', 'marker'])):
    __slots__ = ()  # Saves memory, avoiding the need to create __dict__ for each interval

    def __new__(cls, time, data, marker=None):
        return super(TimestampedData, cls).__new__(cls, time, data, marker)

    def __cmp__(self, other):
        """
        Tells whether other sorts before, after or equal to this
        Interval.

        Sorting is by time then by data fields.

        If data fields are not both sortable types, data fields are
        compared alphabetically by type name.
        :param other: Interval
        :return: -1, 0, 1
        :rtype: int
        """
        s = self[0:1]
        o = other[0:1]
        if s != o:
            return -1 if s < o else 1
        try:
            if self.data == other.data:
                return 0
            return -1 if self.data < other.data else 1
        except TypeError:
            s = type(self.data).__name__
            o = type(other.data).__name__
            if s == o:
                return 0
            return -1 if s < o else 1

    def __lt__(self, other):
        """
        Less than operator. Parrots __cmp__()
        :param other: Interval or point
        :return: True or False
        :rtype: bool
        """
        return self.__cmp__(other) < 0

    def __gt__(self, other):
        """
        Greater than operator. Parrots __cmp__()
        :param other: Interval or point
        :return: True or False
        :rtype: bool
        """
        return self.__cmp__(other) > 0

    def __repr__(self):
        """
        Executable string representation of this Interval.
        :return: string representation
        :rtype: str
        """
        if self.marker is None:
            return 'TSD({0}, {1})'.format(self.time, repr(self.data))
        return 'TSD({0}, {1}, {2})'.format(self.time, repr(self.data), self.marker)

    def __str__(self):
        """
        String representation of this Interval.
        :return: string representation
        :rtype: str
        """
        return 'TSD({0})'.format(repr(self.data))

    def __eq__(self, other):
        """
        Whether the begins equal, the ends equal, and the data fields
        equal. Compare range_matches().
        :param other: Interval
        :return: True or False
        :rtype: bool
        """
        return (
            self.time == other.time and
            self.data == other.data and
            self.marker == other.marker
        )


def trim_to_valid_data_selection(calstate, vis=None):
    """
    Trim an IntervalCalState to the shape of valid (present) data selections.

    This is achieved by trimming Intervals for each dimension (antenna, spw,
    field, intent) to exclude ranges for which no data is present.

    See CAS-9415: CalLibrary needs a way to filter out calibration
    applications for missing data selections

    :param calstate: the calstate to shape
    :param vis: name of the calstate to shape. If not defined, shape all.
    :return: a new, shaped IntervalCalState
    """
    if vis is None:
        vislist = calstate.data.keys()
    else:
        vislist = [vis] if isinstance(vis, str) else vis

    results = {}
    for vis in vislist:
        antenna_tree = calstate.data[vis]

        new_root = intervaltree.IntervalTree()
        for antenna_tuple in calstate.shape[vis]:
            for antenna_ranges, spw_tuple in antenna_tuple:
                for spw_ranges, field_tuple in spw_tuple:
                    for field_ranges, intent_ranges in field_tuple:
                        tree_intervals = (antenna_ranges, spw_ranges, field_ranges, intent_ranges)
                        # print('Shaping to {!r}'.format(tree_intervals))
                        new_root |= trim_nd(antenna_tree, tree_intervals)

        results[vis] = new_root

    return results


def _merge_intervals(unmerged):
    """
    Merge adjacent Intervals (represented by the keys within the input dict)
    that have identical values and output an IntervalTree-friendly tuple of
    constructor arguments.

    For example, a dict containing

        {1: A, 2: B, 3:A, 4:A}

    would be converted to

        ((((1, 2), (3, 5)), 'A'), (((2, 3),), 'B'))

    :param unmerged: a dict mapping IDs to values
    :return: tuple of constructor arguments ready for create_interval_tree_nd
    """
    reversed = collections.defaultdict(set)
    for k, v in unmerged.iteritems():
        reversed[v].add(k)
    return tuple(sorted((tuple(sequence_to_range(seq) for seq in contiguous_sequences(v)), k)
                         for k, v in reversed.items()))


def _print_dimensions(calstate):
    """
    Debugging function used to print the dimensions of an IntervalCalState.

    :param calstate: the calstate to inspect
    :return:
    """
    for vis, antenna_tree in calstate.data.iteritems():
        for antenna_interval in antenna_tree.items():
            antenna_ranges = (antenna_interval.begin, antenna_interval.end)
            for spw_interval in antenna_interval.data.data:
                spw_ranges = (spw_interval.begin, spw_interval.end)
                for field_interval in spw_interval.data.data:
                    field_ranges = (field_interval.begin, field_interval.end)
                    for intent_interval in field_interval.data.data:
                        intent_ranges = (intent_interval.begin, intent_interval.end)

                        tree_intervals = (os.path.basename(vis), antenna_ranges, spw_ranges, field_ranges, intent_ranges)
                        print('{!r}'.format(tree_intervals))


def get_calto_from_inputs(inputs):
    """
    Get a CalTo data selection object based on the state of an Inputs object
    """
    return CalTo(vis=inputs.vis, field=inputs.field, spw=inputs.spw, intent=inputs.intent, antenna=inputs.antenna)


def copy_calfrom(calfrom, gaintable=None, gainfield=None, interp=None, spwmap=None, caltype=None,
                 calwt=None):
    """
    Return a copy of a CalFrom object, optionally overriding the copy's
    properties with the given function arguments.

    :param calfrom: the CalFrom to copy
    :type calfrom: CalFrom
    :param gaintable: new gaintable value, or None to adopt from the CalFrom
    :param gainfield: new gainfield value, or None to adopt from the CalFrom
    :param interp: new interp value, or None to adopt from the CalFrom
    :param spwmap: new spwmap value, or None to adopt from the CalFrom
    :param caltype: new caltype value, or None to adopt from the CalFrom
    :param calwt: new calwt value, or None to adopt from the CalFrom
    :return: new CalFrom object
    :rtype: CalFrom
    """
    if gaintable is None:
        gaintable = calfrom.gaintable
    if gainfield is None:
        gainfield = calfrom.gainfield
    if interp is None:
        interp = calfrom.interp
    if spwmap is None:
        spwmap = calfrom.spwmap
    if caltype is None:
        caltype = calfrom.caltype
    if calwt is None:
        calwt = calfrom.calwt

    return CalFrom(gaintable=gaintable, gainfield=gainfield, interp=interp, spwmap=spwmap, caltype=caltype, calwt=calwt)


def set_calstate_marker(calstate, marker):
    """
    Return a copy of a calstate, modified so that TimeStampedData objects in 
    the final leaf node are annotated with the given marker object.
     
    Technical details:
        
    CalFroms are flyweight objects, so two identical CalFroms have the same 
    hash. Identical hashes stop the IntervalTree union function from working 
    as expected: IntervalTrees are based on sets, and as such adding two 
    lists of CalFrom with identical hashes results in just one CalFrom list in 
    the final IntervalTree, when we actually *wanted* the duplicate to be 
    added.
    
    This function is used to ensure that CalState arithmetic works as 
    expected. By changing the TimeStampedData marker and thus making the 
    hashes different, 'identical' calibrations can indeed be duplicated in the
    IntervalTree union operation, and subsequently operated on in a 
    merge_equals step.
    
    :param calstate: the calstate to modify
    :param marker: the object to annotate calstates with
    :return: annotated calstate 
    """
    calstate_copy = copy.deepcopy(calstate)

    for vis, antenna_tree in calstate_copy.data.iteritems():
        for antenna_interval in antenna_tree.items():
            for spw_interval in antenna_interval.data.data:
                for field_interval in spw_interval.data.data:

                    to_remove = [i for i in field_interval.data.data]
                    to_add = []

                    intent_intervaltree = field_interval.data.data
                    for intent_interval in intent_intervaltree:
                        old_tsd = intent_interval.data
                        new_tsd = TimestampedData(time=old_tsd.time, data=old_tsd.data, marker=marker)
                        to_add.append(intervaltree.Interval(intent_interval.begin, intent_interval.end, new_tsd))

                    map(intent_intervaltree.remove, to_remove)
                    map(intent_intervaltree.add, to_add)

    return calstate_copy
