package org.ops.ui.solver.model;

import java.io.File;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

import psengine.PSEngine;
import psengine.PSSolver;

public class SolverModel {
	/** Property name in the engine config: list of search paths for includes */
	private final String INCLUDE_PATH = "nddl.includePath";

	private Logger log = Logger.getLogger(getClass().getName());
	private PSEngine engine = null;
	private PSSolver solver = null;
	private ArrayList<SolverListener> listeners = new ArrayList<SolverListener>();

	private ArrayList<StepStatisticsRecord> stepStatistics = new ArrayList<StepStatisticsRecord>();
	
	private ArrayList<File> loadedFiles = new ArrayList<File>();

	public synchronized void configure(File file, File configFile,
			int horizonStart, int horizonEnd) {
		if (engine != null)
			throw new IllegalStateException("Reinitializing solver model");

		this.engine = PSEngine.makeInstance();
		this.engine.start();

		
		// NDDL files should be loaded before solver is created
		loadNddlFile(file);

		this.solver = engine.createSolver(configFile.getAbsolutePath());
		log.log(Level.INFO, "Solver created with {0}", configFile);
		this.setHorizon(horizonStart, horizonEnd);

		// Initialize records
		stepStatistics.clear();
		stepStatistics.add(new StepStatisticsRecord(solver, 0));
		for (SolverListener lnr : listeners)
			lnr.solverStarted();
	}

	public synchronized void shutdown() {
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

	/** @return true if the engine is up and running */
	public boolean isConfigured() {
		return solver != null;
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
			loadedFiles.add(file);
		} catch (Exception e) {
			System.err.println("Cannot load NDDL file? " + e);
		} finally {
			this.engine.getConfig().setProperty(INCLUDE_PATH, oldPath);
		}
	}
	
	/** Get all NDDL files loaded in this engine, includes are not followed */
	public ArrayList<File> getLoadedFiles() {
		return loadedFiles;
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
}
