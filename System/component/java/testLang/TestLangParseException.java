package testLang;

public class TestLangParseException extends Exception {
  public TestLangParseException(final String message) {
    super(message);
  }
  public TestLangParseException(final Throwable other) {
    super(other);
  }
}
