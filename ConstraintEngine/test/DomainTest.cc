#include "AbstractDomain.hh"
#include <iostream>
#include <cassert>
#include <list>

using namespace Prototype;
using namespace std;

void testEquate(const LabelSet& a, const LabelSet& b)
{
  LabelSet l_a(a);
  LabelSet l_b(b);
  l_a.equate(l_b);
}

void testIntersection()
{
  IntervalIntDomain dom1(100, 1000);
  IntervalIntDomain dom2(250, 2000);
  dom1.intersect(dom2);
}

void outerLoopForTestIntersection()
{
  for (int i=0;i<1000000;i++)
    testIntersection();
}

void outerLoopForTestEquate()
{
  std::list<Prototype::LabelStr> values;
  values.push_back(Prototype::LabelStr("A"));
  values.push_back(Prototype::LabelStr("B"));
  values.push_back(Prototype::LabelStr("C"));
  values.push_back(Prototype::LabelStr("D"));
  values.push_back(Prototype::LabelStr("E"));
  values.push_back(Prototype::LabelStr("F"));
  values.push_back(Prototype::LabelStr("G"));
  values.push_back(Prototype::LabelStr("H"));
  LabelSet ls_a(values);

  values.clear();
  values.push_back(Prototype::LabelStr("1"));
  values.push_back(Prototype::LabelStr("2"));
  values.push_back(Prototype::LabelStr("3"));
  values.push_back(Prototype::LabelStr("E"));
  values.push_back(Prototype::LabelStr("4"));
  values.push_back(Prototype::LabelStr("5"));
  values.push_back(Prototype::LabelStr("6"));
  values.push_back(Prototype::LabelStr("7"));
  values.push_back(Prototype::LabelStr("8"));
  LabelSet ls_b(values);

  for (int i=0;i<1000000;i++)
    testEquate(ls_a, ls_b);
}

void main()
{
  //outerLoopForTestEquate();
  outerLoopForTestIntersection();
  cout << "Finished" << endl;
}
