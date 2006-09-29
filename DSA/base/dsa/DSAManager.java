package dsa;

import dsa.impl.DSAImpl;

public class DSAManager 
{
    public static DSA getInstance()
    {
    	return DSAImpl.instance();
    }
}
