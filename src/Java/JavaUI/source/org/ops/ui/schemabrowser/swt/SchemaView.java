package org.ops.ui.schemabrowser.swt;

import org.eclipse.core.filesystem.EFS;
import org.eclipse.core.filesystem.IFileStore;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ITreeSelection;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;
import org.eclipse.ui.part.ViewPart;
import org.eclipse.ui.texteditor.ITextEditor;
import org.ops.ui.filemanager.model.AstNode;
import org.ops.ui.schemabrowser.model.SchemaNode;
import org.ops.ui.schemabrowser.model.SchemaSource;
import org.ops.ui.solver.model.SolverAdapter;
import org.ops.ui.solver.model.SolverModel;
import org.ops.ui.solver.swt.SolverModelSWT;
import org.ops.ui.solver.swt.SolverModelView;

/**
 * Europa schema browser - SWT version
 * 
 * @author Tatiana Kichkaylo
 */
public class SchemaView extends ViewPart implements SolverModelView {
	public static final String VIEW_ID = "org.ops.ui.schemabrowser.swt.SchemaView";
	private TreeViewer viewer;
	private final SchemaSource source = new SchemaSource(null);
	private SolverModel model;

	// For now reload schema only on engine start/stop. If schema changes as
	// a result of stepping, will also need to update then. Not doing it
	// now, because too lazy to restore tree expansion
	private SolverAdapter listener = new SolverAdapter() {
		@Override
		public void solverStarted() {
			reloadView();
		}

		@Override
		public void solverStopped() {
			reloadView();
		}
	};

	/** Switch this view to the given model, possibly NULL */
	@Override
	public void setModel() {
		if (this.model != null)
			this.model.removeSolverListener(listener);
		this.model = SolverModelSWT.getCurrent();
		if (model != null)
			model.addSolverListener(listener);
		reloadView();
	}

	@Override
	public void createPartControl(Composite parent) {
		viewer = new TreeViewer(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL);
		SchemaContentProvider cProvider = new SchemaContentProvider(source);
		viewer.setContentProvider(cProvider);
		viewer.setLabelProvider(new SchemaLabelProvider(parent.getFont()));
		viewer.setInput(cProvider.getRootNode());
		viewer.addDoubleClickListener(new IDoubleClickListener() {
			@Override
			public void doubleClick(DoubleClickEvent event) {
				ISelection sel = event.getSelection();
				if (!(sel instanceof ITreeSelection))
					return;
				Object ob = ((ITreeSelection) sel).getFirstElement();
				if (!(ob instanceof SchemaNode))
					return;
				AstNode ast = ((SchemaNode) ob).getAst();
				if (ast == null)
					return;

				String fileName = ast.getFileName();
				if (fileName == null)
					return;

				Path path = new Path(fileName);
				IFileStore store = EFS.getLocalFileSystem().getStore(path);
				assert (!store.fetchInfo().isDirectory() && store.fetchInfo()
						.exists());
				IWorkbenchPage page = PlatformUI.getWorkbench()
						.getActiveWorkbenchWindow().getActivePage();
				try {
					ITextEditor editor = (ITextEditor) IDE
							.openEditorOnFileStore(page, store);
					IDocument doc = editor.getDocumentProvider().getDocument(
							editor.getEditorInput());
					int offset = doc.getLineOffset(ast.getLine() - 1);
					int end = doc.getLineOffset(ast.getEndLine());
					editor.setHighlightRange(offset, end - offset + 1, true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
		setModel();
	}

	@Override
	public void setFocus() {
		viewer.getControl().setFocus();
	}

	private void reloadView() {
		source.setModel(model);
		SchemaContentProvider cProvider = (SchemaContentProvider) viewer
				.getContentProvider();
		cProvider.initialize();
		viewer.refresh();
	}
}
