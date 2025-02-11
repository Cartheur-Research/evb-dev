using System.Text;

namespace CsEvb
{
    public class F18A
  {
    public const uint InvalidValue = 0x80000000;

    //public const uint ParameterStackSize = 8;
    //public const uint ReturnStackSize = 8;
    public const uint StackSize = 8;

    public const uint ROMBits = 6;
    public const uint ROMSize = 1u << (int)ROMBits;
    public const uint ROMMask = ROMSize - 1;

    public const uint RAMBits = 6;
    public const uint RAMSize = 1u << (int)RAMBits;
    public const uint RAMMask = RAMSize - 1;

    public const uint MEMBits = 6;
    public const uint MEMSize = 1u << (int)MEMBits;
    public const uint MEMMask = MEMSize - 1;

    //============================================================================

    public enum Opcode
    {
      RET,
      EX,
      JUMP,
      CALL,
      UNEXT,
      NEXT,
      IF,
      MIF,
      FETCHP,
      FETCHPLUS,
      FETCHB,
      FETCHA,
      STOREP,
      STOREPLUS,
      STOREB,
      STOREA,
      PLUSMUL,
      MUL2,
      DIV2,
      INV,
      ADD,
      AND,
      XOR,
      DROP,
      DUP,
      POP,
      OVER,
      A,
      NOP,
      PUSH,
      BSTORE,
      ASTORE,
      limit
    }

    static public string? OpcodeToString(Opcode op)
    {
      switch (op)
      {
        case Opcode.RET: return "ret";
        case Opcode.EX: return "ex";
        case Opcode.JUMP: return "jump";
        case Opcode.CALL: return "call";
        case Opcode.UNEXT: return "unext";
        case Opcode.NEXT: return "next";
        case Opcode.IF: return "if";
        case Opcode.MIF: return "-if";
        case Opcode.FETCHP: return "@p";
        case Opcode.FETCHPLUS: return "@+";
        case Opcode.FETCHB: return "@b";
        case Opcode.FETCHA: return "@";
        case Opcode.STOREP: return "!p";
        case Opcode.STOREPLUS: return "!+";
        case Opcode.STOREB: return "!b";
        case Opcode.STOREA: return "!";
        case Opcode.PLUSMUL: return "+*";
        case Opcode.MUL2: return "2*";
        case Opcode.DIV2: return "2/";
        case Opcode.INV: return "inv";
        case Opcode.ADD: return "+";
        case Opcode.AND: return "and";
        case Opcode.XOR: return "xor";
        case Opcode.DROP: return "drop";
        case Opcode.DUP: return "dup";
        case Opcode.POP: return "pop";
        case Opcode.OVER: return "over";
        case Opcode.A: return "a";
        case Opcode.NOP: return "nop";
        case Opcode.PUSH: return "push";
        case Opcode.BSTORE: return "b!";
        case Opcode.ASTORE: return "a!";
      }
      return null;
    }

    //============================================================================



    //============================================================================

    public enum Register
    {
      T,
      S,
      R,
      P,
      I,
      A,
      B,
      IO,
      UP,     // up port
      LT,     // left port
      DN,     // down port
      RT,     // right port
      limit
    }

    //============================================================================

    public enum Port
    {
      UP,
      LEFT,
      DOWN,
      RIGHT,
      limit
    }

    //============================================================================

    public enum IOBitCtrl
    {
      TRISTATE,
      WEAK_PULLDOWN,
      LOW,
      HIGH,
      limit
    }

    //============================================================================

    public enum IOBitWrite
    {
      PIN1_ctl = 0,
      PIN1_value = 1,
      PIN3_ctl = 2,
      PIN3_value = 3,
      PIN5_ctl = 4,
      PIN5_value = 5,
      WD = 11,
      PIN17_ctl = 16,
      PIN17_value = 17,
    }

    //============================================================================

    public enum IOBitAltWrite
    {
      DA_lsb = 0,
      DA_msb = 11,
      DB = 12,
      CTL = 13,
      VCO = 14,
      SR = 17,
    }

    //============================================================================

