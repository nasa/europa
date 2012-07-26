package org.ops.ui.gantt.swing;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Calendar;
import java.util.GregorianCalendar;

import javax.swing.JTable;
import javax.swing.JButton;
import javax.swing.SwingUtilities;
import javax.swing.table.TableModel;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableCellEditor;

import com.egantt.model.drawing.ContextConstants;
import com.egantt.model.drawing.ContextResources;
import com.egantt.model.drawing.DrawingPart;
import com.egantt.model.drawing.DrawingState;
import com.egantt.model.drawing.axis.AxisInterval;
import com.egantt.model.drawing.axis.MutableInterval;
import com.egantt.model.drawing.axis.MutableIntervalListener;
import com.egantt.model.drawing.part.ListDrawingPart;
import com.egantt.model.drawing.part.MutableDrawingPart;
import com.egantt.model.drawing.part.event.DrawingPartEvent;
import com.egantt.model.drawing.part.event.DrawingPartListener;
import com.egantt.model.drawing.state.AbstractDrawingState;
import com.egantt.model.drawing.state.BasicDrawingState;

import ext.egantt.drawing.module.EditorDrawingModule;
import ext.egantt.drawing.module.GradientColorModule;
import ext.egantt.swing.GanttDrawingPartHelper;
import ext.egantt.swing.GanttEntryHelper;
import ext.egantt.swing.GanttTable;
import ext.egantt.swing.GanttToolBar;
import ext.egantt.swing.JTableHelper;

import org.ops.ui.gantt.model.PSGanttActivity;
import org.ops.ui.gantt.model.PSGanttModel;

/*
 * PSGantt implementation that uses egantt
 */
