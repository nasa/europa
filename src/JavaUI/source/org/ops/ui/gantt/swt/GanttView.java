package org.ops.ui.gantt.swt;

import java.util.Calendar;
import java.util.List;

import org.eclipse.nebula.widgets.ganttchart.AdvancedTooltip;
import org.eclipse.nebula.widgets.ganttchart.ColorCache;
import org.eclipse.nebula.widgets.ganttchart.DefaultSettings;
import org.eclipse.nebula.widgets.ganttchart.GanttChart;
import org.eclipse.nebula.widgets.ganttchart.GanttEvent;
import org.eclipse.nebula.widgets.ganttchart.GanttGroup;
import org.eclipse.nebula.widgets.ganttchart.ISettings;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.part.ViewPart;
import org.ops.ui.gantt.model.GanttActivity;
import org.ops.ui.gantt.model.GanttModel;
import org.ops.ui.main.swt.EuropaPlugin;
import org.ops.ui.solver.model.SolverListener;
import org.ops.ui.solver.model.SolverModel;

/**
 * Gantt chart view - SWT version for the Eclipse plugin
 * 
 * @author Tatiana Kichkaylo
 */
public class GanttView extends ViewPart implements SolverListener {
	public static final String VIEW_ID = "org.ops.ui.gantt.swt.GanttView";

	/** Solver model, initialized in createPartControl */
	private SolverModel solverModel;

	/** The Gantt chart widget */
	private GanttChart ganttChart = null;

	@Override
	public void createPartControl(Composite parent) {
		solverModel = EuropaPlugin.getDefault().getSolverModel();
		solverModel.addSolverListener(this);

		// Create a chart
		DefaultSettings settings = new DefaultSettings() {
			@Override
			public boolean showPropertiesMenuOption() {
				return false;
			}

			@Override
			public boolean showDeleteMenuOption() {
				return false;
			}

			@Override
			public int getInitialView() {
				return VIEW_D_DAY; // numbers, not dates
			}

			@Override
			public int getInitialZoomLevel() {
				return ZOOM_HOURS_MAX;
			}

			@Override
			public boolean enableResizing() {
				return false;
			}

			@Override
			public boolean enableDragAndDrop() {
				return false;
			}

			@Override
			public String getTextDisplayFormat() {
				return ""; // use tooltip to avoid crowding "#name#";
			}

			@Override
			public boolean allowHeaderSelection() {
				return false;
			}

			@Override
			public boolean showDateTips() {
				return false;
			}

			@Override
			public String getDefaultAdvancedTooltipText() {
				StringBuffer buf = new StringBuffer();
				buf.append("\\ceStart #sd#, end #ed#, duration #days#");
				return buf.toString();
			}
		};
		ganttChart = new GanttChart(parent, SWT.NONE, settings);
	}

	@Override
	public void dispose() {
		solverModel.removeSolverListener(this);
		super.dispose();
	}

	@Override
	public void setFocus() {
	}

	public void afterOneStep(long time) {
	}

	public void afterStepping() {
		updateChart();
	}

	public void beforeStepping() {
	}

	public void solverStarted() {
		updateChart();
	}

	public void solverStopped() {
	}

	private void updateChart() {
		ganttChart.getGanttComposite().clearChart();

		GanttModel model = new GanttModel(solverModel);
		int offset = model.getStart();
		for (int i = 0; i < model.getResourceCount(); i++) {
			GanttGroup group = new GanttGroup(ganttChart);

			List<GanttActivity> resActivities = model.getActivities(i);
			for (GanttActivity act : resActivities) {
				int s = act.getStartMin() - offset;
				int e = act.getEndMin() - offset;

				GanttEvent event = new GanttEvent(ganttChart, act.getText(),
						wrap(s), wrap(e), 0);
				if (act.hasViolation())
					event.setStatusColor(ColorCache.getColor(250, 0, 0));
				else
					event.setStatusColor(ColorCache.getColor(0, 250, 0));
				StringBuffer b = new StringBuffer();
				b.append("Start ").append(asInterval(s, act.getStartMax()));
				b.append(", end ").append(asInterval(e, act.getEndMax()));
				b.append(", duration ").append(
						asInterval(act.getDurMin(), act.getDurMax()));
				String title = act.getText() + " @ " + model.getResourceName(i);
				event.setAdvancedTooltip(new AdvancedTooltip(title, b
						.toString()));
				group.addEvent(event);
			}
		}
	}

	private String asInterval(int min, int max) {
		if (min == max)
			return String.valueOf(min);
		return "[" + min + ", " + max + "]";
	}

	private Calendar wrap(int value) {
		Calendar cal;
		if (ganttChart.getSettings().getInitialView() == ISettings.VIEW_D_DAY)
			cal = (Calendar) ganttChart.getSettings().getDDayRootCalendar()
					.clone();
		else
			cal = Calendar.getInstance();
		cal.add(Calendar.DATE, value);
		return cal;
	}
}
