=====================
Scantable Description
=====================

Introduction
============

The Scantable is the ASAP single-dish data container. It was designed
based on the data abstraction of the PKSreader API.

ASAP data handling works on objects called scantables. A scantable holds your
data, and also provides functions to operate upon it.

The building block of a scantable is an integration, which is a single row of
a scantable. Each row contains just one spectrum for each beam, IF and
polarisation. For example Parkes OH-multibeam data would normally contain
13 beams, 1 IF and 2 polarisations, Parkes methanol-multibeam data would
contain 7 beams, 2 IFs and 2 polarisations while the Mopra 8-GHz MOPS
filterbank will produce one beam, many IFs, and 2-4 polarisations.

All of the combinations of Beams/IFs an Polarisations are contained in separate
rows. These rows are grouped in cycles (same time stamp).

A collection of cycles for one source is termed a scan (and each scan has a
unique numeric identifier, the SCANNO). A scantable is then a collection of
one or more scans. If you have scan-averaged your data in time, i.e. you
have averaged all cycles within a scan, then each scan would hold just
one (averaged) integration.

Schema
======

The Scantable consist of a main (casacore) Table and several sub-tables which
are referenced via **ID**. The following descriptions use the following syntax

    **COLUMN/KEYWORD NAME** - *DATA_TYPE(N)=DEFAULT_VALUE*

where *DEFAULT_VALUE* is optional and *(N)* indicates a Vector of that type

----------
Main Table
----------

The main table consists of global data (keywords) and row-based data (columns)


Keywords
--------

* **VERSION** - *uInt*

    the version of the scantable for compatibility tests

* **POLTYPE** - *String=linear*

    the polarisation type, i.e. *stokes*, *linear* or *circular*

* **DIRECTIONREF** - *String=J2000*

    the reference frame of the direction (coordinate) [redundant?!]

* **OBSMODE** - *String=""*

    no idea...

* **UTC** - *String*

    most likely the starting time of the observation (file)

* **AntennaName** - *String*

    the name of the antenna

* **AntennaPosition** - *Double(3)*

    the WGS position of the antenna

* **FluxUnit** - *String*

    the unit of the channel values i.e. *Jy* or *K*

* **Observer** - *String*

    the name of the observer

* **Obstype** -  *String*

    probably telescope specific name/encoding

* **Project** -  *String*

    e.g. proposal project number, survey name

* **Bandwidth** - *Double*

    self-explanatory

* **nIF, nBeam, nPol, nChan** - *Int*

    the number of IF, Beam, Polarisation and Channel values in the scantable
    This is currently **FIXED**

* **FreqRefFrame** - *String*

    redundant info - also in **FREQUENCIES** sub-table

* **FreqRefVal** - *Double*

    no idea...

Columns
-------

* **SPECTRA** - *Float(nchannel)*

    the spectral channel data.

* **FLAGTRA** - *uChar(nchannel)*

    the corresponding flags

* **TSYS** - *Float(nchannel)*

    the channel-based system temperature values

* **FLAGROW** - *uInt=-1*

    spectrum based flags

* **TIME** - *MEpoch*

    The mjd time when the observation took place

* **INTERVAL** - *Double*

    the integration time in seconds

* **DIRECTION** - *MDirection*

    the associated direction on the sky

* **AZIMUTH, ELEVATION** - *Float*

    the azimuth/elevation when the spectrum was collected

* **SRCNAME** - *string*

    the name of the source observered

* **SRCTYPE** - *Int=-1*

    the tyep of the source, i.e. indicating if it is an on source scan or
    off source. This will be used for calibration

* **SCANNO, BEAMNO, POLNO, IFNO, CYCLENO** - *uInt*

    These columns index (0-based) the respective values.

    * SCANNO: the number odf the scan. A scan is usually multiple integrations
              (cycles)

    * CYCLENO: the integration number within a scan (sub-scan?)

    * IFNO: the index of the IF (spectral window)

    * BEAMNO: the index of the beam (in a multibeam system)

    * POLNO: the index of the polarisation, e.g. XX=0, YY=1, Real(XY)=2,
             Imag(XY)=3

* **REFBEAMNO** - *Int=-1*

    optional index of the reference beam in a multibeam obervation

* **FREQ_ID, MOLECULE_ID, TCAL_ID, FOCUS_ID, WEATHER_ID, FIT_ID** - *Int*

    the reference indeces to the sub-tables

* **FIELDNAME** - *string*

    the name of the field the source belongs to

* **OPACITY** - *Float*

    maybe redundant - not used anywhere

* **SRCPROPERMOTION, SRCVELOCITY, SRCDIRECTION**

    see livedata docs - not used in ASAP

* **SCANRATE**

    see livedata docs - not used in ASAP

----------
Sub-Tables
----------

FREQUENCY
---------

""""""""
Keywords
""""""""

* **BASEFRAME** - *String=TOPO*
* **FRAME** - *String=TOPO*
* **EQUINOX** - *String=J2000*
* **UNIT** - *String=""*
* **DOPPLER** - *String=RADIO*

"""""""
Columns
"""""""

* **REFPIX** - *Double*

* **REFVAL** - *Double*

* **INCREMENT** - *Double*

MOLECULES
---------

""""""""
Keywords
""""""""

* **UNIT** - *String=Hz*

"""""""
Columns
"""""""

* **RESTFREQUENCY** - *Double()*
* **NAME** - *String()*
* **FORMATTEDNAME** - *String()*

FOCUS
-----

""""""""
Keywords
""""""""

* **PARALLACTIFY** - *Bool=False*

    indicates whether the observation was parallactified i.e. observed in such
    a way that the sum of the phases is zero.

"""""""
Columns
"""""""

* **PARANGLE** - *Float*

    the parallacticle angle of the observation

* **ROTATION** - *Float*

    feed rotation

* **AXIS** - *Float*

    ? ask Mark Calabretta

* **TAN** - *Float*

    ? ask Mark Calabretta

* **HAND** - *Float*

    the sign of the polarisation i.e. 1 for XY and '-1' for YX

* **USERANGLE** - *Float*

    A user definable phase correction term

* **MOUNT** - *Float*

    ? ask Jim Caswell

* **XYOFFSET** - *Float*

    ? ask Jim Caswell

* **XYPHASEOFFSET** - *Float*

    ? ask Jim Caswell

.. todo:: add all sub-table descriptions

========================
Mapping to other formats
========================

MS
==

MeasurementSets are specifically designed for synthesis data. As such one of the
main dfferences between the Scantable and a MS is how spectra are stored.
An MS  stores those in an n_pol x n_channel complex matrix (DATA column) and/or
a n_pol x n_channel float matrix (FLOAT_DATA column) whereas the Scantable
stores each polarisation seperately. In case of polarimetry data the (complex)
cross-polarisation is serialised into real and imaginary parts, e.g. a total of
four rows would describe one polarisation.
