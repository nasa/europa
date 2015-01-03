#include "ModuleConstraintEngine.hh"
#include "DataTypes.hh"
#include "Constraints.hh"
#include "ConstraintType.hh"
#include "Propagators.hh"
#include "CFunctions.hh"
#include "CESchema.hh"

#include <boost/cast.hpp>

namespace EUROPA {

  ModuleConstraintEngine::ModuleConstraintEngine()
      : Module("ConstraintEngine")
  {

  }

  ModuleConstraintEngine::~ModuleConstraintEngine()
  {
  }

  void ModuleConstraintEngine::initialize()
  {
  }

  void ModuleConstraintEngine::uninitialize()
  {
  }

  void ModuleConstraintEngine::initialize(EngineId engine)
  {
      CESchema* ces = new CESchema();
      ces->registerDataType((new VoidDT())->getId());
      ces->registerDataType((new BoolDT())->getId());
      ces->registerDataType((new IntDT())->getId());
      ces->registerDataType((new FloatDT())->getId());
      ces->registerDataType((new StringDT())->getId());
      ces->registerDataType((new SymbolDT())->getId());
      engine->addComponent("CESchema",ces);

      ces->registerCFunction((new IsSingleton())->getId());
      ces->registerCFunction((new IsSpecified())->getId());
      ces->registerCFunction((new MaxFunction())->getId());
      ces->registerCFunction((new MinFunction())->getId());
      ces->registerCFunction((new AbsFunction())->getId());
      ces->registerCFunction((new PowFunction())->getId());
      ces->registerCFunction((new SqrtFunction())->getId());
      ces->registerCFunction((new ModFunction())->getId());
      ces->registerCFunction((new RandFunction())->getId());
      ces->registerCFunction((new FloorFunction())->getId());
      ces->registerCFunction((new CeilFunction())->getId());

      ConstraintEngine* ce = new ConstraintEngine(ces->getId());
	  new DefaultPropagator(LabelStr("Default"), ce->getId());
      engine->addComponent("ConstraintEngine",ce);
  }

void ModuleConstraintEngine::uninitialize(EngineId engine) {
  ConstraintEngine* ce =
      boost::polymorphic_cast<ConstraintEngine*>(engine->removeComponent("ConstraintEngine"));
  delete ce;

  CESchema* ces = boost::polymorphic_cast<CESchema*>(engine->removeComponent("CESchema"));
  delete ces;
}

  /**************************************************************************************/

  ModuleConstraintLibrary::ModuleConstraintLibrary()
      : Module("ConstraintLibrary")
  {
  }

  ModuleConstraintLibrary::~ModuleConstraintLibrary()
  {
  }

  void ModuleConstraintLibrary::initialize()
  {
  }

  void ModuleConstraintLibrary::uninitialize()
  {
  }

