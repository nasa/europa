package org.ops.ui.gantt.model;

import psengine.PSResource;
import psengine.PSResourceProfile;
import psengine.PSTimePointList;

/**
 * Implementation of IGanttResource around Europa PSResource
 * 
 * @author tatiana
 */
public class EuropaGanttResource implements IGanttResource {
	protected PSResource resource;
	protected double tmin, tmax, amin, amax;

	public EuropaGanttResource(PSResource resource) {
		this.resource = resource;

		// System.out.println("Limits " + resource.getLimits().getTimes().size()
		// + ", actuals " + resource.getLevels().getTimes().size());
		// Limits
		PSResourceProfile ls = resource.getLimits();
		PSTimePointList ts = ls.getTimes();
		tmin = ls.getLowerBound(ts.get(0));
		tmax = ls.getUpperBound(ts.get(0));
		for (int i = 1; i < ts.size(); i++) {
			int t = ts.get(i);
			tmin = Math.min(tmin, ls.getLowerBound(t));
			tmax = Math.max(tmax, ls.getUpperBound(t));
			// System.out.println(" l " + t + " " + ls.getLowerBound(t) + " " +
			// ls.getUpperBound(t));
		}
		// Actuals
		ls = resource.getFDLevels();
		ts = ls.getTimes();
		amin = tmax;
		amax = tmin;
		for (int i = 0; i < ts.size(); i++) {
			int t = ts.get(i);
			amin = Math.min(amin, ls.getLowerBound(t));
			amax = Math.max(amax, ls.getUpperBound(t));
			// System.out.println(" a " + t + " " + ls.getLowerBound(t) + " " +
			// ls.getUpperBound(t));
		}
	}

	@Override
	public String toString() {
		return resource.getEntityName() + "[" + tmin + ", " + tmax + "] ["
				+ amin + ", " + amax + "]";
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttResource#getName()
	 */
	@Override
	public String getName() {
		return resource.getEntityName();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttResource#getActualMin()
	 */
	@Override
	public double getActualMin() {
		return amin;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttResource#getActualMax()
	 */
	@Override
	public double getActualMax() {
		return amax;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttResource#getLow(int)
	 */
	@Override
	public double getLow(int time) {
		return resource.getFDLevels().getLowerBound(time);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ops.ui.gantt.model.IGanttResource#getHigh(int)
	 */
	@Override
	public double getHigh(int time) {
		return resource.getFDLevels().getUpperBound(time);
	}
}
