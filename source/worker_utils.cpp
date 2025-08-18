#include "worker_utils.h"

/*PUBLIC WORKER_UTILS FUNCTIONS*/

void Worker_utils::install(std::string user_name)
{
    std::ofstream sfile(m_service, std::ios::out);
    sfile << "[Unit]\n";
    sfile << "Description=echobreak\n";
    sfile << "After=multi-user.target\n";
    sfile << "\n";
    sfile << "[Service]\n";
    sfile << "Type=simple\n";
    sfile << "ExecStart=/bin/eb.net\n";
    sfile << "Restart=always\n";
    sfile << "User=" + user_name + "\n";
    sfile << "Group=nogroup\n";
    sfile << "\n";
    sfile << "[Install]\n";
    sfile << "WantedBy=multi-user.target";
    sfile.close();

    system("systemctl enable eb");
}

//Base
void Worker_utils::receive(std::vector<std::string> &messages)
{
    ssize_t bytes_received = recvfrom(m_rec_sock, m_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&m_rec_broadcast_addr, &m_rec_addr_len);
    if(bytes_received > 0){
        messages = split((std::string)m_buffer);
        memset(m_buffer, 0, sizeof(m_buffer));
    }
}

void Worker_utils::send(std::string data)
{
    for (int i = 0; i < m_msg_count; ++i)
    {
        sendto(m_send_sock, data.c_str(), data.size(), 0, (struct sockaddr *)&m_send_broadcast_addr, m_send_addr_len);
    }
}

/*PRIVATE WORKER_UTILS FUNCTIONS*/
void Worker_utils::init()
{
    
    m_broadcast_ip = getBroadCastAddress();

    // For sending
    if ((m_send_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        close(m_send_sock);
        err("Sending: socket err");
    }
    // enabling broadcast
    int opt = 1;
    if (setsockopt(m_send_sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
    {
        close(m_send_sock);
        err("Sending: setsockopt err");
    }

    memset(&m_send_broadcast_addr, 0, sizeof(m_send_broadcast_addr));
    m_send_broadcast_addr.sin_family = AF_INET;
    m_send_broadcast_addr.sin_port = htons(SENDING_PORT);
    m_send_broadcast_addr.sin_addr.s_addr = inet_addr(m_broadcast_ip.c_str());

    // For receiving
    memset(m_buffer, 0, sizeof(m_buffer));
    if ((m_rec_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        close(m_rec_sock);
        err("Receiving: socket err");
    }

    memset(&m_rec_broadcast_addr, 0, sizeof(m_rec_broadcast_addr));
    m_rec_broadcast_addr.sin_family = AF_INET;
    m_rec_broadcast_addr.sin_addr.s_addr = INADDR_ANY;
    m_rec_broadcast_addr.sin_port = htons(REC_PORT);

    if (bind(m_rec_sock, (struct sockaddr *)&m_rec_broadcast_addr, sizeof(m_rec_broadcast_addr)) < 0)
    {
        close(m_rec_sock);
        err("Receiving: binding err");
    }
}

void Worker_utils::err(std::string wtf)
{
    std::ofstream log("log", std::ios::app);
    log << wtf << std::endl;
    log.close();
    exit(EXIT_FAILURE);
}
