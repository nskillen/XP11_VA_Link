using System;
using System.Linq;
using System.Text;

namespace XP11_VA_Link
{
    class DataRef
    {
        [Flags] public enum Type
        {
            Unknown = 0,
            Int = 1,
            Float = 2,
            Double = 4,
            FloatArray = 8,
            IntArray = 16,
            Data = 32
        }

        public string Name { get; set; }

        public object Value {
            get {
                switch (DataType)
                {
                    case Type.Int: return IntVal;
                    case Type.Float: return FloatVal;
                    case Type.Double: return DoubleVal;
                    case Type.FloatArray: return FloatArray;
                    case Type.IntArray: return IntArray;
                    case Type.Data: return Data;
                    default: throw new Exception(string.Format("Unexpected data type {0}", DataType));
                }
            }
        }

        public Type DataType { get; set; }
        public int IntVal { get; set; }
        public float FloatVal { get; set; }
        public double DoubleVal { get; set; }
        public float[] FloatArray { get; set; }
        public int[] IntArray { get; set; }
        public string Data { get; set; }

        public override string ToString()
        {
            try
            {
                StringBuilder sb = new StringBuilder();
                sb.Append(Name);
                sb.Append(":");
                sb.Append((int)DataType);
                sb.Append(":");

                if ((DataType & Type.Int) != 0)
                {
                    sb.Append(IntVal);
                }
                else if ((DataType & Type.Float) != 0)
                {
                    sb.Append(FloatVal);
                }
                else if ((DataType & Type.Double) != 0)
                {
                    sb.Append(DoubleVal);
                }
                else if ((DataType & Type.FloatArray) != 0)
                {
                    sb.Append(string.Join(",", FloatArray));
                }
                else if ((DataType & Type.IntArray) != 0)
                {
                    sb.Append(string.Join(",", IntArray));
                }
                else if ((DataType & Type.Data) != 0)
                {
                    sb.Append(Data);
                }
                else
                {
                    sb.Append("{unknown_datatype}");
                }

                return sb.ToString();
            }
            catch (Exception ex)
            {
                return ex.ToString();
            }
        }
    }
}
