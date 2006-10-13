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

	public  int getOpenDecisionCnt();	
	
	public  boolean isExhausted();

	public  boolean isTimedOut();
	
	public  boolean isConstraintConsistent();

	public  boolean hasFlaws();	
}