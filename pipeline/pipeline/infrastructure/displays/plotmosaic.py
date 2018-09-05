import math
from decimal import Decimal

import matplotlib.pyplot
import numpy
import scipy.interpolate
from matplotlib.lines import Line2D
from matplotlib.patches import Circle
from matplotlib.ticker import FuncFormatter

import pipeline.domain.unitformat as unitformat
from pipeline.domain.measures import FrequencyUnits, DistanceUnits, Distance, ArcUnits, EquatorialArc
from pipeline.infrastructure.casatools import quanta
import pipeline.infrastructure

LOG = pipeline.infrastructure.get_logger(__name__)

# used when deciding primary beam colour
SEVEN_M = Distance(7, DistanceUnits.METRE)
# used to convert frequency to wavelength
C_MKS = 299792458


def plot_mosaic(ms, source, figfile):
    """
    Produce a plot of the pointings with the primary beam FWHM and field names.
    """
    fields = [f for f in source.fields]

    median_ref_freq = numpy.median([spw.ref_frequency.to_units(FrequencyUnits.HERTZ)
                                    for spw in ms.get_spectral_windows(science_windows_only=True)])

    median_ref_wavelength = Distance(C_MKS / median_ref_freq, DistanceUnits.METRE)

    dish_diameters = {Distance(d, DistanceUnits.METRE) for d in {a.diameter for a in ms.antennas}}

    # convert radians to degrees, constrained within [0,360]
    ra_deg = numpy.array([quanta.convert(f.mdirection['m0']['value'], 'deg')['value'] % 360 for f in fields])
    dec_deg = numpy.array([quanta.convert(f.mdirection['m1']['value'], 'deg')['value'] for f in fields])

    all_ras = [EquatorialArc(o, ArcUnits.DEGREE) for o in ra_deg]
    all_decs = [EquatorialArc(o, ArcUnits.DEGREE) for o in dec_deg]
    mean_ra = sum(all_ras, EquatorialArc(0)) / len(all_ras)
    mean_dec = sum(all_decs, EquatorialArc(0)) / len(all_decs)

    # scale by cos(dec) to make the pointing pattern in angle on sky
    relative_ra = [(a - mean_ra) * Decimal(math.cos(mean_dec.to_units(ArcUnits.RADIAN))) for a in all_ras]
    relative_dec = [(a - mean_dec) for a in all_decs]

    ra_range = max(relative_ra) - min(relative_ra)
    dec_range = max(relative_dec) - min(relative_dec)
    x_label_offset = 0.02 * float(ra_range.to_units(ArcUnits.ARC_SECOND))
    y_label_offset = 0.02 * float(dec_range.to_units(ArcUnits.ARC_SECOND))

    fig = matplotlib.pyplot.figure()
    ax = fig.add_subplot(1, 1, 1)

    taper = antenna_taper_factor(ms.antenna_array.name)
    # field labels overlap and become unintelligible if there are too many of
    # them
    draw_field_labels = True if len(fields) <= 500 else False
    # for the same reason, just draw the field numbers if there's a risk of the
    # plot labels becoming unintelligible
    include_field_name_in_label = True if len(fields) <= 20 else False

    legend_labels = {}
    legend_colours = {}
    for diameter in dish_diameters:
        primary_beam = primary_beam_fwhm(median_ref_wavelength, diameter, taper)
        radius_arcsecs = 0.5 * float(primary_beam.to_units(ArcUnits.ARC_SECOND))

        for field, rel_ra, rel_dec in zip(fields, relative_ra, relative_dec):
            x = float(rel_ra.to_units(ArcUnits.ARC_SECOND))
            y = float(rel_dec.to_units(ArcUnits.ARC_SECOND))
            colour = get_dish_colour(diameter, field)

            cir = Circle((x, y), radius=radius_arcsecs, facecolor='none', edgecolor=colour, linestyle='dotted',
                         alpha=0.6)
            ax.add_patch(cir)

            if draw_field_labels:
                if include_field_name_in_label:
                    label = '{} (#{})'.format(field.name, field.id)
                else:
                    label = '{}'.format(field.id)
                ax.text(x + x_label_offset, y + y_label_offset, label, fontsize=12, color=colour, weight='normal')

            ax.plot(x, y, '{}+'.format(colour), markersize=4)

            label = 'T$_{{sys}}$-only field' if is_tsys_only(field) else str(diameter)
            if label not in legend_labels:
                legend_labels[label] = Line2D(range(1), range(1), color=colour, linewidth=2, linestyle='dotted')
                legend_colours[label] = colour

    title_string = '{}, {}, average freq.={}'.format(ms.basename, source.name,
                                                     unitformat.frequency.format(median_ref_freq))
    ax.set_title(title_string, size=12)
    ra_string = '{:02d}$^{{\\rm h}}${:02d}$^{{\\rm m}}${:02.3f}$^{{\\rm s}}\!\!.$'.format(*mean_ra.toHms())
    ax.set_xlabel('Right ascension offset from {}'.format(ra_string))
    dec_string = '{:02d}$\degree${:02d}$^\prime${:02.1f}$^{{\prime\prime}}$'.format(*mean_dec.toDms())
    ax.set_ylabel('Declination offset from {}'.format(dec_string))

    leg_lines = [legend_labels[i] for i in sorted(legend_labels)]
    leg_labels = sorted(legend_labels)
    leg = ax.legend(leg_lines, leg_labels, prop={'size': 10})
    leg.get_frame().set_alpha(0.8)
    for text in leg.get_texts():
        text.set_color(legend_colours[text.get_text()])

    y = 0.02
    pb_formatter = get_arc_formatter(1)
    for d in dish_diameters:
        colour = get_dish_colour(d)
        pb = primary_beam_fwhm(median_ref_wavelength, d, taper)
        pb_degs = float(pb.to_units(ArcUnits.DEGREE))
        msg = '{} primary beam = {}'.format(d, pb_formatter.format(pb_degs))
        t = ax.text(0.02, y, msg, color=colour, transform=ax.transAxes, size=10)
        t.set_bbox(dict(facecolor='white', edgecolor='none', alpha=0.75))
        y += 0.05

    ax.axis('equal')
    ax.margins(0.05)

    arcsec_formatter = FuncFormatter(label_format)
    ax.xaxis.set_major_formatter(arcsec_formatter)
    ax.yaxis.set_major_formatter(arcsec_formatter)
    ax.xaxis.grid(True, which='major')
    ax.yaxis.grid(True, which='major')

    fig.savefig(figfile)
    matplotlib.pyplot.close(fig)


