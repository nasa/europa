
import java.util.*;
import java.io.*;
import java.lang.reflect.*;
import javax.swing.*;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.ItemListener;
import java.awt.event.ItemEvent;


public class NQueensUI
{
	protected static JDesktopPane desktop_;
	protected static JComboBox optList_;
	protected static OptimizerArgValues optArgValues_;
	protected static Map optArgTextValues_;
	protected static JPanel optLauncherPanel_; 
	protected static JPanel optArgsPanel_; 
	protected static int windowCnt_=0;
	
    public static void run()
    {
    	List od=loadOptimizerDescriptors();
	    SwingUtilities.invokeLater(new UICreator(od));
    }   
    
    static List loadOptimizerDescriptors()
    {
        List l = new Vector();

        l.add(new OptimizerDescriptor(
        		"TSOptimizer",
				new OptimizerArg[] {
        				new OptimizerArg("DataFile","Data File","data/Problem-16-81.txt"),						
        				new OptimizerArg("Replications","Replications","1"),						
        				new OptimizerArg("MoveTabuTenure","Keep performed moves in tabu list for this number of iterations","20"),						
        				new OptimizerArg("SolutionTabuTenure","Keep visited solutions in tabu list for this number of iterations","50"),						
        				new OptimizerArg("MaxFailures","Max Consecutive Iterations without improvement before quitting","3500"),
        		}
        	)
		);
        
        return l;
    }
    
    static JInternalFrame makeOptFrame(
    		String name,
    		OptimizerCarSeqDisplay optDialog)
    {
        JInternalFrame frame = new JInternalFrame(name);
        frame.getContentPane().setLayout(new BorderLayout());
        frame.getContentPane().add(optDialog.getUIPanel(),BorderLayout.CENTER);
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
    
    public static Optimizer makeTSOptimizer(OptimizerArgValues args)
    {
    	return new CarSequencingSolverTS(
    		"TabuSearchTSP",	
			args.getInt("MoveTabuTenure"), 
			args.getInt("SolutionTabuTenure"), 
			args.getInt("MaxFailures") // number of iterations without improvement before quitting
    	);
    }
        
    static void runOptimizer(OptimizerDescriptor optDesc,
    		                 OptimizerArgValues args)
    {
    	try {
    		Class c = Class.forName("NQueensUI");
    		Class argTypes[]=new Class[]{OptimizerArgValues.class};
    		Method m = c.getMethod("make"+optDesc.getName(),argTypes);
    		final Optimizer opt = (Optimizer)m.invoke(null,new Object[]{args});

            final int replications = args.getInt("Replications");
    		final String dataFile = (String)args.get("DataFile");
    		final DescCarSequencingProblem problem = CarSequencing.readProblem(dataFile);

    		// hook in UI
    		OptimizerCarSeqDisplay optDialog = new OptimizerCarSeqDisplay(problem);  
    	    opt.addOptimizerListener(optDialog);
            makeOptFrame(optDesc.getName()+" - "+dataFile,optDialog);            
            
            Thread handler = new Thread() {
            	public void run() {
            		for (int i=0;i<replications;i++) {
            			CarSequence sequence = CarSequencing.problemToSequence(problem);
            			opt.solve(sequence);  
            			Log.debug("Finished replication "+(i+1)+" out of "+replications);
            		}
            	}
            };
            handler.start();
    	}
    	catch (Exception e) {
    		throw new RuntimeException(e);
    	}
    }

    static void fakeSleep(long msecs)
        throws Exception
    {
        String s = new String("");
        synchronized (s) {
            s.wait(msecs);
        }    	
    }
    
    private static class UICreator
        implements Runnable
    {
    	List optDescriptors_;
    	
	    public UICreator(List od) 
	    { 
	    	optDescriptors_=od;
	    }
	    
	    public void run() 
	    {	
	    	createAndShowGUI(optDescriptors_); 
	    }    	
    }

    static private void createAndShowGUI(List optDescriptors) 
    {
    	try {
    		//UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
    		//UIManager.setLookAndFeel("javax.swing.plaf.metal.MetalLookAndFeel");
    		JFrame.setDefaultLookAndFeelDecorated(true);
    		
    		//Create and set up the window.
    		JFrame frame = new JFrame("NQueens Optimizers");
    		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    		frame.getContentPane().setLayout(new BorderLayout());    		
    		desktop_ = new JDesktopPane();
    		frame.getContentPane().add(makeOptLauncherPanel(optDescriptors),BorderLayout.NORTH);
    		frame.getContentPane().add(desktop_,BorderLayout.CENTER);

    		//Display the window.
    		frame.pack();
    		frame.setSize(700,700);
    		frame.setVisible(true);
    	}
    	catch (Exception e) {
    		throw new RuntimeException(e);
    	}
    }    
    
    static JPanel makeOptLauncherPanel(List optDescriptors)
    {
    	JPanel panel = new JPanel(new FlowLayout());
    	
    	optList_ = new JComboBox(optDescriptors.toArray());
    	optList_.addItemListener(new ItemListener() {
    		public void itemStateChanged(ItemEvent e)  
    		{
    			if (e.getStateChange()==ItemEvent.SELECTED)
    		        resetArgsPanel();	
    		} 
    	});
    	

    	panel.add(new JScrollPane(optList_));
    	JButton optBtn = new JButton("Run Optimizer");
    	optBtn.addActionListener(new ActionListener() {
    		public void actionPerformed(ActionEvent e) 
    		{
    			optArgValues_ = new OptimizerArgValues();
    	        OptimizerDescriptor od = (OptimizerDescriptor)optList_.getSelectedItem();
    	        OptimizerArg args[]=od.getArgs();
    	        for (int i=0;i<args.length;i++) {
    	        	JTextField tf = (JTextField)optArgTextValues_.get(args[i].name);
    	        	optArgValues_.put(args[i].name,tf.getText());
    	        }
    			
    			runOptimizer(
    					(OptimizerDescriptor)optList_.getSelectedItem(),
						optArgValues_);
    		} 
    	});
    	
    	panel.add(optBtn);
    	
    	optLauncherPanel_ = new JPanel(new BorderLayout());
    	optLauncherPanel_.add(BorderLayout.NORTH,panel);
    	optArgsPanel_=new JPanel(new BorderLayout());
    	optLauncherPanel_.add(BorderLayout.CENTER,optArgsPanel_);
    	
    	resetArgsPanel();
    	
    	return optLauncherPanel_;
    }
    
    static void resetArgsPanel()
    {
    	optArgTextValues_ = new HashMap();
        OptimizerDescriptor od = (OptimizerDescriptor)optList_.getSelectedItem();
        OptimizerArg args[]=od.getArgs();
        JPanel p = new JPanel(new GridLayout(args.length,2));
        for (int i=0;i<args.length;i++) {
        	p.add(new JLabel(args[i].description));
        	JTextField tf =new JTextField(args[i].defaultValue); 
        	p.add(tf);
        	optArgTextValues_.put(args[i].name,tf);
        }
        optArgsPanel_.removeAll();
        optArgsPanel_.add(BorderLayout.CENTER,p);
        optLauncherPanel_. revalidate();
    }
    
    static int getInt(String s)
    {
    	return (new Integer(s)).intValue();
    }

    static Integer getTokenAsInteger(StringTokenizer tok)
    {
	    return new Integer(tok.nextToken());
    }	
}
