namespace XP11_VA_Link
{
    class Logger
    {
        public enum Level
        {
            Trace = 0,
            Debug = 1,
            Info = 2,
            Warning = 3,
            Error = 4,
            Critical = 5
        }

        public delegate void vaProxyLogFn(string msg, string color);

        private vaProxyLogFn logFn;
        public Level MinLevel { get; set; }


        public Logger(vaProxyLogFn fn)
        {
            logFn = fn;
            MinLevel = Level.Debug;
        }

        public void Log(Level level, string msg, string color = "black")
        {
            if (level >= MinLevel)
            {
                logFn("[XP11_VA_Link :: " + level.ToString() + "] " + msg, color);
            }
        }

        public void LogAtLevel(Level level, string msg)
        {
            switch (level)
            {
                case Level.Trace:
                    Trace(msg); break;
                case Level.Debug:
                    Debug(msg); break;
                case Level.Info:
                    Info(msg); break;
                case Level.Warning:
                    Warn(msg); break;
                case Level.Error:
                    Error(msg); break;
                case Level.Critical:
                    Critical(msg); break;
            }
        }

        public void Trace(string msg) { Log(Level.Trace, msg, "gray"); }
        public void Debug(string msg) { Log(Level.Debug, msg, "black"); }
        public void Info(string msg) { Log(Level.Info, msg, "black"); }
        public void Warn(string msg) { Log(Level.Warning, msg, "yellow"); }
        public void Error(string msg) { Log(Level.Error, msg, "red"); }
        public void Critical(string msg) { Log(Level.Critical, msg, "red"); }
    }
}
