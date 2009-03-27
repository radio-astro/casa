      EXTERNAL ap_to_kp, ap_to_kptab
      REAL     ap_to_kp, ap_to_kptab
      INTEGER  n0day81,maxdays,imfna
      REAL*8   pi,degrad,nul,een,twee
      PARAMETER (pi = 3.14159265358979323846d0)
      PARAMETER (degrad = pi / 1.8d2)
      PARAMETER (nul=0.d0, een=1.d0, twee=2.d0)
      PARAMETER (maxdays=14)
      REAL*8  bydp,bysig,bzdp,bzsig
      CHARACTER*80 pkpf,pimf,pgdb
      
     
      COMMON/PIMWRAPPER/bydp,bysig,bzdp,bzsig,
     1 imfna,n0day81
      COMMON/PIMPATHS/pkpf,pimf,pgdb
