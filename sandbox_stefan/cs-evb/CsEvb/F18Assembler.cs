using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CsEvb
{
  public class F18Assembler
  {
    public enum Command
    {
      undefined,
      NUMBER,           // #
      COLON,            // :
      DOT,              // .
      SKIP,             // ..
      LESSSL,           // <sl
      COMMA,            // ,
      SEMICOLON,        // ;
      COMMENT,          // (
      PCY,              // +cy
      MCY,              // -cy
      ABEGIN,           // A[
      SETP,             // =P
      ENDA,             // ]]
      LCOMMENT,         // line comment //
      HERE,             // here
      BEGIN,            // begin
      ELSE,             // else
      THEN,             // then
      IF,               // if
      WHILE,            // while
      UNTIL,            // until
      MIF,              // -if
      MWHILE,           // -while
      MUNTIL,           // -until
      ZIF,              // zif
      AHEAD,            // ahead
      LEAP,             // leap
      AGAIN,            // again
      REPEAT,           // repeat
      FOR,              // for
      NEXT,             // next
      SNEXT,            // *next
      ORG,              // org
      QORG,             // ?org ( n-) error if org is not equal to expected value
      NL,               // NL
      NDL,              // NDL
      EQU,              // equ (label definition)
      ITS,              // its
      SWAP,             // SWAP
      LIT,              // lit
      ALIT,             // alit
      AWAIT,            // await
      AVAIL,            // avail
      LOAD,             // LOAD
      ASSIGN_A,         // /A ( n ) define register A
      ASSIGN_B,         // /B ( n ) define register B
      ASSIGN_IO,        // /IO ( n ) define register IO
      ASSIGN_P,         // /P ( n ) define register P
      ASSIGN_STACK,     // /STACK ( n0 .. n9 ) define parameter stack content
      ASSIGN_RSTACK,    // /RSTACK ( n0 .. n8 ) define return stack content
      DO_ADD,           // [+]
      DO_SUB,           // [-]
      DO_MUL,           // [*]
      DO_DIV,           // [/]
      DO_AND,           // AND
      DO_OR,            // OR
      DO_XOR,           // XOR
      DO_INV,           // INV
      INCLUDE,
      limit
    }

    public void Assemble(F18A core, string source, bool is_rom = false)
    {

    }


  }
}
