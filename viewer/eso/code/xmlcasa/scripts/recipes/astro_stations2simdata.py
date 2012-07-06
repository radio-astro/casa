from simutil import simutil

pdbi = {'long': '05:54:28.5', # deg East
        'lat': '44:38:02.0',  # deg
        'alt': 2550,          # m
        'diam': 15,           # m
        'a': 'w27 e04 E24 E68 N29 N46',
        'b': 'W27 W12 E12 E23 N20 N46',
        'c': 'W12 W09 E04 E10 N11 N17',
        'd': 'W08 W05 E03 N02 N07 N11'}

def dms2d(dms):
    """
    Given a d:m:s string, return it as a float in degrees.
    """
    d, m, s = map(float, dms.split(':'))
    m += s / 60.0
    m /= 60.0
    if dms[0] == '-':
        d -= m
    else:
        d += m
    return d

def astro_stations2simdata(aststatfn):
    """
    Reads aststatfn and writes it to simdata compatible antennalists
    pdbi-a.cfg, pdbi-b.cfg, pdbi-c.cfg, and pdbi-d.cfg.

    aststatfn: gildas-src-mmmxxx/packages/astro/etc/astro_stations.dat
               1st line is ignored: 800  / Largest baseline length
               The rest are:
                            ITRF relative to PdBI center (m)
               Station      X          Y              Z
    simdatfn:  simdata compatible configuration file, in ITRF.
    """
    mysim = simutil()
    pdbi['itrf'] = mysim.locxyz2itrf(dms2d(pdbi['lat']), dms2d(pdbi['long']),
                                     locz=pdbi['alt'])
    
    statdict = {}
    asf = open(aststatfn, 'r')
    line = asf.readline()         # Skip 1st line.
    for line in asf:
        stn, xstr, ystr, zstr = line.strip().split()
        statdict[stn] = [float(c) + d for (c, d) in zip((xstr, ystr, zstr), pdbi['itrf'])]
    asf.close()
    for cfg in ('a', 'b', 'c', 'd'):
        stns = [s.upper() for s in pdbi[cfg].split()]
        ofile = open('pdbi-' + cfg + '.cfg', 'w')
        ofile.write('# observatory=IRAM_PDB\n') # Is the _ necessary?
        ofile.write('# coordsys=XYZ\n')
        ofile.write('#\n')
        ofile.write('# Plateau de Bure %s configuration,\n' % cfg.upper())
        ofile.write('# converted to simdata format from\n#  %s\n#    (%s)\n'
                    % (aststatfn, time.strftime("%Y-%m-%d",
                                                time.localtime(os.path.getmtime(aststatfn)))))
        ofile.write('# by recipes.astro_stations2simdata\n')
        ofile.write('#\n')
        ofile.write('#       X              Y                Z       Diam  Station\n')
        for stn in stns:
            ofile.write('% 14.12g  % 14.12g  % 14.12g  %4.1f  %s\n' %
                        (statdict[stn][0],
                         statdict[stn][1],
                         statdict[stn][2],
                         pdbi['diam'],
                         stn))
        ofile.close()

