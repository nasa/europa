package UBO;

import java.util.List;
import java.util.SortedMap;
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.Vector;

import org.ops.ui.util.SimpleTimer;

import psengine.PSEngine;
import psengine.PSToken;

public class ExhaustiveSolver
    extends RCPSPSolverBase 
{
    List<DecisionPoint> decisionStack_ = new Vector<DecisionPoint>();
    int lowerBound_;
    int upperBound_;
    SortedSet<String> precedenceOracle_;
    DecisionPoint curDP_;
    
    public ExhaustiveSolver()
    {        
    }
    
    @Override
    public void solve(
            PSEngine psengine, 
            long timeout, 
            int bound,
            boolean usePSResources) 
    {
        // TODO: get this to work
        solve(psengine,timeout,bound,Integer.MAX_VALUE,null,null,null);
    }    

    public void solve(
            PSEngine psengine,
            long timeout,
            int lowerBound,
            int upperBound,
            SortedMap<Integer,PSToken>  activities,
            List<Resource> resources,
            List<Precedence> precedenceOracle)
    {
        init(psengine,timeout,lowerBound,upperBound,activities,resources,precedenceOracle);
        
        for (;;) {
            if (timer_.getElapsed() > timeout_) {
                timedOut_ = true;
                doSolverFinished("timed out");
                return;
            }

            if (curDP_ == null)
                curDP_ = getNextDecisionPoint();
            
            if (curDP_ == null) {
                int makespan = getMakespan();
                //RCPSPUtil.dbgout("ExhaustiveSolver found solution with makespan : "+makespan+" decisionStack depth : "+decisionStack_.size());

                // if we have an improved upper bound, save solution
                if (makespan < upperBound_) {
                    upperBound_ = makespan;
                    bestMakespan_ = upperBound_;
                    bestSolution_.clear();
                    bestSolution_.addAll(precedences_);
                    timeToBest_ = timer_.getElapsed();
                    RCPSPUtil.dbgout("ExhaustiveSolver found new best makespan "+bestMakespan_);                    
                }  
                
                // if upper and lower bound cross, we're done
                if (upperBound_ <= lowerBound_) {
                    doSolverFinished("Bounds crossed");
                    return;
                }    
                
                if (decisionStack_.isEmpty()) {
                    doSolverFinished("exhausted");
                    return;
                }
                else 
                    doBacktrack("Backtracking after finding solution");
            }
            else {
                if (curDP_.hasNext()) {
                    executeDecision(curDP_);

                    if (psengine_.getViolation() > 0) 
                        doBacktrack("Backtracking because of violation");
                    else if (getMakespan() > upperBound_) {
                        //RCPSPUtil.dbgout("Backtracking because of upper bound "+getMakespan()+" vs. "+upperBound_);                        
                        doBacktrack("Backtracking because of upper bound "+getMakespan()+" vs. "+upperBound_);
                    }
                }
                else if (decisionStack_.isEmpty()) {
                    doSolverFinished("exhausted");
                    return;
                }
                else {
                    doBacktrack("Backtracking because decision point exhausted choices");
                }
            }
        }
    }
    
    protected void doSolverFinished(String msg)
    {
        timer_.stop();
        RCPSPUtil.dbgout("ExhaustiveSolver.solve finished("+msg+") in "+timer_.getElapsedString()+" best makespan:"+getBestMakespan());
        if (bestMakespan_ < Integer.MAX_VALUE) {
            restoreBestSolution();
            RCPSPUtil.dbgout("Best Solution:"+getSolutionAsString());
        }
    }
    
    public void init(
            PSEngine psengine,
            long timeout,
            int lowerBound,
            int upperBound,
            SortedMap<Integer,PSToken> activities,
            List<Resource> resources,
            List<Precedence> precedenceOracle)
    {
        timer_ = new SimpleTimer();
        timer_.start();        

        psengine_ = psengine;
        timeout_ = timeout;
        lowerBound_ = lowerBound;
        upperBound_ = upperBound;
        activities_ = activities;
        resources_ = resources;
        
        precedenceOracle_ = new TreeSet<String>();
        for (Precedence p : precedenceOracle)
            precedenceOracle_.add(p.toString());
        
        precedences_ = new Vector<Precedence>();
        bestSolution_ = new Vector<Precedence>();
        bestMakespan_ = Integer.MAX_VALUE;
        curDP_ = null;
    }
    
    int lastIdx_ = 0;
    
    protected DecisionPoint getNextDecisionPoint()
    {
        double lowestLevel = Double.MAX_VALUE;
        Resource maxViolatedResource = null;
        int maxViolatedTime = Integer.MIN_VALUE;
        
        int start = lastIdx_;
        for (int i=lastIdx_;;) {
            Resource r = resources_.get(i);
            ResourceViolationInfo rvi = r.getMaxViolation();
            
            if ((rvi.level < lowestLevel) && (rvi.level < 0)) {
                lowestLevel = rvi.level;
                maxViolatedResource = r;
                maxViolatedTime = rvi.time;
            }
            
            i = (i+1) % resources_.size();
            if (i == start)
                break;
        }  
        
        if (maxViolatedResource == null)
            return null;
        else
            return new DecisionPoint(maxViolatedResource,maxViolatedTime);
    }
    
    protected void executeDecision(DecisionPoint dp)
    {
        decisionStack_.add(dp);
        dp.execute();   
        curDP_ = null;
    }
    
    protected void doBacktrack(String reason)
    {
        curDP_ = decisionStack_.remove(decisionStack_.size()-1);
        curDP_.undo();    
        //RCPSPUtil.dbgout(reason+" new stack size:"+decisionStack_.size());
    }
    
    protected class DecisionPoint
    {
        List<Precedence> choices_;
        SortedSet<Precedence> executedChoices_;
        Precedence lastExecutedChoice_;
   
        public DecisionPoint(Resource r, int t)
        {
            choices_ = new Vector<Precedence>();
            List<PSToken> conflictSet = r.getConflictSet(t);

            if (conflictSet.size() < 2)
                RCPSPUtil.dbgout("ERROR!!: conflictSet size is :"+conflictSet.size());
            
            for (int i=0;i<conflictSet.size();) {
                PSToken pred = conflictSet.get(i++); 
                PSToken succ = conflictSet.get(i++); 
                if (!isPrecedence(pred,succ))
                    choices_.add(new Precedence(r.getPSResource(),pred,succ));
            }
            
            lastExecutedChoice_ = null;
        }
         
         public boolean hasNext() 
         {
             return !choices_.isEmpty();
         }
         
         public void execute()
         {
             Precedence toExecute = choices_.get(0);
             for (Precedence p : choices_) {
                 if (precedenceOracle_.contains(p.toString())) {
                     toExecute = p;
                     //RCPSPUtil.dbgout("Found in oracle:"+p);
                     break;
                 }                     
             }
             
             choices_.remove(toExecute);
             addPrecedence(toExecute);             
             lastExecutedChoice_ = toExecute;
             //RCPSPUtil.dbgout("DP("+decisionStack_.size()+","+choices_.size()+")-addedPrecedence:"+toExecute);
         }
         
         public void undo()
         {
             if (lastExecutedChoice_ == null) {
                 RCPSPUtil.dbgout("ERROR! called DecisionPoint.undo on null choice");
                 return;
             }
             
             removePrecedence(lastExecutedChoice_);             
             //RCPSPUtil.dbgout("DP("+decisionStack_.size()+","+choices_.size()+")-removedPrecedence:"+lastExecutedChoice_);
             lastExecutedChoice_ = null;
         }         
    }
}


