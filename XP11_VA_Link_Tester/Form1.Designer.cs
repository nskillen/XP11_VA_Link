namespace XP11_VA_Link_Tester
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
            this.lbDataRefs = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tbDataRefName = new System.Windows.Forms.TextBox();
            this.btnGetValue = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lbDataRefs
            // 
            this.lbDataRefs.FormattingEnabled = true;
            this.lbDataRefs.Location = new System.Drawing.Point(12, 37);
            this.lbDataRefs.Name = "lbDataRefs";
            this.lbDataRefs.Size = new System.Drawing.Size(776, 394);
            this.lbDataRefs.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(76, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Dataref Name:";
            // 
            // tbDataRefName
            // 
            this.tbDataRefName.Enabled = false;
            this.tbDataRefName.Location = new System.Drawing.Point(91, 10);
            this.tbDataRefName.Name = "tbDataRefName";
            this.tbDataRefName.Size = new System.Drawing.Size(616, 20);
            this.tbDataRefName.TabIndex = 2;
            // 
            // btnGetValue
            // 
            this.btnGetValue.Enabled = false;
            this.btnGetValue.Location = new System.Drawing.Point(713, 8);
            this.btnGetValue.Name = "btnGetValue";
            this.btnGetValue.Size = new System.Drawing.Size(75, 23);
            this.btnGetValue.TabIndex = 3;
            this.btnGetValue.Text = "Get Value";
            this.btnGetValue.UseVisualStyleBackColor = true;
            this.btnGetValue.Click += new System.EventHandler(this.BtnGetValue_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 443);
            this.Controls.Add(this.btnGetValue);
            this.Controls.Add(this.tbDataRefName);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lbDataRefs);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox lbDataRefs;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbDataRefName;
        private System.Windows.Forms.Button btnGetValue;
    }
}

