using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

    enum Opcode
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

    enum Register
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

    enum Port
    {
      UP,
      LEFT,
      DOWN,
      RIGHT,
      limit
    }

    enum IOBitCtrl
    {
      TRISTATE,
      WEAK_PULLDOWN,
      LOW,
      HIGH,
      limit
    }

    enum IOBitWrite
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

    enum IOBitAltWrite
    {
      DA_lsb = 0,
      DA_msb = 11,
      DB = 12,
      CTL = 13,
      VCO = 14,
      SR = 17,
    }

    enum IOBitRead
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

    enum IOPortAddr
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

    enum RomType
    {
      undefined,
      Basic,
      Analog,
      Digital,
      Serdes_boot,
      Sync_boot,
      Async_boot,
      Spi_boot,
      Wire1,
      limit
    }

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

    public struct State
    {
      private RegisterSet register_;

      public State()
      {
        register_ = new();
      }
    }


  }




}
