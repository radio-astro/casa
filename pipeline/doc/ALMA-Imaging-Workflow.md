ALMA interferometry imaging workflow

- hif_makeimlist

  - minimum task parameters:
    - intent (default to 'TARGET')
    - spw (defaults to science spws)
    - specmode (defaults to 'cube' for 'TARGET', else 'mfs')

  - create list of imaging targets with these parameters per target set:
    - heuristics instance
    - field
    - spw (for specmode='cont' remove spws without data -> re-use for VLASS)
    - cell (common size for all targets of one hif_makeimlist call (min of all))
    - phasecenter
    - imsize (common size for all targets of one hif_makeimlist call (max of all))
    - specmode
    - nchan (pilot image)
    - width (pilot image)
    - imagename
    - LSRK spw selection if cont.dat / lines.dat exists
    - nbin (if set by hif_checkproductsize)
    - gridder ('standard' or 'mosaic')
    - stokes 'I'
    - start (task parameter if set)
    - uvrange (task parameter if set)


- hif_makeimages

  - set additional parameters via task interface:
    - hm_masking ('centralregion' or 'auto')
    - hm_autotest
    - hm_cleaning ('rms')
    - maxncleans

  - run hif_tclean per imaging target (HPC: one job per calibrator image or parallel=True for tclean for intent='TARGET')

- hif_tclean

  - calculate PB limits to define annuli (start with 0.2/0.3, adjust to imsize)
  - determine deconvolver ('mtmfs' for 'cont' if fractional bandwidth > 10% else 'hogbom')
  - determine nterms (2 for deconvolver='mtmfs' else None)
  - determine start, width, nchan for 'cube' (uses LSRK overlap and skips edge channels)
  - determine TOPO spw selections per MS
  - determine aggregate LSRK and TOPO bandwidths
  - calculate sensitivity
    - median of field ID sensitivities because sensitivity calculator takes only one ID at a time
    - corrections for effective channel width and mosaic overlap
  - calculate initial threshold estimate as tlimit * sensitivity
  - make dirty cube
  - make pb based mask using annuli if not 'auto'
  - calculate dirty cube statistics using annuli
  - calculate dirty dynamic range (max(residual) / sensitivity)
  - adjust threshold using dirty dynamic range heuristics
  - caclulate niter using beam / mask size heuristics
  - copy iter0 products to iter1 names
  - make clean cube
  - calculate clean cube statistics using annuli
