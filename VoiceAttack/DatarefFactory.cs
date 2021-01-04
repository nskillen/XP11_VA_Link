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

        public DataRef FromObject(string datarefName, int datarefType, object value)
        {
            DataRef r = new DataRef();
            r.Name = datarefName;

            switch (datarefType)
            {
                case 1:
                    r.DataType = DataRef.Type.Int;
                    r.IntVal = (int)value;
                    break;
                case 2:
                    r.DataType = DataRef.Type.Float;
                    decimal df = (decimal)value;
                    r.FloatVal = (float)df;
                    break;
                case 4:
                    r.DataType = DataRef.Type.Double;
                    decimal dd = (decimal)value;
                    r.DoubleVal = (double)dd;
                    break;
                case 8:
                    r.DataType = DataRef.Type.FloatArray;
                    r.FloatArray = (float[])value;
                    break;
                case 16:
                    r.DataType = DataRef.Type.IntArray;
                    r.IntArray = (int[])value;
                    break;
                case 32:
                    r.DataType = DataRef.Type.Data;
                    r.Data = (string)value;
                    break;
                default:
                    logger.Error("Unknown dataref type: " + datarefType);
                    return null;
            }

            return r;
        }
    }
}