    public enum IOBitRead
    {
      PIN1 = 1,
      PIN3 = 3,
      PIN5 = 5,
      UW = 9,
      UR = 10,
      LW = 11,
      LR = 12,
      DW = 13,
      DR = 14,
      RW = 15,
      RR = 16,
      PIN17 = 17,
    }

    //============================================================================

    public enum IOPortAddr
    {
      io = 0x08,
      data = 0x14,
      ___u = 0x10,
      __l_ = 0x20,
      __lu = 0x30,
      _d__ = 0x40,
      _d_u = 0x50,
      _dl_ = 0x60,
      _dlu = 0x70,
      r___ = 0x80,
      r__u = 0x90,
      r_l_ = 0xa0,
      r_lu = 0xb0,
      rd__ = 0xc0,
      rd_u = 0xd0,
      rdl_ = 0xe0,
      rdlu = 0xf0,
    }


    public string? AddrToString(uint addr)
    {
      switch (addr)
      {
        case 0x15d: return "io";
        case 0x171: return "ldata";
        case 0x141: return "data";
        case 0x157: return "warp";
        case 0x175: return "--l-";
        case 0x115: return "-d--";
        case 0x135: return "-dl-";
        case 0x1d5: return "r---";
        case 0x1f5: return "r-l-";
        case 0x195: return "rd--";
        case 0x1b5: return "rdl-";
        case 0x145: return "---u";
        case 0x165: return "--lu";
        case 0x105: return "-d-u";
        case 0x125: return "-dlu";
        case 0x1c5: return "r--u";
        case 0x1e5: return "r-lu";
        case 0x185: return "rd-u";
        case 0x1a5: return "rdlu";
        default: break;
      }
      return null;
    }

    //============================================================================

    public enum RomType
    {
      undefined,
      Basic,
      Analog,
      Digital,
      SerdesBoot,
      SyncBoot,
      AsyncBoot,
      SpiBoot,
      Wire1,
      limit
    }

    //============================================================================

    public struct RegisterSet
    {
      private uint[] data_;

      public RegisterSet()
      {
        data_ = new uint[(int)Register.limit];
      }

      public uint[] Data { get { return data_; } }

      public uint T { get { return data_[(int)Register.T]; } set { data_[(int)Register.T] = value; } }
      public uint S { get { return data_[(int)Register.S]; } set { data_[(int)Register.S] = value; } }
      public uint R { get { return data_[(int)Register.R]; } set { data_[(int)Register.R] = value; } }
      public uint P { get { return data_[(int)Register.P]; } set { data_[(int)Register.P] = value; } }
      public uint I { get { return data_[(int)Register.I]; } set { data_[(int)Register.I] = value; } }
      public uint A { get { return data_[(int)Register.A]; } set { data_[(int)Register.A] = value; } }
      public uint B { get { return data_[(int)Register.B]; } set { data_[(int)Register.B] = value; } }
      public uint IO { get { return data_[(int)Register.IO]; } set { data_[(int)Register.IO] = value; } }
      public uint UP { get { return data_[(int)Register.UP]; } set { data_[(int)Register.UP] = value; } }
      public uint LT { get { return data_[(int)Register.LT]; } set { data_[(int)Register.LT] = value; } }
      public uint DN { get { return data_[(int)Register.DN]; } set { data_[(int)Register.DN] = value; } }
      public uint RT { get { return data_[(int)Register.RT]; } set { data_[(int)Register.RT] = value; } }

    }

    //============================================================================

    public struct Memory
    {
      private uint[] data_;
      private Dictionary<string, int> label_map_;

      public Memory()
      {
        data_ = new uint[MEMSize];
        label_map_ = new();
      }
    }

    //============================================================================

    static private string relay = @"
  # xA1 org
  : relay ( a) r> a! @+ >r @+ zif drop ahead SWAP then r> over >r @p a relay !b !b !b begin @+ !b unext
  : done then a >r a! ;";

    static private string warm = @"
  # xA9 org
  : warm await ;";

    static private string poly = @"
  # xAA org
  : poly ( xn-xy) r> a! >r @+ a begin >r *. r> a! @+ + a next >r ;";

