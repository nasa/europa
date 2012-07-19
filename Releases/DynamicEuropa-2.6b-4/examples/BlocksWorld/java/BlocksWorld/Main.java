package BlocksWorld;

import psengine.PSUtil;
import psengine.util.LibraryLoader;
import psengine.PSEngine;
import org.ops.ui.PSDesktop;
import bsh.Interpreter;

class Main 
{
    protected static PSEngine psEngine_;
    
    public static void main(String args[]) 
    {
	    String debugMode = args[0];
        PSUtil.loadLibraries(debugMode);	   

	psEngine_ = PSEngine.makeInstance();
	psEngine_.start();
	Runtime.getRuntime().addShutdownHook(new ShutdownHook());
	loadCustomCode(debugMode);
		
	if(args.length > 2 && args[2].equals("nogui"))
		{
			Interpreter bshInterpreter_ = new bsh.Interpreter();
			try {
			bshInterpreter_.set("psengine", psEngine_);
		        bshInterpreter_.eval("source(\""+args[1]+"\");");
			}
			catch (Exception e) {
			     throw new RuntimeException(e);
			}            		
		}
		else
		{
			PSDesktop d = PSDesktop.makeInstance(psEngine_,args);
			d.runUI();
		}
    }

    protected static void loadCustomCode(String debugMode)
    {
    	//Load module with any custom code if it exists:
    	String libName = "BlocksWorld_" + debugMode;
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
}
