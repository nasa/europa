package org.europa.engine.impl;

import java.io.Reader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;
import org.europa.engine.Engine;
import org.europa.engine.EngineComponent;
import org.europa.engine.EngineConfig;
import org.europa.engine.EngineModule;
import org.europa.engine.LanguageInterpreter;

public class EngineBase 
	implements Engine 
{
	private final static Logger LOG = Logger.getLogger(EngineBase.class);

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
    		LOG.info("Engine starting...");
            initializeModules();
    		initializeByModules();
    		isStarted_ = true;
    		LOG.info("Engine started");
    	}
	}
	
	protected void initializeModules()
	{
		for(EngineModule m : modules_)
			initializeModule(m);
	}
	
	protected void initializeModule(EngineModule m)
	{
		m.initialize();
		LOG.info("Initialized module: "+m.getName());
	}
	
	protected void initializeByModules()
	{
		for(EngineModule m : modules_)
			initializeByModule(m);
	}
	
	protected void initializeByModule(EngineModule m)
	{
		m.initialize(this);
		LOG.info("Engine initialized by module: "+m.getName());
	}
	
	@Override
	public void shutdown() 
	{
    	if(isStarted_) {
    		LOG.info("Engine shutting down...");
    		uninitializeByModules();
            uninitializeModules();
    		isStarted_ = false;
    		LOG.info("Engine shutdown finished");
    	}
	}

	protected void uninitializeModules()
	{
		for(EngineModule m : modules_)
			uninitializeModule(m);
	}
	
	protected void uninitializeModule(EngineModule m)
	{
		m.uninitialize();
		LOG.info("Uninitialized module: "+m.getName());
	}
	
	protected void uninitializeByModules()
	{
		for(EngineModule m : modules_)
			uninitializeByModule(m);
	}

	protected void uninitializeByModule(EngineModule m)
	{
		m.uninitialize(this);
		LOG.info("Engine uninitialized by module: "+m.getName());
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
		modules_.add(m);
		
		if (isStarted()) {
			initializeModule(m);
			initializeByModule(m);
		}
	}

	@Override
	public EngineModule removeModule(String name) 
	{
		EngineModule m = getModule(name);
		if (m != null) {
			modules_.remove(m);
			if (isStarted()) {
				uninitializeByModule(m);
				uninitializeModule(m);
			}
		}
		
		return m;
	}

	@Override
	public EngineModule getModule(String name) 
	{
		for (EngineModule m : modules_) 
			if (m.getName().equals(name)) 
				return m;
			
		return null;
	}

	@Override
	public Collection<EngineModule> getAllModules() 
	{
		return modules_;
	}

	@Override
	public void addComponent(EngineComponent c) 
	{
		EngineComponent old = components_.put(c.getName(), c);
		c.setEngine(this);
		
		if (old != null)
			LOG.info("Component "+c.getName()+" has been replaced by a new instance"); 	
	}

	@Override
	public EngineComponent removeComponent(String name) 
	{
		EngineComponent old = components_.remove(name);
		if (old != null)
			old.setEngine(null);
		
		return old;
	}

	@Override
	public EngineComponent getComponent(String name) 
	{
		return components_.get(name);
	}

	@Override
	public Collection<EngineComponent> getAllComponents() 
	{
		return components_.values();
	}

	@Override
	public void addLanguageInterpreter(String language,
			LanguageInterpreter interpreter) 
	{
		LanguageInterpreter old = interpreters_.put(language, interpreter);	
		
		if (old != null)
			LOG.info("Interpreter for language "+language+" has been replaced by a new one"); 	
	}

	@Override
	public LanguageInterpreter removeLanguageInterpreter(String language) 
	{
		return interpreters_.remove(language);
	}

	@Override
	public LanguageInterpreter getLanguageInterpreter(String language) 
	{
		return interpreters_.get(language);
	}

	@Override
	public Collection<LanguageInterpreter> getAllInterpreters() 
	{
		return interpreters_.values();
	}
}
