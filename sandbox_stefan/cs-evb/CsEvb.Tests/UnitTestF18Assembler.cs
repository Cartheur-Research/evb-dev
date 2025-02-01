namespace CsEvb.Tests;

public class UnitTestF18Assembler
{

  [Fact]
  public void Test1()
  {
    CsEvb.F18A f18a = new();
    CsEvb.F18Assembler ass = new();

    Assert.Null(CsEvb.F18Assembler.OpcodeFromString(""));
    Assert.Null(CsEvb.F18Assembler.OpcodeFromString("invalid"));
    Assert.Equal(CsEvb.F18A.Opcode.RET, CsEvb.F18Assembler.OpcodeFromString("ret"));
    Assert.Equal(CsEvb.F18A.Opcode.EX, CsEvb.F18Assembler.OpcodeFromString("ex"));
    Assert.Equal(CsEvb.F18A.Opcode.JUMP, CsEvb.F18Assembler.OpcodeFromString("jump"));
    Assert.Equal(CsEvb.F18A.Opcode.CALL, CsEvb.F18Assembler.OpcodeFromString("call"));
    Assert.Equal(CsEvb.F18A.Opcode.UNEXT, CsEvb.F18Assembler.OpcodeFromString("unext"));
    Assert.Equal(CsEvb.F18A.Opcode.NEXT, CsEvb.F18Assembler.OpcodeFromString("next"));
    Assert.Equal(CsEvb.F18A.Opcode.IF, CsEvb.F18Assembler.OpcodeFromString("if"));
    Assert.Equal(CsEvb.F18A.Opcode.MIF, CsEvb.F18Assembler.OpcodeFromString("-if"));
    Assert.Equal(CsEvb.F18A.Opcode.FETCHP, CsEvb.F18Assembler.OpcodeFromString("@p"));
    Assert.Equal(CsEvb.F18A.Opcode.FETCHPLUS, CsEvb.F18Assembler.OpcodeFromString("@+"));
    Assert.Equal(CsEvb.F18A.Opcode.FETCHB, CsEvb.F18Assembler.OpcodeFromString("@b"));
    Assert.Equal(CsEvb.F18A.Opcode.FETCHA, CsEvb.F18Assembler.OpcodeFromString("@"));
    Assert.Equal(CsEvb.F18A.Opcode.STOREP, CsEvb.F18Assembler.OpcodeFromString("!p"));
    Assert.Equal(CsEvb.F18A.Opcode.STOREPLUS, CsEvb.F18Assembler.OpcodeFromString("!+"));
    Assert.Equal(CsEvb.F18A.Opcode.STOREB, CsEvb.F18Assembler.OpcodeFromString("!b"));
    Assert.Equal(CsEvb.F18A.Opcode.STOREA, CsEvb.F18Assembler.OpcodeFromString("!"));
    Assert.Equal(CsEvb.F18A.Opcode.PLUSMUL, CsEvb.F18Assembler.OpcodeFromString("+*"));
    Assert.Equal(CsEvb.F18A.Opcode.MUL2, CsEvb.F18Assembler.OpcodeFromString("2*"));
    Assert.Equal(CsEvb.F18A.Opcode.DIV2, CsEvb.F18Assembler.OpcodeFromString("2/"));
    Assert.Equal(CsEvb.F18A.Opcode.INV, CsEvb.F18Assembler.OpcodeFromString("inv"));
    Assert.Equal(CsEvb.F18A.Opcode.ADD, CsEvb.F18Assembler.OpcodeFromString("+"));
    Assert.Equal(CsEvb.F18A.Opcode.AND, CsEvb.F18Assembler.OpcodeFromString("and"));
    Assert.Equal(CsEvb.F18A.Opcode.XOR, CsEvb.F18Assembler.OpcodeFromString("xor"));
    Assert.Equal(CsEvb.F18A.Opcode.DROP, CsEvb.F18Assembler.OpcodeFromString("drop"));
    Assert.Equal(CsEvb.F18A.Opcode.DUP, CsEvb.F18Assembler.OpcodeFromString("dup"));
    Assert.Equal(CsEvb.F18A.Opcode.POP, CsEvb.F18Assembler.OpcodeFromString("pop"));
    Assert.Equal(CsEvb.F18A.Opcode.OVER, CsEvb.F18Assembler.OpcodeFromString("over"));
    Assert.Equal(CsEvb.F18A.Opcode.A, CsEvb.F18Assembler.OpcodeFromString("a"));
    Assert.Equal(CsEvb.F18A.Opcode.NOP, CsEvb.F18Assembler.OpcodeFromString("nop"));
    Assert.Equal(CsEvb.F18A.Opcode.PUSH, CsEvb.F18Assembler.OpcodeFromString("push"));
    Assert.Equal(CsEvb.F18A.Opcode.BSTORE, CsEvb.F18Assembler.OpcodeFromString("b!"));
    Assert.Equal(CsEvb.F18A.Opcode.ASTORE, CsEvb.F18Assembler.OpcodeFromString("a!"));

    Assert.Null(CsEvb.F18Assembler.CommandFromString(""));
    Assert.Null(CsEvb.F18Assembler.CommandFromString("xyz"));

    Assert.Equal(CsEvb.F18Assembler.Command.ADD, CsEvb.F18Assembler.CommandFromString("+"));
  }


