package dsa.impl;

import dsa.Entity;

public class EntityBase 
    implements Entity 
{
    protected int m_key;
    protected String m_name;

    public EntityBase(int key)
    {
        this(key,null);	
    }
    
    public EntityBase(int key, String name) 
    {
    	m_key = key;
    	if (name == null)
    		m_name = this.getClass().getSimpleName() + "-" + key;
    	else 
    		m_name = name;
    }

    public int getKey() {return m_key;}
    public String getName() { return m_name; }
}
