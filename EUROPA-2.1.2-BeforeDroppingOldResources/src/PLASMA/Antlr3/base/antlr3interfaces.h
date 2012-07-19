/** \file
 * Declarations for all the antlr3 C runtime interfaces/classes. This
 * allows the structures that define the interfaces to contain pointers to
 * each other without trying to sort out the cyclic interdependencies that
 * would otherwise result.
 */
#ifndef	_ANTLR3_INTERFACES_H
#define	_ANTLR3_INTERFACES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef	struct ANTLR3_INT_STREAM_struct						*pANTLR3_INT_STREAM;

/// Pointer to an instantiation of the 'class' #ANTLR3_BASE_RECOGNIZER
/// \ingroup ANTLR3_BASE_RECOGNIZER
///
typedef struct ANTLR3_BASE_RECOGNIZER_struct				*pANTLR3_BASE_RECOGNIZER;
/// Pointer to an instantiation of 'class' #ANTLR3_RECOGNIZER_SHARED_STATE		
/// \ingroup ANTLR3_RECOGNIZER_SHARED_STATE		
///
typedef	struct ANTLR3_RECOGNIZER_SHARED_STATE_struct		*pANTLR3_RECOGNIZER_SHARED_STATE;

/// Pointer to an instantiation of 'class' #ANTLR3_BITSET_LIST
/// \ingroup ANTLR3_BITSET_LIST
///
typedef struct ANTLR3_BITSET_LIST_struct					*pANTLR3_BITSET_LIST;

/// Pointer to an instantiation of 'class' #ANTLR3_BITSET							
/// \ingroup ANTLR3_BITSET							
///
typedef struct ANTLR3_BITSET_struct							*pANTLR3_BITSET;

/// Pointer to an instantiation of 'class' #ANTLR3_TOKEN_FACTORY					
/// \ingroup ANTLR3_TOKEN_FACTORY					
///
typedef struct ANTLR3_TOKEN_FACTORY_struct					*pANTLR3_TOKEN_FACTORY;
/// Pointer to an instantiation of 'class' #ANTLR3_COMMON_TOKEN					
/// \ingroup ANTLR3_COMMON_TOKEN					
///
typedef struct ANTLR3_COMMON_TOKEN_struct					*pANTLR3_COMMON_TOKEN;

/// Pointer to an instantiation of 'class' #ANTLR3_EXCEPTION						
/// \ingroup ANTLR3_EXCEPTION						
///
typedef struct ANTLR3_EXCEPTION_struct						*pANTLR3_EXCEPTION;

/// Pointer to an instantiation of 'class' #ANTLR3_HASH_BUCKET					
/// \ingroup ANTLR3_HASH_BUCKET					
///
typedef struct ANTLR3_HASH_BUCKET_struct					*pANTLR3_HASH_BUCKET;
/// Pointer to an instantiation of 'class' #ANTLR3_HASH_ENTRY						
/// \ingroup ANTLR3_HASH_ENTRY						
///
typedef struct ANTLR3_HASH_ENTRY_struct						*pANTLR3_HASH_ENTRY;
/// Pointer to an instantiation of 'class' #ANTLR3_HASH_ENUM						
/// \ingroup ANTLR3_HASH_ENUM						
///
typedef struct ANTLR3_HASH_ENUM_struct						*pANTLR3_HASH_ENUM;
/// Pointer to an instantiation of 'class' #ANTLR3_HASH_TABLE						
/// \ingroup ANTLR3_HASH_TABLE						
///
typedef struct ANTLR3_HASH_TABLE_struct						*pANTLR3_HASH_TABLE;

/// Pointer to an instantiation of 'class' #ANTLR3_LIST							
/// \ingroup ANTLR3_LIST							
///
typedef struct ANTLR3_LIST_struct							*pANTLR3_LIST;
/// Pointer to an instantiation of 'class' #ANTLR3_VECTOR_FACTORY					
/// \ingroup ANTLR3_VECTOR_FACTORY					
///
typedef struct ANTLR3_VECTOR_FACTORY_struct					*pANTLR3_VECTOR_FACTORY;
/// Pointer to an instantiation of 'class' #ANTLR3_VECTOR							
/// \ingroup ANTLR3_VECTOR							
///
typedef struct ANTLR3_VECTOR_struct							*pANTLR3_VECTOR;
/// Pointer to an instantiation of 'class' #ANTLR3_STACK							
/// \ingroup ANTLR3_STACK							
///
typedef struct ANTLR3_STACK_struct							*pANTLR3_STACK;

