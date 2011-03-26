package org.ops.ui.solver.model;

import java.io.File;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

import psengine.PSEngine;
import psengine.PSSolver;
import psengine.StringErrorStream;

/**
 * Solver model corresponds to a single engine, or "run". Each launch of an NDDL
 * model creates a separate solver model. 
 * 
 * @author tatiana
 */
public class SolverModel {
	/** Property name in the engine config: list of search paths for includes */
	private final String INCLUDE_PATH = "nddl.includePath";
	
	protected Logger log = Logger.getLogger(getClass().getName());
	private PSEngine engine = null;
	private PSSolver solver = null;	
	private ArrayList<SolverListener> listeners = new ArrayList<SolverListener>();

	private ArrayList<StepStatisticsRecord> stepStatistics = new ArrayList<StepStatisticsRecord>();

	/** Loaded model files. In theory may be more than one */
	// private ArrayList<File> loadedFiles = new ArrayList<File>();
	private File modelFile = null;
	/** Horizon bounds */
	private int horizonStart, horizonEnd;
	/** Planner configuration file */
	private File plannerConfig = null;
	
	public void configure(File modelFile, File plannerConfigFile,
			int horizonStart, int horizonEnd) {
		this.modelFile = modelFile;
		this.plannerConfig = plannerConfigFile;
		this.horizonStart = horizonStart;
		this.horizonEnd = horizonEnd;
	}

	public PSEngine getEngine() {
		return engine;
	}

	private void loadNddlFile(File file) {
		String oldPath = this.engine.getConfig().getProperty(INCLUDE_PATH);
		try {
			String newPath = file.getParent();
			if (oldPath != null)
				newPath = newPath + ":" + oldPath;
			this.engine.getConfig().setProperty(INCLUDE_PATH, newPath);
			// Call plain nddl, not AST, so that it loads
			this.engine.executeScript("nddl", file.getAbsolutePath(), true);
			// loadedFiles.add(file);
		} catch (Exception e) {
			System.err.println("Cannot load NDDL file? " + e);
		} finally {
			this.engine.getConfig().setProperty(INCLUDE_PATH, oldPath);
		}
	}
	
	public File getModelFile() {
		return this.modelFile;
	}
	
	public File getPlannerConfig() {
		return this.plannerConfig;
	}

	/** @return two numbers: horizon start and horizon end */
	public int[] getHorizon() {
		int[] res = new int[2];
		// In case model is build from BSH file, not NDDL
		if (solver == null)
			return res;
		res[0] = solver.getHorizonStart();
		res[1] = solver.getHorizonEnd();
		return res;
	}

	public void setHorizon(int horizonStart, int horizonEnd) {
		this.solver.configure(horizonStart, horizonEnd);
	}

	public int getStepCount() {
		return solver.getStepCount();
	}

	public boolean canStep() {
		return solver.hasFlaws() && !solver.isExhausted()
				&& !solver.isTimedOut();
	}

	public void addSolverListener(SolverListener lnr) {
		if (!listeners.contains(lnr))
			listeners.add(lnr);
	}

	public void removeSolverListener(SolverListener lnr) {
		listeners.remove(lnr);
	}

	/**
	 * Make one step (assuming we can)
	 * 
	 * @return time in ms
	 */
	public long stepOnce() {
		assert (canStep());
		for (SolverListener lnr : listeners)
			lnr.beforeStepping();
		long time = System.currentTimeMillis();
		solver.step();
		time -= System.currentTimeMillis();
		StepStatisticsRecord rec = new StepStatisticsRecord(solver, time);
		assert (rec.getStep() == solver.getStepCount());
		this.stepStatistics.add(rec);
		for (SolverListener lnr : listeners)
			lnr.afterStepping();
		return time;
	}

	/** Run for up to N steps. @return the actual number of steps performed */
	public long stepN(int times, boolean notifyEachStep) {
		for (SolverListener lnr : listeners)
			lnr.beforeStepping();
		int actual = 0;
		while (canStep() && actual < times) {
			long time = System.currentTimeMillis();
			solver.step();
			time = System.currentTimeMillis() - time;
			StepStatisticsRecord rec = new StepStatisticsRecord(solver, time);
			assert (rec.getStep() == solver.getStepCount());
			this.stepStatistics.add(rec);
			actual++;
			if (notifyEachStep)
				for (SolverListener lnr : listeners)
					lnr.afterOneStep(time);
		}
		for (SolverListener lnr : listeners)
			lnr.afterStepping();
		return actual;
	}

	public StepStatisticsRecord getStepStatistics(int step) {
		if (step >= this.stepStatistics.size())
			return StepStatisticsRecord.getEmpty();
		return this.stepStatistics.get(step);
	}

	/** @return true if the engine is not up and running. */
	public boolean isTerminated() {
		return solver == null;
	}
	
	/** Start and configure engine. Assume it is not already running */
	public synchronized void start() {
		if (engine != null)
			throw new IllegalStateException("Restarting solver model without shutdown");

		StringErrorStream.setErrorStreamToString();
		this.engine = PSEngine.makeInstance();
		this.engine.start();

		// NDDL files should be loaded before solver is created
		loadNddlFile(this.modelFile);

		this.solver = engine.createSolver(this.plannerConfig.getAbsolutePath());
		log.log(Level.INFO, "Solver created with {0}", this.plannerConfig);
		this.setHorizon(horizonStart, horizonEnd);

		// Initialize records
		stepStatistics.clear();
		stepStatistics.add(new StepStatisticsRecord(solver, 0));
		for (SolverListener lnr : listeners)
			lnr.solverStarted();
	}

	/** Shutdown the engine */
	public synchronized void terminate() {
		if (engine == null)
			throw new IllegalStateException(
					"Cannot shutdown - nothing initialized");
		if (solver != null)
			solver.delete();
		engine.shutdown();
		engine.delete();
		solver = null;
		engine = null;
		for (SolverListener lnr : listeners)
			lnr.solverStopped();
	}

	/** Get contents of the engine output stream and clean the stream up */
	public String retrieveEngineOutput() {
		return StringErrorStream.retrieveString();
	}
}
