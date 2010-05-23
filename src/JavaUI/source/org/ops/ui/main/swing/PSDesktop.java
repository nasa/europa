package org.ops.ui.main.swing;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.swing.JDesktopPane;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.UIManager;
import javax.swing.WindowConstants;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

// Old version import org.ops.ui.gantt.swing.EGanttView;
import org.ops.ui.beanshell.swing.BeanShellView;
import org.ops.ui.gantt.swing.GanttView;
import org.ops.ui.schemabrowser.swing.SchemaView;
import org.ops.ui.solver.model.SolverModel;
import org.ops.ui.solver.swing.OpenDecisionsView;
import org.ops.ui.solver.swing.PSSolverDialog;

import psengine.PSUtil;

public class PSDesktop extends JFrame {

	protected File workingDir;
	private JDesktopPane desktop;
	private Logger log = Logger.getLogger(getClass().getName());

	// Solver model. It has engine inside
	private SolverModel solverModel;

	// Viewer
	private SchemaView schemaBrowser;
	private PSSolverDialog solverDialog;
	private OpenDecisionsView openDecisions;
	// private EGanttView ganttView;
	private GanttView ganttView;
	private BeanShellView bshView;

	private JMenu userMenu = null;

	private PSDesktop(File dataFile, File solverConfig, File workingDir) {
		assert (workingDir != null);
		this.workingDir = workingDir;
		this.desktop = new JDesktopPane();
		this.add(this.desktop);

		// Closing behavior. Add a question dialog?
		this.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				askAndExit();
			}
		});

		// Hook up engine
		Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {
			public void run() {
				releaseEngine();
			}
		}));

		hookupEngine(dataFile, solverConfig);

		// Build views
		this.schemaBrowser = new SchemaView(this.solverModel);
		this.desktop.add(this.schemaBrowser);

		this.solverDialog = new PSSolverDialog(this.solverModel);
		this.solverDialog.setVisible(true);
		this.desktop.add(this.solverDialog);

		this.openDecisions = new OpenDecisionsView(this.solverModel);
		this.desktop.add(this.openDecisions);

		// this.ganttView = new EGanttView(this.solverModel);
		this.ganttView = new GanttView(this.solverModel);
		this.desktop.add(this.ganttView);

		this.bshView = new BeanShellView(solverModel, this);
		this.desktop.add(this.bshView);

		// Finish up
		buildMenu();
		updateTitle(dataFile);
		this.setSize(600, 700);
	}

	protected void askAndExit() {
		if (JOptionPane.showConfirmDialog(this, "Close this application?",
				"Exit", JOptionPane.YES_NO_OPTION) == JOptionPane.OK_OPTION) {
			System.exit(0);
		}
	}

	private void buildMenu() {
		JMenuBar bar = new JMenuBar();
		this.setJMenuBar(bar);
		JMenu menu;

		menu = new JMenu("File");
		bar.add(menu);
		JMenuItem item = new JMenuItem("Load BSH file");
		menu.add(item);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				File fl = askForFile(workingDir, "Bean Shell files", "bsh",
						"Choose BSH file to interpret");
				if (fl != null)
					bshView.loadFile(fl);
			}
		});
		menu.addSeparator();
		item = new JMenuItem("Exit");
		menu.add(item);
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				askAndExit();
			}
		});

		menu = new JMenu("Windows");
		bar.add(menu);
		menu.add(this.schemaBrowser.getToggleMenuItem());
		menu.add(this.solverDialog.getToggleMenuItem());
		menu.add(this.openDecisions.getToggleMenuItem());
		menu.add(this.ganttView.getToggleMenuItem());
		menu.add(this.bshView.getToggleMenuItem());
	}

	private void updateTitle(File dataFile) {
		final String prefix = "Europa desktop";
		this.setTitle(prefix + ": " + dataFile);
	}

	/**
	 * Pops up a file chooser dialog to open file. @return full path to the
	 * chosen file, or null
	 */
	private static File askForFile(File baseDir, String typeTitle,
			String typeExtension, String dialogTitle) {
		JFileChooser chooser = new JFileChooser();
		FileFilter filter = new FileNameExtensionFilter(typeTitle,
				typeExtension);
		chooser.setCurrentDirectory(baseDir);
		chooser.addChoosableFileFilter(filter);
		chooser.setFileFilter(filter);
		chooser.setDialogTitle(dialogTitle);
		int res = chooser.showOpenDialog(null);
		if (res != JFileChooser.APPROVE_OPTION)
			return null;
		File file = chooser.getSelectedFile();
		return file;
	}

	/** Create and connect PSEngine */
	protected void hookupEngine(File data, File solverConfig) {
		String debugMode = "g";
		try {
			PSUtil.loadLibraries(debugMode);
		} catch (UnsatisfiedLinkError e) {
			log
					.log(
							Level.SEVERE,
							"Cannot load Europa libraries. Please make the "
									+ "dynamic libraries are included in LD_LIBRARY_PATH "
									+ "(or PATH for Windows)", e);
			System.exit(1);
		}

		// loadFile(dataFile);

		solverModel = new SolverModel();
		if (data != null && solverConfig != null)
			solverModel.configure(data, solverConfig, 0, 100);

		log.log(Level.INFO, "Engine started");
	}

	/** Shutdown and release engine. Save any state if necessary */
	protected synchronized void releaseEngine() {
		if (solverModel != null) {
			solverModel.shutdown();
			solverModel = null;
			log.log(Level.INFO, "Engine released");
		}
	}

	private static File tryToSetFile(String kind, String name, File old) {
		File f = new File(name);
		if (f.exists())
			return f;
		System.err.println(kind + " file " + f + " is not found");
		return old;
	}

	public EuropaInternalFrame makeNewFrame(String title) {
		EuropaInternalFrame res = new EuropaInternalFrame(title);
		this.desktop.add(res);
		// Hook up to menu
		if (userMenu == null) {
			userMenu = new JMenu("User");
			this.getJMenuBar().add(userMenu);
		}
		userMenu.add(res.getToggleMenuItem());
		return res;
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception ex) {
			System.out.println("Unable to load native look and feel");
		}

		// Parse command line parameters to get data file and configs
		File dataFile = null, solverConfig = null, bshFile = null;
		for (int i = 0; i < args.length; i++) {
			if ("-config".equals(args[i])) {
				solverConfig = tryToSetFile("Config", args[++i], solverConfig);
			} else if ("-nddl".equals(args[i])) {
				dataFile = tryToSetFile("NDDL", args[++i], dataFile);
			} else if ("-bsh".equals(args[i])) {
				bshFile = tryToSetFile("BSH", args[++i], bshFile);
			} else {
				if (dataFile == null)
					dataFile = tryToSetFile("NDDL", args[i], dataFile);
				else if (solverConfig == null)
					solverConfig = tryToSetFile("Config", args[i], solverConfig);
				else
					bshFile = tryToSetFile("BSH", args[i], bshFile);
			}
		}

		// Pop up dialogs if needed
		File baseDir;
		if (dataFile != null)
			baseDir = dataFile.getParentFile();
		else if (solverConfig != null)
			baseDir = solverConfig.getParentFile();
		else
			baseDir = new File(".");
		if (dataFile == null && bshFile == null) {
			dataFile = askForFile(baseDir, "NDDL files", "nddl",
					"Choose NDDL initial state file");
			if (dataFile == null) {
				JOptionPane.showMessageDialog(null,
						"No NDDL file chosen. Exiting");
				return;
			}
			baseDir = dataFile.getParentFile();
		}
		if (solverConfig == null && bshFile == null) {
			solverConfig = askForFile(baseDir, "XML files", "xml",
					"Choose PlannerConfig.xml file");
			if (solverConfig == null) {
				JOptionPane.showMessageDialog(null,
						"No solver config chosen. Exiting");
				return;
			}
		}

		PSDesktop me = new PSDesktop(dataFile, solverConfig, baseDir);
		if (bshFile != null)
			me.bshView.loadFile(bshFile);
		me.setVisible(true);
	}

}
