package org.ops.ui.solver.model;

import java.text.NumberFormat;

public abstract class TimeFormatHelper {
	
	private TimeFormatHelper() {}

	private final static NumberFormat decimal3 = NumberFormat
			.getNumberInstance();
	static {
		decimal3.setMaximumFractionDigits(3);
	}

	public static String formatTime(long msecs) {
		if (msecs < 1000)
			return Long.toString(msecs) + " msecs";
		else if (msecs < (60 * 1000))
			return decimal3.format(msecs / 1000.0) + " secs";
		else
			return decimal3.format(msecs / (60 * 1000.0)) + " mins";
	}

}
