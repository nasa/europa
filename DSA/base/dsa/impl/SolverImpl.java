package dsa.impl;

import dsa.Solver;
import net.n3.nanoxml.IXMLElement;

public class SolverImpl implements Solver 
{
    private static SolverImpl s_instance = null;

    public static Solver createInstance(String configurationFile, int horizonStart, int horizonEnd, int maxSteps, int maxDepth)
    {
    	if(s_instance == null)
    	    s_instance = new SolverImpl(maxSteps, maxDepth);

    	s_instance.updateState(JNI.solverConfigure(configurationFile, horizonStart, horizonEnd));

    	return s_instance;
    }

    public static Solver instance() throws Exception 
    {
    	if(s_instance == null)
    	    throw new Exception("No solver allocated.");

    	return s_instance;
    }

    private SolverImpl(int maxSteps, int maxDepth)
    {
	    m_maxSteps = maxSteps;
	    m_maxDepth = maxDepth;
    }

    public boolean solve()
    {
	    updateState(JNI.solverSolve(m_maxSteps, m_maxDepth));
	    return !hasFlaws();
    }

    public void step() 
    {
	    updateState(JNI.solverStep());
    }

    public void reset()
    {
	    updateState(JNI.solverReset());
    }

    public int getStepCount(){ return m_stepCount; }

    public int getDepth() {return m_depth;}
    
    public int getOpenDecisionCnt() { return m_openDecisionCount; }

    public boolean isExhausted(){ return m_isExhausted;}

    public boolean isTimedOut(){ return m_isTimedOut;}

    public boolean hasFlaws(){ return m_hasFlaws;}

    private void updateState(String xmlStr)
    {
    	m_stepCount = 0;
    	m_depth = 0;
    	m_isExhausted = false;
    	m_isTimedOut = false;
    	m_hasFlaws = true;

    	// Read the state file and parse the data
    	try{
    		IXMLElement state = Util.toXML(xmlStr);
    		m_stepCount = state.getAttribute("stepCount", 0);
    		m_depth = state.getAttribute("depth", 0);
    		m_openDecisionCount = state.getAttribute("openDecisionCount", 0);
    		m_isExhausted = (state.getAttribute("isExhausted", 0) == 0 ? false : true);
    		m_isTimedOut = (state.getAttribute("isTimedOut", 0) == 0 ? false : true);
    		m_hasFlaws = (state.getAttribute("hasFlaws", 0) == 0 ? false : true);
    	}
    	catch(Exception e){
    		e.printStackTrace();
    	}

    }

    protected int m_maxSteps;
    protected int m_maxDepth;
    protected int m_stepCount;
    protected int m_depth;
    protected int m_openDecisionCount;
    protected boolean m_isExhausted;
    protected boolean m_isTimedOut;
    protected boolean m_hasFlaws;
}
