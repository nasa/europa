package nddl;

public class EmptyDomainException extends Exception
{
	public EmptyDomainException() {
		super();
	}
	public EmptyDomainException(String message) {
		super(message);
	}
	public EmptyDomainException(String message, Throwable cause) {
		super(message,cause);
	}
	public EmptyDomainException(Throwable cause) {
		super(cause);
	}
	public EmptyDomainException(NddlType cause) {
		super("Domain of "+cause.toString()+" was emptied.");
	}
}
