package UBO;

import java.util.Collection;
import psengine.PSToken;
import psengine.PSValueList;
import psengine.PSVarValue;
import psengine.PSVariable;

public class RCPSPUtil 
{
    public static int getActivity(PSToken tok)
    {
        return 	tok.getMaster().getParameter("m_identifier").getSingletonValue().asInt();
    }
    
    public static int getLb(PSVariable v)
    {
    	if (v.isSingleton())
    		return (new Double(valueToString(v.getSingletonValue()))).intValue();
    	else if (v.isInterval())
    		return (int)v.getLowerBound();
    	
    	throw new RuntimeException("Can only get lb for singleton or interval vars");
    }
    
    public static int getUb(PSVariable v)
    {
    	if (v.isSingleton())
    		return (new Double(valueToString(v.getSingletonValue()))).intValue();
    	else if (v.isInterval())
    		return (int)v.getUpperBound();
    	
    	throw new RuntimeException("Can only get ub for singleton or interval vars");
    }
    
    public static boolean overlaps(PSToken tok, int t)
    {
    	int lb = getLb(tok.getStart());
    	int ub = getLb(tok.getEnd());
    	
    	return ((lb <= t) && (t <= ub));
    }    

    public static String valueToString(PSVarValue v)
    {
    	String type = v.getType().toString();
    	
    	if ("STRING".equals(type))
    		return v.asString();
    	if ("INTEGER".equals(type))
    		return new Integer(v.asInt()).toString();	
    	if ("DOUBLE".equals(type))
    		return new Double(v.asDouble()).toString();
    	if ("BOOLEAN".equals(type))
    		return new Boolean(v.asBoolean()).toString();
    	if ("OBJECT".equals(type))
    		return v.asObject().getEntityName();
    	
    	return "ERROR!!! UNKNOWN TYPE :" + type;
    }

    public static String varValueToString(PSVariable var)
    {	
    	if (var.isSingleton()) 
    		return valueToString(var.getSingletonValue());	
    	else if (var.isInterval()) {
    	    StringBuffer buf = new StringBuffer();
    		buf.append("[").append(var.getLowerBound()).append(",")
    		               .append(var.getUpperBound()).append("]");
    		return buf.toString();
    	}
    	else if (var.isEnumerated()) {
    		PSValueList l = var.getValues();
    	    StringBuffer buf = new StringBuffer();
    	    buf.append("[");
    	    for (int i=0;i<l.size();i++) {
    	    	if (i>0)
    	    		buf.append(",");
    	    	buf.append(valueToString(l.get(i)));
    	    }
    	    buf.append("]");
    	    return buf.toString();
    	}
    	
    	throw new RuntimeException("Unexpected ERROR: variable "+var.getEntityName()+" is not one of {Singleton, Interval, Enumeration}");
    }    

    public static void ground(Collection<PSToken> tokens)
    {
    	for (PSToken t : tokens) {
    		ground(t.getStart());
    		ground(t.getEnd());
    	}
    }
    
    public static void ground(PSVariable v)
    {
    	int value = RCPSPUtil.getLb(v);
    	v.specifyValue(PSVarValue.getInstance(value));
    }    

    public static void undoGround(Collection<PSToken> tokens)
    {
    	for (PSToken t : tokens) {
    		undoGround(t.getStart());
    		undoGround(t.getEnd());
    	}
    }
    public static void undoGround(PSVariable v)
    {
    	int value = RCPSPUtil.getLb(v);
    	v.reset();
    }    

    
    public static void dbgout(String msg)
    {
    	System.out.println(msg);
    }        
}
