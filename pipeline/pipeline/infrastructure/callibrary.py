from __future__ import absolute_import
import collections
import copy
import itertools
import os
import re
import string
import types
import weakref

import asap

from . import logging
from . import utils

LOG = logging.get_logger(__name__)


CalToArgs = collections.namedtuple('CalToArgs',
                                   ['vis','spw','field','intent','antenna'])

# struct used to link calapplication to the task and inputs that created it
CalAppOrigin = collections.namedtuple('CalAppOrigin', ['task', 'inputs'])


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
        if type(calfrom) is not types.ListType:
            calfrom = [calfrom]
        self.calfrom = calfrom
        self.origin = origin

    @staticmethod
    def from_export(s):
        """
        Unmarshal a CalApplication from a string.

        :rtype: the unmarshalled :class:`CalApplication` object
        """
        d = eval(string.replace(s, 'applycal(', 'dict('))
        calto = CalTo(vis=d['vis'], field=d['field'], spw=d['spw'], 
                      antenna=d['antenna'])
        
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
            with utils.open_table(gaintable) as caltable:
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
        args = {'vis'       : self.vis,
                'field'     : self.field,
                'intent'    : self.intent,
                'spw'       : self.spw,
                'antenna'   : self.antenna,
                'gaintable' : self.gaintable,
                'gainfield' : self.gainfield,
                'spwmap'    : self.spwmap,
                'interp'    : self.interp,
                'calwt'     : self.calwt}
        
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
        'unknown'      : 0,
        'gaincal'      : 1,
        'bandpass'     : 2,
        'tsys'         : 3,
        'wvr'          : 4,
        'polarization' : 5,
        'antpos'       : 6,
        'gc'           : 7,
        'opac'         : 8,
        'rq'           : 9,
        'swpow'        : 10,
    }

    CALTYPE_TO_VISCAL = {
        'gaincal'  : ('G JONES', 'GSPLINE'),
        'bandpass' : ('B JONES', 'BPOLY'),
        'tsys'     : ('B TSYS',),
        'antpos'   : ('KANTPOS JONES',)
    }
    
    VISCAL = {
        'P JONES'       : 'P Jones (parallactic angle phase)',
        'T JONES'       : 'T Jones (polarization-independent troposphere)',
        'TF JONES'      : 'Tf Jones (frequency-dependent atmospheric complex gain)',
        'G JONES'       : 'G Jones (electronic Gain)',
        'B JONES'       : 'B Jones (bandpass)',
        'DGEN JONES'    : 'Dgen Jones (instrumental polarization)',
        'DFGEN JONES'   : 'Dfgen Jones (frequency-dependent instrumental polarization)',
        'D JONES'       : 'D Jones (instrumental polarization)',
        'DF JONES'      : 'Df Jones (frequency-dependent instrumental polarization)',
        'J JONES'       : 'J Jones (generic polarized gain)',
        'M MUELLER'     : 'M Mueller (baseline-based)',
        'MF MUELLER'    : 'Mf Mueller (closure bandpass)',
        'TOPAC'         : 'TOpac (Opacity corrections in amplitude)',
        'TFOPAC'        : 'TfOpac (frequency-dependent opacity)',
        'X MUELLER'     : 'X Mueller (baseline-based)',
        'X JONES'       : 'X Jones (antenna-based)',
        'XF JONES'      : 'Xf Jones (antenna-based)',
        'GLINXPH JONES' : 'GlinXph Jones (X-Y phase)',
        'B TSYS'        : 'B TSYS (freq-dep Tsys)',
        'BPOLY'         : 'B Jones Poly (bandpass)',
        'GSPLINE'       : 'G Jones SPLINE (elec. gain)',
        'KANTPOS JONES' : 'KAntPos Jones (antenna position errors)',
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
                spwmap=[], caltype='unknown', calwt=True):
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


