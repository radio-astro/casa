------------------------------------------------------------------------
r25313 | justo.gonzalez | 2013-07-05 17:09:03 +0200 (Fri, 05 Jul 2013) | 11 lines


       New Development: no
            JIRA Issue: CAS-5013
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: 
         Test Programs: 
  Put in Release Notes: No
             Module(s): mstransform
           Description: Added tests for case when the SPWs to be combined have different EXPOSURE and WEIGHT_SPECTRUM has to be used to combine them

------------------------------------------------------------------------
r24965 | sandra.castro | 2013-06-14 15:31:21 +0200 (Fri, 14 Jun 2013) | 13 lines


       New Development: Yes
            JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: Yes
What Interface Changed: realmodelcol sub-parameter moved to under datacolumn
         Test Programs: test_mstransform
  Put in Release Notes: No
             Module(s): mstransform
           Description: Modified the interface and placed the sub-parameter realmodelcol under some values of the datacolumn parameter. This parameter can be set for values 'all','model','data,model,corrected'. When set to True, it will make real a virtual MODEL column and write it to the output MS/MMS.

	

------------------------------------------------------------------------
r24797 | justo.gonzalez | 2013-06-04 18:49:02 +0200 (Tue, 04 Jun 2013) | 11 lines


       New Development: no
            JIRA Issue: CAS-4901
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: 
         Test Programs: 
  Put in Release Notes: No
             Module(s): mstransform
           Description: - Starting integration of time average code into mstransform framework 

------------------------------------------------------------------------
r23894 | sandra.castro | 2013-04-17 19:19:28 +0200 (Wed, 17 Apr 2013) | 14 lines


       New Development: No
            JIRA Issue: CAS-5014
        Ready for Test: Yes
     Interface Changes: Yes
What Interface Changed: changed the default of separationaxis
         Test Programs: test_partition
  Put in Release Notes: No
             Module(s): partition task
           Description: Changed the default of separationaxis to both. This forced me to catch when NULL
           MS selection would lead to empty subMSs in disk. Created a method to check if scan/spw selections
           are valid. If not, then do not send this job to an engine. 
	

------------------------------------------------------------------------
r23856 | sandra.castro | 2013-04-16 11:01:16 +0200 (Tue, 16 Apr 2013) | 14 lines


       New Development: No
            JIRA Issue: CAS-5014
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_partition
  Put in Release Notes: No
             Module(s): 
           Description: Changed the parameter taskname to thistask in
		   setupCluster() because it fails when there is no cluster available.

	

------------------------------------------------------------------------
r23839 | sandra.castro | 2013-04-15 15:18:16 +0200 (Mon, 15 Apr 2013) | 12 lines


       New Development: No
            JIRA Issue: CAS-5014
        Ready for Test: Yes
     Interface Changes: Yes
What Interface Changed: new parameters parallel and flagbackup. Removed calmsselection and timebin.
         Test Programs: test_partition
  Put in Release Notes: No
             Module(s): 
           Description: Changed partition so that it calls the MSTHelper class to do all the job. It uses the MSTransform framework for all the computation. I have also added two new parameters, parallel and flagbackup. I have also removed the parameters calmsselection (and sub-parameters) and timebin.
	

------------------------------------------------------------------------
r23384 | scott.rankin | 2013-03-22 19:52:50 +0100 (Fri, 22 Mar 2013) | 72 lines

Merged revisions 23339-23340,23367-23368 via svnmerge from 
https://svn.cv.nrao.edu/svn/casa/branches/release-4_1_0/gcwrap

........
  r23339 | sandra.castro | 2013-03-21 03:39:13 -0600 (Thu, 21 Mar 2013) | 12 lines
  
  
         New Development: No
              JIRA Issue: CAS-4755
          Ready for Test: Yes
       Interface Changes: No
  What Interface Changed: Please list interface changes
           Test Programs: test_mst_cvel
    Put in Release Notes: No
               Module(s): 
             Description: Fixed the phasecenter parameter parsing in the task.
  	
