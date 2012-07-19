package dsa;

public class DSAManager 
{
    public static DSA getDSA()
    {
    	return DSAImpl.instance();
    }
}
