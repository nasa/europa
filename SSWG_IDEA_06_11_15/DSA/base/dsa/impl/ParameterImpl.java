package dsa.impl;

import dsa.Parameter;

public class ParameterImpl 
    implements Parameter 
{
	protected String name_;
	protected String value_;
	
	public ParameterImpl(String n,String v)
	{
		name_ = n;
		value_ = v;
	}

	public String getName() { return name_; }
	public String getValue() { return value_; }
}
