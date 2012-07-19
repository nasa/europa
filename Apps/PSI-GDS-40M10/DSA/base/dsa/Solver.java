package dsa;

import java.util.List;

public interface Solver 
{
	public  boolean solve();

	public  void step();

	public  void reset();

	public  int getStepCount();

	public  int getDepth();
	
    public List<String> getOpenDecisions();	

	public  String getLastExecutedDecision();	
	
	public  boolean isExhausted();

	public  boolean isTimedOut();
	
	public  boolean isConstraintConsistent();

	public  boolean hasFlaws();	
	
    // Configuration
    public Integer getHorizonStart();
    public Integer getHorizonEnd();
    public Integer getMaxSteps();
    public Integer getMaxDepth();
    public String getConfigFilename();	
    
    // Listeners
    public void addListener(SolverListener l);
    public void removeListener(SolverListener l);
}