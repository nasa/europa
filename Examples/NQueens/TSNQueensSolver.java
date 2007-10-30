// Tabu Search NQueens Solver

import java.util.SortedSet;
import java.util.List;
import java.util.Vector;
import java.util.TreeSet;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import psengine.*;

class TSNQueensSolver
{
	protected PSEngine psengine_;
	protected Map tabuList_;
	protected int tabuTenure_=10;
	protected int curIteration_=0;
	protected int queenCnt_;
	protected List observers_;
	
	public interface SolverObserver
	{
		public void iterationCompleted(int iteration);
		public void newSolutionFound(int iteration,List queenPositions,List queenViolations);
	}
	
    public TSNQueensSolver(int n,PSEngine engine)
    {
        psengine_ = engine;	
        tabuList_ = new HashMap();
        queenCnt_ = n;
        observers_ = new Vector();
    }

    public void addObserver(SolverObserver o) { observers_.add(o); }
    public void removeObserver(SolverObserver o) { observers_.remove(o); }
 
    void notifyIterationCompleted(int iteration)
    {
        for (int i=0;i<observers_.size();i++) {
        	((SolverObserver)observers_.get(i)).iterationCompleted(iteration);
        }
    }
    
    void notifyNewSolutionFound(int iteration)
    {
    	List queenValues = new Vector();
    	List queenViolations = new Vector();
    	
    	PSVariableList l = psengine_.getGlobalVariables();	
    	
    	for (int i=0;i<queenCnt_;i++) {
    		PSVariable v = l.get(i);
    		Integer value = v.getSingletonValue().asInt();
    		queenValues.add(value);
    		queenViolations.add(v.getViolationExpl());
    	}

        for (int i=0;i<observers_.size();i++) {
        	((SolverObserver)observers_.get(i)).newSolutionFound(iteration,queenValues,queenViolations);
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
    
    SortedSet getMoves(PSVariable queen,int curPos)
    {
    	SortedSet moves = new TreeSet();
    	
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
    	
    	int bestIter = curIteration_;
        double bestCost = psengine_.getViolation();
        
    	for (int i=0;psengine_.getViolation() > 0 && i < maxIter;i++) {
    	    PSVariable queenToMove = getQueenWithMaxViolation();
    		int curPos = queenToMove.getSingletonValue().asInt();
    	    SortedSet moves = getMoves(queenToMove,curPos);
    	    
    	    boolean moved = false;
            Iterator it = moves.iterator();
            while (it.hasNext() && !moved) {
            	moved = makeMove(queenToMove,curPos,(Move)it.next(),false);
            }
            
            if (!moved) {
            	dbgout("Forced move!");
            	makeMove(queenToMove,curPos,(Move)moves.first(),true);
                // TODO: restart?
            }
            
            double cost = psengine_.getViolation();
            if (cost < bestCost) {
            	bestCost = cost;
            	bestIter = curIteration_;
            	notifyNewSolutionFound(curIteration_);
            }
            
	        dbgout(i+":"+queensToString());            
            curIteration_++;           
            notifyIterationCompleted(curIteration_);
            
            if (curIteration_-bestIter > 50) {
            	restart();
            	bestIter = curIteration_;
            }
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

