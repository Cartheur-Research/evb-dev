namespace CsEvb;

public class Chip
{
  public virtual int NoOfRows()
  {
    throw new NotImplementedException();
  }

  public virtual int NoOfColumns()
  {
    throw new NotImplementedException();
  }

  public virtual int GetLastRow()
  {
    return NoOfRows() - 1;
  }

  public virtual int GetLastColumn()
  {
    return NoOfColumns() - 1;
  }


}
