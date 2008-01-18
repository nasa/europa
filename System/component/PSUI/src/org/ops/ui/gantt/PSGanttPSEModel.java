package org.ops.ui.gantt;

import java.util.Calendar;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.text.SimpleDateFormat;
import java.util.Locale;

import org.ops.ui.gantt.PSGanttActivity;
import org.ops.ui.gantt.PSGanttActivityImpl;

import psengine.PSEngine;
import psengine.PSToken;
import psengine.PSTokenList;
import psengine.PSObjectList;

public class PSGanttPSEModel 
    implements PSGanttModel 
{
	Calendar startHorizon_;
	String objectsType_;
	PSObjectList resources_;
    int timeUnit_;
	
    public PSGanttPSEModel(PSEngine pse, Calendar startHorizon, String objectsType)
    {
        this(pse,startHorizon,objectsType,Calendar.MINUTE);    
    }
    
    public PSGanttPSEModel(PSEngine pse, Calendar startHorizon, String objectsType, int timeUnit)
	{
	    startHorizon_ = startHorizon;
	    objectsType_ = objectsType;
	    resources_ = pse.getObjectsByType(objectsType_);
	    timeUnit_=timeUnit;
	    
	}

	public Iterator<PSGanttActivity> getActivities(int resource) 
	{
		assert (resource >=0 && resource < getResourceCount());
		
		// TODO: cache activities?
		List<PSGanttActivity> acts = new ArrayList<PSGanttActivity>();
		
		PSTokenList tokens = resources_.get(resource).getTokens();
		for (int i=0;i<tokens.size();i++) {
			PSToken token = tokens.get(i);
			acts.add(new PSGanttActivityImpl(token.getKey(),
					                         instantToCalendar(token.getStart().getLowerBound()),
					                         instantToCalendar(token.getEnd().getLowerBound()),
					                         token.getViolation()
					                         )
			);
		}
		
		return acts.iterator();
	}
	
	protected Calendar instantToCalendar(double i)
	{
		Calendar retval = (Calendar)startHorizon_.clone();
		retval.add(timeUnit_, (int)i);
		//System.out.println("instantToCalendar:"+i.value()+" -> "+SimpleDateFormat.getInstance().format(retval.getTime()));
		return retval;
	}

	public String getResourceColumn(int resource, int column) 
	{
		if (column == 0 && resource < getResourceCount())
			return resources_.get(resource).getName();
		
		return "";
	}

	static String resourceColumnNames_[] = { "Name" };
	public String[] getResourceColumnNames() 
	{
		return resourceColumnNames_;
	}

	public int getResourceCount() 
	{
		return resources_.size();
	}

	public void setActivityStart(Object key, Calendar start) 
	{
		// TODO Auto-generated method stub
		notifyChange(key,"StartChanged",start);
	}

	public void setActivityFinish(Object key, Calendar finish) 
	{
		// TODO Auto-generated method stub
		notifyChange(key,"FinishChanged",finish);
	}

    static int notificationCnt_=0;
	protected void notifyChange(Object key, String type, Object value)
	{
		Object newValue=value;
		
		if (value instanceof Calendar) {
			SimpleDateFormat formatter = new SimpleDateFormat("MM/dd/yyyy hh:mm:ss", new Locale("en","US"));
			newValue = formatter.format(((Calendar)value).getTime());
		}
		
        System.out.println(++notificationCnt_ + " - Object changed - {"+
        		"id:"+key+" "+
        		"type:"+type+" "+
        		"newValue:"+newValue+ "}"
        );		
	}
}
