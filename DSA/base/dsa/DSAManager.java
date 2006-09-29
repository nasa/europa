package dsa;

import dsa.impl.DSAImpl;

public class DSAManager 
{
    public static DSA getDSA()
    {
    	return DSAImpl.instance();
    }
}
