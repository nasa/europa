/** \file
 *  Abstraction of Common tree to provide payload and string representation of node.
 *
 * \todo May not need this in the end
 */

#ifndef	ANTLR3_PARSETREE_H
#define	ANTLR3_PARSETREE_H

#include    <antlr3basetree.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ANTLR3_PARSE_TREE_struct
{
    /** Any interface that implements methods in this interface
     *  may need to point back to itself using this pointer to its
     *  super structure.
     */
    void    * super;

    /** The payload that the parse tree node passes around
     */
    void    * payload;

    /** An encapsulated BASE TREE strcuture (NOT a pointer)
      * that perfoms a lot of the dirty work of node management
      */
    ANTLR3_BASE_TREE	    baseTree;

    /** How to dup this node
     */
    pANTLR3_BASE_TREE	    (*dupNode)	(struct ANTLR3_PARSE_TREE_struct * tree);

    /** Return the type of this node
     */
    ANTLR3_UINT32	    (*getType)	(struct ANTLR3_PARSE_TREE_struct * tree);

    /** Return the string representation of the payload (must be installed
     *  when the payload is added and point to a function that knwos how to 
     *  manifest a pANTLR3_STRING from a node.
     */
    pANTLR3_STRING	    (*toString)	(struct ANTLR3_PARSE_TREE_struct * payload);

    void		    (*free)	(struct ANTLR3_PARSE_TREE_struct * tree);

}
    ANTLR3_PARSE_TREE;

#ifdef __cplusplus
}
#endif

#endif
