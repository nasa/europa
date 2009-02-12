package org.ops.ui;

import java.awt.BorderLayout;
import java.util.Calendar;
import java.util.List;
import java.util.Vector;
import java.util.Map;
import java.util.HashMap;


import javax.swing.JInternalFrame;
import javax.swing.JTabbedPane;
import javax.swing.SwingUtilities;
import javax.swing.JDesktopPane;
import javax.swing.JFrame;
import javax.swing.JTable;
import javax.swing.JScrollPane;

import bsh.Interpreter;
import bsh.util.JConsole;

import org.ops.ui.chart.PSJFreeResourceChart;
import org.ops.ui.chart.PSResourceChart;
import org.ops.ui.chart.PSResourceChartPSEModel;
import org.ops.ui.gantt.PSEGantt;
import org.ops.ui.gantt.PSGantt;
import org.ops.ui.gantt.PSGanttPSEModel;
import org.ops.ui.solver.PSSolverDialog;
import org.ops.ui.util.Util;
import org.ops.ui.mouse.ActionViolationsPanel;
import org.ops.ui.mouse.ActionDetailsPanel;
import org.ops.ui.nddl.NddlAshInterpreter;
import org.ops.ui.nddl.NddlTokenMarker;
import org.ops.ui.anml.AnmlInterpreter;
import org.ops.ui.anml.AnmlTokenMarker;
import org.ops.ui.ash.AshConsole;

import org.josql.contrib.JoSQLSwingTableModel;

import psengine.*;

public class PSDesktop
{
	public static PSDesktop instance_=null;

	protected JDesktopPane desktop_;
	protected int windowCnt_=0;
	protected PSEngine psEngine_=null;
    protected NddlAshInterpreter nddlInterpreter_;
	protected JConsole bshConsole_;
    protected Interpreter bshInterpreter_;
    
	protected static String debugMode_="g";
	protected static String bshFile_=null;

	public static void main(String[] args)
	{		
	    String debugMode = args[0];
        PSUtil.loadLibraries(debugMode);	   

	    PSEngine engine = PSEngine.makeInstance();
	    engine.start();
		Runtime.getRuntime().addShutdownHook(new ShutdownHook());
		
		PSDesktop d = PSDesktop.makeInstance(engine,args);
		d.runUI();
	}
	
    static class ShutdownHook extends Thread 
    {
	    public ShutdownHook()
	    {
	        super("ShutdownHook");
	    }
	    
	    public void run() 
	    {
	        PSDesktop.getInstance().getPSEngine().shutdown();
	    }
    }	  

    public String getLibsMode() { return debugMode_; }
	
    public static PSDesktop getInstance()
    {
    	if (instance_ == null)
    		instance_ = makeInstance(PSEngine.makeInstance(),new String[]{"g",null});
    	
    	return instance_;
    }
    
	public static Map<String,String> parseArgs(String args[])
	{
		Map<String,String> retval = new HashMap<String,String>();
		String debugMode = "g";
		String bshFile = null; 
		
		if (args.length > 0)
 		    debugMode = args[0];
		if ((args.length > 1) && (args[1].length()>0))
			bshFile = args[1];

		retval.put("debugMode", debugMode);
		retval.put("bshFile",bshFile);
		
		return retval;
	}
	
	public static PSDesktop makeInstance(PSEngine pse,String args[])
	{
		if (instance_ != null) 
			throw new RuntimeException("PSDesktop is a singleton");

		init(args);
        instance_ = new PSDesktop(pse);
        
        return instance_;
 	}

   public static void init(String[] args)
    {   
        init(parseArgs(args));    
    }
    
	public static void init(Map<String,String> args)
	{
		debugMode_ = args.get("debugMode");
		bshFile_ = args.get("bshFile");		
	}
	
	protected PSDesktop(PSEngine pse)
	{
		assert (pse != null);
	    psEngine_ = pse;
        nddlInterpreter_ = new NddlAshInterpreter(psEngine_);
        bshConsole_ = new JConsole();
        bshInterpreter_ = new Interpreter(bshConsole_);                	    
	}
		
    public void runUI()
    {
	    SwingUtilities.invokeLater(new UICreator());
    }

