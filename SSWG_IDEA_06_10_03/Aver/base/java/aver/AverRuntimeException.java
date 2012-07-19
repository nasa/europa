package aver;

public class AverRuntimeException extends Exception {
  public AverRuntimeException(final String message) {
    super(message);
  }
  public AverRuntimeException(final Throwable other) {
    super(other);
  }
}
