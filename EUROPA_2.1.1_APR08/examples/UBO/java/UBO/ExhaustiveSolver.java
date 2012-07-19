package UBO;


/*
 * This is an exhaustive solver used by the Hybrid solver, it uses :
 * - The incoming precedenceOracle to guide precedence selection at each decision point
 * - the incoming upper bound to prune the search tree faster.
 * TODO: this was a very quick implementation to try the hybrid idea, it doesn't scale very well. 
 *        use ideas from best performing B&B solvers for this problem, combined with the oracle and strengthened bound, it should kick ass.
 */
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
        doSolve();
    }
    
    protected void doSolve()
    {
        for (;;) {
            if (timer_.getElapsed() > timeout_) {
                timedOut_ = true;
                doSolveFinished("timed out");
                break;
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
                    RCPSPUtil.dbgout("ExhaustiveSolver found new best makespan "+bestMakespan_+" "+getSolutionAsString());                    
                }  

                // if upper and lower bound cross, we're done
                if (upperBound_ <= lowerBound_) {
                    doSolveFinished("Bounds crossed");
                    break;
                }    

                if (decisionStack_.isEmpty()) {
                    doSolveFinished("exhausted");
                    break;
                }
                else 
                    doBacktrack("Backtracking after finding solution");
            }
            else {
                if (curDP_.hasNext()) {
                    executeDecision(curDP_);

                    if (psengine_.getViolation() > 0) 
                        doBacktrack("Backtracking because of violation");
                    else if (getMakespan() > upperBound_) 
                        doBacktrack("Backtracking because of upper bound "+getMakespan()+" vs. "+upperBound_);
                }
                else if (decisionStack_.isEmpty()) {
                    doSolveFinished("exhausted");
                    break;
                }
                else {
                    doBacktrack("Backtracking because decision point exhausted choices");
                }
            }
        }        
    }
    
    protected void doSolveFinished(String msg)
    {
        timer_.stop();
        RCPSPUtil.dbgout("ExhaustiveSolver.solve finished("+msg+") in "+timer_.getElapsedString()+" best makespan:"+getBestMakespan());
        if (bestMakespan_ < Integer.MAX_VALUE) {
            RCPSPUtil.dbgout("ExhaustiveSolver found new Best Solution:"+getSolutionAsString());
        }
        restoreBestSolution();
        resetState();
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
        activities_ = activities;
        resources_ = resources;
        lowerBound_ = lowerBound;
        upperBound_ = upperBound;
        
        precedenceOracle_ = new TreeSet<String>();
        for (Precedence p : precedenceOracle)
            precedenceOracle_.add(p.toString());
                
        resetSolution();
        resetState();
    }
    
    void resetState()
    {
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
        
        if (maxViolatedResource == null) {
            //RCPSPUtil.dbgout("No violations found!");
            return null;
        }
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
        TreeSet<Precedence> choices_;
        SortedSet<Precedence> executedChoices_;
        Precedence lastExecutedChoice_;
   
        public DecisionPoint(Resource r, int t)
        {
            choices_ = new TreeSet<Precedence>();
            List<PSToken> conflictSet = r.getConflictSet(t);

            if (conflictSet.size() < 2)
                RCPSPUtil.dbgout("ERROR!!: conflictSet size is :"+conflictSet.size());
            
            for (int i=0;i<conflictSet.size();) {
                PSToken pred = conflictSet.get(i++); 
                PSToken succ = conflictSet.get(i++); 
                int succStart = RCPSPUtil.getUb(succ.getStart());
                int predFinish = RCPSPUtil.getLb(pred.getEnd());
                int buffer = succStart-predFinish;
                if (!isPrecedence(pred,succ))
                    choices_.add(new Precedence(r.getPSResource(),pred,succ,buffer));
            }
            
            lastExecutedChoice_ = null;
        }
         
         public boolean hasNext() 
         {
             return !choices_.isEmpty();
         }
         
         public void execute()
         {
             Precedence toExecute = choices_.first();
             
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