    public JInternalFrame makeNewFrame(String name)
    {
        JInternalFrame frame = new JInternalFrame(name);
        frame.getContentPane().setLayout(new BorderLayout());
	    desktop_.add(frame);
	    int offset=windowCnt_*15;
	    windowCnt_++;
	    frame.setLocation(offset,offset);
	    frame.setSize(700,300);
	    frame.setResizable(true);
	    frame.setClosable(true);
	    frame.setMaximizable(true);
	    frame.setIconifiable(true);
        frame.setVisible(true);
        return frame;
    }

    private class UICreator
        implements Runnable
    {
	    public void run()
	    {
	    	createAndShowGUI();
	    }
    }

    private void createAndShowGUI()
    {
    	try {
    		//UIManager.setLookAndFeel("javax.swing.plaf.metal.MetalLookAndFeel");
    		JFrame.setDefaultLookAndFeelDecorated(true);

    		//Create and set up the window.
    		JFrame frame = new JFrame("Planning & Scheduling UI");
    		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    		frame.getContentPane().setLayout(new BorderLayout());
    		createDesktop();
    		frame.getContentPane().add(desktop_,BorderLayout.CENTER);

    		//Display the window.
    		frame.pack();
    		frame.setSize(1200,600);
    		frame.setVisible(true);
    	}
    	catch (Exception e) {
    		e.printStackTrace();
    		System.exit(0);
    	}
    }

