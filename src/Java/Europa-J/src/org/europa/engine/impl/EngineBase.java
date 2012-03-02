package org.europa.engine.impl;

import java.io.Reader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.europa.engine.Engine;
import org.europa.engine.EngineComponent;
import org.europa.engine.EngineConfig;
import org.europa.engine.EngineModule;
import org.europa.engine.LanguageInterpreter;

public class EngineBase 
	implements Engine 
{
	protected boolean isStarted_;
	protected EngineConfig config_;
	protected List<EngineModule> modules_;
	protected Map<String,EngineComponent> components_;
	protected Map<String,LanguageInterpreter> interpreters_;
	
	public EngineBase()
	{
		isStarted_ = false;
		modules_ = new ArrayList<EngineModule>();
		components_ = new HashMap<String,EngineComponent>();
		interpreters_ = new HashMap<String,LanguageInterpreter>();
	}
	
	@Override
	public void start() 
	{
    	if(!isStarted_)
    	{
            initializeModules();
    		initializeByModules();
    		isStarted_ = true;
    	}
	}
	
	protected void initializeModules()
	{
		for(EngineModule m : modules_)
			m.initialize();
	}
	
	protected void initializeByModules()
	{
		for(EngineModule m : modules_)
			m.initialize(this);
	}

	@Override
	public void shutdown() 
	{
    	if(isStarted_)
    	{
    		uninitializeByModules();
            uninitializeModules();
    		isStarted_ = false;
    	}
	}

	protected void uninitializeModules()
	{
		for(EngineModule m : modules_)
			m.uninitialize();
	}
	
	protected void uninitializeByModules()
	{
		for(EngineModule m : modules_)
			m.uninitialize(this);
	}

	@Override
	public boolean isStarted() 
	{
		return isStarted_;
	}

	@Override
	public String executeScript(String language, Reader script) 
	{
		LanguageInterpreter i = interpreters_.get(language);
		
		if (i == null)
			throw new RuntimeException("No interpreter registered for language:"+language);
		
		return i.interpret(script, "<stdin>"); // TODO: fix input name
	}

	@Override
	public EngineConfig getConfig() 
	{
		return config_;
	}

	@Override
	public void setConfig(EngineConfig cfg) 
	{
		config_ = cfg;
	}

	@Override
	public void addModule(EngineModule m) 
	{
		// TODO Auto-generated method stub
	}

	@Override
	public EngineModule removeModule(String name) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public EngineModule getModule(String name) 
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Collection<EngineModule> getAllModules() 
	{
		return modules_;
	}

	@Override
	public void addComponent(EngineComponent c) {
		// TODO Auto-generated method stub

	}

	@Override
	public EngineComponent removeComponent(String name) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public EngineComponent getComponent(String name) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Collection<EngineComponent> getAllComponents() 
	{
		return components_.values();
	}

	@Override
	public LanguageInterpreter addLanguageInterpreter(String language,
			LanguageInterpreter interpreter) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public LanguageInterpreter removeLanguageInterpreter(String language) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public LanguageInterpreter getLanguageInterpreter(String language) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Collection<LanguageInterpreter> getAllInterpreters() 
	{
		return interpreters_.values();
	}
}
