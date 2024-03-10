#ifndef NETCODE_DISABLE
#include "config.h"

#define IRC_BUF_SIZE 4096

#ifdef NDEBUG
void netIRCPrintf(const char *fmt, ...)
{
    UNREFERENCED_PARAMETER(fmt);
    return;
}
#else
#define netIRCPrintf OSD_Printf
#endif

typedef struct {
    int sock;
    struct sockaddr_in addr;
    char buf[IRC_BUF_SIZE];
    int len;
    char nick[32];
    char chan[32];
    char name[32];
    char pong[64];
    int index;
    int state;
    int timer;
} ircd_t;

enum BLOOD_IRC_STATE {
    BLOOD_IRC_DISCONNECTED = 0,
    BLOOD_IRC_CONNECTED,
    BLOOD_IRC_INSIDE_ROOM,
    BLOOD_IRC_STATE_MAX
};

char gWanIp4[16] = "127.0.0.1";
char gIRCState = BLOOD_IRC_DISCONNECTED;
const char *kIRCServ = "irc.libera.chat";
const int kIRCPort = 6667; // clearnet irc
const char *kIRCChan = "##notbloodserv";

static int netIRCAppend(char *buf, const char *sep, const char *key)
{
    if (key && strlen(key))
    {
        if (strlen(buf))
        {
            strcat(buf, " ");
            if (sep)
                strcat(buf, sep);
        }
        strcat(buf, key);
    }
    return 0;
}

static void netIRCSend(int sock, char *text, const char *cmd, const char *opt)
{
    int len;
    char buf[IRC_BUF_SIZE] = {'\0'};
    netIRCAppend(buf, 0, cmd);
    netIRCAppend(buf, 0, opt);
    netIRCAppend(buf, ":", text);
    strcat(buf, "\n");
    len = strlen(buf);
    netIRCPrintf("[send %d bytes] %s", len, buf);
    send(sock, buf, len, 0);
}

static void ircd_parse(ircd_t *cl, char *buf, int len)
{
    char opt[IRC_BUF_SIZE];
    char from[IRC_BUF_SIZE];
    char mask[IRC_BUF_SIZE];
    char dest[IRC_BUF_SIZE];
    char cmd[IRC_BUF_SIZE];
    char text[IRC_BUF_SIZE];
    int code;

    int sock = cl->sock;

    if (cl->state == 0)
    {
        netIRCSend(sock, 0, "NICK", cl->nick);
        Bsprintf(opt, "%s %s %s", "jpilot", "test", "localhost");
        netIRCSend(sock, cl->name, "USER", opt);
        cl->state = 1;
    }
    else
    {
        if (Bsscanf(buf, "PING :%s", text) == 1)
        {
            netIRCSend(sock, text, "PONG", "");
        }
        else if (Bsscanf(buf, ":%s %03d %s :%[^\r^\n]", from, &code, dest, text) > 1)
        {
            switch (code)
            {
                case 1:
                    netIRCSend(sock, 0, "JOIN", cl->chan);
                    break;
            }
        }
        else if (Bsscanf(buf, ":%[^!]!%s %s %s :%[^\r^\n]", from, mask, cmd, dest, text) >= 3)
        {
            const char bSelf = Bstrncmp(from, cl->nick, len) == 0;
            if (!Bstrncmp(cmd, "JOIN", len))
            {
                const char bChanServ = Bstrncmp(from, "ChanServ", len) == 0;
                if (bSelf) // we finally joined the channel
                {
                    gIRCState = BLOOD_IRC_INSIDE_ROOM;
                    if (gNetMode == NETWORK_NONE)
                        NetworkBrowserState("CONNECTED TO MASTER LIST");
                }
                if ((gNetMode == NETWORK_SERVER) && (gIRCState == BLOOD_IRC_INSIDE_ROOM) && !bChanServ)
                {
                    char pubaddress[128];
                    Bsprintf(pubaddress, "BLADR_%s_%05d", gWanIp4, ClipRange(gNetPort, 0, 65353));
                    for (int i = 0; i < MAXPLAYERNAME-1; i++) // last character is always null, so skip it
                        Bsprintf(pubaddress, "%s-%02d", pubaddress, (int)szPlayerName[i]&0x7F);
                    Bsprintf(pubaddress, "%s_%01d_%01d_BLADR", pubaddress, numplayers, gNetPlayers);
                    netIRCSend(sock, pubaddress, "PRIVMSG", cl->chan);
                }
            }
            else if ((!Bstrncmp(cmd, "PART", len) && bSelf) || !Bstrncmp(cmd, "KICK", len)) // we left channel/someone was kicked/something horrible has happened, close irc connection
            {
                gIRCState = BLOOD_IRC_CONNECTED;
                if (gNetMode == NETWORK_NONE)
                    NetworkBrowserState("DISCONNECTED");
            }
            else if ((gNetMode == NETWORK_NONE) && !Bstrncmp(cmd, "PRIVMSG", len) && !Bstrncmp(dest, cl->chan, len) && !Bstrncmp(text, "BLADR", 5)) // test
            {
                char sIp[16] = {'\0'}, sName[MAXPLAYERNAME] = {'\0'};
                int nIp[4] = {0}, nPort = 0, nName[MAXPLAYERNAME] = {0}, nClientsCur, nClientsMax;
                if (Bsscanf(text, "BLADR_%d.%d.%d.%d_%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d_%d_%d_BLADR", &nIp[0], &nIp[1], &nIp[2], &nIp[3], &nPort, &nName[0], &nName[1], &nName[2], &nName[3], &nName[4], &nName[5], &nName[6], &nName[7], &nName[8], &nName[9], &nName[10], &nName[11], &nName[12], &nName[13], &nName[14], &nClientsCur, &nClientsMax) == 22) // we found a valid server
                {
                    Bsprintf(sIp, "%d.%d.%d.%d", nIp[0]&0xFF, nIp[1]&0xFF, nIp[2]&0xFF, nIp[3]&0xFF);
                    for (int i = 0; i < MAXPLAYERNAME-1; i++) // last character is always null, so skip it
                        sName[i] = nName[i];
                    if (!Bstrncmp(&text[6], sIp, Bstrlen(sIp)) && (nPort == ClipRange(nPort, 0, 65353))) // if the sanitized string ip we've parsed matches the original string
                    {
                        netIRCPrintf("Found: %d.%d.%d.%d:%d (Host: %s, Players: %d\\%d)\n", nIp[0], nIp[1], nIp[2], nIp[3], nPort, sName, nClientsCur, nClientsMax);
                        NetworkBrowserAdd(sIp, nPort, sName, nClientsCur, nClientsMax);
                        return;
                    }
                }
                netIRCPrintf("Could not parse server address, ignoring\n");
            }
        }
    }
}