class CalState(collections.defaultdict):
    """
    CalState is a data structure used to map calibrations for all data
    registered with the pipeline.
    
    It is implemented as a multi-dimensional array indexed by data selection
    parameters (ms, spw, field, intent, antenna), with the end value being a 
    list of CalFroms, representing the calibrations to be applied to that data
    selection.
    """

    def __init__(self, default_factory=_ms_dim):
        super(CalState, self).__init__(default_factory)

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
        '''
        Get a standard dictionary of the items in the tree.
        '''
        return dict([(k, (CalState.dictify(v) if isinstance(v, dict) else v))
                     for (k, v) in dd.items()])

    def merged(self, hide_empty=False):
        hashes = {}
        flattened = self._flattened(hide_empty=hide_empty)
        for (calto_tup, calfrom) in flattened.iteritems():
            # create a tuple, as lists are not hashable
            calfrom_hash = tuple([hash(cf) for cf in calfrom])
            if calfrom_hash not in hashes:
                LOG.trace('Creating new CalFrom hash for %s' % calfrom)
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

    def _flattened(self, keyReducer=lambda a,b:a+b, keyLift=lambda x:(x,),
                   init=(), hide_empty=True):
        def _flattenIter(pairs, _keyAccum=init):
            atoms = ((k,v) for k,v in pairs if not isinstance(v, collections.Mapping))
            submaps = ((k,v) for k,v in pairs if isinstance(v, collections.Mapping))
            def compress(k):
                return keyReducer(_keyAccum, keyLift(k))
            return itertools.chain(
                (
                    (compress(k),v) for k,v in atoms
                ),
                *[
                    _flattenIter(submap.items(), compress(k))
                    for k,submap in submaps
                ]
            )

        flat = dict(_flattenIter(self.items()))
        
        if hide_empty:
            return dict([(k, v) for (k, v) in flat.items() 
                         if len(v) is not 0])
        return flat 

    def as_applycal(self):
        calapps = [CalApplication(k,v) 
                   for k,v in self.merged(hide_empty=True).items()]
        return '\n'.join([str(c) for c in calapps])

    def __str__(self):
        return self.as_applycal()

    def __repr__(self):
        return self.as_applycal()
#        return 'CalState(%s)' % repr(CalState.dictify(self.merged))

        
class CalLibrary(object):
    """
    CalLibrary is the root object for the pipeline calibration state.
    """
    def __init__(self, context):
        self._context = context
        self._active = CalState()
        self._applied = CalState()

    def clear(self):
        self._active = CalState()
        self._applied = CalState()
        
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
                            # now that we use immutable flyweights, we don't 
                            # need the deepcopy
#                             cf_copy = copy.deepcopy(cf)
                            calstate[ms_name][spw_id][field_id][intent][antenna_id].append(cf)

        LOG.trace('Calstate after _add:\n'
                  '%s' % calstate.as_applycal())

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

    def _remove(self, calto, calfrom, calstate):
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
                                LOG.debug('%s not found in calstate' % c)

        LOG.trace('Calstate after _remove:\n'
                  '%s' % calstate.as_applycal())

    def add(self, calto, calfroms):
        self._add(calto, calfroms, self._active)

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
        LOG.info('Exporting current calibration state to %s' % filename)
        self._export(self._active, filename)

    def export_applied(self, filename=None):
        """
        Export the applied calibration state to disk.
        
        The calibrations held in the 'applied' CalState will be written to
        disk as a set of equivalent applycal calls.
        """
        filename = self._calc_filename(filename)
        LOG.info('Exporting applied calibration state to %s' % filename)
        self._export(self._applied, filename)

    def get_calstate(self, calto, hide_null=True, ignore=[]):
        """
        Get the calibration state for a target data selection.
        """
        # wrap the text-only CalTo in a CalToIdAdapter, which will parse the
        # CalTo properties and give us the appropriate subtable IDs to iterate
        # over 
        id_resolver = CalToIdAdapter(self._context, calto)        
        ms_name = id_resolver.ms.name

        result = CalState()
        for spw_id in id_resolver.spw:
            for field_id in id_resolver.field:
                for intent in id_resolver.get_field_intents(field_id, spw_id):
                    for antenna_id in id_resolver.antenna:
                        calfroms_orig = self._active[ms_name][spw_id][field_id][intent][antenna_id][:]

                        # Make the hash function ignore the ignored properties
                        # by setting their value to the default (and equal) 
                        # value.
                        calfroms_copy = [self._copy_calfrom(cf, ignore) 
                                         for cf in calfroms_orig]

                        result[ms_name][spw_id][field_id][intent][antenna_id] = calfroms_copy

        return result

    def _copy_calfrom(self, to_copy, ignore=[]):
        calfrom_properties = ['caltype', 'calwt', 'gainfield', 'gaintable',
                              'interp', 'spwmap']
        
        copied = dict((k, getattr(to_copy, k)) for k in calfrom_properties 
                      if k not in ignore)
        
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
            self._active = CalState()
           
        for calapp in calapps:
            LOG.debug('Adding %s' % calapp)        
            self.add(calapp.calto, calapp.calfrom)

        LOG.info('Calibration state after import:\n'
                 '%s' % self.active.as_applycal())

    def mark_as_applied(self, calto, calfrom):
        self._remove(calto, calfrom, self._active)
        self._add(calto, calfrom, self._applied)

        LOG.debug('New calibration state:\n'
                 '%s' % self.active.as_applycal())
        LOG.debug('Applied calibration state:\n'
                  '%s' % self.applied.as_applycal())


