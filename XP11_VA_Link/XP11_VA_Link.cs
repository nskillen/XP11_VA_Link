﻿using System;

namespace XP11_VA_Link
{
    public class XP11_VA_Link
    {
        private static XP11Link link = null;
        private static Logger logger;

        public static Guid VA_Id()
        {
            return new Guid("C10DE7EF-0FF0-4A10-876F-7300DA04F094");
        }

        public static string VA_DisplayName()
        {
            return "X-Plane 11 / VoiceAttack Link";
        }

        public static string VA_DisplayInfo()
        {
            return "";
        }

        /*
         * dynamic vaProxy description
         *
         * vaProxy has the following properties:
         *
         * vaProxy.Context                     :: string                           Read-only string set by "Plugin Context" box in the "Execute External Plugin Function" screen
         * vaProxy.SessionState                :: Dictionary<string, object>       Read-write value accessible in VA_Init1, VA_Invoke1, and VA_Exit1. Serves as session storage for the plugin.
         *                                                                         Initialized with 3 values:
         *                                                                         - VA_DIR    - install directory of VoiceAttack
         *                                                                         - VA_APPS   - apps/plugins directory of VoiceAttack
         *                                                                         - VA_SOUNDS - sounds directory of VoiceAttack
         * vaProxy.ProxyVersion                :: System.Version                   The current version of VoiceAtack
         * vaProxy.Stopped                     :: bool                             Returns true when the user has clicked the "Stop All Commands" button, or a "Stop All Commands" action has been issued
         * vaProxy.InstallDir                  :: string                           The VoiceAttack install folder
         * vaProxy.SoundsDir                   :: string                           The VoiceAttack sound pack folder
         * vaProxy.AppsDir                     :: string                           The VoiceAttack apps/plugins folder
         * vaProxy.IsRelease                   :: bool                             Whether the current version of VoiceAttack is a full release version (or, presumably, a dev/beta build)
         * vaProxy.IsTrial                     :: bool                             Whether the user is using VoiceAttack in trial mode, or has purchased a license
         * vaProxy.PluginsEnabled              :: bool                             Whether plugins have been enabled (intended for inline functions, not plugins).
         * vaProxy.NestedTokensEnabled         :: bool                             Whether the "Use Nested Tokens" option has been enabled
         * vaProxy.AutoProfileSwitchingEnabled :: bool                             Whether automatic profile switching has been enabled
         * vaProxy.MainWindowHandle            :: IntPtr                           VoiceAttack's main window handle
         *
         * vaProxy has the following setter functions:
         * vaProxy.SetSmallInt                 :: void(string name, short? val)    Sets a small integer variable
         * vaProxy.SetInt                      :: void(string name, int? val)      Sets an integer variable
         * vaProxy.SetDecimal                  :: void(string name, decimal? val)  Sets a decimal variable
         * vaProxy.SetText                     :: void(string name, string val)    Sets a text variable
         * vaProxy.SetBoolean                  :: void(string name, bool? val)     Sets a boolean variable
         * vaProxy.SetDate                     :: void(string name, DateTime? val) Sets a date variable
         *                                     
         * and the corresponding getter functions:
         * vaProxy.GetSmallInt                 :: short?(string name)              Gets a small integer variable, returns null if the variable does not exist
         * vaProxy.GetInt                      :: int?(string name)                Gets an integer variable, returns null if the variable does not exist
         * vaProxy.GetDecimal                  :: decimal?(string name)            Gets a decimal variable, returns null if the variable does not exist
         * vaProxy.GetText                     :: string(string name)              Gets a text variable, returns null if the variable does not exist
         * vaProxy.GetBoolean                  :: bool?(string name)               Gets a boolean variable, returns null if the variable does not exist
         * vaProxy.GetDate                     :: DateTime?(string name)           Gets a date variable, returns null if the variable does not exist
         *                                     
         * and the following other functions:
         * vaProxy.ProfileNames                :: string[]()                       Returns a string array of all profile names
         * vaProxy.WriteToLog                  :: void(string val, string color)   Writes val to the log, with the specified color. Acceptable colors are [red, blue, green, yellow, orange, purple, blank, grey, black, pink]
         * vaProxy.ClearLog                    :: void()                           Clears the log
         * vaProxy.SetOpacity                  :: void(int)                        Sets main screen opacity. Value can be between 0 and 100 inclusive
         * vaProxy.PluginPath                  :: string()                         Contains the full path of the currently executing plugin
         * vaProxy.ResetStopFlag               :: void()                           Resets the flag used by the "Stop All Commands" action (see vaProxy.Stopped)
         * vaProxy.Close                       :: void()                           Closes the VoiceAttack main window (Same as clicking the X in the top-right)
         */

        /// <summary>
        /// Called once when the plugin is initialized
        /// </summary>
        /// <param name="vaProxy"></param>
        public static void VA_Init1(dynamic vaProxy)
        {

            logger = new Logger((msg, color) => vaProxy.WriteToLog(msg, color));
            logger.MinLevel = Logger.Level.Trace;
            vaProxy.WriteToLog("VA_Init1", "green");
            link = new XP11Link(logger);
        }

        /// <summary>
        /// Called once when VoiceAttack exits
        /// </summary>
        /// <param name="vaProxy"></param>
        public static void VA_Exit1(dynamic vaProxy)
        {
            link = null;
        }