public class PSEGantt 
    extends PSGantt
    implements MouseMotionListener, DrawingPartListener
{
	/**
	 * 
	 */
	private static final long serialVersionUID = -7727371579397987014L;
	
	GanttTable gantt_;
    Map<Object,MutableDrawingPart> actEntries_;
    JButton refreshBtn_;
    Calendar start_;
    Calendar end_;
    PSGanttColorProvider colorProvider_;
	
    public PSEGantt(PSGanttModel model,
    		        Calendar start, 
    		        Calendar end)
    {
    	super(model);
    	start_ = start;
    	end_ = end;
    	colorProvider_ = new DefaultColorProvider();
    	
    	makeGantt();
    	
    	refreshBtn_ = new JButton("Refresh");
    	setLayout(new BorderLayout());
    	//add(BorderLayout.NORTH,refreshBtn_);
    	add(BorderLayout.CENTER,gantt_);
    	add(new GanttToolBar(gantt_.getViewManager(GanttTable.TIME_AXIS)), BorderLayout.SOUTH);    	
    }
    
    public PSGanttColorProvider getColorProvider() { return colorProvider_; }
    public void setColorProvider(PSGanttColorProvider cp) { colorProvider_ = cp; }
    
    protected void makeGantt()
    {
    	gantt_ = new GanttTable(makeTableModel(model_));

    	//System.out.println("model rows:"+model.getTableModel().getRowCount());
    	TableColumnModel columnModel = gantt_.getColumnModel(1);
    	TableColumn column = columnModel.getColumn(0);
    	
    	// Enable drag and drop
    	TableCellEditor editor = gantt_.getDefaultEditor(1, AbstractDrawingState.class);
    	column.setCellEditor(editor);
    	
    	gantt_.getDrawingContext().put(
    	    	ContextConstants.EDITING_AXIS,
    	    	ContextResources.OTHER_PROPERTY,
    	    	GanttTable.TIME_AXIS);

	    gantt_.getDrawingContext().put(
	    		ContextConstants.EDITING_MODE,
    	    	ContextResources.OTHER_PROPERTY,  
    	    	EditorDrawingModule.MOVE_RESIZE_EDITOR);    	

    	// Display timeline at the top
    	column.setHeaderValue(GanttEntryHelper.createCalendar());
    	gantt_.setTimeRange(start_.getTime(),end_.getTime());    	
    	gantt_.addMouseMotionListener(this);

    	/* TODO: Disable row selection?
    	JTable activityTable = gantt_.getTableComponent(1);
    	activityTable.setRowSelectionAllowed(false);
    	activityTable.setColumnSelectionAllowed(false);
    	activityTable.setCellSelectionEnabled(false);
    	*/    	    	
    }
    
    public void refresh()
    {
    	remove(gantt_);
    	makeGantt();
    	add(BorderLayout.CENTER,gantt_); 
    }
    
    protected TableModel makeTableModel(PSGanttModel model)
    {
    	String[][] columnNames = {
    			model.getResourceColumnNames(),
    			{"Timeline"}
    	};

    	Object[][] data = new Object[model.getResourceCount()][2];

    	actEntries_ = new HashMap<Object,MutableDrawingPart>();
    	for (int i=0; i<model.getResourceCount();i++) {
    		BasicDrawingState activities = GanttDrawingPartHelper.createDrawingState();
    		ListDrawingPart actList = GanttDrawingPartHelper.createDrawingPart();

    		Iterator<PSGanttActivity> resActivities = model.getActivities(i);
    		while (resActivities.hasNext()) {
    			PSGanttActivity act = resActivities.next();
    			String context = mapColor(colorProvider_.getColor(act));
    			
    			MutableDrawingPart entry = GanttDrawingPartHelper.createActivityEntry(
    					act.getKey(), 
    					act.getStart().getTime(),
    					act.getFinish().getTime(), 
    					context, 
    					actList
    			);    	    	
    			actEntries_.put(act.getKey(), entry);
    			//System.out.println("ID:"+key+" Start:"+start.getTime()+" Finish:"+finish.getTime());
    		}

			actList.addDrawingPartListener(this);
    		activities.addDrawingPart(actList);

    		data[i][0] = model.getResourceColumn(i, 0);
    		data[i][1] = activities;
    	}

    	return JTableHelper.createTableModel(data, columnNames);    		
    }
    
	// DrawingPartListener methods
	public void stateChanged(DrawingPartEvent event) 
	{
		String colors[] = {
				GradientColorModule.GREEN_GRADIENT_CONTEXT,
				GradientColorModule.RED_GRADIENT_CONTEXT,				
		};
		
		Object key = event.getSource();
		MutableDrawingPart entry = actEntries_.get(key);
		//LongInterval li = (LongInterval)entry.getInterval()[0];		
		//int idx = (int)(Math.round(Math.random()));
		//System.out.println("range:"+li.getRangeValue()+" idx:"+idx);
		//entry.setContext(key, colors[idx]);
		
		List changes = (List)(event.changes().next());
		
		int changeType = ((Integer)changes.get(0)).intValue();
		Long newValue = (Long)changes.get(1);
		Calendar calValue = new GregorianCalendar();
		calValue.setTime(new java.util.Date(newValue.longValue()));
		
		switch (changeType) {
		  case MutableIntervalListener.START_CHANGED :
			  model_.setActivityStart(key,calValue);
			  break;
		  case MutableIntervalListener.FINISH_CHANGED :
			  model_.setActivityFinish(key,calValue);
			  break;
	      default:
	    	  System.err.println("ERROR:Unknow MutableIntervalListener change:"+changeType);
		}
	}    
    
	// MouseMotionListenerMethods    
    public void mouseDragged(MouseEvent e) 
    {
    	if (e.isPopupTrigger())
    		return;
    }

    public void mouseMoved(MouseEvent e) 
    { 
    	if (e.isPopupTrigger())
    		return;

    	Rectangle cellRect =  getCellRect((JTable) e.getComponent(), e.getPoint());
    	if (cellRect == null)
    		return;

    	DrawingState state = getDrawingState((JTable) e.getComponent(), e.getPoint());
    	if (state == null)
    		return;
    	
    	MouseEvent evt = getTranslatedMouseEvent(e, cellRect);

    	MutableInterval interval = (MutableInterval) getInterval(
				evt.getPoint(),
				3,  
				state, 
				GanttTable.TIME_AXIS);
		
    	Object key = (interval == null ? null : interval.getKey());
    	notifyMouseMoved(key);
    }

	public MutableInterval getInterval(
			Point point, 
			int buffer, 
			DrawingState drawing, 
			Object axisKey) 
	{
		Object key = drawing.getValueAt(point, buffer, buffer);
		return (MutableInterval) getInterval(key, drawing, axisKey);
	}
	
	protected AxisInterval getInterval(Object key, DrawingState drawing, Object axisKey) 
	{
		for (Iterator iter = drawing.parts(); iter.hasNext();) {
			DrawingPart part = (DrawingPart) iter.next();
			if (part.isSummaryPart())
				continue;
			
			AxisInterval[] interval = part.getInterval(key,
					new AxisInterval[] {});
			int index = part.keys().indexOf(axisKey);
			if (index < 0)
				continue;
			
			if (interval != null && index < interval.length && interval[index] != null)
				return interval[index];
		}
		return null;
	}

	protected Rectangle getCellRect(JTable table, Point location) 
    {
    	int row = table.rowAtPoint(location);
    	int column = table.columnAtPoint(location);

    	if (row < 0 || column < 0)
    		return null;

    	return table.getCellRect(row, column, true);
    }		
    
	protected MouseEvent getTranslatedMouseEvent(MouseEvent evt, Rectangle cellRect) 
	{ 
		Point location = new Point(evt.getPoint());
		location.translate(-cellRect.x, -cellRect.y);
		
		
		Component component = SwingUtilities.getAncestorOfClass(GanttTable.class, evt.getComponent()) != null 
				? SwingUtilities.getAncestorOfClass(GanttTable.class, evt.getComponent()) : evt.getComponent();
		return location != null ? 
				new MouseEvent(component, evt.getID(), evt.getWhen(), evt.getModifiers(),
				location.x, location.y, evt.getClickCount(), evt.isPopupTrigger(), evt.getButton()) : null;
	}    
	
	protected DrawingState getDrawingState(JTable table, Point location) 
	{
		int row = table.rowAtPoint(location);
		int column = table.columnAtPoint(location);
	
		if (row < 0 || column < 0)
			return null;
	
		Object value = table.getValueAt(row, column);
		return value instanceof DrawingState ? (DrawingState) value : null;
	}	
	
	protected static class DefaultColorProvider
	    implements PSGanttColorProvider
    {
        public Color getColor(PSGanttActivity activity) 
        {
            return (activity.getViolation() == 0 ? Color.GREEN : Color.RED);
        }
    }
	
	static Map<Color,String> colorMap_;
	
	static String mapColor(Color c)
	{
	    String retval = colorMap_.get(c);
	    
	    if (retval == null)
	        retval = GradientColorModule.WHITE_GRADIENT_CONTEXT;
	    
	    return retval;
	}
	
	static 
	{
	    colorMap_ = new HashMap<Color,String>();
	    colorMap_.put(Color.BLACK, GradientColorModule.BLACK_GRADIENT_CONTEXT);
        colorMap_.put(Color.BLUE, GradientColorModule.BLUE_GRADIENT_CONTEXT);
        colorMap_.put(Color.GREEN, GradientColorModule.GREEN_GRADIENT_CONTEXT);
        colorMap_.put(Color.RED, GradientColorModule.RED_GRADIENT_CONTEXT);
        colorMap_.put(Color.WHITE, GradientColorModule.WHITE_GRADIENT_CONTEXT);
	}
}
