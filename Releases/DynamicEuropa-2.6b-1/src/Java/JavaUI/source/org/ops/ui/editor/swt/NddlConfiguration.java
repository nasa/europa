package org.ops.ui.editor.swt;

import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.TextAttribute;
import org.eclipse.jface.text.presentation.IPresentationReconciler;
import org.eclipse.jface.text.presentation.PresentationReconciler;
import org.eclipse.jface.text.rules.DefaultDamagerRepairer;
import org.eclipse.jface.text.rules.Token;
import org.eclipse.jface.text.source.ISourceViewer;
import org.eclipse.ui.editors.text.TextSourceViewerConfiguration;

public class NddlConfiguration extends TextSourceViewerConfiguration {

	/** File-level scanner. White spaces and processing instructions */
	private NddlScanner scanner;
	/** Map of RBG to Color that knows to release resources */
	private ColorManager colorManager;

	public NddlConfiguration(ColorManager colorManager) {
		this.colorManager = colorManager;
	}

	@Override
	public String[] getConfiguredContentTypes(ISourceViewer sourceViewer) {
		return new String[] { IDocument.DEFAULT_CONTENT_TYPE,
				NddlPartitionScanner.NDDL_COMMENT };
	}

	protected NddlScanner getNddlScanner() {
		if (scanner == null) {
			scanner = new NddlScanner(colorManager);
			scanner.setDefaultReturnToken(new Token(new TextAttribute(
					colorManager.getColor(INddlColorConstants.DEFAULT))));
		}
		return scanner;
	}

	/**
	 * Reconciler tells the UI how to paint text. All types recognized by
	 * partition scanner should have sections
	 */
	@Override
	public IPresentationReconciler getPresentationReconciler(
			ISourceViewer sourceViewer) {
		PresentationReconciler reconciler = new PresentationReconciler();

		DefaultDamagerRepairer dr = null;

		// If no particular element has been detected, i.e. it is still
		// "default", use file-level scanner
		dr = new DefaultDamagerRepairer(getNddlScanner());
		reconciler.setDamager(dr, IDocument.DEFAULT_CONTENT_TYPE);
		reconciler.setRepairer(dr, IDocument.DEFAULT_CONTENT_TYPE);

		// Separate painter for comments
		NonRuleBasedDamagerRepairer ndr = new NonRuleBasedDamagerRepairer(
				new TextAttribute(colorManager
						.getColor(INddlColorConstants.NDDL_COMMENT)));
		reconciler.setDamager(ndr, NddlPartitionScanner.NDDL_COMMENT);
		reconciler.setRepairer(ndr, NddlPartitionScanner.NDDL_COMMENT);

		return reconciler;
	}
}