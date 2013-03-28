// baejsn_printutil.h                                                 -*-C++-*-
#ifndef INCLUDED_BAEJSN_PRINTUTIL
#define INCLUDED_BAEJSN_PRINTUTIL

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a utility for encoding simple types in the JSON format.
//
//@CLASSES:
//  baejsn_PrintUtil: utility for printing simple types in JSON
//
//@SEE_ALSO: baejsn_encoder, baejsn_parserutil
//
//@AUTHOR: Raymond Chiu (schiu49), Rohan Bhindwale (rbhindwa)
//
//@DESCRIPTION: This component provides utility functions for encoding a
// 'bdeat' Simple type in the JSON format.  The primary method is 'printValue',
// which encodes a specified object and is overloaded for all 'bdeat' Simple
// types.  The following table describes the format in which various Simple
// types are encoded.
//..
//  Simple Type          JSON Type  Notes
//  -----------          ---------  -----
//  char                 number
//  unsigned char        number
//  int                  number
//  unsigned int         number
//  bsls::Types::Int64   number
//  bsls::Types::Uint64  number
//  float                number
//  double               number
//  char *               string
//  bsl::string          string
//  bdet_Date            string     ISO 8601 format
//  bdet_DateTz          string     ISO 8601 format
//  bdet_Time            string     ISO 8601 format
//  bdet_TimeTz          string     ISO 8601 format
//  bdet_DatetimeTz      string     ISO 8601 format
//  bdet_DatetimeTz      string     ISO 8601 format
//..
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Encoding a Simple 'struct' into JSON
///-----------------------------------------------
// Suppose we want to serialize some data into JSON.
//
// First, we define a struct, 'Employee', to contain the data:
//..
//  struct Employee {
//      const char *d_firstName;
//      const char *d_lastName;
//      int         d_age;
//  };
//..
// Then, we create an 'Employee' object and populate it with data:
//..
//  Employee john;
//  john.d_firstName = "John";
//  john.d_lastName = "Doe";
//  john.d_age = 20;
//..
//  Now, we create an output stream and manually construct the JSON string
//  using 'baejsn_PrintUtil':
//..
//  bsl::ostringstream oss;
//  oss << '{' << '\n';
//  baejsn_PrintUtil::printValue(oss, "firstName");
//  oss << ':';
//  baejsn_PrintUtil::printValue(oss, john.d_firstName);
//  oss << ',' << '\n';
//  baejsn_PrintUtil::printValue(oss, "lastName");
//  oss << ':';
//  baejsn_PrintUtil::printValue(oss, john.d_lastName);
//  oss << ',' << '\n';
//  baejsn_PrintUtil::printValue(oss, "age");
//  oss << ':';
//  baejsn_PrintUtil::printValue(oss, john.d_age);
//  oss << '\n' << '}';
//..
//  Finally, we print out the JSON string:
//..
//  if (verbose) {
//      std::cout << oss.str();
//  }
//..
//  The output should look like:
//..
//  {
//  "firstName":"John",
//  "lastName":"Doe",
//  "age":20
//  }
//..

#ifndef INCLUDED_BAESCM_VERSION
#include <baescm_version.h>
#endif

#ifndef INCLUDED_BDEPU_ISO8601
#include <bdepu_iso8601.h>
#endif

#ifndef INCLUDED_BDES_FLOAT
#include <bdes_float.h>
#endif

#ifndef INCLUDED_BSLS_TYPES
#include <bsls_types.h>
#endif

#ifndef INCLUDED_BSL_IOMANIP
#include <bsl_iomanip.h>
#endif

#ifndef INCLUDED_BSL_OSTREAM
#include <bsl_ostream.h>
#endif

#ifndef INCLUDED_BSL_STRING
#include <bsl_string.h>
#endif

#ifndef INCLUDED_BSL_LIMITS
#include <bsl_limits.h>
#endif

namespace BloombergLP {

class bdet_Date;
class bdet_Time;
class bdet_Datetime;
class bdet_DateTz;
class bdet_TimeTz;
class bdet_DatetimeTz;

                        // ======================
                        // class baejsn_PrintUtil
                        // ======================

struct baejsn_PrintUtil {
    // This 'struct' provides functions for printing objects to output streams
    // in JSON format.

    // PRIVATE CLASS METHODS
    template <class TYPE>
    static int printDateAndTime(bsl::ostream& stream, const TYPE& value);
        // Encode the specified 'value' into JSON using ISO 8601 format and
        // output the result to the specified 'stream'.

    template <typename TYPE>
    static int printFloatingPoint(bsl::ostream& stream, TYPE value);
        // Encode the specified floating point 'value' into JSON and output the
        // result to the specified 'stream'.

