package org.ops.ui.main.swt;

import java.io.File;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.debug.core.ILaunch;
import org.eclipse.debug.core.ILaunchConfiguration;
import org.eclipse.debug.core.model.ILaunchConfigurationDelegate;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PlatformUI;
import org.ops.ui.solver.swt.SolverModelSWT;
import org.ops.ui.solver.swt.SolverView;

public class NddlLauncher implements ILaunchConfigurationDelegate,
		NddlConfigurationFields {
	private final static String ERROR_TITLE = "Problem launching";

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
					model.start();

					// Set new data
					IWorkbenchPage page = workbench.getActiveWorkbenchWindow()
							.getActivePage();
					SolverView view = (SolverView) page
							.showView(SolverView.VIEW_ID);
					view.updateLaunchList();
					SolverModelSWT.makeActive(model);
				} catch (Throwable e) {
					MessageDialog.openError(null, ERROR_TITLE, e.getMessage());
				}
			}
		});
	}
}
