package org.ops.ui.utils;

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
}
