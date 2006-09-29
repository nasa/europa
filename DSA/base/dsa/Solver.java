package dsa;

public interface Solver 
{
	public abstract boolean solve();

	public abstract void step();

	public abstract void reset();

	public abstract int getStepCount();

	public abstract int getDepth();

	public abstract boolean isExhausted();

	public abstract boolean isTimedOut();
}