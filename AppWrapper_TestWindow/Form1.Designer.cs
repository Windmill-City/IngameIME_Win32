namespace AppWrapper_TestWindow
{
    partial class Form1
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
            this.IMEStateChange = new System.Windows.Forms.Label();
            this.label_DisplayStr = new System.Windows.Forms.Label();
            this.label_CompCaret = new System.Windows.Forms.Label();
            this.label_CandName = new System.Windows.Forms.Label();
            this.CandListData = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // IMEStateChange
            // 
            this.IMEStateChange.AutoSize = true;
            this.IMEStateChange.Font = new System.Drawing.Font("SimSun", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.IMEStateChange.Location = new System.Drawing.Point(12, 419);
            this.IMEStateChange.Name = "IMEStateChange";
            this.IMEStateChange.Size = new System.Drawing.Size(186, 22);
            this.IMEStateChange.TabIndex = 2;
            this.IMEStateChange.Text = "IMEState:Unknown";
            this.IMEStateChange.Click += new System.EventHandler(this.IMEStateChange_Click);
            // 
            // label_DisplayStr
            // 
            this.label_DisplayStr.AutoSize = true;
            this.label_DisplayStr.Font = new System.Drawing.Font("Microsoft YaHei", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_DisplayStr.Location = new System.Drawing.Point(12, 9);
            this.label_DisplayStr.Name = "label_DisplayStr";
            this.label_DisplayStr.Size = new System.Drawing.Size(119, 30);
            this.label_DisplayStr.TabIndex = 3;
            this.label_DisplayStr.Text = "Test Input";
            // 
            // label_CompCaret
            // 
            this.label_CompCaret.AutoSize = true;
            this.label_CompCaret.Font = new System.Drawing.Font("SimSun", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_CompCaret.Location = new System.Drawing.Point(567, 419);
            this.label_CompCaret.Name = "label_CompCaret";
            this.label_CompCaret.Size = new System.Drawing.Size(186, 22);
            this.label_CompCaret.TabIndex = 4;
            this.label_CompCaret.Text = "Comp CaretPos: 0";
            // 
            // label_CandName
            // 
            this.label_CandName.AutoSize = true;
            this.label_CandName.Font = new System.Drawing.Font("SimSun", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_CandName.Location = new System.Drawing.Point(13, 109);
            this.label_CandName.Name = "label_CandName";
            this.label_CandName.Size = new System.Drawing.Size(164, 22);
            this.label_CandName.TabIndex = 5;
            this.label_CandName.Text = "CandidateList:";
            // 
            // CandListData
            // 
            this.CandListData.AutoSize = true;
            this.CandListData.Font = new System.Drawing.Font("SimSun", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.CandListData.Location = new System.Drawing.Point(15, 144);
            this.CandListData.Name = "CandListData";
            this.CandListData.Size = new System.Drawing.Size(88, 16);
            this.CandListData.TabIndex = 6;
            this.CandListData.Text = "Candidates";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(15, 407);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(83, 12);
            this.label2.TabIndex = 7;
            this.label2.Text = "ClickToChange";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.CandListData);
            this.Controls.Add(this.label_CandName);
            this.Controls.Add(this.label_CompCaret);
            this.Controls.Add(this.label_DisplayStr);
            this.Controls.Add(this.IMEStateChange);
            this.ImeMode = System.Windows.Forms.ImeMode.On;
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label IMEStateChange;
        private System.Windows.Forms.Label label_DisplayStr;
        private System.Windows.Forms.Label label_CompCaret;
        private System.Windows.Forms.Label label_CandName;
        private System.Windows.Forms.Label CandListData;
        private System.Windows.Forms.Label label2;
    }
}

