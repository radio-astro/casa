from __future__ import absolute_import
import collections
import copy
import itertools
import os
import string
import types

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


CalToArgs = collections.namedtuple('CalToArgs',
                                   ['vis','spw','field','antenna'])


class CalApplication(object):
    def __init__(self, calto, calfrom):
        self.calto = calto
        if type(calfrom) is not types.ListType:
            calfrom = [calfrom]
        self.calfrom = calfrom

    @staticmethod
    def from_export(s):
        d = eval(string.replace(s, 'applycal(', 'dict('))
        calto = CalTo(vis=d['vis'], field=d['field'], spw=d['spw'], 
                      antenna=d['antenna'])
        
        # wrap these values in a list if they are single valued, 
        # eg. 'm31' -> ['m31']
        for key in ('gainfield', 'gaintable', 'interp'):
            if type(d[key]) is types.StringType:
                d[key] = [d[key]]

        # do the same for spwmap. A bit more complicated, as a single valued
        # spwmap is a list of integers, or may not have any values at all.                
        try:            
            if type(d['spwmap'][0]) is not types.ListType:
                d['spwmap'] = [d['spwmap']]
        except IndexError, _:
            d['spwmap'] = [d['spwmap']]

        zipped = zip(d['gaintable'], d['gainfield'], d['interp'], d['spwmap'])

        calfroms = []
        for (gaintable, gainfield, interp, spwmap) in zipped:
            with casatools.TableReader(gaintable) as caltable:
                viscal = caltable.getkeyword('VisCal')
            
            calfrom = CalFrom(gaintable, gainfield=gainfield, interp=interp, 
                              spwmap=spwmap) 
            calfrom.caltype = CalFrom.get_caltype_for_viscal(viscal) 
            LOG.trace('Marking caltable \'%s\' as caltype \'%s\''
                      '' % (gaintable, calfrom.caltype))

            calfroms.append(calfrom)
        
        return CalApplication(calto, calfroms)

    def as_applycal(self):
        args = {'vis'       : self.vis,
                'field'     : self.field,
                'spw'       : self.spw,
                'antenna'   : self.antenna,
                'gaintable' : self.gaintable,
                'gainfield' : self.gainfield,
                'spwmap'    : self.spwmap,
                'interp'    : self.interp    }
        
        for key in ('gaintable', 'gainfield', 'spwmap', 'interp'):
            if type(args[key]) is types.StringType:
                args[key] = '\'%s\'' % args[key]
        
        return ('applycal(vis=\'{vis}\', field=\'{field}\', spw=\'{spw}\', '
                'antenna=\'{antenna}\', gaintable={gaintable}, '
                'gainfield={gainfield}, spwmap={spwmap}, interp={interp})'
                ''.format(**args))

    @property
    def antenna(self):
        return self.calto.antenna
    
    def exists(self):
        for cf in self.calfrom: 
            if not os.path.exists(cf.gaintable):
                return False
        return True

    @property
    def field(self):
        return self.calto.field

    @property
    def gainfield(self):
        l = [cf.gainfield for cf in self.calfrom]
        return l[0] if len(l) is 1 else l

    @property
    def gaintable(self):
        l = [cf.gaintable for cf in self.calfrom]
        return l[0] if len(l) is 1 else l
    
    @property
    def interp(self):
        l = [cf.interp for cf in self.calfrom]
        return l[0] if len(l) is 1 else l
        
    @property
    def spw(self):
        return self.calto.spw
    
    @property
    def spwmap(self):
        l = [cf.spwmap for cf in self.calfrom]
        return l[0] if len(l) is 1 else l 

    @property
    def vis(self):
        return self.calto.vis

    def __str__(self):
        return self.as_applycal()
    
    def __repr__(self):
        return 'CalApplication(%s, %s)' % (self.calto, self.calfrom)


