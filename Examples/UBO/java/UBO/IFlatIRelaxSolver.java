package UBO;

import java.util.Collection;
import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.TreeSet;
import java.util.HashSet;
import java.util.Vector;
import psengine.*;
import org.ops.ui.util.SimpleTimer;

public class IFlatIRelaxSolver 
{
    int curIteration_ = 0;
    int nbStable_ = 0;       
	int maxStable_ = 10000;
    SimpleTimer timer_;
    boolean timedOut_;
    long timeout_;

    int bestMakespan_;
    long timeToBest_;
    int makespanBound_;
    List<Precedence> bestSolution_;
    
    PSEngine psengine_;
    boolean usePSResources_;
    List<Resource> resources_;
    
    SortedMap<Integer,PSToken> activities_;
    Set<Integer> criticalPath_;
    List<Precedence> precedences_;
    Map<String,Integer> noGoods_;
    boolean hasViolations_;
    
    // TODO: pass in problem definition to be able to compute critical path
    public void solve(PSEngine psengine,
    		          long timeout, // in msecs
    		          int bound, 
    		          boolean usePSResources)
    {
       init(psengine,timeout,bound,usePSResources);
       
       for (int i=0; true ; i++) {
          flatten();
          updateSolution(i);
          updateCriticalPrecedences();
          
          if ((nbStable_ > maxStable_) || (bestMakespan_ <= makespanBound_))
             break;
          
          if (timer_.getElapsed() > timeout) {
              timedOut_ = true;
              break;
          }
          
          relax();
          curIteration_++;
       }       

       restoreBestSolution();
       timer_.stop();
       RCPSPUtil.dbgout("IFlatIrelax.solve() done in "+timer_.getElapsedString());
       RCPSPUtil.dbgout("best makespan is "+bestMakespan_+" for solution "+getSolutionAsString());
    }

    public boolean timedOut() { return timedOut_; }
    public long getElapsedMsecs() { return timer_.getElapsed(); }
    public int getBestMakespan() { return bestMakespan_; }
    public long getTimeToBest() { return timeToBest_; }
    
    protected Resource makeResource(PSResource r,int capacity)
    {
    	if (usePSResources_)
    		return new PSResourceWrapper(psengine_,r,capacity);
    	else
    		return new RCPSPResource(psengine_,r,capacity);
    }
    
    protected void init(PSEngine psengine,long timeout,int bound, boolean usePSResources)
    {
        timer_ = new SimpleTimer();
        timeout_ = timeout;
        timedOut_ = false;
        timeToBest_=timeout;
        
        psengine_ = psengine;
        makespanBound_ = bound;
        usePSResources_ = usePSResources;
        
        List<PSResource> res = PSUtil.toResourceList(psengine.getObjectsByType("CapacityResource"));
        resources_ = new Vector<Resource>();
        
        for (PSResource r : res) {
            PSResourceProfile prof = r.getLimits();
            int t = prof.getTimes().get(0);
            int capacity = (int) prof.getUpperBound(t);
        	resources_.add(makeResource(r,capacity));
            //RCPSPUtil.dbgout("capacity for resource "+r.getName()+" is "+capacity);
        }
        
        PSTokenList tokens = psengine.getTokens();
        activities_ = new TreeMap<Integer,PSToken>();
        for (int i=0;i<tokens.size();i++) {
        	PSToken tok = tokens.get(i);
        	if (tok.getName().startsWith("CapacityResource")) {
        		int ownerIdx = RCPSPUtil.getActivity(tok);
        	    activities_.put(ownerIdx,tok);
        	}
        }

        criticalPath_ = new HashSet<Integer>();
        precedences_ = new Vector<Precedence>();
        noGoods_ = new HashMap<String,Integer>();
        
    	bestMakespan_ = Integer.MAX_VALUE;
        bestSolution_ = new Vector<Precedence>();
        
        curIteration_ = 0;
    	nbStable_ = 0;
    	
    	// Completely relax finish time
    	PSVariable v = psengine_.getVariableByName("maxDuration");
    	v.specifyValue(PSVarValue.getInstance(100000));
    	
        timer_.start();
    }

    public void flatten()
    {
    	hasViolations_ = true;
		boolean addedConstraint = true;
    	
    	while (hasViolations_) {
    		hasViolations_ = false;
    		addedConstraint = false;
    		for (Resource r : resources_) {
    			//RCPSPUtil.dbgout("Before flatten() step : "+r.toString());
    			
    			int t = r.getMostViolatedTime();
    			if (t >= 0) { 
					hasViolations_ = true;
    				if (addPrecedenceConstraint(r,t)) 
    					addedConstraint = true;
    			}

    			//RCPSPUtil.dbgout("After flatten() step : "+r.toString());    			
    		}
    		
    		if (!addedConstraint)
    			break;
    		
            if (timer_.getElapsed() > timeout_) {
                timedOut_ = true;
                break;
            }    		
    	}
    	
    	//if (hasViolations_)
    	//	RCPSPUtil.dbgout("WARNING:unable to remove all violations");   
        //RCPSPUtil.dbgout("flatten() finished "+timer_.getElapsedString());
    }
    
