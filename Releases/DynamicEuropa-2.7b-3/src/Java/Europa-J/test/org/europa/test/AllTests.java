package org.europa.test;

import org.junit.BeforeClass;
import org.junit.AfterClass;
import org.junit.runners.Suite;
import org.junit.runner.RunWith;

import org.apache.log4j.PropertyConfigurator;
import org.europa.engine.impl.TestEngine;

@RunWith(Suite.class)
@Suite.SuiteClasses({
	TestEngine.class
})

public class AllTests 
{
	@BeforeClass
	public static void setUp()
	{
		String log4jConfig="test/AllTests.log4j.properties";
		PropertyConfigurator.configure(log4jConfig);
	}

	@AfterClass
	public static void tearDown()
	{		
	}
}
