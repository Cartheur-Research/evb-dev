namespace CsEvb;

public class GA144 : Chip
{
  public const int NoOfRows = 8;
  public const int NoOfColumns = 18;

  private readonly F18A[] nodes_;
  private readonly F18Assembler assembler_ = new();

  private int GetIndex(int row, int column)
  {
    return column * NoOfRows + row;
  }

  public F18A GetNode(int n)
  {
    return nodes_[GetIndex(n / 100, n % 100)];
  }

  public override int GetNoOfRows()
  {
    return NoOfRows;
  }

  public override int GetNoOfColumns()
  {
    return NoOfColumns;
  }

  public override F18A.RomType GetRomType(int node)
  {
    switch (node)
    {
      case 007: return F18A.RomType.Node007;
      case 008: return F18A.RomType.Node008;
      case 009: return F18A.RomType.Node009;
      case 106: return F18A.RomType.Node106;
      case 107: return F18A.RomType.Node107;
      case 108: return F18A.RomType.Node108;
      case 001:
      case 701: return F18A.RomType.SerdesBoot;
      case 705: return F18A.RomType.SpiBoot;
      case 300: return F18A.RomType.SyncBoot;
      case 708: return F18A.RomType.AsyncBoot;
      case 117:
      case 617:
      case 717:
      case 713:
      case 709: return F18A.RomType.Analog;
      case 100:
      case 500:
      case 600:
      case 417:
      case 317: return F18A.RomType.Digital;
      case 200: return F18A.RomType.Wire1;
      default: break;
    }
    return F18A.RomType.Basic;
  }

  public GA144()
  {
    nodes_ = new F18A[NoOfRows * NoOfColumns];

    string rom;
    F18A node;
    //F18Assembler ass = new();

    for (int r = 0; r < NoOfRows; ++r)
    {
      for (int c = 0; c < NoOfColumns; ++c)
      {
        node = new F18A(this, r, c);
        nodes_[GetIndex(r, c)] = node;
        node.ClearROM();
        node.SetRomType(GetRomType(node.GetNodeNo()));
        rom = node.GetROMSource();
        assembler_.Assemble(this, node, rom, true);
      }
    }


    //for (int c = 0; c < NoOfColumns; ++c)
    //{
    //  for (int r = 0; r < NoOfRows; ++r)
    //  {
    //    F18A node = new F18A(this, r, c);
    //    nodes_[GetIndex(r, c)] = node;
    //    node.SetRomType(GetRomType(node.GetNodeNo()));
    //    ass.Assemble(node, node.GetROMSource(), true);
    //  }
    //}

    //if (false)
    //{
    node = nodes_[GetIndex(1, 8)];
    Console.WriteLine("-------------- ROM -------------------");
    Console.WriteLine(assembler_.Disassemble(node, 0x80, 0xBF));
    rom = node.GetROMSource();
    assembler_.Assemble(this, node, rom, true);
    Console.WriteLine("-------------- ROM -------------------");
    //}
  }

  public void Compile(int row, int column)
  {
    F18A node;
    node = nodes_[GetIndex(row, column)];
    node.ClearRAM();
    assembler_.Assemble(this, node, node.Source, false);
  }


}