    private void createDesktop()
    {
    	desktop_ = new JDesktopPane();

        // BeanShell scripting
        JInternalFrame consoleFrame = makeNewFrame("Console");
        consoleFrame.getContentPane().add(bshConsole_);
        new Thread(bshInterpreter_).start();

        registerBshVariables();

        if (bshFile_ != null) {
            try {
        	    bshInterpreter_.eval("source(\""+bshFile_+"\");");
            }
            catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
        //consoleFrame.setIcon(true);
    }

    public void addBshVariable(String name,Object obj)
    {
        try {
            bshInterpreter_.set(name,obj);
        }
        catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
    
    protected void registerBshVariables()
    {
        addBshVariable("desktop",this);
        addBshVariable("psengine",getPSEngine());
        addBshVariable("nddlInterp", nddlInterpreter_);
    }

    public void makeTableFrame(String title,List l,String fields[])
    {
    	JInternalFrame frame = this.makeNewFrame(title);
    	JTable table = Util.makeTable(l,fields);
    	JScrollPane scrollpane = new JScrollPane(table);
    	frame.getContentPane().add(scrollpane);
		frame.setSize(frame.getSize()); // Force repaint
    }

    /*
     * Creates a table on the results of a JoSQL query
     */
    public void makeTableFrame(String title,List l,String josqlQry)
    {
    	try {
    		JInternalFrame frame = this.makeNewFrame(title);

    		// TODO: JoSQLSwingTableModel doesn't preserve column names, it hsould be easy to add
    		JoSQLSwingTableModel model =  new JoSQLSwingTableModel();
    		model.parse(josqlQry);
    		model.execute(l);

    		/*
    		Query qry = new Query();
    		qry.parse(josqlQry);
    		List data = qry.execute(l).getResults();
    		TableModel model = Util.makeTableModel(data, new String[]{"toString"});
    		*/

    		JTable table = new JTable(model);
    		JScrollPane scrollpane = new JScrollPane(table);
    		frame.getContentPane().add(scrollpane);
    		frame.setSize(frame.getSize()); // Force repaint
    	}
    	catch (Exception e) {
    		throw new RuntimeException(e);
    	}
    }
    
    public PSEngine getPSEngine()
    {
    	return psEngine_;
    }

    public PSSolverDialog makeSolverDialog(PSSolver solver)
    {
    	try {
    		JInternalFrame frame = makeNewFrame("Solver");
    		frame.getContentPane().setLayout(new BorderLayout());
    		PSSolverDialog d = new PSSolverDialog(this,solver);
    		frame.getContentPane().add(new JScrollPane(d));
    		frame.setSize(675,375);
    		
    		return d;
    	}
    	catch (Exception e)
    	{
    		throw new RuntimeException(e);
    	}
    }

    public void showTokens(PSObject o)
    {
         PSTokenList l = o.getTokens();
         showTokens("Activities for "+o.getEntityName(),l);
    }

    public void showTokens(String title,PSTokenList l)
    {
        List columnNames = new Vector();
        List data = new Vector();
        columnNames.add("Key");
        columnNames.add("Name");
        columnNames.add("Type");

        for (int i=0; i<l.size();i++) {
       	 List row = new Vector();
       	 PSToken t = l.get(i);
       	 row.add(t.getKey());
       	 row.add(t.getEntityName());
       	 row.add(t.getEntityType());
       	 PSVariableList vars = t.getParameters();
       	 for (int j=0; j<vars.size();j++) {
       		 PSVariable var = vars.get(j);
       		 row.add(var.toString());

       		 // Only add cols for the first row
       		 if (i==0)
       			 columnNames.add(var.getEntityName());
       	 }
       	 data.add(row);
        }

    	JInternalFrame frame = makeNewFrame(title);
   	    JTable table = new JTable(new Util.MatrixTableModel(data,columnNames));
   	    JScrollPane scrollpane = new JScrollPane(table);
   	    frame.getContentPane().add(scrollpane);    	
		frame.setSize(frame.getSize()); // Force repaint
    }
    
    public JInternalFrame makeResourceGanttFrame(
            String objectsType,
            Calendar start,
            Calendar end)
    {
        return makeResourceGanttFrame(objectsType,start,end,Calendar.MINUTE);
    }
    
    public JInternalFrame makeResourceGanttFrame(
    		String objectsType,
	        Calendar start,
	        Calendar end,
	        int timeUnit)
    {
        PSGantt gantt = new PSEGantt(new PSGanttPSEModel(getPSEngine(),start,objectsType,timeUnit),start,end);

        JInternalFrame frame = makeNewFrame("Resource Schedule");
        frame.getContentPane().add(gantt);
		frame.setSize(frame.getSize()); // Force repaint

        return frame;
    }

    public PSResourceChart makeResourceChart(PSResource r,Calendar start)
    {
    	return new PSJFreeResourceChart(
    			r.getEntityName(),
    			new PSResourceChartPSEModel(r),
    			start);
    }

    public JInternalFrame makeResourcesFrame(String type,Calendar start)
    {
        JTabbedPane resourceTabs = new JTabbedPane();
        List<PSResource> resources = PSUtil.toResourceList(getPSEngine().getObjectsByType(type));
        for (PSResource r : resources) 
            resourceTabs.add(r.getEntityName(),makeResourceChart(r,start));

        JInternalFrame frame = makeNewFrame("Resources");
        frame.getContentPane().add(resourceTabs);
		frame.setSize(frame.getSize()); // Force repaint

        return frame;
    }

    public JInternalFrame makeViolationsFrame()
    {
        ActionViolationsPanel vp = new ActionViolationsPanel(getPSEngine());
        JInternalFrame frame = makeNewFrame("Violations");
        frame.getContentPane().add(vp);
        frame.setLocation(500,180);
        frame.setSize(300,300);

        return frame;
    }

    public JInternalFrame makeDetailsFrame()
    {
        ActionDetailsPanel dp = new ActionDetailsPanel(getPSEngine());
        JInternalFrame frame = makeNewFrame("Details");
        frame.getContentPane().add(dp);
        frame.setLocation(800,180);
        frame.setSize(300,200);

        return frame;
    }    
    
    public PSSolver makeSolver(String config,int horizonStart,int horizonEnd)
    {
    	PSSolver solver = getPSEngine().createSolver(config);
    	solver.configure(horizonStart,horizonEnd);
    	
    	return solver;
    }
    
    public void makeNddlConsole()
    {
    	JInternalFrame nddlInterpFrame = makeNewFrame("Nddl Console");
    	AshConsole console = new AshConsole(nddlInterpreter_);
    	nddlInterpreter_.setConsole(console);
    	console.setTokenMarker(new NddlTokenMarker());
    	nddlInterpFrame.setContentPane(console);    
    }

    public void makeAnmlConsole()
    {
      AnmlInterpreter anmlInterpreter = new AnmlInterpreter();
      JInternalFrame anmlInterpFrame = makeNewFrame("Anml Console");
      AshConsole console = new AshConsole(anmlInterpreter);
      anmlInterpreter.setConsole(console);
      console.setTokenMarker(new AnmlTokenMarker());
      anmlInterpFrame.setContentPane(console);
    }    
}

