package org.ops.ui.editor.swt;

import java.util.HashMap;
import java.util.Map;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IMarker;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.ui.texteditor.MarkerUtilities;
import org.ops.ui.editor.model.OutlineNode;
import org.ops.ui.filemanager.model.AstNode;
import org.ops.ui.filemanager.model.ErrorRecord;
import org.ops.ui.filemanager.model.FileModel;
import org.ops.ui.main.swt.EuropaPlugin;

public class NddlContentProvider implements IStructuredContentProvider,
		ITreeContentProvider {

	private NddlEditor editor;

	public NddlContentProvider(NddlEditor nddlEditor) {
		this.editor = nddlEditor;
	}

	public void inputChanged(Viewer v, Object oldInput, Object newInput) {
	}

	public void dispose() {
	}

	public Object[] getElements(Object parent) {
		return getChildren(parent);
	}

	public AstNode getParent(Object child) {
		if (child instanceof OutlineNode) {
			System.out.println("getParent for OutlineNode " + child);
		}
		return null;
	}

	public Object[] getChildren(Object parent) {
		if (!(parent instanceof OutlineNode))
			return new Object[0];
		return ((OutlineNode) parent).getChildren().toArray();
	}

	public boolean hasChildren(Object parent) {
		if (!(parent instanceof OutlineNode))
			return false;
		return !((OutlineNode) parent).getChildren().isEmpty();
	}

	public void reload(IFile file) {
		String fileName = file.getLocation().toString();
		FileModel fm = FileModel.getModel(fileName);

		// Update outline, if any
		if (editor.getOutlinePage() != null)
			editor.getOutlinePage().update(
					OutlineNode.makeTree(fm == null ? null : fm.getAST()));

		// Update error markers on the editor
		IResource resource = (IResource) editor.getEditorInput().getAdapter(
				IResource.class);
		if (resource != null) {
			IDocument document = editor.getDocumentProvider().getDocument(
					editor.getEditorInput());
			clearMarkers(resource);

			// Report our own records as is, accumulate counters for includes
			HashMap<String, Integer> parents = new HashMap<String, Integer>();
			for (ErrorRecord rec : fm.getErrors()) {
				String fname = rec.getFileName();
				if (fileName.equals(fname))
					createErrorMarker(resource, document, rec);
				else if (fname != null) {
					Integer count = parents.get(fname);
					if (count == null)
						count = 1;
					else
						count = count + 1;
					parents.put(fname, count);
				}
			}

			// Report included errors as one per file
			for (String name : parents.keySet())
				try {
					Map<String, ? super Object> map = new HashMap<String, Object>();
					map.put(IMarker.MESSAGE, parents.get(name)
							+ " error in included file " + name);
					map.put(IMarker.SEVERITY, Integer
							.valueOf(IMarker.SEVERITY_ERROR));
					MarkerUtilities
							.createMarker(resource, map, IMarker.PROBLEM);
				} catch (Exception ce) {
					EuropaPlugin.getDefault().logError("Creating marker", ce);
				}
		}
	}

	private void clearMarkers(IResource resource) {
		try {
			resource.deleteMarkers(IMarker.PROBLEM, false,
					IResource.DEPTH_INFINITE);
		} catch (CoreException e) {
			EuropaPlugin.getDefault().logError("Deleting error markers", e);
		}
	}

	private void createErrorMarker(IResource resource, IDocument document,
			ErrorRecord record) {
		try {
			Map<String, ? super Object> map = new HashMap<String, Object>();
			map.put(IMarker.LINE_NUMBER, record.getLine());
			map.put(IMarker.MESSAGE, record.getMessage());

			int off = document.getLineOffset(record.getLine() - 1);
			off += record.getOffset(); // offset in line
			int len = record.getLength();
			if (len == 0)
				len = 1;
			map.put(IMarker.CHAR_START, off);
			map.put(IMarker.CHAR_END, off + len);

			map.put(IMarker.SEVERITY, Integer.valueOf(IMarker.SEVERITY_ERROR));

			MarkerUtilities.createMarker(resource, map, IMarker.PROBLEM);
		} catch (Exception ce) {
			EuropaPlugin.getDefault().logError("Creating marker", ce);
		}
	}
}