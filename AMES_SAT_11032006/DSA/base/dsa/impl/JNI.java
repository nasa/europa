package dsa.impl;


public class JNI 
{
    public static native void load(String model);
    public static native void addPlan(String txSource);
    public static native String getComponents();
    public static native String getActions(int componentKey);
    public static native String getAction(int actionKey);
    public static native String getConditions(int actionKey);
    public static native String getEffects(int actionKey);
    public static native String getChildActions(int actionKey);
    public static native String getViolations(int actionKey);
    public static native String getMaster(int actionKey);
    public static native String getComponentForAction(int actionKey);
    public static native String getResources();
    public static native String getResourceCapacityProfile(int resourceKey);
    public static native String getResourceUsageProfile(int resourceKey);

    /** Solver API Calls **/
    public static native String solverConfigure(String configFile, int horizonStart, int horizonEnd);
    public static native String solverSolve(int maxSteps, int maxDepth);
    public static native String solverStep();
    public static native String solverReset();
    public static native String solverClear();
    public static native String solverGetOpenDecisions();

    /** Call-back handlers **/
    public static void handleCallBack(){
	System.out.println("Called from C++");
    }
}