    static int printString(bsl::ostream&            stream,
                           const bslstl::StringRef& value);
        // Encode the specified string 'value' into JSON format and output the
        // result to the specified 'stream'.

  public:
    // CLASS METHODS
    static int printValue(bsl::ostream& stream, bool                    value);
    static int printValue(bsl::ostream& stream, char                    value);
    static int printValue(bsl::ostream& stream, signed char             value);
    static int printValue(bsl::ostream& stream, unsigned char           value);
    static int printValue(bsl::ostream& stream, short                   value);
    static int printValue(bsl::ostream& stream, unsigned short          value);
    static int printValue(bsl::ostream& stream, int                     value);
    static int printValue(bsl::ostream& stream, unsigned int            value);
    static int printValue(bsl::ostream& stream, bsls::Types::Int64      value);
    static int printValue(bsl::ostream& stream, bsls::Types::Uint64     value);
    static int printValue(bsl::ostream& stream, float                   value);
    static int printValue(bsl::ostream& stream, double                  value);
    static int printValue(bsl::ostream& stream, const char             *value);
    static int printValue(bsl::ostream& stream, const bsl::string&      value);
    static int printValue(bsl::ostream& stream, const bdet_Time&        value);
    static int printValue(bsl::ostream& stream, const bdet_Date&        value);
    static int printValue(bsl::ostream& stream, const bdet_Datetime&    value);
    static int printValue(bsl::ostream& stream, const bdet_TimeTz&      value);
    static int printValue(bsl::ostream& stream, const bdet_DateTz&      value);
    static int printValue(bsl::ostream& stream, const bdet_DatetimeTz&  value);
        // Encode the specified 'value' into JSON format and output the result
        // to the specified 'stream'.  Return 0 on success and a non-zero
        // value otherwise.
};

// ============================================================================
//                        INLINE FUNCTION DEFINITIONS
// ============================================================================

                            // -----------------------
                            // struct baejsn_PrintUtil
                            // -----------------------

// PRIVATE MANIPULATORS
template <class TYPE>
inline
int baejsn_PrintUtil::printDateAndTime(bsl::ostream& stream, const TYPE& value)
{
    char buffer[bdepu_Iso8601::BDEPU_MAX_DATETIME_STRLEN + 1];
    bdepu_Iso8601::generate(buffer, value, sizeof buffer);
    return printValue(stream, buffer);
}

template <typename TYPE>
int baejsn_PrintUtil::printFloatingPoint(bsl::ostream& stream, TYPE value)
{
    if (bdes_Float::isNan(value)
     || value == bsl::numeric_limits<TYPE>::infinity()
     || value == -bsl::numeric_limits<TYPE>::infinity()) {
        return -1;                                                    // RETURN
    }

    bsl::streamsize         prec  = stream.precision();
    bsl::ios_base::fmtflags flags = stream.flags();

    stream.precision(bsl::numeric_limits<TYPE>::digits10);

    stream << value;

    stream.precision(prec);
    stream.flags(flags);
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, bool value)
{
    stream << (value ? "true" : "false");
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, short value)
{
    stream << value;
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, int value)
{
    stream << value;
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&      stream,
                                 bsls::Types::Int64 value)
{
    stream << value;
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, unsigned char value)
{
    stream << static_cast<int>(value);
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, unsigned short value)
{
    stream << value;
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, unsigned int value)
{
    stream << value;
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&       stream,
                                 bsls::Types::Uint64 value)
{
    stream << value;
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, float value)
{
    return printFloatingPoint(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, double value)
{
    return printFloatingPoint(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, const char *value)
{
    return printString(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream& stream, char value)
{
    signed char tmp(value);  // Note that 'char' is unsigned on IBM.

    stream << static_cast<int>(tmp);
    return 0;
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&      stream,
                                 const bsl::string& value)
{
    return printString(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&    stream,
                                 const bdet_Time& value)
{
    return printDateAndTime(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&    stream,
                                 const bdet_Date& value)
{
    return printDateAndTime(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&        stream,
                                 const bdet_Datetime& value)
{
    return printDateAndTime(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&      stream,
                                 const bdet_TimeTz& value)
{
    return printDateAndTime(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&      stream,
                                 const bdet_DateTz& value)
{
    return printDateAndTime(stream, value);
}

inline
int baejsn_PrintUtil::printValue(bsl::ostream&          stream,
                                 const bdet_DatetimeTz& value)
{
    return printDateAndTime(stream, value);
}

}  // close namespace BloombergLP

#endif

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2012
//      All Rights Reserved.
//      Property of Bloomberg L.P.  (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ----------------------------------
