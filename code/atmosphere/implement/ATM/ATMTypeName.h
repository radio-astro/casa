namespace atm{
  
  // must be in the same order as for the enumeration
  string  AtmosphereType::type_[] = { "tropical",
				      "midlatSummer", 
				      "midlatWinter",
				      "subarcticSummer",  
				      "subarcticWinter" };
  
  
  string  AtmosphereType::name(int n){
    if(n>4)return "";
    return type_[n];
  }
  
  string  AtmosphereType::name(Atmospheretype n){
    return type_[n-1];
  }
  
}
