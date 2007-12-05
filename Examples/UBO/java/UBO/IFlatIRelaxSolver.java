package UBO;

import java.util.Collection;
import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Vector;
import psengine.*;
import org.ops.ui.util.SimpleTimer;

public class IFlatIRelaxSolver 
{
    int nbStable_ = 0;       
	int maxStable_ = 100;
    SimpleTimer timer_;

    int bestMakespan_;
    int makespanBound_;
    List<Precedence> bestSolution_;
    
    PSEngine psengine_;
    boolean usePSResources_;
    List<Resource> resources_;
    
    SortedMap<Integer,PSToken> activities_;
    List<Precedence> precedences_;
    List<Precedence> noGoods_;
    boolean hasViolations_;
    
    // TODO: pass in problem definition to be able to compute critical path
    public void solve(PSEngine psengine,
    		          int maxIterations,
    		          int bound, 
    		          boolean usePSResources)
    {
       init(psengine,bound,usePSResources);
       
       for (int i=0; i<maxIterations; i++) {
          flatten();
          updateSolution(i);
          updateCriticalPrecedences();
          
          if ((nbStable_ > maxStable_) || (bestMakespan_ <= makespanBound_))
             break;
          
          relax();
       }       

       restoreBestSolution();
       RCPSPUtil.dbgout("IFlatIrelax.solve() done in "+timer_.getElapsedString());
       RCPSPUtil.dbgout("best makespan is "+bestMakespan_+" for solution "+getSolutionAsString());
    }
    
    
    protected Resource makeResource(PSResource r)
    {
    	int capacity=10;// TODO: don't hardcode capacity
    	if (usePSResources_)
    		return new PSResourceWrapper(psengine_,r,capacity);
    	else
    		return new RCPSPResource(psengine_,r,capacity);
    }
    
    protected void init(PSEngine psengine,int bound, boolean usePSResources)
    {
        timer_ = new SimpleTimer();

        psengine_ = psengine;
        makespanBound_ = bound;
        usePSResources_ = usePSResources;
        
        List<PSResource> res = PSUtil.toResourceList(psengine.getObjectsByType("CapacityResource"));
        resources_ = new Vector<Resource>();
        
        for (PSResource r : res) 
        	resources_.add(makeResource(r));
        
        PSTokenList tokens = psengine.getTokens();
        activities_ = new TreeMap<Integer,PSToken>();
        for (int i=0;i<tokens.size();i++) {
        	PSToken tok = tokens.get(i);
        	if (tok.getName().startsWith("CapacityResource")) {
        		int ownerIdx = RCPSPUtil.getActivity(tok);
        	    activities_.put(ownerIdx,tok);
        	}
        }

        precedences_ = new Vector<Precedence>();
        noGoods_ = new Vector<Precedence>();
        
    	bestMakespan_ = Integer.MAX_VALUE;
        bestSolution_ = new Vector<Precedence>();
        
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
    	}
    	
