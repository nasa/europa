package UBO;

import java.util.Collection;
import java.util.List;
import java.util.Vector;

import org.ops.ui.util.SimpleTimer;

import psengine.*;

/*
 * Seeing the the IFIRSolver very quickly finds good quality solutions, and that optimal solutions are normally not far away
 * This solvers combined local search with exhaustive search :
 * - if first runs IFIR for a portion of the allowed time to try to find a good quality solution
 * - it then runs an exhaustive solver that uses the IFIR solution as an oracle to guide decisions, and the makespan as a 
 *   sharpened upper bound
 */
public class HybridSolver
    implements RCPSPSolver
{
    IFlatIRelaxSolver ifirSolver_;
    ExhaustiveSolver exhSolver_;
    SimpleTimer timer_;
    RCPSPSolver lastSolver_;
    
    public HybridSolver()
    {       
    }    
    
    public String getName() { return "HybridSolver"; }
    
    public void solve(PSEngine psengine,
            long timeout, // in msecs
            int bound, 
            boolean usePSResources)
    {
        timer_ = new SimpleTimer();
        timer_.start();
        
        long ifirTimeout = (long)(timeout*0.5);
        long exhTimeout = timeout-ifirTimeout;
        
        int ifirBound = (int)(bound*1.0); // TODO: use 10% higher?
        ifirSolver_ = new IFlatIRelaxSolver();        
        ifirSolver_.solve(psengine, ifirTimeout, ifirBound, usePSResources);
        
        lastSolver_ = ifirSolver_;
        int upperBound = ifirSolver_.getBestMakespan();
        if (upperBound <= bound) {
            timer_.stop();
            return;
        }
        
        List<Precedence> oracle = new Vector<Precedence>();
        oracle.addAll(ifirSolver_.getBestSolution());
        ifirSolver_.undoSolve(); 
        
        exhSolver_ = new ExhaustiveSolver();
        exhSolver_.solve(
                psengine,
                exhTimeout,
                bound,
                upperBound,
                ifirSolver_.getActivityMap(),
                ifirSolver_.getResources(),
                oracle
        );        
        timer_.stop();
        if (exhSolver_.getBestMakespan() < Integer.MAX_VALUE)
            lastSolver_ = exhSolver_;
        else
            ifirSolver_.restoreBestSolution();
    }

    public Collection<PSToken> getActivities() { return lastSolver_.getActivities(); }
    public int getMakespan()                   { return lastSolver_.getMakespan(); }
    public int getBestMakespan()               { return lastSolver_.getBestMakespan(); }
    public long getElapsedMsecs()              { return timer_.getElapsed(); }
    public long getTimeToBest()                { return (lastSolver_ == ifirSolver_ ? lastSolver_.getTimeToBest() : ifirSolver_.getElapsedMsecs()+lastSolver_.getTimeToBest()); }

    public String getSolutionAsString()        { return lastSolver_.getName() + " " + lastSolver_.getSolutionAsString(); }

    public void undoSolve() { lastSolver_.undoSolve(); }
}

