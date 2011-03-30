package org.ops.ui.solver.swt;

import java.util.HashMap;
import java.util.logging.Level;

import org.eclipse.core.resources.IResource;
import org.eclipse.debug.core.DebugException;
import org.eclipse.debug.core.ILaunch;
import org.eclipse.debug.core.model.IProcess;
import org.eclipse.debug.core.model.IStreamsProxy;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IViewPart;
import org.eclipse.ui.IViewReference;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PlatformUI;
import org.ops.ui.gantt.swt.DetailsView;
import org.ops.ui.solver.model.SolverModel;

/**
 * This is Eclipse extension of the common SolverModel. This version of solver
 * model implements IProcess, so that it can be started and stopped through
 * standard Launch mechanism. The IProcess stuff is separated into this class to
 * avoid dependency of Swing code on Eclipse libraries.
 * 
 * @author tatiana
 */
public class SolverModelSWT extends SolverModel implements IProcess {
	/** Launch this model belongs to */
	private ILaunch launch;
	/** The map of attributes for IProcess */
	private HashMap<String, String> attributes = new HashMap<String, String>();
	/**
	 * Various Eclipse views may use this static variable to locate the current
	 * model everybody is looking at
	 */
	private static SolverModelSWT current = null;

	public SolverModelSWT(ILaunch launch) {
		this.launch = launch;
	}

	public static SolverModelSWT getCurrent() {
		return current;
	}

	@Override
	public Object getAdapter(@SuppressWarnings("rawtypes") Class adapter) {
		if (adapter.equals(ILaunch.class))
			return launch;
		// This means someone pointed to this configuration in the debug view.
		// We do not actually mean to return an adapter, just flip models.
		if (adapter.equals(IResource.class))
			SolverModelSWT.makeActive(this);
		log.log(Level.FINER, "Called getAdapter for {0}", adapter);
		return null;
	}

	protected final void doTerminate() {
		super.terminate();
	}

	/**
	 * Wrap a perfectly good terminate method to make it thread safe for Debug
	 * view
	 */
	@Override
	public synchronized void terminate() {
		Display.getDefault().asyncExec(new Runnable() {
			@Override
			public void run() {
				doTerminate();
			}
		});
	}

	@Override
	/** If running, can terminate */
	public boolean canTerminate() {
		return !isTerminated();
	}

	@Override
	public String getLabel() {
		return "Europa engine";
	}

	@Override
	public ILaunch getLaunch() {
		return launch;
	}

	@Override
	public IStreamsProxy getStreamsProxy() {
		// The engine has no streams, so no proxy
		log.log(Level.FINEST, "SolverModel.getStreamsProxy");
		return null;
	}

	@Override
	public void setAttribute(String key, String value) {
		this.attributes.put(key, value);
	}

	@Override
	public String getAttribute(String key) {
		return this.attributes.get(key);
	}

	@Override
	/** For now, always return 0 as return value */
	public int getExitValue() throws DebugException {
		return 0;
	}

	/** Make this model active and notify all views */
	public static void makeActive(SolverModelSWT model) {
		IWorkbench workbench = PlatformUI.getWorkbench();
		IWorkbenchPage page = workbench.getActiveWorkbenchWindow()
				.getActivePage();

		current = model;
		IViewReference[] vrefs = page.getViewReferences();
		for (int i = 0; i < vrefs.length; i++) {
			IViewPart v = vrefs[i].getView(false);
			if (v != null && v instanceof SolverModelView) {
				((SolverModelView) v).setModel();
			}
			if (v != null && v instanceof DetailsView) {
				((DetailsView) v).setModel();
			}
		}
	}
}
