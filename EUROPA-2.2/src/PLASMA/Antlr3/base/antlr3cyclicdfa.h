/// Definition of a cyclic dfa structure such that it can be
/// initialized at compile time and have only a single
/// runtime function that can deal with all cyclic dfa
/// structures and show Java how it is done ;-)
///
#ifndef	ANTLR3_CYCLICDFA_H
#define	ANTLR3_CYCLICDFA_H

#include    <antlr3baserecognizer.h>
#include    <antlr3intstream.h>

#ifdef __cplusplus
extern "C" {

// If this header file is included as part of a generated recognizer that
// is being compiled as if it were C++, and this is Windows, then the const elements
// of the structure cause the C++ compiler to (rightly) point out that
// there can be no instantiation of the structure because it needs a constructor
// that can initialize the data, however these structures are not
// useful for C++ as they are pre-generated and static in the recognizer.
// So, we turn off those warnings, which are only at /W4 anyway.
//
#ifdef ANTLR3_WINDOWS
#pragma warning	(push)
#pragma warning (disable : 4510)
#pragma warning (disable : 4512)
#pragma warning (disable : 4610)
#endif
#endif

typedef struct ANTLR3_CYCLIC_DFA_struct
{
    /// Decision number that a particular static structure
    ///  represents.
    ///
    const ANTLR3_INT32		decisionNumber;

    /// What this decision represents
    ///
    const pANTLR3_UCHAR		description;

    ANTLR3_INT32			(*specialStateTransition)   (void * ctx, pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_INT_STREAM is, struct ANTLR3_CYCLIC_DFA_struct * dfa, ANTLR3_INT32 s);

    ANTLR3_INT32			(*specialTransition)	    (void * ctx, pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_INT_STREAM is, struct ANTLR3_CYCLIC_DFA_struct * dfa, ANTLR3_INT32 s);

    ANTLR3_INT32			(*predict)					(void * ctx, pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_INT_STREAM is, struct ANTLR3_CYCLIC_DFA_struct * dfa);

    const ANTLR3_INT32		    * const eot;
    const ANTLR3_INT32		    * const eof;
    const ANTLR3_INT32		    * const min;
    const ANTLR3_INT32		    * const max;
    const ANTLR3_INT32		    * const accept;
    const ANTLR3_INT32		    * const special;
    const ANTLR3_INT32			* const * const transition;

}
    ANTLR3_CYCLIC_DFA;

typedef ANTLR3_INT32		(*CDFA_SPECIAL_FUNC)   (void * , pANTLR3_BASE_RECOGNIZER , pANTLR3_INT_STREAM , struct ANTLR3_CYCLIC_DFA_struct * , ANTLR3_INT32);

#ifdef __cplusplus
}
#ifdef ANTLR3_WINDOWS
#pragma warning	(pop)
#endif
#endif

#endif
