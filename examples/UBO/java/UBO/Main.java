package UBO;

import psengine.PSUtil;
import psengine.util.LibraryLoader;
import psengine.PSEngine;
import org.ops.ui.PSDesktop;
import psengine.util.SimpleTimer;
import psengine.*;


import java.io.BufferedWriter;
import java.io.FileWriter;


class Main
{
	protected static PSEngine psEngine_;
	   
	public static void main(String args[])
	{
		if (args.length < 3)
		{
		    String debugMode = args[0];
	        PSUtil.loadLibraries(debugMode);	   

		    psEngine_ = PSEngine.makeInstance();
		    psEngine_.start();
			Runtime.getRuntime().addShutdownHook(new ShutdownHook());
			loadCustomCode(debugMode);
			
			PSDesktop d = PSDesktop.makeInstance(psEngine_,args);
			d.runUI();
		}
	    else
	    {
	        runBatchTest(args);
	    }
	}
	
    protected static void loadCustomCode(String debugMode)
    {
    	//Load module with any custom code if it exists:
    	String libName = "UBO_" + debugMode;
    	String fullLibName = LibraryLoader.getResolvedName(libName); 
    	if(fullLibName == null) {
    		// Run 'make' to compile the library if you need it:
    		System.out.println("INFO: Custom library " + libName + " wasn't found and won't be loaded.");  
    	}
    	else {
    		// WARNING: Shared library loaded twice (see ticket #164)
    		System.load(fullLibName);
    		psEngine_.loadModule(fullLibName);
    	}  	
    }
    
    
    static class ShutdownHook extends Thread 
    {
	    public ShutdownHook()
	    {
	        super("ShutdownHook");
	    }
	    
	    public void run() 
	    {
           	psEngine_.shutdown();
	    }
    }	  
	
    
	public static void runBatchTest(String args[])
	{
	    try {
	        String debugMode = args[0];
	        String test = args[2];
	        Integer bound = new Integer(args[3]);
	        long timeoutMsecs = (new Integer(args[4])).intValue() * 1000; 
	        String solver = args[5];

	        LibraryLoader.loadLibrary("System_"+debugMode);        
	        PSEngine engine = PSEngine.makeInstance();
	        engine.start();
	        String nddlModel = "UBO-gen-initial-state.nddl";
	        engine.executeScript("nddl",nddlModel,true/*isFile*/);

	        if ("BuiltIn".equals(solver))
	            runBuiltInSolver(engine,test,bound,timeoutMsecs);
	        else if ("IFIR".equals(solver))
	            runRCPSPSolver(engine,new IFlatIRelaxSolver(),test,bound,timeoutMsecs);
            else if ("Hybrid".equals(solver))
                runRCPSPSolver(engine,new HybridSolver(),test,bound,timeoutMsecs);
	        else
	            throw new RuntimeException("Unknown solver:"+solver);

	        //engine.shutdown();  TODO: this is causing problems      
	    }
	    catch (Exception e) {
            throw new RuntimeException(e);	        
	    }
	}	
	
	public static void runBuiltInSolver(PSEngine engine,String test,Integer bound, long timeoutMsecs)
	{
        PSVariable v = engine.getVariableByName("maxDuration");
        v.specifyValue(PSVarValue.getInstance(bound));

        PSSolver solver = engine.createSolver("PlannerConfig.xml"/*config*/);
	    solver.configure(0/*horizonStart*/,1000/*horizonEnd*/);	   
	    
	    SimpleTimer timer = new SimpleTimer();
	    timer.start();
	    boolean timedOut = false;
	    while (solver.hasFlaws() && !solver.isExhausted()) {
	        solver.step();
	        if (timer.getElapsed() > timeoutMsecs) {
	            timedOut = true;
	            break;
	        }
	    }
        timer.stop();
        
        // Save results
        // test-name bound best-makespan time-in-msecs solution stepCount
        int makespan = ((timedOut || solver.isExhausted()) ? 0 : bound); // TODO: this could be lower, ground solution and find out
        StringBuffer buf = new StringBuffer();
        String separator="    ";
        buf.append(test).append(separator)
           .append(bound).append(separator)
           .append(makespan).append(separator)
           .append(timer.getElapsed()).append(separator)
           //.append(s.getSolutionAsString()) // TODO: extract solution
           .append(solver.getStepCount()).append(separator)
           .append("\n");
        
        writeToFile("Solver-BuiltIn-"+timeoutMsecs+".txt",buf.toString());  
	}

    public static void runRCPSPSolver(PSEngine engine,RCPSPSolver s, String test,Integer bound, long timeoutMsecs)
    {     
        // TODO: since this is randomized, run several times an get avg
        boolean usePSResources = false; // TODO: eventually switch to true
        s.solve(engine,timeoutMsecs,bound,usePSResources);
        //RCPSPUtil.ground(s.getActivities());
        
        // Save results
        // test-name bound best-makespan time-in-msecs solution
        int makespan = (s.getBestMakespan() != Integer.MAX_VALUE ? s.getBestMakespan() : 0);
        StringBuffer buf = new StringBuffer();
        String separator="    ";
        buf.append(test).append(separator)
           .append(bound).append(separator)
           .append(makespan).append(separator)
           .append(s.getElapsedMsecs()).append(separator)
           .append(s.getTimeToBest()).append(separator)
           .append(s.getSolutionAsString())
           .append("\n");
        
        String filename = "Solver-"+s.getName()+"-"+(s.getActivities().size()-2)+".txt";
        writeToFile(filename,buf.toString());
    }	
    
    protected static void writeToFile(String filename,String str)
    {
        try {
            BufferedWriter out = new BufferedWriter(new FileWriter(filename,true/*append*/));
            out.write(str);
            out.close();
        } 
        catch (Exception e) {
            throw new RuntimeException(e);
        }                
    }
}
