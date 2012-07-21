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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <ngpcore/p3dcompat.h> /* for snprintf definition in MSVC environment */
#include <ngpcore/p3dexcept.h>
#include <ngpcore/p3diostream.h>


typedef struct
 {
  char            *Old;
 } P3DLocaleInfo;

static void        SetLocaleNumericStd(P3DLocaleInfo      *OldLocale)
 {
  OldLocale->Old = strdup(setlocale(LC_NUMERIC,NULL));

  setlocale(LC_NUMERIC,"C");
 }

static void        RestoreLocaleNumeric
                                      (P3DLocaleInfo      *OldLocale)
 {
  setlocale(LC_NUMERIC,OldLocale->Old);

  free(OldLocale->Old);
 }

const char        *P3DExceptionIO::GetMessage
                                      () const
 {
  return("Input/output error");
 }

                   P3DInputStringFmtStream::P3DInputStringFmtStream
                                      (P3DInputStringStream
                                                          *SourceStream)
 {
  this->SourceStream = SourceStream;
 }

typedef struct
 {
  unsigned int     Start;
  unsigned int     Length;
 } P3DWordInfo;

static bool        GetWordInfoNext    (P3DWordInfo        *WordInfo,
                                       const char         *Str)
 {
  WordInfo->Start += WordInfo->Length;
  WordInfo->Length = 0;

  while ((Str[WordInfo->Start] == ' ') ||
         (Str[WordInfo->Start] == '\t'))
   {
    WordInfo->Start++;
   }

  if (Str[WordInfo->Start] == '\0')
   {
    return(false);
   }

  do
   {
    WordInfo->Length++;
   } while ((Str[WordInfo->Start + WordInfo->Length] != ' ')  &&
            (Str[WordInfo->Start + WordInfo->Length] != '\t') &&
            (Str[WordInfo->Start + WordInfo->Length] != '\0'));

  return(true);
 }


static bool        GetWordInfoFirst   (P3DWordInfo        *WordInfo,
                                       const char         *Str)
 {
  WordInfo->Start  = 0;
  WordInfo->Length = 0;

  return(GetWordInfoNext(WordInfo,Str));
 }

void               P3DInputStringFmtStream::ReadFmtStringTagged
                                      (const char         *Tag,
                                       const char         *Format,
                                       ...)
 {
  char                                 Buffer[1024];
  char                                 ValueBuffer[256];
  P3DWordInfo                          WordInfo;
  unsigned int                         FieldIndex;
  unsigned int                         FieldCount;
  va_list                              FieldValues;
  P3DLocaleInfo                        LocaleInfo;

  SetLocaleNumericStd(&LocaleInfo);

  va_start(FieldValues,Format);

  try
   {
    ReadDataString(Buffer,sizeof(Buffer));

    if (!GetWordInfoFirst(&WordInfo,Buffer))
     {
      throw P3DExceptionGeneric("invalid data file string format");
     }

    if (strlen(Tag) != WordInfo.Length)
     {
      throw P3DExceptionGeneric("invalid string tag");
     }

    if (memcmp(Tag,&Buffer[WordInfo.Start],WordInfo.Length) != 0)
     {
      throw P3DExceptionGeneric("invalid string tag");
     }

    FieldCount = strlen(Format);

    for (FieldIndex = 0; FieldIndex < FieldCount; FieldIndex++)
     {
      if (!GetWordInfoNext(&WordInfo,Buffer))
       {
        throw P3DExceptionGeneric("unsufficient value count in data file string");
       }

      switch (Format[FieldIndex])
       {
        case ('s') :
         {
          char         *Value;
          unsigned int  Size;

          Value = va_arg(FieldValues,char*);
          Size  = va_arg(FieldValues,unsigned int);

          if (WordInfo.Length >= Size)
           {
            throw P3DExceptionGeneric("string value is too large");
           }

          memcpy(Value,&Buffer[WordInfo.Start],WordInfo.Length);
          Value[WordInfo.Length] = '\0';
         } break;

        case ('u') :
         {
          if (WordInfo.Length >= sizeof(ValueBuffer))
           {
            throw P3DExceptionGeneric("unsigned int value is too large");
           }

          memcpy(ValueBuffer,&Buffer[WordInfo.Start],WordInfo.Length);
          ValueBuffer[WordInfo.Length] = '\0';

          if (sscanf(ValueBuffer,"%u",va_arg(FieldValues,unsigned int*)) < 1)
           {
            throw P3DExceptionGeneric("invalid unsigned int value");
           }
         } break;

        case ('f') :
         {
          if (WordInfo.Length >= sizeof(ValueBuffer))
           {
            throw P3DExceptionGeneric("float value is too large");
           }

          memcpy(ValueBuffer,&Buffer[WordInfo.Start],WordInfo.Length);
          ValueBuffer[WordInfo.Length] = '\0';

          if (sscanf(ValueBuffer,"%f",va_arg(FieldValues,float*)) < 1)
           {
            throw P3DExceptionGeneric("invalid float value");
           }
         } break;

        case ('b') :
         {
          if      (WordInfo.Length == 4)
           {
            if (memcmp(&Buffer[WordInfo.Start],"true",4) == 0)
             {
              *(va_arg(FieldValues,bool*)) = 1;
             }
            else
             {
              throw P3DExceptionGeneric("invalid boolean value");
             }
           }
          else if (WordInfo.Length == 5)
           {
            if (memcmp(&Buffer[WordInfo.Start],"false",5) == 0)
             {
              *(va_arg(FieldValues,bool*)) = 0;
             }
            else
             {
              throw P3DExceptionGeneric("invalid boolean value");
             }
           }
          else
           {
            throw P3DExceptionGeneric("invalid boolean value");
           }
         } break;

        default    :
         {
          throw P3DExceptionGeneric("invalid format string field type");
         }
       }
     }
   }
  catch (...)
   {
    va_end(FieldValues);

    RestoreLocaleNumeric(&LocaleInfo);

    throw;
   }

  va_end(FieldValues);

  RestoreLocaleNumeric(&LocaleInfo);
 }

