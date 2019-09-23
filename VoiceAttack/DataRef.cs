using System;
using System.Linq;
using System.Text;

namespace XP11_VA_Link
{
    class DataRef
    {
        public enum Type
        {
            Unknown = 0,
            Int = 1,
            Float = 2,
            Double = 4,
            FloatArray = 8,
            IntArray = 16,
            Data = 32
        }

        public string Name { get; private set; }

        public Type DataType { get; private set; }
        public int IntVal { get; private set; }
        public float FloatVal { get; private set; }
        public double DoubleVal { get; private set; }
        public float[] FloatArray { get; private set; }
        public int[] IntArray { get; private set; }
        public string Data { get; private set; }

        public static DataRef FromString(string datarefName, string response)
        {
            char[] delims = { ';' };
            string[] parts = response.Split(delims, 3);
            if (parts.Length != 3)
            {
                return null;
            }
            string retrievedDatarefName = parts[0];
            int datarefType = int.Parse(parts[1]);
            string datarefValue = parts[2];

            if (retrievedDatarefName != datarefName)
            {
                return null;
            }
            
            try
            {
                DataRef r = new DataRef();
                r.Name = datarefName;
                r.DataType = (Type)datarefType;
                switch (r.DataType)
                {
                    case Type.Int:
                        r.IntVal = int.Parse(datarefValue);
                        break;
                    case Type.Float:
                        r.FloatVal = float.Parse(datarefValue);
                        break;
                    case Type.Double:
                        r.DoubleVal = double.Parse(datarefValue);
                        break;
                    case Type.FloatArray:
                        string[] floats = datarefValue.Split(',');
                        r.FloatArray = floats.Select(f => float.Parse(f)).ToArray();
                        break;
                    case Type.IntArray:
                        string[] ints = datarefValue.Split(',');
                        r.IntArray = ints.Select(i => int.Parse(i)).ToArray();
                        break;
                    case Type.Data:
                        r.Data = datarefValue;
                        break;
                    case Type.Unknown:
                    default:
                        // TODO: log unknown data ref type
                        return null;
                }
                return r;
            } catch
            {
                // TODO: log error
                return null;
            }
        }

        public static DataRef FromObject(string datarefName, object value)
        {
            DataRef r = new DataRef();
            r.Name = datarefName;

            if (value is int)
            {
                r.DataType = Type.Int;
                r.IntVal = (int)value;
            }
            else if (value is float)
            {
                r.DataType = Type.Float;
                r.FloatVal = (float)value;
            }
            else if (value is double)
            {
                r.DataType = Type.Double;
                r.DoubleVal = (double)value;
            }
            else if (value is float[])
            {
                r.DataType = Type.FloatArray;
                r.FloatArray = (float[])value;
            }
            else if (value is int[])
            {
                r.DataType = Type.IntArray;
                r.IntArray = (int[])value;
            }
            else if (value is string)
            {
                r.DataType = Type.Data;
                r.Data = (string)value;
            }
            else
            {
                // TODO: handle unknown value type
                return null;
            }

            return r;
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(Name);
            sb.Append(";");
            sb.Append((int)DataType);
            sb.Append(";");
            switch (DataType)
            {
                case Type.Int:
                    sb.Append(IntVal);
                    break;
                case Type.Float:
                    sb.Append(FloatVal);
                    break;
                case Type.Double:
                    sb.Append(DoubleVal);
                    break;
                case Type.FloatArray:
                    sb.Append(string.Join(",", FloatArray));
                    break;
                case Type.IntArray:
                    sb.Append(string.Join(",", IntArray));
                    break;
                case Type.Data:
                    sb.Append(Data);
                    break;
                case Type.Unknown:
                default:
                    // TODO: handle bad dataref
                    return null;
            }
            return sb.ToString();
        }
    }
}
