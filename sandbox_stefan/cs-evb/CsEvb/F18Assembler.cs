using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CsEvb
{
  public class F18Assembler
  {
    enum Command {
      undefined,
      number,           // #
      colon,            // :
      dot,              // .
      skip,             // ..
      lesssl,           // <sl
      comma,            // ,
      semicolon,        // ;
      comment,          // (
      pcy,              // +cy
      mcy,              // -cy
      abegin,           // A[
      setp,             // =P
      enda,             // ]]
      lcomment,         // line comment //
      here,
      begin,
      _else,
      then,
      _if,
      _while,
      until,
      mif,
      mwhile,
      muntil,
      zif,
      ahead,
      leap,
      again,
      repeat,
      _for,
      next,
      snext,
      org,
      qorg,             // ?org ( n-) error if org is not equal to expected value
      nl,
      ndl,
      equ,              // label definition
      its,
      swap,
      lit,
      alit,
      await,
      avail,
      load,
      assign_a,         // /A
      assign_b,         // /B
      assign_io,        // /IO
      assign_p,         // /P
      assign_stack,     // /STACK
      assign_rstack,    // /RSTACK
      do_add,           // [+]
      do_sub,           // [-]
      do_mul,           // [*]
      do_div,           // [/]
      do_and,           // AND
      do_or,            // OR
      do_xor,           // XOR
      do_inv,           // INV
      include,
      limit
    }

  }
}
