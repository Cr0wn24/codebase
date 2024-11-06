#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.cpp"
#include "os/os_inc.cpp"

#include "meta/meta_main.h"

static B32
is_whitespace(U8 ch)
{
  B32 result = ((ch == ' ') ||
                (ch == '\t'));
  return result;
}

static B32
is_alpha(U8 ch)
{
  B32 result = ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
  return result;
}

static B32
is_numeric(U8 ch)
{
  B32 result = ('0' <= ch && ch <= '9');
  return result;
}

static void
eat_all_whitespace(Tokenizer *tokenizer)
{
  while(!is_at_end_of_stream(tokenizer, 0))
  {
    if(is_whitespace(tokenizer->at[0]))
    {
      tokenizer->at++;
    }
    else if(tokenizer->at[0] == '/' &&
            tokenizer->at[1] == '/')
    {
      while(!is_at_end_of_stream(tokenizer, 0) && tokenizer->at[0] != '\n')
      {
        tokenizer->at++;
      }
    }
    else if(tokenizer->at[0] == '/' &&
            tokenizer->at[1] == '*')
    {
      while(!is_at_end_of_stream(tokenizer, 0) && !is_at_end_of_stream(tokenizer, 1) && !(tokenizer->at[0] == '*' && tokenizer->at[1] == '/'))
      {
        tokenizer->at++;
      }

      tokenizer->at += 2;
    }
    else if(tokenizer->at[0] == '\\')
    {
      // NOTE(hampus): this can not be special characters like
      // '\n', because then the string tokenizer would take care of that
      tokenizer->at++;
    }
    else
    {
      break;
    }
  }
}

static B32
is_at_end_of_stream(Tokenizer *tokenizer, U64 offset)
{
  B32 result = (tokenizer->at + offset) >= tokenizer->opl;
  return result;
}

static Token
get_token(Tokenizer *tokenizer)
{
  profile_static_begin();
  Token result = {};
  result.string = str8(tokenizer->at, 1);
  eat_all_whitespace(tokenizer);

  switch(tokenizer->at[0])
  {
    case '\0':
    {
      result.kind = TokenKind_EndOfStream;
      tokenizer->at++;
    }
    break;
    case '(':
    {
      result.kind = TokenKind_OpenParanthesis;
      tokenizer->at++;
    }
    break;
    case ')':
    {
      result.kind = TokenKind_CloseParanthesis;
      tokenizer->at++;
    }
    break;
    case ':':
    {
      result.kind = TokenKind_Colon;
      tokenizer->at++;
    }
    break;
    case ';':
    {
      result.kind = TokenKind_SemiColon;
      tokenizer->at++;
    }
    break;
    case '[':
    {
      result.kind = TokenKind_OpenBracket;
      tokenizer->at++;
    }
    break;
    case ']':
    {
      result.kind = TokenKind_CloseBracket;
      tokenizer->at++;
    }
    break;
    case '{':
    {
      result.kind = TokenKind_OpenBrace;
      tokenizer->at++;
    }
    break;
    case '}':
    {
      result.kind = TokenKind_CloseBrace;
      tokenizer->at++;
    }
    break;
    case ',':
    {
      result.kind = TokenKind_Comma;
      tokenizer->at++;
    }
    break;
    case '#':
    case '.':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '=':
    case '!':
    case '&':
    case '<':
    case '>':
    case '|':
    case '?':
    case '~':
    case '^':
    {
      result.kind = TokenKind_Operator;
      tokenizer->at++;
    }
    break;
    case '\'':
    {
      result.kind = TokenKind_String;
      tokenizer->at++;
      result.string.data = tokenizer->at;
      while(tokenizer->at[0] != '\'' &&
            !is_at_end_of_stream(tokenizer, 0))
      {
        if(tokenizer->at[0] == '\\' &&
           !is_at_end_of_stream(tokenizer, 1))
        {
          tokenizer->at++;
        }
        tokenizer->at++;
      }
      result.string.size = (U64)(tokenizer->at - result.string.data);
      tokenizer->at++;
    }
    break;
    case '"':
    {
      result.kind = TokenKind_String;
      tokenizer->at++;
      result.string.data = tokenizer->at;
      while(tokenizer->at[0] != '"' &&
            !is_at_end_of_stream(tokenizer, 0))
      {
        if(tokenizer->at[0] == '\\' &&
           !is_at_end_of_stream(tokenizer, 1))
        {
          tokenizer->at++;
        }
        tokenizer->at++;
      }
      result.string.size = (U64)(tokenizer->at - result.string.data);
      tokenizer->at++;
    }
    break;
    case '\r':
    {
      result.kind = TokenKind_NewLine;
      tokenizer->at++;
      if(!is_at_end_of_stream(tokenizer, 0) &&
         tokenizer->at[0] == '\n')
      {
        tokenizer->at++;
      }
    }
    break;
    case '\n':
    {
      result.kind = TokenKind_NewLine;
      tokenizer->at++;
    }
    break;
    default:
    {
      if(is_at_end_of_stream(tokenizer, 0))
      {
        result.kind = TokenKind_EndOfStream;
      }
      else
      {
        result.kind = TokenKind_Identifier;
        if(is_alpha(tokenizer->at[0]) || tokenizer->at[0] == '_')
        {
          result.string.data = tokenizer->at;
          while((is_alpha(tokenizer->at[0]) || tokenizer->at[0] == '_' || is_numeric(tokenizer->at[0])) &&
                !is_at_end_of_stream(tokenizer, 0))
          {
            tokenizer->at++;
          }
          result.string.size = (U64)(tokenizer->at - result.string.data);
        }
        else if(is_numeric(tokenizer->at[0]))
        {
          // TODO(hampus): handle floats correctly, 0.1, .1, 0.31f, 1.2f, 1.5, etc
          result.string.data = tokenizer->at;
          while(is_numeric(tokenizer->at[0]) && !is_at_end_of_stream(tokenizer, 0))
          {
            tokenizer->at++;
          }
          result.string.size = (U64)(tokenizer->at - result.string.data);
        }
      }
    }
    break;
  }

  profile_static_end();
  return result;
}

