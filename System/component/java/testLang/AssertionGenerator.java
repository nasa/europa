package testLang;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import net.n3.nanoxml.*;

public class AssertionGenerator {
  private static Map asstnMap;
  private static Map testNumMap;

  static {
    asstnMap = new HashMap();
    testNumMap = new HashMap();
  }

  public static String addAssertion(String testName, IXMLElement xml) {
    Integer suffix = null;
    if(!testNumMap.containsKey(testName))
      testNumMap.put(testName, new Integer(0));
    suffix = (Integer) testNumMap.get(testName);
    testNumMap.put(testName, new Integer(suffix.intValue() + 1));
    String asstnName = testName + "_Assertion" + suffix.toString();
    asstnMap.put(asstnName, new Assertion(asstnName, xml));
    return asstnName;
  }

  public static void toCpp(CppFile header, CppFile impl) {
    for(Iterator it = asstnMap.values().iterator(); it.hasNext();)
      ((Assertion)it.next()).toCpp(header, impl);
  }
}
