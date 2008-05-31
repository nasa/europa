#include "ModuleConstraintEngine.hh"
#include "BoolTypeFactory.hh"
#include "IntervalIntTypeFactory.hh"
#include "IntervalTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"
#include "EnumeratedTypeFactory.hh"
#include "Constraints.hh"
#include "ConstraintLibrary.hh"
#include "DefaultPropagator.hh"


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
      CESchema* tfm = new CESchema();
      tfm->registerFactory((new BoolTypeFactory())->getId());
      tfm->registerFactory((new IntervalIntTypeFactory())->getId());
      tfm->registerFactory((new IntervalTypeFactory())->getId());
      tfm->registerFactory((new EnumeratedTypeFactory("REAL_ENUMERATION", "ELEMENT", EnumeratedDomain(true, "REAL_ENUMERATION")))->getId());
      tfm->registerFactory((new StringTypeFactory())->getId());
      tfm->registerFactory((new SymbolTypeFactory())->getId());
      engine->addComponent("CESchema",tfm);      

      ConstraintEngine* ce = new ConstraintEngine(tfm->getId());
	  new DefaultPropagator(LabelStr("Default"), ce->getId());	  
      engine->addComponent("ConstraintEngine",ce);      
  }
  
  void ModuleConstraintEngine::uninitialize(EngineId engine)
  {	  
      ConstraintEngine* ce = (ConstraintEngine*)engine->removeComponent("ConstraintEngine");      
      delete ce;
      
      CESchema* tfm = (CESchema*)engine->removeComponent("CESchema");      
      delete tfm;
  }
    
  /**************************************************************************************/

  static bool & constraintLibraryInitialized() {
    static bool sl_constraintLibraryInit(false);
    return sl_constraintLibraryInit;
  }

  void initConstraintLibrary() {
    if( !constraintLibraryInitialized()) {
      constraintLibraryInitialized() = true;
     
      debugMsg("ModuleConstraintLibrary:initConstraintLibrary", "Initializing the constraint library");
      // Register constraint Factories
      REGISTER_CONSTRAINT(UnaryConstraint, "UNARY", "Default");
      REGISTER_CONSTRAINT(AddEqualConstraint, "AddEqual", "Default");
      REGISTER_CONSTRAINT(MultEqualConstraint, "MultEqual", "Default");
      REGISTER_CONSTRAINT(AddMultEqualConstraint, "AddMultEqual", "Default");
      REGISTER_CONSTRAINT(AddMultEqualConstraint, "addMulEq", "Default");
      REGISTER_CONSTRAINT(AllDiffConstraint, "AllDiff", "Default");
      REGISTER_CONSTRAINT(CardinalityConstraint, "Cardinality", "Default");
      REGISTER_CONSTRAINT(CondAllDiffConstraint, "CondAllDiff", "Default");
      REGISTER_CONSTRAINT(CondAllSameConstraint, "CondAllSame", "Default");
      REGISTER_CONSTRAINT(CondEqualSumConstraint, "CondEqualSum", "Default");
      REGISTER_CONSTRAINT(CountNonZerosConstraint, "CountNonZeros", "Default");
      REGISTER_CONSTRAINT(CountZerosConstraint, "CountZeros", "Default");
      REGISTER_CONSTRAINT(EqualConstraint, "Equal", "Default");
      REGISTER_CONSTRAINT(EqualMaximumConstraint, "EqualMaximum", "Default");
      REGISTER_CONSTRAINT(EqualMinimumConstraint, "EqualMinimum", "Default");
      REGISTER_CONSTRAINT(EqualProductConstraint, "EqualProduct", "Default");
      REGISTER_CONSTRAINT(EqualSumConstraint, "EqualSum", "Default");
      REGISTER_CONSTRAINT(GreaterThanSumConstraint, "GreaterThanSum", "Default");
      REGISTER_CONSTRAINT(GreaterOrEqThanSumConstraint, "GreaterOrEqThanSum", "Default");
      REGISTER_CONSTRAINT(LessOrEqThanSumConstraint, "LessOrEqThanSum", "Default");
      REGISTER_CONSTRAINT(LessThanConstraint, "LessThan", "Default");
      REGISTER_CONSTRAINT(LessThanEqualConstraint, "LessThanEqual", "Default");
      REGISTER_CONSTRAINT(WithinBounds, "WithinBounds", "Default");
      REGISTER_CONSTRAINT(LessThanSumConstraint, "LessThanSum", "Default");
      REGISTER_CONSTRAINT(LockConstraint, "Lock", "Default");
      REGISTER_CONSTRAINT(MemberImplyConstraint, "MemberImply", "Default");
      REGISTER_CONSTRAINT(NotEqualConstraint, "NotEqual", "Default");
      REGISTER_CONSTRAINT(OrConstraint, "Or", "Default");
      REGISTER_CONSTRAINT(SubsetOfConstraint, "SubsetOf", "Default");
      REGISTER_CONSTRAINT(SubsetOfConstraint, "subsetOf", "Default");
      REGISTER_CONSTRAINT(SubsetOfConstraint, "Singleton", "Default");
      REGISTER_CONSTRAINT(TestEQ, "TestEqual", "Default");
      REGISTER_CONSTRAINT(TestLessThan, "TestLessThan", "Default");
      REGISTER_CONSTRAINT(TestEQ, "testEQ", "Default");
      REGISTER_CONSTRAINT(TestLEQ, "testLEQ", "Default");

      // Europa (NewPlan/ConstraintNetwork) names for the same constraints:
      REGISTER_CONSTRAINT(AddEqualConstraint, "addeq", "Default");
      REGISTER_CONSTRAINT(AddEqualConstraint, "addEq", "Default");
      REGISTER_CONSTRAINT(AddMultEqualConstraint, "addmuleq", "Default");
      REGISTER_CONSTRAINT(AllDiffConstraint, "adiff", "Default"); // all different
      REGISTER_CONSTRAINT(AllDiffConstraint, "fadiff", "Default"); // flexible all different
      REGISTER_CONSTRAINT(AllDiffConstraint, "fneq", "Default"); // flexible not equal
      REGISTER_CONSTRAINT(CardinalityConstraint, "card", "Default"); // cardinality not more than
      REGISTER_CONSTRAINT(CondAllSameConstraint, "condEq", "Default");
      REGISTER_CONSTRAINT(CondAllSameConstraint, "condeq", "Default");
      REGISTER_CONSTRAINT(CondAllSameConstraint, "condasame", "Default");
      REGISTER_CONSTRAINT(TestLessThan, "condlt", "Default");
      REGISTER_CONSTRAINT(TestLEQ, "condleq", "Default");
      REGISTER_CONSTRAINT(CountNonZerosConstraint, "cardeq", "Default"); // cardinality equals
      REGISTER_CONSTRAINT(EqualConstraint, "asame", "Default"); // all same
      REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
      REGISTER_CONSTRAINT(EqualConstraint, "fasame", "Default"); // flexible all same
      REGISTER_CONSTRAINT(EqualMaximumConstraint, "fallmax", "Default"); // flexible all max
      REGISTER_CONSTRAINT(EqualMinimumConstraint, "fallmin", "Default"); // flexible all min
      REGISTER_CONSTRAINT(EqualProductConstraint, "product", "Default");
      REGISTER_CONSTRAINT(EqualSumConstraint, "sum", "Default");
      REGISTER_CONSTRAINT(LessOrEqThanSumConstraint, "leqsum", "Default");
      REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
      REGISTER_CONSTRAINT(LessThanConstraint, "lessThan", "Default");
      REGISTER_CONSTRAINT(LessThanEqualConstraint, "leq", "Default");
      REGISTER_CONSTRAINT(WithinBounds, "withinBounds", "Default");
      REGISTER_CONSTRAINT(MemberImplyConstraint, "memberImply", "Default");
      REGISTER_CONSTRAINT(MultEqualConstraint, "mulEq", "Default");
      REGISTER_CONSTRAINT(MultEqualConstraint, "multEq", "Default");
      REGISTER_CONSTRAINT(NegateConstraint, "neg", "Default");
      REGISTER_CONSTRAINT(NotEqualConstraint, "neq", "Default");
      REGISTER_CONSTRAINT(OrConstraint, "for", "Default"); // flexible or
      REGISTER_CONSTRAINT(OrConstraint, "or", "Default");

      REGISTER_CONSTRAINT(AbsoluteValue, "absVal", "Default");

      // Rotate scope right one (last var moves to front) to ...
      // ... change addleq constraint to GreaterOrEqThan constraint:
      REGISTER_ROTATED_CONSTRAINT("addleq", "Default", "GreaterOrEqThanSum", 1);
      // ... change addlt constraint to GreaterThanSum constraint:
      REGISTER_ROTATED_CONSTRAINT("addlt", "Default", "GreaterThanSum", 1);
      // ... change allmax and max constraint to EqualMaximum constraint:
      REGISTER_ROTATED_CONSTRAINT("allmax", "Default", "EqualMaximum", 1);
      REGISTER_ROTATED_CONSTRAINT("max", "Default", "EqualMaximum", 1);
      // ... change allmin and min constraint to EqualMinimum constraint:
      REGISTER_ROTATED_CONSTRAINT("allmin", "Default", "EqualMinimum", 1);
      REGISTER_ROTATED_CONSTRAINT("min", "Default", "EqualMinimum", 1);

      // But addeqcond is harder, requiring two "steps":
      REGISTER_SWAP_TWO_VARS_CONSTRAINT("eqcondsum", "Default", "CondEqualSum", 0, 1);
      REGISTER_ROTATED_CONSTRAINT("addeqcond", "Default", "eqcondsum", 2);

      //constraints formerly from LORAX
      REGISTER_CONSTRAINT(SquareOfDifferenceConstraint, "diffSquare", "Default");
      REGISTER_CONSTRAINT(DistanceFromSquaresConstraint, "distanceSquares", "Default");      
   
      REGISTER_CONSTRAINT(CalcDistanceConstraint, "calcDistance", "Default");

      REGISTER_CONSTRAINT(SineFunction, "sin", "Default");
    } else {
       debugMsg("ModuleConstraintLibrary:initConstriantLibrary", "Constraint library already initalized - no action taken");
    }
  }
  
  void uninitConstraintLibrary() {
    if (constraintLibraryInitialized()) {
      ConstraintLibrary::purgeAll();
      constraintLibraryInitialized() = false;
    }
  }
  
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
  
  // TODO: move the meat from old functions into these methods, remove old functions
  void ModuleConstraintLibrary::initialize(EngineId engine)
  {
      initConstraintLibrary();
  }
  
  void ModuleConstraintLibrary::uninitialize(EngineId engine)
  {	  
      uninitConstraintLibrary();
  }
  
}
