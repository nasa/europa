/** \file
 * Definition of the ANTLR3 common tree adaptor.
 */

#ifndef	_ANTLR3_COMMON_TREE_ADAPTOR_H
#define	_ANTLR3_COMMON_TREE_ADAPTOR_H

#include    <antlr3defs.h>
#include    <antlr3collections.h>
#include    <antlr3string.h>
#include    <antlr3basetreeadaptor.h>
#include    <antlr3commontree.h>
#include	<antlr3debugeventlistener.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef	struct ANTLR3_COMMON_TREE_ADAPTOR_struct
{
    /** Any enclosing structure/class can use this pointer to point to its own interface.
     */
    void    * super;

    /** Base interface implementation, embedded structure
     */
    ANTLR3_TREE_ADAPTOR	baseAdaptor;

    /** Tree factory for producing new nodes as required without needing to track
     *  memory allocation per node.
     */
    pANTLR3_ARBORETUM	arboretum;

}
    ANTLR3_COMMON_TREE_ADAPTOR;

#ifdef __cplusplus
}
#endif

#endif
