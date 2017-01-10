using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RestCSharpCHAOS
{
     public enum SnapshotOperations {create,delete,restore}
     public enum VariableOperations { set, get, del }
     
     
     class RestAPI
    {

        private String coords;
        private  UInt32 retry;
        public static long ToEpoch(DateTime dateTime) { return (long)(dateTime - new DateTime(1970, 1, 1)).TotalSeconds*1000; }
        public static DateTime FromEpoch(long epoch) { return new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Unspecified).AddSeconds(epoch/1000); }

        public static String GetHtmlAnswer(String Question)
        {
            System.Net.WebResponse resp;
            String result = String.Empty;

            try
            {
                System.Net.HttpWebRequest myReq = (System.Net.HttpWebRequest)(System.Net.WebRequest.Create(Question));
                myReq.Timeout = 3000;
                myReq.Accept = "\r\n";
                resp = myReq.GetResponse();


            }

            catch (System.Net.WebException)
            {
                return String.Empty;
            }
            System.IO.Stream ReceiveStream = resp.GetResponseStream();
            System.Text.Encoding encode = System.Text.Encoding.GetEncoding("utf-8");
            System.IO.StreamReader readStream = new System.IO.StreamReader(ReceiveStream, encode);

            char[] read = new char[256];
            int count = readStream.Read(read, 0, 256);
            while (count > 0)
            {
                // Dump the 256 characters on a string and display the string onto the console.
                String str = new String(read, 0, count);
                result += str;
                count = readStream.Read(read, 0, 256);
            }
            return result;
        }
        public RestAPI(String IP, UInt32 port)
        {
            coords = IP + ":" + port.ToString();
            retry = 5;


        }
        public RestAPI(String completeIP)
        {
            coords = completeIP;
            retry = 5;
        }

        public String queryhst(String device,Int64 startmsecEpoch, Int64 stopmsecEpoch)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=queryhst&parm={\"start\":" + startmsecEpoch.ToString() + ",\"end\":" + stopmsecEpoch.ToString() + "}";
            for ( UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String queryhst(String device, DateTime start, DateTime stop)
        {
            Int64 startEpoch, stopEpoch;
            startEpoch = RestAPI.ToEpoch(start);
            stopEpoch = RestAPI.ToEpoch(stop);
            return this.queryhst(device, startEpoch, stopEpoch);
            
        }
        public String queryhst(String device, DateTime start, DateTime stop,String param)
        {
            Int64 startEpoch, stopEpoch;
            startEpoch = RestAPI.ToEpoch(start);
            stopEpoch = RestAPI.ToEpoch(stop);
            return this.queryhst(device, startEpoch, stopEpoch,param);

        }
        public String queryhst(String device, Int64 startmsecEpoch, Int64 stopmsecEpoch, String param)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=queryhst&parm={\"start\":" + startmsecEpoch.ToString() + ",\"end\":" + stopmsecEpoch.ToString() + ",\"var\":\"" +param+"\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String queryhst(String device, DateTime start, DateTime stop, String param,UInt32 ElementsPerPage)
        {
            Int64 startEpoch, stopEpoch;
            startEpoch = RestAPI.ToEpoch(start);
            stopEpoch = RestAPI.ToEpoch(stop);
            return this.queryhst(device, startEpoch, stopEpoch, param,ElementsPerPage);

        }
        public String queryhst(String device, Int64 startmsecEpoch, Int64 stopmsecEpoch, String param,UInt32 ElementsPerPage)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=queryhst&parm={\"start\":" + startmsecEpoch.ToString() + ",\"end\":" + stopmsecEpoch.ToString() + ",\"var\":\"" + param + "\",\"page\":"+ElementsPerPage.ToString()+"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }

        public String queryhstnext(String device,UInt32 iud)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=queryhstnext&parm={\"uid\":" + iud.ToString()+ "}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String queryhstnext(String device, UInt32 iud,String param)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=queryhstnext&parm={\"uid\":" + iud.ToString() + ",\"var\":\"" + param + "\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }

        public String queryinfo(String device)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=queryinfo";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String queryinfo(String device,UInt32 IdToClear)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=queryinfo&parm={\"clearuid\":" + IdToClear.ToString() + "}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }

        public bool queryClean(String device)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=queryinfo";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                {
                    Ans=Ans.Trim('[',']');
                    String[] Tok = Ans.Split(',');
                    foreach (String iter in Tok)
                    {
                        UInt32 count1;
                        query = this.coords + "/CU?dev=" + device + "&cmd=queryinfo&parm={\"clearuid\":" + iter + "}";
                        for ( count1 = 0; count1 < this.retry; count1++)
                        {
                            Ans = GetHtmlAnswer(query);
                            if (Ans != String.Empty)
                                break;
                        }
                        if (count1 == this.retry)
                            return false;
                    }
                    return true;    
                }
                

            }
            return false;
        }
        
         public String search(bool alive=false,String name="",String what="cu")
        {
            String query = this.coords + "/CU?&cmd=search&parm={alive:"+alive.ToString().ToLower()+",\"what\":\""+what+"\"";
            if (name != String.Empty)
                query += ",\"name\":\"" + name + "\"";
            query += "}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;

        }
        
        public bool snapshot(String snapName,SnapshotOperations Kind)
        {
            String Val = Kind.ToString();
            String query = this.coords + "/CU?&cmd=snapshot&parm={\"name\":\""+snapName+"\",\"what\":\""+Val+"\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                {
                    if (Ans == "[]")
                        return true;
                    else
                        return false;
                }

            }
            return false;
        }
        public bool snapshot(String snapName, SnapshotOperations Kind,String JsonVectorNodeList)
        {
            String Val = Kind.ToString();
            String query = this.coords + "/CU?&cmd=snapshot&parm={\"name\":\"" + snapName + "\",\"what\":\"" + Val + "\",\"node_list\":\""+JsonVectorNodeList+"\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                {
                    if (Ans == "[]")
                        return true;
                    else
                        return false;
                }

            }
            return false;
        }
        
         public String variable(String name, VariableOperations Kind, String Value = "")
        {
            String Val = Kind.ToString();
            String query = this.coords + "/CU?&cmd=variable&parm={\"name\":\""+name+"\",\"what\":\""+Val+"\"";
            if ((Kind == VariableOperations.set) && (Value == String.Empty))
                return String.Empty;
            if (Value != String.Empty)
                query += ",\"value\":" + Value ;
            query += "}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }

        
      
        public String channel(String device, Int32 chan)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=channel&parm="+chan.ToString();
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        private String SendCommandWithoutParameter(String device, String command)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=" + command;
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String status(String device) { return SendCommandWithoutParameter(device, "status"); } 
        public String init(String device) { return SendCommandWithoutParameter(device, "init"); }
        public String deinit(String device) { return SendCommandWithoutParameter(device, "deinit"); }
        public String start(String device) { return SendCommandWithoutParameter(device, "start"); }
        public String stop(String device) { return SendCommandWithoutParameter(device, "stop"); }
        public String recover(String device) { return SendCommandWithoutParameter(device, "recover"); }
        public String listSnapshot(String device) { return SendCommandWithoutParameter(device, "list"); }
        public String desc(String device) { return SendCommandWithoutParameter(device, "desc"); }
        public String sched(String device, UInt32 milliseconds)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=sched&parm=" + milliseconds.ToString();
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;

        }
        public String attr(String device, String Parameter, String value)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=attr&parm={\""+Parameter+"\":\""+value+"\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String readout(String device, String Parameter)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=readout&parm=" + Parameter;
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String readin(String device, String Parameter)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=readin&parm=" + Parameter ;
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String loadSnapshot(String device, String SnapName)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=load&parm={\"snapname\":\"" + SnapName + "\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String saveSnapshot(String device, String SnapName)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=save&parm={\"snapname\":\"" + SnapName + "\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String restoreSnapshot(String device, String SnapName)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=restore&parm={\"snapname\":\"" + SnapName + "\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String deleteSnapshot(String device, String SnapName)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=delete&parm={\"snapname\":\"" + SnapName + "\"}";
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;
        }
        public String SendCustomCommand(String device, String commandName, String JSonParams)
        {
            String query = this.coords + "/CU?dev=" + device + "&cmd=" + commandName;// +"&parm={\"snapname\":\"" + SnapName + "\"}";
            if (JSonParams != String.Empty)
            {
                query += "&parm=" + JSonParams;
            }
            for (UInt32 count = 0; count < this.retry; count++)
            {
                String Ans = GetHtmlAnswer(query);
                if (Ans != String.Empty)
                    return Ans;

            }
            return String.Empty;

        }

    }
    
}
