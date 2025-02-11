using System.Text;

namespace CsEvb
{
    public class F18Assembler
  {
    private Stack<int> stack_ = [];
    private Stack<int> rstack_ = [];
    F18A? node_ = null; // compilation target
    string current_ = ""; // current text
    System.Text.StringBuilder bld_ = new();
    int pos_ = 0; // position in source
    int start_pos_ = 0; // start position of current word in source
    int cell_ = 0; // current cell
    int slot_ = 0; // current slot
    int curr_ = -1; // current position of cell
    int next_ = -1; // next position of cell
    private bool is_skipping_ = false; // used for conditional compilation
    private bool is_skipping_else_ = false; // used for conditional compilation
    private bool is_compiling_ = false; // flag indication that compiling to current node is enabled
    private bool is_finished_ = false; // flag indication that source text is finished

    public enum Command
    {
      undefined,
      ADD,              // +
      SUB,              // -
      MUL,              // *
      DIV,              // /
      MOD,              // mod
      DIVMOD,           // /mod
      INVERT,           // invert
      NEGATE,           // negate
      ABS,              // abs
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
      //DO_SUB,           // [-]
      //DO_MUL,           // [*]
      //DO_DIV,           // [/]
      DO_AND,           // AND
      DO_OR,            // OR
      DO_XOR,           // XOR
      DO_INV,           // INV
      DO_IF,            // #IF
      DO_ELSE,          // #ELSE
      DO_THEN,          // #THEN
      TO_INTERPRET,     // [
      TO_COMPILE,       // ]
      INCLUDE,
      limit
    }

    static private Dictionary<string, Command> command_map_g;
    static private Dictionary<string, F18A.Opcode> opcode_map_g;

    static F18Assembler()
    {
      command_map_g = new();
      opcode_map_g = new();

      foreach (Command cmd in Enum.GetValues(typeof(Command)))
      {
        string? txt = CommandToString(cmd);
        if (txt is null)
        {
          //throw new InvalidProgramException("missing text for Command '" + cmd.ToString() + "'");
          Console.WriteLine("missing text for Command '" + cmd.ToString() + "'");
        }
        else
        {
          command_map_g[txt] = cmd;
        }
      }
      foreach (F18A.Opcode op in Enum.GetValues(typeof(F18A.Opcode)))
      {
        string? txt = F18A.OpcodeToString(op);
        if (txt is null)
        {
          //throw new InvalidProgramException("missing text for Command '" + cmd.ToString() + "'");
          Console.WriteLine("missing text for Opcode '" + op.ToString() + "'");
        }
        else
        {
          opcode_map_g[txt] = op;
        }
      }
      // alias
      opcode_map_g[">r"] = F18A.Opcode.PUSH;
      opcode_map_g["r>"] = F18A.Opcode.POP;
    }

    public static Command? CommandFromString(string txt)
    {
      if (command_map_g.TryGetValue(txt, out var res)) { return res; }
      return null;
    }

    public static F18A.Opcode? OpcodeFromString(string txt)
    {
      if (opcode_map_g.TryGetValue(txt, out var res)) { return res; }
      return null;
    }

    public static string? CommandToString(Command cmd)
    {
      switch (cmd)
      {
        case Command.ADD: return "+";
        case Command.SUB: return "-";
        case Command.MUL: return "*";
        case Command.DIV: return "/";
        case Command.MOD: return "mod";
        case Command.DIVMOD: return "/mod";
        case Command.NUMBER: return "#";
        case Command.COLON: return ":";
        case Command.DOT: return ".";
        case Command.SKIP: return "..";
        case Command.LESSSL: return "<sl";
        case Command.COMMA: return ",";
        case Command.SEMICOLON: return ";";
        case Command.COMMENT: return "(";
        case Command.PCY: return "+cy";
        case Command.MCY: return "-cy";
        case Command.ABEGIN: return "A[";
        case Command.SETP: return "=P";
        case Command.ENDA: return "]]";
        case Command.LCOMMENT: return "//";
        case Command.HERE: return "here";
        case Command.BEGIN: return "begin";
        case Command.ELSE: return "else";
        case Command.THEN: return "then";
        case Command.IF: return "if";
        case Command.WHILE: return "while";
        case Command.UNTIL: return "until";
        case Command.MIF: return "-if";
        case Command.MWHILE: return "-while";
        case Command.MUNTIL: return "-until";
        case Command.ZIF: return "zif";
        case Command.AHEAD: return "ahead";
        case Command.AVAIL: return "avail";
        case Command.LEAP: return "leap";
        case Command.AGAIN: return "again";
        case Command.REPEAT: return "repeat";
        case Command.FOR: return "for";
        case Command.NEXT: return "next";
        case Command.SNEXT: return "*next";
        case Command.ORG: return "org";
        case Command.QORG: return "?org";
        case Command.NL: return "NL";
        case Command.NDL: return "NDL";
        case Command.EQU: return "equ";
        case Command.ITS: return "its";
        case Command.SWAP: return "SWAP";
        case Command.LIT: return "lit";
        case Command.ALIT: return "alit";
        case Command.AWAIT: return "await";
        case Command.LOAD: return "LOAD";
        case Command.ASSIGN_A: return "/A";
        case Command.ASSIGN_B: return "/B";
        case Command.ASSIGN_IO: return "/IO";
        case Command.ASSIGN_P: return "/P";
        case Command.INCLUDE: return "include";
        case Command.DO_ADD: return "[+]";
        //case Command.DO_SUB: return "[-]";
        //case Command.DO_MUL: return "[*]";
        //case Command.DO_DIV: return "[/]";
        case Command.DO_AND: return "AND";
        case Command.DO_OR: return "OR";
        case Command.DO_XOR: return "XOR";
        case Command.DO_INV: return "INV";
        case Command.DO_IF: return "#IF";
        case Command.DO_ELSE: return "#ELSE";
        case Command.DO_THEN: return "#THEN";
        default: break;
      }
      return null;
    }