/// Pointer to an instantiation of 'class' #ANTLR3_INPUT_STREAM					
/// \ingroup ANTLR3_INPUT_STREAM					
///
typedef struct ANTLR3_INPUT_STREAM_struct					*pANTLR3_INPUT_STREAM;
/// Pointer to an instantiation of 'class' #ANTLR3_LEX_STATE						
/// \ingroup ANTLR3_LEX_STATE						
///
typedef struct ANTLR3_LEX_STATE_struct						*pANTLR3_LEX_STATE;

/// Pointer to an instantiation of 'class' #ANTLR3_STRING_FACTORY					
/// \ingroup ANTLR3_STRING_FACTORY					
///
typedef struct ANTLR3_STRING_FACTORY_struct					*pANTLR3_STRING_FACTORY;
/// Pointer to an instantiation of 'class' #ANTLR3_STRING							
/// \ingroup ANTLR3_STRING							
///
typedef struct ANTLR3_STRING_struct							*pANTLR3_STRING;

/// Pointer to an instantiation of 'class' #ANTLR3_TOKEN_SOURCE					
/// \ingroup ANTLR3_TOKEN_SOURCE					
///
typedef struct ANTLR3_TOKEN_SOURCE_struct					*pANTLR3_TOKEN_SOURCE;
/// Pointer to an instantiation of 'class' #ANTLR3_TOKEN_STREAM					
/// \ingroup ANTLR3_TOKEN_STREAM					
///
typedef	struct ANTLR3_TOKEN_STREAM_struct					*pANTLR3_TOKEN_STREAM;
/// Pointer to an instantiation of 'class' #ANTLR3_COMMON_TOKEN_STREAM			
/// \ingroup ANTLR3_COMMON_TOKEN_STREAM			
///
typedef	struct ANTLR3_COMMON_TOKEN_STREAM_struct			*pANTLR3_COMMON_TOKEN_STREAM;

/// Pointer to an instantiation of 'class' #ANTLR3_CYCLIC_DFA						
/// \ingroup ANTLR3_CYCLIC_DFA						
///
typedef struct ANTLR3_CYCLIC_DFA_struct						*pANTLR3_CYCLIC_DFA;

/// Pointer to an instantiation of 'class' #ANTLR3_LEXER							
/// \ingroup ANTLR3_LEXER							
///
typedef	struct ANTLR3_LEXER_struct							*pANTLR3_LEXER;
/// Pointer to an instantiation of 'class' #ANTLR3_PARSER							
/// \ingroup ANTLR3_PARSER							
///
typedef struct ANTLR3_PARSER_struct							*pANTLR3_PARSER;

/// Pointer to an instantiation of 'class' #ANTLR3_BASE_TREE						
/// \ingroup ANTLR3_BASE_TREE						
///
typedef	struct ANTLR3_BASE_TREE_struct						*pANTLR3_BASE_TREE;
/// Pointer to an instantiation of 'class' #ANTLR3_COMMON_TREE					
/// \ingroup ANTLR3_COMMON_TREE					
///
typedef struct ANTLR3_COMMON_TREE_struct					*pANTLR3_COMMON_TREE;
/// Pointer to an instantiation of 'class' #ANTLR3_ARBORETUM						
/// \ingroup ANTLR3_ARBORETUM						
///
typedef	struct ANTLR3_ARBORETUM_struct						*pANTLR3_ARBORETUM;
/// Pointer to an instantiation of 'class' #ANTLR3_PARSE_TREE						
/// \ingroup ANTLR3_PARSE_TREE						
///
typedef	struct ANTLR3_PARSE_TREE_struct						*pANTLR3_PARSE_TREE;

