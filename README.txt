KK   KK MMM    MMM LL      
KK  KK  MMMM  MMMM LL      
KKKKK   MM MMMM MM LL      
KK  KK  MM  MM  MM LL      
KK   KK MM      MM LLLLLL

 - Memory Layout -

Memory layout is defined by a bit string specification. The initial byte has the
following layout:

 MSB               LSB
 +---------+---------+
 | A B B B | C C C C |
 +---------+---------+

 A - the multiple flag, if this is set the following bits represent a multiplier
     that is applied to the type specified

 B - the field type, the following values are supported:

     001 - plain data
     010 - a data structure
     011 - a variable-length array
     111 - pointer

 C - the field alignment, the following values are supported:

     0000 - a byte-aligned field (e.g. char, bool)
     0001 - a two-byte aligned field (e.g. wchar_t, short int)
     0010 - a four-byte aligned field (e.g. long int, float)
     0011 - an eight-byte aligned field (e.g. double)
     1111 - a pointer-aligned field (e.g. void *, size_t)

Multiple Flag

If the multiple flag is set on a byte, the next bytes form the count using the
following bit pattern:

 MSB               LSB
 +---------+---------+
 | A B B B | B B B B |
 +---------+---------+

 A - the multiple flag, if it is set, shift the count up by 7 bits and repeat
     until the flag is not set

 B - the length field, this may only be 000-0000 if the multiple flag is set or
     this is the final bit after one or more bytes with the multiple flag set

Plain Data

The specification of plain data is minimal, but the combination of the multiple
flag and the width is flexible enough to meet most needs. Interacting with the
data is outside of the purpose of the memory layout spec, and the memory system
does not need to know whether the bytes it is reserving are ints/floats, signed/
unsigned or any other fact that would be required to pick instructions to
operate on the data. If, like in some implementations of double, the alignment
does not match the size of the data, the multiple flag may be utilized to give
the memory system more freedom, or the larger alignment may be specified for
simplicity.

Data Structures

Data structures must have the value of the alignment fields set to 0000.  The
shape of the structure is defined by reading bytes until a NULL byte is found,
at which point the structure ends.

Arrays

Arrays must not have the multiple flag set. The element type of the array is
defined by the next byte of the spec string (or multiple bytes if the byte
specifies the multiple flag or the data structure or array types). The value of
the align field is used to specify the width of the field that will hold the
length of the array. This length field will be added to the header of the
resulting allocation and must be passed in separately when an array is
allocated. If an allocation has multiple variable-length arrays, it may be
possible to re-configure the lengths as long as the new lengths result in an
object of a size less than or equal to the initial allocation.
