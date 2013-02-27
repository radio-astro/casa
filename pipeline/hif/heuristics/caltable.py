import pipeline.infrastructure.api as api
import pipeline.infrastructure.filenamer as filenamer

class BandpassCaltable(api.Heuristic):
    def calculate(self, output_dir='', stage=None, method=None, **task_args):
        namer = filenamer.BandpassCalibrationTable()
        namer.output_dir(output_dir)

        namer.asdm(task_args['vis'])
        namer.spectral_window(task_args.get('spw', None))
        namer.solint(task_args.get('solint', None))

        if 'bandtype' in task_args:
            if task_args['bandtype'] == 'B':
                namer.channel_fit()
            if task_args['bandtype'] == 'BPOLY':
                namer.poly_fit()
                
        if method:
            namer.method(method)

        if stage:
            namer.stage(stage)
        
        return namer.get_filename(True)


class GaincalCaltable(api.Heuristic):
    def calculate(self, output_dir='', stage=None, method=None, **task_args):
        namer = filenamer.GainCalibrationTable()
        namer.output_dir(output_dir)

        namer.asdm(task_args['vis'])
        #namer.spectral_window(task_args.get('spw', None))
        namer.spectral_window_nochan(task_args.get('spw', None))
        namer.solint(task_args.get('solint', None))

        if 'gaintype' in task_args:
            if task_args['gaintype'] == 'GSPLINE':
                namer.spline_fit()

        if 'calmode' in task_args:
            if task_args['calmode'] == 'a':
                namer.amplitude_only_gain_cal()
            if task_args['calmode'] == 'p':
                namer.phase_only_gain_cal()

        if method:
            namer.method(method)

        if stage:
            namer.stage(stage)

        return namer.get_filename(True)


class FluxCaltable(api.Heuristic):
    def calculate(self, output_dir='', stage=None, **task_args):
        namer = filenamer.FluxCalibrationTable()
        namer.output_dir(output_dir)
        namer.asdm(task_args['vis'])

        if stage:
            namer.stage(stage)

        return namer.get_filename(True)


class AntposCaltable(api.Heuristic):
    def calculate(self, output_dir='', stage=None, **task_args):
        namer = filenamer.AntposCalibrationTable()
        namer.output_dir(output_dir)
        namer.asdm(task_args['vis'])

        if stage:
            namer.stage(stage)

        return namer.get_filename(True)

class TsysCaltable(api.Heuristic):
    def calculate(self, output_dir='', stage=None, **task_args):
        namer = filenamer.TsysCalibrationTable()
        namer.output_dir(output_dir)
        namer.asdm(task_args['vis'])

        if stage:
            namer.stage(stage)

        return namer.get_filename(True)


class WvrgCaltable(api.Heuristic):
    def calculate(self, output_dir='', stage=None, **task_args):
        namer = filenamer.WvrgCalibrationTable()
        namer.output_dir(output_dir)
        namer.asdm(task_args['vis'])
        namer.smooth(task_args.get('smooth', None))

        if stage:
            namer.stage(stage)

        return namer.get_filename(True)