    public void relax()
    {
    	double probOfDeletion = 0.2; 
    	int before = precedences_.size();
    	
    	boolean removed = false;
        psengine_.setAutoPropagation(false);

        // remove some of those on the critical path with some probability
  		List<Precedence> precs = new Vector<Precedence>();
   		precs.addAll(precedences_);
   	    for (Precedence p : precs) {
   	    	if (p.isCritical && (Math.random() < probOfDeletion)) { 
    			removePrecedence(p);    	    		
    			removed = true;
   	    	}
    	    	
   	        if (timer_.getElapsed() > timeout_) {
   	            timedOut_ = true;
   	            break;
   	        }           
   	    }
    	    
   	    // make sure we remove at least one precedence
    	if (!removed) {
            for (Precedence p : precs) {
                if (p.isCritical) {
                    removePrecedence(p);                    
                    break;                    
                }
            }
    	}

        psengine_.setAutoPropagation(true);        
    	
    	//RCPSPUtil.dbgout("Removed "+(before-precedences_.size())+" out of "+before+" precedences");
        //RCPSPUtil.dbgout("after relax. makespan (current,best)=("+getMakespan()+","+bestMakespan_+")");
        //RCPSPUtil.dbgout("relax() finished "+timer_.getElapsedString());
    }
    
    /*
     * 	Try to fix capacity violation on resource r at time t by adding a precedence constraint
     */
    protected boolean addPrecedenceConstraint(Resource r,int t)
    {  
    	List<PSToken> conflictSet = r.getConflictSet(t);
    	
    	if (conflictSet.size() < 2) {
    		RCPSPUtil.dbgout(r.getName()+": conflict set has less than 2 elements, bailing out without adding precedence constraint");
    		return false;
    	}
    	
    	TreeSet<Precedence> candidates = new TreeSet<Precedence>();
    	
    	// look for the pair of tokens with max(succ.LatestStart-pred.EarliestFinish)
    	for (int i=0;i<conflictSet.size();) {
    		PSToken pred = conflictSet.get(i++); 
    		PSToken succ = conflictSet.get(i++); 
   			if ((pred != succ) && !isPrecedence(pred,succ) && !isNoGood(pred,succ)) {
   			    int succStart = RCPSPUtil.getUb(succ.getStart());
   			    int predFinish = RCPSPUtil.getLb(pred.getEnd());
   			    int buffer = succStart-predFinish;
   			    candidates.add(new Precedence(r.getPSResource(),pred,succ,buffer));
                //RCPSPUtil.dbgout(RCPSPUtil.getActivity(succ)+"("+succ.getStart()+") - "+RCPSPUtil.getActivity(pred)+"("+pred.getEnd()+")");
                //RCPSPUtil.dbgout(r.getName()+" "+es+" "+lf+" "+buffer);
   			}
    	}
    	
    	for (Precedence p : candidates) {          
    		addPrecedence(p);    		 
    		if (psengine_.getViolation() > 0) {
    			removePrecedence(p);    			
    			addNoGood(p.pred,p.succ); // this can happen because of the max distance constraints, add a no-good    			
    		} 
    		else
    		    return true;
    	}
    	
		//RCPSPUtil.dbgout("WARNING!: for "+r.getName()+" could not find activity pair with positive slack, bailing out without adding precedence constraint");
    	return false;
    }
    
    String getNoGoodKey(PSToken pred,PSToken succ)
    {
        int actPred = RCPSPUtil.getActivity(pred);
        int actSucc = RCPSPUtil.getActivity(succ);
        return actPred + "<" + actSucc;        
    }
    
    int tabuTenure_ = 1;
    
    void addNoGood(PSToken pred,PSToken succ)
    {
        String key = getNoGoodKey(pred,succ);
        noGoods_.put(key,curIteration_+tabuTenure_);        
        //RCPSPUtil.dbgout(curIteration_+" added noGood:{"+key+"}");
    }
    
    boolean isNoGood(PSToken pred,PSToken succ)
    {
        String key = getNoGoodKey(pred,succ);
        Integer iteration = noGoods_.get(key);
        
        if (iteration == null)
            return false;
        
        return (iteration.intValue() > curIteration_);
    }
    
    protected void addPrecedence(Precedence p)
    {
	    //RCPSPUtil.dbgout("adding {"+RCPSPUtil.getActivity(p.pred)+"<"+RCPSPUtil.getActivity(p.succ)+"} because of "+p.res.getName());
	    p.res.addPrecedence(p.pred,p.succ);
		precedences_.add(p);
	    //RCPSPUtil.dbgout("added {"+RCPSPUtil.getActivity(p.pred)+"<"+RCPSPUtil.getActivity(p.succ)+"} because of "+p.res.getName());
    }

