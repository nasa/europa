/** \file
 * Base implementation of an ANTLR3 parser.
 *
 *
 */
#ifndef	_ANTLR3_PARSER_H
#define	_ANTLR3_PARSER_H

#include    <antlr3defs.h>
#include    <antlr3baserecognizer.h>

#ifdef __cplusplus
extern "C" {
#endif

/** This is the main interface for an ANTLR3 parser.
 */
typedef	struct ANTLR3_PARSER_struct
{
    /** All superstructure implementers of this interface require a pointer to their selves,
     *  which they can reference using the super pointer here.
     */
    void			* super;

    /** A pointer to the base recognizer, where most of the parser functions actually
     *  live because they are shared between parser and tree parser and this is the
     *  easier way than copying the interface all over the place. Macros hide this
     *  for the generated code so it is easier on the eye (though not the debugger ;-).
     */
    pANTLR3_BASE_RECOGNIZER			rec;

    /** A provider of a tokenstream interface, for the parser to consume
     *  tokens from.
     */
    pANTLR3_TOKEN_STREAM			tstream;

	/** A pointer to a function that installs a debugger object (it also
	 *  installs the debugging versions of the parser methods. This means that 
	 *  a non debug parser incurs no overhead because of the debugging stuff.
	 */
	void					(*setDebugListener)	(struct ANTLR3_PARSER_struct	* parser, pANTLR3_DEBUG_EVENT_LISTENER dbg);

    /** A pointer to a function that installs a token stream 
     * for the parser.
     */
    void					(*setTokenStream)	(struct ANTLR3_PARSER_struct	* parser, pANTLR3_TOKEN_STREAM);

    /** A pointer to a function that returns the token stream for this 
     *  parser.
     */
    pANTLR3_TOKEN_STREAM	(*getTokenStream)	(struct ANTLR3_PARSER_struct	* parser);

    /** Pointer to a function that knows how to free resources of an ANTLR3 parser.
     */
    void			(*free)			(struct ANTLR3_PARSER_struct	* parser);

}
    ANTLR3_PARSER;

#ifdef __cplusplus
}
#endif

#endif
