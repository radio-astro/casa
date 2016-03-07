========================
Tutorial #1 - Basic ASAP
========================

.. sectionauthor:: Kate Brooks

**Main goal:** Get a basic understanding of ASAP commands to import and plot data

Files
-----

* 2008-03-12_09320-M999.rpf Data file (1.1 Mb)


Data Log
--------

* ON-OFF Position switching mode with Mopra

* 2 Scans (1 OFF and 1 ON)

* 1 MOPS Zoom band

Instructions
------------

1. Work through the list of commands given in the text file to
   calibrate data taken with the Mopra telescope. Commands
   should be typed line-by-line into ASAP. Seek help from the
   tutors if there are any commands you don't understand.

   To start asap simply run the following at your prompt::

      	    asap

2. Write a python script to automate the calibration procedure
   used in step 1. Your python script should permit interaction
   with the plotter and should be executed in a terminal (and not
   within ASAP) with the following command:: 

   	  python -i myscript.py

   The python script to be run outside asap needs to have the following line 
   at the top of the file::

      	  from asap import *

   This will import all asap functionality into the "normal" python.

Commands
--------

.. code-block:: python

	# Load data file into memory and view description 
	s = scantable('2008-03-12_0932-M999.rpf')
	print s

	# Set the plotting mode
	plotter.set_mode(stacking='i', panelling='t')

	# Plot all raw data
	plotter.plot(s)

	# Set the doppler convention
	s.set_doppler('RADIO')

	# Set the rest frame
	s.set_freqframe('LSRK')

	# Set the observed rest frequency in Hz
	s.set_restfreqs([86243.37e6])

	# Define the channel unit
	s.set_unit('km/s') 

	# Form the quotient spectra
	q=s.auto_quotient()

	# Average all scans in time, aligning in velocity
	# Note: That for this dataset there is only 1 scan and so this step is redundant  
	av = q.average_time(align=True)

	# Average the two polarisations together
	iav = av.average_pol()

	# Plot the spectrum
	plotter.plot(iav)

	# Remove the baseline (set to 0 order). Specify the signal-free channels
	msk = iav.create_mask([-200,-50],[50,180])
	iav.poly_baseline(msk,0)
	plotter.plot(iav)

	# Smooth the data with boxcar, full width = 3
	siav = iav.smooth(kernel = 'boxcar', width = 3, insitu = False)
	plotter.plot(siav)

	# Scale the data according to scaling fudge factor
	# Eg. With beam efficiency of 0.49 at 86 GHz
	iav.scale(2)  

	# Make final plot for saving
	plotter.set_range(-20,30)  
	plotter.plot(siav)
	plotter.set_legend(mode=-1)
	plotter.set_title(['Orion-SiO'], fontsize=18) 
	plotter.text(10,95,"SiO (2-1 v=1) at 86243.440 MHz", fontsize=12)
	plotter.text(-19,95,"2008/03/12", fontsize=12)
	plotter.text(-19,90,"Zoom Mode", fontsize=12)

	# Save plot as postscript file
	plotter.save('Orion-SiO.ps')
