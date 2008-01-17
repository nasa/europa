package UBO;

import java.util.Collection;
import java.util.List;
import java.util.Vector;

import org.ops.ui.util.SimpleTimer;

import psengine.*;

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
    
    public void solve(PSEngine psengine,
            long timeout, // in msecs
            int bound, 
            boolean usePSResources)
    {
        timer_ = new SimpleTimer();
        timer_.start();
        
        long ifirTimeout = (long)(timeout*0.1);
        long exhTimeout = timeout-ifirTimeout;
        
        int ifirBound = bound; // TODO: use 10% higher?
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
        ifirSolver_.undoSolve(); // TODO: best solution needs to be reapplid if we don't find better
        
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
    public String getSolutionAsString()        { return lastSolver_.getSolutionAsString(); }
    public long getTimeToBest()                { return lastSolver_.getTimeToBest(); }

    public void undoSolve() 
    {
        // TODO Auto-generated method stub        
    }
}

