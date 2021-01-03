using System;
using System.Linq;

namespace XP11_VA_Link
{
    class DatarefFactory
    {
        private Logger logger;

        public DatarefFactory(Logger logger)
        {
            this.logger = logger;
        }

        public DataRef FromString(string expectedDataref, string retrievedDataref)
        {
            logger.Trace("Requested to build dataref " + expectedDataref + " from " + retrievedDataref);
            char[] delims = { ':' };
            string[] parts = retrievedDataref.Split(delims, 3);
            if (parts.Length != 3)
            {
                logger.Error("Expected 3 parts to dataref message, but got " + parts.Length);
                return null;
            }
            string retrievedDatarefName = parts[0];
            int datarefType = int.Parse(parts[1]);
            string datarefValue = parts[2];

            if (retrievedDatarefName != expectedDataref)
            {
                logger.Error("Dataref name mismatch: " + retrievedDataref + " vs " + expectedDataref);
                return null;
            }

            try
            {
                DataRef r = new DataRef();
                r.Name = expectedDataref;
                r.DataType = (DataRef.Type)datarefType;
                if ((r.DataType & DataRef.Type.Int) != 0)
                {
                    r.IntVal = int.Parse(datarefValue);
                }
                else if ((r.DataType & DataRef.Type.Float) != 0)
                {
                    r.FloatVal = float.Parse(datarefValue);
                }
                else if ((r.DataType & DataRef.Type.Double) != 0)
                {
                    r.DoubleVal = double.Parse(datarefValue);
                }
                else if ((r.DataType & DataRef.Type.IntArray) != 0)
                {
                    r.FloatArray = datarefValue
                        .Split(',')
                        .Select(f => float.Parse(f))
                        .ToArray();
                }
                else if ((r.DataType & DataRef.Type.FloatArray) != 0)
                {
                    r.IntArray = datarefValue
                        .Split(',')
                        .Select(i => int.Parse(i))
                        .ToArray();
                }
                else if ((r.DataType & DataRef.Type.Data) != 0)
                {
                    r.Data = datarefValue;
                }
                else
                {
                    logger.Error("r.DataType does not match any type: " + r.DataType);
                    return null;
                }

                logger.Trace("Built dataref: " + r.ToString());
                return r;
            }
            catch (Exception ex)
            {
                logger.Error(ex.Message);
                return null;
            }
        }

        public DataRef FromObject(string datarefName, object value)
        {
            DataRef r = new DataRef();
            r.Name = datarefName;

            if (value is int)
            {
                r.DataType = DataRef.Type.Int;
                r.IntVal = (int)value;
            }
            else if (value is float)
            {
                r.DataType = DataRef.Type.Float;
                r.FloatVal = (float)value;
            }
            else if (value is double)
            {
                r.DataType = DataRef.Type.Double;
                r.DoubleVal = (double)value;
            }
            else if (value is float[])
            {
                r.DataType = DataRef.Type.FloatArray;
                r.FloatArray = (float[])value;
            }
            else if (value is int[])
            {
                r.DataType = DataRef.Type.IntArray;
                r.IntArray = (int[])value;
            }
            else if (value is string)
            {
                r.DataType = DataRef.Type.Data;
                r.Data = (string)value;
            }
            else
            {
                logger.Error("Unknown dataref type: " + r.DataType);
                return null;
            }

            return r;
        }
    }
}
