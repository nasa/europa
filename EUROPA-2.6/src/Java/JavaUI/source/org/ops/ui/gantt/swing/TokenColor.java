package org.ops.ui.gantt.swing;

import java.awt.Color;

/**
 * Color structure for painting a token
 * 
 * @author Tatiana
 */
public class TokenColor {
	public Color start, end, duration, body, outOfBounds;

	public TokenColor(Color start, Color end, Color duration, Color body) {
		this(start, end, duration, body, Color.red);
	}

	public TokenColor(Color start, Color end, Color duration, Color body, Color outOfBounds) {
		this.start = start;
		this.end = end;
		this.duration = duration;
		this.body = body;
		this.outOfBounds = outOfBounds;
	}
}
