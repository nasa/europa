#include "TestSupport.hh"

ConstraintEngineId DefaultEngineAccessor::s_instance;

// Wouldn't this be useful for planners, e.g., as well?
// --wedgingt 2004 Mar 10
void initConstraintLibrary() {
  static bool s_runAlready(false);

  if (!s_runAlready) {
    // Register constraint Factories
    REGISTER_UNARY(SubsetOfConstraint, "SubsetOf", "Default");
    REGISTER_NARY(AddEqualConstraint, "AddEqual", "Default");
    REGISTER_NARY(AddMultEqualConstraint, "AddMultEqual", "Default");
    REGISTER_NARY(AllDiffConstraint, "AllDiff", "Default");
    REGISTER_NARY(CardinalityConstraint, "Cardinality", "Default");
    REGISTER_NARY(CondAllDiffConstraint, "CondAllDiff", "Default");
    REGISTER_NARY(CondAllSameConstraint, "CondAllSame", "Default");
    REGISTER_NARY(CondEqualSumConstraint, "CondEqualSum", "Default");
    REGISTER_NARY(CountNonZerosConstraint, "CountNonZeros", "Default");
    REGISTER_NARY(CountZerosConstraint, "CountZeros", "Default");
    REGISTER_NARY(EqualConstraint, "Equal", "Default");
    REGISTER_NARY(EqualMaximumConstraint, "EqualMaximum", "Default");
    REGISTER_NARY(EqualMinimumConstraint, "EqualMinimum", "Default");
    REGISTER_NARY(EqualProductConstraint, "EqualProduct", "Default");
    REGISTER_NARY(EqualSumConstraint, "EqualSum", "Default");
    REGISTER_NARY(GreaterThanSumConstraint, "GreaterThanSum", "Default");
    REGISTER_NARY(GreaterOrEqThanSumConstraint, "GreaterOrEqThanSum", "Default");
    REGISTER_NARY(LessOrEqThanSumConstraint, "LessOrEqThanSum", "Default");
    REGISTER_NARY(LessThanConstraint, "LessThan", "Default");
    REGISTER_NARY(LessThanEqualConstraint, "LessThanEqual", "Default");
    REGISTER_NARY(LessThanSumConstraint, "LessThanSum", "Default");
    REGISTER_NARY(MemberImplyConstraint, "MemberImply", "Default");
    REGISTER_NARY(MultEqualConstraint, "MultEqual", "Default");
    REGISTER_NARY(NotEqualConstraint, "NotEqual", "Default");
    REGISTER_NARY(OrConstraint, "Or", "Default");

    // Europa (NewPlan/ConstraintNetwork) names for the same constraints:
    REGISTER_NARY(AddEqualConstraint, "addeq", "Default");
    REGISTER_NARY(AddMultEqualConstraint, "addmuleq", "Default");
    REGISTER_NARY(AllDiffConstraint, "adiff", "Default"); // all different
    REGISTER_NARY(EqualConstraint, "asame", "Default"); // all same
    REGISTER_NARY(CardinalityConstraint, "card", "Default"); // cardinality not more than
    REGISTER_NARY(CountNonZerosConstraint, "cardeq", "Default"); // cardinality equals
    REGISTER_NARY(CondAllSameConstraint, "condeq", "Default");
    REGISTER_NARY(EqualConstraint, "eq", "Default");
    REGISTER_NARY(EqualConstraint, "fasame", "Default"); // flexible all same
    REGISTER_NARY(OrConstraint, "for", "Default"); // flexible or
    REGISTER_NARY(LessThanEqualConstraint, "leq", "Default");
    REGISTER_NARY(LessOrEqThanSumConstraint, "leqsum", "Default");
    REGISTER_NARY(LessThanConstraint, "lt", "Default");
    REGISTER_NARY(MemberImplyConstraint, "memberImply", "Default");
    REGISTER_NARY(NotEqualConstraint, "neq", "Default");
    REGISTER_NARY(OrConstraint, "or", "Default");
    REGISTER_NARY(EqualProductConstraint, "product", "Default");
    REGISTER_NARY(EqualSumConstraint, "sum", "Default");

    // Rotate scope right one (last var moves to front) to ...
    // ... change addleq constraint to GreaterOrEqThan constraint:
    REGISTER_ROTATED_NARY("addleq", "Default", "GreaterOrEqThanSum", 1);
    // ... change addlt constraint to GreaterThanSum constraint:
    REGISTER_ROTATED_NARY("addlt", "Default", "GreaterThanSum", 1);
    // ... change max constraint to EqualMaximum constraint:
    REGISTER_ROTATED_NARY("max", "Default", "EqualMaximum", 1);
    // ... change min constraint to EqualMinimum constraint:
    REGISTER_ROTATED_NARY("min", "Default", "EqualMinimum", 1);

    // But addeqcond is harder, requiring two "steps":
    REGISTER_SWAP_TWO_VARS_NARY("eqcondsum", "Default", "CondEqualSum", 0, 1);
    REGISTER_ROTATED_NARY("addeqcond", "Default", "eqcondsum", 2);

    s_runAlready = true;
  }
}

bool loggingEnabled() {
  static const char *envStr = getenv("PROTOTYPE_ENABLE_LOGGING");
  static const bool enabled = (envStr != NULL && atoi(envStr) != 0);
  return(enabled);
}
