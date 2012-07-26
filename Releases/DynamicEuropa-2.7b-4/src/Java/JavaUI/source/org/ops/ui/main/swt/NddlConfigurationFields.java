package org.ops.ui.main.swt;

/**
 * Attribute names in a launch configuration for NDDL models
 * 
 * @author Tatiana Kichkaylo
 */
public interface NddlConfigurationFields {
	/** Id of configuration type in plugin.xml */
	public static final String CONF_TYPE_ID = "EuropaPlugin.launch.nddl.configurationType";
	/** Default horizon boundaries */
	public static final int DEF_HORIZON_START = 0;
	public static final int DEF_HORIZON_END = 100;

	/* Field names */
	public static final String DIR_NAME = "Europa.launcher.dirName";
	public static final String MODEL_NAME = "Europa.launcher.modelName";
	public static final String CONFIG_NAME = "Europa.launcher.plannerConfig";
	public static final String HORIZON_START = "Europa.launcher.horizonStart";
	public static final String HORIZON_END = "Europa.launcher.horizonEnd";

	/** Default name of planner configuration file */
	public static final String DEFAULT_PLANNER_CONFIG = "PlannerConfig.xml";
}