class CalTo(object):
    def __init__(self, vis=None, field='', spw='', antenna='', obs_id=None):
        self.vis = vis
        self.field = field
        self.spw = spw
        self.antenna = antenna
        self.obs_id = obs_id

    @property
    def antenna(self):
        return self._antenna
    
    @antenna.setter
    def antenna(self, value=''):
        value = value if value is not None else ''
        self._antenna = utils.find_ranges(str(value))

    @property
    def field(self):
        return self._field
    
    @field.setter
    def field(self, value=''):
        value = value if value is not None else ''
        self._field = str(value)

    @property
    def spw(self):
        return self._spw
    
    @spw.setter
    def spw(self, value=''):
        value = value if value is not None else ''
        self._spw = utils.find_ranges(str(value))

    @property
    def vis(self):
        return self._vis
    
    @vis.setter
    def vis(self, value=None):
        self._vis = str(value)

    def __repr__(self):
        return ('CalTo(vis=\'%s\', field=\'%s\', spw=\'%s\', antenna=\'%s\','
                'obs_id=%s)' % (self.vis, self.field, self.spw, self.antenna,
                                self.obs_id))


class CalFrom(object):
    CALTYPES = {
        'unknown'      : 0,
        'gaincal'      : 1,
        'bandpass'     : 2,
        'tsys'         : 3,
        'wvr'          : 4,
        'polarization' : 5,
        'antpos'       : 6, 
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
    
    def __init__(self, gaintable, gainfield=None, interp=None, spwmap=None,
                 caltype=None):
        self.gaintable = gaintable
        self.gainfield = gainfield
        self.interp = interp
        self.spwmap = spwmap
        self.caltype = caltype

    @property
    def caltype(self):
        return self._caltype
    
    @caltype.setter
    def caltype(self, value):
        if value is None:
            value = 'unknown'
        value = string.lower(value)
        assert value in CalFrom.CALTYPES
        self._caltype = value

    @property
    def gainfield(self):
        return self._gainfield
    
    @gainfield.setter
    def gainfield(self, value):
        if value is None:
            value = ''
        if type(value) is not types.StringType:
            raise ValueError, 'gainfield must be a string'
        self._gainfield = value

    @staticmethod
    def get_caltype_for_viscal(viscal):
        s = string.upper(viscal)
        for caltype, viscals in CalFrom.CALTYPE_TO_VISCAL.items():
            if s in viscals:
                return caltype
        return 'unknown'

    @property
    def interp(self):
        return self._interp
    
    @interp.setter
    def interp(self, value):
        if value is None:
            value = 'linear,linear'
        if type(value) is not types.StringType:
            raise ValueError, 'interp must be a string'
        self._interp = value
        
    @property
    def spwmap(self):
        return self._spwmap
    
    @spwmap.setter
    def spwmap(self, value):
        if value is None:
            value = []
        if not isinstance(value, list):
            raise ValueError, 'spwmap must be a list'
        self._spwmap = value[:]

    def __eq__(self, other):
        return (self.gaintable == other.gaintable and
                self.gainfield == other.gainfield and
                self.interp    == other.interp    and
                self.spwmap    == other.spwmap)
    
    def __hash__(self):
        result = 17
        result = 37*result + hash(self.gaintable)
        result = 37*result + hash(self.gainfield)
        result = 37*result + hash(self.interp)
        result = 37*result + hash(tuple([o for o in self.spwmap]))
        return result

    def __repr__(self):
        return ('CalFrom(\'%s\', gainfield=\'%s\', interp=\'%s\', '
                'spwmap=\'%s\', caltype=\'%s\')' % (self.gaintable, 
                self.gainfield, self.interp, self.spwmap, self.caltype))


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
    def ms(self):
        return self._context.observing_run.get_ms(self._calto.vis)

    @property
    def spw(self):
        return [spw.id for spw in self.ms.get_spectral_windows(
                self._calto.spw, science_windows_only=False)]

    @property
    def obs_id(self):
        return self._calto.obs_id

    def __repr__(self):
        return ('CalToIdAdapter(ms=\'%s\', field=\'%s\', spw=\'%s\', ' 
                'antenna=\'%s\', obs_id=%s)' % (self.ms.name, self.field,
                                                self.spw, self.antenna, 
                                                self.obs_id))


# CalState extends defaultdict. For defaultdicts to be pickleable, their 
# default factories must be defined at the module level.
def _antenna_dim(): return []
def _field_dim(): return collections.defaultdict(_antenna_dim)
def _spw_dim(): return collections.defaultdict(_field_dim)
def _ms_dim(): return collections.defaultdict(_spw_dim)


class CalState(collections.defaultdict):
    def __init__(self, default_factory=_ms_dim):
        super(CalState, self).__init__(default_factory)

    def get_caltable(self, caltypes=None):
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

    def merged(self):
        hashes = {}
        flattened = self._flattened()
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
            return dict([(k, v) for (k, v) in flat.items() if v is not []])
        return flat 

    def as_applycal(self):
        calapps = [CalApplication(k,v) for k,v in self.merged().items()]
        return '\n'.join([str(c) for c in calapps])

    def __str__(self):
        return self.as_applycal()

    def __repr__(self):
        return self.as_applycal()
#        return 'CalState(%s)' % repr(CalState.dictify(self.merged))

        
class CalLibrary(object):
    def __init__(self, context):
        self._context = context
        self._active = CalState()
        self._applied = CalState()

    def _add(self, calto, calfroms, calstate):
        if type(calfroms) is not types.ListType:
            calfroms = [calfroms]
        
        calto = CalToIdAdapter(self._context, calto)
        ms_name = calto.ms.name
        
        for spw_id in calto.spw:
            for field_id in calto.field:
                for antenna_id in calto.antenna:
                    for cf in calfroms:
                        cf_copy = copy.deepcopy(cf)
                        calstate[ms_name][spw_id][field_id][antenna_id].append(cf_copy)

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
                for antenna_id in calto.antenna:
                    current = calstate[ms_name][spw_id][field_id][antenna_id]
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
        return self._active

    @property
    def applied(self):
        return self._applied

    def export(self, filename=None):
        filename = self._calc_filename(filename)
        LOG.info('Exporting current calibration state to %s' % filename)
        self._export(self._active, filename)

    def export_applied(self, filename=None):
        filename = self._calc_filename(filename)
        LOG.info('Exporting applied calibration state to %s' % filename)
        self._export(self._applied, filename)

    def get_calstate(self, calto, hide_null=True):
        # wrap the text-only CalTo in a CalToIdAdapter, which will parse the
        # CalTo properties and give us the appropriate subtable IDs to iterate
        # over 
        id_resolver = CalToIdAdapter(self._context, calto)        
        ms_name = id_resolver.ms.name

        result = CalState()
        for spw_id in id_resolver.spw:
            for field_id in id_resolver.field:
                for antenna_id in id_resolver.antenna:
                    # perhaps this should be deepcopied. Do we trust all 
                    # clients using this method?
                    v = self._active[ms_name][spw_id][field_id][antenna_id][:]
                    result[ms_name][spw_id][field_id][antenna_id] = v

        return result

    def import_state(self, filename=None):
        filename = self._calc_filename(filename)

        LOG.info('Importing calibration state from %s' % filename)
        calapps = []
        with open(filename, 'r') as import_file:
            for line in [l for l in import_file if l.startswith('applycal(')]:
                calapp = CalApplication.from_export(line)                
                calapps.append(calapp)

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



def _gen_hash(o):
    """
    Makes a hash from a dictionary, list, tuple or set to any level, that 
    contains only other hashable types (including any lists, tuples, sets,
    and dictionaries).
    """
    LOG.trace('_gen_hash(%s)' % str(o))
    if isinstance(o, set) or isinstance(o, tuple) or isinstance(o, list):        
        return tuple([_gen_hash(e) for e in o])    
    
    elif not isinstance(o, dict):
        h = hash(o)
        LOG.trace('Hash: %s=%s' % (o, h))
        return hash(o)
    
    new_o = copy.deepcopy(o)
    for k, v in new_o.items():
        new_o[k] = _gen_hash(v)
    
    return hash(tuple(frozenset(new_o.items())))
