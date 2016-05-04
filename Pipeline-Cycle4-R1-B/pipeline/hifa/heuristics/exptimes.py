'''
Compute the mean per spectral window per scan exposure and
integration times for the selected data
'''

def get_scan_exptimes (ms, fieldnamelist, intent, spwidlist): 

    """
    Input Parameters
                   ms: The pipeline context ms object
        fieldnamelist: The list of field names to be selected
               intent: The intent of the fields to be selected
            spwidlist: The list of spw ids to be selected
    """

    fieldset = set(fieldnamelist)

    # Get the scan list
    obscans = []
    for scan in ms.get_scans (scan_intent=intent):
        # Remove scans not associated with the input field names
        scanfieldset = set([field.name for field in scan.fields])
        if len(fieldset.intersection(scanfieldset)) == 0:
            continue
        obscans.append(scan)

    # Return an empty list
    if not obscans:
        return []

    exptimes = []

    # Loop over the spw ids
    for spwid in spwidlist:

        # Get spectral window
        try:
            spw = ms.get_spectral_window(spwid)
        except:
            exptimes.append((spwid, 0.0))
            continue

        # Find scans associated with the spw. They may be different from
        # one spw to the next
        spwscans = []
        for obscan in obscans:
            scanspwset = set ([scanspw.id for scanspw in list(obscan.spws) \
                if scanspw.num_channels not in (1,4)])
            if len(set([spwid]).intersection(scanspwset)) == 0:
                continue
            spwscans.append(obscan)
        if not spwscans:
            exptimes.append((spwid, 0.0))
            continue

        # Limit the scans per spw to those for the first field
        #    in the scan sequence.
        fieldnames = [field.name for field in spwscans[0].fields]
        fieldname = fieldnames[0]
        fscans = []
        for scan in spwscans:
            fnames = [field.name for field in scan.fields]
            if fieldname != fnames[0]:
                continue
            fscans.append(scan)
        if not fscans:
            exptimes.append((spwid, 0.0))
            continue

        # Retrieve total exposure time and mean integration time in minutes
        #    Add to dictionary
        exposureTime = 0.0
        for scan in fscans:
            exposureTime = exposureTime + scan.exposure_time(spw.id).total_seconds()
        exptimes.append ((spwid, exposureTime / len(fscans)))

        return exptimes
