package org.ops.ui.editor.swt;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.text.source.ISourceViewer;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.editors.text.TextEditor;
import org.eclipse.ui.views.contentoutline.IContentOutlinePage;

/**
 * NDDL editor with syntax highlighting.
 * 
 * @author Tatiana
 */
public class NddlEditor extends TextEditor {

	private ColorManager colorManager;
	private NddlContentProvider provider;
	private NddlOutlinePage outlinePage;

	public NddlEditor() {
		super();
		colorManager = new ColorManager();
		setSourceViewerConfiguration(new NddlConfiguration(colorManager));
		setDocumentProvider(new NddlDocumentProvider());
		provider = new NddlContentProvider(this);
		// setEditorContextMenuId("org.ops.ui.NddlEditorScope");
	}

	/**
	 * Create the part control.
	 * 
	 * @see org.eclipse.ui.IWorkbenchPart#createPartControl(org.eclipse.swt.widgets.Composite)
	 */
	@Override
	public void createPartControl(Composite parent) {
		super.createPartControl(parent);
	}

	@Override
	public void dispose() {
		colorManager.dispose();
		super.dispose();
	}

	/**
	 * Used by platform to get the OutlinePage and ProjectionSupport adapter.
	 * 
	 * @see org.eclipse.core.runtime.IAdaptable#getAdapter(java.lang.Class)
	 */
	@SuppressWarnings("unchecked")
	@Override
	public Object getAdapter(Class required) {
		if (IContentOutlinePage.class.equals(required)) {
			if (this.outlinePage == null) {
				// Outline updates itself on initialization
				this.outlinePage = new NddlOutlinePage(this);
			}
			return outlinePage;
		}
		return super.getAdapter(required);
	}
	
	public NddlContentProvider getNddlContentProvider() {
		return provider;
	}

	public NddlOutlinePage getOutlinePage() {
		return outlinePage;
	}

	/**
	 * @return The source viewer of this editor
	 */
	public ISourceViewer getViewer() {
		return getSourceViewer();
	}

	public IFile getFile() {
		IFileEditorInput input = (IFileEditorInput) this.getEditorInput();
		return input.getFile();
	}

	/** Document has been modified. Update outline, error markers, etc */
	@Override
	protected void editorSaved() {
		super.editorSaved();
		provider.reload(getFile());
	}
}
