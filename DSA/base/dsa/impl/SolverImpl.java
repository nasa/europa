package dsa.impl;

import java.util.List;
import java.util.Vector;
import java.util.StringTokenizer;

import dsa.Solver;
import dsa.SolverListener;
import net.n3.nanoxml.IXMLElement;

public class SolverImpl implements Solver 
{
    private static SolverImpl s_instance = null;

    public static Solver createInstance(String configurationFile, int horizonStart, int horizonEnd, int maxSteps, int maxDepth)
    {
    	if(s_instance == null)
    	    s_instance = new SolverImpl(configurationFile, horizonStart, horizonEnd, maxSteps, maxDepth);

    	s_instance.updateState(JNI.solverConfigure(configurationFile, horizonStart, horizonEnd));

    	return s_instance;
    }

    public static Solver instance() throws Exception 
    {
    	if(s_instance == null)
    	    throw new Exception("No solver allocated.");

    	return s_instance;
    }

    private SolverImpl(String configurationFile, int horizonStart, int horizonEnd, int maxSteps, int maxDepth)
    {
    	m_configFilename = configurationFile;
    	m_horizonStart = horizonStart;
    	m_horizonEnd = horizonEnd;
	    m_maxSteps = maxSteps;
	    m_maxDepth = maxDepth;
	    
	    m_listeners = new Vector<SolverListener>();
    }

    public boolean solve()
    {
	    updateState(JNI.solverSolve(m_maxSteps, m_maxDepth));
	    return !hasFlaws();
    }

    public void step() 
    {
	    updateState(JNI.solverStep());
	    for (SolverListener l : m_listeners)
	    	l.stepCompleted(this);
    }

    public void reset()
    {
	    updateState(JNI.solverReset());
    }

    public int getStepCount(){ return m_stepCount; }

    public int getDepth() {return m_depth;}
    
    public String getLastExecutedDecision() { return m_lastExecutedDecision; }

    public boolean isExhausted(){ return m_isExhausted;}

    public boolean isTimedOut(){ return m_isTimedOut;}

	public  boolean isConstraintConsistent() { return m_isConstraintConsistent; }
    
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
    		m_lastExecutedDecision = state.getAttribute("lastExecutedDecision", "");
    		m_isExhausted = (state.getAttribute("isExhausted", 0) == 0 ? false : true);
    		m_isTimedOut = (state.getAttribute("isTimedOut", 0) == 0 ? false : true);
    		m_isConstraintConsistent = (state.getAttribute("isConstraintConsistent", 0) == 0 ? false : true);
    		m_hasFlaws = (state.getAttribute("hasFlaws", 0) == 0 ? false : true);
    	} 
    	catch(Exception e){
    		e.printStackTrace();
    	}

    }
    
    public List<String> getOpenDecisions()
    {
    	String s = JNI.solverGetOpenDecisions();
    	List<String> retval = new Vector<String>();
    	//System.out.println(s);
    	
    	StringTokenizer tok = new StringTokenizer(s,"\n");
    	while (tok.hasMoreTokens()) {
    		String decision = tok.nextToken();
    		if (!decision.startsWith(" Open")
    				&& !decision.startsWith(" }")) {
    			retval.add(decision);
    		}
    	}
    	
    	return retval;
    }
    
    // Configuration
    public Integer getHorizonStart() { return m_horizonStart; }
    public Integer getHorizonEnd() { return m_horizonEnd;}
    public Integer getMaxSteps() { return m_maxSteps;}
    public Integer getMaxDepth() { return m_maxDepth; }
    public String getConfigFilename() { return m_configFilename; }

    protected int m_horizonStart;
    protected int m_horizonEnd;
    protected int m_maxSteps;
    protected int m_maxDepth;
    protected String m_configFilename;
    
    protected int m_stepCount;
    protected int m_depth;
    protected String m_lastExecutedDecision;
    protected boolean m_isExhausted;
    protected boolean m_isTimedOut;
    protected boolean m_isConstraintConsistent;
    protected boolean m_hasFlaws;
    
    List<SolverListener> m_listeners;

	public void addListener(SolverListener l) 
	{
		m_listeners.add(l);
	}

	public void removeListener(SolverListener l) 
	{
		m_listeners.remove(l);
	}    
}
