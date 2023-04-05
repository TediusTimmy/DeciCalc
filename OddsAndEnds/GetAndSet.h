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
#ifndef GETANDSET_H
#define GETANDSET_H

Forwards::Engine::Cell* getCellAt(Forwards::Engine::SpreadSheet*, size_t col, size_t row);
void initCellAt(Forwards::Engine::SpreadSheet*, size_t col, size_t row);
void removeCellAt(Forwards::Engine::SpreadSheet*, size_t col, size_t row);

std::string computeCell(Forwards::Engine::CallingContext&, Forwards::Parser::GetterMap&, Forwards::Engine::Cell*, size_t col, size_t row);
void recalc(Forwards::Engine::CallingContext&, Forwards::Parser::GetterMap& map, bool c, bool t, bool l, size_t m);

int getWidth(const std::map<size_t, int>& map, size_t col, int def);
void incWidth(std::map<size_t, int>& map, size_t col, int def);
void decWidth(std::map<size_t, int>& map, size_t col, int def);

#endif /* GETANDSET_H */
