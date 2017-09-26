# Pipeline Flagging Tasks

The Pipeline contains a number of tasks that flag bad data, based on e.g. online flags or user-provided flags. A 
sub-set of the pipeline flagging tasks will flag data based on evaluation of their own flagging heuristics, and a 
majority of this sub-set use a common Pipeline Flagging framework. The following note provides a description of the
common flagging framework, and an in-depth explanation of individual flagging tasks. 

## Summary current pipeline tasks

Status as of September 2017:

The following pipeline tasks make use of the flagging framework and are part of (at least) one of the standard pipeline recipes:
- hif_rawflagchans: flags raw data in MS.
- [hifa/hsd]_tsysflag: flags Tsys caltable created by [hifa/hsd]_tsyscal.
- hif_lowgainflag: flags antennas in MS, based on evaluating heuristics on gain amplitude caltable created within task.
- hifa_wvrgcalflag: evaluates if applying WVR correction provides tangible improvement, flagging WVR caltable in process where needed, using WVR caltable created within task (using hifa_wvrgcal).

The following pipeline tasks make use of the flagging framework, but are not presently part of standard pipeline recipes:
- hif_atmflag: flags channels where the atmospheric transmission is low
- hif_gainflag: flags antennas in MS, based on evaluating heuristics on gain amplitude caltable created within task.

Finally, the following are pipeline tasks that perform flagging, but that do not make use of the common flagging framework:
- hif_correctedampflag: Flag corrected - model amplitudes based on calibrators.
- hifa_bandpassflag: uses hif_correctedampflag to flag corrected-model amplitude outliers for the bandpass calibrator.
- hifa_gfluxscaleflag: uses hif_correctedampflag to flag corrected-model amplitude outliers for the phase and flux calibrators.
- hifa_flagdata (unused in standard recipes): performs basic deterministic flagging on MS (e.g. shadowed antennas, online flags, autocorrelations, etc)
- hifa_fluxcalflag (unused in standard recipes): locates and flags line regions in solar-system flux calibrators.


## General overview of a flagging task within framework

The main flagging pipeline tasks are done in two steps:

1. Generate a "flagging view"

   Some quantity of interest is taken from the MS or a caltable, depending on the flagging task, and this quantity is represented in a 1D or 2D matrix called the "flagging view". Flagging views are typically generated separately for each spw, often also separately for each polarisation. Typical examples of flagging views include:
   - measured data as function of Antenna1 and time (2D, e.g. Tsysflag)
   - measured data as function of channel and baseline (2D, e.g. Rawflagchans)
   - median normalized Tsys as function of channel (1D, see Tsysflag)

2. Flag the "view"
   The flagging view is evaluated against a series of 1 or more flagging rules. Each flagging rule applies a heuristic to the flagging view to identify what elements in the view need to be flagged, and creates a corresponding Flag Command. 
   
   In a single flagging pipeline task, a list of Flag Commands are compiled from the evaluation of all flagging views, and these flagging commands are immediately applied to the appropriate MS or caltable before the end of the flagging task.


## How individual flagging tasks work

### Task: hif_rawflagchans
This task uses raw data for the intent = 'BANDPASS' to identify outliers. Flagging commands for outlier data points are applied to all intents in the MS.

**View generation**

Rawflagchans creates a separate view for each spw and each polarisation. Each view is created by calculating the following:
- calculate average of measured data as function of baseline vs. channel
- for each baseline, subtract the median value within this baseline value from this baseline
- for each channel, subtract the median value within this channel from this channel

**View flagging**

The views are evaluated against the following two flagging rules:

1. "bad quadrant" matrix flagging rule

   This starts with the "baseline" vs. "channel" flagging view. In this view, some data points may already be flagged, e.g. due to an earlier pipeline stage.
   
   First, outliers are identified as those data points in the flagging view whose value deviates from the median value of all non-flagged data points by a threshold-factor * the median absolute deviation of the values of all non-flagged data points, where the threshold is 'fbq_hilo_limit' (default: 8.0). 
   
   In formula: 
   
   ```flagging mask = (data - median(all non-flagged data)) > (MAD(all non-flagged data) * fbq_hilo_limit)```
   
   Next, the flagging view is considered as split up in 4 channel quadrants, and each antenna is evaluated separately as follows:
   - select baselines belonging to antenna and select channels belonging to quadrant
   - determine number of newly found outlier datapoints within selection
   - determine number of originally unflagged datapoints within selection
   - determine fraction of "# of newly found outliers" over "# of originally unflagged datapoints"
   - if the latter fraction exceeds the fraction threshold 'fbq_antenna_frac_limit' (default: 0.2), then a flagging command is generated that will flag all channels within the evaluated quadrant for the evaluated antenna.
   - otherwise, no action is taken (i.e. the newly found outlier datapoints are not individually flagged by this rule)

   Next, the flagging view is still considered as split up in 4 channel quadrants, and each baseline is evaluated separately, as follows:
   - select baseline and select channels belong to quadrant
   - determine number of newly found outlier datapoints within selection
   - determine number of originally unflagged datapoints within selection
   - determine fraction of "# of newly found outliers" over "# of originally unflagged datapoints"
   - if the latter fraction exceeds the fraction threshold 'fbq_baseline_frac_limit' (default: 1.0), then a flagging command is generated that will flag all channels within the evaluated quadrant for the evaluated baseline.
   - otherwise, no action is taken (i.e. the newly found outlier datapoints are not individually flagged by this rule)