  [Fact]
  public void Test2()
  {
    bool result;

    CsEvb.F18Assembler ass = new();

    string txt = " a bb ccc\r\n";
    int pos = 0;
    int start_pos;

    System.Text.StringBuilder bld = new();

    result = ass.ScanNext(txt, ref pos, out start_pos, ' ', bld);
    Assert.True(result);
    Assert.Equal(2, pos);
    Assert.Equal(1, start_pos);
    Assert.Equal("a", bld.ToString());
    result = ass.ScanNext(txt, ref pos, out start_pos, ' ', bld);
    Assert.True(result);
    Assert.Equal(5, pos);
    Assert.Equal(3, start_pos);
    Assert.Equal("bb", bld.ToString());
    result = ass.ScanNext(txt, ref pos, out start_pos, ' ', bld);
    Assert.True(result);
    Assert.Equal(9, pos);
    Assert.Equal(6, start_pos);
    Assert.Equal("ccc", bld.ToString());
    result = ass.ScanNext(txt, ref pos, out start_pos, ' ', bld);
    Assert.False(result);
    Assert.Equal(11, pos);
    Assert.Equal(11, start_pos);
    Assert.Equal("", bld.ToString());

    pos = 0;
    result = ass.SkipNext(txt, ref pos, ' ');
    Assert.True(result);
    Assert.Equal(2, pos);
    result = ass.SkipNext(txt, ref pos, ' ');
    Assert.True(result);
    Assert.Equal(5, pos);
    result = ass.SkipNext(txt, ref pos, ' ');
    Assert.True(result);
    Assert.Equal(9, pos);
    result = ass.SkipNext(txt, ref pos, ' ');
    Assert.False(result);
    Assert.Equal(11, pos);


    txt = " a      Hello World\" ";

    pos = 0;
    result = ass.ScanNext(txt, ref pos, out start_pos, ' ', bld);
    Assert.True(result);
    Assert.Equal(2, pos);
    Assert.Equal(1, start_pos);
    Assert.Equal("a", bld.ToString());
    result = ass.ScanNext(txt, ref pos, out start_pos, '"', bld);
    Assert.True(result);
    Assert.Equal(20, pos);
    Assert.Equal(8, start_pos);
    Assert.Equal("Hello World", bld.ToString());

    pos = 0;
    result = ass.SkipNext(txt, ref pos, ' ');
    Assert.True(result);
    Assert.Equal(2, pos);
    result = ass.SkipNext(txt, ref pos, '"');
    Assert.True(result);
    Assert.Equal(20, pos);


  }

  [Fact]
  public void Test3()
  {
    CsEvb.F18A f18a = new();
    CsEvb.F18Assembler ass = new();

    ass.Assemble(f18a, CsEvb.F18A.GetROMSource(CsEvb.F18A.RomType.Basic), true);

  }
}