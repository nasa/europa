/**
 * @file DomainTest.cc
 * @brief Performance tests of domain classes.
 * @note Not run as part of the 'jam test' target in ../Jamrules,
 * as it is not a module test.
 */

#include "LabelStr.hh"
#include "Domain.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "ConstraintEngine.hh"
#include "Constraints.hh"
#include "DefaultPropagator.hh"
#include "Variable.hh"
#include "EqualityConstraintPropagator.hh"

#include <iostream>
#include <list>
#include <vector>

using namespace EUROPA;
using namespace std;

static void testEquate(const LabelSet& a, const LabelSet& b) {
  LabelSet l_a(a);
  LabelSet l_b(b);
  l_a.equate(l_b);
}

static void testIntersection() {
  IntervalIntDomain dom1(100, 1000);
  IntervalIntDomain dom2(250, 2000);
  dom1.intersect(dom2);
}

static void outerLoopForTestIntersection() {
  for (int i = 0; i < 1000000; i++)
    testIntersection();
}

static void outerLoopForTestEquate() {
  std::list<EUROPA::LabelStr> values;
  values.push_back(EUROPA::LabelStr("A"));
  values.push_back(EUROPA::LabelStr("B"));
  values.push_back(EUROPA::LabelStr("C"));
  values.push_back(EUROPA::LabelStr("D"));
  values.push_back(EUROPA::LabelStr("E"));
  values.push_back(EUROPA::LabelStr("F"));
  values.push_back(EUROPA::LabelStr("G"));
  values.push_back(EUROPA::LabelStr("H"));
  LabelSet ls_a(values);

  values.clear();
  values.push_back(EUROPA::LabelStr("1"));
  values.push_back(EUROPA::LabelStr("2"));
  values.push_back(EUROPA::LabelStr("3"));
  values.push_back(EUROPA::LabelStr("E"));
  values.push_back(EUROPA::LabelStr("4"));
  values.push_back(EUROPA::LabelStr("5"));
  values.push_back(EUROPA::LabelStr("6"));
  values.push_back(EUROPA::LabelStr("7"));
  values.push_back(EUROPA::LabelStr("8"));
  LabelSet ls_b(values);

  for (int i = 0; i < 1000000; i++)
    testEquate(ls_a, ls_b);
}

static void testLabelSetEqualityPerformance(const ConstraintEngineId& ce) {
  std::list<EUROPA::LabelStr> values;
  values.push_back(EUROPA::LabelStr("V0"));
  values.push_back(EUROPA::LabelStr("V1"));
  values.push_back(EUROPA::LabelStr("V2"));
  values.push_back(EUROPA::LabelStr("V3"));
  values.push_back(EUROPA::LabelStr("V4"));
  values.push_back(EUROPA::LabelStr("V5"));
  values.push_back(EUROPA::LabelStr("V6"));
  values.push_back(EUROPA::LabelStr("V7"));
  values.push_back(EUROPA::LabelStr("V8"));
  values.push_back(EUROPA::LabelStr("V9"));
  LabelSet labelSet(values);


  Variable<LabelSet> v0(ce, labelSet);
  Variable<LabelSet> v1(ce, labelSet);
  Variable<LabelSet> v2(ce, labelSet);
  Variable<LabelSet> v3(ce, labelSet);
  Variable<LabelSet> v4(ce, labelSet);
  Variable<LabelSet> v5(ce, labelSet);
  Variable<LabelSet> v6(ce, labelSet);
  Variable<LabelSet> v7(ce, labelSet);
  Variable<LabelSet> v8(ce, labelSet);
  Variable<LabelSet> v9(ce, labelSet);

  std::vector<ConstrainedVariableId> variables;

  variables.push_back(v0.getId());
  variables.push_back(v1.getId());
  EqualConstraint c0(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v1.getId());
  variables.push_back(v2.getId());
  EqualConstraint c1(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v2.getId());
  variables.push_back(v3.getId());
  EqualConstraint c2(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v3.getId());
  variables.push_back(v4.getId());
  EqualConstraint c3(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v4.getId());
  variables.push_back(v5.getId());
  EqualConstraint c4(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v5.getId());
  variables.push_back(v6.getId());
  EqualConstraint c5(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v6.getId());
  variables.push_back(v7.getId());
  EqualConstraint c6(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v7.getId());
  variables.push_back(v8.getId());
  EqualConstraint c7(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v8.getId());
  variables.push_back(v9.getId());
  EqualConstraint c8(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v0.getId());
  variables.push_back(v1.getId());
  variables.push_back(v2.getId());
  variables.push_back(v3.getId());
  variables.push_back(v4.getId());
  variables.push_back(v5.getId());
  variables.push_back(v6.getId());
  variables.push_back(v7.getId());
  variables.push_back(v8.getId());
  variables.push_back(v9.getId());

  Variable<LabelSet>*  p_v0 = (Variable<LabelSet>*) v0.getId();


  for(int i = 10; i > 2; i--){
    values.pop_back();
    LabelSet newDomain(values);
    Variable<LabelSet>*  p_v = (Variable<LabelSet>*) variables[i-1];
    p_v->specify(newDomain);
    ce->propagate();
    assertTrue(ce->constraintConsistent());
    assertTrue(p_v0->getDerivedDomain().getSize() == i-1);
  }
}

