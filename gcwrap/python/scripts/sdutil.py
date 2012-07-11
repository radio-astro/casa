from casac import casac
import os
import asap as sd
from asap import _to_list

qatool = casac.quanta()

def assert_infile_exists(infile=None):
    if (infile == ""):
        raise Exception, "infile is undefined"

    filename = os.path.expandvars(infile)
    filename = os.path.expanduser(filename)
    if not os.path.exists(filename):
        mesg = "File '%s' not found." % (filename)
        raise Exception, mesg


def get_default_outfile_name(infile=None, outfile=None, suffix=None):
    if (outfile == ""):
        res = infile.rstrip("/") + suffix
    else:
        res = outfile
    return res


def assert_outfile_canoverwrite_or_nonexistent(outfile=None, outform=None, overwrite=None):
    filename = os.path.expandvars(outfile)
    filename = os.path.expanduser(filename)
    if not overwrite and (outform.upper != "ASCII"):
        if os.path.exists(filename):
            mesg = "Output file '%s' exists." % (filename)
            raise Exception, mesg


def get_listvalue(value):
    return _to_list(value, int) or []

def get_selector(in_scans=None, in_ifs=None, in_pols=None, \
                 in_field=None, in_beams=None):
    scans = get_listvalue(in_scans)
    ifs   = get_listvalue(in_ifs)
    pols  = get_listvalue(in_pols)
    beams = get_listvalue(in_beams)
    selector = sd.selector(scans=scans, ifs=ifs, pols=pols, beams=beams)

    if (in_field != ""):
        # NOTE: currently can only select one
        # set of names this way, will probably
        # need to do a set_query eventually
        selector.set_name(in_field)

    return selector


def get_restfreq_in_Hz(s_restfreq):
    #### This function can be altered by the following 10 lines ###
#     if not qatool.isquantity(s_restfreq):
#         mesg = "Input value is not a quantity"
#         raise Exception, mesg
#     if qatool.compare(s_restfreq,'Hz'):
#         return qatool.convert(s_restfreq, 'Hz')['value']
#     elif qatool.quantity(s_restfreq)['unit'] == '':
#         return float(s_restfreq)
#     else:
#         mesg = "wrong unit of restfreq."
#         raise Exception, mesg
    ###############################################################
    value = 0.0
    unit = ""
    s = s_restfreq.replace(" ","")

    for i in range(len(s))[::-1]:
        if s[i].isalpha():
            unit = s[i] + unit
        else:
            value = float(s[0:i+1])
            break

    if (unit == "") or (unit.lower() == "hz"):
        return value
    elif (len(unit) == 3) and (unit[1:3].lower() == "hz"):
        unitprefix = unit[0]
        factor = 1.0

        if (unitprefix == "a"):
            factor = 1.0e-18
        elif (unitprefix == "f"):
            factor = 1.0e-15
        elif (unitprefix == "p"):
            factor = 1.0e-12
        elif (unitprefix == "n"):
            factor = 1.0e-9
        elif (unitprefix == "u"):
            factor = 1.0e-6
        elif (unitprefix == "m"):
            factor = 1.0e-3
        elif (unitprefix == "k"):
            factor = 1.0e+3
        elif (unitprefix == "M"):
            factor = 1.0e+6
        elif (unitprefix == "G"):
            factor = 1.0e+9
        elif (unitprefix == "T"):
            factor = 1.0e+12
        elif (unitprefix == "P"):
            factor = 1.0e+15
        elif (unitprefix == "E"):
            factor = 1.0e+18
        
        return value*factor
    else:
        mesg = "wrong unit of restfreq."
        raise Exception, mesg


def normalise_restfreq(in_restfreq):
    if isinstance(in_restfreq, float):
        return in_restfreq
    elif isinstance(in_restfreq, int):
        return float(in_restfreq)
    elif isinstance(in_restfreq, str):
        return get_restfreq_in_Hz(in_restfreq)
    elif isinstance(in_restfreq, list):
        res = []
        for i in xrange(len(in_restfreq)):
            elem = in_restfreq[i]
            if isinstance(elem, float):
                res.append(elem)
            elif isinstance(elem, int):
                res.append(float(elem))
            elif isinstance(elem, str):
                res.append(get_restfreq_in_Hz(elem))
            elif isinstance(elem, dict):
                if isinstance(elem["value"], float):
                    res.append(elem)
                elif isinstance(elem["value"], int):
                    dictelem = {}
                    dictelem["name"]  = elem["name"]
                    dictelem["value"] = float(elem["value"])
                    res.append(dictelem)
                elif isinstance(elem["value"], str):
                    dictelem = {}
                    dictelem["name"]  = elem["name"]
                    dictelem["value"] = get_restfreq_in_Hz(elem["value"])
                    res.append(dictelem)
            else:
                mesg = "restfreq elements must be float, int, or string."
                raise Exception, mesg
        return res
    else:
        mesg = "wrong type of restfreq given."
        raise Exception, mesg

