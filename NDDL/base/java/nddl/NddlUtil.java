package nddl;

import java.text.ParseException;
import java.text.ParsePosition;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

/**
 * A class containing some useful static functions.
 */
public class NddlUtil {
  // lispy ways of getting substrings from a dot separated list.
  public static String first(String s) {
    if(s == null) return "";
    int firstDot = s.indexOf('.');
    if(firstDot==-1) return s;
    else             return s.substring(0,firstDot);
  }

  public static String rest(String s) {
    if(s == null) return null;
    int firstDot = s.indexOf('.');
    if(firstDot==-1) return null;
    else             return s.substring(firstDot+1);
  }

  public static String last(String s) {
    if(s == null) return "";
    int lastDot = s.lastIndexOf('.');
    if(lastDot==-1) return s;
    else            return s.substring(lastDot+1);
  }

  public static String butLast(String s) {
    if(s == null) return null;
    int lastDot = s.lastIndexOf('.');
    if(lastDot==-1) return null;
    else            return s.substring(0,lastDot);
  }

  public static String append(String context, String name) {
    if(context == null) return name;
    if(name == null) return context;
    if(context.length()>0&&name.length()>0)
      return context+"."+name;
    else if(context.length()>0)
      return context;
    return name;
  }

  public static String listAsString(List s) {
    // this may not work everywhere, but it should be ok.
    String toRet = s.toString();
    return "(" + toRet.substring(1,toRet.length()-1) +")";
  }

  public static String nonull(String s) {
    if(s == null) return "";
    else return s;
  }

  public static String getenv(String name) {
    String toRet = System.getProperty(name);
    if(toRet == null) {
      try {
        toRet = System.getenv(name);
      }
      catch(Exception ex) {
        toRet = null;
      }
    }
    return toRet;
  }

  public static final Set immutableSet(String[] elements) {
    Set toRet = new HashSet(elements.length);
    for(int i = 0; i < elements.length; ++i) 
      toRet.add(elements[i]);
    return Collections.unmodifiableSet(toRet);
  }

  public static String toDelimitedString(Set elements, String delimiter) {
    return toDelimitedString(elements, delimiter, false);
  }

  public static String toDelimitedString(Set elements, String delimiter, boolean clip) {
    StringBuffer toRet = new StringBuffer(128);
    boolean clipStart = clip;
    for(Iterator i = elements.iterator(); i.hasNext();) {
      Object element = i.next();
      if(!clipStart)
        toRet.append(delimiter);
      else
        clipStart = false;
      toRet.append(element.toString());
    }
    if(!clip)
      toRet.append(delimiter);
    return toRet.toString();
  }

  public static String expandEnvVariables(String str) throws ParseException {
    return expandEnvVariables(str, new ParsePosition(0));
  }

  public static String expandEnvVariables(String str, ParsePosition pos) throws ParseException {
    if(pos.getIndex() == str.length())
      return "";
    StringBuffer toRet = new StringBuffer((int) ((str.length() - pos.getIndex()) * 1.5));
    char c;
    do {
      c = str.charAt(pos.getIndex());
      inc(pos);
      switch(c) {
        case '$':
          toRet.append(expandVariable(str, pos));
          break;
        case '\\':
          if(pos.getIndex() < str.length()) {
            toRet.append(str.charAt(pos.getIndex()));
            inc(pos);
          }
          else
            toRet.append('\\');
          break;
        default:
          toRet.append(c);
      }
    } while(pos.getIndex() < str.length());
    return toRet.toString();
  }

  private static String expandVariable(String str, ParsePosition pos) throws ParseException {
    if(pos.getIndex() == str.length())
      throw new ParseException("Expecting Environment Variable Name, found End of String.", pos.getIndex());
    char c = str.charAt(pos.getIndex());
    switch(c) {
      case '$':
        String env = getenv(expandVariable(str, inc(pos)));
        return (env != null)? env : "";
      case '{':
        return expandComplexVariable(str, inc(pos));
      default:
        return expandSimpleVariable(str, pos);
    }
  }

  private static String expandSimpleVariable(String str, ParsePosition pos) throws ParseException {
    char c = str.charAt(pos.getIndex());

    int start = pos.getIndex();
    if(!((c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         (c == '_')))
      throw new ParseException("Expecting Start of Environment Variable Name, found invalid character '" + c + "'.", pos.getIndex());
    do {
      inc(pos);
      if(pos.getIndex() == str.length())
        break;
      c = str.charAt(pos.getIndex());
    } while((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '_'));
    String env = getenv(str.substring(start, pos.getIndex()));
    return (env != null)? env : "";
  }

  private static String expandComplexVariable(String str, ParsePosition pos) throws ParseException {
    if(pos.getIndex() == str.length())
      throw new ParseException("Expecting Environment Variable Name followed by '}', found end of String", pos.getIndex());
    StringBuffer toRet = new StringBuffer((int) ((str.length() - pos.getIndex()) * 1.5));
    int subStart;
    char c;
    do {
      c = str.charAt(pos.getIndex());
      subStart = pos.getIndex();
      inc(pos);
      switch(c) {
        case '$':
          validateEnvNameAppend(toRet, expandVariable(str, pos), subStart);
          break;
        case '\\':
          if(pos.getIndex() < str.length()) {
            validateEnvNameAppend(toRet, new Character(str.charAt(pos.getIndex())), subStart);
            inc(pos);
          }
          else
            validateEnvNameAppend(toRet, new Character('\\'), subStart);
          break;
        case '}':
          String env = getenv(toRet.toString());
          return (env != null)? env : "";
        default:
          validateEnvNameAppend(toRet, new Character(c), subStart);
      }
    } while(pos.getIndex() < str.length());
    throw new ParseException("Expecting '}' before end of string", pos.getIndex());
  }

  private static void validateEnvNameAppend(StringBuffer sb, Object o, int subStart) throws ParseException {
    char c;
    if(o instanceof Character) {
      c = ((Character) o).charValue();
      if(sb.length() == 0) {
        if((c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z') ||
             (c == '_'))
          sb.append(c);
        else 
          throw new ParseException("Expecting Start of Environment Variable Name in subexpression, found invalid character '" + c + "'.", subStart);
      }
      else {
        if((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '_'))
          sb.append(c);
        else
          throw new ParseException("Expecting Environment Variable Name in subexpression, found invalid character '" + c + "'.", subStart);
      }
    }
    else if(o instanceof String) {
      String str = (String) o;
      if(str.length() == 0)
        throw new ParseException("Expecting Environment Variable Name, empty string found", subStart);
      c = str.charAt(0);
      int i=0;
      if(sb.length() == 0) { 
        if(!((c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z') ||
             (c == '_')))
          throw new ParseException("Expecting Start of Environment Variable Name, found invalid character '" + c + "'.", subStart);
        i=1;
      }
      while(i<str.length()) {
        c = str.charAt(i++);
        if(!((c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z') ||
             (c >= '0' && c <= '9') ||
             (c == '_')))
          throw new ParseException("Expecting Environment Variable Name in subexpression, found invalid character '" + c + "'.", subStart);
      }
      sb.append(str);
    }
    // As this is a private method, it's input should be controlled enough to prevent this from happening.
    else
      throw new ClassCastException("Expecting a String or Character, found a " + o.getClass().getName());
  }

  private static ParsePosition inc(ParsePosition pos) {
    pos.setIndex(pos.getIndex() + 1);
    return pos;
  }
}
