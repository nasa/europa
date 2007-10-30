package Light;

import org.ops.ui.PSDesktop;
import org.ops.ui.util.LibraryLoader;
import psengine.PSEngineWithResources;

class Main
{
	public static void main(String args[])
	{
		PSDesktop.run(args);
	}
	
	/*
	 * PSDesktop instanciates ints own EUROPA PSEngine internally, you can get a handle on it by calling
	 * PSDesktop.desktop.getPSEngine()
	 * 
	 * If you don't want to use PSDesktop, but instead create your own instance of EUROPA to manipulate programmatically
	 * you can use this method. For now, only one PSEngine instance can be create per process.
	 * 
	 * debugMode = "g" for debug, "o" for optimized
	 */
    public PSEngineWithResources makePSEngine(String debugMode)
    {
    	PSEngineWithResources psEngine;
        LibraryLoader.loadLibrary("System_"+debugMode);
        psEngine = new PSEngineWithResources();
        psEngine.start();

    	return psEngine;
    }	
}