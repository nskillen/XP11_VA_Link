using System;
using System.ComponentModel;
using System.IO;
using System.IO.Pipes;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace XP11_VA_Link_Tester
{
    public partial class Form1 : Form
    {
        private BindingList<string> rows;
        NamedPipeClientStream pipe;

        public Form1()
        {
            InitializeComponent();
            enableFormControls();
            rows = new BindingList<string>();
            lbDataRefs.DataSource = rows;
            pipe = new NamedPipeClientStream(".", "{2145AB63-BF83-40A4-8A9D-A358D45AF1C1}", PipeDirection.InOut);
        }

        private void connectToPipe(bool enableControlsAfter)
        {
            addValueToList("Connecting to pipe...");
            pipe.Connect();
            pipe.ReadMode = PipeTransmissionMode.Message;
            addValueToList("Pipe connected");
            if (enableControlsAfter) { enableFormControls(); }
        }

        private void disableFormControls()
        {
            tbDataRefName.Enabled = false;
            btnGetValue.Enabled = false;
        }

        private void enableFormControls()
        {
            if (InvokeRequired)
            {
                this.BeginInvoke(new Action(enableFormControls));
                return;
            }

            tbDataRefName.Enabled = true;
            tbDataRefName.Text = string.Empty;
            btnGetValue.Enabled = true;
        }

        private void BtnGetValue_Click(object sender, EventArgs e)
        {
            string dataRef = tbDataRefName.Text;

            disableFormControls();
            Task.Run(() =>
            {
                if (!pipe.IsConnected)
                {
                    connectToPipe(false);
                }
                
                try
                {
                    StreamReader sr = new StreamReader(pipe);
                    StreamWriter sw = new StreamWriter(pipe);
                    sw.AutoFlush = true;

                    addValueToList("Requesting dataref " + dataRef + "...");
                    sw.Write(dataRef);
                    addValueToList("Reading response...");
                    string response = sr.ReadLine();
                    addValueToList("Response: " + response);
                    
                    /*
                    byte[] bytes = Encoding.UTF8.GetBytes(dataRef);
                    StringBuilder responseString = new StringBuilder();

                    pipe.Write(bytes, 0, bytes.Length);

                    addValueToList("Request sent, awaiting reply...");
                    do
                    {
                        byte[] buffer = new byte[1024];
                        var bytesRead = pipe.Read(buffer, 0, buffer.Length);
                        responseString.Append(Encoding.UTF8.GetString(buffer, 0, bytesRead));
                    } while (!pipe.IsMessageComplete);
                    addValueToList(responseString.ToString());
                    */
                } catch (Exception ex)
                {
                    addValueToList("Error getting dataref " + dataRef + ": " + ex.Message);
                }
            });
        }

        private void addValueToList(string newVal)
        {
            if (InvokeRequired)
            {
                this.BeginInvoke(new Action<string>(addValueToList), newVal);
                return;
            }

            rows.Add(newVal);
        }
    }
}
