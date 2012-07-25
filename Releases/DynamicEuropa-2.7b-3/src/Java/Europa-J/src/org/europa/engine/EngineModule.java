package org.europa.engine;

public interface EngineModule 
{
	public String getName();
	
	public void initialize();
	public void uninitialize();
	
	public void initialize(Engine e);
	public void uninitialize(Engine e);
}
