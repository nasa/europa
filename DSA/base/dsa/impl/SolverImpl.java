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

    public boolean isExhausted(){ return m_isExhausted;}

    public boolean isTimedOut(){ return m_isTimedOut;}

    private boolean hasFlaws(){ return m_hasFlaws;}

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
    		m_isExhausted = (state.getAttribute("isExhausted", 0) == 0 ? false : true);
    		m_isTimedOut = (state.getAttribute("isTimedOut", 0) == 0 ? false : true);
    		m_hasFlaws = (state.getAttribute("hasFlaws", 0) == 0 ? false : true);
    	}
    	catch(Exception e){
    		e.printStackTrace();
    	}

    }

    private int m_maxSteps;
    private int m_maxDepth;
    private int m_stepCount;
    private int m_depth;
    private boolean m_isExhausted;
    private boolean m_isTimedOut;
    private boolean m_hasFlaws;
}