static B32
require_token(Tokenizer *tokenizer, TokenKind kind)
{
  Token token = get_token(tokenizer);
  B32 result = token.kind == kind;
  return result;
}

static B32
token_is_kind(Token token, TokenKind kind)
{
  B32 result = token.kind == kind;
  return result;
}

static void
display_error(String8 error, String8 file_path, U64 line_number)
{
  TempArena scratch = GetScratch(0, 0);
  String8 string = str8_push(scratch.arena, "%S(%llu): error: %S", file_path, line_number, error);
  fprintf(stderr, "%.*s\n", str8_expand(string));
  os_exit(1);
}

static void
get_all_source_files_from_directory_recursively(Arena *arena, String8 directory, SourceFileList *list)
{
  TempArena scratch = GetScratch(&arena, 1);
  OS_Handle file_iterator = os_file_iterator_init(directory);
  for(String8 string = {}; os_file_iterator_next(scratch.arena, file_iterator, &string);)
  {
    string = str8_append(arena, Str8Lit("\\"), string);
    string = str8_append(arena, directory, string);

    if(!str8_match(Str8Lit("generated"), str8_skip_last_slash(directory)))
    {
      OS_FileAttributes file_attribs = os_get_file_attributes(string);
      if(file_attribs.directory)
      {
        get_all_source_files_from_directory_recursively(arena, string, list);
      }
      else
      {
        if(str8_match(str8_postfix(string, 4), Str8Lit(".cpp")))
        {
          SourceFileNode *n = push_array<SourceFileNode>(arena, 1);
          n->path = string;
          dll_push_back(list->first, list->last, n);
        }
      }
    }
  }
}

