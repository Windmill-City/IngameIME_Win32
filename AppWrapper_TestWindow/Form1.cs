#define UILess

using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace AppWrapper_TestWindow
{
    public partial class Form1 : Form
    {
        private AppWrapper appWrapper;

        private CompositionHandler compHandler;
        private CandidateListWrapper candWrapper;

        private String compStr = "";
        private String storedStr = "";

        public Form1()
        {
            InitializeComponent();
            this.Load += Form1_Load;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            appWrapper = new AppWrapper();
#if UILess
            appWrapper.Initialize(Handle, ActivateMode.UIELEMENTENABLEDONLY);
#else
            appWrapper.Initialize(Handle, ActivateMode.DEFAULT);
#endif

            //Composition
            compHandler = appWrapper.GetCompHandler();
            compHandler.eventGetCompExt += CompHandler_eventGetCompExt;
            compHandler.eventComposition += CompHandler_eventComposition;
#if UILess
            //CandidateList
            candWrapper = appWrapper.GetCandWapper();
            candWrapper.eventCandidateList += CandWrapper_eventCandidateList;
#endif
            appWrapper.EnableIME();
            IMEStateChange.Text = "IMEState:" + (appWrapper.m_IsIMEEnabled ? "Enabled" : "Disabled");
        }

        #region Handle CandidateList

        private void CandWrapper_eventCandidateList(refCandidateList list)
        {
            CandListData.Text = "";
            foreach (var cand in list.Candidates)
            {
                CandListData.Text += cand;
                CandListData.Text += "\r\n";
            }
        }

        #endregion Handle CandidateList

        #region Handle Composition

        private void CompHandler_eventComposition(refCompositionEventArgs args)
        {
            label_CompCaret.Text = string.Format("Comp CaretPos: {0} ", args.m_caretPos);

            storedStr += args.m_strCommit;
            compStr = args.m_strComp;

            label_DisplayStr.Text = storedStr + compStr;
        }

        #endregion Handle Composition

        #region Handle CompExt

        [StructLayout(LayoutKind.Sequential)]
        public struct RECT
        {
            public int left;
            public int top;
            public int right;
            public int bottom;
        }

        private void CompHandler_eventGetCompExt(IntPtr rect)
        {
            RECT rect_ = (RECT)Marshal.PtrToStructure(rect, typeof(RECT));//Map from

            Font f = new Font("Microsoft YaHei", 20F, System.Drawing.FontStyle.Regular, GraphicsUnit.Pixel);
            Size sif = TextRenderer.MeasureText(storedStr, f, new Size(0, 0), TextFormatFlags.NoPadding);
            Size sif2 = TextRenderer.MeasureText(compStr, f, new Size(0, 0), TextFormatFlags.NoPadding);
            //Map rect
            rect_.left = label_DisplayStr.Location.X + sif.Width;
            rect_.top = label_DisplayStr.Location.Y;
            //should use Font height, because some IME draw CompStr themselves, when CompStr is Empty
            //so the candidate window wont cover the text
            rect_.bottom = rect_.top + f.Height;
            rect_.right = rect_.left + sif2.Width;

            Marshal.StructureToPtr(rect_, rect, true);//Map to
        }

        #endregion Handle CompExt

        #region Handle WinForm

        private void IMEStateChange_Click(object sender, EventArgs e)
        {
            if (appWrapper.m_IsIMEEnabled)
                appWrapper.DisableIME();
            else
                appWrapper.EnableIME();
            IMEStateChange.Text = "IMEState:" + (appWrapper.m_IsIMEEnabled ? "Enabled" : "Disabled");
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Back:
                    if (storedStr.Length > 0)
                        storedStr = storedStr.Remove(storedStr.Length - 1, 1);
                    this.label_DisplayStr.Text = storedStr + compStr;
                    return true;
            }
            return base.ProcessCmdKey(ref msg, keyData);
        }

        #endregion Handle WinForm
    }
}