........
  r23340 | sandra.castro | 2013-03-21 08:30:14 -0600 (Thu, 21 Mar 2013) | 14 lines
  
  
         New Development: No
              JIRA Issue: CAS-4755
          Ready for Test: Yes
       Interface Changes: No
  What Interface Changed: Please list interface changes
           Test Programs: test_mst_cvel
    Put in Release Notes: No
               Module(s): 
             Description: Removed empty string parameters from the configuration
             dictionary before parsing it to the tool. Reset the start_p and width_p values
             in MSTransformDataHandler::parseFreqSpecParams() also for mode=velocity.
  	
........
  r23367 | takeshi.nakazato | 2013-03-22 02:05:45 -0600 (Fri, 22 Mar 2013) | 19 lines
  
  
         New Development: No
              JIRA Issue: No
          Ready for Test: Yes
       Interface Changes: No
  What Interface Changed: Please list interface changes
           Test Programs: default(sdcal2)
                          calmode='tsys'
                          inp()
                          see if subparameter 'tsysiflist' shows up or not
    Put in Release Notes: No
               Module(s): Module Names change impacts.
             Description: Describe your changes here...
  
  Bug fix in xml interface definition.
  
  	
........
  r23368 | takeshi.nakazato | 2013-03-22 02:07:01 -0600 (Fri, 22 Mar 2013) | 16 lines
  
  
         New Development: No
              JIRA Issue: No
          Ready for Test: Yes
       Interface Changes: No
  What Interface Changed: online help updated
           Test Programs: List test programs
    Put in Release Notes: No
               Module(s): Module Names change impacts.
             Description: Describe your changes here...
  
  Updated online help
  
  	
........

------------------------------------------------------------------------
r23340 | sandra.castro | 2013-03-21 15:30:14 +0100 (Thu, 21 Mar 2013) | 14 lines
Merged via: r23384


       New Development: No
            JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mst_cvel
  Put in Release Notes: No
             Module(s): 
           Description: Removed empty string parameters from the configuration
           dictionary before parsing it to the tool. Reset the start_p and width_p values
           in MSTransformDataHandler::parseFreqSpecParams() also for mode=velocity.
	

------------------------------------------------------------------------
r23339 | sandra.castro | 2013-03-21 10:39:13 +0100 (Thu, 21 Mar 2013) | 12 lines
Merged via: r23384


       New Development: No
            JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mst_cvel
  Put in Release Notes: No
             Module(s): 
           Description: Fixed the phasecenter parameter parsing in the task.
	

------------------------------------------------------------------------
r23326 | scott.rankin | 2013-03-20 20:55:43 +0100 (Wed, 20 Mar 2013) | 62 lines

Merged revisions 23282,23285-23286,23314 via svnmerge from 
https://svn.cv.nrao.edu/svn/casa/branches/release-4_1_0/gcwrap

........
  r23282 | tak.tsutsumi | 2013-03-18 16:25:28 -0600 (Mon, 18 Mar 2013) | 14 lines
  
  
         New Development: No
              JIRA Issue: Yes CAS-4590
          Ready for Test: Yes
       Interface Changes: No
  What Interface Changed: Please list interface changes
           Test Programs: List test programs
    Put in Release Notes: No
               Module(s): Module Names change impacts.
             Description: Previous modification appears to re-introduce the past bug 
  
  	
........
  r23285 | sandra.castro | 2013-03-19 06:11:54 -0600 (Tue, 19 Mar 2013) | 12 lines
  
  
         New Development: No
              JIRA Issue: CSV-2576
          Ready for Test: Yes
       Interface Changes: No
  What Interface Changed: Please list interface changes
           Test Programs: test_importasdm
    Put in Release Notes: No
               Module(s): mstransform
             Description: Added a test to catch this problem with flagging autocorrelations.
  	
........
  r23286 | sandra.castro | 2013-03-19 06:37:53 -0600 (Tue, 19 Mar 2013) | 12 lines
  
  
         New Development: No
              JIRA Issue: No
          Ready for Test: Yes
       Interface Changes: No
  What Interface Changed: Please list interface changes
           Test Programs: test_flagdata
    Put in Release Notes: No
               Module(s): 
             Description: Changes in the VI2 caused the log messages about the chunks to display an array for arrayid. I've changed it to print only the first element of the array.
  	
