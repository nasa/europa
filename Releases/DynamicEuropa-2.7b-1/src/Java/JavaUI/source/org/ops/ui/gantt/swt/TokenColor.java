package org.ops.ui.gantt.swt;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.swt.graphics.Color;

/**
 * Color structure for painting a token
 * 
 * @author Tatiana
 */
public class TokenColor {
	public Color start, end, duration, body, outOfBounds;

	public TokenColor(Color start, Color end, Color duration, Color body) {
		this(start, end, duration, body, ColorConstants.red);
	}

	public TokenColor(Color start, Color end, Color duration, Color body, Color outOfBounds) {
		this.start = start;
		this.end = end;
		this.duration = duration;
		this.body = body;
		this.outOfBounds = outOfBounds;
	}
}
