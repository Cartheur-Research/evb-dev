
namespace CsEvb;

public class Chip
{
  public virtual int GetNoOfRows()
  {
    throw new NotImplementedException();
  }

  public virtual int GetNoOfColumns()
  {
    throw new NotImplementedException();
  }

  public virtual F18A.RomType GetRomType(int node)
  {
    throw new NotImplementedException();
  }

  public int GetLastRow()
  {
    return GetNoOfRows() - 1;
  }

  public int GetLastColumn()
  {
    return GetNoOfColumns() - 1;
  }


}