    protected void removePrecedence(Precedence p)
    {
	    //RCPSPUtil.dbgout("removing {"+RCPSPUtil.getActivity(p.pred)+"<"+RCPSPUtil.getActivity(p.succ)+"} because of "+p.res.getName());
		p.res.removePrecedence(p.pred,p.succ);
		precedences_.remove(p);
	    //RCPSPUtil.dbgout("removed {"+RCPSPUtil.getActivity(p.pred)+"<"+RCPSPUtil.getActivity(p.succ)+"} because of "+p.res.getName());    	
    }
    
    protected void updateSolution(int iteration)
    {    	
    	int newMakespan = getMakespan();
    	String violationMsg = (hasViolations_ ? " with violations" : "");
		//RCPSPUtil.dbgout("Iteration "+iteration+": found makespan "+newMakespan+violationMsg);
    	
    	if (newMakespan < bestMakespan_ && !hasViolations_) {
    		bestMakespan_ = newMakespan;
    		bestSolution_.clear();
    		bestSolution_.addAll(precedences_);
    		timeToBest_ = timer_.getElapsed();
    		RCPSPUtil.dbgout("Iteration "+iteration+": new best makespan "+bestMakespan_);
    		nbStable_=0;
    	}
    	else {
    		nbStable_++;
    	}
    }
    
    // TODO: do this faster with a key
    boolean isPrecedence(PSToken pred,PSToken succ)
    {
    	int actPred = RCPSPUtil.getActivity(pred);
    	int actSucc = RCPSPUtil.getActivity(succ);
    	
    	for (Precedence p : precedences_) {
    		if ((RCPSPUtil.getActivity(p.pred)==actPred) && (RCPSPUtil.getActivity(p.succ)==actSucc)) 
    			return true;    		
    	}
    	
    	return false;
    }
    
    protected void restoreBestSolution()
    {
        psengine_.setAutoPropagation(false);
        
        for (Precedence p : precedences_)
        	p.res.removePrecedence(p.pred,p.succ);
        
        for (Precedence p : bestSolution_)
        	p.res.addPrecedence(p.pred,p.succ);               

        psengine_.setAutoPropagation(true);        
    }
    
    public void undoSolve()
    {
        psengine_.setAutoPropagation(false);

        for (Precedence p : bestSolution_)
        	p.res.removePrecedence(p.pred,p.succ);
        
        psengine_.setAutoPropagation(true);        
        
        reset();
    }
    
    protected void reset()
    {
    	bestSolution_.clear();
    	precedences_.clear();
    	noGoods_.clear();
    }
   
    
    public Collection<PSToken> getActivities()
    {
    	return activities_.values();
    }
    
    protected void updateCriticalPrecedences()
    {
        // TODO: this only works if we're working with a solution that doesn't have any temporal violations
        /*
        int savedUb = RCPSPUtil.getUb(getProjectFinish());
        
        // TODO: mark activities on the critical path
        for (Precedence p : precs)  { 
            if (criticalPath_.contains(pred.getKey) || criticalPath_.contains(pred.getKey))
                p.isCritical = true;
            else
                p.isCritical = false;
        } 
        */               
    }
    
    public int getMakespan()
    {
        return RCPSPUtil.getLb(getProjectFinish());
    }
    
    public PSVariable getProjectFinish()
    {
        return activities_.get(activities_.lastKey()).getEnd();
    }

    public String printResources()
    {
    	StringBuffer buf = new StringBuffer();
    	
        for (Resource r : resources_) {
        	buf.append(r.toString()).append("\n");
        }
        
        return buf.toString();        	        	
    }
    
    public String getSolutionAsString()
    {
    	StringBuffer buf = new StringBuffer();
    	
        for (Precedence p : bestSolution_) 
        	buf.append("{").append(RCPSPUtil.getActivity(p.pred)).append("<").append(RCPSPUtil.getActivity(p.succ)).append("}");
        
        return buf.toString();
    }
    
    protected static class Precedence
        implements Comparable
    {
    	public PSResource res;
    	public PSToken pred;
    	public PSToken succ;
    	public boolean isCritical;
    	public int buffer;
    	
    	public Precedence(PSResource r,PSToken p,PSToken s)
    	{
    	    this(r,p,s,0);
            int succStart = RCPSPUtil.getUb(succ.getStart());
            int predFinish = RCPSPUtil.getLb(pred.getEnd());
            buffer = succStart-predFinish;              	    
    	}
    	
        public Precedence(PSResource r,PSToken p,PSToken s, int b)
    	{
    		res = r;
    		pred = p;
    		succ = s;    	
    		isCritical = true;
            buffer = b;
    	}   
    	
    	public int compareTo(Object o)
    	{
    	    Precedence rhs = (Precedence)o;
    	    return rhs.buffer - buffer;
    	}
    }       
}
