package testLang;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class TestSetGenerator {
  private static Map testsMap;
  private static LinkedList testsList;

  static {
    testsMap = new HashMap();
    testsList = new LinkedList();
  }

  public static String addSet(String name) {
    String setName = "TestSet_" + name;
    if(testsMap.containsKey(setName))
      return setName;
    TestSet set = new TestSet(setName);
    testsMap.put(setName, set);
    testsList.addFirst(set);
    return setName;
  }

  public static String addSet(String parentName, String name) {
    String setName = addSet(name);
    ((TestSet)testsMap.get(parentName)).addTest(setName);
    return setName;
  }

  public static void addAssertion(String parentName, String name) {
    ((TestSet)testsMap.get(parentName)).addAssertion(name);
  }

  public static void toCpp(CppFile file) {
    for(Iterator it = testsList.iterator(); it.hasNext();)
      ((TestSet)it.next()).toCpp(file);
  }
}