........
  r23314 | sandra.castro | 2013-03-20 08:36:43 -0600 (Wed, 20 Mar 2013) | 12 lines
  
  
         New Development: No
              JIRA Issue: CAS-4755
          Ready for Test: Yes
       Interface Changes: No
  What Interface Changed: Please list interface changes
           Test Programs: test_mst_split
    Put in Release Notes: No
               Module(s): 
             Description: Fixed the timerange parameter parsing in the task.
  	
........

------------------------------------------------------------------------
r23314 | sandra.castro | 2013-03-20 15:36:43 +0100 (Wed, 20 Mar 2013) | 12 lines
Merged via: r23326


       New Development: No
            JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mst_split
  Put in Release Notes: No
             Module(s): 
           Description: Fixed the timerange parameter parsing in the task.
	

------------------------------------------------------------------------
r23211 | sandra.castro | 2013-03-13 14:27:27 +0100 (Wed, 13 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: Yes
What Interface Changed: renamed freqaverage to chanaverage and freqbin to chanbin
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): 
           Description: Renamed parameters and added more usage examples.
------------------------------------------------------------------------
r23191 | sandra.castro | 2013-03-12 16:18:23 +0100 (Tue, 12 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: 
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): 
           Description: Added support for when freqbin is a list, both for MS and MMS. Added better error handling and changed the return value of the task to True when creating an MMS. Added new tests for freqaverage and MMS creation.
------------------------------------------------------------------------
r23158 | sandra.castro | 2013-03-08 17:06:28 +0100 (Fri, 08 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: Yes
What Interface Changed: changed type of tileshape
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): 
           Description: Changed the type of parameter tileshape. It is a list with one or three int elements.
------------------------------------------------------------------------
r23155 | sandra.castro | 2013-03-08 14:06:05 +0100 (Fri, 08 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: Yes
What Interface Changed: new parameter realmodelcol
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): 
           Description: Added the parameter realmodelcol to add a virtual MODEL column to the output MS.
------------------------------------------------------------------------
r23153 | sandra.castro | 2013-03-08 10:55:53 +0100 (Fri, 08 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): 
           Description: Added a sub-parameter to createmms called parallel, which when set to True run the task in parallel. By default it is set to False to run in sequential. Added new tests for this case,
------------------------------------------------------------------------
r23138 | sandra.castro | 2013-03-07 17:37:49 +0100 (Thu, 07 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): 
           Description: Removed ddistart from when separationaxis='scan'. Cleaned up a bit the task code. Added a new parameter ignoreflags to verifyMS. Removed duplicated tests from test_mstransform that already exist in test_mst_cvel. Updated test_mst_partition with re-indexed values for spws.
------------------------------------------------------------------------
r23135 | sandra.castro | 2013-03-06 11:38:25 +0100 (Wed, 06 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mst_partition
  Put in Release Notes: No
            Module(s): 
           Description: Final consolidation of spw/ddi sub-tables is included. Fixed a bug in the ddistart when separationaxis=both. The parameter ddistart is now set to hidden in the XML file. Fixed tests that needed re-indexed spw ids. Added a new class of tests for MMS testing.
------------------------------------------------------------------------
r23118 | sandra.castro | 2013-03-05 18:03:07 +0100 (Tue, 05 Mar 2013) | 10 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): 
           Description: Added a method to consolidate the spw/data_description subtables. Now the
           created MMS have consistent spw sub-tables.
