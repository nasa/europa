package UBO;

import java.util.List;
import java.util.SortedMap;
import java.util.SortedSet;
import java.util.TreeSet;
import java.util.Vector;

import psengine.PSEngine;
import psengine.PSToken;
import psengine.util.SimpleTimer;

/*
 * this is a different version of the Exhaustive solver that didn't turn out to be an improvement
 * instead of starting from scratch, it removes 1, then 2, then 3 ... precedences from the incoming oracle and then 
 * performs and exhaustive search.
 * Leaving the code around for now, in case this idea can be reused in some form later.
 * For now it looks like it'll be much better to improve the Exhaustive solver by using ideas from existing B&B solvers
 */
public class SolDistExhaustiveSolver
    extends RCPSPSolverBase 
{
    List<DecisionPoint> decisionStack_ = new Vector<DecisionPoint>();
    int lowerBound_;
    int upperBound_;
    SortedSet<String> precedenceOracle_;
    DecisionPoint curDP_;
    
    public SolDistExhaustiveSolver()
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

    Precedence noGood_ = null;
    
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

        if (precedenceOracle.size() > 0) {
            for (int i=0;i<precedenceOracle.size();i++) {
                OracleIterator it = new OracleIterator(precedenceOracle,i+1);
                
                while (it.hasNext()) {
                    List<Precedence> exclusions = it.next();                    

                    removePrecedences(exclusions);
                    doSolve();

                    if (upperBound_ <= lowerBound_ || timedOut_) {
                        solveFinished(precedenceOracle);
                        return;       
                    }
                    
                    undoSolve();
                    addPrecedences(exclusions);                    
                }
            }
        }
        else {
            doSolve();
        }

        solveFinished(precedenceOracle);
    }
    
    protected void solveFinished(List<Precedence> precedenceOracle)
    {
        bestSolution_.addAll(precedenceOracle);
        timer_.stop();        
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
        // TODO: timer_.stop();
        //RCPSPUtil.dbgout("ExhaustiveSolver.solve finished("+msg+") in "+timer_.getElapsedString()+" best makespan:"+getBestMakespan());
        //if (bestMakespan_ < Integer.MAX_VALUE) {
        //    RCPSPUtil.dbgout("ExhaustiveSolver found new Best Solution:"+getSolutionAsString());
        //}
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
    
    boolean isNoGood(PSToken pred,PSToken succ)
    {
        return false;
       /* 
      if (noGood_ == null)
          return false;
      
      return ((noGood_.pred.getKey()==pred.getKey()) && (noGood_.succ.getKey() == succ.getKey()));
      */
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
                if (!isPrecedence(pred,succ) && !isNoGood(pred,succ))
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
             
             /*
             for (Precedence p : choices_) {
                 if (precedenceOracle_.contains(p.toString())) {
                     toExecute = p;
                     //RCPSPUtil.dbgout("Found in oracle:"+p);
                     break;
                 }                     
             }
             */
             
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
    
    void addPrecedences(List<Precedence> l)
    {
        psengine_.setAutoPropagation(false);
        
        for (Precedence p : l)
            p.res.addPrecedence(p.pred,p.succ);
        
        psengine_.setAutoPropagation(true);                
    }
    
    void removePrecedences(List<Precedence> l)
    {
        psengine_.setAutoPropagation(false);
        
        for (Precedence p : l)
            p.res.removePrecedence(p.pred,p.succ);
        
        psengine_.setAutoPropagation(true);                
    }
    
    class OracleIterator
    {
        int setSize_;
        List<Precedence> source_;
        List<Integer> counter_;
        int counterRow_;
        int max_;
        
        public OracleIterator(List<Precedence> source,int setSize)
        {
            setSize_ = setSize;
            source_ = source;
            counter_ = new Vector<Integer>();
            for (int i=0;i<setSize;i++)
                counter_.add(i);
            
            max_ = source_.size()-1;
            counterRow_=counter_.size()-1;
        }
        
        boolean hasNext()
        {
            return counterRow_ >= 0;   
        }
        
        void incCounter()
        {
            int newValue = counter_.get(counterRow_)+1;
            while ((newValue + (counter_.size()-counterRow_) > max_) && counterRow_>=0) {
                counterRow_--;
                if (counterRow_ >= 0)
                    newValue = counter_.get(counterRow_)+1;                
            }
            
            if (counterRow_ >=0) {
                for (int i=0;(counterRow_+i)<counter_.size();i++)
                     counter_.set(counterRow_+i, newValue+i);
            }
        }
        
        List<Precedence> next()
        {
            List<Precedence> retval = new Vector<Precedence>();
            
            for (Integer i : counter_)
                retval.add(source_.get(i));
            
            incCounter();
            //printExcluded(retval);
            
            return retval;
        }
        
        void printExcluded(List<Precedence> retval)
        {
            StringBuffer buf = new StringBuffer();
            buf.append("Excluded:");
            for (Precedence p: retval)
                buf.append(p.toString()).append(",");
            RCPSPUtil.dbgout(buf.toString());            
        }
    }
        
}