2. "outlier" matrix flagging rule

   Data points in the flagging view are identified as outliers if their value deviates from the median value of all non-flagged data points by a threshold-factor * the median absolute deviation of the values of all non-flagged data points, where the threshold is 'fhl_limit' (default: 20.0).
   
   In formula: 
   
   ```flagging mask = (data - median(all non-flagged data)) > (MAD(all non-flagged data) * fhl_limit)```

   Flagging commands are generated for each of the identified outlier data points.
   
   If the number of data points in the flagging view are smaller than the minimum sample 'fhl_minsample' (default: 5), then no flagging is attempted.

### Task: hif_lowgainflag

This task first creates a phased-up bandpass caltable, then a gain phase caltable, and finally a gain amplitude caltable. This final gain amplitude caltable is used to identify antennas with outlier gains, for each spw. Flagging commands for outlier antennas (per spw) are applied to the entire MS.

**View generation**

A separate view is created for spw. Each view is a matrix with axes "time" vs. "antenna". Each point in the matrix is the absolute gain amplitude for that antenna/timestamp.

**View flagging**

The views are evaluated against the "nmedian" matrix flagging rule, where data points are identified as outliers if:

1. their value is smaller than a threshold-factor * median of all non-flagged data points, where the threshold is 'fnm_lo_limit' (default: 0.7), or,
2. their value is larger than a threshold-factor * median of all non-flagged data points, where the threshold is 'fnm_hi_limit' (default: 1.3).

Flagging commands are generated for each of the identified outlier data points.

### Task: hifa_tsysflag, hsd_tsysflag

This task flags the Tsys cal table created by the [hifa/hsd]_tsyscal pipeline task.

Tsysflag provides six separate flagging metrics, where each metric creates its own flagging view and has its own corresponding flagging rule(s).

In the current standard pipeline, all six metrics are active, and evaluated in the order set by "metric_order" (default: 'nmedian, derivative, edgechans, fieldshape, birdies, toomany').

- Metric 1: "nmedian"
   
   **View generation**
   
   A separate view is generated for each polarisation and each spw. Each view is a matrix with axes "time" vs. "antenna". Each point in the matrix is the median value of the Tsys spectrum for that antenna/time.
   
   **View flagging**
   
   The views are evaluated against the "nmedian" matrix flagging rule, where data points are identified as outliers if their value is larger than a threshold-factor * median of all non-flagged data points, where the threshold is 'fnm_limit' (default: 2.0).
   
   Flagging commands are generated for each of the identified outlier data points.

- Metric 2: "derivative"

   **View generation**

   A separate view is generated for each polarisation and each spw. Each view is a matrix with axes "time" vs. "antenna". Each point in the matrix is calculated as follows:
   - calculate "valid_data" as the channel-to-channel difference in Tsys for that antenna/timestamp (for unflagged channels)
   - calculate ```median( abs( valid_data - median(valid_data) ) ) * 100.0```
   
   **View flagging**
   
   The views are evaluated against the "max abs" matrix flagging rule, where data points are identified as outliers if their absolute value exceeds the threshold "fd_max_limit" (default: 5).
   
   Flagging commands are generated for each of the identified outlier data points.

- Metric 3: "edgechans"

   **View generation**

   A separate view is generated for each spw and each of these intents: ATMOSPHERE, BANDPASS, and AMPLITUDE. Each view contains a "median" Tsys spectrum where for each channel the value is calculated as the median value of all selected (spw,intent) Tsys spectra in that channel (this combines data from all antennas together).

   **View flagging**

   The views are evaluated against the "edges" vector flagging rule, which flags all channels from the outmost edges (first and last channel) until the first channel for which the channel-to-channel difference first falls below a threshold * the median channel-to-channel difference, where the threshold is "fe_edge_limit" (default: 3.0).
   
   A single flagging command is generated for all channels newly identified as "edge channels".

