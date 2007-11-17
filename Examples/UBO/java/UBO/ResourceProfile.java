package UBO;

import java.util.Iterator;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Set;
import java.util.Map.Entry;

import psengine.*;

public class ResourceProfile 
{	 
	protected Resource res_;
	protected SortedMap<Integer,Integer> levels_;
	
	public ResourceProfile(Resource r)
	{
	    res_ = r;
	    init();
	}
	
	protected void init()
	{
		levels_ = new TreeMap<Integer,Integer>();
		levels_.put(0,res_.getCapacity());
		
		PSTokenList acts = res_.getPSResource().getTokens();
		for (int i=0;i<acts.size();i++) {
			PSToken act = acts.get(i);
		    int start = RCPSPUtil.getLb(act.getParameter("start"));	
		    int end = RCPSPUtil.getLb(act.getParameter("end"));
		    int qty = RCPSPUtil.getLb(act.getParameter("quantity"));
		    
		    update(start,end,qty);
		}
		
		
	}
	
	protected void update(int start,int end,int qty)
	{		
		Integer levelAtStart=levels_.get(start);
		if (levelAtStart == null) {		   
    	   Iterator<Entry<Integer,Integer>> it = levels_.entrySet().iterator();				
		   while(it.hasNext()) {
			   Entry<Integer,Integer> entry = it.next();
			   if (entry.getKey() < start)
				   levelAtStart = entry.getValue();
			   else
				   break;
		   }
		   levels_.put(start, levelAtStart);
		}
		
		Integer lastValue=levelAtStart;
		SortedMap<Integer,Integer> submap = levels_.subMap(start, end); // this doesn't include end
 	    Iterator<Entry<Integer,Integer>> it = submap.entrySet().iterator();				
	    while(it.hasNext()) {
		   Entry<Integer,Integer> entry = it.next();
		   lastValue = entry.getValue()-qty;
           entry.setValue(lastValue);		   
	    }
		
		Integer levelAtEnd=levels_.get(end);
		if (levelAtEnd == null)
			levels_.put(end, lastValue+qty);
      
	}	
	
	public Set<Integer> getTimes() { return levels_.keySet(); }
	
	// TODO: make this more sophisticated
	public Integer getLevel(Integer time) { return levels_.get(time); }
	
	public String toString()
	{
		StringBuffer buf = new StringBuffer();

		Iterator<Integer> times = getTimes().iterator();
		while (times.hasNext()) {
			Integer t = times.next();
			buf.append("{").append(t).append("=").append(getLevel(t)).append("}");
		}
		
		return buf.toString();
	}
}
