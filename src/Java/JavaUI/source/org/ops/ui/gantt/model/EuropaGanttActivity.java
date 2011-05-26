package org.ops.ui.gantt.model;

import psengine.PSToken;

/**
 * Implementation of IGanttActivity around Europa PSToken.
 */
public class EuropaGanttActivity implements IGanttActivity {
	protected PSToken token;
	protected int startMin, startMax;

	public EuropaGanttActivity(PSToken token, int horStart, int horEnd) {
		this.token = token;

		// If the token overlaps startHorizon, pretend it starts at time 0
		// (otherwise, it won't get shown in our gantt table)
		startMin = (int) Math.floor(token.getStart().getLowerBound());
		startMax = (int) Math.ceil(token.getStart().getUpperBound());
		if (startMin < horStart && horStart < getEndMax()) {
			startMin = horStart;
		}
		if (startMax < horStart && horStart < getEndMax()) {
			startMax = horStart;
		}
	}

	// TBS: Is it inappropriate to return this? We want access to more details
	// in DetailsView
	@Override
	public PSToken getData() {
		return token;
	}

	public boolean hasViolation() {
		return token.getViolation() != 0;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttActivity#getStartMin()
	 */
	@Override
	public int getStartMin() {
		return startMin;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttActivity#getStartMax()
	 */
	@Override
	public int getStartMax() {
		return startMax;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttActivity#getEndMin()
	 */
	@Override
	public int getEndMin() {
		return (int) Math.floor(token.getEnd().getLowerBound());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttActivity#getEndMax()
	 */
	@Override
	public int getEndMax() {
		return (int) Math.ceil(token.getEnd().getUpperBound());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttActivity#getDurMin()
	 */
	@Override
	public int getDurMin() {
		return (int) Math.floor(token.getDuration().getLowerBound());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttActivity#getDurMax()
	 */
	@Override
	public int getDurMax() {
		return (int) Math.floor(token.getDuration().getUpperBound());
	}

	public int getKey() {
		return token.getEntityKey();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttActivity#getText()
	 */
	@Override
	public String getText() {
		return token.getEntityName();
	}
}
