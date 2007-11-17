package UBO;

import java.util.List;

import psengine.PSResource;
import psengine.PSToken;

public interface Resource {

	public abstract int getCapacity();

	public abstract PSResource getPSResource();

	public abstract String getName();

	public abstract ResourceProfile getLevels();

	public abstract List<PSToken> getConflictSet(int t);

	// TODO: this should be supported by PSResource
	public abstract int getMostViolatedTime();

}