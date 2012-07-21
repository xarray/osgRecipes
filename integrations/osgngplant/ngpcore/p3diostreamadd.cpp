/***************************************************************************

 Copyright (c) 2007 Sergey Prokhorchuk.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. Neither the name of the author nor the names of contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

***************************************************************************/

#include <string.h>

#include <ngpcore/p3diostream.h>
#include <ngpcore/p3diostreamadd.h>

                   P3DInputStringStreamString::P3DInputStringStreamString
                                      (const char         *Str)
 {
  DataSize = strlen(Str);

  Data = new char[DataSize + 1];

  strcpy(Data,Str);

  Pos = 0;
 }

                   P3DInputStringStreamString::~P3DInputStringStreamString
                                      ()
 {
  delete [] Data;
 }

void               P3DInputStringStreamString::ReadString
                                      (char               *Buffer,
                                       unsigned int        BufferSize)
 {
  bool                                 Done;
  unsigned int                         Index;
  char                                 CurrChar;

  if (BufferSize < 2)
   {
    throw P3DExceptionAssert();
   }

  Done  = false;
  Index = 0;

  while (!Done)
   {
    if (Pos >= DataSize)
     {
      Buffer[Index] = 0;
      Done          = true;
     }
    else
     {
      CurrChar = Data[Pos];
      Pos++;

      switch (CurrChar)
       {
        case ('\r')  :
         {
          /* skip it */
         } break;

        case ('\n')  :
         {
          Buffer[Index] = 0;
          Done          = true;

          Index++;
         } break;

        default      :
         {
          Buffer[Index] = CurrChar;
          Index++;

          if (Index < (BufferSize - 1))
           {
           }
          else
           {
            Buffer[Index] = 0;

            Done = true;
           }
         }
       }
     }
   }
 }

bool               P3DInputStringStreamString::Eof
                                      () const
 {
  return(Pos >= DataSize);
 }


