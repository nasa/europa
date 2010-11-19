package org.ops.ui.main.swt;

import java.io.File;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.debug.core.DebugPlugin;
import org.eclipse.debug.core.ILaunchConfiguration;
import org.eclipse.debug.core.ILaunchConfigurationType;
import org.eclipse.debug.core.ILaunchConfigurationWorkingCopy;
import org.eclipse.debug.ui.ILaunchShortcut;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IFileEditorInput;

/**
 * Launch shortcut to appear in "Run as" menus
 * 
 * @author Tatiana Kichkaylo
 */
public class NddlLaunchShortcut implements ILaunchShortcut,
		NddlConfigurationFields {
	public void launch(ISelection selection, String mode) {
		if (!(selection instanceof IStructuredSelection)) {
			System.out.println("Not structured selection " + selection);
			return;
		}
		IStructuredSelection ssel = (IStructuredSelection) selection;
		if (ssel.size() != 1) {
			System.out.println("Expected 1 element, got " + ssel.size());
			return;
		}
		if (!(ssel.getFirstElement() instanceof IFile)) {
			System.out.println("Expected IFile, got "
					+ ssel.getFirstElement().getClass().getName());
			return;
		}
		launchFile((IFile) ssel.getFirstElement(), mode);
	}

	public void launch(IEditorPart editor, String mode) {
		if (!(editor.getEditorInput() instanceof IFileEditorInput)) {
			MessageDialog.openError(null, null,
					"Cannot run NDDL model - not a file editor");
			return;
		}

		IFile file = ((IFileEditorInput) editor.getEditorInput()).getFile();
		launchFile(file, mode);
	}

	private void launchFile(IFile file, String mode) {
		try {
			ILaunchConfiguration conf = findConfiguration(file);
			if (conf == null) {
				ILaunchConfigurationType type = DebugPlugin.getDefault()
						.getLaunchManager().getLaunchConfigurationType(
								CONF_TYPE_ID);
				ILaunchConfigurationWorkingCopy wc = type.newInstance(null,
						DebugPlugin.getDefault().getLaunchManager()
								.generateUniqueLaunchConfigurationNameFrom(
										file.getName()));
				wc.setAttribute(DIR_NAME, file.getParent().getLocation()
						.toString());
				wc.setAttribute(MODEL_NAME, file.getName());
				wc.setAttribute(CONFIG_NAME, DEFAULT_PLANNER_CONFIG);
				conf = wc.doSave();
			}
			conf.launch(mode, null);
		} catch (CoreException e) {
			MessageDialog.openError(null, null, e.getMessage());
		}
	}

	/** Find configuration for the given NDDL file. If any */
	private ILaunchConfiguration findConfiguration(IFile file) {
		try {
			ILaunchConfiguration[] all = DebugPlugin.getDefault()
					.getLaunchManager().getLaunchConfigurations();
			for (ILaunchConfiguration c : all) {
				if (!c.hasAttribute(DIR_NAME) || !c.hasAttribute(MODEL_NAME))
					continue;

				File local = new File(c.getAttribute(DIR_NAME, (String) null),
						c.getAttribute(MODEL_NAME, (String) null));
				if (local.equals(file.getLocation().toFile()))
					return c;
			}
		} catch (CoreException e) {
			MessageDialog.openError(null, null, e.getMessage());
		}
		return null;
	}
}