    	//if (hasViolations_)
    	//	RCPSPUtil.dbgout("WARNING:unable to remove all violations");   
    }
    
    public void relax()
    {
    	double probOfDeletion = 0.02;
    	int before = precedences_.size();
    	
    	for (int i=0;i<4;i++) {
      	    // remove some of those on the critical path with some probability
    		List<Precedence> precs = new Vector<Precedence>();
    		precs.addAll(precedences_);
    	    for (Precedence p : precs) {
    	    	if (p.isCritical && (Math.random() < probOfDeletion)) 
   	    			removePrecedence(p);    	    		
    	    }
    	}
    	
    	RCPSPUtil.dbgout("Removed "+(before-precedences_.size())+" out of "+before+" precedences");
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
    	
    	PSToken bestPred=null,bestSucc=null;
    	int maxBuffer=Integer.MIN_VALUE;
    	
    	// look for the pair of tokens with max(succ.LatestStart-pred.EarliestFinish)
    	for (int i=0;i<conflictSet.size();) {
    		PSToken pred = conflictSet.get(i++); 
    		PSToken succ = conflictSet.get(i++); 
   			if ((pred != succ) && !isPrecedence(pred,succ) && !isNoGood(pred,succ)) {
   			    int succStart = RCPSPUtil.getUb(succ.getParameter("start"));
   			    int predFinish = RCPSPUtil.getLb(pred.getParameter("end"));
   			    int buffer = succStart-predFinish;
   			    //RCPSPUtil.dbgout(RCPSPUtil.getActivity(succ)+"("+succ.getParameter("start")+") - "+RCPSPUtil.getActivity(pred)+"("+pred.getParameter("end")+")");
   			    //RCPSPUtil.dbgout(r.getName()+" "+es+" "+lf+" "+buffer);
   			    if (buffer > maxBuffer) {
   			    	maxBuffer = buffer;
   			    	bestPred = pred;
   			    	bestSucc = succ;
   			    }
   			}
    	}
    	
    	if (maxBuffer >= 0) {
    		Precedence p = new Precedence(r.getPSResource(),bestPred,bestSucc); 
    		addPrecedence(p);
    		if (psengine_.getViolation() > 0) {
    			removePrecedence(p);
    			// this can happen because of the max distance constraints, add a no-good
    			addNoGood(bestPred,bestSucc);
    			RCPSPUtil.dbgout("added noGood:{"+RCPSPUtil.getActivity(bestPred)+"<"+RCPSPUtil.getActivity(bestSucc)+"}");
    			return false;
    		}
    		return true;
    	}
    	
		//RCPSPUtil.dbgout("WARNING!: for "+r.getName()+" could not find activity pair with positive slack, bailing out without adding precedence constraint");
    	return false;
    }
    
    void addNoGood(PSToken pred,PSToken succ)
    {
    	noGoods_.add(new Precedence(null,pred,succ));
    }
    
    boolean isNoGood(PSToken pred,PSToken succ)
    {
    	int actPred = RCPSPUtil.getActivity(pred);
    	int actSucc = RCPSPUtil.getActivity(succ);
    	
    	for (Precedence p : noGoods_) {
    		if ((RCPSPUtil.getActivity(p.pred)==actPred) && (RCPSPUtil.getActivity(p.succ)==actSucc)) 
    			return true;
    	}
    	
    	return false;
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
	    RCPSPUtil.dbgout("removed {"+RCPSPUtil.getActivity(p.pred)+"<"+RCPSPUtil.getActivity(p.succ)+"} because of "+p.res.getName());    	
    }
    
    protected void updateSolution(int iteration)
    {    	
    	int newMakespan = getMakespan();
    	String violationMsg = (hasViolations_ ? " with violations" : "");
		RCPSPUtil.dbgout("Iteration "+iteration+": found makespan "+newMakespan+violationMsg);
    	
    	if (newMakespan < bestMakespan_ && !hasViolations_) {
    		bestMakespan_ = newMakespan;
    		bestSolution_.clear();
    		bestSolution_.addAll(precedences_);
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
        for (Precedence p : precedences_)
        	p.res.removePrecedence(p.pred,p.succ);
        
        for (Precedence p : bestSolution_)
        	p.res.addPrecedence(p.pred,p.succ);               
    }
    
    public void undoSolve()
    {
        for (Precedence p : bestSolution_)
        	p.res.removePrecedence(p.pred,p.succ);
        
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
        // TODO: Implement this	
    }
    
    public int getMakespan()
    {
        return RCPSPUtil.getLb(activities_.get(activities_.lastKey()).getParameter("end"));
    }

    public String printResources()
    {
    	StringBuffer buf = new StringBuffer();
    	
        for (Resource r : resources_) {
        	buf.append(r.toString()).append("\n");
        }
        
        return buf.toString();        	        	
    }
    
    protected String getSolutionAsString()
    {
    	StringBuffer buf = new StringBuffer();
    	
        for (Precedence p : bestSolution_) 
        	buf.append("{").append(RCPSPUtil.getActivity(p.pred)).append("<").append(RCPSPUtil.getActivity(p.succ)).append("}");
        
        return buf.toString();
    }
    
    protected static class Precedence
    {
    	public PSResource res;
    	public PSToken pred;
    	public PSToken succ;
    	public boolean isCritical;
    	
    	public Precedence(PSResource r,PSToken p,PSToken s)
    	{
    		res = r;
    		pred = p;
    		succ = s;    	
    		isCritical = true;
    	}
    }
}