static S32
os_entry_point(String8List args)
{
  ProfileInit("meta");
  ProfileInitThread();

  Arena *arena = arena_alloc();

  String8 exe_path = os_get_executable_path(arena);
  String8 exe_directory_path = str8_chop_last_slash(exe_path);
  String8 project_directory_path = str8_chop_last_slash(exe_directory_path);
  String8 code_path = str8_append(arena, project_directory_path, Str8Lit("\\src"));

  {
    TempArena scratch = GetScratch(0, 0);
    String8 base_meta_file_data = Str8Lit("#ifndef BASE_META_H\n"
                                          "#define BASE_META_H\n\n"
                                          "#define META_EMBED_FILE(path, name) extern String8 name;\n\n"
                                          "#endif // BASE_META_H");
    String8 base_meta_file_path = str8_append(scratch.arena, code_path, Str8Lit("\\codebase\\base\\base_meta.h"));
    AssertAlways(os_file_write(base_meta_file_path, base_meta_file_data));
  }

  String8List layer_paths = {};
  {
    TempArena scratch = GetScratch(0, 0);
    OS_Handle layers_folder_iterator = os_file_iterator_init(code_path);
    for(String8 string = {}; os_file_iterator_next(scratch.arena, layers_folder_iterator, &string);)
    {
      if(!str8_match(string, Str8Lit("third_party")))
      {
        string = str8_append(arena, Str8Lit("\\"), string);
        string = str8_append(arena, code_path, string);
        str8_list_push(arena, &layer_paths, string);
      }
    }
    str8_list_push(arena, &layer_paths, code_path);
  }

  SourceFileList source_files_list = {};
  String8 project_path = {};

  profile_scope_begin("gather source files");
  for(String8Node *n = layer_paths.first; n != 0; n = n->next)
  {
    get_all_source_files_from_directory_recursively(arena, n->v, &source_files_list);
  }
  profile_scope_end();

  profile_scope_begin("parse");
  for(SourceFileNode *n = source_files_list.first; n != 0; n = n->next)
  {
    String8 directory = str8_chop_last_slash(n->path);
    String8 file_name = str8_skip_last_slash(n->path);
    String8 file_contents = os_file_read(arena, n->path);
    Tokenizer tokenizer = {};
    tokenizer.at = file_contents.data;
    tokenizer.opl = file_contents.data + file_contents.size;
    U64 line_number = 1;
    B32 parsing = true;
    while(parsing)
    {
      Token token = get_token(&tokenizer);
      switch(token.kind)
      {
        default:
        {
        }
        break;
        case TokenKind_NewLine:
        {
          line_number += 1;
        }
        break;
        case TokenKind_Identifier:
        {
          profile_scope_begin("parse identifier");
          if(str8_match(token.string, Str8Lit("META_EMBED_FILE")))
          {
            if(require_token(&tokenizer, TokenKind_OpenParanthesis))
            {
              Token file_path_token = get_token(&tokenizer);
              if(token_is_kind(file_path_token, TokenKind_String))
              {
                String8 file_path = file_path_token.string;
                if(require_token(&tokenizer, TokenKind_Comma))
                {
                  Token variable_name_token = get_token(&tokenizer);
                  if(token_is_kind(variable_name_token, TokenKind_Identifier))
                  {
                    TempArena scratch = GetScratch(0, 0);
                    String8 generated_variable_name = variable_name_token.string;
                    String8 full_file_path = {};
                    {
                      String8List path_list = {};
                      str8_list_push(scratch.arena, &path_list, str8_chop_last_slash(n->path));
                      str8_list_push(scratch.arena, &path_list, Str8Lit("\\"));
                      str8_list_push(scratch.arena, &path_list, file_path);
                      full_file_path = str8_join(scratch.arena, &path_list);
                    }
                    String8 file_data = os_file_read(scratch.arena, full_file_path);
                    if(file_data.size != 0)
                    {
                      String8 generated_directory_path = str8_append(scratch.arena, directory, Str8Lit("\\generated"));
                      String8 editor_main_generated_file_path = str8_push(scratch.arena, "%S\\%S_meta.cpp", generated_directory_path, str8_chop(file_name, 4));
                      AssertAlways(os_directory_create(generated_directory_path));
                      if((n->flags & SourceFileFlag_Open) == 0)
                      {
                        AssertAlways(os_file_write(editor_main_generated_file_path, Str8Lit("")));
                        n->flags |= SourceFileFlag_Open;
                        n->file_stream_handle = os_file_stream_open(editor_main_generated_file_path);
                      }

                      StaticArray<U8, kilobytes(16)> buffer = {};
                      U64 buffer_used_size = 0;

                      {
                        String8 string = str8_push(scratch.arena, "\nStaticArray<U8, %" PRIU64 "> %S_data =\n{\n", file_data.size, generated_variable_name);
                        AssertAlways(os_file_stream_write(n->file_stream_handle, string));
                      }

                      for(U64 idx = 0; idx < file_data.size; idx += 1)
                      {
                        String8 string = {};
                        if(idx % 20 == 0)
                        {
                          string = cstr_format(buffer.val + buffer_used_size, array_count(buffer) - buffer_used_size, "0x%x,\n", file_data.data[idx]);
                        }
                        else
                        {
                          string = cstr_format(buffer.val + buffer_used_size, array_count(buffer) - buffer_used_size, "0x%x, ", file_data.data[idx]);
                        }

                        buffer_used_size += string.size;
                        if((buffer_used_size + 6) > array_count(buffer))
                        {
                          AssertAlways(os_file_stream_write(n->file_stream_handle, str8(buffer.val, buffer_used_size)));
                          buffer_used_size = 0;
                        }
                      }

                      AssertAlways(os_file_stream_write(n->file_stream_handle, str8(buffer.val, buffer_used_size)));

                      {
                        String8 string = str8_push(scratch.arena, "\n};\nString8 %S = {.data = (U8 *)%S_data.val, .size = array_count(%S_data)};", generated_variable_name, generated_variable_name, generated_variable_name);
                        AssertAlways(os_file_stream_write(n->file_stream_handle, string));
                      }
                    }
                    else
                    {
                      display_error(Str8Lit("failed to read file"), n->path, line_number);
                    }
                  }
                  else
                  {
                    display_error(Str8Lit("missing second argument"), n->path, line_number);
                  }
                }
                else
                {
                  display_error(Str8Lit("missing comma for second argument"), n->path, line_number);
                }
              }
              else
              {
                display_error(Str8Lit("expected string for path to file"), n->path, line_number);
              }
            }
            else
            {
              display_error(Str8Lit("expected open paranthesis"), n->path, line_number);
            }
          }
          profile_scope_end();
        }
        break;
        case TokenKind_EndOfStream:
        {
          if((n->flags & SourceFileFlag_Open) != 0)
          {
            AssertAlways(os_file_stream_close(n->file_stream_handle));
          }
          parsing = false;
        }
        break;
      }
    }
  }

  profile_scope_end();

  ProfileQuitThread();
  ProfileQuit();

  return 0;
}