package org.ops.ui.editor.swt;

import java.io.IOException;
import java.io.InputStream;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.IDocumentPartitioner;
import org.eclipse.jface.text.rules.FastPartitioner;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.editors.text.FileDocumentProvider;
import org.eclipse.ui.ide.FileStoreEditorInput;

/**
 * Extension of document provider that hooks up partition scanner. The scanner
 * is needed to paint comments
 */
public class NddlDocumentProvider extends FileDocumentProvider {

	@Override
	protected IDocument createDocument(Object element) throws CoreException {
		IDocument document = super.createDocument(element);
		if (document != null) {
			IDocumentPartitioner partitioner = new FastPartitioner(
					new NddlPartitionScanner(),
					new String[] { NddlPartitionScanner.NDDL_COMMENT });
			partitioner.connect(document);
			document.setDocumentPartitioner(partitioner);
		}
		return document;
	}

	/* Default method does not like files outside of workspace */
	@Override
	protected boolean setDocumentContent(IDocument document,
			IEditorInput editorInput, String encoding) throws CoreException {
		if (editorInput instanceof FileStoreEditorInput) {
			try {
				InputStream stream = ((FileStoreEditorInput) editorInput)
						.getURI().toURL().openStream();
				try {
					setDocumentContent(document, stream, encoding);
				} finally {
					try {
						stream.close();
					} catch (IOException x) {
					}
				}
			} catch (Exception e) {
			}
			return true;
		}
		return super.setDocumentContent(document, editorInput, encoding);
	}
}