    public int Position { get { return pos_; } }
    public int StartPosition { get { return start_pos_; } }

    public void Assemble(F18A node, string source, bool is_rom)
    {
      node_ = node;
      Compile(source);
    }

    public int ToDigit(char ch, int radix)
    {
      if (radix > 10)
      {
        if ((ch >= 'a') && (ch < ('a' + radix - 10)))
        {
          return ch - 'a' + 10;
        }
        if ((ch >= 'A') && (ch < ('A' + radix - 10)))
        {
          return ch - 'A' + 10;
        }
        if ((ch >= '0') && (ch <= '9'))
        {
          return ch - '0';
        }
      }
      else if ((ch >= '0') && (ch < ('0' + radix)))
      {
        return ch - '0';
      }
      return -1;
    }

    public bool ConvertToNumber(string txt, out int val)
    {
      bool is_neg = false;
      int limit = txt.Length;
      int radix = 10;
      val = 0;
      if (limit > 0)
      {
        int i = 0;
        if (i<limit)
        {
          switch (txt[i])
          {
            case '+': ++i; break;
            case '-': is_neg = true; ++i; break;
            default: break;
           }
        }
        if (i < limit)
        {
          switch (txt[i])
          {
            case '%': radix = 2; break;

            case '$':
            case 'x':
            case 'X':
              radix = 16;
              ++i;
              break;
            default: break;
          }
        }
        if (i >= limit) { return false; }
        while (i < limit)
        {
          char ch = txt[i++];
          int dval = ToDigit(ch, radix);
          if (dval >= 0)
          {
            val *= radix;
            val += dval;
          }
          else
          {
            switch (ch)
            {
              case '.':
              break;

              case 'R':
              case 'r':
                if ((val < 2) || (val > 36))
                {
                  return false;
                }
                radix = val;
                val = 0;
                break;

              default:
                return false;
            }
          }
        }
        if (is_neg) { val = -val; }
        val &= 0x3ffff;
        return true;
      }
      return false;
    }


    public bool ScanNext(string src, ref int pos, out int start_pos, char del, StringBuilder bld)
    {
      bld.Clear();
      int limit = src.Length;
      bool del_is_space = del == ' ';
      if (del_is_space)
      {
        while ((pos < limit) && (src[pos] <= ' ')) { ++pos; }
      }
      else
      {
        while ((pos < limit) && ((src[pos] <= ' ') || (src[pos] == del))) { ++pos; }
      }
      if (pos >= limit)
      {
        start_pos = pos;
        return false;
      }
      start_pos = pos;
      if (del_is_space)
      {
        while ((pos < limit) && (src[pos] > ' '))
        {
          bld.Append(src[pos++]);
        }
      }
      else
      {
        while ((pos < limit) && (src[pos] != del))
        {
          bld.Append(src[pos++]);
        }
        ++pos;
      }
      return true;
    }

    public bool SkipNext(string src, ref int pos, char del)
    {
      int limit = src.Length;
      bool del_is_space = del == ' ';
      if (del_is_space)
      {
        while ((pos < limit) && (src[pos] <= ' ')) { ++pos; }
      }
      else
      {
        while ((pos < limit) && ((src[pos] <= ' ') || (src[pos] == del))) { ++pos; }
      }
      if (pos >= limit)
      {
        return false;
      }
      if (del_is_space)
      {
        while ((pos < limit) && (src[pos] > ' '))
        {
          ++pos;
        }
      }
      else
      {
        while ((pos < limit) && (src[pos] != del))
        {
          ++pos;
        }
        ++pos;
      }
      return true;
    }

    public int Pop()
    {
      return stack_.Pop();
    }

    public void Push(int value)
    {
      stack_.Push(value);
    }