------------------------------------------------------------------------
r23116 | sandra.castro | 2013-03-05 14:44:04 +0100 (Tue, 05 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mst_partition
  Put in Release Notes: No
            Module(s): 
           Description: Added support for channel selection when creating an MMS.
------------------------------------------------------------------------
r23101 | sandra.castro | 2013-03-04 10:11:00 +0100 (Mon, 04 Mar 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mst_partition
  Put in Release Notes: No
            Module(s): 
           Description: Removed createmms=false.
------------------------------------------------------------------------
r23087 | sandra.castro | 2013-03-01 18:30:08 +0100 (Fri, 01 Mar 2013) | 14 lines


       New Development: No
            JIRA Issue: CAS-4866
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mst_split
  Put in Release Notes: No
             Module(s): mstransform
           Description: Added a block of code to udpate the FLAG_CMD subtable
		   from split. Enabled tests to verify this.

	

------------------------------------------------------------------------
r23081 | sandra.castro | 2013-03-01 12:17:02 +0100 (Fri, 01 Mar 2013) | 18 lines


       New Development: No
            JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mst_cvel
  Put in Release Notes: No
             Module(s): mstransform
           Description: Changed the default for start and width when mode is
		   not channel. Adding all the necessary methods for partition, but they
		   are disabled for the task until the tests are finished. I am forcing
		   createmms=False in this version. This version also contains a
		   temporary parameter called ddistart. It will be hidden once the XML
		   schema is changed.

	

------------------------------------------------------------------------
r23017 | sandra.castro | 2013-02-26 16:49:56 +0100 (Tue, 26 Feb 2013) | 13 lines


       New Development: No
            JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: 
  Put in Release Notes: No
             Module(s): mstransform
           Description: Renamed parameter nspws to nspw in task and xml. Avoid passing empty string parameters to the config dictionary.

	

------------------------------------------------------------------------
r22985 | sandra.castro | 2013-02-22 10:05:32 +0100 (Fri, 22 Feb 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: Yes
            Module(s): 
           Description: Changed the default of freqbin to 1, to match split. Added the possibility to have a list of freqbin to the task.
------------------------------------------------------------------------
r22984 | sandra.castro | 2013-02-22 09:48:53 +0100 (Fri, 22 Feb 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: Yes
            Module(s): 
           Description: Changed the interface of the task. Removed the separatespws parameter. Added a sub-parameter nspws to the regridms transformation. Added examples in the XML on how to run the task.
------------------------------------------------------------------------
r22867 | sandra.castro | 2013-02-12 16:41:17 +0100 (Tue, 12 Feb 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): mstransform
           Description: Added missing correlation parameter in config dictionary.
------------------------------------------------------------------------
r22866 | sandra.castro | 2013-02-12 14:35:09 +0100 (Tue, 12 Feb 2013) | 9 lines

      New Development: Yes
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): mstransform
           Description: Enabled frequency averaging transformation in mstransform task. Added initial tests for this transformation. Enabled test_hanning2 after the seg fault was fixed in CAS-4866. Updated the inline help.
------------------------------------------------------------------------
r22844 | sandra.castro | 2013-02-11 16:37:49 +0100 (Mon, 11 Feb 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): mstransform
           Description: Added more tests for the regridms and Hanning transformations.
------------------------------------------------------------------------
r22831 | sandra.castro | 2013-02-08 14:52:36 +0100 (Fri, 08 Feb 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): mstransform
           Description: Added more tests for the combinespws and regridms transformations.
------------------------------------------------------------------------
r22823 | sandra.castro | 2013-02-07 16:46:53 +0100 (Thu, 07 Feb 2013) | 9 lines

      New Development: No
           JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
            Module(s): mstransform
           Description: The phasecenter parameter does not need a string casting. Added a new helper function in testhelper.py to get the frequencies of a list of channels in an MS. Added more tests for the regridms transformation.
------------------------------------------------------------------------
r22819 | sandra.castro | 2013-02-07 11:59:52 +0100 (Thu, 07 Feb 2013) | 13 lines


       New Development: No
            JIRA Issue: CAS-4755
        Ready for Test: Yes
     Interface Changes: No
What Interface Changed: Please list interface changes
         Test Programs: test_mstransform
  Put in Release Notes: No
             Module(s): Module Names change impacts.
           Description: Casting some of the regridms parameters to string before parsing them to the tool. Enabled the regridms transformation in the task. Added a few tests for this mode.

	

------------------------------------------------------------------------
r22388 | scott.rankin | 2012-12-25 18:55:31 +0100 (Tue, 25 Dec 2012) | 3 lines

Converting CASA repository to follow common Subversion conventions.


------------------------------------------------------------------------


