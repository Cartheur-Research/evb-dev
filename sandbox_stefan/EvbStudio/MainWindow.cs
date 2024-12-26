using System;
using System.Drawing;
using System.Windows.Forms;

namespace EvbStudio
{
    public partial class MainWindow : Form
  {
    public MainWindow()
    {
      main_menu_ = new MainMenu();
      MenuItem file_menu = main_menu_.MenuItems.Add("&File");
      file_menu.MenuItems.Add(new MenuItem("-"));
      file_menu.MenuItems.Add(new MenuItem("&Exit", new EventHandler(this.exitMenuItem_Click), Shortcut.CtrlX));

      this.Menu = main_menu_;

      InitializeComponent();
    }

    private void exitMenuItem_Click(object sender, EventArgs e)
    {
      this.Close();
    }

    private void MainWindow_Load(object sender, EventArgs e)
    {

    }

    private void MainWindow_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
    {
      Graphics g = e.Graphics;
      g.DrawString("Hello World", fnt_, System.Drawing.Brushes.Blue, new Point(100, 100));
    }

    private Font fnt_ = new Font("Arial", 10);
  }
}
