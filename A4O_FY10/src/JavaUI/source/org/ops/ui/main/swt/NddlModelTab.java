package org.ops.ui.main.swt;

import java.io.File;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.debug.core.ILaunchConfiguration;
import org.eclipse.debug.core.ILaunchConfigurationWorkingCopy;
import org.eclipse.debug.ui.AbstractLaunchConfigurationTab;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.DirectoryDialog;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

/** Tab in a launch configuration dialog */
public class NddlModelTab extends AbstractLaunchConfigurationTab implements
		NddlConfigurationFields, ModifyListener {
	private Text fDirText;
	private Button fDirButton;
	private Text fNddlModelText;
	private Button fNddlModelButton;
	private Text fPlannerConfigText;
	private Button fPlannerConfigButton;
	private Text fHorizonStart;
	private Text fHorizonEnd;

	public void createControl(Composite parent) {
		Composite comp = new Composite(parent, SWT.NONE);
		setControl(comp);
		// PlatformUI.getWorkbench().getHelpSystem().setHelp(getControl(),
		// IDebugHelpContextIds.LAUNCH_CONFIGURATION_DIALOG_COMMON_TAB);
		comp.setLayout(new GridLayout(3, false));
		comp.setFont(parent.getFont());

		new Label(comp, SWT.NONE).setText("Root directory");
		fDirText = createSingleText(comp, 1);
		fDirText.addModifyListener(this);
		fDirButton = createPushButton(comp, "Browse", null);
		fDirButton.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent evt) {
				handleDirectoryButtonSelected(fDirText,
						"Select base directory for model execution");
			}
		});
		new Label(comp, SWT.NONE).setText("Nddl model file");
		fNddlModelText = createSingleText(comp, 1);
		fNddlModelText.addModifyListener(this);
		fNddlModelButton = createPushButton(comp, "Browse", null);
		fNddlModelButton.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent evt) {
				handleLocationButtonSelected(fNddlModelText,
						"Select NDDL file to load", new String[] { "nddl" });
			}
		});
		new Label(comp, SWT.NONE).setText("Planner config");
		fPlannerConfigText = createSingleText(comp, 1);
		fPlannerConfigText.addModifyListener(this);
		fPlannerConfigButton = createPushButton(comp, "Browse", null);
		fPlannerConfigButton.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent evt) {
				handleLocationButtonSelected(fPlannerConfigText,
						"Select planner config file", new String[] { "xml" });
			}
		});
		new Label(comp, SWT.NONE).setText("Horizon start");
		fHorizonStart = createSingleText(comp, 2);
		fHorizonStart.addModifyListener(this);
		new Label(comp, SWT.NONE).setText("Horizon end");
		fHorizonEnd = createSingleText(comp, 2);
		fHorizonEnd.addModifyListener(this);
	}
	
	public void modifyText(ModifyEvent e) {
		this.updateLaunchConfigurationDialog();
	}
	
	private Text createSingleText(Composite parent, int hspan) {
		Text t = new Text(parent, SWT.SINGLE | SWT.BORDER);
		t.setFont(parent.getFont());
		GridData gd = new GridData(GridData.FILL_HORIZONTAL);
		gd.horizontalSpan = hspan;
		t.setLayoutData(gd);
		return t;
	}

	private void handleLocationButtonSelected(Text field, String title,
			String[] ext) {
		String dir = fDirText.getText();
		String value = field.getText();
		File file = new File(dir, value);
		FileDialog dialog = new FileDialog(getShell());
		dialog.setFileName(file.getAbsolutePath());
		dialog.setText(title);
		// dialog.setFilterExtensions(ext);
		value = dialog.open();
		if (value != null) {
			if (value.startsWith(dir))
				value = value.substring(dir.length() + 1);
			field.setText(value);
		}
	}

	private void handleDirectoryButtonSelected(Text field, String title) {
		String value = field.getText();
		DirectoryDialog dialog = new DirectoryDialog(getShell());
		dialog.setFilterPath(value);
		// dialog.setFileName(value);
		dialog.setText(title);
		value = dialog.open();
		if (value != null) {
			field.setText(value);
		}
	}

	public String getName() {
		return "Input files";
	}

	public void initializeFrom(ILaunchConfiguration configuration) {
		try {
			fDirText.setText(configuration.getAttribute(DIR_NAME, ""));
			fNddlModelText.setText(configuration.getAttribute(MODEL_NAME, ""));
			fPlannerConfigText.setText(configuration.getAttribute(CONFIG_NAME,
					""));
			fHorizonStart.setText(String.valueOf(configuration.getAttribute(
					HORIZON_START, DEF_HORIZON_START)));
			fHorizonEnd.setText(String.valueOf(configuration.getAttribute(
					HORIZON_END, DEF_HORIZON_END)));
		} catch (CoreException e) {
			EuropaPlugin.getDefault().logError(
					"Cannot initialize nddl model tab", e);
			e.printStackTrace();
		}
	}

	public void performApply(ILaunchConfigurationWorkingCopy configuration) {
		configuration.setAttribute(DIR_NAME, fDirText.getText());
		configuration.setAttribute(MODEL_NAME, fNddlModelText.getText());
		configuration.setAttribute(CONFIG_NAME, fPlannerConfigText.getText());

		configuration.setAttribute(HORIZON_START, makeInt(fHorizonStart,
				DEF_HORIZON_START));
		configuration.setAttribute(HORIZON_END, makeInt(fHorizonEnd,
				DEF_HORIZON_END));
	}

	private int makeInt(Text field, int def) {
		try {
			return new Integer(field.getText());
		} catch (NumberFormatException e) {
			return def;
		}
	}

	public void setDefaults(ILaunchConfigurationWorkingCopy configuration) {
		configuration.setAttribute(CONFIG_NAME, DEFAULT_PLANNER_CONFIG);
		configuration.setAttribute(HORIZON_START, DEF_HORIZON_START);
		configuration.setAttribute(HORIZON_END, DEF_HORIZON_END);
	}

	@Override
	public boolean isValid(ILaunchConfiguration launchConfig) {
		// Have dir name?
		String name = fDirText.getText();
		File dir = new File(name);
		if (!dir.exists()) {
			setErrorMessage("Directory " + name + " is not found");
			return false;
		}
		if (!dir.isDirectory()) {
			setErrorMessage(name + " is not a directory");
			return false;
		}
		// Have valid model name? Just check the name, not the contents
		name = fNddlModelText.getText();
		File file = new File(dir, name);
		if (!file.exists()) {
			setErrorMessage("Model file " + name + " does not exist");
			return false;
		}

		// Planner config?
		name = fPlannerConfigText.getText();
		file = new File(dir, name);
		if (!file.exists()) {
			setErrorMessage("Planner config " + name + " does not exist");
			return false;
		}

		// Horizon bounds
		int start, end;
		name = fHorizonStart.getText();
		try {
			start = new Integer(name);
		} catch (NumberFormatException ex) {
			setErrorMessage("Horizon start should be an integer");
			return false;
		}
		name = fHorizonEnd.getText();
		try {
			end = new Integer(name);
		} catch (NumberFormatException ex) {
			setErrorMessage("Horizon end should be an integer");
			return false;
		}
		if (start >= end) {
			setErrorMessage("Horizon end should be greater than horizon start");
			return false;
		}

		setErrorMessage(null);
		return true;
	}
}
