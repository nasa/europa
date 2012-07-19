package org.ops.ui.solver.swt;

import java.util.ArrayList;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.RowData;
import org.eclipse.swt.widgets.Composite;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import org.ops.ui.solver.model.StepStatisticsRecord;

/**
 * Statistics charts of the solver moved into a separate view.
 * 
 * @author Tatiana Kichkaylo
 */
public class StatisticsChartsView extends SolverModelViewImpl {
	public static final String VIEW_ID = "org.ops.ui.solver.swt.StatisticsView";

	/* Labels for chart */
	private final String lblTimePerStep = "Time (secs) per Step ",
			lblAvgPerStep = "Avg time (sec) per Step",
			lblStepNumber = "Step Number", lblTimeSec = "Time (secs)",
			lblOpenDecCount = "Open Decision Count",
			lblDecsInPlan = "Decisions in Plan";

	protected XYSeries stepTimeSeries = new XYSeries(lblTimePerStep);
	protected XYSeries stepAvgTimeSeries = new XYSeries(lblAvgPerStep);
	protected XYSeries decisionCntSeries = new XYSeries(lblOpenDecCount);
	protected XYSeries solverDepthSeries = new XYSeries(lblDecsInPlan);

	/** Total running time, seconds */
	private double totalTimeSec = 0;

	/** Switch this view to the given model, possibly NULL */
	@Override
	public void setModel() {
		super.setModel();
		if (model != null && !model.isTerminated())
			updateChartData();
	}

	@Override
	public void createPartControl(Composite parent) {
		FillLayout layout = new FillLayout();
		parent.setLayout(layout);

		XYSeriesCollection data = new XYSeriesCollection();
		data.addSeries(this.stepTimeSeries);
		data.addSeries(this.stepAvgTimeSeries);
		new SolverChartComposite(parent, SWT.BORDER, lblTimePerStep,
				lblStepNumber, lblTimeSec, data, false)
				.setLayoutData(new RowData(200, 200));
		data = new XYSeriesCollection();
		data.addSeries(this.decisionCntSeries);
		new SolverChartComposite(parent, SWT.BORDER, lblOpenDecCount,
				lblStepNumber, lblOpenDecCount, data, false);
		data = new XYSeriesCollection();
		data.addSeries(this.solverDepthSeries);
		new SolverChartComposite(parent, SWT.BORDER, lblDecsInPlan,
				lblStepNumber, lblDecsInPlan, data, false);

		// If the solver is already running, update the chart data
		setModel();
		if (model != null && !model.isTerminated())
			updateChartData();
	}

	private void updateChartData() {
		clearSeries();

		// Total step count
		int stepCount = model.getStepCount();
		for (int i = 1; i <= stepCount; i++) {
			StepStatisticsRecord rec = model.getStepStatistics(i);
			double secs = rec.getDurationMs() / 1000.0;
			totalTimeSec += secs;
			ArrayList<String> flaws = rec.getFlaws();
			int decs = flaws == null ? 0 : flaws.size();
			solverDepthSeries.add(i, rec.getDepth());
			stepTimeSeries.add(i, secs);
			stepAvgTimeSeries.add(i, totalTimeSec / i);
			decisionCntSeries.add(i, decs);
		}
	}

	@Override
	public void afterOneStep(long time) {
		int stepCnt = model.getStepCount();
		double secs = time / 1000.0;
		totalTimeSec += secs;
		StepStatisticsRecord rec = model.getStepStatistics(stepCnt);
		ArrayList<String> flaws = rec.getFlaws();
		int decs = flaws == null ? 0 : flaws.size();
		solverDepthSeries.add(stepCnt, rec.getDepth());
		stepTimeSeries.add(stepCnt, secs);
		stepAvgTimeSeries.add(stepCnt, totalTimeSec / stepCnt);
		decisionCntSeries.add(stepCnt, decs);
	}

	@Override
	public void solverStarted() {
		totalTimeSec = 0;
	}

	@Override
	public void solverStopped() {
		clearSeries();
	}

	private void clearSeries() {
		stepTimeSeries.clear();
		stepAvgTimeSeries.clear();
		decisionCntSeries.clear();
		solverDepthSeries.clear();
	}
}
