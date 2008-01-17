package UBO;

import java.util.Collection;
import java.util.List;
import java.util.SortedMap;

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
    protected List<Precedence> precedences_;
    
    protected List<Precedence> bestSolution_;    
        
    public Collection<PSToken> getActivities() { return activities_.values(); }    
    public List<Resource> getResources()       { return resources_; }
    public List<Precedence> getPrecedences()   { return precedences_; }

    public long getElapsedMsecs() { return timer_.getElapsed(); }
    public int getBestMakespan() { return bestMakespan_; }
    public long getTimeToBest() { return timeToBest_; }
    public List<Precedence> getBestSolution() { return bestSolution_; }
    
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
        return activities_.get(activities_.lastKey()).getEnd();
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
}
