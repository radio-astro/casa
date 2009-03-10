/** \mainpage C++ interface of the ATM model for ALMA (June 16th, 2008 status)  
 *
 * \section HISTORY ATM history
 *
 * The Atmospheric Transmission at Microwaves model (ATM) has been the result of research conducted through 
 * the years by under the leadership of J. Cernicharo (1985-1995) and Juan R. Pardo (1995-2008) with the collaboration 
 * of many different people and groups (see list of collaborators in the different publications). This work has combined 
 * modeling efforts, literature research, spectroscopy calculations, experimental refinement and validation, and research 
 * applications. A fortran-77 code was written and distributed upon request to scientists from both the astrophysics and remote 
 * sensing communities.
 *
 * An agreement was signed between Consejo Superior de Investigaciones Cientificas (CSIC, Spain) and the European Southern 
 * Observatory (ESO) to develop a C++ interface of ATM for use within the TelCal subsystem of ALMA. Such an interface to a fortran 
 * core of libraries was written an finalized by the beginning of 2007. As of June 2007 the fortran core of libraries had been 
 * completely translated into C++ code so that now there is no dependency on any fortran code. 
 *
 * Together with the C++ ATM module, a set of test examples had been implemented. Retrieval procedures are tested against real data 
 * provided mostly by FTS and WVR experiments from both Mauna Kea and Chajnantor. 
 *
 * A new contract between CSIC and ESO was signed in June 2007 for tests and maintenance of ATM C++. The new contract is for 5 years with 
 * a 0.1 FTE effort per year provided by CSIC.
 *
 * Finally, a new agreement to use ATM C++ as free LPGL software for the ALMA project has been signed in Feb. 2008.
 *
 * \section ATMFORALMA ATM C++ for ALMA
 *
 * \subsection Requests Requests
 *
 *  The ATM C++ development meets the following requests of the subsystems OFFLINE and TELCAL:
 * 
 * - Has its own procedure to build reference atmospheric profiles from a few basic 
 *   parameters (ground pressure and temperature, 
 *   site altitude, water vapor scale height, tropospheric lapse rate, atmosphere type,...)
 * - Allows for use of user-supplied atmospheric profiles layer-by-layer.   
 * - It is able to predict atmospheric absorption, emission and path length (or give the local refractive index) along the line of sight for all ALMA bands.
 * - The predicted spectra can be based on measured atmospheric parameters at the site: temperature, pressure, humidity.
 * - The prediction can incorporate results from water vapor retrievals based on Fourier Transform Spectroscopy (FTS) and/or 
 *   Water Vapor Radiometry (WVR) measurements. 
 * - Water Vapor Retrieval algorithms are available. Sky Coupling of the concerned devices can also be retrieved. Other retrieval schemes are easy to incorporate.
 * - Examples based on real FTS and WVR data are available. 
 *
 * \subsection Architecture Architecture
 *
 * These basic classes are:
 *
 * - <a href="classatm_1_1AtmProfile.html">AtmProfile</a>, to create and manage atmospheric profiles. An additional class 
 * (<a href="classatm_1_1AtmType.html">AtmType</a>
 * is inherited by this one. 
 * - <a href="classatm_1_1SpectralGrid.html">SpectralGrid</a>. The modeling of the atmosphere opacity, phase and radiance 
 * can be done at a given
 * frequency or for a set of frequencies within spectral window(s). In this later case, for convenience,
 * this additional class named <a href="classatm_1_1SpectralGrid.html">SpectralGrid.html</a> is also provided. 
 * It allows to specify in a simple and concise manner 
 * the set of frequencies to be considered.
 * - <a href="classatm_1_1RefractiveIndex.html">RefractiveIndex</a>, to extract refractive indexes </a>.
 * - <a href="classatm_1_1RefractiveIndexProfile.html">RefractiveIndexProfile</a>, to create the profiles of refractive indexes corresponding to the 
 * frequencies of <a href="classatm_1_1SpectralGrid.html">SpectralGrid</a> and the layers of the <a href="classatm_1_1AtmProfile.html">AtmProfile</a>.
 * - <a href="classatm_1_1SkyStatus.html">SkyStatus</a>, to perform radiative transfer calculations using the atmospheric profile 
 * from <a href="classatm_1_1AtmProfile.html">AtmProfile</a> and the corresponding absorption profile from 
 * <a href="classatm_1_1RefractiveIndexProfile.html">RefractiveIndexProfile</a>. Water vapor retrievals and 
 * phase correction values can also be performed using numerical methods in this class and the 
 * corresponding input information (radiometric measurements in WVR bands for example). In order to handdle water vapor retrievals more easily, two 
 * complementary classes have been defined:  <a href="classatm_1_1WVRMeasurement.html">WVRMeasurement</a> and 
 * <a href="classatm_1_1WaterVaporRadiometer.html">WaterVaporRadiometer</a>
 * .
 * 
 * The ATM code uses its own conventions for the units
 * of the physical quantities. A set of classes to handle these 
 * datatypes has been defined:
 *
 * - <a href="classatm_1_1Angle.html">Angle</a> (see also ATMAngle.h file reference)      
 * - <a href="classatm_1_1Frequency.html">Frequency</a> (ATMFrequency.h file reference)
 * - <a href="classatm_1_1Humidity.html">Humidity</a> (ATMHumidity.h file reference)
 * - <a href="classatm_1_1Length.html">Length</a> (ATMLength.h file reference)
 * - <a href="classatm_1_1Percent.html">Percent</a> (ATMPercent.h file reference)
 * - <a href="classatm_1_1Pressure.html">Pressure</a> (ATMPressure.h file reference)
 * - <a href="classatm_1_1MassDensity.html">MassDensity</a> (ATMMassDensity.h file reference)
 * - <a href="classatm_1_1NumberDensity.html">NumberDensity</a> (ATMNumberDensity.h file reference)
 * - <a href="classatm_1_1Temperature.html">Temperature</a> (ATMTemperature.h file reference)
 * .
 * to decouple the system of units adopted within the ATM 
 * library from the ones that C++ clients adopt for their
 * own convenience. 
 *
 * Auxiliary classes to help handling sets of measurements made by water vapor radiometer are defined:
 *
 * - <a href="classatm_1_1WVRMeasurement.html">WVRMeasurement</a>
 * - <a href="classatm_1_1WaterVaporRadiometer.html">WaterVaporRadiometer</a>
 *
 * \section TESTS ATM C++ test examples
 *
 * Several tests examples for the different main classes have been developed and documented. These tests 
 * are described in the links provided below. The executables can be found in /TelCalResults/Libraries/ATM/bin.
 *
 * - AtmosphereTypeTest is a test of this enumeration. See <a href="AtmosphereTypeTest_8cpp.html">AtmosphereTypeTest.cpp</a>
 * - AtmProfileTest is a very simple test and example of how to create an object of the 
 * <a href="classatm_1_1AtmProfile.html">AtmProfile</a> class. See <a href="AtmProfileTest_8cpp.html">AtmProfileTest.cpp</a> 
 * to check how it works.
 * - SpectrslGridTest provides a quite complete test of the <a href="classatm_1_1SpectralGrid.html">SpectralGrid</a> class. See <a href="SpectralGridTest_8cpp.html">SpectralGridTest.cpp</a> 
 * to check how it works.
 * - RefractiveIndexProfileTest provides a very straightforward test of the <a href="classatm_1_1RefractiveIndexProfile.html">RefractiveIndexProfile</a> class, but also fo the <a href="classatm_1_1AtmProfile.html">AtmProfile</a> class. See <a href="RefractiveIndexProfileTest_8cpp.html">RefractiveIndexProfileTest.cpp</a> to check how it works.
 * - SkyStatusTest provides a test of the <a href="classatm_1_1SkyStatus.html">SkyStatus</a> class. One object of this class can be used per antenna in order to account for the sky status for that particular antenna. See <a href="SkyStatusTest_8cpp.html">SkyStatusTest.cpp</a> to check how it works.
 * - FTSRetrievalTest provides a test of the <a href="classatm_1_1SkyStatus.html">SkyStatus</a> class combined with the capabilities of 
 *   the <a href="classatm_1_1SkyStatus.html#z13_110">WaterVaporRetrieval_fromFTS</a> function. See <a href="FTSRetrievalTest_8cpp.html">FTSRetrievalTest.cpp</a> to check how it works.
 * - WVRRetrievalTest provides a very complete test of all ATM classes, focussing on water vapor retrieval capabilities from real data. This test is 
 *   based in the publication Pardo et al., Astrophysical Journal Suppl., 153, 363-367 (2004). See <a href="WVRRetrievalTest_8cpp.html">WVRRetrievalTest.cpp</a> to check how it works.
 * 
 *  
 *
 * \section PUBLICATIONS ATM publications
 *
 * As a result of this effort through the years many of papers have appeared:  
 *
 * - J. R. Pardo, L. Pagani, M. Gerin  and  C. Prigent: "Evidence of the Zeeman Splitting in the 2(1)->0(1) Rotational Transition of the atmospheric 16O18O Molecule from Ground-Based Measurements"; J. Quant. Spectr. and Radiat. Transfer, 54, N6, 931-943 (1995). 
 * - J. R. Pardo, J. Cernicharo, E. Lellouch  and  G. Paubert: "Ground-Based Measurements of Middle Atmospheric Water Vapor at 183 GHz"; Journal of Geophysical Research, 101, D22, 28723-28730 (1996). 
 * - J. R. Pardo, J. Cernicharo  and  L. Pagani: "Ground-Based Spectroscopic Observations of Atmospheric Ozone from 142 to 359 GHz in Southern Europe"; Journal of Geophysical Research, 103, D6, 6189-6202 (1998)
 * - E. Serabyn, E. Weisstein, D.C. Lis,  and  J. R. Pardo: "Submillimeter FTS Measurements of Atmospheric Opacity above Mauna Kea"; Applied Optics, 37, 12, 2185-2198 (1998). 
 * - J. R. Pardo, M. Gerin, C. Prigent, J. Cernicharo, G. Rochard  and P. Brunel: "Remote sensing of the mesospheric temperature profile from close-to-nadir observations: discussion about the capabilities of the 57.5-62.5 GHz frequency band and the 118.75 GHz single O2 line"; J. Quant. Spectr. and Radiat. Transfer, 60, N4, 559-571 (1998).
 * - S. Matsushita, H. Matsuo, J. R. Pardo and  S. Radford: "FTS Measurements of Submm-Wave Atmospheric Opacity at Pampa la Bola II : Supra-THz Windows and Model Fitting"; Publ. Astron. Soc. Japan 51, 603-610 (1999).
 * - J. R. Pardo, L. Pagani, G. Olofsson, P. Febvre and J. Tauber: "Balloon-borne submillimeter observations of upper stratospheric O2 and O3"; J. Quant. Spectr. and Radiat. Transfer, 67, 2, 169-180 (2000). 
 * - J. R. Pardo, E. Serabyn and  J. Cernicharo: "Submillimeter atmospheric transmission measurements on Mauna Kea during extremely dry El Niño conditions: Implications for broadband opacity contributions"; J. Quant. Spectr. and Radiat. Transfer, 68/4, 419-433 (2001). 
 * - L. Garand, D. S. Turner, M. Larocque, J. Bates, S. Boukabara, P. Brunel, F. Chevalier, G. Deblonde, R. Engelen, M. Hollingshead, D. Jackson, G. Jedlovec, J. Joiner, T. Kleespies, D. S. McKague, L. McMillin, J.-L. Moncet, J. R. Pardo, P. J. Rayer, E. Salathe, R. Saunders, N. A. Scott, P. Van Delst, and H. Woolf: "Radiance and Jacobian intercomparison of radiative transfer models applied to HIRS and AMSU channels"; Journal of Geophysical Research, 106, 24017-24031, (2001).
 * - C. Prigent J.R. Pardo, M.I. Mishchenko and W.B. Rossow: "Microwave polarized signatures generated within cloud systems: SSM/I observations interpreted with radiative transfer simulations"; Journal of Geophysical Research, 106, 28243-28258, (2001)
 * - J. R. Pardo, J. Cernicharo, and E. Serabyn: "Atmospheric Transmission at Microwaves (ATM): An Improved Model for mm/submm applications"; IEEE Trans. on Antennas and Propagation, 49/12, 1683-1694 (2001). 
 * - M. Ridal, J. R. Pardo, D. P. Murtagh, F. Merino and  L. Pagani: "Microwave temperature and pressure measurements with the Odin satellite: II. Retrieval method"; Canadian Journal of Physics, 80(4), 455-467, (2002). 
 * - J. R. Pardo, M. Ridal, D. P. Murtagh and  J. Cernicharo: "Microwave temperature and pressure measurements with the Odin satellite: I. Observational method"; Canadian Journal of Physics, 80(4), 443-454, (2002). 
 * - M. Wiedner, C. Prigent, J. R. Pardo, O. Nuissier, J.-P. Chaboureau, J.-P. Pinty, and P. Mascart: "Modeling of passive microwave responses in convective situations using output from mesoscale models: Comparison with TRMM/TMI satellite observations"; Journal of Geophysical Research, 109, D6, 06214 (2004). 
 * - J. R. Pardo, M. Wiedner, E. Serabyn, C.D. Wilson, C. Cunningham, R.E. Hills, and J. Cernicharo: "Side-by-side comparison of Fourier Transform Spectroscopy and Water Vapor Radiometry as tools for the calibration of mm/submm ground-based observatories"; Astrophysical Journal Suppl., 153, 363-367 (2004). 
 * - J.R. Pardo, E. Serabyn, M.C. Wiedner, and J. Cernicharo: "Measured telluric continuum-like opacity beyond 1 THz"; J. Quant. Spec. and Radiat. Transfer 96/3-4, 537-545 (2005). 
 * - C. Prigent, E. Defer, J.R. Pardo, C. Pearl, W. Rossow, J.-P. Pinty: "Relations of polarized scattering signatures observed by the TRMM Microwave Instrument with electrical processes in cloud systems"; Geophysical Journal Letters 42, L04810-L04813 (2005). 
 * - Philip Rosenkranz, Stefan Buelher, Dietrich Feist, Tim Hewison, Nicole Jacquinet-Husson, J.R. Pardo, and Roger Saunders: "Emission and Spectroscopy of the Clear Atmosphere", Chapter 2 in "Thermal Microwave Radiation - Applications for Remote Sensing"; Christian Mätzler and Philip W. Rosenkranz, A.Battaglia and J.P. Wigneron (eds), IEE Electromagnetic Waves Series, London, UK. , ISBN: 0-86341-573-3 & 978-086341-573-9 (2006) 
 * - Alessandro Battaglia, Clemens Simmer, Susanne Crewell, Harald Czekala, Claudia Emde, Frank Marzano, Michael Mishchenko, Juan R. Pardo, and Catherine Prigent: "Emission and Scattering by Clouds and Precipitation", Chapter 3 in "Thermal Microwave Radiation - Applications for Remote Sensing"; Christian Mätzler and Philip W. Rosenkranz, A.Battaglia and J.P. Wigneron (eds), IEE Electromagnetic Waves Series, London, UK., ISBN: 0-86341-573-3 & 978-086341-573-9 (2006) 
 * - C. Prigent, J.R. Pardo, W.B. Rossow: "Comparisons of the millimeter and submillimeter frequency bands for atmospheric temperature and water vapor soundings for clear and cloudy skies"; Journal of Applied Meteorology and Climatology, 45, 1622-1633 (2006) 
 * - I. Meirold-Mautner, C. Prigent, J.R. Pardo, J.-P. Chaboureau, J.-P. Pinty, M. Mech, S. Crewell: "Radiative transfer simulations using mesoscale cloud model outputs and comparisons with passive microwave and infrared satellite observations for mid-latitude situations"; Journal of the Atmospheric Sciences, 64/5, 1550-1568 (2007)
 * - J.P. Chaboureau, N. Sohne, J.P. Pinty, I. Meirold-Mautner, E. Defer, C. Prigent, J. R. Pardo, M. Mech, S. Crewell: "A Midlatitude Precipitating Cloud Database Validated with Satellite Observations"; Journal of Applied Meteorology and Climatology, in press (2007)
 *
 *
 *
 */