static int ircd_init(ircd_t *cl, const char *name, int port)
{
    struct sockaddr_in addr;
    struct hostent *hp;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = 0;
    hp = gethostbyname(name);
    if (hp)
        addr.sin_addr.s_addr = *(unsigned int *)hp->h_addr_list[0];

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        netIRCPrintf("failed to connect to %s, closing...\n", name);
        return 0;
    }
    netIRCPrintf("connected to %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

#ifdef WIN32 // setnonblock
    unsigned long on = 1;
    ioctlsocket(sock, FIONBIO, &on);
#else
    fcntl(sock, F_SETFL, O_NONBLOCK, 1);
#endif
    cl->sock = sock;
    cl->state = 0;
    cl->len = 0;
    ircd_parse(cl, 0, 0); // initialize state
    return 1;
}

static int ircd_update(ircd_t *cl)
{
    int len;
    char *p;
    char buf[IRC_BUF_SIZE];
    int sock = cl->sock;

    len = recv(sock, buf, IRC_BUF_SIZE - 1, 0);
    if (len <= 0) // no response yet
        return 0;

    buf[len] = '\0';
    netIRCPrintf("[recv %d bytes] %s\n", len, buf);

    if (cl->len + len < IRC_BUF_SIZE)
    {
        memcpy(cl->buf + cl->len, buf, len);
        cl->len += len;
    }

    while ((p = (char *)memchr(cl->buf, '\n', cl->len)))
    {
        len = (p - cl->buf) + 1;
        memmove(buf, cl->buf, len);
        memmove(cl->buf, cl->buf + len, IRC_BUF_SIZE - len);
        cl->len -= len;
        buf[len - 1] = 0;
        ircd_parse(cl, buf, len);
    }

    return 1;
}

int netGetWanIp4(void)
{
    struct sockaddr_in server;
    char server_reply[2048];
    const char *request = "GET / HTTP/1.0\r\nHost: whatismyip.akamai.com\r\n\r\n";
    int recv_size;
    struct hostent *hp;
    static char bSuccess = 0;

    if (bSuccess) // we've already collected our wan ip, don't bother doing this again
        return bSuccess;

#ifdef _WIN32
    WSADATA wsa;
    SOCKET s = INVALID_SOCKET;
    netIRCPrintf("\nInitialising Winsock...\n");
    if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0)
    {
        netIRCPrintf("Winsock unable to initialize. Error Code: %d\n", WSAGetLastError());
        return bSuccess;
    }
    netIRCPrintf("Initialized\n");