void               P3DInputStringFmtStream::ReadDataString
                                      (char               *Buffer,
                                       unsigned int        BufferSize)
 {
  /*FIXME: skip empty lines here */
  SourceStream->ReadString(Buffer,BufferSize);
 }

                   P3DInputStringStreamFile::P3DInputStringStreamFile
                                      ()
 {
  Source = NULL;
 }

                   P3DOutputStringFmtStream::P3DOutputStringFmtStream
                                      (P3DOutputStringStream
                                                          *Target)
 {
  this->Target = Target;
 }

void               P3DOutputStringFmtStream::WriteString
                                      (const char         *Buffer)
 {
  Target->WriteString(Buffer);
 }

void               P3DOutputStringFmtStream::WriteString
                                      (const char         *Format,
                                       ...)
 {
  unsigned int                         FieldCount;
  unsigned int                         FieldIndex;
  char                                 FieldType;
  va_list                              FieldValues;
  char                                 Buffer[255 + 1];
  P3DLocaleInfo                        LocaleInfo;

  SetLocaleNumericStd(&LocaleInfo);

  va_start(FieldValues,Format);

  try
   {
    Target->AutoLnDisable();

    FieldCount = strlen(Format);

    for (FieldIndex = 0; FieldIndex < FieldCount; FieldIndex++)
     {
      if (FieldIndex > 0)
       {
        Target->WriteString(" ");
       }

      FieldType = Format[FieldIndex];

      switch (FieldType)
       {
        case ('s') :
         {
          /*FIXME: escape \" chars and append \" at start and end of line */
          Target->WriteString(va_arg(FieldValues,char*));
         } break;

        case ('u') :
         {
          if (snprintf(Buffer,sizeof(Buffer),"%u",va_arg(FieldValues,unsigned int)) >= (int)(sizeof(Buffer)))
           {
            throw P3DExceptionGeneric("unsigned int value string representation is too large");
           }

          Target->WriteString(Buffer);
         } break;

        case ('f') :
         {
          if (snprintf(Buffer,sizeof(Buffer),"%f",(double)va_arg(FieldValues,double)) >= (int)(sizeof(Buffer)))
           {
            throw P3DExceptionGeneric("float value string representation is too large");
           }

          Target->WriteString(Buffer);
         } break;

        case ('b') :
         {
          if ((bool)va_arg(FieldValues,int))
           {
            Target->WriteString("true");
           }
          else
           {
            Target->WriteString("false");
           }
         } break;

        default    :
         {
          throw P3DExceptionGeneric("invalid format string field type");
         }
       }
     }
   }
  catch (...)
   {
    va_end(FieldValues);

    RestoreLocaleNumeric(&LocaleInfo);

    Target->AutoLnEnable();

    throw;
   }

  va_end(FieldValues);

  RestoreLocaleNumeric(&LocaleInfo);

  Target->AutoLnEnable();
  Target->WriteString("");
 }

                   P3DInputStringStreamFile::~P3DInputStringStreamFile
                                      ()
 {
  if (Source != NULL)
   {
    Close();
   }
 }

