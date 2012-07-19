package nddl;

import java.util.Iterator;
import java.util.Set;
import java.util.HashSet;
import java.io.PrintStream;

public class DebugMsg
{
	/// if this is false then no messages are ever printed. (overrides all other options)
	static boolean debugMsgEnabled = true;
	/// The set of debugMessages which are enabled (the marker for a given debug message must contain the toString of some element in this set).
	static Set enabled = new HashSet();
	/// if this is true then all messages will be printed regardless of the contents of enabled.
	static boolean allEnabled = false;
	/// The stream to which all messages will be printed.
	static PrintStream stream = System.out;

	/// A message which will be printed if this module is enabled (no marker)
	public static boolean debugMsg(Object data)
	{
		return debugMsg(null,data,false);
	}

	public static boolean debugMsg(String marker, Object data)
	{
		return debugMsg(marker,data,true);
	}

	/// A message (data) which will be printed to stream if it's marker is contained within enabled or null.
	public static boolean debugMsg(String marker, Object data, boolean printMarker)
	{
		if(isEnabled(marker)) {
			if(marker!=null && printMarker)
			{
				stream.print("[");
				stream.print(marker);
				stream.print("]");
			}
			stream.println(data);
		}
		return true;
	}

	/**
	 * Add some pattern to the set of enabled debugMessages.
	 * Any leading whitespace as well as an optional colon will be stripped.
	 */
	public static void enable(String pattern)
	{
		// Strip any leading whitespace followed by an optional colon.
		enabled.add(pattern.replaceAll("^\\s+(:)?",""));
	}

	/**
	 * Test if a given marker is currently enabled.
	 * This consists of testing if any enabled patterns are contained within this
	 * marker according to String.indexOf().
	 */
	public static boolean isEnabled(String marker)
	{
		if(!debugMsgEnabled) return false;
		if(allEnabled || marker == null) return true;
		for(Iterator i = enabled.iterator(); i.hasNext();)
		{
			String enabledMessage = i.next().toString();
			if(marker.indexOf(enabledMessage) != -1) return true;
		}
		return false;
	}

	/**
	 * Change the stream that's in use by this module.
	 */
	public void setStream(PrintStream stream)
	{
		this.stream = stream;
	}

	/**
	 * This classes toString() method (suitable for printing in a debugMsg itself).
	 * This method is not called toString() because it cannot match the signature of toString()
	 * as provided by Object.
	 */
	public static String staticToString()
	{
		StringBuffer toRet = new StringBuffer(512);
		toRet.append("debugMsg [");
		toRet.append("On: ").append(new Boolean(debugMsgEnabled).toString()).append(", ");
		if(stream == System.out) toRet.append("Stream: System.out, ");
		else if(stream == System.err) toRet.append("Stream: System.err, ");
		else toRet.append("Stream: ").append(stream.toString()).append(", ");
		toRet.append("Enabled: ").append(enabled.toString());
		toRet.append("]");
		return toRet.toString();
	}
}