### single dish specific

class SDCalApplication(object):
    def __init__(self, calto, calfrom):
        self.calto = calto
        if type(calfrom) is not types.ListType:
            calfrom = [calfrom]
        self.calfrom = calfrom

    @staticmethod
    def from_export(s):
        d = eval(string.replace(s, 'sdcal2(', 'dict('))
        calto = CalTo(vis=d['infile'])
        
        # wrap these values in a list if they are single valued, 
        # eg. 'm31' -> ['m31']
        for key in ('applytable',):
            if type(d[key]) is types.StringType:
                d[key] = [d[key]]

        calfroms = []
        for tab in d['applytable']:
            with utils.open_table(tab) as applytable:
                caltype = applytable.getkeyword('ApplyType')

            if caltype == 'CALTSYS':
                calfrom = SDCalFrom(tab, interp=d['interp'], spwmap=d['ifmap'])
                calfrom.caltype = 'tsys'
            else:
                calfrom = SDCalFrom(tab, interp=d['interp'])
                calfrom.caltype = 'sky'
            LOG.trace('Marking caltable \'%s\' as caltype \'%s\''
                      '' % (tab, calfrom.caltype))

            calfroms.append(calfrom)
        
        return SDCalApplication(calto, calfroms)

    @staticmethod
    def iflist_to_spw(iflist):
        if isinstance(iflist, int):
            spw = str(iflist)
        else:
            # assume list or numpy.array 
            spw = str(list(iflist))[1:-1].replace(' ','')
        return spw

    @staticmethod
    def spw_to_iflist(spw):
        # currently only supports simple spw specification
        # using , and ~
        elements = spw.split(',')
        iflist = []
        for elem in elements:
            if elem.isdigit():
                iflist.append(int(elem))
            elif re.match('^[0-9]+~[0-9]+$', elem):
                s = [int(e) for e in elem.split('~')]
                iflist.extend(range(s[0],s[1]+1))
        return iflist
    
    def as_applycal(self):
        args = {'infile'    : self.infile,
                'calmode'   : 'apply',
                'field'     : self.field,
                'scanlist'  : self.scanlist,
                'iflist'    : self.iflist,
                'pollist'   : self.pollist,
                'applytable': self.applytable,
                'ifmap'     : self.ifmap,
                'interp'    : self.interp,
                'overwrite' : True            }
        
        for key in ('applytable', 'interp'):
            if type(args[key]) is types.StringType:
                args[key] = '\'%s\'' % args[key]
        
        return ('sdcal2(infile=\'{infile}\', calmode=\'{calmode}\', applytable={applytable},  '
                'ifmap={ifmap}, interp={interp}, '
                'scanlist={scanlist}, field=\'{field}\', iflist={iflist}, pollist={pollist}) '
                ''.format(**args))

    @property
    def infile(self):
        vis = self.calto.vis
        antenna = self.calto.antenna
        from asap.scantable import is_scantable
        if is_scantable(vis):
            return vis
        else:
            # must be MS
            # scantable name is <MS_prefix>.<antenna_name>.asap
            s = vis.split('.')
            return '.'.join(s[:-1]+[antenna,'asap'])

    def exists(self):
        for cf in self.calfrom: 
            if not os.path.exists(cf.gaintable):
                return False
        return True

    @property
    def field(self):
        return self.calto.field

    @property
    def scanlist(self):
        return []

    @property
    def iflist(self):
        return SDCalApplication.spw_to_iflist(self.calto.spw)

    @property
    def pollist(self):
        return []

    @property
    def applytable(self):
        l = [cf.gaintable for cf in self.calfrom]
        return l[0] if len(l) is 1 else l

    @property
    def ifmap(self):
        ifmap_ = {}
        for cf in self.calfrom:
            if cf.spwmap is not None:
                for (k,v) in cf.spwmap.items():
                    if ifmap_.has_key(k):
                        ifmap_[k] = ifmap_[k] + v
                    else:
                        ifmap_[k] = v
        return ifmap_
    
    @property
    def interp(self):
        # temporal
        return 'linear,cspline'
        
    def __str__(self):
        return self.as_applycal()
    
    def __repr__(self):
        return 'SDCalApplication(%s, %s)' % (self.calto, self.calfrom)

