package dsa.impl;

import java.util.*;

import dsa.Parameter;
import dsa.Slot;
import dsa.Token;

public class TokenImpl
    extends EntityBase
    implements Token, Slot
{

    public TokenImpl(String type, int key, int startLb, int startUb, int endLb, int endUb, int durationLb, int durationUb )
    {
	super(key,type);
	m_type = type;
	m_earliestStart = startLb;
	m_latestStart = startUb;
	m_earliestEnd = endLb;
	m_latestEnd = endUb;
	m_durationMin = durationLb;
	m_durationMax = durationUb;
	m_parameters = new Vector<Parameter>();
    }

    public String getType(){return m_type;}

    public boolean isTrue(){return true;}

    public int getEarliestStart() {return m_earliestStart;}

    public int getLatestStart() {return m_latestStart;}

    public int getEarliestEnd() {return m_earliestEnd;}

    public int getLatestEnd() {return m_latestEnd;}

    public int getDurationMin() {return m_durationMin;}

    public int getDurationMax() {return m_durationMax;}

    public List<Parameter> getParameters(){
	return m_parameters;
    }


    private String m_type;
    private int m_earliestStart;
    private int m_latestStart;
    private int m_earliestEnd;
    private int m_latestEnd;
    private int m_durationMin;
    private int m_durationMax;
    private List<Parameter> m_parameters;
}