void               P3DInputStringStreamFile::Open
                                      (const char         *FileName)
 {
  if (Source != NULL)
   {
    Close();
   }

  Source = fopen(FileName,"rb");

  if (Source == NULL)
   {
    throw P3DExceptionIO();
   }
 }

void               P3DInputStringStreamFile::Close
                                      ()
 {
  if (Source != NULL)
   {
    fclose(Source);

    Source = NULL;
   }
 }

void               P3DInputStringStreamFile::ReadString
                                      (char               *Buffer,
                                       unsigned int        BufferSize)
 {
  bool                                 Ready;
  int                                  CurrChar;
  unsigned int                         Index;

  /*FIXME: slow and bruteforced, must be rewritten ;) */

  if ((Source == NULL) || (BufferSize < 2))
   {
    throw P3DExceptionAssert();
   }

  Ready  = false;
  Index  = 0;

  while (!Ready)
   {
    CurrChar = fgetc(Source);

    if      (CurrChar == EOF)
     {
      if (feof(Source))
       {
        Buffer[Index] = 0;
        Ready = true;
       }
      else
       {
        throw P3DExceptionIO();
       }
     }
    else if (CurrChar == '\r')
     {
      /* ignore it */
     }
    else if (CurrChar == '\n')
     {
      Buffer[Index] = 0;

      Ready = true;
     }
    else
     {
      Buffer[Index] = (unsigned char)CurrChar;

      Index++;

      if (Index < (BufferSize - 1))
       {
       }
      else
       {
        Buffer[Index] = 0;

        Ready = true;
       }
     }
   }
 }

bool               P3DInputStringStreamFile::Eof
                                      () const
 {
  int                                  TempChar;

  if (Source == NULL)
   {
    throw P3DExceptionAssert();
   }

  TempChar = fgetc(Source);
  ungetc(TempChar,Source);

  if (TempChar == EOF)
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

                   P3DOutputStringStreamFile::P3DOutputStringStreamFile
                                      ()
 {
  Target = NULL;
  AutoLn = true;
 }

                   P3DOutputStringStreamFile::~P3DOutputStringStreamFile
                                      ()
 {
  if (Target != NULL)
   {
    Close();
   }
 }

void               P3DOutputStringStreamFile::Open
                                      (const char         *FileName)
 {
  if (Target != NULL)
   {
    Close();
   }

  Target = fopen(FileName,"wt");

  if (Target == NULL)
   {
    throw P3DExceptionIO();
   }
 }

void               P3DOutputStringStreamFile::Close
                                      ()
 {
  if (Target != NULL)
   {
    if (fclose(Target) != 0)
     {
      throw P3DExceptionIO();
     }

    Target = NULL;
   }
 }

void               P3DOutputStringStreamFile::WriteString
                                      (const char         *Buffer)
 {
  if (Target == NULL)
   {
    throw P3DExceptionAssert();
   }

  if (AutoLn)
   {
    if (fprintf(Target,"%s\n",Buffer) < 0)
     {
      throw P3DExceptionIO();
     }
   }
  else
   {
    if (fprintf(Target,"%s",Buffer) < 0)
     {
      throw P3DExceptionIO();
     }
   }
 }

void               P3DOutputStringStreamFile::AutoLnEnable
                                      ()
 {
  AutoLn = true;
 }

void               P3DOutputStringStreamFile::AutoLnDisable
                                      ()
 {
  AutoLn = false;
 }

