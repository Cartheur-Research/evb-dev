namespace CsEvb.Tests;

public class UnitTestF18Assembler
{
    [Fact]
    public void Test1()
    {
      CsEvb.F18A f18a = new();
      CsEvb.F18Assembler ass = new();

      ass.Assemble(f18a, CsEvb.F18A.GetROMSource(CsEvb.F18A.RomType.Basic));

    }
}