/// Pointer to an instantiation of 'class' #ANTLR3_TREE_NODE_STREAM				
/// \ingroup ANTLR3_TREE_NODE_STREAM				
///
typedef struct ANTLR3_TREE_NODE_STREAM_struct				*pANTLR3_TREE_NODE_STREAM;
/// Pointer to an instantiation of 'class' #ANTLR3_COMMON_TREE_NODE_STREAM		
/// \ingroup ANTLR3_COMMON_TREE_NODE_STREAM		
///
typedef	struct ANTLR3_COMMON_TREE_NODE_STREAM_struct		*pANTLR3_COMMON_TREE_NODE_STREAM;
/// Pointer to an instantiation of 'class' #ANTLR3_TREE_WALK_STATE				
/// \ingroup ANTLR3_TREE_WALK_STATE				
///
typedef struct ANTLR3_TREE_WALK_STATE_struct				*pANTLR3_TREE_WALK_STATE;

/// Pointer to an instantiation of 'class' #ANTLR3_BASE_TREE_ADAPTOR				
/// \ingroup ANTLR3_BASE_TREE_ADAPTOR				
///
typedef struct ANTLR3_BASE_TREE_ADAPTOR_struct				*pANTLR3_BASE_TREE_ADAPTOR;
/// Pointer to an instantiation of 'class' #ANTLR3_COMMON_TREE_ADAPTOR			
/// \ingroup ANTLR3_COMMON_TREE_ADAPTOR			
///
typedef	struct ANTLR3_COMMON_TREE_ADAPTOR_struct			*pANTLR3_COMMON_TREE_ADAPTOR;

/// Pointer to an instantiation of 'class' #ANTLR3_TREE_PARSER					
/// \ingroup ANTLR3_TREE_PARSER					
///
typedef struct ANTLR3_TREE_PARSER_struct					*pANTLR3_TREE_PARSER;

/// Pointer to an instantiation of 'class' #ANTLR3_INT_TRIE						
/// \ingroup ANTLR3_INT_TRIE						
///
typedef struct ANTLR3_INT_TRIE_struct						*pANTLR3_INT_TRIE;

/// Pointer to an instantiation of 'class' #ANTLR3_REWRITE_RULE_ELEMENT_STREAM	
/// \ingroup ANTLR3_REWRITE_RULE_ELEMENT_STREAM	
///
typedef struct ANTLR3_REWRITE_RULE_ELEMENT_STREAM_struct	*pANTLR3_REWRITE_RULE_ELEMENT_STREAM;
/// Pointer to an instantiation of 'class' #ANTLR3_REWRITE_RULE_ELEMENT_STREAM	
/// \ingroup ANTLR3_REWRITE_RULE_ELEMENT_STREAM	
///
typedef	struct ANTLR3_REWRITE_RULE_ELEMENT_STREAM_struct	*pANTLR3_REWRITE_RULE_TOKEN_STREAM;

/// Pointer to an instantiation of 'class' #ANTLR3_REWRITE_RULE_SUBSTREE_STREAM	
/// \ingroup ANTLR3_REWRITE_RULE_SUBTREE_STREAM	
///
typedef	struct ANTLR3_REWRITE_RULE_ELEMENT_STREAM_struct	*pANTLR3_REWRITE_RULE_SUBTREE_STREAM;

/// Pointer to an instantiation of 'class' #ANTLR3_REWRITE_RULE_NODE_STREAM	
/// \ingroup ANTLR3_REWRITE_RULE_NODE_STREAM	
///
typedef	struct ANTLR3_REWRITE_RULE_ELEMENT_STREAM_struct	*pANTLR3_REWRITE_RULE_NODE_STREAM;

/// Pointer to an instantiation of 'class' #ANTLR3_DEBUG_EVENT_LISTENER			
/// \ingroup ANTLR3_DEBUG_EVENT_LISTENER			
///
typedef struct ANTLR3_DEBUG_EVENT_LISTENER_struct			*pANTLR3_DEBUG_EVENT_LISTENER;

#ifdef __cplusplus
}
#endif

#endif
