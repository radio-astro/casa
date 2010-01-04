/** \file */
#if     !defined(ATM_ATMOSPHERETYPE_H)
/**  \brief Enumeration of basic atmosphere types.
 */
// values must start from 1 and go by step of 1
/** Enumerated list of atmosphere types: there are 5 available types. */



namespace atm {
  
  enum Atmospheretype{ tropical=1,        TROPICAL=1,          //!< tropical
		       midlatSummer=2,    MIDLATSUMMER=2,      //!< midlatSummer
		       midlatWinter=3,    MIDLATWINTER=3,      //!< midlatWinter
		       subarcticSummer=4, SUBARCTICSUMMER=4,   //!< subarcticSummer
		       subarcticWinter=5, SUBARCTICWINTER=5};  //!< subarcticWinter
  
  
  // Pure static class
  using namespace std;
  
#include <string>
  
  
  /** AtmosphereType is a pure static class (a singleton) giving the list
      of atmosphere types available when using the ATM library */
  class AtmosphereType{
  public:
    
    //@{ Accessors:
    /** Accessor to the name of an atmosphere type for a given index in the list of atmosphere
	types */
    static string name(int n);
    
    /** Accessor to the name of an atmosphere type for a given atmosphere
	type in the enumerated list typed Atmospheretype */
    static string name(Atmospheretype);
    //@}
    
    AtmosphereType(){};
  private:
    static string  type_[];
    
  };
  
}

#define ATM_ATMOSPHERETYPE_H
#endif