static void outerLoopLabelSetEqualConstraint(bool useEquivalenceClasses) {
  ConstraintEngine ce;

  if (useEquivalenceClasses)
    new EqualityConstraintPropagator(LabelStr("Equal"), ce.getId());
  else
    new DefaultPropagator(LabelStr("Equal"), ce.getId());

  for (int i = 0; i < 1000; i++)
    testLabelSetEqualityPerformance(ce.getId());
}

static void testIntervalEqualityPerformance(const ConstraintEngineId& ce) {
  IntervalIntDomain intSort(-1000, 1000);
  Variable<IntervalIntDomain> v0(ce, intSort);
  Variable<IntervalIntDomain> v1(ce, intSort);
  Variable<IntervalIntDomain> v2(ce, intSort);
  Variable<IntervalIntDomain> v3(ce, intSort);
  Variable<IntervalIntDomain> v4(ce, intSort);
  Variable<IntervalIntDomain> v5(ce, intSort);
  Variable<IntervalIntDomain> v6(ce, intSort);
  Variable<IntervalIntDomain> v7(ce, intSort);
  Variable<IntervalIntDomain> v8(ce, intSort);
  Variable<IntervalIntDomain> v9(ce, intSort);

  std::vector<ConstrainedVariableId> variables;

  variables.push_back(v0.getId());
  variables.push_back(v1.getId());
  EqualConstraint c0(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v1.getId());
  variables.push_back(v2.getId());
  EqualConstraint c1(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v2.getId());
  variables.push_back(v3.getId());
  EqualConstraint c2(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v3.getId());
  variables.push_back(v4.getId());
  EqualConstraint c3(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v4.getId());
  variables.push_back(v5.getId());
  EqualConstraint c4(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v5.getId());
  variables.push_back(v6.getId());
  EqualConstraint c5(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v6.getId());
  variables.push_back(v7.getId());
  EqualConstraint c6(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v7.getId());
  variables.push_back(v8.getId());
  EqualConstraint c7(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v8.getId());
  variables.push_back(v9.getId());
  EqualConstraint c8(LabelStr("Equal"), LabelStr("Equal"), ce, variables);

  variables.clear();
  variables.push_back(v0.getId());
  variables.push_back(v1.getId());
  variables.push_back(v2.getId());
  variables.push_back(v3.getId());
  variables.push_back(v4.getId());
  variables.push_back(v5.getId());
  variables.push_back(v6.getId());
  variables.push_back(v7.getId());
  variables.push_back(v8.getId());
  variables.push_back(v9.getId());

  Variable<IntervalIntDomain>*  p_v0 = (Variable<IntervalIntDomain>*) v0.getId();

  int lb = -1000;
  int ub = 1000;

  for (int i = 10; i > 2; i--) {
    lb += 100;
    ub -= 100;
    IntervalIntDomain newDomain(lb, ub);
    Variable<IntervalIntDomain>* p_v = (Variable<IntervalIntDomain>*) variables[i-1];
    p_v->specify(newDomain);
    ce->propagate();
    assertTrue(ce->constraintConsistent());
    assertTrue(p_v0->getDerivedDomain().getUpperBound() == ub);
    assertTrue(p_v0->getDerivedDomain().getLowerBound() == lb);
  }
}

static void outerLoopIntervalEqualConstraint(bool useEquivalenceClasses) {
  ConstraintEngine ce;

  if (useEquivalenceClasses)
    new EqualityConstraintPropagator(LabelStr("Equal"), ce.getId());
  else
    new DefaultPropagator(LabelStr("Equal"), ce.getId());

  for (int i = 0; i < 1000; i++)
    testIntervalEqualityPerformance(ce.getId());
}

int main() {
  //outerLoopForTestEquate();
  outerLoopForTestIntersection();
  //outerLoopLabelSetEqualConstraint(true);
  //outerLoopIntervalEqualConstraint(true);
  cout << "Finished" << endl;
  exit(0);
}
