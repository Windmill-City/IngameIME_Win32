using System;
using System.Drawing;
using System.Runtime.InteropServices;
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
        }

        private void AppWrapper_eventCompSel(int acpStart, int acpEnd)
        {
            label3.Text = String.Format("CompSel: {0} | {1}", acpStart, acpEnd);
        }

        private String compStr = "";
        private String storedStr = "";

        [StructLayout(LayoutKind.Sequential)]
        public struct RECT
        {
            public int left;
            public int top;
            public int right;
            public int bottom;
        }

        private void AppWrapper_eventGetCompExt(IntPtr rRect)
        {
            RECT rect = (RECT)Marshal.PtrToStructure(rRect, typeof(RECT));//Map from

            Font f = new Font("Microsoft YaHei", 20F, System.Drawing.FontStyle.Regular, GraphicsUnit.Pixel);
            Size sif = TextRenderer.MeasureText(storedStr, f, new Size(0, 0), TextFormatFlags.NoPadding);
            Size sif2 = TextRenderer.MeasureText(compStr, f, new Size(0, 0), TextFormatFlags.NoPadding);
            //Map rect
            rect.left = label2.Location.X + sif.Width;
            rect.top = label2.Location.Y;
            //should use Font height, because some IME draw CompStr themselves, when CompStr is Empty
            //so the candidate window wont cover the text
            rect.bottom = rect.top + f.Height;
            rect.right = rect.left + sif2.Width;

            Marshal.StructureToPtr(rect, rRect, true);//Map to
        }

        private void AppWrapper_eventCompStr(string str)
        {
            compStr = str;
            this.label2.Text = storedStr + compStr;
        }

        private void AppWrapper_eventCommit(string str)
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