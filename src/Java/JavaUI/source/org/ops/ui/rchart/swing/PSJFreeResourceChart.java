package org.ops.ui.rchart.swing;

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
import org.jfree.data.time.RegularTimePeriod;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;
import org.jfree.ui.RectangleInsets;

import psengine.PSResourceProfile;
import psengine.PSTimePointList;
import org.ops.ui.rchart.model.PSResourceChartModel;

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
            "Profiles for "+resourceName_,  // title
            "Time",             // x-axis label
            "Value",            // y-axis label
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

        int seriesCnt = 5;
        TimeSeries ts[] = new TimeSeries[seriesCnt];
        
        ts[0] = resourceProfileToTimeSeries("FD Level Lower Bound", model_.getFDLevel(),false);
        ts[1] = resourceProfileToTimeSeries("FD Level Upper Bound", model_.getFDLevel(),true);
        ts[2] = resourceProfileToTimeSeries("Limit Lower Bound", model_.getLimit(),false);
        ts[3] = resourceProfileToTimeSeries("Limit Upper Bound", model_.getLimit(),true);
        ts[4] = resourceProfileToTimeSeries("Capacity", model_.getCapacity(),true);
        
        RegularTimePeriod maxX = new Minute(((Calendar)start_.clone()).getTime());
        for (int i=0;i<seriesCnt;i++) {
        	int idx = ts[i].getItemCount()-1;
        	if (idx > 0) {
        		RegularTimePeriod xValue = ts[i].getTimePeriod(idx);
        		if (xValue.compareTo(maxX) > 0)
        			maxX = xValue;
        	}
        }
        
        for (int i=0;i<seriesCnt; i++) {
        	addYvalue(ts[i],maxX);
            dataset.addSeries(ts[i]);
        }
        
        return dataset;
    } 
    
    protected void addYvalue(TimeSeries ts,RegularTimePeriod maxX)
    {
    	if (ts.getItemCount()==0)
    		return;

    	int idx = ts.getItemCount()-1;
    	
    	RegularTimePeriod xValue = ts.getTimePeriod(idx);
    	
    	if (xValue.compareTo(maxX) < 0) {
    		Number yValue = ts.getValue(idx);
    		ts.addOrUpdate(maxX, yValue);
    	}
    }
    
    protected TimeSeries resourceProfileToTimeSeries(String name,PSResourceProfile rp,boolean useUB)
    {
    	TimeSeries ts = new TimeSeries(name,Minute.class);
    	
    	double lastValue = Double.NEGATIVE_INFINITY;
    	int lastTime = Integer.MIN_VALUE;
    	
    	PSTimePointList times = rp.getTimes();
    	for (int j=0; j<times.size();j++) {
    		Integer i = times.get(j);
    		// push negative entries to right before 0
    		if (i < 0)
    			i = -1;
    		
    		Calendar time = (Calendar)start_.clone();
    		// TODO: can't assume time unit is minutes, should ask DSA
    		time.add(Calendar.MINUTE, i);
    		Minute t = new Minute(time.getTime());
    		
    		double value = (useUB ? rp.getUpperBound(i) : rp.getLowerBound(i));
    		
    		// Don't try to display infinity values
    		// Hack!: EUROPA should expose infinity constants through swig
    		double einfinity = 1e8;
    		if (value >= einfinity)
    			continue;
    		
   		    // Display a step function
   		    if ((lastTime >=0) && (lastValue != value) && (i-lastTime > 1)) {
   		    	time.add(Calendar.MINUTE, -1);
   		    	Minute t1 = new Minute(time.getTime());
   	   		    ts.addOrUpdate(t1,lastValue);
   		    }

   		    ts.addOrUpdate(t,value); 
   		    lastTime = i;
   		    lastValue = value;
    	}
    	
    	return ts;    		
    }
}
