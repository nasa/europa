package org.ops.ui.ash;

/*
 * AshTokenMarker.java - Ash Shell token marker
 */

import javax.swing.text.Segment;
import org.ops.ui.ash.KeywordMap;
import org.ops.ui.ash.TokenMarker;
import org.ops.ui.ash.Token;

/**
 * Ash Shell token marker.
 *
 * @author Matthew E. Boyce
 * @version $Id: AshTokenMarker.java,v 1.2 2007-03-01 23:37:33 meboyce Exp $
 */
public class AshTokenMarker extends TokenMarker {
	protected String prompt;

  public AshTokenMarker(String prompt) {
		this.prompt = prompt;
    this.keywords = getKeywords();
  }

  public byte markTokensImpl(byte token, Segment line, int lineIndex) {
    char[] array = line.array;
    int offset = line.offset;
    lastOffset = offset;
    lastKeyword = offset;
    int length = line.count + offset;
    boolean backslash = false;

loop:
    for(int i = offset; i < length; i++) {
      // test for prompt!
      if(lastOffset + prompt.length() <= length) {
        String lineStart = new String(array, lastOffset, prompt.length());
        if(lineStart.equals(prompt)) {
          addToken(i - lastOffset,token);
          addToken(prompt.length(), Token.LABEL);
          i+=prompt.length();
          lastOffset = lastKeyword = i;
          if(i >= length)
            continue;
        }
      }
      int i1 = (i+1);

      char c = array[i];
      if(c == '\\') {
        backslash = !backslash;
        continue;
      }

      switch(token) {
        case Token.NULL:
          switch(c) {
            case '"':
              doKeyword(line,i,c);
              if(backslash)
                backslash = false;
              else {
                addToken(i - lastOffset,token);
                token = Token.LITERAL1;
                lastOffset = lastKeyword = i;
              }
              break;
            case '\'':
              doKeyword(line,i,c);
              if(backslash)
                backslash = false;
              else {
                addToken(i - lastOffset,token);
                token = Token.LITERAL2;
                lastOffset = lastKeyword = i;
              }
              break;
            case '!':
              backslash = false;
              doKeyword(line,i,c);
              if(length - i > 1) {
                if(array[i1] == '!') {
                  addToken(i - lastOffset,token);
                  addToken(length - i,Token.ERROR);
                  lastOffset = lastKeyword = length;
                }
                else {
                  addToken(i - lastOffset,token);
                  addToken(length - i,Token.WARNING);
                  lastOffset = lastKeyword = length;
                }
              }
              break loop;
            case '/':
              backslash = false;
              doKeyword(line,i,c);
              if(length - i > 1) {
                switch(array[i1]) {
                  case '*':
                    addToken(i - lastOffset,token);
                    lastOffset = lastKeyword = i;
                    if(length - i > 2 && array[i+2] == '*')
                      token = Token.COMMENT2;
                    else
                      token = Token.COMMENT1;
                    break;
                  case '/':
                    addToken(i - lastOffset,token);
                    addToken(length - i,Token.COMMENT1);
                    lastOffset = lastKeyword = length;
                    break loop;
                }
              }
              break;
            default:
              backslash = false;
              if(!Character.isLetterOrDigit(c) && c != '_' && c != '#' &&
							   c != '-' && c != '+' && c != '.' ) {
                doKeyword(line,i,c);
              }
              break;
          }
          break;
        case Token.COMMENT1:
        case Token.COMMENT2:
          backslash = false;
          if(c == '*' && length - i > 1) {
            if(array[i1] == '/') {
              i++;
              addToken((i+1) - lastOffset,token);
              token = Token.NULL;
              lastOffset = lastKeyword = i+1;
            }
          }
          break;
        case Token.LITERAL1:
          if(backslash)
            backslash = false;
          else if(c == '"') {
            addToken(i1 - lastOffset,token);
            token = Token.NULL;
            lastOffset = lastKeyword = i1;
          }
          break;
        case Token.LITERAL2:
          if(backslash)
            backslash = false;
          else if(c == '\'') {
            addToken(i1 - lastOffset,Token.LITERAL1);
            token = Token.NULL;
            lastOffset = lastKeyword = i1;
          }
          break;
        default:
          throw new InternalError("Invalid state: "
              + token);
      }
    }

    if(token == Token.NULL)
      doKeyword(line,length,'\0');

    switch(token) {
      case Token.LITERAL1:
      case Token.LITERAL2:
        addToken(length - lastOffset,Token.INVALID);
        token = Token.NULL;
        break;
      case Token.KEYWORD2:
        addToken(length - lastOffset,token);
        if(!backslash)
          token = Token.NULL;
      default:
        addToken(length - lastOffset,token);
        break;
    }

    return token;
  }

  public static KeywordMap getKeywords() {
    return new KeywordMap(false);
  }

  protected KeywordMap keywords;
  protected int lastOffset;
  protected int lastKeyword;

  protected boolean doKeyword(Segment line, int i, char c) {
    int i1 = i+1;

    int len = i - lastKeyword;
    byte id = keywords.lookup(line,lastKeyword,len);
    if(id != Token.NULL) {
      if(lastKeyword != lastOffset)
        addToken(lastKeyword - lastOffset,Token.NULL);
      addToken(len,id);
      lastOffset = i;
    }
    lastKeyword = i1;
    return false;
  }
}
