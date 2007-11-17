package UBO;

import java.util.List;
import java.util.Vector;

import psengine.*;

/*
 * This is a thin wrapper around PSResource
 */
public class PSResourceWrapper extends ResourceBase 
{
    public PSResourceWrapper(PSEngineWithResources pse,PSResource r, int capacity)
    {
    	super(pse,r,capacity);
    }
        
    public List<PSToken> getConflictSet(int t)
    {
    	List<PSToken> retval = new Vector<PSToken>();

    	PSTimePointList tokKeys = res_.getOrderingChoices(t);
        for (int i=0;i<tokKeys.size();i++) {
        	PSToken tok = psengine_.getTokenByKey(tokKeys.get(i));
           	retval.add(tok);
        }
        
    	return retval;
    }
    
    public int getMostViolatedTime()
    {
    	int t = -1;
    	double lowestLevel = Double.MAX_VALUE;
    	
		PSResourceProfile prof = res_.getLevels();
		PSTimePointList times = prof.getTimes();
		for (int i=0; i<times.size(); i++) {
			double level = prof.getLowerBound(times.get(i));
			if (level < lowestLevel) {
				lowestLevel = level;
				if (level < 0)
					t = times.get(i);
			}
		}
		
		if (t>=0)
		    RCPSPUtil.dbgout("MaxViolation for "+res_.getName()+" "+lowestLevel+" at time "+t);
		
		return t;    	
    }        
}
