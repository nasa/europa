
import org.ops.ui.PSDesktop;
import org.ops.ui.util.LibraryLoader;
import psengine.PSEngine;

class Main
{
	public static void main(String args[])
	{
		PSDesktop.run(args);
	}
	
	/*
	 * PSDesktop instantiates its own EUROPA PSEngine internally, you can get a handle on it by calling
	 * PSDesktop.desktop.getPSEngine()
	 * 
	 * If you don't want to use PSDesktop, but instead create your own instance of EUROPA to manipulate programmatically
	 * you can use this method. For now, only one PSEngine instance can be create per process.
	 * 
	 * debugMode = "g" for debug, "o" for optimized
	 */
	public static void nonPSDesktopMain(String args[])
	{
		String debugMode = args[0];
        LibraryLoader.loadLibrary("System_"+debugMode);
        
	    PSEngine.initialize();
	    
	    PSEngine engine = PSEngine.makeInstance();
        engine.start();
    	// use engine....
        engine.shutdown();
        
	    PSEngine.terminate();
	}	
}
