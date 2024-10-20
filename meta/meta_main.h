#ifndef META_MAIN_H
#define META_MAIN_H

enum TokenKind
{
  TokenKind_Unknown,

  TokenKind_OpenParanthesis,
  TokenKind_CloseParanthesis,
  TokenKind_Colon,
  TokenKind_SemiColon,
  TokenKind_OpenBracket,
  TokenKind_CloseBracket,
  TokenKind_OpenBrace,
  TokenKind_CloseBrace,
  TokenKind_Operator,
  TokenKind_Comma,

  TokenKind_Identifier,
  TokenKind_Numeric,
  TokenKind_String,

  TokenKind_NewLine,
  TokenKind_EndOfStream,
};

struct Token
{
  TokenKind kind;
  String8 string;
};

typedef U32 SourceFileFlags;
enum
{
  SourceFileFlag_Open = (1 << 0),
};

struct SourceFileNode
{
  SourceFileNode *next;
  SourceFileNode *prev;
  String8 path;
  SourceFileFlags flags;
  OS_Handle file_stream_handle;
};

struct SourceFileList
{
  SourceFileNode *first;
  SourceFileNode *last;
};

struct Tokenizer
{
  U8 *at;
  U8 *opl;
};

function B32 is_at_end_of_stream(Tokenizer *tokenizer, U64 offset);

#endif // META_MAIN_H
