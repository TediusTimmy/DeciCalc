/*
BSD 3-Clause License

Copyright (c) 2023, Thomas DiModica
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Backwards/Engine/StdLib.h"

#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/ConstantsSingleton.h"

#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/CellRangeExpand.h"
#include "Backwards/Engine/CellRefEval.h"
#include "Backwards/Engine/DebuggerHook.h"
#include "Backwards/Engine/ProgrammingException.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h"
#include "Backwards/Types/NilValue.h"
#include "Backwards/Types/CellRangeValue.h"
#include "Backwards/Types/CellRefValue.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <ctime>

#include "dm_double_pretty.h"

namespace Backwards
 {

namespace Engine
 {

   //////////
   // Expression and Statement are built on these first 7 functions.
   //////////

   STDLIB_CONSTANT_DECL(NewArray)
    {
      return ConstantsSingleton::getInstance().EMPTY_ARRAY;
    }

   STDLIB_CONSTANT_DECL(NewDictionary)
    {
      return ConstantsSingleton::getInstance().EMPTY_DICTIONARY;
    }

   STDLIB_BINARY_DECL(PushBack)
    {
      if (typeid(Types::ArrayValue) == typeid(*first))
       {
         // Yes, construct a new container on modification.
         // All operations treat ValueTypes as immutable, so this is safe.
         std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
         result->value = static_cast<const Types::ArrayValue&>(*first).value;
         result->value.push_back(second);
         return result;
       }
      else
       {
         throw Types::TypedOperationException("Error pushing back to non-Array.");
       }
    }

   STDLIB_TERNARY_DECL(Insert)
    {
      if (typeid(Types::DictionaryValue) == typeid(*first))
       {
         // Yes, construct a new container on modification.
         std::shared_ptr<Types::DictionaryValue> result = std::make_shared<Types::DictionaryValue>();
         result->value = static_cast<const Types::DictionaryValue&>(*first).value;
         result->value[second] = third;
         return result;
       }
      else
       {
         throw Types::TypedOperationException("Error inserting into non-Dictionary.");
       }
    }

   STDLIB_BINARY_DECL(GetValue)
    {
      if (typeid(Types::DictionaryValue) == typeid(*first))
       {
         std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator iter =
            static_cast<const Types::DictionaryValue&>(*first).value.find(second);
         if (static_cast<const Types::DictionaryValue&>(*first).value.end() != iter)
          {
            return iter->second;
          }
         else
          {
            throw Types::TypedOperationException("Key not found in Dictionary.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to retrieve key from non-Dictionary.");
       }
    }

   STDLIB_BINARY_DECL(GetIndex)
    {
      if (typeid(Types::ArrayValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            double index = dm_double_todouble(static_cast<const Types::FloatValue&>(*second).value);
            if ((index >= 0.0) && (index < static_cast<double>(static_cast<const Types::ArrayValue&>(*first).value.size())))
             {
               return static_cast<const Types::ArrayValue&>(*first).value[static_cast<size_t>(index)];
             }
            else
             {
               throw Types::TypedOperationException("Array Index Out-of-Bounds.");
             }
          }
         else
          {
            throw Types::TypedOperationException("Error indexing with non-Float.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error indexing into non-Array.");
       }
    }

   STDLIB_TERNARY_DECL(SetIndex)
    {
      if (typeid(Types::ArrayValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            double index = dm_double_todouble(static_cast<const Types::FloatValue&>(*second).value);
            if ((index >= 0.0) && (index < static_cast<double>(static_cast<const Types::ArrayValue&>(*first).value.size())))
             {
               // Yes, construct a new container on modification.
               std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
               result->value = static_cast<const Types::ArrayValue&>(*first).value;
               result->value[static_cast<size_t>(index)] = third;
               return result;
             }
            else
             {
               throw Types::TypedOperationException("Array Index Out-of-Bounds.");
             }
          }
         else
          {
            throw Types::TypedOperationException("Error indexing with non-Float.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error indexing into non-Array.");
       }
    }

   //////////
   // These next 5 are the most basic in telling if it works.
   //////////

#define LOGGINGFUNCTIONDEFN(x,y) \
   STDLIB_UNARY_DECL_WITH_CONTEXT(x) \
    { \
      if (typeid(Types::StringValue) == typeid(*arg)) \
       { \
         context.logger->log(y + static_cast<const Types::StringValue&>(*arg).value); \
         return arg; \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error logging non-String."); \
       } \
    }

   LOGGINGFUNCTIONDEFN(Error, "ERROR: ")
   LOGGINGFUNCTIONDEFN(Warn, "WARN: ")
   LOGGINGFUNCTIONDEFN(Info, "INFO: ")

   STDLIB_UNARY_DECL_WITH_CONTEXT(Fatal)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         context.logger->log("FATAL: " + static_cast<const Types::StringValue&>(*arg).value);
         throw FatalException(static_cast<const Types::StringValue&>(*arg).value);
       }
      else
       {
         throw FatalException("Error logging non-String while trying to generate a Fatal message.");
       }
    }

   STDLIB_UNARY_DECL(ToString)
    {
      if (typeid(Types::FloatValue) == typeid(*arg))
       {
         char temp [25];
         dm_double_toprettystring(static_cast<const Types::FloatValue&>(*arg).value, temp);
         return std::make_shared<Types::StringValue>(temp);
       }
      else
       {
         throw Types::TypedOperationException("Error converting non-Float to String.");
       }
    }

   //////////
   // The other 36 functions of the Standard Library in no particular order. (Eval and EnterDebugger are not counted here.)
   //////////

   STDLIB_BINARY_DECL(PushFront)
    {
      if (typeid(Types::ArrayValue) == typeid(*first))
       {
         std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
         result->value.push_back(second);
         const std::vector<std::shared_ptr<Types::ValueType> >& source = static_cast<const Types::ArrayValue&>(*first).value;
         result->value.insert(result->value.end(), source.begin(), source.end());
         return result;
       }
      else
       {
         throw Types::TypedOperationException("Error pushing front to non-Array.");
       }
    }

   STDLIB_UNARY_DECL(PopBack)
    {
      if (typeid(Types::ArrayValue) == typeid(*arg))
       {
         if (false == static_cast<const Types::ArrayValue&>(*arg).value.empty())
          {
            std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
            result->value = static_cast<const Types::ArrayValue&>(*arg).value;
            result->value.pop_back();
            return result;
          }
         else
          {
            throw Types::TypedOperationException("Error popping back of empty Array.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error popping back of non-Array.");
       }
    }

   STDLIB_UNARY_DECL(PopFront)
    {
      if (typeid(Types::ArrayValue) == typeid(*arg))
       {
         if (false == static_cast<const Types::ArrayValue&>(*arg).value.empty())
          {
            std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
            const std::vector<std::shared_ptr<Types::ValueType> >& source = static_cast<const Types::ArrayValue&>(*arg).value;
            result->value.assign(source.begin() + 1, source.end());
            return result;
          }
         else
          {
            throw Types::TypedOperationException("Error popping front of empty Array.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error popping front of non-Array.");
       }
    }

#define RTTIFUNCTIONDEFN(x,y) \
   STDLIB_UNARY_DECL(x) \
    { \
      if (typeid(y) == typeid(*arg)) \
       { \
         return ConstantsSingleton::getInstance().FLOAT_ONE; \
       } \
      else \
       { \
         return ConstantsSingleton::getInstance().FLOAT_ZERO; \
       } \
    }

   RTTIFUNCTIONDEFN(IsFloat, Types::FloatValue)
   RTTIFUNCTIONDEFN(IsString, Types::StringValue)
   RTTIFUNCTIONDEFN(IsArray, Types::ArrayValue)
   RTTIFUNCTIONDEFN(IsDictionary, Types::DictionaryValue)
   RTTIFUNCTIONDEFN(IsFunction, Types::FunctionValue)
   RTTIFUNCTIONDEFN(IsNil, Types::NilValue)
   RTTIFUNCTIONDEFN(IsCellRange, Types::CellRangeValue)
   RTTIFUNCTIONDEFN(IsCellRef, Types::CellRefValue)

   STDLIB_UNARY_DECL(Length)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         return std::make_shared<Types::FloatValue>(dm_double_fromdouble(static_cast<const Types::StringValue&>(*arg).value.size()));
       }
      else
       {
         throw Types::TypedOperationException("Error getting length of non-String.");
       }
    }

   STDLIB_UNARY_DECL(Size)
    {
      if (typeid(Types::ArrayValue) == typeid(*arg))
       {
         return std::make_shared<Types::FloatValue>(dm_double_fromdouble(static_cast<const Types::ArrayValue&>(*arg).value.size()));
       }
      else if (typeid(Types::DictionaryValue) == typeid(*arg))
       {
         return std::make_shared<Types::FloatValue>(dm_double_fromdouble(static_cast<const Types::DictionaryValue&>(*arg).value.size()));
       }
      else
       {
         throw Types::TypedOperationException("Error getting size of non-Collection.");
       }
    }

   STDLIB_BINARY_DECL(NewArrayDefault)
    {
      if (typeid(Types::FloatValue) == typeid(*first))
       {
         double size = dm_double_todouble(static_cast<const Types::FloatValue&>(*first).value);
         if ((size >= 0.0) && (size < static_cast<double>(std::numeric_limits<size_t>::max())))
          {
            std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
            result->value.resize(static_cast<size_t>(size), second);
            return result;
          }
         else
          {
            throw Types::TypedOperationException("Error creating Array size either negative or too big.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error creating Array with non-Float size.");
       }
    }

   STDLIB_TERNARY_DECL(SubString)
    {
      if (typeid(Types::StringValue) == typeid(*first))
       {
         if (typeid(Types::FloatValue) == typeid(*second))
          {
            if (typeid(Types::FloatValue) == typeid(*third))
             {
               double stringLength = static_cast<double>(static_cast<const Types::StringValue&>(*first).value.length());
               double startIndex = dm_double_todouble(static_cast<const Types::FloatValue&>(*second).value);
               double endIndex = dm_double_todouble(static_cast<const Types::FloatValue&>(*third).value);
               if ((startIndex >= 0.0) && (startIndex <= stringLength) &&
                  (endIndex >= 0.0) && (endIndex <= stringLength) &&
                  (endIndex >= startIndex))
                {
                  std::shared_ptr<Types::StringValue> result = std::make_shared<Types::StringValue>();
                  result->value = static_cast<const Types::StringValue&>(*first).value.substr(static_cast<size_t>(startIndex), static_cast<size_t>(endIndex - startIndex));
                  return result;
                }
               else
                {
                  throw Types::TypedOperationException("Error getting substring with either beginning or ending index not in String, or ending before beginning.");
                }
             }
            else
             {
               throw Types::TypedOperationException("Error getting substring with non-Float ending position.");
             }
          }
         else
          {
            throw Types::TypedOperationException("Error getting substring with non-Float starting position.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error getting substring of non-String.");
       }
    }

   STDLIB_BINARY_DECL(ContainsKey)
    {
      if (typeid(Types::DictionaryValue) == typeid(*first))
       {
         std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator iter =
            static_cast<const Types::DictionaryValue&>(*first).value.find(second);
         if (static_cast<const Types::DictionaryValue&>(*first).value.end() != iter)
          {
            return ConstantsSingleton::getInstance().FLOAT_ONE;
          }
         else
          {
            return ConstantsSingleton::getInstance().FLOAT_ZERO;
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to determine if key is in non-Dictionary.");
       }
    }

   STDLIB_BINARY_DECL(RemoveKey)
    {
      if (typeid(Types::DictionaryValue) == typeid(*first))
       {
         std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator iter =
            static_cast<const Types::DictionaryValue&>(*first).value.find(second);
         if (static_cast<const Types::DictionaryValue&>(*first).value.end() != iter)
          {
            std::shared_ptr<Types::DictionaryValue> result = std::make_shared<Types::DictionaryValue>();
            result->value = static_cast<const Types::DictionaryValue&>(*first).value;
            result->value.erase(second);
            return result;
          }
         else
          {
            throw Types::TypedOperationException("Key not found in Dictionary.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to remove key from non-Dictionary.");
       }
    }

   STDLIB_UNARY_DECL(GetKeys)
    {
      if (typeid(Types::DictionaryValue) == typeid(*arg))
       {
         std::shared_ptr<Types::ArrayValue> result = std::make_shared<Types::ArrayValue>();
         for (std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator iter =
            static_cast<const Types::DictionaryValue&>(*arg).value.begin();
            static_cast<const Types::DictionaryValue&>(*arg).value.end() != iter; ++iter)
          {
            result->value.push_back(iter->first);
          }
         return result;
       }
      else
       {
         throw Types::TypedOperationException("Error trying to get all keys from non-Dictionary.");
       }
    }

   STDLIB_CONSTANT_DECL(NaN)
    {
      return ConstantsSingleton::getInstance().FLOAT_NAN;
    }

#define MINMAXDEFN(x,y,z) \
   STDLIB_BINARY_DECL(x) \
    { \
      if (typeid(Types::FloatValue) == typeid(*first)) \
       { \
         if (typeid(Types::FloatValue) == typeid(*second)) \
          { \
            double fVal = dm_double_todouble(static_cast<const Types::FloatValue&>(*first).value); \
            double sVal = dm_double_todouble(static_cast<const Types::FloatValue&>(*second).value); \
            if (true == std::isnan(fVal)) \
             { \
               return first; \
             } \
            else if (true == std::isnan(sVal)) \
             { \
               return second; \
             } \
            return (!!y(fVal, sVal)) ? first : second; \
          } \
         else \
          { \
            throw Types::TypedOperationException("Error computing " z " with non-Float second argument."); \
          } \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error computing " z " with non-Float first argument."); \
       } \
    }

   MINMAXDEFN(Max, dm_double_isgreaterequal, "max")
   MINMAXDEFN(Min, dm_double_islessequal, "min")

#define BASICONEARGMATHDEFN(x,y,z) \
   STDLIB_UNARY_DECL(x) \
    { \
      if (typeid(Types::FloatValue) == typeid(*arg)) \
       { \
         return std::make_shared<Types::FloatValue>(y(static_cast<const Types::FloatValue&>(*arg).value)); \
       } \
      else \
       { \
         throw Types::TypedOperationException("Error trying to compute " z " of non-Float."); \
       } \
    }

   BASICONEARGMATHDEFN(Abs, dm_double_fabs, "absolute value")
   BASICONEARGMATHDEFN(Round, dm_double_round, "rounded value")
   BASICONEARGMATHDEFN(Floor, dm_double_floor, "rounded to negative infinity")
   BASICONEARGMATHDEFN(Ceil, dm_double_ceil, "rounded to positive infinity")
   BASICONEARGMATHDEFN(IsInfinity, dm_double_isinf, "is infinity")
    // Well, technically, I guess it SHOULD return true if the argument is not a Float....
   BASICONEARGMATHDEFN(IsNaN, dm_double_isnan, "is special not-a-number value")

   STDLIB_UNARY_DECL(Sqr)
    {
      if (typeid(Types::FloatValue) == typeid(*arg))
       {
         dm_double x = static_cast<const Types::FloatValue&>(*arg).value;
         return std::make_shared<Types::FloatValue>(dm_double_mul(x, x));
       }
      else
       {
         throw Types::TypedOperationException("Error trying to square non-Float.");
       }
    }

   STDLIB_UNARY_DECL(ValueOf)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         std::stringstream str (static_cast<const Types::StringValue&>(*arg).value);
         double val;
         str >> val;
         if (!str.fail() && (str.get() == std::char_traits<char>::eof()))
          {
            return std::make_shared<Types::FloatValue>(dm_double_fromstring(static_cast<const Types::StringValue&>(*arg).value.c_str()));
          }
         else
          {
            throw Types::TypedOperationException("String did not contain valid Float value.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to get value of non-String.");
       }
    }

   STDLIB_UNARY_DECL(FromCharacter)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         const std::string& str (static_cast<const Types::StringValue&>(*arg).value);
         if (1U == str.size())
          {
            return std::make_shared<Types::FloatValue>(dm_double_fromdouble(str[0]));
          }
         else
          {
            throw Types::TypedOperationException("String was not single character.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to convert non-String to Float character code point.");
       }
    }

   STDLIB_UNARY_DECL(ToCharacter)
    {
      if (typeid(Types::FloatValue) == typeid(*arg))
       {
         double val = dm_double_todouble(static_cast<const Types::FloatValue&>(*arg).value);
         if ((val > static_cast<double>(std::numeric_limits<char>::min())) &&
            (val < static_cast<double>(std::numeric_limits<char>::max())))
          {
            std::string str;
            str += static_cast<char>(val);
            return std::make_shared<Types::StringValue>(str);
          }
         else
          {
            throw Types::TypedOperationException("Float is not a valid character code point.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to convert non-Float to single character String.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(DebugPrint)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         context.logger->log(static_cast<const Types::StringValue&>(*arg).value);
         return arg;
       }
      else
       {
         throw Types::TypedOperationException("Error logging non-String.");
       }
    }

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(EnterDebugger)
    {
      if (nullptr != context.debugger)
       {
         context.debugger->EnterDebugger("", context);
       }
      return ConstantsSingleton::getInstance().FLOAT_ZERO;
    }

   STDLIB_CONSTANT_DECL(GetRoundMode)
    {
      return std::make_shared<Types::FloatValue>(dm_double_fromdouble(static_cast<double>(dm_fegetround())));
    }

   STDLIB_UNARY_DECL(SetRoundMode)
    {
      if (typeid(Types::FloatValue) == typeid(*arg))
       {
         double val = dm_double_todouble(static_cast<const Types::FloatValue&>(*arg).value);
         if ((val >= static_cast<double>(DM_FE_TONEAREST)) &&
            (val <= static_cast<double>(DM_FE_FROMZERO)))
          {
            (void) dm_fesetround(static_cast<int>(val));
            return arg;
          }
         else
          {
            throw Types::TypedOperationException("Float is not a valid rounding mode.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to convert non-Float to rounding mode.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(EvalCell)
    {
      if (typeid(Types::CellRefValue) == typeid(*arg))
       {
         try
          {
            const CellRefEval& val = dynamic_cast<CellRefEval&>(*static_cast<const Types::CellRefValue&>(*arg).value);
            return val.evaluate(context);
          }
         catch (const std::bad_cast&)
          {
            throw ProgrammingException("CellRefHolder is not a CellRefEval.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to evaluate non-CellRef.");
       }
    }

   STDLIB_UNARY_DECL_WITH_CONTEXT(ExpandRange)
    {
      if (typeid(Types::CellRangeValue) == typeid(*arg))
       {
         try
          {
            const CellRangeExpand& val = dynamic_cast<CellRangeExpand&>(*static_cast<const Types::CellRangeValue&>(*arg).value);
            return val.expand(context);
          }
         catch (const std::bad_cast&)
          {
            throw ProgrammingException("CellRangeHolder is not a CellRangeExpand.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error trying to expand non-CellRange.");
       }
    }

 } // namespace Engine

 } // namespace Backwards
