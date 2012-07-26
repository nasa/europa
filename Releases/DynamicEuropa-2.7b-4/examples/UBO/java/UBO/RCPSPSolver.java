package UBO;

import java.util.Collection;

import psengine.PSEngine;
import psengine.PSToken;

public interface RCPSPSolver 
{
    public String getName();
    
    public void solve(PSEngine psengine, 
                      long timeout, // in msecs
                      int bound, 
                      boolean usePSResources);

    public long getElapsedMsecs();

    public int getBestMakespan();

    public long getTimeToBest();

    public void undoSolve();

    public Collection<PSToken> getActivities();

    public int getMakespan();

    public String getSolutionAsString();

}