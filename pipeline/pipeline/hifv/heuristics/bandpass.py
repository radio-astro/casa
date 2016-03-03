from __future__ import absolute_import

from pipeline.infrastructure import casa_tasks


def do_bandpass(vis, caltable, context=None, RefAntOutput=None, ktypecaltable=None, bpdgain_touse=None):
    """Run CASA task bandpass"""

    m = context.observing_run.get_ms(vis)
    bandpass_field_select_string = context.evla['msinfo'][m.name].bandpass_field_select_string
    bandpass_scan_select_string = context.evla['msinfo'][m.name].bandpass_scan_select_string
    minBL_for_cal = max(3,int(len(m.antennas)/2.0))

    BPGainTables = list(context.callibrary.active.get_caltable())
    BPGainTables.append(ktypecaltable)
    BPGainTables.append(bpdgain_touse)

    bandpass_task_args = {'vis'         :vis,
                          'caltable'    :caltable,
                          'field'       :bandpass_field_select_string,
                          'spw'         :'',
                          'intent'      :'',
                          'selectdata'  :True,
                          'uvrange'     :'',
                          'scan'        :bandpass_scan_select_string,
                          'solint'      :'inf',
                          'combine'     :'scan',
                          'refant'      :RefAntOutput[0].lower(),
                          'minblperant' :minBL_for_cal,
                          'minsnr'      :5.0,
                          'solnorm'     :False,
                          'bandtype'    :'B',
                          'fillgaps'    :0,
                          'smodel'      :[],
                          'append'      :False,
                          'docallib'    :False,
                          'gaintable'   :BPGainTables,
                          'gainfield'   :[''],
                          'interp'      :[''],
                          'spwmap'      :[],
                          'parang'      :False}

    job = casa_tasks.bandpass(**bandpass_task_args)

    return job