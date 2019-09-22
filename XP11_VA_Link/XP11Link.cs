using System;
using System.Linq;

namespace XP11_VA_Link
{
    class XP11Link
    {
        private readonly UDPSocket socket;
        private bool connected;

        private const int DREF_STRING_LEN = 400;
        private struct dref_struct_in
        {
            int dref_freq;
            int dref_en;
            char[] dref_string;

            public dref_struct_in(int freq, int en, string text)
            {
                dref_freq = freq;
                dref_en = en;
                dref_string = new char[DREF_STRING_LEN];
                int i;
                for (i = 0; i < DREF_STRING_LEN-1 && i < text.Length; i++)
                {
                    dref_string[i] = text[i];
                }
                dref_string[i] = '\0';
            }

            public byte[] ToBytes()
            {

                void writeTo(byte[] target, int offset, byte[] source)
                {
                    if (offset + source.Length >= target.Length)
                    {
                        throw new ArgumentOutOfRangeException("Attempted to write past end of target buffer");
                    }

                    for (int i = 0; i < source.Length; i++)
                    {
                        target[offset + i] = source[i];
                    }
                }
                byte[] bytes = new byte[2 * sizeof(int) + DREF_STRING_LEN];
                writeTo(bytes, 0, BitConverter.GetBytes(dref_freq));
                writeTo(bytes, sizeof(int), BitConverter.GetBytes(dref_en));
                writeTo(bytes, 2 * sizeof(int), dref_string.Select(c => (byte)c).ToArray<byte>());
                return bytes;
            }
        }

        private struct dref_struct_out
        {
            int dref_en;
            float dref_flt;
        }

        private const int DREF_PATH_LEN = 500;
        private struct dref_struct
        {
            float var;
            char[] dref_path;

            public dref_struct(float var, string path)
            {
                this.var = var;
                dref_path = new char[DREF_PATH_LEN];
                int i;
                for (i = 0; i < DREF_PATH_LEN -1 && i < path.Length; i++)
                {
                    dref_path[i] = path[i];
                }
                dref_path[i] = '\0';
                while (i < DREF_PATH_LEN - 1)
                {
                    dref_path[++i] = ' ';
                }
            }
        }

        public XP11Link(UDPSocket.LoggingCallback logCallback)
        {
            socket = new UDPSocket(logCallback);
            connected = false;
        }

        public void Connect()
        {
            connected = socket.Connect("127.0.0.1", 49000);
        }

        public void Disconnect()
        {
            socket.Disconnect();
        }

        public bool IsConnected()
        {
            return connected;
        }
        
        public string GetDataref(string dataref)
        {
            if (IsConnected())
            {
                socket.Send("RREF");
                return null;
            }
            else
            {
                return null;
            }
        }

        public void SetDataref(string dataref, object value)
        {
            throw new NotImplementedException("Method not yet implemented");
        }
    }
}
