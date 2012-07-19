/** \file
 * Implementation of the base functionality for an ANTLR3 parser.
 */
#include    <antlr3parser.h>

/* Parser API 
 */
static void					setDebugListener	(pANTLR3_PARSER parser, pANTLR3_DEBUG_EVENT_LISTENER dbg);
static void					setTokenStream		(pANTLR3_PARSER parser, pANTLR3_TOKEN_STREAM);
static pANTLR3_TOKEN_STREAM	getTokenStream		(pANTLR3_PARSER parser);
static void					freeParser		    (pANTLR3_PARSER parser);

ANTLR3_API pANTLR3_PARSER
antlr3ParserNewStreamDbg		(ANTLR3_UINT32 sizeHint, pANTLR3_TOKEN_STREAM tstream, pANTLR3_DEBUG_EVENT_LISTENER dbg, pANTLR3_RECOGNIZER_SHARED_STATE state)
{
	pANTLR3_PARSER	parser;

	parser = antlr3ParserNewStream(sizeHint, tstream, state);

	if	(parser == NULL)
    {
		return	NULL;
    }

	parser->setDebugListener(parser, dbg);

	return parser;
}

ANTLR3_API pANTLR3_PARSER
antlr3ParserNew		(ANTLR3_UINT32 sizeHint, pANTLR3_RECOGNIZER_SHARED_STATE state)
{
    pANTLR3_PARSER	parser;

    /* Allocate memory
     */
    parser	= (pANTLR3_PARSER) ANTLR3_MALLOC(sizeof(ANTLR3_PARSER));

    if	(parser == NULL)
    {
		return	NULL;
    }

    /* Install a base parser
     */
    parser->rec =  antlr3BaseRecognizerNew(ANTLR3_TYPE_PARSER, sizeHint, state);

    if	(parser->rec == NULL)
    {
		parser->free(parser);
		return	NULL;
    }

    parser->rec->super	= parser;

    /* Parser overrides
     */
    parser->rec->exConstruct	=  antlr3MTExceptionNew;

    /* Install the API
     */
	parser->setDebugListener	=  setDebugListener;
    parser->setTokenStream		=  setTokenStream;
    parser->getTokenStream		=  getTokenStream;

    parser->free			=  freeParser;

    return parser;
}

ANTLR3_API pANTLR3_PARSER
antlr3ParserNewStream	(ANTLR3_UINT32 sizeHint, pANTLR3_TOKEN_STREAM tstream, pANTLR3_RECOGNIZER_SHARED_STATE state)
{
    pANTLR3_PARSER	parser;

    parser  = antlr3ParserNew(sizeHint, state);

    if	(parser == NULL)
    {
		return	NULL;
    }

    /* Everything seems to be hunky dory so we can install the 
     * token stream.
     */
    parser->setTokenStream(parser, tstream);

    return parser;
}

static void		
freeParser			    (pANTLR3_PARSER parser)
{
    if	(parser->rec != NULL)
    {
		// This may have ben a delegate or delegator parser, in which case the
		// state may already have been freed (and set to NULL therefore)
		// so we ignore the state if we don't have it.
		//
		if	(parser->rec->state != NULL)
		{
			if	(parser->rec->state->following != NULL)
			{
				parser->rec->state->following->free(parser->rec->state->following);
				parser->rec->state->following = NULL;
			}
		}
	    parser->rec->free(parser->rec);
	    parser->rec	= NULL;

    }
    ANTLR3_FREE(parser);
}

static void					
setDebugListener		(pANTLR3_PARSER parser, pANTLR3_DEBUG_EVENT_LISTENER dbg)
{
	// Set the debug listener. There are no methods to override
	// because currently the only ones that notify the debugger
	// are error reporting and recovery. Hence we can afford to
	// check and see if the debugger interface is null or not
	// there. If there is ever an occasion for a performance
	// sensitive function to use the debugger interface, then
	// a replacement function for debug mode should be supplied
	// and installed here.
	//
	parser->rec->debugger	= dbg;

	// If there was a tokenstream installed already
	// then we need to tell it about the debug interface
	//
	if	(parser->tstream != NULL)
	{
		parser->tstream->setDebugListener(parser->tstream, dbg);
	}
}

static void			
setTokenStream		    (pANTLR3_PARSER parser, pANTLR3_TOKEN_STREAM tstream)
{
    parser->tstream = tstream;
    parser->rec->reset(parser->rec);
}

static pANTLR3_TOKEN_STREAM	
getTokenStream		    (pANTLR3_PARSER parser)
{
    return  parser->tstream;
}














