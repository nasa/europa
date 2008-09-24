/// \file
/// Provides implementations of string (or memory) streams as input
/// for ANLTR3 lexers.
///
#include    <antlr3.h>

/// \brief Create an in-place ASCII string stream as input to ANTLR 3.
///
/// An in-place string steam is the preferred method of supplying strings to ANTLR as input 
/// for lexing and compiling. This is because we make no copies of the input string but
/// read from it right where it is.
///
/// \param[in] inString	Pointer to the string to be used as the input stream
/// \param[in] size	Size (in 8 bit ASCII characters) of the input string
/// \param[in] name	NAme to attach the input stream (can be NULL pointer)
///
/// \return
///	- Pointer to new input stream context upon success
///	- One of the ANTLR3_ERR_ defines on error.
///
/// \remark
///  - ANTLR does not alter the input string in any way.
///  - String is slightly incorrect in that the passed in pointer can be to any
///    memory in C version of ANTLR3 of course.
////
ANTLR3_API pANTLR3_INPUT_STREAM	
antlr3NewAsciiStringInPlaceStream   (pANTLR3_UINT8 inString, ANTLR3_UINT32 size, pANTLR3_UINT8 name)
{
	// Pointer to the input stream we are going to create
	//
	pANTLR3_INPUT_STREAM    input;

	// Allocate memory for the input stream structure
	//
	input   = (pANTLR3_INPUT_STREAM)
					ANTLR3_MALLOC(sizeof(ANTLR3_INPUT_STREAM));

	if	(input == NULL)
	{
		return	NULL;
	}

	// Structure was allocated correctly, now we can install the pointer.
	//
	input->isAllocated	= ANTLR3_FALSE;
	input->data			= inString;
	input->sizeBuf		= size;

	// Call the common 8 bit ASCII input stream handler initializer.
	//
	antlr3AsciiSetupStream(input, ANTLR3_CHARSTREAM);

	// Now we can set up the file name
	//
	input->istream->streamName	= input->strFactory->newStr(input->strFactory, name == NULL ? (pANTLR3_UINT8)"-memory-" : name);
	input->fileName				= input->istream->streamName;

	return  input;
}

/// \brief Create an in-place UCS2 string stream as input to ANTLR 3.
///
/// An in-place string steam is the preferred method of supplying strings to ANTLR as input 
/// for lexing and compiling. This is because we make no copies of the input string but
/// read from it right where it is.
///
/// \param[in] inString	Pointer to the string to be used as the input stream
/// \param[in] size	Size (in 16 bit ASCII characters) of the input string
/// \param[in] name	Name to attach the input stream (can be NULL pointer)
///
/// \return
///	- Pointer to new input stream context upon success
///	- One of the ANTLR3_ERR_ defines on error.
///
/// \remark
///  - ANTLR does not alter the input string in any way.
///  - String is slightly incorrect in that the passed in pointer can be to any
///    memory in C version of ANTLR3 of course.
////
ANTLR3_API pANTLR3_INPUT_STREAM	
antlr3NewUCS2StringInPlaceStream   (pANTLR3_UINT16 inString, ANTLR3_UINT32 size, pANTLR3_UINT16 name)
{
	// Pointer to the input stream we are going to create
	//
	pANTLR3_INPUT_STREAM    input;

	// Layout default file name string in correct encoding
	//
	ANTLR3_UINT16   defaultName[] = { '-', 'm', 'e', 'm', 'o', 'r', 'y', '-', '\0' };

	// Allocate memory for the input stream structure
	//
	input   = (pANTLR3_INPUT_STREAM)
					ANTLR3_MALLOC(sizeof(ANTLR3_INPUT_STREAM));

	if	(input == NULL)
	{
		return	NULL;
	}

	// Structure was allocated correctly, now we can install the pointer.
	//
	input->isAllocated	= ANTLR3_FALSE;
	input->data			= inString;
	input->sizeBuf		= size;

	// Call the common 16 bit input stream handler initializer.
	//
	antlr3UCS2SetupStream   (input, ANTLR3_CHARSTREAM);

	input->istream->streamName	= input->strFactory->newStr(input->strFactory, name == NULL ? (pANTLR3_UINT8)defaultName : (pANTLR3_UINT8)name);
	input->fileName				= input->istream->streamName;


	return  input;
}

/// \brief Create an ASCII string stream as input to ANTLR 3, copying the input string.
///
/// This string stream first makes a copy of the string at the supplied pointer
///
/// \param[in] inString	Pointer to the string to be copied as the input stream
/// \param[in] size	Size (in 8 bit ASCII characters) of the input string
/// \param[in] name	NAme to attach the input stream (can be NULL pointer)
///
/// \return
///	- Pointer to new input stream context upon success
///	- One of the ANTLR3_ERR_ defines on error.
///
/// \remark
///  - ANTLR does not alter the input string in any way.
///  - String is slightly incorrect in that the passed in pointer can be to any
///    memory in C version of ANTLR3 of course.
////
pANTLR3_INPUT_STREAM	antlr3NewAsciiStringCopyStream	    (pANTLR3_UINT8 inString, ANTLR3_UINT32 size, pANTLR3_UINT8 name)
{
	// Pointer to the input stream we are going to create
	//
	pANTLR3_INPUT_STREAM    input;

	// Allocate memory for the input stream structure
	//
	input   = (pANTLR3_INPUT_STREAM)
		ANTLR3_MALLOC(sizeof(ANTLR3_INPUT_STREAM));

	if	(input == NULL)
	{
		return	NULL;
	}

	// Indicate that we allocated this input and allocate it
	//
	input->isAllocated	    = ANTLR3_TRUE;
	input->data		    = ANTLR3_MALLOC((size_t)size);

	if	(input->data == NULL)
	{
		return		NULL;
	}

	// Structure was allocated correctly, now we can install the pointer and set the size.
	//
	ANTLR3_MEMMOVE(input->data, (const void *)inString, size);
	input->sizeBuf  = size;

	// Call the common 8 bit ASCII input stream handler
	// initializer type thingy doobry function.
	//
	antlr3AsciiSetupStream(input, ANTLR3_CHARSTREAM);


	input->istream->streamName	= input->strFactory->newStr(input->strFactory, name == NULL ? (pANTLR3_UINT8)"-memory-" : name);
	input->fileName				= input->istream->streamName;

	return  input;
}
