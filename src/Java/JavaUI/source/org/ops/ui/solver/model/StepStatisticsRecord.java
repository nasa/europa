package org.ops.ui.solver.model;

import java.util.ArrayList;

import psengine.PSSolver;
import psengine.PSStringList;

/**
 * A single record containing various aspects of step statistics
 * 
 * @author Tatiana Kichkaylo
 */
public class StepStatisticsRecord {
	private int stepNum;
	private String decisionMade;
	private ArrayList<String> flaws;
	private long durationMs;
	private int depth;

	private StepStatisticsRecord() {
		flaws = new ArrayList<String>();
	}

	private static final StepStatisticsRecord emptyRecord = new StepStatisticsRecord();

	public static StepStatisticsRecord getEmpty() {
		return emptyRecord;
	}

	/**
	 * Create a record for the current state of the solver.
	 * 
	 * @param solver
	 *            the solver to read data from
	 * @param timeMs
	 *            duration of the last step made, in milliseconds
	 */
	public StepStatisticsRecord(PSSolver solver, long timeMs) {
		stepNum = solver.getStepCount();
		durationMs = timeMs;
		decisionMade = solver.getLastExecutedDecision();
		depth = solver.getDepth();

		flaws = new ArrayList<String>();
		if (solver.hasFlaws()) {
			PSStringList l = solver.getFlaws();
			for (int j = 0; j < l.size(); j++)
				flaws.add(l.get(j));
		}
	}

	@Override
	public String toString() {
		return "StepStat " + stepNum;
	}

	public int getStep() {
		return stepNum;
	}

	public String getDecisionMade() {
		return decisionMade;
	}

	public StringBuffer getDecisionAsHtml(StringBuffer b) {
		if (b == null)
			b = new StringBuffer();
		if (decisionMade == null || "".equals(decisionMade)) {
			b.append("No decision");
			return b;
		}

		// Pull it apart
		String[] parts = decisionMade.split(":");

		// Some better way to format it?
		for (String s : parts)
			b.append(s).append("<br/>\n");

		return b;
	}

	public ArrayList<String> getFlaws() {
		return flaws;
	}

	public long getDurationMs() {
		return durationMs;
	}

	public int getDepth() {
		return depth;
	}
}