    static private string interp = @"
  : interp ( ims-v) dup >r >r over
    begin 2/ unext a! and >r @+ dup @+ inv . + inv r> a! dup dup xor 
    begin +* unext >r drop r> . + ;";

    static private string filter = @"
  : taps ( y x c - y' x') r> a! >r begin @+ @ >r a >r *.17 r> a! >r !+ r> . + r> next @ a! ;";

    static private string mul17 = @"
  : *.17 ( a b - a a*b) a! 16. >r dup dup xor begin +* unext inv +* a -if drop inv 2* ; then drop 2* inv ;";

    static private string shift = @"
  : lsh ( w n-1 - w') for 2* unext ;
  : rsh ( w n-1 - w') for 2/ unext ;";

    static private string triangle = @"
  : triangle ( ang*4) x1.0000 over -if drop . + ; then drop inv . + inv ;";

    static private string muldot = @"
  : *. ( f1 f2 - f1 f1*f2) *.17 a 2* -if drop inv 2* inv ; then drop 2* ;";

    static private string divide = @"
  +cy
  : clc ( -2-) dup dup xor dup . + drop ;
  : --u/mod ( hld-rq) clc
  : -u/mod ( hld-ra) a! 17. >r
    begin begin dup . + . >r dup . + dup a . + -if drop r> *next dup . + ; then over xor xor . r> next dup . + ;
  -cy";

    static private string wire1 = @"
  # x9E org
  : rcv ( s-sn) a >r dup dup a! 17. for begin
  : bit @ drop @b -if drop inv 2* inv *next r> a! ; then drop 2* next r> a! ;
  # xAA org
  : cold left center a! . io b! dup dup xB7. dup >r >r 16. >r @ drop @b # await -until drop a! . bit ;
  : fwd-b7 >r rcv a! rcv >r zif ; thn begin rcv !+ next ;";


    static private string dac = @"
  : -dac ( legacy entry name ...)
  : dac27 ( m c p a w - m c p) dup >r >r over r> inv . +  >r >r  x155. r> over xor a 
    begin unext !b . begin unext !b !b ;";

    static private string spi = @"
  # x2A NL ---  # x2B NL --+  # x3A NL +--  # x3B NL +-+ # x2F NL -++
  # xC2 org
  : 8obits ( dw-dw') 7. for  ( obit) leap 2* *next ;
  : ibit ( dw-dw') @b . -if  drop inv 2* ;  then drop 2* inv ;
  : half ( dwc-dw) !b over for . . unext ;
  : select ( dw-dw) -++ half --+ half ;
  : obit ( dw-dw) then  -if +-- half +-+ half ; then
  : rbit ( dw-dw) --- half --+ half ;
  : 18ibits ( d-dw) dup 17. >r
  : ibits   begin rbit ibit inv next ;
  : u2/ ( n - n) 2/ x1FFFF. and ;
  # 497 NL spispeed
  # xC00 NL spicmd
  # 0 NL spiadr
  : cold @b inv ..  avail -until  spispeed spiadr >r spicmd
  : spi-boot ( d ah - d x) select  8obits 8obits drop r> . 8obits 8obits
  : spi-exec ( dx-dx) drop 18ibits x1E000. . +  # rdl- -until >r 18ibits a! 18ibits
  : spi-copy ( dn-dx) >r zif ; then begin 18ibits !+ next  dup ;";


    static private string async = @"
  # xCB equ 18ibits
  : cold   x31A5. a! @  @b .. -if
  : ser-exec ( x - d)   18ibits drop >r 18ibits drop a! 18ibits
  : ser-copy ( xnx-d)   drop >r zif ;  then begin 18ibits drop !+ next ;  then drop avail alit >r >r ;
  : wait ( x-1/1)   begin . drop @b -until  . drop ;
  : sync ( x-3/2-d)   dup dup wait  xor inv >r begin @b . -if . drop *next await ;  then . drop r> inv 2/ ;
  : start ( dw-4/2-dw,io) dup wait over dup 2/ . + >r
  : delay ( -1/1-io) begin @b . -if then . drop next @b ;
   ( 18ibits ( x-4/6-dwx) sync sync dup start ( 2bits) leap leap
  : byte   then drop start leap
  : 4bits   then leap
  : 2bits   then then leap
  : 1bit ( nw,io - nw,io) then >r 2/ r> over xor x20000. and xor over >r delay ;";

    static private string sync = @"
  # xBE equ sget
  : cold   x31A5. a! @ @b . . -if avail # x3.FC00 [+] lit  dup >r dup
    begin drop @b . -if ( rising) *next  SWAP then avail alit >r  drop >r ; then
  : ser-exec ( x - x)   sget >r  sget a!  sget
  : ser-copy ( n)   >r zif ;  then begin sget !+ next ;
  ( sget) ( -4/3-w)   dup leap leap
  : 6in   then then leap leap
  : 2in   then then  2* 2*  dup
  begin . drop @b . inv -until  inv 2. and dup begin . drop @b . . -until  2. and 2/ xor xor ;";


    private RegisterSet register_ = new();
    private Memory ram_ = new();
    private Memory rom_ = new();
    private string source_ = "";
    private string comment_ = "";
    private RomType rom_type_ = RomType.undefined;
    private Dictionary<string, int> label_map_ = [];

    public F18A()
    {
    }

    public void RegisterLabel(string label, int addr)
    {
      label_map_[label] = addr;
    }

    public bool TryLookupLabel(string label, out int addr)
    {
      if (label_map_.TryGetValue(label, out addr)) { return true; }
      addr = -1;
      return false;
    }

    public RomType ROMType { get { return rom_type_; } set { rom_type_ = value; } }
    public Memory RAM { get { return ram_; } }
    public Memory ROM { get { return rom_; } }

    public string Source { get { return source_; } }
    public string Comment { get { return comment_; } }

    public string GetROMSource()
    {
      return GetROMSource(rom_type_);
    }

    //public Memory GetRAM() { return ram_; }
    //public Memory GetROM() { return rom_; }

    public static string GetROMSource(RomType rom_type)
    {
      StringBuilder sb = new StringBuilder();
      switch (rom_type)
      {
        case RomType.Basic:
          sb.Append(relay);
          sb.Append(warm);
          sb.Append(" # xB0 org");
          sb.Append(mul17);
          sb.Append(muldot);
          sb.Append(filter);
          sb.Append(interp);
          sb.Append(triangle);
          sb.Append(divide);
          sb.Append(poly);
          break;

        case RomType.Analog:
          sb.Append(relay);
          sb.Append(warm);
          sb.Append(" # xB0 org");
          sb.Append(mul17);
          sb.Append(muldot);
          sb.Append(dac);
          sb.Append(interp);
          sb.Append(triangle);
          sb.Append(divide);
          sb.Append(poly);
          break;

        case RomType.SpiBoot:
          sb.Append(relay);
          sb.Append(warm);
          sb.Append(spi);
          break;

        case RomType.Wire1:
          sb.Append(warm);
          sb.Append(wire1);
          sb.Append(triangle);
          sb.Append(mul17);
          sb.Append(muldot);
          sb.Append(divide);
          break;

        case RomType.SerdesBoot:
          sb.Append(relay);
          sb.Append(warm);
          sb.Append(" : cold x3141. a! x3FFFE. dup !  rdlu cold ;");
          sb.Append(" # xB0 org");
          sb.Append(mul17);
          sb.Append(muldot);
          sb.Append(filter);
          sb.Append(interp);
          sb.Append(triangle);
          sb.Append(divide);
          break;

        case RomType.AsyncBoot:
          sb.Append(relay);
          sb.Append(warm);
          sb.Append(async);
          sb.Append(shift);
          break;

        case RomType.SyncBoot:
          sb.Append(relay);
          sb.Append(warm);
          sb.Append(sync);
          sb.Append(mul17);
          sb.Append(filter);
          sb.Append(triangle);
          break;

        default: break;
      }
      return sb.ToString();
    }


  }




}
