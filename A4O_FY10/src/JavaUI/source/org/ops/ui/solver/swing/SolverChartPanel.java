package org.ops.ui.solver.swing;

import java.awt.Color;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeriesCollection;
import org.jfree.ui.RectangleInsets;

public class SolverChartPanel extends ChartPanel {
	public SolverChartPanel(String title, String xAxisLabel, String yAxisLabel,
			XYSeriesCollection dataset, boolean createLegend) {
		super(null);
		JFreeChart chart = createChart(title, xAxisLabel, yAxisLabel, dataset,
				createLegend);
		this.setChart(chart);
		this.setMouseZoomable(true, false);
	}

	protected JFreeChart createChart(String title, String xAxisLabel,
			String yAxisLabel, XYDataset dataset, boolean createLegend) {
		JFreeChart chart = ChartFactory.createXYLineChart(title, xAxisLabel,
				yAxisLabel, dataset, PlotOrientation.VERTICAL, createLegend,
				true, // generate tooltips?
				false // generate URLs?
				);

		chart.setBackgroundPaint(Color.white);

		XYPlot plot = (XYPlot) chart.getPlot();
		plot.setBackgroundPaint(Color.lightGray);
		plot.setDomainGridlinePaint(Color.white);
		plot.setRangeGridlinePaint(Color.white);
		plot.setAxisOffset(new RectangleInsets(5.0, 5.0, 5.0, 5.0));
		plot.setDomainCrosshairVisible(true);
		plot.setRangeCrosshairVisible(true);

		XYItemRenderer r = plot.getRenderer();
		if (r instanceof XYLineAndShapeRenderer) {
			XYLineAndShapeRenderer renderer = (XYLineAndShapeRenderer) r;
			renderer.setBaseShapesVisible(true);
			renderer.setBaseShapesFilled(true);
		}

		return chart;
	}
}
