#ifndef	_ANTLR3ERRORS_H
#define	_ANTLR3ERRORS_H

#define	ANTLR3_SUCCESS	0
#define	ANTLR3_FAIL	1

#define	ANTLR3_TRUE	1
#define	ANTLR3_FALSE	0

/** Indicates end of character stream and is an invalid Unicode code point. */
#define ANTLR3_CHARSTREAM_EOF	0xFFFFFFFF

/** Indicates  memoizing on a rule failed.
 */
#define	MEMO_RULE_FAILED	0xFFFFFFFE
#define	MEMO_RULE_UNKNOWN	0xFFFFFFFF


#define	ANTLR3_ERR_BASE	    0
#define	ANTLR3_ERR_NOMEM    (ANTLR3_ERR_BASE + 1)
#define	ANTLR3_ERR_NOFILE   (ANTLR3_ERR_BASE + 2)
#define	ANTLR3_ERR_HASHDUP  (ANTLR3_ERR_BASE + 3)

#endif	/* _ANTLR3ERRORS_H */
