using System;
using System.Net.Sockets;
using System.Text;

namespace XP11_VA_Link
{
    class UDPSocket
    {
        public delegate void LoggingCallback(string log);

        private Socket socket;
        private const int bufSize = 8 * 1024; // 8kB
        private State state;
        private LoggingCallback logCallback;
        private AsyncCallback asyncCallback;


        public class State
        {
            public byte[] buffer = new byte[bufSize];
        }

        public UDPSocket(LoggingCallback logCallback = null)
        {
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            state = new State();
            this.logCallback = logCallback;
        }

        public bool Connect(string address, int port)
        {
            socket.Connect(address, port);
            return socket.Connected;
        }

        public void Disconnect()
        {
            socket.Disconnect(false);
        }

        public void Send(string text)
        {
            byte[] data = Encoding.ASCII.GetBytes(text);
            socket.BeginSend(data, 0, data.Length, SocketFlags.None, (asyncResult) =>
            {
                State so = (State)asyncResult.AsyncState;
                int bytes = socket.EndSend(asyncResult);
                logCallback?.Invoke(string.Format("SEND: {0} {1}", bytes, text));
            }, state);
        }

        public string Receive()
        {
            socket.BeginReceive(state.buffer, 0, bufSize, SocketFlags.None, asyncCallback = (asyncResult) =>
            {
                State so = (State)asyncResult.AsyncState;
                int bytes = socket.EndReceive(asyncResult);
                socket.BeginReceive(so.buffer, 0, bufSize, SocketFlags.None, asyncCallback, so);
                string text = Encoding.ASCII.GetString(so.buffer, 0, bytes);
                logCallback?.Invoke(string.Format("RECV: {0} {1}", bytes, text));
            }, state);
            throw new NotImplementedException("This function is not yet implemented");
        }
    }
}
