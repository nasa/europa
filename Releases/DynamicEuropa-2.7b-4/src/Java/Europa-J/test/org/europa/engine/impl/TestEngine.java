package org.europa.engine.impl;

import java.io.Reader;

import junit.framework.Assert;

import org.europa.engine.Engine;
import org.europa.engine.EngineComponent;
import org.europa.engine.EngineModule;
import org.europa.engine.LanguageInterpreter;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

@RunWith(Suite.class)
@Suite.SuiteClasses({
	TestEngine.Lifecycle.class
})

public class TestEngine 
{
	public static class Lifecycle
	{
		@Test
		public void TestLifecycle()
		{
			Engine engine = new EngineBase();
			Assert.assertFalse(engine.isStarted());
			
			engine.addModule(new GenericModule("module1"));
			engine.addModule(new GenericModule("module2"));
			
			engine.start();
			Assert.assertTrue(engine.isStarted());

			Assert.assertEquals(2, engine.getAllModules().size());
			Assert.assertEquals(4, engine.getAllComponents().size());
			Assert.assertEquals(2, engine.getAllInterpreters().size());
						
			engine.shutdown();
			Assert.assertFalse(engine.isStarted());
		}
	}
	
	protected static class GenericModule
		implements EngineModule
	{
		protected String name_;
		
		public GenericModule(String name)
		{
			name_ = name;
		}
		
		protected String makeComponentName(String c)
		{
			return name_+"-"+c;
		}
		
		@Override
		public String getName() { return name_; }

		@Override
		public void initialize() 
		{
		}

		@Override
		public void uninitialize() 
		{
		}

		@Override
		public void initialize(Engine e) 
		{			
			e.addComponent(new GenericComponent(makeComponentName("ConstraintEngine")));
			e.addComponent(new GenericComponent(makeComponentName("PlanDatabase")));
			e.addLanguageInterpreter("nddl", new NoopInterpreter());
			e.addLanguageInterpreter("anml", new NoopInterpreter());
		}

		@Override
		public void uninitialize(Engine e) 
		{
			e.removeComponent("ConstraintEngine");
			e.removeComponent("PlanDatabase");
			e.removeLanguageInterpreter("nddl");
			e.removeLanguageInterpreter("anml");
		}
	}
	
	protected static class GenericComponent
		implements EngineComponent
	{
		protected String name_;
		protected Engine engine_;
		
		public GenericComponent(String name)
		{
			name_ = name;
		}
		
		@Override
		public String getName() { return name_; }

		@Override
		public Engine getEngine() { return engine_; }

		@Override
		public void setEngine(Engine e) { engine_ = e; }
	}
	
	protected static class NoopInterpreter
		implements LanguageInterpreter
	{
		@Override
		public String interpret(Reader input, String inputName) 
		{
			return "noop";
		}
	}
}
