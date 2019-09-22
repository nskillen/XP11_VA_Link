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

        public void Log(Level level, string msg, string color)
        {
            if (level >= MinLevel)
            {
                logFn("[XP11_VA_Link :: " + level.ToString() + "] " + msg, color);
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
