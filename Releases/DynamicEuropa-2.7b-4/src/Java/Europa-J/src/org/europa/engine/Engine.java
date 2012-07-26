package org.europa.engine;

import java.io.Reader;
import java.util.Collection;

public interface Engine 
{
	public void start();
	public void shutdown();
	
	public boolean isStarted();
	
	public String executeScript(String language, Reader script);
	
	public EngineConfig getConfig();	
	public void setConfig(EngineConfig cfg);	
	
	public void addModule(EngineModule m);
	public EngineModule removeModule(String name);
	public EngineModule getModule(String name);
	public Collection<EngineModule> getAllModules();

	public void addComponent(EngineComponent c);
	public EngineComponent removeComponent(String name);
	public EngineComponent getComponent(String name);
	public Collection<EngineComponent> getAllComponents();

	public void addLanguageInterpreter(String language, LanguageInterpreter interpreter);
	public LanguageInterpreter removeLanguageInterpreter(String language);
	public LanguageInterpreter getLanguageInterpreter(String language);
	public Collection<LanguageInterpreter> getAllInterpreters();
}
