package org.ops.ui.solver;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.text.NumberFormat;
import java.util.List;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.JInternalFrame;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextField;
import javax.swing.JButton;
import javax.swing.SwingUtilities;
import javax.swing.JSplitPane;
import javax.swing.border.Border;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import org.jfree.data.xy.XYDataset;
import org.jfree.ui.RectangleInsets;
import org.jfree.chart.plot.PlotOrientation;

import org.ops.ui.PSDesktop;

import psengine.PSEngine;
import psengine.PSSolver;
import psengine.PSStringList;

public class PSSolverDialog 
    extends JPanel
    implements ActionListener
{
	private static final long serialVersionUID = -7130640151223933584L;
	
	protected JTextField horizonStart_ = new JTextField("0");
	protected JTextField horizonEnd_  = new JTextField("5760");
	protected JTextField maxSteps_ = new JTextField("2000");
	protected JTextField maxDepth_ = new JTextField("100000");
	protected JTextField configFile_ = new JTextField("../Crew1/PlannerConfig.xml");
	protected JTextField incSteps_ = new JTextField("500",8);
	
	protected JButton btnConfigure_= new JButton("Configure");
	protected JButton btnStep_= new JButton("Go");
	protected JButton btnSolve_ = new JButton("Solve");
	protected long startTime_ = 0;
	protected JLabel lblRunTime_ = new JLabel(format(startTime_));
	protected JLabel lblStepCnt_ = new JLabel("0");
	protected JLabel lblMaxStepCnt_ = new JLabel("0");
	
	protected XYSeries solverDepthSeries_;
	protected XYSeries stepTimeSeries_;
	protected XYSeries stepAvgTimeSeries_;
	protected double totalTime_;
	protected XYSeries decisionCntSeries_;
	protected JPanel chartsPanel_;
	
	protected OpenDecisionsPanel openDecisions_;
	
	protected JSplitPane topSplitPane_;
	
	protected PSDesktop desktop_;
	protected PSSolver solver_;
	protected Integer maxStepsValue_=100;
	protected Integer maxDepthValue_=100;
	
	protected List<PSSolverDialogListener> listeners_ = new Vector<PSSolverDialogListener>();
	
    public PSSolverDialog(PSDesktop desktop,PSSolver solver)
    {
    	desktop_ = desktop;
        solver_ = solver;
        
        if (solver != null) {
    	    horizonStart_ = new JTextField(new Integer(solver_.getHorizonStart()).toString(),15);
    	    horizonEnd_  = new JTextField(new Integer(solver_.getHorizonEnd()).toString(),15);
    	    maxSteps_ = new JTextField(maxStepsValue_.toString(),8);
    	    maxDepth_ = new JTextField(maxDepthValue_.toString(),8);
    	    configFile_ = new JTextField(solver_.getConfigFilename());
    	    incSteps_ = new JTextField(maxStepsValue_.toString(),8);
        }
        
    	totalTime_ = 0;
    	        
    	JPanel solverPanel = new JPanel(new BorderLayout());
    	JTabbedPane tp = new JTabbedPane();
    	tp.add("Run",makeRunPanel());
    	tp.add("Configure",makeConfigPanel());
    	solverPanel.add(tp);
    	
    	openDecisions_ = new OpenDecisionsPanel(); 

    	setLayout(new BorderLayout());	    
        add(solverPanel);

        JInternalFrame frame = desktop_.makeNewFrame("Solver Open Decisions");
        frame.getContentPane().add(openDecisions_);
        frame.setSize(400,400);
        frame.setLocation(700,20);
    }
    
    public void addListener(PSSolverDialogListener l) { listeners_.add(l); }
    public void removeListener(PSSolverDialogListener l) { listeners_.remove(l); }
    
    protected JPanel makeConfigPanel()
    {
    	JPanel p = new JPanel(new FlowLayout());

    	JPanel configPanel = new JPanel();
        configPanel.setLayout(new GridLayout(5,2));
        configPanel.add(new JLabel("Horizon Start"));configPanel.add(horizonStart_);
        configPanel.add(new JLabel("Horizon End"));configPanel.add(horizonEnd_);
        configPanel.add(new JLabel("Max Steps"));configPanel.add(maxSteps_);
        configPanel.add(new JLabel("Max Depth"));configPanel.add(maxDepth_);
        configPanel.add(new JLabel("Config File"));configPanel.add(configFile_);

        btnConfigure_.addActionListener(this);btnConfigure_.setActionCommand("configure");

        JPanel buttonPanel = new JPanel(new FlowLayout());        
        buttonPanel.add(btnConfigure_);        
         
    	JPanel p1 = new JPanel(new BorderLayout());
        p1.add(BorderLayout.CENTER,configPanel);
        p1.add(BorderLayout.SOUTH,buttonPanel);

        p.add(p1);
        
        return p;
    }
    
    protected JPanel makeRunPanel()
    {
    	JPanel p = new JPanel(new BorderLayout());
    	
    	chartsPanel_ = new JPanel(new GridLayout(1,3));
    	
    	stepTimeSeries_ = new XYSeries("Time (secs) per Step");
    	stepAvgTimeSeries_ = new XYSeries("Avg Time (secs) per Step");
    	chartsPanel_.add(makeChartPanel(
            	"Time (secs) per Step ",  // title
                "Step Number",             // x-axis label
                "Time (secs)",                  // y-axis label
                new XYSeries[]{stepTimeSeries_,stepAvgTimeSeries_},
                false // Create legend
    	));
    	    	
    	decisionCntSeries_ = new XYSeries("Open Decision Cnt");
    	chartsPanel_.add(makeChartPanel(
            	"Open Decision Count",  // title
                "Step Number",          // x-axis label
                "Open Decision Count",  // y-axis label
                new XYSeries[]{decisionCntSeries_},
                false // Create legend
    	));    	

    	solverDepthSeries_ = new XYSeries("Decisions in Plan");
    	chartsPanel_.add(makeChartPanel(
            	"Decisions in Plan",  // title
                "Step Number",        // x-axis label
                "Decisions in Plan",  // y-axis label
                new XYSeries[]{solverDepthSeries_},
                false // Create legend
    	));    

        btnStep_.addActionListener(this);btnStep_.setActionCommand("step");
        btnSolve_.addActionListener(this);btnSolve_.setActionCommand("solve");

        boolean enabled = solver_ != null;
    	btnStep_.setEnabled(enabled);
    	btnSolve_.setEnabled(enabled);
        
        JPanel buttonPanel = new JPanel(new GridLayout(1,4));
        /*
		lblMaxStepCnt_.setText(solver_.getMaxSteps().toString());
        buttonPanel.add(new JLabel("Max Step Count : "));
        buttonPanel.add(lblMaxStepCnt_);
        */
        
        buttonPanel.add(new JLabel("Run for"));
        buttonPanel.add(incSteps_);
        buttonPanel.add(new JLabel("steps"));
        buttonPanel.add(btnStep_);
        //buttonPanel.add(btnSolve_);
        JPanel p1 = new JPanel(new FlowLayout());
        p1.add(buttonPanel);

        JPanel totalsPanel = new JPanel(new GridLayout(1,4));
		lblStepCnt_.setText(Integer.toString(solver_.getStepCount()));
        totalsPanel.add(new JLabel("Step Count : "));
        totalsPanel.add(lblStepCnt_);        
        totalsPanel.add(new JLabel("Run Time :"));
        totalsPanel.add(lblRunTime_);
        JPanel p2 = new JPanel(new FlowLayout());
        p2.add(totalsPanel);
        
        JPanel p3 = new JPanel(new FlowLayout());
        p3.add(chartsPanel_);
    	p.add(BorderLayout.NORTH,p1);
    	p.add(BorderLayout.CENTER,p3);
    	p.add(BorderLayout.SOUTH,p2);
    	return p;
    }

	protected JPanel makeChartPanel(
			String title,
			String xAxisLabel,
			String yAxisLabel,
			XYSeries series[],
			boolean createLegend)
    {
        XYSeriesCollection dataset = new XYSeriesCollection();
        for (XYSeries s : series)
            dataset.addSeries(s);
        
        JFreeChart chart = createChart(title,xAxisLabel,yAxisLabel,dataset,createLegend);

        return makeChartPanel(chart);
    }
    
	protected JPanel makeChartPanel(JFreeChart chart)
    {
    	JPanel p = new JPanel(new BorderLayout());

        ChartPanel chartPanel = new ChartPanel(chart);
        chartPanel.setPreferredSize(new Dimension(200, 150));
        chartPanel.setMouseZoomable(true, false);
        
        Border border = BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(1, 1, 1, 1),
                BorderFactory.createEtchedBorder()
        );
        chartPanel.setBorder(border);
        
        p.add(new JScrollPane(chartPanel));
        return p;
    }
    
    protected JFreeChart createChart(
    		String title,
    		String xAxisLabel,
    		String yAxisLabel,
    		XYDataset dataset,
    		boolean createLegend) 
	{
	    JFreeChart chart = ChartFactory.createXYLineChart(
	        title,
	        xAxisLabel,
	        yAxisLabel,
	        dataset,            
	        PlotOrientation.VERTICAL,
	        createLegend,               // create legend?
	        true,               // generate tooltips?
	        false               // generate URLs?
	    );
	
	    chart.setBackgroundPaint(Color.white);
	
	    XYPlot plot = (XYPlot) chart.getPlot();
	    plot.setBackgroundPaint(Color.lightGray);
	    plot.setDomainGridlinePaint(Color.white);
	    plot.setRangeGridlinePaint(Color.white);
	    plot.setAxisOffset(new RectangleInsets(5.0, 5.0, 5.0, 5.0));
	    plot.setDomainCrosshairVisible(true);
	    plot.setRangeCrosshairVisible(true);
	    
	    XYItemRenderer r = plot.getRenderer();
	    if (r instanceof XYLineAndShapeRenderer) {
	        XYLineAndShapeRenderer renderer = (XYLineAndShapeRenderer) r;
	        renderer.setBaseShapesVisible(true);
	        renderer.setBaseShapesFilled(true);
	    }
	
	    return chart;
	}    
    
	public void configureSolver()
	{
		int horizonStart = getInteger(horizonStart_.getText());
		int horizonEnd = getInteger(horizonEnd_.getText());
		String config = configFile_.getText();
	    solver_ = desktop_.getPSEngine().createSolver(config);
	    solver_.configure(horizonStart,horizonEnd);
	}
	
	protected int getInteger(String text)
	{
		return new Integer(text.trim());
	}
    
    public void stepSolver()
    {
    	try {
			startTime_ = System.currentTimeMillis();
            setButtons(false);
            
    		int maxIter = getInteger(incSteps_.getText());
            int stepCnt = solver_.getStepCount();
    		int max =  stepCnt+maxIter;
    		
    		for (int i=1; solver_.hasFlaws() 
    			 && !solver_.isExhausted()
    			 && !solver_.isTimedOut()
    			 && stepCnt<max;i++) { 

    			long t = System.currentTimeMillis();
    			solver_.step(); 
    			double secs = (System.currentTimeMillis()-t)/1000.0;
                totalTime_ += secs;
    			stepCnt = solver_.getStepCount();
    			solverDepthSeries_.add(stepCnt,solver_.getDepth());
    			stepTimeSeries_.add(stepCnt,secs);
    			stepAvgTimeSeries_.add(stepCnt,totalTime_/stepCnt);
    			
    			if (solver_.isConstraintConsistent()) {
    				// TODO: this is weird, it takes the solver one more iteration to set its internal m_noFlawsFound flag, fix it
    				if (solver_.hasFlaws()) {
    					List<String> openDecisions = new Vector<String>();
    					PSStringList l = solver_.getFlaws();
    					for (int j=0;j<l.size();j++)
    						openDecisions.add(l.get(j).toString());

    					decisionCntSeries_.add(stepCnt,openDecisions.size());
    					openDecisions_.addEntry(stepCnt,openDecisions,solver_.getLastExecutedDecision());
    				}
    			}
    			forceRepaint(chartsPanel_);
    			//System.out.println(stepCnt + "-" + solver.getDepth()+" - "+secs);
    			lblRunTime_.setText(format(System.currentTimeMillis()-startTime_));
    			lblStepCnt_.setText(Integer.toString(solver_.getStepCount()));
    			
    			for (PSSolverDialogListener l : listeners_)
    				l.stepCompleted(solver_);
    		}
    		
            setButtons(true);
    	}
    	catch (Exception e) {
            setButtons(true);
    		e.printStackTrace();
    	}
    }    
    
    protected void forceRepaint(final java.awt.Container container)
    {
	    SwingUtilities.invokeLater(new Runnable() {
            public void run() {
        	    container.repaint();
            }
        });
    }
    
    public void runSolver()
    {
    	try {
            setButtons(false);
   			solver_.solve(maxStepsValue_,maxDepthValue_); 
            setButtons(true);
    	}
    	catch (Exception e) {
    		e.printStackTrace();
    	}
    }    

    protected void setButtons(boolean isEnabled)
    {
    	btnStep_.setEnabled(isEnabled);
    	btnSolve_.setEnabled(isEnabled);
    	btnConfigure_.setEnabled(isEnabled);    	
    }
    
    public void actionPerformed(ActionEvent e) 
    {
        if ("configure".equals(e.getActionCommand())) {
        	configureSolver();
        	btnStep_.setEnabled(true);
        	btnSolve_.setEnabled(true);
        	return;
        } 

        Thread handler;
        
        if ("step".equals(e.getActionCommand())) {
        	handler = new Thread() {
            	public void run() {
                	stepSolver();
            	}
            };
            handler.start();
        	return;
        } 
        
        if ("solve".equals(e.getActionCommand())) {
            handler = new Thread() {
            	public void run() {
                	runSolver();
            	}
            };
            handler.start();
        	return;
        }         
    }  
    
    public static String format (long msecs) {
        if (msecs < 1000)
          return Long.toString(msecs) + " msecs";
        else if (msecs < (60*1000))
          return decimal3.format(msecs / 1000.0) + " secs";
        else
          return decimal3.format(msecs / (60*1000.0)) + " mins";
      }

      private final static NumberFormat decimal3 = NumberFormat.getNumberInstance();
      {
        decimal3.setMaximumFractionDigits(3);
      }    
}
