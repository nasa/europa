
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.List;
import java.util.Vector;

import javax.swing.*;

import psengine.*;

class NQueensPanel 
     extends MouseAdapter
     implements TSNQueensSolver.SolverObserver
{
	protected NQueensBoard solverBoard_;
	protected TSNQueensSolver solver_;
	protected JPanel uiPanel_;
	protected JButton solveBtn_;
	protected JTabbedPane tabbedPane_;
	protected JTextField maxIterations_;
	protected JLabel iterationCount_;
    protected List solutions_;
    protected int curSolution_;
    protected JButton nextBtn_,prevBtn_;
	
	public NQueensPanel(PSEngine psengine,
			            int queenCnt, 
			            JTextArea mouseInfo)
	{
		createUIPanel(psengine,queenCnt,mouseInfo);
		solver_ = new TSNQueensSolver(queenCnt,psengine);
		solver_.addObserver(this);
		solutions_ = new Vector();
		curSolution_ = -1;
	}
	
	public JPanel getUIPanel() { return uiPanel_; }
	
	public void createUIPanel(PSEngine psengine,int queenCnt, JTextArea mouseInfo)
	{
		uiPanel_ = new JPanel(new BorderLayout());

		JPanel solverPanel = new JPanel(new BorderLayout());
		solveBtn_ = new JButton("Solve");
		solveBtn_.addMouseListener(this);
		maxIterations_ = new JTextField("100",5);
		JPanel topPanel = new JPanel(new FlowLayout());
		topPanel.add(new JLabel("Max Iterations:"));
		topPanel.add(maxIterations_);
		topPanel.add(solveBtn_);
		iterationCount_ = new JLabel("Iteration : 0");
		topPanel.add(iterationCount_);

		solverBoard_ = new NQueensBoard(psengine,queenCnt,mouseInfo,true);

		JPanel bottomPanel = new JPanel(new FlowLayout());
		nextBtn_ = new JButton(">>");
		nextBtn_.addMouseListener(this);
		nextBtn_.setEnabled(false);
		prevBtn_ = new JButton("<<");
		prevBtn_.addMouseListener(this);
		prevBtn_.setEnabled(false);
		bottomPanel.add(prevBtn_);
		bottomPanel.add(nextBtn_);
		
		solverPanel.add(BorderLayout.NORTH,topPanel);
		solverPanel.add(BorderLayout.CENTER,solverBoard_);
		solverPanel.add(BorderLayout.SOUTH,bottomPanel);
		
		tabbedPane_ = new JTabbedPane();
		tabbedPane_.addTab("Current State",new NQueensBoard(psengine,queenCnt,mouseInfo,false));
		tabbedPane_.addTab("Solver",solverPanel);

		uiPanel_.add(tabbedPane_);	
	}
	
   	public void mouseClicked(MouseEvent e)
    {
   		try {
   			if (e.getSource()==solveBtn_) {   				
   				SolverRunner handler = new SolverRunner();
   				handler.start();        	
   			}
   	        if (e.getSource()==nextBtn_) {
   	            setCurrentSolution(curSolution_+1);
   	       	}
   	        if (e.getSource()==prevBtn_) {
   	            setCurrentSolution(curSolution_-1);
   	        }
   		}
   		catch (Exception ex) {
   			throw new RuntimeException(ex);
   		}
    }	
   	
   	class SolverRunner
   	    extends Thread
   	{
		public void run() 
		{
			solveBtn_.setEnabled(false);
			tabbedPane_.setEnabledAt(0,false);
			int maxIter = Integer.parseInt(maxIterations_.getText());
	    	solver_.solve(maxIter);
			tabbedPane_.setEnabledAt(0,true);
			solveBtn_.setEnabled(true);
		}   	    
   	}

	public void iterationCompleted(int iteration) 
	{
	    SwingUtilities.invokeLater(new Updater1(iteration));
	}

	public void newSolutionFound(int iteration, List queenPositions, List queenViolations) 
	{
		solutions_.add(new Object[]{new Integer(iteration),queenPositions,queenViolations});
	    SwingUtilities.invokeLater(new Updater2());
	}

	protected void setCurrentSolution(int i)
	{
    	if (i >= solutions_.size() || i < 0)
    		return;
    	
    	curSolution_=i;

    	if (curSolution_==solutions_.size()-1)
    		nextBtn_.setEnabled(false);
    	else
    		nextBtn_.setEnabled(true);

    	if (curSolution_==0)
    		prevBtn_.setEnabled(false);
    	else
    		prevBtn_.setEnabled(true);

    	solverBoard_.setSolution((Object[])solutions_.get(curSolution_));
	}
	
	class Updater1 implements Runnable
	{
		int iteration_;
		
		public Updater1(int i) { iteration_ = i; }
        
		public void run() {
            iterationCount_.setText("Iteration : "+iteration_);		
        }	
	}
	
	class Updater2 implements Runnable
	{
        public void run() {
    	    setCurrentSolution(solutions_.size()-1);
        }	
	}
}