  void ModuleConstraintLibrary::initialize(EngineId engine)
  {
      debugMsg("ModuleConstraintLibrary:initialize", "Initializing the constraint library");

      CESchema* ces = boost::polymorphic_cast<CESchema*>(engine->getComponent("CESchema"));

      // Register constraint Factories
      REGISTER_CONSTRAINT_TYPE(ces,AbsoluteValueCT, "absVal", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,AddEqualCT, "addEq", "Default");

      REGISTER_CONSTRAINT(ces,AddMultEqualConstraint, "addMulEq", "Default");
      REGISTER_CONSTRAINT(ces,AllDiffConstraint, "allDiff", "Default"); // all different
      REGISTER_CONSTRAINT(ces,CalcDistanceConstraint, "calcDistance", "Default");
      REGISTER_CONSTRAINT(ces,CardinalityConstraint, "cardinality", "Default"); // cardinality not more than
      REGISTER_CONSTRAINT(ces,CondAllDiffConstraint, "condAllDiff", "Default");
      REGISTER_CONSTRAINT(ces,CondAllSameConstraint, "condEq", "Default");
      REGISTER_CONSTRAINT(ces,CondEqualSumConstraint, "condEqSum", "Default");
      REGISTER_CONSTRAINT(ces,CountNonZerosConstraint, "countNonZeroes", "Default");
      REGISTER_CONSTRAINT(ces,CountZerosConstraint, "countZeroes", "Default");
      REGISTER_CONSTRAINT(ces,DistanceFromSquaresConstraint, "distanceSquares", "Default");

      REGISTER_CONSTRAINT_TYPE(ces,EqualCT, "eq", "Default");

      REGISTER_CONSTRAINT(ces,EqualMaximumConstraint, "eqMaximum", "Default");
      REGISTER_CONSTRAINT(ces,EqualMinimumConstraint, "eqMinimum", "Default");
      REGISTER_CONSTRAINT(ces,EqualProductConstraint, "eqProduct", "Default");

      REGISTER_CONSTRAINT(ces,EqualSumConstraint, "eqSum", "Default");

      REGISTER_CONSTRAINT(ces,GreaterThanSumConstraint, "greaterThanSum", "Default");
      REGISTER_CONSTRAINT(ces,GreaterOrEqThanSumConstraint, "greaterOrEqThanSum", "Default");
      REGISTER_CONSTRAINT(ces,LessOrEqThanSumConstraint, "lessOrEqThanSum", "Default");

      REGISTER_CONSTRAINT_TYPE(ces,LessThanCT, "lt", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,LessThanEqualCT, "leq", "Default");

      REGISTER_CONSTRAINT(ces,LessThanSumConstraint, "lessThanSum", "Default");
      REGISTER_CONSTRAINT(ces,LockConstraint, "lock", "Default");
      REGISTER_CONSTRAINT(ces,MemberImplyConstraint, "memberImply", "Default");
      REGISTER_CONSTRAINT(ces,MultEqualConstraint, "mulEq", "Default");
      // Minh: Added (06/06/2012) new entries for DivEqualConstraint
      REGISTER_CONSTRAINT(ces,DivEqualConstraint, "divEq", "Default");
      // Minh: END
      REGISTER_CONSTRAINT(ces,NegateConstraint, "neg", "Default");
      REGISTER_CONSTRAINT(ces,NotEqualConstraint, "neq", "Default");
      REGISTER_CONSTRAINT(ces,OrConstraint, "or", "Default");
      REGISTER_CONSTRAINT(ces,SineFunction, "sin", "Default");
      REGISTER_CONSTRAINT(ces,SquareOfDifferenceConstraint, "diffSquare", "Default");
      REGISTER_CONSTRAINT(ces,SubsetOfConstraint, "subsetOf", "Default");

      REGISTER_CONSTRAINT_TYPE(ces,TestAndCT, "testAnd", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestEQCT, "testEq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestLessThanCT, "testLessThan", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestLEQCT, "testLeq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestNEQCT, "testNeq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestOrCT, "testOr", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestSingletonCT, "testSingleton", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestSpecifiedCT, "testSpecified", "Default");

      REGISTER_CONSTRAINT(ces,UnaryConstraint, "UNARY", "Default");

      REGISTER_CONSTRAINT(ces,WithinBounds, "withinBounds", "Default");

      REGISTER_CONSTRAINT(ces,MaxConstraint, "maxf", "Default");
      REGISTER_CONSTRAINT(ces,MinConstraint, "minf", "Default");
      REGISTER_CONSTRAINT(ces,AbsConstraint, "abs", "Default");
      REGISTER_CONSTRAINT(ces,PowConstraint, "pow", "Default");
      REGISTER_CONSTRAINT(ces,SqrtConstraint, "sqrt", "Default");
      REGISTER_CONSTRAINT(ces,ModConstraint, "mod", "Default");
      REGISTER_CONSTRAINT(ces,FloorConstraint, "floor", "Default");
      REGISTER_CONSTRAINT(ces,CeilConstraint, "ceil", "Default");
      REGISTER_CONSTRAINT(ces,RandConstraint, "rand", "Default");




      // Rotate scope right one (last var moves to front) to ...
      // ... change addleq constraint to GreaterOrEqThan constraint:
      REGISTER_ROTATED_CONSTRAINT(ces,"addLeq", "Default", "greaterOrEqThanSum", 1);
      // ... change addlt constraint to GreaterThanSum constraint:
      REGISTER_ROTATED_CONSTRAINT(ces,"addLt", "Default", "greaterThanSum", 1);
      // ... change allmax and max constraint to EqualMaximum constraint:
      REGISTER_ROTATED_CONSTRAINT(ces,"allMax", "Default", "eqMaximum", 1);
      REGISTER_ROTATED_CONSTRAINT(ces,"max", "Default", "eqMaximum", 1);
      // ... change allmin and min constraint to EqualMinimum constraint:
      REGISTER_ROTATED_CONSTRAINT(ces,"allMin", "Default", "eqMinimum", 1);
      REGISTER_ROTATED_CONSTRAINT(ces,"min", "Default", "eqMinimum", 1);

      // But addeqcond is harder, requiring two "steps":
      REGISTER_SWAP_TWO_VARS_CONSTRAINT(ces,"eqCondSum", "Default", "condEqSum", 0, 1);
      REGISTER_ROTATED_CONSTRAINT(ces,"addEqCond", "Default", "eqCondSum", 2);
  }

  void ModuleConstraintLibrary::uninitialize(EngineId engine)
  {
    CESchema* ces = boost::polymorphic_cast<CESchema*>(engine->getComponent("CESchema"));
      // TODO: should be more selective and only remove the constraints we added above
      ces->purgeConstraintTypes();
  }
}
