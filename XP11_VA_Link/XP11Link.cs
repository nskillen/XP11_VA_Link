using System;
using System.IO;
using System.IO.Pipes;

namespace XP11_VA_Link
{
    class XP11Link
    {
        private NamedPipeClientStream pipe;
        private StreamReader sr;
        private StreamWriter sw;
        private Logger logger;

        public XP11Link(Logger logger)
        {
            this.logger = logger;
            pipe = new NamedPipeClientStream(".", "{2145AB63-BF83-40A4-8A9D-A358D45AF1C1}", PipeDirection.InOut);
            logger.Debug("Logger given to XP11Link");
        }

        public void Connect()
        {
            logger.Debug("Connecting...");
            pipe.Connect(5000);
            sr = new StreamReader(pipe);
            sw = new StreamWriter(pipe);
            sw.AutoFlush = true;
            logger.Debug("Connection successful");
        }

        public void Disconnect()
        {
            logger.Debug("Disconnecting...");
            sr = null;
            sw = null;
            pipe = null;
        }

        public bool IsConnected()
        {
            return pipe.IsConnected;
        }
        
        public DataRef GetDataref(string dataRefName)
        {
            logger.Debug("Getting dataref...");
            if (!IsConnected())
            {
                Connect();
            }
            
            sw.Write("get;" + dataRefName);
            string reply = sr.ReadLine();
            logger.Debug("GetDataRef result: " + reply);
            if (reply == "{invalid_dataref}")
            {
                return null;
            }

            return DataRef.FromString(dataRefName, reply);
        }

        public bool SetDataref(DataRef dataRef)
        {
            logger.Debug("Setting dataref...");
            if (!IsConnected())
            {
                Connect();
            }

            sw.Write("set;" + dataRef);
            string reply = sr.ReadLine();
            logger.Debug("SetDataRef result: " + reply);
            return reply == "{ok}";
        }
    }
}
