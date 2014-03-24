import unittest
import inspect
import re
import abc

from taskinit import casalog

def skipUnlessHasParam(param):
    def wrapper(func):
        import functools
        @functools.wraps(func)
        def _wrapper(*args, **kwargs):
            task = args[0].task
            task_args = inspect.getargs(task.func_code).args
            if isinstance(param, str):
                condition = param in task_args
                reason = '%s doesn\'t have parameter \'%s\''%(task.__name__, param)
            else:
                # should be a list
                condition = all([p in task_args for p in param])
                reason = '%s doesn\'t have parameter %s'%(task.__name__, map(lambda x: '\'' + str(x) + '\'', param))
            return unittest.skipUnless(condition, reason)(func)(*args, **kwargs)
        return _wrapper
    return wrapper

def skipIfNoChannelSelection(func):
    import functools
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        task = args[0].task
        condition = args[0].spw_channel_selection
        reason = '%s doens\'nt accept channel selection on parameter spw'%(task.__name__)
        return unittest.skipUnless(condition, reason)(func)(*args, **kwargs)
    return wrapper

class SelectionSyntaxTest(unittest.TestCase):
    """
    Base class for data selection syntax unit test.

    All the tests are defined in this class but all fail by
    default. Subclasses must override necessary tests.
    Unnecessary tests will be automatically skipped.

    In the test implementation, it is recommended to use
    run_task method instead of running task directly. It
    will check whether the test is properly implemented in
    terms of its intent.
    """
    __metaclass__ = abc.ABCMeta
        
    infile = None

    @abc.abstractproperty
    def task(self):
        """
        task is an abstract attribute that must be defined
        at each subclass. It must point task as a function
        object.
        """
        return None

    @abc.abstractproperty
    def spw_channel_selection(self):
        """
        spw_channel_selection is an abstract attribute that must be
        defined at each subclass. It must return True if target
        task accepts channel_selection in spw and return False
        otherwise.
        """
        return None

    ### field selection syntax test ###
    @skipUnlessHasParam('field')
    def test_field_value_default(self):
        """test_field_value_default: Test default value for field"""
        self._default_test()
        
    @skipUnlessHasParam('field')
    def test_field_id_exact(self):
        """test_field_id_exact: Test field selection by id"""
        self._default_test()
        
    @skipUnlessHasParam('field')
    def test_field_id_lt(self):
        """test_field_id_lt: Test field selection by id (<N)"""
        self._default_test()

    @skipUnlessHasParam('field')
    def test_field_id_gt(self):
        """test_field_id_gt: Test field selection by id (>N)"""
        self._default_test()

    @skipUnlessHasParam('field')
    def test_field_id_range(self):
        """test_field_id_range: Test field selection by id ('N~M')"""
        self._default_test()

    @skipUnlessHasParam('field')
    def test_field_id_list(self):
        """test_field_id_list: Test field selection by id ('N,M')"""
        self._default_test()

    @skipUnlessHasParam('field')
    def test_field_id_exprlist(self):
        """test_field_id_exprlist: Test field selection by id ('EXPR0,EXPR1')"""
        self._default_test()

    @skipUnlessHasParam('field')
    def test_field_value_exact(self):
        """test_field_value_exact: Test field selection by name"""
        self._default_test()

    @skipUnlessHasParam('field')
    def test_field_value_pattern(self):
        """test_field_value_pattern: Test field selection by pattern match"""
        self._default_test()

    @skipUnlessHasParam('field')
    def test_field_value_list(self):
        """test_field_value_list: Test field selection by name list"""
        self._default_test()

    @skipUnlessHasParam('field')
    def test_field_mix_exprlist(self):
        """test_field_mix_list: Test field selection by name and id"""
        self._default_test()

    ### spw selection syntax test ###
    @skipUnlessHasParam('spw')
    def test_spw_id_default(self):
        """test_spw_id_default: Test default value for spw"""
        self._default_test()
        
    @skipUnlessHasParam('spw')
    def test_spw_id_exact(self):
        """test_spw_id_exact: Test spw selection by id ('N')"""
        self._default_test()
        
    @skipUnlessHasParam('spw')
    def test_spw_id_lt(self):
        """test_spw_id_lt: Test spw selection by id ('<N')"""
        self._default_test()

    @skipUnlessHasParam('spw')
    def test_spw_id_gt(self):
        """test_spw_id_lt: Test spw selection by id ('>N')"""
        self._default_test()

    @skipUnlessHasParam('spw')
    def test_spw_id_range(self):
        """test_spw_id_range: Test spw selection by id ('N~M')"""
        self._default_test()

    @skipUnlessHasParam('spw')
    def test_spw_id_list(self):
        """test_spw_id_list: Test spw selection by id ('N,M')"""
        self._default_test()

    @skipUnlessHasParam('spw')
    def test_spw_id_exprlist(self):
        """test_spw_id_exprlist: Test spw selection by id ('EXP0,EXP1')"""
        self._default_test()

    @skipUnlessHasParam('spw')
    def test_spw_id_pattern(self):
        """test_spw_id_pattern: Test spw selection by wildcard"""

    @skipUnlessHasParam('spw')
    def test_spw_value_frequency(self):
        """test_spw_value_frequency: Test spw selection by frequency range ('FREQ0~FREQ1')"""
        self._default_test()

    @skipUnlessHasParam('spw')
    def test_spw_value_velocity(self):
        """test_spw_value_velocity: Test spw selection by velocity range ('VEL0~VEL1')"""
        self._default_test()

    @skipUnlessHasParam('spw')
    def test_spw_mix_exprlist(self):
        """test_spw_mix_exprlist: Test spw selection by id and frequency/velocity range"""
        self._default_test()

    ### spw (channel) selection syntax test ###
    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_default_channel(self):
        """test_spw_id_default_channel: Test spw selection with channel range (':CH0~CH1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_default_frequency(self):
        """test_spw_id_default_frequency: Test spw selection with channel range (':FREQ0~FREQ1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_default_velocity(self):
        """test_spw_id_default_velocity: Test spw selection with channel range (':VEL0~VEL1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_default_list(self):
        """test_spw_id_default_list: Test spw selection with multiple channel range (':CH0~CH1;CH2~CH3')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_exact_channel(self):
        """test_spw_id_exact_channel: Test spw selection with channel range ('N:CH0~CH1')"""
        self._default_test()
        
    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_exact_frequency(self):
        """test_spw_id_exact_frequency: Test spw selection with channel range ('N:FREQ0~FREQ1')"""
        self._default_test()
        
    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_exact_velocity(self):
        """test_spw_id_exact_velocity: Test spw selection with channel range ('N:VEL0~VEL1')"""
        self._default_test()
        
    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_exact_list(self):
        """test_spw_id_exact_list: Test spw selection with channel range ('N:CH0~CH1;CH2~CH3')"""
        self._default_test()
        
    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_pattern_channel(self):
        """test_spw_id_pattern_channel: Test spw selection with channel range ('*:CH0~CH1')"""
        self._default_test()
        
    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_pattern_frequency(self):
        """test_spw_id_pattern_frequency: Test spw selection with channel range ('*:FREQ0~FREQ1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_pattern_velocity(self):
        """test_spw_id_pattern_velocity: Test spw selection with channel range ('*:VEL0~VEL1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_pattern_list(self):
        """test_spw_id_pattern_list: Test spw selection with channel range ('*:CH0~CH1;CH2~CH3')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_value_frequency_channel(self):
        """test_spw_value_frequency_channel: Test spw selection with channel range ('FREQ0~FREQ1:CH0~CH1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_value_frequency_frequency(self):
        """test_spw_value_frequency_frequency: Test spw selection with channel range ('FREQ0~FREQ1:FREQ2~FREQ3')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_value_frequency_velocity(self):
        """test_spw_value_frequency_velocity: Test spw selection with channel range ('FREQ0~FREQ1:VEL0~VEL1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_value_frequency_list(self):
        """test_spw_value_frequency_list: Test spw selection with channel range ('FREQ0~FREQ1:CH0~CH1;CH2~CH3')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_value_velocity_channel(self):
        """test_spw_value_velocity_channel: Test spw selection with channel range ('VEL0~VEL1:CH0~CH1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_value_velocity_frequency(self):
        """test_spw_value_velocity_frequency: Test spw selection with channel range ('VEL0~VEL1:FREQ0~FREQ1')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_value_velocity_velocity(self):
        """test_spw_value_velocity_velocity: Test spw selection with channel range ('VEL0~VEL1:VEL2~VEL3')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_value_velocity_list(self):
        """test_spw_value_velocity_list: Test spw selection with channel range ('VEL0~VEL1:CH0~CH1;CH2~CH3')"""
        self._default_test()

    @skipIfNoChannelSelection
    @skipUnlessHasParam('spw')
    def test_spw_id_list_channel(self):
        """test_spw_id_list_channel: Test spw selection with channnel range ('ID0:CH0~CH1,ID1:CH2~CH3')"""
        self._default_test()
        
    ### timerange selection syntax test ###
    @skipUnlessHasParam('timerange')
    def test_timerange_value_default(self):
        """test_timerange_value_default: Test default value for timerange"""
        self._default_test()
        
    @skipUnlessHasParam('timerange')
    def test_timerange_value_exact(self):
        """test_timerange_value_exact: Test timerange selection by syntax 'T0'"""
        self._default_test()

    @skipUnlessHasParam('timerange')
    def test_timerange_value_range(self):
        """test_timerange_value_range: Test timerange selection by syntax 'T0~T1'"""
        self._default_test()

    @skipUnlessHasParam('timerange')
    def test_timerange_value_lt(self):
        """test_timerange_value_lt: Test timerange selection by syntax '<T0'"""
        self._default_test()

    @skipUnlessHasParam('timerange')
    def test_timerange_value_gt(self):
        """test_timerange_value_gt: Test timerange selection by syntax '>T0'"""
        self._default_test()

    @skipUnlessHasParam('timerange')
    def test_timerange_value_interval(self):
        """test_timerange_value_interval: Test timerange selection by syntax 'T0+dT'"""
        self._default_test()

    ### scan selection syntax test ###
    @skipUnlessHasParam('scan')
    def test_scan_id_default(self):
        """test_scan_id_default: Test default value for scan"""
        self._default_test()
        
    @skipUnlessHasParam('scan')
    def test_scan_id_exact(self):
        """test_scan_id_exact: Test scan selection by id ('N')"""
        self._default_test()
        
    @skipUnlessHasParam('scan')
    def test_scan_id_lt(self):
        """test_scan_id_lt: Test scan selection by id ('<N')"""
        self._default_test()

    @skipUnlessHasParam('scan')
    def test_scan_id_gt(self):
        """test_scan_id_gt: Test scan selection by id ('>N')"""
        self._default_test()

    @skipUnlessHasParam('scan')
    def test_scan_id_range(self):
        """test_scan_id_range: Test scan selection by id ('N~M')"""
        self._default_test()

    @skipUnlessHasParam('scan')
    def test_scan_id_list(self):
        """test_scan_id_list: Test scan selection by id ('N,M')"""
        self._default_test()

    @skipUnlessHasParam('scan')
    def test_scan_id_exprlist(self):
        """test_scan_id_exprlist: Test scan selection by id ('EXP0,EXP1')"""
        self._default_test()

    ### pol selection syntax test ###
    @skipUnlessHasParam('pol')
    def test_pol_id_default(self):
        """test_pol_id_default: Test default value for pol"""
        self._default_test()
        
    @skipUnlessHasParam('pol')
    def test_pol_id_exact(self):
        """test_pol_id_exact: Test pol selection by id ('N')"""
        self._default_test()
        
    @skipUnlessHasParam('pol')
    def test_pol_id_lt(self):
        """test_pol_id_lt: Test pol selection by id ('<N')"""
        self._default_test()

    @skipUnlessHasParam('pol')
    def test_pol_id_gt(self):
        """test_pol_id_gt: Test pol selection by id ('>N')"""
        self._default_test()

    @skipUnlessHasParam('pol')
    def test_pol_id_range(self):
        """test_pol_id_range: Test pol selection by id ('N~M')"""
        self._default_test()

    @skipUnlessHasParam('pol')
    def test_pol_id_list(self):
        """test_pol_id_list: Test pol selection by id ('N,M')"""
        self._default_test()

    @skipUnlessHasParam('pol')
    def test_pol_id_exprlist(self):
        """test_pol_id_exprlist: Test pol selection by id ('EXP0,EXP1')"""
        self._default_test()

    ### beam selection syntax test ###
    @skipUnlessHasParam('beam')
    def test_beam_id_default(self):
        """test_beam_id_default: Test default value for beam"""
        self._default_test()
        
    @skipUnlessHasParam('beam')
    def test_beam_id_exact(self):
        """test_beam_id_exact: Test beam selection by id ('N')"""
        self._default_test()
        
    @skipUnlessHasParam('beam')
    def test_beam_id_lt(self):
        """test_beam_id_lt: Test beam selection by id ('<N')"""
        self._default_test()

    @skipUnlessHasParam('beam')
    def test_beam_id_gt(self):
        """test_beam_id_gt: Test beam selection by id ('>N')"""
        self._default_test()

    @skipUnlessHasParam('beam')
    def test_beam_id_range(self):
        """test_beam_id_range: Test beam selection by id ('N~M')"""
        self._default_test()

    @skipUnlessHasParam('beam')
    def test_beam_id_list(self):
        """test_beam_id_list: Test beam selection by id ('N,M')"""
        self._default_test()

    @skipUnlessHasParam('beam')
    def test_beam_id_exprlist(self):
        """test_beam_exprlist: Test beam selection by id ('EXP0,EXP1')"""
        self._default_test()

    def _default_test(self):
        """
        This is default test that always fails.
        """
        #func_name = inspect.stack()[1][3]
        func_name = self._get_test_name()
        self.fail('You have to implement %s!'%(func_name))

    def _get_test_name(self, regular_test=True):
        stack = inspect.stack()
        func_name_list = [s[3] for s in stack]
        index = 0
        #print index
        if regular_test:
            pattern = '^test_[a-z]+_(id|value|mix)_[a-z]+(_[a-z]+)?$'
        else:
            pattern = '^test.+$'
        while index < len(func_name_list) \
                and re.match(pattern, func_name_list[index]) is None:
            index += 1
        #print index
        if index < len(func_name_list):
            test_name = func_name_list[index]
            return test_name
        else:
            self.fail('Failed to get test name')

    def _get_pattern(self, ptype, psubtype, channel_selection=None):
        pattern = '.+$'
        # selection string mixed with id and string
        if psubtype == 'exprlist':
            pattern = '^.+(\,.+)+$'
        else:
            if ptype == 'id':
                if psubtype == 'exact':
                    pattern = '^[0-9]+$'
                elif psubtype == 'lt':
                    pattern = '^<[0-9]+$'
                elif psubtype == 'gt':
                    pattern = '^>[0-9]+$'
                elif psubtype == 'range':
                    pattern = '^[0-9]+~[0-9]+$'
                elif psubtype == 'list':
                    pattern = '^[0-9]+(\,[0-9]+)+$'
            elif ptype == 'value':
                if psubtype == 'exact':
                    pattern = '^[^*]+$'
                elif psubtype == 'pattern':
                    pattern = '^.*(\*.*)+$'
                elif psubtype == 'range':
                    pattern = '^[^*]+~[^*]+$'
                elif psubtype == 'frequency':
                    pattern = '^[0-9.]+([eE]-?[0-9]+)?([kMG]?Hz)?~[0-9.]+([eE]-?[0-9]+)?[kMG]?Hz$'
                elif psubtype == 'velocity':
                    pattern = '^-?[0-9.]+([eE]-?[0-9]+)?(k?m/s)?~-?[0-9.]+([eE]-?[0-9]+)?k?m/s$'
                elif psubtype == 'lt':
                    pattern = '^<[^*]+$'
                elif psubtype == 'gt':
                    pattern = '^>[^*]+$'
                elif psubtype == 'interval':
                    pattern = '^[^*]+\+[^*]+$'
        if channel_selection is not None:
            if psubtype == 'list':
                pattern = pattern.replace('[0-9]','.')
            elif psubtype != 'default':
                pattern = pattern.replace('$', ':.+~.+$')
        return pattern

    def run_task(self, **kwargs):
        func_name = self._get_test_name()
        casalog.post('func_name=\'%s\''%(func_name), priority='DEBUG')
        fragments = func_name.split('_')
        func_name_elements = func_name.split('_')
        param, ptype, psubtype = func_name_elements[1:4]
        if len(func_name_elements) > 4:
            channel_selection = func_name_elements[4]
        else:
            channel_selection = None
        casalog.post('%s: channel_selection=%s'%(func_name, channel_selection))

        # Test 1: target parameter must be set unless the test is 'default'
        #         otherwise test fails            
        if psubtype != 'default' or channel_selection is not None:
            self.assertIn(param, kwargs.keys(),
                          msg='parameter \'%s\' must be specified'%(param))

        # Test 2: appropriate value must be set for target parameter
        #         otherwise test fails
        if psubtype == 'default' and channel_selection is None:
            self.assertTrue((param not in kwargs.keys()) or len(kwargs[param]) == 0,
                            msg='parameter \'%s\' must be default'%(param))
        else:
            param_value = kwargs[param]
            pattern = self._get_pattern(ptype, psubtype, channel_selection)
            casalog.post('%s: pattern=\'%s\', param_value=\'%s\''%(func_name, pattern, param_value),
                         priority='DEBUG')
            if pattern is not None:
                self.assertNotEqual(re.match(pattern, param_value), None,
                                    msg='parameter \'%s\' gets invalid value \'%s\''%(param, param_value))
        
        # execute task here
        d = kwargs.copy()
        if not d.has_key('infile'):
            d['infile'] = self.infile
        #task_string = '%s(%s)'%(self.task.__name__, ','.join(['%s=\'%s\''%(k,v) for (k,v) in d.items()]))
        #print 'executing %s'%(task_string)
        result = self.task(**d)

        # return result
        return result
            
