import abc
import re

import pipeline.infrastructure.api as api
import pipeline.infrastructure.filenamer as filenamer


def _truncate_floats(s, precision=3):
    """
    Return a copy of the input string with all floating point numbers
    truncated to the given precision.

    Example:
    truncate_float('a,2,123.456789', 3) => 'a,2,123.456'

    :param s: the string to modify
    :param precision: the maximum floating point precision
    :return: a string
    """
    # we need fixed-width terms to so a positive look-behind search,
    # so instead of finding the offending digits and removing them,
    # we have to capture the good digits and cut off the remainder
    # during replacement.
    pattern = '(\d+\.\d{%s})\d+' % precision
    return re.sub(pattern, '\\1', s)


class CaltableNamer(api.Heuristic):
    """
    CaltableNamer is the base class for heuristics that name calibration
    tables. The execution flow is largely the same for all naming heuristics,
    so it provides a common implementation using the Template design pattern,
    providing a stub method where subclasses may customise the naming class
    as required.
    """

    def calculate(self, output_dir='', stage=None, **task_args):
        namer = self.get_namer()
        namer.output_dir(output_dir)
        namer.asdm(task_args['vis'])

        if stage:
            namer.stage(stage)

        self.customise(namer, task_args)

        return namer.get_filename(True)

    @abc.abstractmethod
    def get_namer(self):
        """
        Get a Filenamer instance appropriate to this class.

        This method must be implemented by all extending classes.

        :return: a Filenamer instance appropriate to the extending class.
        """
        raise NotImplementedError

    def customise(self, namer, task_args):
        """
        Set any class-specific filenaming attributes on the given file namer.

        This function gives subclasses an opportunity to modify the Filenamer
        object before the filename is returned.

        :param namer: the namer to customise
        :param task_args: the dict of extra arguments given to the file namer
        :return:
        """
        pass


class AmpCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.AmplitudeCalibrationTable()


class AntposCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.AntposCalibrationTable()


class UVcontCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.UVcontCalibrationTable()

    def customise(self, namer, task_args):
        namer.source(task_args.get('source', None))


class BandpassCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.BandpassCalibrationTable()

    def customise(self, namer, task_args):
        namer.spectral_window(task_args.get('spw', None))
        solint = task_args.get('solint', None)
        namer.solint(_truncate_floats(solint, 3))

        if 'bandtype' in task_args:
            if task_args['bandtype'] == 'B':
                namer.channel_fit()
            if task_args['bandtype'] == 'BPOLY':
                namer.poly_fit()

        if 'method' in task_args:
            namer.method(task_args['method'])


class DelayCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.DelayCalibrationTable()


class PolCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.InstrumentPolCalibrationTable()


class FluxCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.FluxCalibrationTable()


class GaincalCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.GainCalibrationTable()

    def customise(self, namer, task_args):
        namer.spectral_window_nochan(task_args.get('spw', None))
        if 'solint' in task_args:
            # convert integer solints to str for truncation
            solint = str(task_args['solint'])
            namer.solint(_truncate_floats(solint, 3))

        if 'gaintype' in task_args:
            if task_args['gaintype'] == 'GSPLINE':
                namer.spline_fit()

        if 'calmode' in task_args:
            if task_args['calmode'] == 'a':
                namer.amplitude_only_gain_cal()
            if task_args['calmode'] == 'p':
                namer.phase_only_gain_cal()

        if 'method' in task_args:
            namer.method(task_args['method'])


class GainCurvestable(CaltableNamer):
    def get_namer(self):
        return filenamer.GainCurvesCalibrationTable()


class OpCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.OpCalibrationTable()


class RqCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.RqCalibrationTable()


class SwpowCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.SwpowCalibrationTable()


class TecMapstable(CaltableNamer):
    def get_namer(self):
        return filenamer.TecMapsCalibrationTable()


class TsysCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.TsysCalibrationTable()


class WvrgCaltable(CaltableNamer):
    def get_namer(self):
        return filenamer.WvrgCalibrationTable()

    def customise(self, namer, task_args):
        namer.smooth(task_args.get('smooth', None))


class XYf0Caltable(CaltableNamer):
    def get_namer(self):
        return filenamer.XYf0CalibrationTable()
