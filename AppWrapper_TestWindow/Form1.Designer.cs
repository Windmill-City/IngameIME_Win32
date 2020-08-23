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
            this.label1 = new System.Windows.Forms.Label();
            this.label_DisplayStr = new System.Windows.Forms.Label();
            this.label_CompCaret = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("SimSun", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label1.Location = new System.Drawing.Point(12, 419);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(351, 22);
            this.label1.TabIndex = 2;
            this.label1.Text = "IMEState:Enabled(ClickToChange)";
            this.label1.Click += new System.EventHandler(this.label1_Click);
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
            this.label_CompCaret.Location = new System.Drawing.Point(602, 419);
            this.label_CompCaret.Name = "label_CompCaret";
            this.label_CompCaret.Size = new System.Drawing.Size(186, 22);
            this.label_CompCaret.TabIndex = 4;
            this.label_CompCaret.Text = "Comp CaretPos: 0";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.label_CompCaret);
            this.Controls.Add(this.label_DisplayStr);
            this.Controls.Add(this.label1);
            this.ImeMode = System.Windows.Forms.ImeMode.On;
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label_DisplayStr;
        private System.Windows.Forms.Label label_CompCaret;
    }
}

