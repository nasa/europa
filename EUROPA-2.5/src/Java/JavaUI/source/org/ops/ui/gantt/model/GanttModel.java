package org.ops.ui.gantt.model;

import java.util.ArrayList;
import java.util.List;

import org.ops.ui.solver.model.SolverModel;

import psengine.PSObjectList;
import psengine.PSResource;
import psengine.PSToken;
import psengine.PSTokenList;

/** Copied from the original PSUI and somewhat simplified */
public class GanttModel {
	/** Visible bounds on all activities */
	private int start, end;
	private PSObjectList resources;

	public GanttModel(SolverModel solverModel) {
		this(solverModel, "Object");
	}

	public GanttModel(SolverModel solverModel, String objectsTypes) {
		this.resources = solverModel.getEngine().getObjectsByType(objectsTypes);
		int[] hor = solverModel.getHorizon();

		// Compute start and end of the time line, truncate at horizon
		boolean first = true;
		for (int r = 0; r < resources.size(); r++) {
			PSTokenList tokens = resources.get(r).getTokens();
			for (int i = 0; i < tokens.size(); i++) {
				PSToken token = tokens.get(i);
				int s = (int) (token.getStart().getLowerBound());
				if (s < hor[0])
					s = hor[0];
				int e = (int) (token.getEnd().getUpperBound());
				if (e > hor[1])
					e = hor[1];
				if (first) {
					start = s;
					end = e;
					first = false;
				} else {
					start = Math.min(start, s);
					end = Math.max(end, e);
				}
			}
		}
	}

	public int getResourceCount() {
		return resources.size();
	}

	public List<GanttActivity> getActivities(int resource) {
		assert (resource >= 0 && resource < getResourceCount());

		// Original comment: cache activities?
		ArrayList<GanttActivity> acts = new ArrayList<GanttActivity>();

		PSTokenList tokens = resources.get(resource).getTokens();
		for (int i = 0; i < tokens.size(); i++) {
			PSToken token = tokens.get(i);

			acts.add(new GanttActivity(token, start, end));
		}

		return acts;
	}

	public GanttResource getResource(int resource) {
		PSResource r = PSResource.asPSResource(resources.get(resource));
		if (r == null)
			return null;
		return new GanttResource(r);
	}

	public int getStart() {
		return start;
	}

	public int getEnd() {
		return end;
	}

	public String getResourceName(int index) {
		return resources.get(index).getEntityName();
	}
}
