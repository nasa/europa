package testLang;

public class TestLangRuntimeException extends Exception {
  public TestLangRuntimeException(final String message) {
    super(message);
  }
  public TestLangRuntimeException(final Throwable other) {
    super(other);
  }
}
