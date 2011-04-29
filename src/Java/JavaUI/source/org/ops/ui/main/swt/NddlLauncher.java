package org.ops.ui.main.swt;

import java.io.File;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.debug.core.ILaunch;
import org.eclipse.debug.core.ILaunchConfiguration;
import org.eclipse.debug.core.model.ILaunchConfigurationDelegate;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IViewReference;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PlatformUI;
import org.ops.ui.solver.swt.SolverModelSWT;
import org.ops.ui.solver.swt.SolverView;

public class NddlLauncher implements ILaunchConfigurationDelegate,
		NddlConfigurationFields {
	private final static String ERROR_TITLE = "Problem launching";
	/**
	 * Ugly hack to enable loading of configuration without starting it when
	 * restoring Solver view. Set in SolverView and then restored in launch of
	 * this class. Restoring here, because the actual start happens in UI
	 * thread, but which time SolverView restoration has long since exited. See
	 * issue 115.
	 */
	public static boolean startModel = true;

	@Override
	public void launch(ILaunchConfiguration configuration, String mode,
			final ILaunch launch, IProgressMonitor monitor)
			throws CoreException {
		// System.out.println("Launcher called " + configuration);

		File dir = new File(configuration.getAttribute(DIR_NAME, (String) null));
		final File nddl = new File(dir, configuration.getAttribute(MODEL_NAME,
				(String) null));
		final File config = new File(dir, configuration.getAttribute(
				CONFIG_NAME, (String) null));
		final int horizonStart = configuration.getAttribute(HORIZON_START,
				DEF_HORIZON_START);
		final int horizonEnd = configuration.getAttribute(HORIZON_END,
				DEF_HORIZON_END);

		if (!nddl.exists()) {
			MessageDialog.openError(null, ERROR_TITLE, "File does not exist "
					+ nddl);
			return;
		}
		if (!config.exists()) {
			MessageDialog.openError(null, ERROR_TITLE, "File does not exist "
					+ config);
			return;
		}

		Display.getDefault().asyncExec(new Runnable() {
			@Override
			public void run() {
				try {
					// Activate perspective
					IWorkbench workbench = PlatformUI.getWorkbench();
					workbench.showPerspective(NddlRunPerspective.PESPECTIVE_ID,
							workbench.getActiveWorkbenchWindow());

					// Create new model for the launch
					SolverModelSWT model = new SolverModelSWT(launch);
					model.configure(nddl, config, horizonStart, horizonEnd);
					launch.addProcess(model);
					// Start the model unless asked otherwise
					if (startModel)
						model.start();
					else
						startModel = true; // restore after used once

					// Set new data
					IWorkbenchPage page = workbench.getActiveWorkbenchWindow()
							.getActivePage();

					// In case the application has one (or more) versions of SolverView,
					// don't assume it's the SolverView class itself we need to update:
					// (TBS:  This seems messy to me, but without it, the Launches drop-down
					// isn't populated for a class I have that extends SolverView)
					// TODO:  Instead, we probably want to have listeners for launch updates
					boolean foundOne = false;
					IViewReference[] refs = page.getViewReferences();
					for(IViewReference ref: refs) {
						if(ref.getView(false) != null &&
								ref.getView(false) instanceof SolverView) {
								((SolverView) ref.getView(false)).updateLaunchList();
								foundOne = true;
						}
					}

					// If none exist, show first and then update launch:
					if(!foundOne) {
						SolverView view = (SolverView) page
							.showView(SolverView.VIEW_ID);
						view.updateLaunchList();
					}
					SolverModelSWT.makeActive(model);
				} catch (Throwable e) {
					MessageDialog.openError(null, ERROR_TITLE, e.getMessage());
				}
			}
		});
	}
}
