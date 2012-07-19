package org.ops.ui.editor.swt;

import org.eclipse.jface.text.BadLocationException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.Position;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerFilter;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.ui.views.contentoutline.ContentOutlinePage;
import org.ops.ui.editor.model.OutlineNode;
import org.ops.ui.filemanager.model.AstNode;

public class NddlOutlinePage extends ContentOutlinePage {

	private NddlEditor editor;

	private OutlineNode input = null;

	public NddlOutlinePage(NddlEditor nddlEditor) {
		this.editor = nddlEditor;
	}

	/**
	 * Creates the control ie. creates all the stuff that matters and is visible
	 * in the outline.
	 * 
	 * Actions must be created before menus and toolbars.
	 * 
	 * @param parent
	 */
	@Override
	public void createControl(Composite parent) {
		super.createControl(parent);
		NddlContentProvider provider = editor.getNddlContentProvider();
		
		// initialize the tree viewers
		TreeViewer viewer = getTreeViewer();
		viewer.setContentProvider(provider);
		viewer.addFilter(new FileViewerFilter());
		viewer.setLabelProvider(new NddlLabelProvider());

		// set the selection listener
		viewer.addSelectionChangedListener(this);

		// finally set the input
		if (this.input != null)
			viewer.setInput(this.input);
		else
			provider.reload(editor.getFile());
	}

	class FileViewerFilter extends ViewerFilter {
		@Override
		public boolean select(Viewer viewer, Object parentElement,
				Object element) {
			if (element instanceof OutlineNode) {
				OutlineNode n = (OutlineNode) element;
				// No file name is either error (type=0) or something like NDDL
				if (n.getFileName() == null)
					return n.getType() != null;
				return n.getFileName().endsWith(getFileName());
			}
			return false;
		}
	}

	public void update(OutlineNode input) {
		this.input = input;

		TreeViewer viewer = getTreeViewer();
		if (viewer != null) {

			Control control = viewer.getControl();
			if (control != null && !control.isDisposed()) {
				control.setRedraw(false);
				// save viewer state
				// ISelection selection = viewer.getSelection();
				viewer.getTree().deselectAll();

				Object[] expandedElements = viewer.getExpandedElements();

				/*
				 * ArrayList oldNodes = new ArrayList(expandedElements.length);
				 * for (int i = 0; i < expandedElements.length; i++) {
				 * oldNodes.add(expandedElements[i]); }
				 */

				// set new input
				viewer.setInput(input);

				// restore viewer state
				viewer.setExpandedElements(expandedElements);

				/*
				 * ArrayList newNodes = new ArrayList(); OutlineNode newNode;
				 * for (Iterator iter = this.input.getRootNodes().iterator();
				 * iter.hasNext();) { newNode = (OutlineNode)iter.next();
				 * restoreExpandState(newNode, oldNodes, newNodes); }
				 */
				control.setRedraw(true);
			}
		}
	}

	private String getFileName() {
		return editor.getFile().getLocation().toString();
	}

	/**
	 * Focuses the editor to the text of the selected item.
	 * 
	 * @param event
	 *            the selection event
	 */
	@Override
	public void selectionChanged(SelectionChangedEvent event) {
		super.selectionChanged(event);
		ISelection selection = event.getSelection();
		if (selection.isEmpty()) {
			editor.resetHighlightRange();
		} else {
			OutlineNode node = (OutlineNode) ((IStructuredSelection) selection)
					.getFirstElement();
			IDocument doc = editor.getDocumentProvider().getDocument(
					editor.getEditorInput());

			Position position = computePosition(node.getAst(), doc);
			if (position != null) {
				try {
					editor.setHighlightRange(position.getOffset(), position
							.getLength(), true);
					editor.getViewer().revealRange(position.getOffset(),
							position.getLength());
				} catch (IllegalArgumentException x) {
					editor.resetHighlightRange();
				}
			} else {
				editor.resetHighlightRange();
			}
		}
	}

	/**
	 * This is kinda hackish, because the last character in a block is not
	 * included in the AST, so we do not really know where a multi-line
	 * statement ends
	 */
	private Position computePosition(AstNode node, IDocument doc) {
		try {
			int start = doc.getLineOffset(node.getLine() - 1)
					+ node.getOffset();
			int end;
			if (node.getLine() == node.getEndLine())
				end = node.getText().length();
			else
				end = doc.getLineOffset(node.getEndLine() + 1) - start;
			return new Position(start, end);
		} catch (BadLocationException e) {
			e.printStackTrace();
			return null;
		}
	}
}
