using System.Drawing;
using System.Windows.Forms;

namespace EvbStudio
{
  partial class MainWindow
  {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
      if (disposing && (components != null))
      {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
      this.SuspendLayout();
      // 
      // MainWindow
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(1565, 889);
      this.Name = "MainWindow";
      this.Text = "EVB";
      this.Load += new System.EventHandler(this.MainWindow_Load);
      this.Paint += new System.Windows.Forms.PaintEventHandler(this.MainWindow_Paint);
      this.ResumeLayout(false);

    }

    #endregion


    private MainMenu main_menu_;
  }
}

