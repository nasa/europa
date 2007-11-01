package Crew1;

import psengine.*;


public class RMLWriter
{
	protected StringBuffer out_;
	protected int indent_;
	protected int activityCnt_;
	
	public RMLWriter()
	{
	}
	
	protected void generateObservationStart(String id, String name)
	{
	    out_.append(indentation()).append("<Observation Id=\"").append(id).append("\">\n");
	    indent_++;
	    out_.append(indentation()).append("<Name>").append(name).append("</Name>\n");
	}
	
	protected void generateObservationEnd()
	{
		out_.append(indentation()).append("<Custodian></Custodian>\n");
		out_.append(indentation()).append("<UplinkPriority>0</UplinkPriority>\n");
		out_.append(indentation()).append("<StartTime>").append("TODO!! put plan start time here").append("</StartTime>\n");
	    indent_--;
	    out_.append(indentation()).append("</Observation>\n");				
	}
	
	protected String indentation()
	{
		StringBuffer buf = new StringBuffer();
		
		for (int i=0; i < indent_;i++) 
			buf.append("    ");
		
		return buf.toString();
	}
	
	protected void generateActDayNight()
	{
	    // TODO: implement this	
		out_.append(indentation()).append("<!-- TODO!: generate Day-Night activities-->\n");
	}
	
	protected void generateActOrbit()
	{
	    // TODO: implement this	
		out_.append(indentation()).append("<!-- TODO!: generate Orbit activities-->\n");
	}
	
	protected void generateActVHF()
	{
	    // TODO: implement this	
		out_.append(indentation()).append("<!-- TODO!: generate VHF activities-->\n");
	}
	
	protected void generateActTDRS()
	{
	    // TODO: implement this	
		out_.append(indentation()).append("<!-- TODO!: generate TDRS activities-->\n");
	}
	
	protected void generateOSTPVBands(int i)
	{
		String id = "OSTPV Bands "+i;
		generateObservationStart(id,id);

		generateActDayNight();
		generateActOrbit();
		generateActVHF();
		generateActTDRS();	
		
		generateObservationEnd();
	}
	
	protected void generateActivity(PSToken t)
	{
		String id = t.getName() + (activityCnt_++);
		out_.append(indentation()).append("<Activity Id=\"").append(id).append("\"  xsi:type=\"Activity\">\n");
		indent_++;
	    // TODO : generate the body of the activity
		out_.append(indentation()).append("<!-- TODO!: generate activity body -->\n");
		indent_--;
     	out_.append(indentation()).append("</Activity>\n"); 	
	}
	
	protected void generateActivities(PSObject obj)
	{
		generateObservationStart(obj.getName(),obj.getName());

		PSTokenList toks = obj.getTokens();
	    
	    for (int i=0;i<toks.size();i++)
	    	generateActivity(toks.get(i));
	    
	    generateObservationEnd();
	}
	
	protected void generateConstraints()
	{
		// TODO: implement this
		out_.append(indentation()).append("<!-- TODO!: generate constraints -->\n");
	}
	
	public String toRML(PSEngine e)
	{
	    out_ = new StringBuffer();	
        indent_ = 0;
        activityCnt_=0;
        
		out_.append(indentation()).append("<RML Version=\"2.3j\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"); indent_++;
		out_.append(indentation()).append("<Plan>\n"); indent_++;
		out_.append(indentation()).append("<Observations>\n"); indent_++;

		for (int i=1;i<=2;i++)
		    generateOSTPVBands(1);
		
		// TODO: there is a 3rd OSTPV band that looks different
		out_.append(indentation()).append("<!-- TODO!: generate  3rd OSTPV bands observation -->\n");
		
		PSObjectList objs = e.getObjectsByType("Crew");
		for (int i=0;i<objs.size();i++) 
			generateActivities(objs.get(i));
		
		generateConstraints();

		out_.append(indentation()).append("</Observations>\n"); indent_--;
		out_.append(indentation()).append("</Plan>\n"); indent_--;
		out_.append(indentation()).append("</RML>\n"); indent_--;
		
	    return out_.toString();
	}	
}