        /// <summary>
        /// Called whenever the user triggers a 'stop all commands' action
        /// </summary>
        public static void VA_StopCommand()
        {
            // Not sure if I'll need this
        }

        /// <summary>
        /// Called via 'Execute external plugin function' action in voiceattack
        /// </summary>
        /// <param name="vaProxy"></param>
        public static void VA_Invoke1(dynamic vaProxy)
        {
            try
            {
                vaProxy.SetBoolean("xp11_va_link_complete", false);
                try
                {
                    logger.Debug("Context: " + vaProxy.Context);
                    switch (vaProxy.Context)
                    {
                        case "GetDataref":
                            GetDataRef(vaProxy);
                            break;
                        case "SetDataref":
                            SetDataRef(vaProxy);
                            break;
                        case "GetLogLevel":
                            vaProxy.SetText("logLevel", logger.MinLevel.ToString());
                            break;
                        case "SetLogLevel":
                            string logLevel = vaProxy.GetText("logLevel");
                            if (logLevel != null)
                            {
                                try
                                {
                                    Logger.Level level = (Logger.Level)Enum.Parse(typeof(Logger.Level), logLevel, true);
                                    logger.MinLevel = level;
                                    logger.Log(level, "Minimum log level is now " + logLevel, "black");
                                }
                                catch (Exception e)
                                {
                                    logger.Error("Failed to set log level: " + e.Message);
                                }
                            }
                            break;
                        default:
                            logger.Warn(string.Format("Unknown context variable: {0}", vaProxy.Context));
                            break;
                    }
                }
                finally
                {
                    vaProxy.SetBoolean("xp11_va_link_complete", true);
                }
            }
            catch (Exception e)
            {
                logger.Critical(e.StackTrace);
                throw e;
            }
        }

        private static void GetDataRef(dynamic vaProxy)
        {
            string dataRefName = vaProxy.GetText("dataRefName");
            string targetVar = vaProxy.GetText("targetVar");
            if (targetVar == null) { targetVar = "dataRefValue"; }

            DataRef dr = link.GetDataref(dataRefName);
            if (dr != null)
            {
                logger.Info("Got dataref: " + dr.ToString());
                switch (dr.DataType)
                {
                    case DataRef.Type.Int:
                        vaProxy.SetInt(targetVar, dr.IntVal);
                        break;
                    case DataRef.Type.Float:
                        vaProxy.SetDecimal(targetVar, dr.FloatVal);
                        break;
                    case DataRef.Type.Double:
                        vaProxy.SetDecimal(targetVar, dr.DoubleVal);
                        break;
                    case DataRef.Type.FloatArray:
                        throw new ArgumentException("Array datarefs are not yet supported");
                    case DataRef.Type.IntArray:
                        throw new ArgumentException("Array datarefs are not yet supported");
                    case DataRef.Type.Data:
                        vaProxy.SetString(targetVar, dr.Data);
                        break;
                    case DataRef.Type.Unknown:
                    default:
                        throw new ArgumentException("Unknown dataref type: " + dr.DataType);
                }
            } else
            {
                logger.Warn("Failed to get dataref: " + dataRefName);
            }
        }

        private static void SetDataRef(dynamic vaProxy)
        {
            string dataRefName = vaProxy.GetText("dataRefName");
            int dataRefType = ((int?)vaProxy.GetInt("dataRefType")).GetValueOrDefault(0);
            object dataRefValue = null;

            switch ((DataRef.Type)dataRefType)
            {
                case DataRef.Type.Int:
                    int? ival = (int?)vaProxy.GetInt("dataRefValue");
                    if (ival.HasValue)
                    {
                        dataRefValue = ival.Value;
                    }
                    break;
                case DataRef.Type.Float:
                    decimal? fval = (decimal?)vaProxy.GetDecimal("dataRefValue");
                    if (fval.HasValue)
                    {
                        dataRefValue = (float)fval.Value;
                    }
                    break;
                case DataRef.Type.Double:
                    decimal? dval = (decimal?)vaProxy.GetDecimal("dataRefValue");
                    if (dval.HasValue)
                    {
                        dataRefValue = (double)dval.Value;
                    }
                    break;
                case DataRef.Type.FloatArray:
                    throw new ArgumentException("Array datarefs are not yet supported");
                case DataRef.Type.IntArray:
                    throw new ArgumentException("Array datarefs are not yet supported");
                case DataRef.Type.Data:
                    dataRefValue = (string)vaProxy.GetText("dataRefValue");
                    break;
                case DataRef.Type.Unknown:
                default:
                    throw new ArgumentException("Unsupported data type: " + dataRefType);
            }

            DataRef dr = DataRef.FromObject(dataRefName, dataRefValue);
            if (dr != null)
            {
                logger.Debug("Will set dataref: " + dr.ToString());
                if (link.SetDataref(dr))
                {
                    logger.Info("Successfully set dataref: " + dr.ToString());
                }
                else
                {
                    logger.Warn("Failed to set dataref: " + dr.ToString());
                }
            }
            else
            {
                logger.Error("Failed to convert input into dataref");
                logger.Debug("dataRefName: " + dataRefName);
                logger.Debug("dataRefType: " + dataRefType);
                logger.Debug("dataRefValue: " + dataRefValue);
            }
        }
    }
}
