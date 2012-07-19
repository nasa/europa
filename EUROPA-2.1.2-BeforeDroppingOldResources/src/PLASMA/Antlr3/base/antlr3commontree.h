/** Interface for an ANTLR3 common tree which is what gets
 *  passed around by the AST producing parser.
 */

#ifndef	_ANTLR3_COMMON_TREE_H
#define	_ANTLR3_COMMON_TREE_H

#include    <antlr3defs.h>
#include    <antlr3basetree.h>
#include    <antlr3commontoken.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ANTLR3_COMMON_TREE_struct
{

	/// Not used by ANTLR, but if a super structure is created above
    /// this structure, it can be used to point to the start of the super
    /// structure, where additional data and function pointers can be stored.
    ///
    void					* super;

    /// Start token index that encases this tree
    ///
    ANTLR3_MARKER			  startIndex;

    /// End token that encases this tree
    ///
    ANTLR3_MARKER			  stopIndex;

    /// A single token, this is the payload for the tree
    ///
    pANTLR3_COMMON_TOKEN      token;

	/// Points to the node that has this node as a child.
	/// If this is NULL, then this is the root node.
	///
	pANTLR3_COMMON_TREE		  parent;

	/// What index is this particular node in the child list it
	/// belongs to?
	///
	ANTLR3_INT32			  childIndex;
	
	/// Pointer to the tree factory that manufactured this
	/// token. This can be used by duplication methods and so on
	/// to manufacture another auto-tracked common tree structure
	///
	pANTLR3_ARBORETUM	factory;

    /// An encapsulated BASE TREE structure (NOT a pointer)
    /// that performs a lot of the dirty work of node management
    /// To this we add just a few functions that are specific to the 
    /// payload. You can further abstract common tree so long
    /// as you always have a baseTree pointer in the top structure
    /// and copy it from the next one down. 
    /// So, lets say we have a structure JIMS_TREE. 
    /// It needs an ANTLR3_BASE_TREE that will support all the
    /// general tree duplication stuff.
    /// It needs a ANTLR3_COMMON_TREE structure embedded or completely
    /// provides the equivalent interface.
    /// It provides it's own methods and data.
    /// To create a new one of these, the function provided to
    /// the tree adaptor (see comments there) should allocate the
    /// memory for a new JIMS_TREE structure, then call
    /// antlr3InitCommonTree(<addressofembeddedCOMMON_TREE>)
    /// antlr3BaseTreeNew(<addressofBASETREE>)
    /// The interfaces for BASE_TREE and COMMON_TREE will then
    /// be initialized. You then call and you can override them or just init
    /// JIMS_TREE (note that the base tree in common tree will be ignored)
    /// just the top level base tree is used). Codegen will take care of the rest.
    /// 
    ANTLR3_BASE_TREE	    baseTree;
     
}
    ANTLR3_COMMON_TREE;

/// \brief ANTLR3 Tree factory interface to create lots of trees efficiently
///  rather than creating and freeing lots of little bits of memory.
///
typedef	struct ANTLR3_ARBORETUM_struct
{
    /// Pointers to the array of tokens that this factory has produced so far
    ///
    pANTLR3_COMMON_TREE    *pools;

    /// Current pool tokens we are allocating from
    ///
    ANTLR3_INT32			thisPool;

    /// The next token to throw out from the pool, will cause a new pool allocation
    ///  if this exceeds the available tokenCount
    ///
    ANTLR3_UINT32			nextTree;

    /// Trick to initialize tokens and their API quickly, we set up this token when the
    /// factory is created, then just copy the memory it uses into the new token.
    ///
    ANTLR3_COMMON_TREE	    unTruc;

	/// Pointer to a vector factory that is used to create child list vectors
	/// for any child nodes that need them. This means that we auto track the
	/// vectors and auto free them when we close the factory. It also means
	/// that all rewriting trees can use the same tree factory and the same
	/// vector factory and we do not dup any nodes unless we must do so 
	/// explicitly because of context such as an empty rewrite stream and
	/// ->IMAGINARY[ID] so on. This makes memory tracking much simpler and 
	/// tempts no errors.
	///
	pANTLR3_VECTOR_FACTORY   vFactory;

    /// Pointer to a function that returns a new tree
    ///
    pANTLR3_BASE_TREE	    (*newTree)		(struct ANTLR3_ARBORETUM_struct * factory);
    pANTLR3_BASE_TREE	    (*newFromTree)	(struct ANTLR3_ARBORETUM_struct * factory, pANTLR3_COMMON_TREE tree);
    pANTLR3_BASE_TREE	    (*newFromToken)	(struct ANTLR3_ARBORETUM_struct * factory, pANTLR3_COMMON_TOKEN token);

    /// Pointer to a function the destroys the factory
    ///
    void		    (*close)	    (struct ANTLR3_ARBORETUM_struct * factory);
}
    ANTLR3_ARBORETUM;

#ifdef __cplusplus
}
#endif

#endif


