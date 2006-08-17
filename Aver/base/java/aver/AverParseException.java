package aver;

public class AverParseException extends Exception {
  public AverParseException(final String message) {
    super(message);
  }
  public AverParseException(final Throwable other) {
    super(other);
  }
}
