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
#include "Backwards/Engine/ConstantsSingleton.h"

#include "Backwards/Engine/Statement.h"

#include <cmath>

namespace Backwards
 {

namespace Engine
 {

   ConstantsSingleton::ConstantsSingleton() :
      FLOAT_ZERO(std::make_shared<Types::FloatValue>(dm_double_fromdouble(0.0))),
      FLOAT_ONE(std::make_shared<Types::FloatValue>(dm_double_fromdouble(1.0))),
      EMPTY_ARRAY(std::make_shared<Types::ArrayValue>()),
      EMPTY_DICTIONARY(std::make_shared<Types::DictionaryValue>()),
      ONE_TRUE_NOP(std::make_shared<NOP>(Input::Token())),
      FLOAT_NAN(std::make_shared<Types::FloatValue>(dm_double_fromdouble(std::nan(""))))
    {
    }

   ConstantsSingleton& ConstantsSingleton::getInstance()
    {
      static ConstantsSingleton instance;
      return instance;
    }

 } // namespace Engine

 } // namespace Backwards
