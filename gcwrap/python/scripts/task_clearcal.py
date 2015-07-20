import os
from taskinit import cbtool, tbtool, mstool, casalog, write_history
from parallel.parallel_task_helper import ParallelTaskHelper


def clearcal(
    vis=None,
    field=None,
    spw=None,
    intent=None,
    addmodel=None,
    ):

    casalog.origin('clearcal')

    # Do the trivial parallelization
    if ParallelTaskHelper.isParallelMS(vis):
        helper = ParallelTaskHelper('clearcal', locals())
        helper.go()
        return

    # Local versions of the tools
    tblocal = tbtool()
    cblocal = cbtool()
    mslocal = mstool()

    try:

        # we will initialize scr cols only if we don't create them
        doinit = False

        if (type(vis) == str) & os.path.exists(vis):
            tblocal.open(vis)
            doinit = tblocal.colnames().count('CORRECTED_DATA') > 0
            tblocal.close()

            # We ignore selection if creating the scratch columns
            if not doinit:
                casalog.post('Need to create scratch columns; ignoring selection.'
                             )

            cblocal.open(vis, addmodel=addmodel)
        else:
            raise Exception, \
                'Visibility data set not found - please verify the name'

        # If necessary (scr col not just created), initialize scr cols
        if doinit:
            cblocal.selectvis(field=field, spw=spw, intent=intent)
            cblocal.initcalset(1)
        cblocal.close()

        # Write history to the MS
        param_names = clearcal.func_code.co_varnames[:clearcal.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        casalog.post('Updating the history in the output', 'DEBUG1')
        write_history(mslocal, vis, 'clearcal', param_names,
                      param_vals, casalog)
        
    except Exception, instance:

        print '*** Error ***', instance

