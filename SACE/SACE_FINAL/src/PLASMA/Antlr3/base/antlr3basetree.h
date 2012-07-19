/// \file
/// Definition of the ANTLR3 base tree.
///

#ifndef	_ANTLR3_BASE_TREE_H
#define	_ANTLR3_BASE_TREE_H

#include    <antlr3defs.h>
#include    <antlr3collections.h>
#include    <antlr3string.h>

#ifdef __cplusplus
extern "C" {
#endif

/// A generic tree implementation with no payload.  You must subclass to
/// actually have any user data.  ANTLR v3 uses a list of children approach
/// instead of the child-sibling approach in v2.  A flat tree (a list) is
/// an empty node whose children represent the list.  An empty (as in it does not
/// have payload itself), but non-null node is called "nil".
///
typedef	struct ANTLR3_BASE_TREE_struct
{

    /// Implementers of this interface sometimes require a pointer to their selves.
    ///
    void    *	    super;

    /// Generic void pointer allows the grammar programmer to attach any structure they
    /// like to a tree node, in many cases saving the need to create their own tree
    /// and tree adaptors. ANTLR does not use this pointer, but will copy it for you and so on.
    ///
    void    *	    u;

    /// The list of all the children that belong to this node. They are not part of the node
    /// as they belong to the common tree node that implements this.
    ///
    pANTLR3_VECTOR  children;

    /// This is used to store the current child index position while descending
    /// and ascending trees as the tree walk progresses.
    ///
    ANTLR3_MARKER   savedIndex;

    /// A string factory to produce strings for toString etc
    ///
    pANTLR3_STRING_FACTORY strFactory;

	/// A pointer to a function that returns the common token pointer
	/// for the payload in the supplied tree.
	///
    pANTLR3_COMMON_TOKEN    (*getToken)					(struct ANTLR3_BASE_TREE_struct * tree);  

    void					(*addChild)					(struct ANTLR3_BASE_TREE_struct * tree, void * child);

    void					(*addChildren)				(struct ANTLR3_BASE_TREE_struct * tree, pANTLR3_LIST kids);

    void    				(*createChildrenList)		(struct ANTLR3_BASE_TREE_struct * tree);

    void    *				(*deleteChild)				(struct ANTLR3_BASE_TREE_struct * tree, ANTLR3_UINT32 i);

	void					(*replaceChildren)			(struct ANTLR3_BASE_TREE_struct * parent, ANTLR3_INT32 startChildIndex, ANTLR3_INT32 stopChildIndex, struct ANTLR3_BASE_TREE_struct * t);

    void    *				(*dupNode)					(struct ANTLR3_BASE_TREE_struct * dupNode);

    void    *				(*dupTree)					(struct ANTLR3_BASE_TREE_struct * tree);

    ANTLR3_UINT32			(*getCharPositionInLine)	(struct ANTLR3_BASE_TREE_struct * tree);

    void    *				(*getChild)					(struct ANTLR3_BASE_TREE_struct * tree, ANTLR3_UINT32 i);

	void    				(*setChildIndex)			(struct ANTLR3_BASE_TREE_struct * tree, ANTLR3_INT32 );
	ANTLR3_INT32			(*getChildIndex)			(struct ANTLR3_BASE_TREE_struct * tree );

    ANTLR3_UINT32			(*getChildCount)			(struct ANTLR3_BASE_TREE_struct * tree);

	struct ANTLR3_BASE_TREE_struct *
							(*getParent)				(struct ANTLR3_BASE_TREE_struct * tree);

	void    				(*setParent)				(struct ANTLR3_BASE_TREE_struct * tree, struct ANTLR3_BASE_TREE_struct * parent);

    ANTLR3_UINT32			(*getType)					(struct ANTLR3_BASE_TREE_struct * tree);

    void    *				(*getFirstChildWithType)	(struct ANTLR3_BASE_TREE_struct * tree, ANTLR3_UINT32 type);

    ANTLR3_UINT32			(*getLine)					(struct ANTLR3_BASE_TREE_struct * tree);

    pANTLR3_STRING			(*getText)					(struct ANTLR3_BASE_TREE_struct * tree);

    ANTLR3_BOOLEAN			(*isNilNode)					(struct ANTLR3_BASE_TREE_struct * tree);

    void					(*setChild)					(struct ANTLR3_BASE_TREE_struct * tree, ANTLR3_UINT32 i, void * child);

    pANTLR3_STRING			(*toStringTree)				(struct ANTLR3_BASE_TREE_struct * tree);

    pANTLR3_STRING			(*toString)					(struct ANTLR3_BASE_TREE_struct * tree);

	void					(*freshenPACIndexesAll)		(struct ANTLR3_BASE_TREE_struct * tree);

	void					(*freshenPACIndexes)		(struct ANTLR3_BASE_TREE_struct * tree, ANTLR3_UINT32 offset);

    void    				(*free)						(struct ANTLR3_BASE_TREE_struct * tree);

}
    ANTLR3_BASE_TREE;

#ifdef __cplusplus
}
#endif


#endif
