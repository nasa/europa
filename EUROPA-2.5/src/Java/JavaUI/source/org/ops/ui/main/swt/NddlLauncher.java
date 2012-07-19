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
import org.eclipse.ui.PlatformUI;
import org.ops.ui.solver.model.SolverModel;
import org.ops.ui.solver.swt.SolverView;

public class NddlLauncher implements ILaunchConfigurationDelegate,
		NddlConfigurationFields {
	private final static String ERROR_TITLE = "Problem launching";

	public void launch(ILaunchConfiguration configuration, String mode,
			ILaunch launch, IProgressMonitor monitor) throws CoreException {
		System.out.println("Launcher called " + configuration);

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

			public void run() {
				try {
					// Activate perspective
					IWorkbench workbench = PlatformUI.getWorkbench();
					workbench.showPerspective(NddlRunPerspective.PESPECTIVE_ID,
							workbench.getActiveWorkbenchWindow());

					// Close whatever was opened in SolverModel
					SolverModel model = EuropaPlugin.getDefault()
							.getSolverModel();
					if (model.isConfigured())
						model.shutdown();

					// Set new data
					SolverView view = (SolverView) workbench
							.getActiveWorkbenchWindow().getActivePage()
							.showView(SolverView.VIEW_ID);
					view.setFields(nddl, config, horizonStart, horizonEnd);
					model.configure(nddl, config, horizonStart, horizonEnd);
				} catch (Throwable e) {
					MessageDialog.openError(null, ERROR_TITLE, e.getMessage());
				}
			}
		});
	}
}