#else
    int s = -1;
#endif

    s = socket(AF_INET, SOCK_STREAM, 0); // create sock
#ifdef _WIN32
    if (s == INVALID_SOCKET)
    {
        netIRCPrintf("Could not create socket: %d", WSAGetLastError());
#else
    if (s < 0)
    {
        netIRCPrintf("Could not create socket\n");
#endif
        goto WANIPFAILED;
    }
    netIRCPrintf("Socket created\n");

    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    (hp = gethostbyname("whatismyip.akamai.com")) ? server.sin_addr.s_addr = *(unsigned int *)hp->h_addr_list[0] : 0;
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) // connect to remote server
    {
        netIRCPrintf("Could not connect sock, aborting...\n");
        goto WANIPFAILED;
    }
    netIRCPrintf("Sock successfully connected\n");

    if (send(s, request, strlen(request), 0) < 0) // send http request
    {
        netIRCPrintf("Unable to request HTTP response, aborting...\n");
        goto WANIPCLOSED;
    }
    netIRCPrintf("Data Send\n");

    recv_size = recv(s, server_reply, sizeof(server_reply), 0); // receive a reply from the server
    if (recv_size < 0)
    {
        netIRCPrintf("Connection timeout, aborting...\n");
        goto WANIPCLOSED;
    }
    netIRCPrintf("Reply received\n");

    server_reply[recv_size] = '\0'; // set NULL character to make it a proper string before printing
    for (int i = recv_size-1; i > 1; i--)
    {
        if (server_reply[i-1] == '\n') // parse ip string from server response
        {
            if ((recv_size - i) >= (int)sizeof(gWanIp4)) // ip address too long, likely a connection error response message - abort
                break;
            Bstrcpy(gWanIp4, &server_reply[i]);
            netIRCPrintf("\nIP: %s\n", gWanIp4);
            bSuccess = 1;
            break;
        }
    }

WANIPCLOSED:
#ifdef _WIN32
    shutdown(s, SD_BOTH);
#else
    shutdown(s, SHUT_RDWR);
#endif

WANIPFAILED:
#ifdef _WIN32
    if (s != INVALID_SOCKET)
        closesocket(s);
    WSACleanup();
#else
    if (s >= 0)
        close(s);
#endif

    return bSuccess;
}

static ircd_t m_ircd;

void netIRCDeinitialize(void)
{
    if (gIRCState == BLOOD_IRC_DISCONNECTED)
        return;

#ifdef _WIN32
    shutdown(m_ircd.sock, SD_BOTH);
#else
    shutdown(m_ircd.sock, SHUT_RDWR);
#endif
    if (m_ircd.sock >= 0)
#ifdef _WIN32
        closesocket(m_ircd.sock);
    WSACleanup();
#else
        close(m_ircd.sock);
#endif

    gIRCState = BLOOD_IRC_DISCONNECTED;
}

int netIRCIinitialize(void)
{
    gIRCState = BLOOD_IRC_DISCONNECTED;

    if (!netGetWanIp4())
        return 0;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(1, 1), &wsa))
        return 0;
#endif

    gIRCState = BLOOD_IRC_CONNECTED;
    Bmemset(&m_ircd, 0, sizeof(ircd_t));
    Bsprintf(m_ircd.nick, gNetMode == NETWORK_SERVER ? "BlSrv%lu%X" : "BlCli%lu%X", (long unsigned int)(time(NULL)/60), (unsigned char)(time(NULL)&0xF));
    Bstrcpy(m_ircd.name, "NotBloodPlayer");
    Bstrcpy(m_ircd.chan, kIRCChan);
    m_ircd.sock = -1;

    if (!ircd_init(&m_ircd, kIRCServ, kIRCPort))
        netIRCDeinitialize();
    return gIRCState;
}

int netIRCProcess(void)
{
    if (gIRCState != BLOOD_IRC_DISCONNECTED)
        ircd_update(&m_ircd);
    return gIRCState;
}
#else
void netIRCDeinitialize(void)
{
    return;
}

int netIRCIinitialize(void)
{
    return 0;
}

int netIRCProcess(void)
{
    return 0;
}
#endif