    public void CompileCommand(Command cmd)
    {
      int par1, par2;
      switch (cmd)
      {
        case Command.ADD:
          if (is_compiling_) { CompileOpcode(F18A.Opcode.ADD); }
          else { par2 = Pop(); par1 = Pop(); Push(par1 + par2); }
          break;

        case Command.SUB:
          par2 = Pop();
          par1 = Pop();
          Push(par1 + par2);
          break;

        case Command.NUMBER:
          if (!ScanNext(current_, ref pos_, out start_pos_, ' ', bld_))
          {
            throw new InvalidDataException("missing number");
          }
          if (!ConvertToNumber(bld_.ToString(), out par1))
          {
            throw new InvalidDataException("text '" + bld_.ToString() + "' cannot be converted to a number");
          }
          CompileNumber(par1);
          break;

        case Command.ORG:
          Flush();
          curr_ = stack_.Pop();
          next_ = IncAddr(curr_);
          break;

        case Command.COLON:
          Flush();
          Word(' ');
          RegisterLabel(bld_.ToString(), curr_);
          is_compiling_ = true;
          break;

        case Command.COMMENT:
          Word(')');
          break;

        default:
          throw new NotImplementedException("Command '" + cmd.ToString() + "' not implemented");
      }
    }

    public void Write(int pos, int value)
    {
      Console.WriteLine("write '" + value.ToString() + "' @" + pos.ToString());
    }

    static public int IncAddr(int addr)
    {
      return addr + 1;
    }
    
    public void RegisterLabel(string name, int addr)
    {
      if (node_ is null)
      {
        throw new InvalidDataException("no node specified for label '" + name + "'");
      }
      node_.RegisterLabel(name, addr);
    }

    public void Word(char del)
    {
      if (!ScanNext(current_, ref pos_, out start_pos_, del, bld_))
      {
        throw new InvalidDataException("word expected");
      }

    }

    public void Flush()
    {
      if (slot_ > 0)
      {
        while (slot_ < 4)
        {
          CompileOpcode(F18A.Opcode.NOP);
        }
        Write(curr_, cell_);
        curr_ = next_;
        next_ = IncAddr(next_);
        slot_ = 0;
      }
    }

    public void CompileOpcode(F18A.Opcode op)
    {
      int opcode = (int)op;
      if (slot_ >= 4) { Flush(); }
      if (slot_ == 3)
      {
        if ((opcode & 3) != 0) { Flush(); }
      }
      int mask = 0x1f;
      switch (slot_)
      {
        case 0: opcode <<= 13; mask <<= 13; break;
        case 1: opcode <<= 8; mask <<= 8; break;
        case 2: opcode <<= 3; mask <<= 3; break;
        case 3: opcode >>= 2; mask >>= 2; break;
      }
      opcode ^= mask & 0x15555;
      cell_ &= ~mask;
      cell_ |= opcode;
      ++slot_;
    }


    public void CompileLiteral(int value)
    {
      CompileOpcode(F18A.Opcode.FETCHPLUS);
      Write(next_, value ^ 0x15555);
      next_ = IncAddr(next_);
    }

    public void CompileNumber(int value)
    {
      if (is_compiling_) { CompileLiteral(value); }
      else { Push(value); }
    }

    public void Compile(string src)
    {
      //stack_.Clear();
      rstack_.Clear();
      is_skipping_ = false;
      is_skipping_else_ = false;
      is_compiling_ = false;
      is_finished_ = false;
      pos_ = 0;
      current_ = src;
      curr_ = -1;
      next_ = -1;
      string word;

      while (!is_finished_)
      {
        if (is_skipping_)
        {
          if (!ScanNext(src, ref pos_, out start_pos_, ' ', bld_))
          {
            throw new InvalidDataException("missing #THEN");
          }
          word = bld_.ToString();
          if (word == "#ELSE")
          {
            if (is_skipping_else_)
            {
              throw new InvalidDataException("missing #THEN");
            }

          }
          else if (word == "#THEN")
          {
            is_skipping_ = false;
            is_skipping_else_ = false;
          }
        }
        else
        {
          if (!ScanNext(src, ref pos_, out start_pos_, ' ', bld_)) { is_finished_ = true; break; }
          word = bld_.ToString();
          Command? cmd = CommandFromString(word);
          if (cmd is not null) { CompileCommand((Command)cmd); continue; }
          F18A.Opcode? op = OpcodeFromString(word);
          if (op is not null) { CompileOpcode((F18A.Opcode)op); continue; }
          if (ConvertToNumber(word, out var value))
          {
            CompileNumber(value);
            continue;
          }
          throw new InvalidDataException("? " + word);
        }
      }

      if (rstack_.Count > 0)
      {
        throw new InvalidDataException("return stack not empty");
      }
      current_ = "";
    }


  }
}
