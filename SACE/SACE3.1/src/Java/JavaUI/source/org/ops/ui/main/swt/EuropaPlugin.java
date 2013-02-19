package org.ops.ui.main.swt;

import java.util.HashMap;

import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.resource.ImageRegistry;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.ui.plugin.AbstractUIPlugin;
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

	/** Map of color resources */
	private HashMap<RGB, Color> colorMap;

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

		colorMap = new HashMap<RGB, Color>();

		// Load the libraries or die
		try {
			PSUtil.loadLibraries();
		} catch (UnsatisfiedLinkError e) {
			logError("Cannot load Europa libraries. Please make the "
					+ "dynamic libraries are included in LD_LIBRARY_PATH "
					+ "(or PATH for Windows)\n" + e.getLocalizedMessage());
			throw e;
		}
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
		for (Color c : colorMap.values())
			c.dispose();
		colorMap = null;
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
		reg.put(CommonImages.IMAGE_START,
				getImageDescriptor(CommonImages.IMAGE_START));
		reg.put(CommonImages.IMAGE_STOP,
				getImageDescriptor(CommonImages.IMAGE_STOP));
	}

	public void logError(String message, Throwable exception) {
		this.getLog().log(
				new Status(IStatus.ERROR, PLUGIN_ID, message, exception));
	}

	public Color getColor(RGB rgb) {
		Color r = colorMap.get(rgb);
		if (r == null) {
			r = new Color(null, rgb);
			colorMap.put(rgb, r);
		}
		return r;
	}
}
