package org.ops.ui.utils;

import java.lang.reflect.Method;
import java.util.List;
import java.util.Vector;

import psengine.PSValueList;
import psengine.PSVariable;

public class Utilities {

	public static String getUserFriendlyValue(PSVariable variable) {
		
		if(variable.isSingleton()) {
			return variable.getSingletonValue().toString();
		}
		
		else if (variable.isInterval()) {
			// TODO:  Combine this with gantt hover functionality for a common output
			// format for variables:
			return "[" + variable.getLowerBound() + ", " + variable.getUpperBound() + "]";
		}
		else if (variable.isEnumerated()){ 
			String retval = "{";
			
			PSValueList vals = variable.getValues();
			
			if(vals.size() == 1) {
				return vals.get(0).toString();
			}
			
			boolean firstOne = true;
			for(int i = 0 ;i < vals.size(); ++i) {
				if(!firstOne) {
					retval = retval + ", " + vals.get(i).toString();
				}
				else {
					retval = "{" + vals.get(i).toString();
					firstOne = false;
				}
			}
			
			return retval + "}";
		}
		else {
			return "TODO:  Provide labels for variable " + variable.getEntityName();
		}
	}

	/*
	 * Transform SWIG generated lists into Java lists
	 * parameter list must support size() and get(int idx)
	 */
	public static List<Object> SWIGList(Object list)
	{
		try {
		    List<Object> retval = new Vector<Object>();
		
		    Method m = list.getClass().getMethod("size", (Class[])null);
		    int size = (Integer)m.invoke(list,(Object[])null);
		    m = list.getClass().getMethod("get", new Class[]{int.class});
		    Object args[] = new Object[1];
		    for (int i=0; i<size;i++) {
		    	args[0] = i;
		    	retval.add(m.invoke(list, args));
		    }
		    	
	    	return retval;
		}
		catch (Exception e) {
			throw new RuntimeException(e);
		}
	}
}
