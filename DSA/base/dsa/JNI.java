package dsa;

import java.io.*;
import java.util.*;

public class JNI {
    public static native void load(String model);
    public static native void loadTransactions(String txSource);
    public static native void getComponents();
    public static native void getViolations(int actionKey);

    /** Solver API Calls **/
    public static native void solverConfigure(String configFile, int horizonStart, int horizonEnd);
    public static native void solverSolve(int maxSteps, int maxDepth);
    public static native void solverStep();
    public static native void solverReset();
    public static native void solverClear();
}
