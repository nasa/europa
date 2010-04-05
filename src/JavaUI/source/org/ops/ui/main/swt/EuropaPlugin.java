package org.ops.ui.main.swt;

import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.resource.ImageRegistry;
import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.ops.ui.solver.model.SolverModel;
import org.osgi.framework.BundleContext;

import psengine.PSUtil;

/**
 * The activator class controls the plug-in life cycle
 * 
 * @author Tatiana Kichkaylo
 */
public class EuropaPlugin extends AbstractUIPlugin {

	/** The plug-in ID */
	public static final String PLUGIN_ID = "org.ops.ui.europaplugin";

	/** The shared instance */
	private static EuropaPlugin plugin;

	/**
	 * Solver execution model. Should be one per plugin. Creates engine instance
	 * inside
	 */
	private SolverModel solverModel;

	/**
	 * The constructor
	 */
	public EuropaPlugin() {
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * org.eclipse.ui.plugin.AbstractUIPlugin#start(org.osgi.framework.BundleContext
	 * )
	 */
	@Override
	public void start(BundleContext context) throws Exception {
		super.start(context);
		plugin = this;

		hookupEngine();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * org.eclipse.ui.plugin.AbstractUIPlugin#stop(org.osgi.framework.BundleContext
	 * )
	 */
	@Override
	public void stop(BundleContext context) throws Exception {
		plugin = null;
		releaseEngine();
		super.stop(context);
	}

	/**
	 * Returns the shared instance
	 * 
	 * @return the shared instance
	 */
	public static EuropaPlugin getDefault() {
		return plugin;
	}

	/**
	 * Returns an image descriptor for the image file at the given plug-in
	 * relative path
	 * 
	 * @param path
	 *            the path
	 * @return the image descriptor
	 */
	public static ImageDescriptor getImageDescriptor(String path) {
		return imageDescriptorFromPlugin(PLUGIN_ID, path);
	}

	/** Create and connect PSEngine */
	protected void hookupEngine() {
		String debugMode = "g";
		try {
			PSUtil.loadLibraries(debugMode);
		} catch (UnsatisfiedLinkError e) {
			logError("Cannot load Europa libraries. Please make the "
					+ "dynamic libraries are included in LD_LIBRARY_PATH "
					+ "(or PATH for Windows)\n" + e.getLocalizedMessage());
			throw e;
		}
	}

	public void logError(String message) {
		this.getLog().log(new Status(IStatus.ERROR, PLUGIN_ID, message));
	}

	@Override
	public void initializeImageRegistry(ImageRegistry reg) {
		super.initializeImageRegistry(reg);
		reg.put(CommonImages.IMAGE_EUROPA,
				getImageDescriptor(CommonImages.IMAGE_EUROPA));
		reg.put(CommonImages.IMAGE_RUN,
				getImageDescriptor(CommonImages.IMAGE_RUN));
		reg.put(CommonImages.IMAGE_HORIZON,
				getImageDescriptor(CommonImages.IMAGE_HORIZON));
	}

	public void logError(String message, Throwable exception) {
		this.getLog().log(
				new Status(IStatus.ERROR, PLUGIN_ID, message, exception));
	}

	/** Shutdown and release engine. Save any state if necessary */
	protected void releaseEngine() {
		if (solverModel != null)
			solverModel.shutdown();
		solverModel = null;
	}

	public SolverModel getSolverModel() {
		if (solverModel == null)
			solverModel = new SolverModel();
		return solverModel;
	}
}
