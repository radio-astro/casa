class util:

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

            #print "gaincal is", gaincal
            #print "fluxcal is", fluxcal
            #print "bandpasscal is", bandpasscal

            ngain = 0
            nbandpass = 0
            nflux = 0

            for i in range(table.nrows()):
                
                if str(field_id[i]) == gaincal:
                    fieldTypes[i] = "GAIN"
                    ngain += 1
                if str(field_id[i]) == bandpasscal:
                    if fieldTypes[i] != "":
                        fieldTypes[i] = fieldTypes[i] + "+BANDPASS"
                    else:
                        fieldTypes[i] = "BANDPASS"
                    nbandpass += 1
                if str(field_id[i]) == fluxcal:
                    if fieldTypes[i] != "":
                        fieldTypes[i] = fieldTypes[i] + "+FLUX"
                    else:
                        fieldTypes[i] = "FLUX"
                    nflux += 1

                #print "field_id[i] = '%s'" %( str(field_id[i]))
                #print "fieldTypes[i] = '%s'" %( fieldTypes[i])

            # This is an assumption of the heuristics code
            if ngain != 1:
                raise Exception, "Exactly 1 GAIN calibrator needed, found %s fields mathcing '%s'" % (ngain, gaincal)
            if nflux != 1:
                raise Exception, "Exactly 1 FLUX calibrator needed, found %s fields mathcing '%s'" % (nflux, fluxcal)
            if nbandpass != 1:
                raise Exception, "Exactly 1 BANDPASS calibrator needed, found %s fields mathcing '%s'" % (nbandpass, bandpasscal)

            return fieldTypes

    get_source_types = staticmethod(get_source_types)
