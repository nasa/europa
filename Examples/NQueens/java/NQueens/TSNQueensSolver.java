package NQueens;

// Tabu Search NQueens Solver

import java.util.SortedSet;
import java.util.List;
import java.util.Vector;
import java.util.TreeSet;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import psengine.*;

public class TSNQueensSolver
{
	protected PSEngine psengine_;
	protected Map<String,Integer> tabuList_;
	protected int tabuTenure_=10;
	protected int curIteration_=0;
	protected int queenCnt_;
	protected List<SolverObserver> observers_;
    protected int bestIter_;
    protected double bestCost_;
	
	public interface SolverObserver
	{
		public void iterationCompleted(int iteration);
		public void newSolutionFound(int iteration,List<Integer> queenPositions,List<String> queenViolations);
	}
	
    public TSNQueensSolver(int n,PSEngine engine)
    {
        psengine_ = engine;	
        tabuList_ = new HashMap<String,Integer>();
        queenCnt_ = n;
        observers_ = new Vector<SolverObserver>();
    }

    public void addObserver(SolverObserver o) { observers_.add(o); }
    public void removeObserver(SolverObserver o) { observers_.remove(o); }
 
    void notifyIterationCompleted(int iteration)
    {
        dbgout(iteration+":"+queensToString());                    
        for (int i=0;i<observers_.size();i++) {
        	observers_.get(i).iterationCompleted(iteration);
        }
    }
    
    void notifyNewSolutionFound(int iteration)
    {
    	List<Integer> queenValues = new Vector<Integer>();
    	List<String> queenViolations = new Vector<String>();
    	
    	PSVariableList l = psengine_.getGlobalVariables();	
    	
    	for (int i=0;i<queenCnt_;i++) {
    		PSVariable v = l.get(i);
    		Integer value = v.getSingletonValue().asInt();
    		queenValues.add(value);
    		queenViolations.add(v.getViolationExpl());
    	}

        for (int i=0;i<observers_.size();i++) {
        	observers_.get(i).newSolutionFound(iteration,queenValues,queenViolations);
        }    	
    }
    
    void init()
    {
        tabuList_.clear();
    	PSVariableList l = psengine_.getGlobalVariables();	
    	
    	for (int i=0;i<queenCnt_;i++) {
        	PSVarValue value = PSVarValue.getInstance((int)(Math.random()*(queenCnt_-1)));
    		PSVariable v = l.get(i);
    		v.specifyValue(value);
    	} 
    	
        bestIter_ = curIteration_;
        bestCost_ = psengine_.getViolation();            
    }
    
    void dbgout(String msg)
    {
        //print(msg);
        System.out.println(msg);
    }
    
    void restart()
    {
    	init();
    	dbgout("Restarted!");
    }
    
    PSVariable getQueenWithMaxViolation()
    {
    	double maxViolation = 0;
    	int maxVar=0;
    	
    	PSVariableList l = psengine_.getGlobalVariables();	
    	
    	for (int i=0;i<queenCnt_;i++) {
    		PSVariable v = l.get(i);
    		if (v.getViolation() > maxViolation) {
    			maxVar = i;
    			maxViolation = v.getViolation();
    		}
    	}    			
    	
    	return l.get(maxVar);
    }

    static class Move
        implements Comparable
    {
    	public int slot_;
    	public double violation_;
    	
        public Move(int slot,double violation)
        {
        	slot_ = slot;
        	violation_ = violation;
        }
    	public int compareTo(Object obj) 
    	{
    		Move rhs = (Move)obj;

    		if (violation_ < rhs.violation_)
    			return -1;
    		else if (violation_ > rhs.violation_)
    			return 1;
    		else 
    			return 0;
    	}        
    }
    
    SortedSet<Move> getMoves(PSVariable queen,int curPos)
    {
    	SortedSet<Move> moves = new TreeSet<Move>();
    	
        for (int i=0;i<queenCnt_;i++) {
        	if (i != curPos) {
        		PSVarValue value = PSVarValue.getInstance(i);
        		queen.specifyValue(value);
        		double v = psengine_.getViolation();
        		moves.add(new Move(i,v));
        	}
        }
        
        return moves;
    }

    public void solve(int maxIter)
    {
    	init();
    	
    	for (int i=0;psengine_.getViolation() > 0 && i < maxIter;i++) {
    	    PSVariable queenToMove = getQueenWithMaxViolation();
    		int curPos = queenToMove.getSingletonValue().asInt();
    	    
    	    boolean moved = false;
            SortedSet<Move> moves = getMoves(queenToMove,curPos);             
            for (Move m : moves) {
            	moved = makeMove(queenToMove,curPos,m,false);
            	if (moved)
            	    break;
            }
            
            if (!moved) 
            	makeMove(queenToMove,curPos,moves.first(),true);
            
            checkSolution(); // See if we have a new best solution            
            notifyIterationCompleted(curIteration_++);
            
            if (curIteration_-bestIter_ > 50) 
            	restart();
    	}
    }
    
    protected void checkSolution()
    {
        double cost = psengine_.getViolation();
        if (cost < bestCost_) {
            bestCost_ = cost;
            bestIter_ = curIteration_;
            notifyNewSolutionFound(curIteration_);
        }        
    }
    
    void addToTabuList(PSVariable queenToMove,int orig,int dest)
    {
        String key = queenToMove.getName() + "_" +orig+"_"+dest;
        tabuList_.put(key,curIteration_+tabuTenure_);        
    }
    
    boolean isTabu(PSVariable queenToMove,int orig,int dest)
    {
        String key = queenToMove.getName() + "_" +orig+"_"+dest;
        Integer iteration = (Integer)tabuList_.get(key);
        if (iteration == null)
        	return false;
        
        return (iteration.intValue() > curIteration_);
    }
    
    boolean makeMove(PSVariable queenToMove,int curPos,Move m,boolean force)
    {
        if (force)
            dbgout("Forced move!");

    	if (force || !isTabu(queenToMove,curPos,m.slot_)) {
		    PSVarValue value = PSVarValue.getInstance(m.slot_);
		    queenToMove.specifyValue(value);
		    dbgout("Moved queen "+queenToMove.getName()+" from "+curPos+" to "+m.slot_); 
	        addToTabuList(queenToMove,curPos,m.slot_);
	        return true;
    	}
    	
    	return false;
    }
    
    String queensToString()
    {
        StringBuffer buf = new StringBuffer();
        
        buf.append("{");
        for (int i=0; i<queenCnt_;i++) {
        	if (i>0)
        		buf.append(",");
        	buf.append(psengine_.getGlobalVariables().get(i).toString());
        }
        buf.append("}");
        buf.append(" violation:").append(psengine_.getViolation());
        
        return buf.toString();
    }    
}

