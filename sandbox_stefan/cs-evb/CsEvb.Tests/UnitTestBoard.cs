using System.Text;
using System.Xml;

namespace CsEvb.Tests;

public class Utf8StringWriter : StringWriter
{
  public override Encoding Encoding => Encoding.UTF8;
}
public class UnitTestBoard
{

  [Fact]
  public void Test1()
  {
    CsEvb.Board board = new();
    Utf8StringWriter sw = new();

    XmlWriterSettings wrs = new XmlWriterSettings();
    wrs.Indent = true;
    wrs.Encoding = System.Text.Encoding.UTF8;
    XmlWriter wr = XmlWriter.Create(sw, wrs);

    wr.WriteStartDocument();
    board.WriteXml(wr);
    wr.WriteEndDocument();
    wr.Flush();
    wr.Close();
    wr.Dispose();

    string xml = sw.ToString();

    StringReader srd = new StringReader(xml);

    board = new();
    XmlReader rd = XmlReader.Create(srd);
    board.ReadXml(rd);

  }


}