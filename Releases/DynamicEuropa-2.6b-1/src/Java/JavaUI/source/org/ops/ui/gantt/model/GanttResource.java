package org.ops.ui.gantt.model;

import psengine.PSResource;
import psengine.PSResourceProfile;
import psengine.PSTimePointList;

public class GanttResource {
	protected PSResource resource;
	protected double tmin, tmax, amin, amax;

	public GanttResource(PSResource resource) {
		this.resource = resource;

		//System.out.println("Limits " + resource.getLimits().getTimes().size()
		//		+ ", actuals " + resource.getLevels().getTimes().size());
		// Limits
		PSResourceProfile ls = resource.getLimits();
		PSTimePointList ts = ls.getTimes();
		tmin = ls.getLowerBound(ts.get(0));
		tmax = ls.getUpperBound(ts.get(0));
		for (int i = 1; i < ts.size(); i++) {
			int t = ts.get(i);
			tmin = Math.min(tmin, ls.getLowerBound(t));
			tmax = Math.max(tmax, ls.getUpperBound(t));
			// System.out.println(" l " + t + " " + ls.getLowerBound(t) + " " + ls.getUpperBound(t));
		}
		// Actuals
		ls = resource.getLevels();
		ts = ls.getTimes();
		amin = tmax;
		amax = tmin;
		for (int i = 0; i < ts.size(); i++) {
			int t = ts.get(i);
			amin = Math.min(amin, ls.getLowerBound(t));
			amax = Math.max(amax, ls.getUpperBound(t));
			// System.out.println(" a " + t + " " + ls.getLowerBound(t) + " " + ls.getUpperBound(t));
		}
	}

	@Override
	public String toString() {
		return resource.getEntityName() + "[" + tmin + ", " + tmax + "] ["
				+ amin + ", " + amax + "]";
	}
	
	public String getName() {
		return resource.getEntityName();
	}
	
	public double getActualMin() {
		return amin;
	}
	
	public double getActualMax() {
		return amax;
	}

	public double getLow(int time) {
		return resource.getLevels().getLowerBound(time);
	}

	public double getHigh(int time) {
		return resource.getLevels().getUpperBound(time);
	}
}
