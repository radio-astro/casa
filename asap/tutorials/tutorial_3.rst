==========================================================================
Tutorial 3 - Spectral-line gaussian component fitting and rms noise levels
==========================================================================

.. sectionauthor:: Shari Breen

Files
-----

* mon.asap: Data file

Data Log
--------

* ON-OFF Position switching mode with Hobart

* 4 scans (2 OFF and 2 ON)

* Dual pol, 1 IF

Instructions
------------

* Work through the list of commands given in the text file to calibrate the data
  and fit gaussian components to the emission. The commands should be typed
  into ASAP line-by-line until you get to the ADVANCED FITTING step which
  will require you to experiment with the parameters.
 
* ADDITIONAL exercise: Determine the rms noise in the spectrum using the
  create mask function you learned in the first section and the stat command
  specified in the text file.

Examples - Fitting gaussians
----------------------------

If you only see a file called *mon.asap.tar*, untar it before getting started

.. code-block: shell

   tar xf mon.asap.tar
   asap

.. code-block:: python

      # Load the scantable for the source into ASAP
      mon = scantable("mon.asap")
      mon.summary()
      # Select the on-source scans
      source=mon.get_scan([1,3])
      # Select the reference scans
      ref=mon.get_scan([0,2])
      # Make the quotient spectra and plot it
      quot=quotient(source,ref)
      plotter.plot(quot)
      # Remove the baseline and plot
      quot.auto_poly_baseline()
      plotter.plot(quot)
      # Average the 2 scans together
      av=quot.average_time(align=True)
      plotter.plot(av)
      # Check the the rest frequency is set correctly and define unit as km/s
      print av.get_restfreqs()
      av.set_unit("km/s") 
      plotter.plot(av)
      # Scale the two polarizations separately
      sel=selector()
      sel.set_polarisations(0)
      av.set_selection(sel)
      av.scale(50.6)
      sel.set_polarisations(1)
      av.set_selection(sel)
      av.scale(32.5)
      # unset the selection
      av.set_selection()
      # Average the polarizations together
      avpol=av.average_pol()
      # Plot the data
      plotter.plot(avpol)
      # Create a mask that contains the emission
      msk=avpol.create_mask([-10,30])
      # Set up some fitting parameters
      f=fitter()
      f.set_scan(avpol,msk)
      f.set_function(gauss=2)
      # Fit a gaussian to the emission, plot the emission and fit
      f.fit()
      f.plot()
      # Save the plot
      plotter.save("monr2.eps")
      # Get the fit parameters
      f.get_parameters()
      # Plot the fit residuals
      f.plot(residual=True)
      # Plot each of the fitted gaussians separately, with or without the fit parameters overlaid
      f.plot(components=0,plotparms=True)
  
      f.plot(components=1)


ADVANCED FITTING
----------------

Sometimes it is necessary to force some of the fitting pa-
rameters such as peak or FWHM. For example, if you want to fix the peak of the
first gaussian component to 39 Jy you can use the following command:

.. code-block:: python

      f.set_gauss_parameters(39, 10.2, 1, peakfixed=1, component=0)

You can now experiment with forcing the values of the different parameters. To
look at the help file on this function type
help fitter.set_gauss parameters
Additional Exercise: Determine the rms noise in the spectrum.

1) Create a mask that excludes the emission (if you leave it in the rms noise will be
   significantly inflated), using the same command as before except this time you will
   have to specify two ranges of values either side of the emission.

2) Find the rms noise of the spectrum:

.. code-block:: python
      
      avpol.stats(stat="rms",mask=mymask)

The stats function can be used to extract many other statistics from the data
such as the maximum and minimum values, the median value and many more. Use
help to find out how to extract these statistics from the spectrum.

