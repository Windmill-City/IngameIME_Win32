using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AppWrapper_TestWindow
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            this.Load += Form1_Load;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Program.appWrapper.eventCommit += AppWrapper_eventCommit;
            Program.appWrapper.eventCompStr += AppWrapper_eventCompStr;
            Program.appWrapper.eventGetCompExt += AppWrapper_eventGetCompExt;
            Program.appWrapper.eventCompSel += AppWrapper_eventCompSel;
        }

        private void AppWrapper_eventCompSel(IntPtr source, int acpStart, int acpEnd)
        {
            label3.Text = String.Format("CompSel: {0} | {1}", acpStart, acpEnd);
        }

        private String compStr = "";
        private String storedStr = "";

        private void AppWrapper_eventGetCompExt(IntPtr source, refRECT rRect)
        {
            Font f = new Font("Microsoft YaHei", 16F, System.Drawing.FontStyle.Regular, GraphicsUnit.Pixel);
            Size sif = TextRenderer.MeasureText(storedStr, f, new Size(0, 0), TextFormatFlags.NoPadding);
            Size sif2 = TextRenderer.MeasureText(compStr, f, new Size(0, 0), TextFormatFlags.NoPadding);
            rRect.left = label2.Location.X + sif.Width;
            rRect.top = label2.Location.Y;
            rRect.right = rRect.left + sif2.Width;
            rRect.bottom = rRect.top + f.Height;
        }

        private void AppWrapper_eventCompStr(IntPtr source, string str)
        {
            compStr = str;
            this.label2.Text = storedStr + compStr;
        }

        private void AppWrapper_eventCommit(IntPtr source, string str)
        {
            storedStr += str;
            this.label2.Text = storedStr + compStr;
        }

        private void label1_Click(object sender, EventArgs e)
        {
            Label label = (Label)sender;
            if (Program.appWrapper.m_IsIMEEnabled)
            {
                Program.appWrapper.DisableIME();
                label.Text = "IMEState:Disabled";
            }
            else
            {
                Program.appWrapper.EnableIME();
                label.Text = "IMEState:Enabled";
            }
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Back:
                    if (storedStr.Length > 0)
                        storedStr = storedStr.Remove(storedStr.Length - 1, 1);
                    this.label2.Text = storedStr + compStr;
                    return true;
            }
            return base.ProcessCmdKey(ref msg, keyData);
        }
    }
}