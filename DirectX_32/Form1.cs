using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using Nektra.Deviare2;
using System.IO.Pipes;


namespace DeviareTest
{
    public partial class Form1 : Form
    {
        private NktSpyMgr _spyMgr;
        private NktProcess _process;
        private object continueevent;
        private NamedPipeClientStream pipe;
        private int count;


        public Form1()
        {
            InitializeComponent();

            _spyMgr = new NktSpyMgr();
            _spyMgr.Initialize();
            _spyMgr.OnFunctionCalled += new DNktSpyMgrEvents_OnFunctionCalledEventHandler(OnFunctionCalled);

            _process = _spyMgr.CreateProcess(@"C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Samples\C++\Direct3D\Bin\x86\SkinnedMesh.exe", true, out continueevent);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            _spyMgr.LoadCustomDll(_process, @"..\..\..\Plugin\bin\Plugins\CRegistryPlugin.dll", false, false);


            pipe = new NamedPipeClientStream(".", "HyperPipe32", PipeDirection.InOut);
            pipe.Connect();

            byte[] data = new byte[4];
            pipe.Read(data, 0, 4);
            Int32 address = BitConverter.ToInt32(data, 0);


            NktHook hook = _spyMgr.CreateHookForAddress((IntPtr)address, "D3D9.DLL!CreateDevice", (int)(eNktHookFlags.flgRestrictAutoHookToSameExecutable | eNktHookFlags.flgOnlyPostCall | eNktHookFlags.flgDontCheckAddress));
            hook.AddCustomHandler(@"..\..\..\Plugin\bin\Plugins\CRegistryPlugin.dll", 0, "");

            hook.Attach(_process, true);
            hook.Hook(true);

            _spyMgr.ResumeProcess(_process, continueevent);
            
        }


        private void OnFunctionCalled(NktHook hook, NktProcess process, NktHookCallInfo hookCallInfo)
        {

            string strOnFunctionCalled = hook.FunctionName + "\n";

            if (hook.FunctionName.CompareTo("D3D9.DLL!CreateDevice") == 0)
            {
                INktParamsEnum paramsEnum = hookCallInfo.Params();

                INktParam param = paramsEnum.First();

                INktParam tempParam = null;

                while (param != null)
                {
                    tempParam = param;

                    param = paramsEnum.Next();
                }

                strOnFunctionCalled +=  " " + tempParam.PointerVal.ToString() + "\n";

            }

            Output(strOnFunctionCalled);
        }

        public delegate void OutputDelegate(string strOutput);

        private void Output(string strOutput)
        {
            if (InvokeRequired)
                BeginInvoke(new OutputDelegate(Output), strOutput);
            else
                textOutput.AppendText(strOutput);
        }

    }
}
