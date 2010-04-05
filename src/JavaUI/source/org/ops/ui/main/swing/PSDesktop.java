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
import javax.swing.filechooser.FileNameExtensionFilter;

// Old version import org.ops.ui.gantt.swing.EGanttView;
import org.ops.ui.gantt.swing.GanttView;
import org.ops.ui.schemabrowser.swing.SchemaView;
import org.ops.ui.solver.model.SolverModel;
import org.ops.ui.solver.swing.OpenDecisionsView;
import org.ops.ui.solver.swing.PSSolverDialog;

import psengine.PSUtil;

public class PSDesktop extends JFrame {

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

	private PSDesktop(File dataFile, File solverConfig) {
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
		JMenuItem item = new JMenuItem("Exit");
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
	}

	private void updateTitle(File dataFile) {
		final String prefix = "Europa desktop";
		this.setTitle(prefix + ": " + dataFile);
	}

	/**
	 * Pops up a file chooser dialog to open .nddl file. @return full path to
	 * the chosen file, or null
	 */
	private static File askForNddlFile() {
		JFileChooser chooser = new JFileChooser();
		chooser.addChoosableFileFilter(new FileNameExtensionFilter(
				"NDDL files", "nddl"));
		int res = chooser.showOpenDialog(null);
		if (res != JFileChooser.APPROVE_OPTION)
			return null;
		File file = chooser.getSelectedFile();
		return file;
	}

	/**
	 * Pops up a file chooser dialog to open .xml file. @return full path to the
	 * chosen file, or null
	 */
	private static File askForXmlFile(String title) {
		JFileChooser chooser = new JFileChooser(title);
		chooser.addChoosableFileFilter(new FileNameExtensionFilter("XML files",
				"xml"));
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
		File dataFile = null, solverConfig = null;
		for (int i = 0; i < args.length; i++) {
			if ("-config".equals(args[i])) {
				solverConfig = tryToSetFile("Config", args[++i], solverConfig);
			} else if ("-nddl".equals(args[i])) {
				dataFile = tryToSetFile("NDDL", args[++i], dataFile);
			} else {
				if (dataFile == null)
					dataFile = tryToSetFile("NDDL", args[i], dataFile);
				else
					solverConfig = tryToSetFile("Config", args[i], solverConfig);
			}
		}

		// Pop up dialogs if needed
		if (dataFile == null) {
			dataFile = askForNddlFile();
			if (dataFile == null) {
				JOptionPane.showMessageDialog(null,
						"No NDDL file chosen. Exiting");
				return;
			}
		}
		if (solverConfig == null) {
			solverConfig = askForXmlFile("blah");
			if (solverConfig == null) {
				JOptionPane.showMessageDialog(null,
						"No solver config chosen. Exiting");
				return;
			}
		}

		PSDesktop me = new PSDesktop(dataFile, solverConfig);
		me.setVisible(true);
	}

}
