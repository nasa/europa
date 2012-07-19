package BlocksWorld;

import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.HashMap;
import psengine.*;

public class BlockWorld
{
	protected List<List<String>> towers_;
	protected Map<String,List<String>> blockToTower_;
	
	public BlockWorld()
	{
	    towers_ = new Vector();
	    blockToTower_ = new HashMap();
	}
	
	public List<List<String>> getTowers() { return towers_; }
	
	public void addBlock(String name,String state,String bottomBlock, PSToken a)
	{
		//System.out.println("BlockWorld.add("+name+","+state+","+bottomBlock+")");

		List<String> tower = blockToTower_.get(name);
		if (tower == null) {
        	tower = new Vector<String>();	
        	towers_.add(tower);
        	blockToTower_.put(name, tower);
		}

		int idx = tower.indexOf(name);
		StringBuffer buf = new StringBuffer();
		buf.append(name).append("=").append(state).append(bottomBlock != null ? "-"+bottomBlock : "")
		   .append("(").append(getBounds(a)).append(")");
		String towerValue = buf.toString();
		
		if (state.equals("OnTable")) {
			if (idx == -1) 
    			tower.add(0,towerValue);
			else
				tower.set(idx,towerValue);
		}
		
		if (state.equals("On")) {
			
			// Add Bottom if we need to
            int bottomIdx = -1;
            for (int i=0;i<tower.size();i++) {
            	if (tower.get(i).toString().startsWith(bottomBlock)) {
            		bottomIdx = i;
            		break;
            	}
            }

            if (bottomIdx == -1) {
            	bottomIdx = (idx == -1 ? 0 : idx++);
            	tower.add(bottomIdx,bottomBlock);
            	blockToTower_.put(bottomBlock, tower);
            }

			if (idx == -1) 
                tower.add(bottomIdx+1,towerValue);
			else 
    			tower.set(idx,towerValue);
		}
    }
	
	public String toString()
	{
		StringBuffer buf = new StringBuffer();
		
		for (List<String> tower : towers_) {
			buf.append("{");
			for (int i=0;i<tower.size();i++) {
				if (i > 0)
					buf.append(",");
		        buf.append(tower.get(i));				
			}
			buf.append("}  ");
		}
		
		return buf.toString();
	}

    // TODO: move to utils package
    public static Object safeBound(int bound)
    {
    	double INF=1e6;
        if (bound < -INF)
        	return "-INF";
        if (bound > INF)
        	return "INF";
        
        return bound;
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
    		return v.asObject().getName();
    	
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
    	
    	throw new RuntimeException("Unexpected ERROR: variable "+var.getName()+" is not one of {Singleton, Interval, Enumeration}");
    }

    public static String getBounds(PSToken t)
    {
        StringBuffer buf = new StringBuffer();
        
        buf.append("[")
           .append(varValueToString(t.getStart()))
           .append(",")
           .append(varValueToString(t.getEnd()))
           .append("]");
        
        return buf.toString();
    }	
}
