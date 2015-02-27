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
	  new DefaultPropagator(std::string("Default"), ce->getId());
      ce->setAllowViolations(engine->getConfig()->getProperty("ConstraintEngine.allowViolations") == "true");
      
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
      REGISTER_CONSTRAINT_TYPE(ces,MultEqualCT, "multEq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,DivEqualCT, "divEq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces, AllDiffCT, "allDiff", "Default");
      REGISTER_CONSTRAINT_TYPE(ces, CalcDistanceCT, "calcDistance", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,CondAllDiffCT, "condAllDiff", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,CondAllSameCT, "condEq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,CountNonZeroesCT, "countNonZeroes", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,CountZeroesCT, "countZeroes", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,DistanceFromSquaresCT, "distanceSquares", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,EqualCT, "eq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,EqualMaximumCT, "eqMaximum", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,EqualMinimumCT, "eqMinimum", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,EqualProductCT, "eqProduct", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,EqualSumCT, "eqSum", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,LessThanCT, "lt", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,LessThanEqualCT, "leq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,LockCT, "lockCT", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,MemberImplyCT, "memberImply", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,MultEqualCT, "mulEq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,DivEqualCT, "divEq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,NegateCT, "neg", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,NotEqualCT, "neq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,OrCT, "or", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,SineCT, "sin", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,SquareOfDifferenceCT, "diffSquare", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,SubsetOfCT, "subsetOf", "Default");

      REGISTER_CONSTRAINT_TYPE(ces,TestAndCT, "testAnd", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestEQCT, "testEq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestLessThanCT, "testLessThan", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestLEQCT, "testLeq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestNEQCT, "testNeq", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestOrCT, "testOr", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestSingletonCT, "testSingleton", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,TestSpecifiedCT, "testSpecified", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,UnaryCT, "UNARY", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,WithinBoundsCT, "withinBounds", "Default");

      REGISTER_CONSTRAINT_TYPE(ces,MaxCT, "maxf", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,MinCT, "minf", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,AbsCT, "abs", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,PowCT, "pow", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,SqrtCT, "sqrt", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,ModCT, "mod", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,FloorCT, "floor", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,CeilCT, "ceil", "Default");
      REGISTER_CONSTRAINT_TYPE(ces,RandCT, "rand", "Default");

      REGISTER_CONSTRAINT_TYPE(ces,EqUnionCT, "eqUnion", "Default");

      // ... change allmax and max constraint to EqualMaximum constraint:
      REGISTER_ROTATED_CONSTRAINT(ces,"allMax", "Default", "eqMaximum", 1);
      REGISTER_ROTATED_CONSTRAINT(ces,"max", "Default", "eqMaximum", 1);
      // ... change allmin and min constraint to EqualMinimum constraint:
      REGISTER_ROTATED_CONSTRAINT(ces,"allMin", "Default", "eqMinimum", 1);
      REGISTER_ROTATED_CONSTRAINT(ces,"min", "Default", "eqMinimum", 1);
  }

  void ModuleConstraintLibrary::uninitialize(EngineId engine)
  {
    CESchema* ces = boost::polymorphic_cast<CESchema*>(engine->getComponent("CESchema"));
      // TODO: should be more selective and only remove the constraints we added above
      ces->purgeConstraintTypes();
  }
}