- Metric 4: "fieldshape"

   **View generation** 
   
   A separate view is generated for each spw and each polarisation. Each view is a matrix with axes "time" vs. "antenna". Each point in the matrix is a measure of the difference of the Tsys spectrum for that time/ant from the median of all Tsys spectra for that ant/spw in the "reference" fields that belong to the reference intent specified by "ff_refintent" (default: "BANDPASS").

   The exact fieldshape value is calculated as:
   
   ```metric = 100 * mean(abs(normalised tsys - reference normalised tsys))```
   
   where a 'normalised' array is defined as: "array / median(array)"

   **View flagging**

   The views are evaluated against the "max abs" matrix flagging rule, where data points are identified as outliers if their absolute value exceeds the threshold "ff_max_limit" (default: 5).

- Metric 5: "birdies"

   **View generation**

   A separate view is generated for each spw and each antenna. Each view contains a "difference" Tsys spectrum calculated as:
"channel-by-channel median of Tsys spectra for antenna within spw" - "channel-by-channel median of Tsys spectra for all antennas within spw".

   **View flagging**

   The views are evaluated against the "sharps" vector flagging rule, which flags each view in two passes:
   1. flag all channels whose absolute difference in value to the following channel exceeds a threshold "fb_sharps_limit" (default: 0.05)
   2. around each newly flagged channel, flag neighbouring channels until their channel-to-channel difference falls below 2 times the median channel-to-channel difference (this is intended to flag the wings of sharp features)
    
   A single flagging command is generated for all channels newly identified as "birdies".

- Metric 6: "toomany"
   
   **View generation**

   A separate view is generated for each polarisation and each spw. Each view is a matrix with axes "time" vs. "antenna". Each point in the matrix is the median value of the Tsys spectrum for that antenna/time. (This is the same as for "nmedian" metric).

   **View flagging**

   The views are evaluated against two separate flagging rules:
   1. "tmf" (too many flags)
   
      This evaluates each timestamp one-by-one, flagging an entire timestamp when the fraction of flagged antennas within this timestamp exceeds the threshold "tmf1_limit" (default: 0.666).

      Flagging commands are generated per timestamp.

   2. "tmef" (too many entirely flagged)

      This evaluates all timestamps at once, flagging all antennas for all timestamps within current view (spw, pol) when the fraction of antennas that are entirely flagged in all timestamps exceeds the threshold "tmef1_limit" (default: 0.666).
      
      Flagging commands are generated for each data point in the view that is newly flagged.
      
### Task: hif_gainflag
Since Cycle 5, this task is not part of any standard recipe (replaced by hifa_bandpassflag and hifa_gfluxscaleflag).

This task first creates a phased-up bandpass caltable, then a gain phase caltable, and finally a gain amplitude caltable. This final gain amplitude caltable is used to identify antennas with outlier gains, for each spw. Flagging commands for outlier antennas (per spw) are applied to the entire MS.

Gainflag offers two separate flagging metrics, where each metric creates its own flagging view and has its own corresponding flagging rule(s). In the latest pipeline release (C4R1), only the "rmsdeviant" metric is active.

- Metric 1: "rmsdeviant" (enabled in C4R1):

   **View generation**

   A separate view is created for each spw. Each view is a matrix with axes "time" vs. "antenna". Each point in the matrix is the "standard deviation of the gain amplitudes for that antenna and all timestamps, divided by the median absolute deviation of the gain amplitude for all antennas and all timestamps".
   
   **View flagging**
   
   The views are evaluated against the "max abs" matrix flagging rule, where data points are identified as outliers if their absolute value exceeds the threshold "frmsdev_limit" (default: 8.0).
   
   Flagging commands are generated for each of the identified outlier data points.

- Metric 2: "mediandeviant" (disabled in C4R1)
   
   **View generation**
   
   A separate view is created for each spw. Each view is a matrix with axes "time" vs. "antenna". Each point in the matrix is calculated as follows: 
   
   ```abs(median(antenna) - median(all antennas)) / mad(all antennas)```
   
   wherein each quantity is calculated across all timestamps.
   
   **View flagging**
   
   The views are evaluated against the "max abs" flagging rule, where data points are identified as outliers if their absolute value exceeds the threshold "fmeddev_limit" (default: 3.0).
   
   Flagging commands are generated for each of the identified outlier data points.
