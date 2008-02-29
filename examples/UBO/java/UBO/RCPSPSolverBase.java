package UBO;

import java.util.Collection;
import java.util.Comparator;
import java.util.List;
import java.util.SortedMap;
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.Vector;

import org.ops.ui.util.SimpleTimer;

import psengine.PSEngine;
import psengine.PSToken;
import psengine.PSVariable;

public abstract class RCPSPSolverBase 
    implements RCPSPSolver 
{
    protected SimpleTimer timer_;
    protected boolean timedOut_;
    protected long timeout_;
    protected long timeToBest_;
    protected int bestMakespan_;

    protected PSEngine psengine_;
    protected List<Resource> resources_;    
    protected SortedMap<Integer,PSToken> activities_;
    protected SortedSet<Precedence> precedences_;
    
    protected SortedSet<Precedence> bestSolution_;    
        
    public String getName() { return getClass().getSimpleName(); }
        
    public Collection<PSToken> getActivities() { return activities_.values(); }    
    public List<Resource> getResources()       { return resources_; }
    public SortedSet<Precedence> getPrecedences()   { return precedences_; }

    public long getElapsedMsecs() { return timer_.getElapsed(); }
    public int getBestMakespan() { return bestMakespan_; }
    public long getTimeToBest() { return timeToBest_; }
    public SortedSet<Precedence> getBestSolution() { return bestSolution_; }
    
    public void undoSolve()
    {
        psengine_.setAutoPropagation(false);

        for (Precedence p : bestSolution_)
            p.res.removePrecedence(p.pred,p.succ);
        
        psengine_.setAutoPropagation(true);        
    }
    
    public void restoreBestSolution()
    {
        psengine_.setAutoPropagation(false);
        
        for (Precedence p : precedences_)
            p.res.removePrecedence(p.pred,p.succ);
        precedences_.clear();
        
        for (Precedence p : bestSolution_)
            p.res.addPrecedence(p.pred,p.succ);               

        psengine_.setAutoPropagation(true);        
    }
        
    public String getSolutionAsString()
    {
        StringBuffer buf = new StringBuffer();
        
        for (Precedence p : bestSolution_) 
            buf.append("{").append(RCPSPUtil.getActivity(p.pred)).append("<").append(RCPSPUtil.getActivity(p.succ)).append("}");
        
        return buf.toString();
    }    
    
    public int getMakespan()
    {
        return RCPSPUtil.getLb(getProjectFinish());
    }
    
    public PSVariable getProjectFinish()
    {
        return activities_.get(activities_.lastKey()).getStart();
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

    
    protected void resetSolution()
    {
        bestMakespan_ = Integer.MAX_VALUE;
        bestSolution_ = new TreeSet<Precedence>(new PrecedenceComparator());        
        precedences_ = new TreeSet<Precedence>(new PrecedenceComparator());
    }
    
    static class PrecedenceComparator
        implements Comparator<Precedence>
    {
        public int compare(Precedence o1, Precedence o2) 
        {
            int diff = RCPSPUtil.getActivity(o1.pred) - RCPSPUtil.getActivity(o2.pred);

            if (diff != 0) 
                return diff;
            else                
                return RCPSPUtil.getActivity(o1.succ) - RCPSPUtil.getActivity(o2.succ);
        }
    }    
}
