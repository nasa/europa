package org.ops.ui.gantt.model;

import psengine.PSToken;

/**
 * Structure for holding information about an activity in Gantt chart. Copied
 * from the original PSUI code
 */
public class GanttActivity {
	protected PSToken token;
	protected int startMin, startMax, endMin, endMax, durMin, durMax;

	public GanttActivity(PSToken token, int horStart, int horEnd) {
		this.token = token;
		
		// If the token overlaps startHorizon, pretend it starts at time 0
		// (otherwise, it won't get shown in our gantt table)
		startMin = (int) Math.floor(token.getStart().getLowerBound());
		startMax = (int) Math.ceil(token.getStart().getUpperBound());
		endMin = (int) Math.floor(token.getEnd().getLowerBound());
		endMax = (int) Math.ceil(token.getEnd().getUpperBound());
		if (startMin < horStart && horStart < endMax) {
			startMin = horStart;
		}
		if (startMax < horStart && horStart < endMax) {
			startMax = horStart;
		}
		// Should we truncate to the upper bound as well?
		durMin = (int) Math.floor(token.getDuration().getLowerBound());
		durMax = (int) Math.floor(token.getDuration().getUpperBound());
	}

	// TBS: Is it inappropriate to return this?  We want access to more details in DetailsView
	public PSToken getToken() { return token; }
	
	public boolean hasViolation() {
		return token.getViolation() != 0;
	}

	public int getStartMin() {
		return startMin;
	}

	public int getStartMax() {
		return startMax;
	}

	public int getEndMin() {
		return endMin;
	}

	public int getEndMax() {
		return endMax;
	}

	public int getDurMin() {
		return durMin;
	}

	public int getDurMax() {
		return durMax;
	}

	public int getKey() {
		return token.getEntityKey();
	}

	public String getText() {
		return token.getEntityName();
	}
}
