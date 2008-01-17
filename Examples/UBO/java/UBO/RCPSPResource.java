package UBO;

import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import psengine.*;

/*
 * This is a version of Resource that performs much more computation outside EUROPA than PSResourceWrapper
 */
public class RCPSPResource extends ResourceBase 
{	
    public RCPSPResource(PSEngine pse,PSResource r, int capacity)
    {
    	super(pse,r,capacity);
    }
    
    public List<PSToken> getConflictSet(int t)
    {
    	List<PSToken> conflictSet = new Vector<PSToken>();

    	PSTokenList toks = res_.getTokens();
        for (int i=0;i<toks.size();i++ ) {
        	PSToken tok = toks.get(i);
            if (RCPSPUtil.overlaps(tok,t)) 
            	conflictSet.add(tok);
        }

    	List<PSToken> retval = new Vector<PSToken>();
    	for (PSToken pred : conflictSet) {
    		for (PSToken succ : conflictSet) {
    		    // Don't try to order tokens that can't be ordered
    		    int predEndLb = RCPSPUtil.getLb(pred.getEnd());
                int succStartUb = RCPSPUtil.getUb(pred.getStart());
    		    
    			if ((pred != succ) && (predEndLb <= succStartUb)) {
    				retval.add(pred);
    				retval.add(succ);
    			}
    		}
    	}
    	
    	return retval;
    }
    
    public ResourceViolationInfo getMaxViolation()
    {
    	int lowestTime = -1;
    	double lowestLevel = Double.MAX_VALUE;
    	
		ResourceProfile prof = getLevels();
		Iterator<Integer> times = prof.getTimes().iterator();
		while (times.hasNext()) {
			Integer t = times.next();
			int level = prof.getLevel(t);
			if (level < lowestLevel) {
				lowestLevel = level;
				if (level < 0)
					lowestTime = t;
			}
		}
		
		//if (lowestTime>=0)
		//   RCPSPUtil.dbgout("MaxViolation for "+res_.getName()+" "+lowestLevel+" at time "+lowestTime);
		
		return new ResourceViolationInfo(lowestTime,lowestLevel);    	
    }
}
