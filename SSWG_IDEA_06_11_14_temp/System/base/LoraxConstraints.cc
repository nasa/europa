#include "LoraxConstraints.hh"

#include "Constraints.hh"
#include "ConstraintEngine.hh"
#include "ConstraintLibrary.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "Utils.hh"
#include <cmath>

namespace EUROPA {

  DifferentPointConstraint::DifferentPointConstraint(
		          const LabelStr& name,
		          const LabelStr& propagatorName,
			  const ConstraintEngineId& constraintEngine,
			  const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
    for (int i = 0; i < ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }

  void DifferentPointConstraint::handleExecute() {
    IntervalDomain& domx1 = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X1]));
    IntervalDomain& domy1 = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y1]));
    IntervalDomain& domx2 = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X2]));
    IntervalDomain& domy2 = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y2]));

    check_error(AbstractDomain::canBeCompared(domx1, domx2));
    check_error(AbstractDomain::canBeCompared(domy1, domy2));

    if (domx1.isOpen() || domy1.isOpen() || domx2.isOpen() || domy2.isOpen())
      return;

    check_error(!domx1.isEmpty() && !domy1.isEmpty() && !domx2.isEmpty() && !domy2.isEmpty());

    std::cout << "TK DiffPoint "<<domx1<<domy1<<" "<<domx2<<domy2<< std::endl;
  }




  AtLeastOneConstraint::AtLeastOneConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void AtLeastOneConstraint::handleExecute() {
    BoolDomain& domx = static_cast<BoolDomain&>(getCurrentDomain(m_variables[V1]));
    BoolDomain& domy = static_cast<BoolDomain&>(getCurrentDomain(m_variables[V2]));

    check_error(AbstractDomain::canBeCompared(domx, domy));

    // Discontinue if either domain is open.
    if (domx.isOpen() || domy.isOpen()) return;

    check_error(!domx.isEmpty() && !domy.isEmpty());

    if ( domx.isSingleton() && domx.isFalse() ) {
      domy.intersect( BoolDomain( true ) );
    }
    if ( !domy.isEmpty() && domy.isSingleton() && domy.isFalse() ) {
      domx.intersect( BoolDomain( true ) );
    }
    //cout << "After "<<domx<<":"<<domx.isEmpty()<<" "<<domy<<":"<<domy.isEmpty()<<std::endl;
  }



  DifferentConstraint::DifferentConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void DifferentConstraint::handleExecute() {
    AbstractDomain& domx = getCurrentDomain(m_variables[V1]);
    AbstractDomain& domy = getCurrentDomain(m_variables[V2]);
    AbstractDomain& doma = getCurrentDomain(m_variables[ANS]);

    // doma should be Bool
    check_error( doma.isBool());

    // domains should be comparable
    check_error(AbstractDomain::canBeCompared(domx, domy));

    // domx and domy should be songletons, or we cannot do anything
    if ( !domx.isSingleton() || !domy.isSingleton() )
      return;
    
    // otherwise get singleton values and compare
    bool neq = !domx.isMember(domy.getSingletonValue());
    
    //cout << "TK Different "<<neq<<" "<<domx<<" "<<domy<< " "<<doma<<std::endl;
    doma.intersect( BoolDomain(neq) );
    //cout << "After "<<doma<<" "<<doma.isEmpty()<<std::endl;
  }


  SquareOfDifferenceConstraint::SquareOfDifferenceConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  /**
     Propagate only forward.
     Moreover, the way it is used, either the points are completely
     specified (singletons), or they are INF
   */
  void SquareOfDifferenceConstraint::handleExecute() {
    AbstractDomain& domx = getCurrentDomain(m_variables[V1]);
    AbstractDomain& domy = getCurrentDomain(m_variables[V2]);
    AbstractDomain& doma = getCurrentDomain(m_variables[RES]);

    // domains should be closed
    if ( domx.isOpen() || domy.isOpen() || 
	 !domx.isSingleton() || !domy.isSingleton() ) 
      return;

    // get the boundaries
    double x, y;
    x = domx.getSingletonValue();
    y = domy.getSingletonValue();
    double square = (x-y)*(x-y);

    doma.intersect( IntervalDomain(square) );

    // cout << "SquareOfDifference "<<x<<" "<<y<<" "<<square<<endl;
  }

  DistanceFromSquaresConstraint::DistanceFromSquaresConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  /**
     Propagate only forward.
     Moreover, the way it is used, either the squares are completely
     specified (singletons), or they are INF
   */
  void DistanceFromSquaresConstraint::handleExecute() {
    AbstractDomain& domx = getCurrentDomain(m_variables[V1]);
    AbstractDomain& domy = getCurrentDomain(m_variables[V2]);
    AbstractDomain& doma = getCurrentDomain(m_variables[RES]);

    // domains should be closed
    if ( domx.isOpen() || domy.isOpen() || 
	 !domx.isSingleton() || !domy.isSingleton() ) 
      return;

    // get the boundaries
    double x, y;
    x = domx.getSingletonValue();
    y = domy.getSingletonValue();
    double distance = sqrt(x+y);

    doma.intersect( IntervalDomain(distance) );

    // cout << "DistanceFromSquares "<<x<<" "<<y<<" "<<distance<<endl;
  }


  //========================================================================
  // Take these into a separate file?
  DriveBatteryConstraint::DriveBatteryConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void DriveBatteryConstraint::handleExecute() {
    IntervalDomain& dom_B = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[CHARGE]));
    IntervalDomain& dom_D = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[DISTANCE]));

    // Discontinue if distance domain is open or not a singleton
    if (dom_D.isOpen() || !dom_D.isSingleton() ) return;
    check_error(!dom_D.isEmpty() && !dom_B.isEmpty());

    // Here we need to plug in a smart function
    double dist = dom_D.getSingletonValue();
    double charge = -dist*4;

    // now set the value
    dom_B.intersect( IntervalDomain( charge ) );

    //cout << "Charge "<<dom_B<<" "<<dom_D<<std::endl;
  }

  DriveDurationConstraint::DriveDurationConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void DriveDurationConstraint::handleExecute() {
    IntervalDomain& dom_T = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[DURATION]));
    IntervalDomain& dom_D = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[DISTANCE]));

    // Discontinue if distance domain is open or not a singleton
    if (dom_D.isOpen() || !dom_D.isSingleton() ) return;
    check_error(!dom_D.isEmpty() && !dom_T.isEmpty());

    // Here we need to plug in a smart function
    double dist = dom_D.getSingletonValue();
	int lower = (int)dist;
	int upper = (int)(dist*2);

    // now set the value
    dom_T.intersect( IntervalIntDomain( lower, upper ) );

    //cout << "Duration "<<dom_T<<" "<<dom_D<<std::endl;
  }



  SampleBatteryConstraint::SampleBatteryConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void SampleBatteryConstraint::handleExecute() {
    IntervalDomain& dom_B = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[CHARGE]));

    check_error(!dom_B.isEmpty());

    // Here we need to plug in a smart function
    double charge = -10;

    // now set the value
    dom_B.intersect( IntervalDomain( charge ) );

    //cout << "Charge "<<dom_B<<std::endl;
  }

  SampleDurationConstraint::SampleDurationConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void SampleDurationConstraint::handleExecute() {
    IntervalDomain& dom_T = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[DURATION]));

    check_error(!dom_T.isEmpty());

    // Here we need to plug in a smart function
    dom_T.intersect( IntervalIntDomain( 3, 10 ) );

    //cout << "Duration "<<dom_T<<std::endl;
  }

  WindPowerConstraint::WindPowerConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void WindPowerConstraint::handleExecute() {
    IntervalDomain& dom_B = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[CHARGE]));
    IntervalDomain& dom_V = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[VELOCITY]));

    // Discontinue if velocity domain is open or not a singleton
    if (dom_V.isOpen() || !dom_V.isSingleton() ) return;
    check_error(!dom_V.isEmpty() && !dom_B.isEmpty());

    // Here we need to plug in a smart function
    double velocity = dom_V.getSingletonValue();

    // now set the value
    dom_B.intersect( IntervalDomain( velocity, velocity ) );

    //cout << "Wind power "<<dom_B<<" "<<dom_V<<std::endl;
  }


  //========================================================================
  // Very cruel way to get rid of Standby's at no point
  FinitePointConstraint::FinitePointConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void FinitePointConstraint::handleExecute() {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));

    check_error(!domx.isEmpty() && !domy.isEmpty());

	// std::cout<<" Finite point "<<domx<<domy<<std::endl ;

	if ( !domx.isSingleton() ) {
	  domx.empty();
	}
	if ( !domy.isSingleton() ) {
	  domy.empty();
	}
  }



} // namespace