def get_arc_formatter(precision):
    """
    Presents a value of equatorial arc in user-friendly units.
    """
    s = '{0:.' + str(precision) + 'f}'
    f = unitformat.UnitFormat(prefer_integers=True)
    f.addUnitOfMagnitude(1. / 10000006, s + ' $\mu$as')
    f.addUnitOfMagnitude(1. / 1000, s + ' mas')
    f.addUnitOfMagnitude(1., s + '$^{{\prime\prime}}$')
    f.addUnitOfMagnitude(60., s + '$^\prime$')
    f.addUnitOfMagnitude(3600., s + '$\degree$')
    return f


# Used to label x and y plot axes
AXES_FORMATTER = get_arc_formatter(0)


def label_format(x, _):
    """Labels plot axes for plots specified in units of arcseconds"""
    # x is given in arcsecs, _ is tick position
    return AXES_FORMATTER.format(x)


def get_dish_colour(dish_diameter, field=None):
    if field and is_tsys_only(field):
        return 'r'
    if dish_diameter == SEVEN_M:
        return 'b'
    else:
        return 'k'


def is_tsys_only(field):
    """
    Returns True if the field was only used to observe Tsys fields.
    """
    return 'TARGET' not in field.intents and 'ATMOSPHERE' in field.intents


def primary_beam_fwhm(wavelength, diameter, taper):
    """
    Implements the Baars formula: b*lambda / D.
      if use2007formula==True, use the formula from Baars 2007 book
        (see au.baarsTaperFactor)
      In either case, the taper value is expected to be entered as positive.
        Note: if a negative value is entered, it is converted to positive.
    The effect of the central obstruction on the pattern is also accounted for
    by using a spline fit to Table 10.1 of Schroeder's Astronomical Optics.
    The default values correspond to our best knowledge of the ALMA 12m antennas.
      diameter: outer diameter of the dish in meters
      obscuration: diameter of the central obstruction in meters
    """
    b = baars_taper_factor(taper) * central_obstruction_factor(diameter)
    lambda_m = wavelength.to_units(DistanceUnits.METRE)
    diameter_m = diameter.to_units(DistanceUnits.METRE)
    return EquatorialArc(Decimal(b) * lambda_m / diameter_m, ArcUnits.RADIAN)


def central_obstruction_factor(diameter, obscuration=0.75):
    """
    Computes the scale factor of an Airy pattern as a function of the
    central obscuration, using Table 10.1 of Schroeder's "Astronomical Optics".
    -- Todd Hunter
    """
    epsilon = obscuration / float(diameter.to_units(DistanceUnits.METRE))
    spline_func = scipy.interpolate.UnivariateSpline([0, 0.1, 0.2, 0.33, 0.4], [1.22, 1.205, 1.167, 1.098, 1.058], s=0)
    return spline_func(epsilon) / 1.22


def baars_taper_factor(taper_dB):
    """
    Converts a taper in dB to the constant X
    in the formula FWHM=X*lambda/D for the parabolic illumination pattern.
    We assume that taper_dB comes in as a positive value.
    - Todd Hunter
    """
    # use Equation 4.13 from Baars 2007 book
    tau = 10 ** (-0.05 * taper_dB)
    return 1.269 - 0.566 * tau + 0.534 * (tau ** 2) - 0.208 * (tau ** 3)


def antenna_taper_factor(array_name):
    # Primary beam taper in dB.
    antenna_taper = {
        'ALMA': 10.0,
        'EVLA': 0.0,
        'VLA': 0.0,
    }
    try:
        return antenna_taper[array_name]
    except KeyError:
        LOG.warning('Unknown array name: {}. Using null antenna taper factor in mosaic plots'.format(array_name))
        return 0.0
