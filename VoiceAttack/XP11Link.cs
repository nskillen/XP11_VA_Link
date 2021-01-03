using System;
using System.IO;
using System.IO.Pipes;
using System.Linq;

namespace XP11_VA_Link
{
    class XP11Link
    {
        private NamedPipeClientStream pipe;
        private StreamReader pipeReader;
        private StreamWriter pipeWriter;
        private Logger logger;
        private DatarefFactory factory;

        public XP11Link(Logger logger)
        {
            this.logger = logger;
            this.factory = new DatarefFactory(logger);

            pipe = new NamedPipeClientStream(".", "{2145AB63-BF83-40A4-8A9D-A358D45AF1C1}", PipeDirection.InOut);
            logger.Debug("Logger given to XP11Link");
        }

        public void Connect(bool reconnect = false)
        {
            if (IsConnected() && !reconnect) { return; }
            logger.Debug("Connecting...");
            if (pipe.IsConnected)
            {
                pipe.Close();
                pipe.Dispose();
            }
            pipe.Connect(5000);
            pipeReader = new StreamReader(pipe);
            pipeWriter = new StreamWriter(pipe);
            pipeWriter.AutoFlush = true;
            logger.Debug("Connection successful");
        }

        public void Disconnect()
        {
            logger.Debug("Disconnecting...");
            pipeReader = null;
            pipeWriter = null;
            pipe = null;
        }

        public bool IsConnected()
        {
            return pipe.IsConnected;
        }
        
        public DataRef GetDataref(string datarefName)
        {
            Connect();

            logger.Debug("Getting dataref...");
            pipeWriter.Write("get:" + datarefName);
            string reply = pipeReader.ReadLine();
            logger.Debug("GetDataRef result: " + reply);

            if (reply == "{invalid_dataref}")
            {
                return null;
            }

            return factory.FromString(datarefName, reply);
        }

        private int setDataRef(DataRef dataref)
        {
            try
            {
                logger.Debug("Setting dataref...");
                pipeWriter.Write("set:" + dataref);
                string reply = pipeReader.ReadLine();
                logger.Debug("SetDataRef result: " + reply);
                return reply == "{ok}" ? 0 : 1;
            }
            catch (IOException)
            {
                return 2;
            }
        }

        public bool SetDataref(DataRef dataref)
        {
            Connect();

            logger.Debug(string.Format("Setting dataref {0} to value {1}", dataref.Name, dataref.Value));

            int retryCount = 2;

            while (retryCount > 0)
            {
                retryCount -= 1;
                switch (setDataRef(dataref))
                {
                    case 0: /* success */
                        return true;
                    case 1: /* error */
                        return false;
                    case 2: /* pipe broken */
                        string logMsg = string.Format("Pipe broken - reconnecting and retrying (atttempt {0} of 2)", 2 - retryCount);
                        logger.Warn(logMsg);
                        Connect(true);
                        break;
                }
            }

            return false;
        }

        public bool BeginCommand(string command)
        {
            Connect();

            string[] commands = command.Split(';');

            pipeWriter.Write(string.Join(";", commands.Select(c => "cmd:" + c + ":begin")));
            string[] replies = pipeReader.ReadLine().Split(';');
            bool all_good = true;

            for (var i = 0; i < replies.Length; i++)
            {
                var reply = replies[i];
                if ("{invalid_command}" == reply)
                {
                    logger.Warn("Command " + commands[i] + " does not exist in X-Plane");
                    return false;
                }
                else
                {
                    all_good &= "{ok}" == reply;
                }
            }

            return all_good;
        }

        public bool EndCommand(string command)
        {
            Connect();

            string[] commands = command.Split(';');

            pipeWriter.Write(string.Join(";", commands.Select(c => "cmd:" + c + ":end")));
            string[] replies = pipeReader.ReadLine().Split(';');
            bool all_good = true;

            for (var i = 0; i < replies.Length; i++)
            {
                var reply = replies[i];
                if ("{invalid_command}" == reply)
                {
                    logger.Warn("Command " + commands[i] + " does not exist in X-Plane");
                    return false;
                }
                else
                {
                    all_good &= "{ok}" == reply;
                }
            }

            return all_good;
        }

        public bool OnceCommand(string command)
        {
            Connect();

            string[] commands = command.Split(';');

            pipeWriter.Write(string.Join(";", commands.Select(c => "cmd:" + c + ":once")));
            string[] replies = pipeReader.ReadLine().Split(';');
            bool all_good = true;

            for (var i = 0; i < replies.Length; i++)
            {
                var reply = replies[i];
                if ("{invalid_command}" == reply)
                {
                    logger.Warn("Command " + commands[i] + " does not exist in X-Plane");
                    return false;
                }
                else
                {
                    all_good &= "{ok}" == reply;
                }
            }

            return all_good;
        }

        public bool HoldCommand(string command, int duration_ms)
        {
            Connect();

            string[] commands = command.Split(';');

            pipeWriter.Write(string.Join(";", commands.Select(c => "cmd:" + c + ":hold:" + duration_ms)));
            string[] replies = pipeReader.ReadLine().Split(';');
            bool all_good = true;

            for (var i = 0; i < replies.Length; i++)
            {
                var reply = replies[i];
                if ("{invalid_command}" == reply)
                {
                    logger.Warn("Command " + commands[i] + " does not exist in X-Plane");
                    all_good = false;
                }
                else if ("{missing_hold_duration}" == reply)
                {
                    logger.Warn("Command " + commands[i] + " sent without hold duration");
                    all_good = false;
                }
                else
                {
                    all_good &= "{ok}" == reply;
                }
            }

            return all_good;
        }
    }
}
