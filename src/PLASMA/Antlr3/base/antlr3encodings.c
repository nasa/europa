/** \File 
 * Provides basic utility functions to convert between
 * the various Unicode character conversions. There are of
 * course various packages that could be used instead of these
 * functions, but then the Antlr 3 C runtime would be dependant
 * on the particular package. Using ICU for this is a good idea if
 * your project is already dependant on it.
 */
#include    <antlr3.h>

/// Convert 8 bit character to ANTLR char form.
///
/// \param[in] inc Input character to transform in 8 bit ASCII form.
/// \return ANTLR3_UCHAR encoding of the character.
///
ANTLR3_API 
ANTLR3_UCHAR antlr3c8toAntlrc(ANTLR3_INT8 inc)
{
    return  (ANTLR3_UCHAR)(inc);
}
