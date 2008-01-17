package UBO;

import java.util.Collection;

import psengine.PSEngine;
import psengine.PSToken;

public interface RCPSPSolver {

    // TODO: pass in problem definition to be able to compute critical path
    public void solve(PSEngine psengine, long timeout, // in msecs
            int bound, boolean usePSResources);

    public long getElapsedMsecs();

    public int getBestMakespan();

    public long getTimeToBest();

    public void undoSolve();

    public Collection<PSToken> getActivities();

    public int getMakespan();

    public String getSolutionAsString();

}