class SDCalToAdapter(CalToIdAdapter):
    def __init__(self, context, calto):
        super(SDCalToAdapter, self).__init__(context, calto)

    @property
    def antenna(self):
        # return name instead of id
        return [a.name for a in self.ms.get_antenna(self._calto.antenna)]
            
class SDCalFrom(CalFrom):
    CALTYPES = {
        'unknown'      : 0,
        'sky'          : 1,
        'tsys'         : 2,
        'ps'           : 1,
        'otfraster'    : 1,
        'otf'          : 1
    }
    
    @staticmethod
    def _calc_hash(gaintable, gainfield, interp, spwmap, calwt):
        result = 17
        result = 37*result + hash(gaintable)
        result = 37*result + hash(gainfield)
        result = 37*result + hash(interp)
        # since spwmap is dict...
        result = 37*result + hash(tuple(spwmap.keys()))
        # spwmap values are list
        for v in spwmap.values():
            result = 37*result + hash(tuple(v))
        result = 37*result + hash(calwt)
        return result

    def __init__(self, gaintable, gainfield=None, interp=None, spwmap=None,
                 caltype=None):
        super(SDCalFrom,self).__init__(gaintable, gainfield, interp, spwmap)
        #self.caltype = caltype

    def __new__(cls, gaintable=None, gainfield='', interp='linear,linear', 
                spwmap={}, caltype='unknown', calwt=True):
        if gaintable is None:
            raise ValueError, 'gaintable must be specified. Got None'
        
        if type(gainfield) is not types.StringType:
            raise ValueError, 'gainfield must be a string. Got %s' % str(gainfield)

        if type(interp) is not types.StringType:
            raise ValueError, 'interp must be a string. Got %s' % str(interp)

        #if type(spwmap) is types.TupleType:
        #    spwmap = [spw for spw in spwmap]

        #if not isinstance(spwmap, list):
        #    raise ValueError, 'spwmap must be a list. Got %s' % str(spwmap)
        # Flyweight instances should be immutable, so convert spwmap to a
        # tuple. This also makes spwmap hashable for our hash function.
        #spwmap = tuple([o for o in spwmap])
                
        caltype = string.lower(caltype)
        assert caltype in SDCalFrom.CALTYPES

        arg_hash = SDCalFrom._calc_hash(gaintable, gainfield, interp, spwmap, 
                                      calwt)
        
        obj = SDCalFrom._CalFromPool.get(arg_hash, None)
        if not obj:
            LOG.trace('Creating new SDCalFrom(gaintable=\'%s\', '
                      'gainfield=\'%s\', interp=\'%s\', spwmap=%s, '
                      'caltype=\'%s\', calwt=%s)' % 
                (gaintable, gainfield, interp, spwmap, caltype, calwt))
            
            obj = object.__new__(cls)
            obj.__gaintable = gaintable
            obj.__gainfield = gainfield
            obj.__interp = interp
            if spwmap is None:
                obj.__spwmap = {}
            else:
                if not isinstance(spwmap, dict):
                    raise ValueError, 'spwmap must be a dict' 
                obj.__spwmap = spwmap.copy()
            if caltype is None:
                obj.__caltype = 'unknown'
            else:
                obj.__caltype = caltype.lower()
            obj.__calwt = calwt

            LOG.debug('Adding new SDCalFrom to pool: %s' % obj)
            SDCalFrom._CalFromPool[arg_hash] = obj
            LOG.trace('New pool contents: %s' % SDCalFrom._CalFromPool.items())
        else:
            LOG.trace('Reusing existing SDCalFrom(gaintable=\'%s\', '
                      'gainfield=\'%s\', interp=\'%s\', spwmap=\'%s\', '
                      'caltype=\'%s\', calwt=%s)' % 
                (gaintable, gainfield, interp, spwmap, caltype, calwt))
                        
        return obj

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
    
    @property
    def interp(self):
        return self.__interp
    
    @property
    def spwmap(self):
        return self.__spwmap
    
    def __hash__(self):
        return SDCalFrom._calc_hash(self.gaintable, self.gainfield, self.interp,
                                  self.spwmap, self.calwt)

    def __repr__(self):
        return ('SDCalFrom(\'%s\', gainfield=\'%s\', interp=\'%s\', '
                'spwmap=\'%s\', caltype=\'%s\')' % (self.gaintable, 
                self.gainfield, self.interp, self.spwmap, self.caltype))

