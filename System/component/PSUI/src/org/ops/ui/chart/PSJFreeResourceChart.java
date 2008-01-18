package org.ops.ui.chart;

import java.util.Calendar;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.text.SimpleDateFormat;

import javax.swing.JPanel;
import javax.swing.JScrollPane;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.time.Minute;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;
import org.jfree.ui.RectangleInsets;

import psengine.PSResourceProfile;
import psengine.PSTimePointList;

public class PSJFreeResourceChart 
    extends PSResourceChart 
{
	/**
	 * 
	 */
	private static final long serialVersionUID = 9008639833839752404L;
	
	JFreeChart chart_;
	Calendar start_;
	
	public PSJFreeResourceChart(String resourceName,
			                    PSResourceChartModel model,
			                    Calendar start)
	{
		super(model,resourceName);
		
		start_ = start;
		chart_ = createChart();
		JPanel chartPanel = makeResourcePanel(chart_);
		setLayout(new BorderLayout());
		add(new JScrollPane(chartPanel));
	}
	
    protected JFreeChart createChart() 
    {
    	XYDataset dataset = createDataset();
        JFreeChart chart = ChartFactory.createTimeSeriesChart(
            "Capacity and Usage Profiles for "+resourceName_,  // title
            "Date",             // x-axis label
            "Level",            // y-axis label
            dataset,            // data
            true,               // create legend?
            true,               // generate tooltips?
            false               // generate URLs?
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
        
        DateAxis axis = (DateAxis) plot.getDomainAxis();
        axis.setDateFormatOverride(new SimpleDateFormat("dd/MM/yy"));
        
        return chart;
    }	
    
    protected JPanel makeResourcePanel(JFreeChart chart)
    {
        ChartPanel chartPanel = new ChartPanel(chart);
        chartPanel.setPreferredSize(new Dimension(500, 270));
        chartPanel.setMouseZoomable(true, false);
        
        return chartPanel;
    }    

    protected XYDataset createDataset() 
    {   	
        TimeSeriesCollection dataset = new TimeSeriesCollection();

        TimeSeries s1 = resourceProfileToTimeSeries("Capacity", model_.getCapacity(),true);
        TimeSeries s2 = resourceProfileToTimeSeries("Usage Upper Bound", model_.getUsage(),true);
        TimeSeries s3 = resourceProfileToTimeSeries("Usage Lower Bound", model_.getUsage(),false);
        dataset.addSeries(s1);
        dataset.addSeries(s2);
        dataset.addSeries(s3);
        
        return dataset;
    }        
    
    protected TimeSeries resourceProfileToTimeSeries(String name,PSResourceProfile rp,boolean useUB)
    {
    	TimeSeries ts = new TimeSeries(name,Minute.class);
    	
    	double lastValue = Double.NEGATIVE_INFINITY;
    	int lastTime = Integer.MIN_VALUE;
    	
    	PSTimePointList times = rp.getTimes();
    	for (int j=0; j<times.size();j++) {
    		Integer i = times.get(j);
    		// Ignore initial MINUS_INFINITY entry
    		if (i < 0)
    			continue;
    		
    		Calendar time = (Calendar)start_.clone();
    		// TODO: can't assume time unit is minutes, should ask DSA
    		time.add(Calendar.MINUTE, i);
    		Minute t = new Minute(time.getTime());
    		
    		double value = (useUB ? rp.getUpperBound(i) : rp.getLowerBound(i));
   		    // Display a step function
   		    if ((lastTime >=0) && (lastValue != value) && (i-lastTime > 1)) {
   		    	time.add(Calendar.MINUTE, -1);
   		    	Minute t1 = new Minute(time.getTime());
   	   		    ts.add(t1,lastValue);
   		    }

   		    ts.add(t,value); 
   		    lastTime = i;
   		    lastValue = value;
    	}
    	
    	return ts;    		
    }
}
