//
// Created by unl on 21.05.17.
//
#include <fstream>

#include "sender.h"

Client::Client(uint32_t InPort, std::string InHostname) {
    Port = InPort;
    Hostname = InHostname;
}

void Client::UploadFile(std::string Path, std::string Filename) {
    Run();
    std::ifstream FileStream(Filename , std::ios::binary | std::ios::ate);
    std::ifstream::pos_type Size = FileStream.tellg();
    FileStream.seekg(0 , std::ios::beg);
    if (Size <= 0)
        throw errors::BadFile;
    std::vector<byte> bytes(static_cast<uint64_t >(Size)),
            Request;
    FileStream.read(reinterpret_cast<char*>(&bytes.at(0)) ,Size);
    Request += static_cast<char>(requests::UPLOAD) + Username + '&' + Password + '&' + Path + Filename + '&' + VecToString(bytes);
    SendRawBytes(Request);
    std::vector<byte> Resp = RecieveRawBytes();
    if (VecToNum(Resp) != static_cast<int>(errors::NoErrors))
        throw static_cast<errors>(Resp[0]);
    FileStream.close();
}

std::string VecToString(std::vector<byte> Vec) {
    std::string Res(Vec.size(),'\0');
    if (Vec.size() > 0)
        bcopy(  &Vec[0], &Res[0], Vec.size() );
    return Res;
}

int64_t VecToNum(std::vector<byte> Vec) {
    std::string Temp = VecToString(Vec);
    return atoi( Temp.c_str());//should test std::to_int64 !
}

std::vector<byte> &operator+=(std::vector<byte> &Vec, const std::string Str) {
    Vec.resize( Vec.size() + Str.length() );
    if (Str.length() > 0)
        bcopy( &Str[0], &Vec[ Vec.size() - Str.length() ] , Str.length() );
    return Vec;
}

void Client::Run() {
    hostent *Server;
    Sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (Sockfd < 0)
        throw errors::OpeningSocket;
    Server = gethostbyname(Hostname.c_str());
    if (Server == NULL)
        throw Client::errors::FindingHost;
    bzero((char *) &Address, sizeof(Address));
    Address.sin_family = AF_INET;
    bcopy(Server->h_addr, (char *) & Address.sin_addr.s_addr, Server->h_length);
    Address.sin_port = htons(Port);
    if ( connect( Sockfd,(struct sockaddr *) &Address, sizeof(Address) ) < 0)
        throw Client::errors::Connecting;
}

void Client::Registrate(std::string Name, std::string Pass) {
    Run();
    std::vector<byte> ToSend;
    ToSend += (char)Client::requests::REGISTRATION + Name + '&' + Pass;
    Username = Name; Password = Pass;
    SendRawBytes(ToSend);
    std::vector<byte> Respond = RecieveRawBytes();
    close(Sockfd);
    if (VecToNum(Respond) != (uint8_t) Client::errors::NoErrors)
        throw (Client::errors)VecToNum(Respond);
    return;
}

void Client::Login(std::string name, std::string pass) {
    Run();
    std::vector<byte> toSend;
    toSend += static_cast<char>(Client::requests::LOGIN) + name + '&' + pass;
    SendRawBytes(toSend);
    std::vector<byte> respond = RecieveRawBytes();
    close(Sockfd);
    if ( VecToNum(respond) != 0 )
        throw (Client::errors)VecToNum(respond);
    Username = name;
    Password = pass;
    return;
}

std::vector<std::string> Client::GetFileListing(std::string Path) {
    Run();
    std::vector<byte> Request;
    Request += static_cast<char>(Client::requests::FILE_LISTING) + Username + '&' + Password + '&' + Path;
    SendRawBytes(Request);
    std::vector<byte> Respond = RecieveRawBytes();
    close(Sockfd);
    std::vector<std::string> Files;
    if (Path != "")
        Files.push_back("..");
    std::string Temp = "";
    for (auto c : Respond){
        if (c == '\n') {
            Files.push_back(Temp);
            Temp = "";
        }
        else
            Temp += c;
    }
    return Files;
}

void Client::DownloadFile(std::string path, std::string &Filename) {
    Run();
    std::vector<byte> Request;
    Request += static_cast<char>(Client::requests::DOWNLOAD) + Username + '&' + Password + '&' + path;
    SendRawBytes(Request);
    std::vector<byte> Respond = RecieveRawBytes();
    close(Sockfd);
    std::ofstream fileStream(Filename , std::ios::binary | std::ios::trunc);
    fileStream.write(reinterpret_cast<char*>(&Respond[0]) , Respond.size());
    fileStream.close();
}