class SDCalState(CalState):
    def __init__(self, default_factory=_ms_dim):
        super(SDCalState, self).__init__(default_factory)

    def get_caltable(self, caltypes=None):
        if caltypes is None:
            caltypes = SDCalFrom.CALTYPES.keys()

        if type(caltypes) is types.StringType:
            caltypes = (caltypes,)
            
        for c in caltypes:
            assert c in SDCalFrom.CALTYPES

        calfroms = (itertools.chain(*self.merged().values()))
        return set([cf.gaintable for cf in calfroms
                    if cf.caltype in caltypes])

    def as_applycal(self):
        calapps = [SDCalApplication(k,v) for k,v in self.merged().items()]
        return '\n'.join([str(c) for c in calapps])

class SDCalLibrary(CalLibrary):
    def __init__(self, context):
        self._context = context
        self._active = SDCalState()
        self._applied = SDCalState()

    def _add(self, calto, calfroms, calstate):
        if type(calfroms) is not types.ListType:
            calfroms = [calfroms]
        
        calto = SDCalToAdapter(self._context, calto)
        ms_name = calto.ms.name
        
        for spw_id in calto.spw:
            for field_id in calto.field:
                for intent in calto.get_field_intents(field_id, spw_id):
                    for antenna_id in calto.antenna:
                        for cf in calfroms:
                            cf_copy = copy.deepcopy(cf)
                            calstate[ms_name][spw_id][field_id][intent][antenna_id].append(cf_copy)

        LOG.trace('Calstate after _add:\n'
                  '%s' % calstate.as_applycal())

    def _export(self, calstate, filename=None):
        filename = self._calc_filename(filename)

        calapps = [SDCalApplication(k,v) for k,v in calstate.merged().items()]
                   
        with open(filename, 'w') as export_file:
            for ca in calapps: 
                export_file.write(ca.as_applycal())
                export_file.write('\n')

    def _remove(self, calto, calfrom, calstate):
        if type(calfrom) is not types.ListType:
            calfrom = [calfrom]
        
        calto = SDCalToAdapter(self._context, calto)
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
                                LOG.debug('%s not found in calstate' % c)

        LOG.trace('Calstate after _remove:\n'
                  '%s' % calstate.as_applycal())

    def get_calstate(self, calto, hide_null=True):
        # wrap the text-only CalTo in a CalToIdAdapter, which will parse the
        # CalTo properties and give us the appropriate subtable IDs to iterate
        # over 
        id_resolver = SDCalToAdapter(self._context, calto)        
        ms_name = id_resolver.ms.name

        result = SDCalState()
        for spw_id in id_resolver.spw:
            for field_id in id_resolver.field:
                for intent in id_resolver.get_field_intents(field_id, spw_id):
                    for antenna_id in id_resolver.antenna:
                        # perhaps this should be deepcopied. Do we trust all 
                        # clients using this method?
                        v = self._active[ms_name][spw_id][field_id][intent][antenna_id][:]
                        result[ms_name][spw_id][field_id][intent][antenna_id] = v

        return result

    def import_state(self, filename=None):
        filename = self._calc_filename(filename)

        calapps = []
        with open(filename, 'r') as import_file:
            for line in [l for l in import_file if l.startswith('sdcal2(')]:
                calapp = SDCalApplication.from_export(line)
                calapp.calto = self._edit_calto(calapp.calto)
                calapps.append(calapp)

        self._active = SDCalState()
        for calapp in calapps:
            LOG.debug('Adding %s' % calapp)        
            self.add(calapp.calto, calapp.calfrom)

        LOG.info('Calibration state after import:\n'
                 '%s' % self.active.as_applycal())

    def _edit_calto(self, calto):
        from asap.scantable import is_scantable
        if is_scantable(calto.vis):
            s = asap.scantable(calto.vis,average=False)
            antenna_name = s.get_antennaname()
            vis = calto.vis.replace(antenna_name+'.asap','ms')
            return CalTo(vis,
                         antenna=antenna_name,
                         field=calto.field,
                         intent=calto.intent,
                         spw=calto.spw)
        else:
            return calto
