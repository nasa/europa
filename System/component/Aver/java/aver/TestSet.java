package aver;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

public class TestSet {
  private String name;
  private List tests;
  private List assertions;
  public TestSet(String name) {
    this.name = name;
    tests = new LinkedList();
    assertions = new LinkedList();
  }
  public void addTest(String testName) {
    tests.add(testName);
  }
  public void addAssertion(String assnName) {
    tests.add(assnName);
  }
  public void toCpp(CppFile file) {
    file.addLine("class " + name + " : public TestSet {");
    file.addLine("public:");
    file.indent();
    file.addLine(name + "(const PlanDatabaseId& planDb) : TestSet(planDb) {");
    file.indent();
    for(Iterator it = assertions.iterator(); it.hasNext();)
      file.addLine("ADD_ASSERTION(" + (String) it.next() + ");");
    for(Iterator it = tests.iterator(); it.hasNext();)
      file.addLine("ADD_TEST(" + (String) it.next() + ");");
    file.unindent();
    file.addLine("}");
    file.unindent();
    file.addLine("};");
  }
}
