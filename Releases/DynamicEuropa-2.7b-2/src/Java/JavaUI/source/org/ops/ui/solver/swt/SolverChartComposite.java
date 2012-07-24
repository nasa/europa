package org.ops.ui.solver.swt;

import java.awt.Color;
import java.awt.Font;

import org.eclipse.swt.widgets.Composite;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeriesCollection;
import org.jfree.experimental.chart.swt.ChartComposite;
import org.jfree.ui.RectangleInsets;

public class SolverChartComposite extends ChartComposite {
	public static final Font DEFAULT_TITLE_FONT = new Font("SansSerif",
			Font.BOLD, 12);

	public SolverChartComposite(Composite comp, int style, String title,
			String xAxisLabel, String yAxisLabel, XYSeriesCollection dataset,
			boolean createLegend) {
		super(comp, style);

		JFreeChart chart = createChart(title, xAxisLabel, yAxisLabel, dataset,
				createLegend);
		this.setChart(chart);
	}

	protected JFreeChart createChart(String title, String xAxisLabel,
			String yAxisLabel, XYDataset dataset, boolean createLegend) {
		boolean generateTooltips = true;
		boolean generateURLs = false;
		JFreeChart chart = ChartFactory.createXYLineChart(title, xAxisLabel,
				yAxisLabel, dataset, PlotOrientation.VERTICAL, createLegend,
				generateTooltips, generateURLs);

		chart.setBackgroundPaint(Color.white);

		XYPlot plot = (XYPlot) chart.getPlot();
		plot.setBackgroundPaint(Color.lightGray);
		plot.setDomainGridlinePaint(Color.white);
		plot.setRangeGridlinePaint(Color.white);
		plot.setAxisOffset(new RectangleInsets(5.0, 5.0, 5.0, 5.0));
		plot.setDomainCrosshairVisible(true);
		plot.setRangeCrosshairVisible(true);

		XYItemRenderer renderer = plot.getRenderer();
		if (renderer instanceof XYLineAndShapeRenderer) {
			XYLineAndShapeRenderer rrenderer = (XYLineAndShapeRenderer) renderer;
			rrenderer.setBaseShapesVisible(true);
			rrenderer.setBaseShapesFilled(true);
		}

		chart.getTitle().setFont(DEFAULT_TITLE_FONT);

		return chart;
	}
}
