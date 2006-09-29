package dsa;

import dsa.impl.SolverImpl;


public class SolverManager 
{
    public static Solver createInstance(String configurationFile, int horizonStart, int horizonEnd, int maxSteps, int maxDepth)
    {
    	return SolverImpl.createInstance(configurationFile, horizonStart, horizonEnd, maxSteps, maxDepth);
    }

    public static Solver instance() throws Exception 
    {
    	return SolverImpl.instance();
    }
}
