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
#include <map>

#include "Backwards/Engine/Logger.h"
#include "Backwards/Input/StringInput.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"

#include "Forwards/Types/ValueType.h"
#include "Forwards/Types/StringValue.h"

#include "StringLogger.h"

Forwards::Engine::Cell* getCellAt(Forwards::Engine::SpreadSheet* sheet, size_t col, size_t row)
 {
   if (col < sheet->sheet.size())
    {
      if (row < sheet->sheet[col].size())
       {
         return sheet->sheet[col][row].get();
       }
    }
   return nullptr;
 }

void initCellAt(Forwards::Engine::SpreadSheet* sheet, size_t col, size_t row)
 {
   if (col >= sheet->sheet.size())
    {
      sheet->sheet.resize(col + 1U);
    }
   if (row >= sheet->sheet[col].size())
    {
      sheet->sheet[col].resize(row + 1U);
    }
   sheet->sheet[col][row] = std::make_unique<Forwards::Engine::Cell>();
 }

void removeCellAt(Forwards::Engine::SpreadSheet* sheet, size_t col, size_t row)
 {
   if (col < sheet->sheet.size())
    {
      if (row < sheet->sheet[col].size())
       {
         sheet->sheet[col][row].reset();
       }
    }
 }

int getWidth(const std::map<size_t, int>& map, size_t col, int def)
 {
   if (map.end() != map.find(col))
    {
      return map.find(col)->second;
    }
   return def;
 }

void incWidth(std::map<size_t, int>& map, size_t col, int def)
 {
   if (map.end() == map.find(col))
    {
      map[col] = def;
    }
   if (40U != map[col])
    {
      ++map[col];
    }
 }

void decWidth(std::map<size_t, int>& map, size_t col, int def)
 {
   if (map.end() == map.find(col))
    {
      map[col] = def;
    }
   if (1U != map[col])
    {
      --map[col];
    }
 }

static std::string clearLog(Backwards::Engine::Logger& logger) // throws std::bad_cast
 {
   std::string result;
   StringLogger& realLogger = dynamic_cast<StringLogger&>(logger);
   if (realLogger.logs.size() > 0U) result = realLogger.logs[0U];
   realLogger.logs.clear();
   return result;
 }

std::string computeCell(Forwards::Engine::CallingContext& context, Forwards::Parser::GetterMap& map, Forwards::Engine::Cell* cell, size_t col, size_t row)
 {
   std::string result;
   Forwards::Engine::CellFrame newFrame (cell, col, row);

   std::shared_ptr<Forwards::Engine::Expression> value = cell->value;
   if (nullptr == value.get())
    {
      Backwards::Input::StringInput interlinked (cell->currentInput);
      Forwards::Input::Lexer lexer (interlinked);
      value = Forwards::Parser::Parser::ParseFullExpression(lexer, map, *context.logger, col, row);
    }
   result = clearLog(*context.logger);

   if (nullptr == value.get())
    {
      return result;
    }

   if (false == context.inUserInput)
    {
      cell->currentInput = "";
      cell->value = value;
    }

   try
    {
      context.pushCell(&newFrame);
         // Evaluate the new cell.
      context.topCell()->cell->inEvaluation = true;
      std::shared_ptr<Forwards::Types::ValueType> newValue = value->evaluate(context);
      context.topCell()->cell->inEvaluation = false;
      if (false == context.inUserInput)
       {
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = newValue;
       }
      context.popCell();
      if (true == context.inUserInput)
       {
         result = value->toString(col, row);
       }
    }
   catch (const std::exception& e)
    {
      result = e.what();
      context.topCell()->cell->inEvaluation = false;
      context.popCell();
    }
   catch (...)
    {
      context.topCell()->cell->inEvaluation = false;
      context.popCell();
    }

   size_t c = result.find('\n');
   if (std::string::npos != c)
    {
      result = result.substr(0U, c);
    }
   return result;
 }

void recalc(Forwards::Engine::CallingContext& context, Forwards::Parser::GetterMap& map, bool c, bool t, bool l, size_t /*m*/)
 {
   context.inUserInput = false;
   ++context.generation;
   if (c) // Going in column-major order
    {
      if (l) // Going from left-to-right
       {
         if (t) // Going from top-to-bottom
          {
            size_t col = 0U;
               // > When you forget the '&' and your program decides to copy a 16 GB array.
            for (auto& column : context.theSheet->sheet)
             {
               size_t row = 0U;
               for (auto& cell : column)
                {
                  if (nullptr != cell.get())
                   {
                     if ((Forwards::Engine::LABEL == cell->type) && (nullptr == cell->value.get()))
                      {
                        cell->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>(cell->currentInput));
                      }
                     (void) computeCell(context, map, cell.get(), col, row);
                   }
                  ++row;
                }
               ++col;
             }
          }
         else // Going from bottom-to-top
          {
          }
       }
      else // Going from right-to-left
       {
         if (t) // Going from top-to-bottom
          {
          }
         else // Going from bottom-to-top
          {
          }
       }
    }
   else // Going in row major order
    {
      if (t) // Going from top-to-bottom
       {
         if (l) // Going from left-to-right
          {
          }
         else // Going from right-to-left
          {
          }
       }
      else // Going from bottom-to-top
       {
         if (l) // Going from left-to-right
          {
          }
         else // Going from right-to-left
          {
          }
       }
    }
 }
