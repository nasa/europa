package nddl;

import java.util.List;

/**
 * A class containing some useful static functions.
 */
public class NddlUtil
{
	// lispy ways of getting substrings from a dot separated list.
	static String first(String s)
	{
		if(s == null) return "";
		int firstDot = s.indexOf('.');
		if(firstDot==-1) return s;
		else             return s.substring(0,firstDot);
	}
	static String rest(String s)
	{
		if(s == null) return null;
		int firstDot = s.indexOf('.');
		if(firstDot==-1) return null;
		else             return s.substring(firstDot+1);
	}
	static String last(String s)
	{
		if(s == null) return "";
		int lastDot = s.lastIndexOf('.');
		if(lastDot==-1) return s;
		else            return s.substring(lastDot+1);
	}
	static String butLast(String s)
	{
		if(s == null) return null;
		int lastDot = s.lastIndexOf('.');
		if(lastDot==-1) return null;
		else            return s.substring(0,lastDot);
	}
	static String append(String context, String name)
	{
		if(context == null) return name;
		if(name == null) return context;
		if(context.length()>0&&name.length()>0)
			return context+"."+name;
		else if(context.length()>0)
			return context;
		return name;
	}
	static String listAsString(List s)
	{
		// this may not work everywhere, but it should be ok.
		String toRet = s.toString();
		return "(" + toRet.substring(1,toRet.length()-1) +")";
	}
	static String nonull(String s)
	{
		if(s == null) return "";
		else return s;
	}
}
