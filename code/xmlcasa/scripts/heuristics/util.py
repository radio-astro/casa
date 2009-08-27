class util:

    # These static variables need to be initialized as containers
    # of field IDs
    source = "-1"
    gaincal = "-1"
    fluxcal = "-1"
    bandpasscal = "-1"

    def get_source_types(table):
        # print "type(table)", type(table)
        if 'SOURCE_TYPE' in table.colnames():
            #print "has column SOURCE_TYPE"
            return table.getcol('SOURCE_TYPE')
        else:
            field_id = table.getcol('SOURCE_ID')

            fieldTypes = [""]*table.nrows()

            #print "source is", source
            #print "gaincal is", gaincal
            #print "fluxcal is", fluxcal
            #print "bandpasscal is", bandpasscal

            nsource = 0
            ngain = 0
            nbandpass = 0
            nflux = 0

            for i in range(table.nrows()):
                
                if str(field_id[i]) in gaincal:
                    fieldTypes[i] = "GAIN"
                    ngain += 1
                if str(field_id[i]) in bandpasscal:
                    if fieldTypes[i] != "":
                        fieldTypes[i] = fieldTypes[i] + "+BANDPASS"
                    else:
                        fieldTypes[i] = "BANDPASS"
                    nbandpass += 1
                if str(field_id[i]) in fluxcal:
                    if fieldTypes[i] != "":
                        fieldTypes[i] = fieldTypes[i] + "+FLUX"
                    else:
                        fieldTypes[i] = "FLUX"
                    nflux += 1
                if str(field_id[i]) in source:
                    if fieldTypes[i] != "":
                        fieldTypes[i] = fieldTypes[i] + "+SOURCE"
                    else:
                        fieldTypes[i] = "SOURCE"
                    nsource += 1

                #print "field_id[i] = '%s'" %( str(field_id[i]))
                #print "fieldTypes[i] = '%s'" %( fieldTypes[i])

            # This is an assumption of the heuristics code
            if nsource == 0:
                raise Exception, "Please specify at least 1 SOURCE field. Found %s fields matching the input '%s'" % (nsource, str(source))
            if ngain == 0:
                raise Exception, "Please specify at least 1 GAIN calibrator. Found %s fields matching the input '%s'" % (ngain, str(gaincal))
            if nflux == 0:
                raise Exception, "Please specify at least 1 FLUX calibrator. Found %s fields matching the input '%s'" % (nflux, str(fluxcal))
            if nbandpass == 0:
                raise Exception, "Please specify at least 1 BANDPASS calibrator. Found %s fields matching the input '%s'" % (nbandpass, str(bandpasscal))

            return fieldTypes

    get_source_types = staticmethod(get_source_types)
