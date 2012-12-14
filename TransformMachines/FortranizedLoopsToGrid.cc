{
  //  T *gridPtr;
  Complex *cfPtr, *phaseGradPtr;
  Int *supportPtr, *cfShapePtr,
    *locPtr, *igrdposPtr, *ilocPtr, *tilocPtr,
    *convOriginPtr;
  Float  *samplingPtr;
  Double *offPtr;
  Bool dummy;

  //  gridPtr       = grid.getStorage(dummy);
  cfPtr         = convFuncV;
  phaseGradPtr  = cached_phaseGrad_p.getStorage(dummy);
  supportPtr    = support.getStorage(dummy);
  samplingPtr   = sampling.getStorage(dummy);
  cfShapePtr    = cfShape.getStorage(dummy);
  locPtr        = loc.getStorage(dummy);
  igrdposPtr    = igrdpos.getStorage(dummy);
  ilocPtr       = iloc.getStorage(dummy);
  tilocPtr      = tiloc.getStorage(dummy);
  offPtr        = off.getStorage(dummy);
  convOriginPtr = convOrigin.getStorage(dummy);
  Int finitePointingOffsets_int = (finitePointingOffsets?1:0),
    psfOnly_int = (psfOnly?1:0);
  Int cf0=cfShape(0), cf1=cfShape(1), cf2=cfShape(2), cf3=cfShape(3);
  Int gnx = nx, gny = ny, gnp = nGridPol, gnc=nGridChan;
  Int phx=cached_phaseGrad_p.shape()[0], phy=cached_phaseGrad_p.shape()[1];

  //
  // Call the FORTRAN function with the gridding inner-loops (in synthesis/fortran/faccumulateToGrid.f)
  //
  faccumulatetogrid_(gridStore, 
		     convFuncV, 
		     &nvalue, 
		     &wVal,
		     supportPtr,samplingPtr,offPtr, convOriginPtr,
		     cfShapePtr,locPtr,igrdposPtr, 
		     &sinDPA, &cosDPA,
		     &finitePointingOffsets_int, 
		     &psfOnly_int,
		     &norm, phaseGradPtr,
		     &gnx, &gny, &gnp, &gnc,
		     &cf0, &cf1, &cf2, &cf3,
		     &phx, &phy);
		     
		     // &nx,&ny,&nGridPol, &nGridChan,
		     // &unity, &unity, &unity, &unity,
		     // &cfShape(0),&cfShape(1),&cfShape(2),&cfShape(3),
		     // &(cached_phaseGrad_p.shape()[0]),
		     // &(cached_phaseGrad_p.shape()[1]));
}
