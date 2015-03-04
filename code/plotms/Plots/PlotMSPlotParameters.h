//# PlotMSPlotParameters.h: Parameter classes for PlotMSPlot classes.
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#ifndef PLOTMSPLOTPARAMETERS_H_
#define PLOTMSPLOTPARAMETERS_H_

#include <casa/Containers/Record.h>
#include <graphics/GenericPlotter/PlotFactory.h>
#include <plotms/PlotMS/PlotMSWatchedParameters.h>

#include <casa/namespace.h>
#include <QDebug>

namespace casa {

//# Forward Declarations.
class PlotMSApp;


// Parameters for a PlotMSPlot.  Parameters for different plot types are
// defined by subparameter groups, which are in the PlotMSPlotParameterGroups
// files.
class PlotMSPlotParameters : public PlotMSWatchedParameters {
    
public:
    // Static //
    
    // Abstract superclass representing a "group" of subparameters of a
    // PlotMSPlotParameters object.
    class Group {
        
        //# Friend class declarations.
        friend class PlotMSPlotParameters;
        
    public:
        // Constructor which takes a plot factory.
        Group(PlotFactoryPtr factory);
        
        // Copy constructor.  Does NOT copy parameter values using operator=().
        Group(const Group& copy);
        
        // Destructor.
        virtual ~Group();
        
        
        // Abstract Methods //
        
        // Returns a clone (deep copy) of this group of subparameters.
        virtual Group* clone() const = 0;
        
        // Returns the name of this group of subparameters.  Will be used as an
        // update flag.
        virtual const String& name() const = 0;
        
        // Returns a Record representing this group of subparameters.
        virtual Record toRecord() const = 0;
        
        // Sets the values for this group of subparameters using the given
        // Record.
        virtual void fromRecord(const Record& record) = 0;
        
        // Returns true if the plot needs to be redrawn when subpamarameters in
        // this group have changed, false otherwise.
        virtual bool requiresRedrawOnChange() const = 0;
        
        
        // Implemented Methods //
        
        // Copy operator.  Should probably be overridden by children to be more
        // efficient.  Only copies if the two groups have the same name.
        virtual Group& operator=(const Group& other);
        
        // Equality operators.  Should probably be overridden by children to be
        // more efficient.
        virtual bool operator==(const Group& other) const;
        virtual bool operator!=(const Group& other) const {
            return !(operator==(other)); }
        
    protected:
        // Should be called by the child class whenever any of the
        // subparameter values have been updated.
        // <group>
        void updated() { updated(requiresRedrawOnChange()); }
        void updated(bool requiresRedraw);
        // </group>
        
        // Returns the parent parameters, or NULL for none.
        // <group>
        PlotMSPlotParameters* parent() { return itsParent_; }
        const PlotMSPlotParameters* parent() const { return itsParent_; }
        // </group>
        
        // Returns the factory.
        // <group>
        PlotFactoryPtr factory() { return itsFactory_; }
        const PlotFactoryPtr factory() const { return itsFactory_; }
        // </group>
        
    private:
        // Parent of this group of subparameters.
        PlotMSPlotParameters* itsParent_;
        
        // Factory.
        PlotFactoryPtr itsFactory_;
        
        
        // Post-thread method for notifying watchers that this group has
        // changed.
    public:
		static void
		notifyWatchers (void *obj, bool wasCanceled)
		{
			Group *cobj = static_cast < Group * >(obj);
			if (cobj != NULL)
				cobj->notifyWatchers_ (wasCanceled);
		}
	private:
		void notifyWatchers_ (bool wasCanceled);
		
	
	    };
	    
    //# Friend class declarations.
    friend class PlotMSPlotParameters::Group;
    
    
    // Non-Static //
    
    // Constructor, which starts out with no subparameter groups.
    PlotMSPlotParameters(PlotFactoryPtr factory);
    
    // Copy constructor.  See operator=().
    PlotMSPlotParameters(const PlotMSPlotParameters& copy);
    
    // Destructor.
    ~PlotMSPlotParameters();
    
    
    // Implements PlotMSWatchedParameters::equals().  Will return false if the
    // other parameters are not of type PlotMSPlotParameters.
    bool equals(const PlotMSWatchedParameters& other, int updateFlags) const;

    
    // Returns the subparameters group with the given name, or NULL for none.
    // <group>
    const Group* group(const String& name) const;
    Group* group(const String& name);
    // </group>
    
    // Returns the templated type of subparameters group, or NULL for none.
    // <group>
    template <class T>
    const T* typedGroup() const {
        const T* g = NULL;
        for(unsigned int i = 0; i < itsGroups_.size(); i++)
            if((g = dynamic_cast<const T*>(itsGroups_[i])) != NULL) return g;
        return NULL;
    }
    template <class T>
    T* typedGroup() {
        T* g = NULL;
        for(unsigned int i = 0; i < itsGroups_.size(); i++){
            if((g = dynamic_cast<T*>(itsGroups_[i])) != NULL) return g;
        }
        return NULL;
    }
    // </group>
    
    // Sets (or adds) the given group of subparameters in this object, cloning
    // it.
    void setGroup(const Group& group);
    
    // Sets (or adds) the default constructor of the templated type of
    // subparameters group.
    template <class T>
    void setGroup() { setGroup(T(itsFactory_)); }
    
    
    // Copy operator.  Copies subparameters groups.
    PlotMSPlotParameters& operator=(const PlotMSPlotParameters& copy);


protected:
    // Factory.
    PlotFactoryPtr itsFactory_;
    
    // Notifies any watchers that the parameters have been updated with the
    // given flags.  If an updater is given, it is NOT notified.
    // <group>
    void notifyWatchers(int updateFlags,
            PlotMSParametersWatcher* updater = NULL);
    void notifyWatchers(const String& updateName,
            PlotMSParametersWatcher* updater = NULL) {
        notifyWatchers(UPDATE_FLAG(updateName), updater); }
    // </group>
    
private:
    // Subparameter groups.
    vector<Group*> itsGroups_;
    
    
    // To be called when one of the groups is updated.
    void groupUpdated(Group* group, bool requiresRedraw);
};

// Helper macros to simplify calling a method on a subparameters group.
// <group>
#define PMS_PP_CALL(PARAMS, GROUP, METHOD, ...)                               \
    if( PARAMS .typedGroup< GROUP >() != NULL)                                \
        PARAMS .typedGroup< GROUP >()-> METHOD ( __VA_ARGS__ );

#define PMS_PP_RETCALL(PARAMS, GROUP, METHOD, DEFAULT, ...)                   \
    (PARAMS .typedGroup< GROUP >() == NULL ? DEFAULT :                        \
        PARAMS .typedGroup< GROUP >()-> METHOD ( __VA_ARGS__ ))
// </group>

}

#endif /* PLOTMSPLOTPARAMETERS_H_ */
