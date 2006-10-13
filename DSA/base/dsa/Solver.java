package dsa;

public interface Solver 
{
	public  boolean solve();

	public  void step();

	public  void reset();

	public  int getStepCount();

	public  int getDepth();

	public  int getOpenDecisionCnt();	
	
	public  boolean isExhausted();

	public  boolean isTimedOut();
	
	public  boolean hasFlaws();	
}