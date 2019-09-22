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

        public static DataRef FromString(string dataRefName, string response)
        {
            int dataRefType = int.Parse(response.Substring(0, response.IndexOf(";")));
            string dataRefValue = response.Substring(response.IndexOf(";") + 1);
            
            try
            {
                DataRef r = new DataRef();
                r.Name = dataRefName;
                r.DataType = (Type)dataRefType;
                switch (r.DataType)
                {
                    case Type.Int:
                        r.IntVal = int.Parse(dataRefValue);
                        break;
                    case Type.Float:
                        r.FloatVal = float.Parse(dataRefValue);
                        break;
                    case Type.Double:
                        r.DoubleVal = double.Parse(dataRefValue);
                        break;
                    case Type.FloatArray:
                        string[] floats = dataRefValue.Split(',');
                        r.FloatArray = floats.Select(f => float.Parse(f)).ToArray();
                        break;
                    case Type.IntArray:
                        string[] ints = dataRefValue.Split(',');
                        r.IntArray = ints.Select(i => int.Parse(i)).ToArray();
                        break;
                    case Type.Data:
                        r.Data = dataRefValue;
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

        public static DataRef FromObject(string dataRefName, object value)
        {
            DataRef r = new DataRef();
            r.Name = dataRefName;

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
