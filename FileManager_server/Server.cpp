//
// Created by unl on 21.05.17.
//

#include "Server.h"

std::vector<byte> &operator+=(std::vector<byte> &Vec, const std::string Str) {
    Vec.resize(Vec.size() + Str.length());
    if (Str.length() > 0)
        bcopy(&Str[0], &Vec[Vec.size() - Str.length()], Str.length());
    return Vec;
}

std::string VecToString(std::vector<byte> Vec) {
    std::string Res(Vec.size(), 'v');
    if (Vec.size() > 0)
        bcopy(&Vec[0], &Res[0], Vec.size());
    return Res;
}

Server::Server(uint32_t InPort) {
    Port = InPort;
}

void Server::Run() {
    int32_t Selfsockfd;
    sockaddr_in Cli_addr;
    Selfsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (Selfsockfd < 0)
        throw errors::OpeningSocket;
    bzero((char *) &Address, sizeof(Address));
    Address.sin_family = AF_INET;
    Address.sin_addr.s_addr = INADDR_ANY;
    Address.sin_port = htons(static_cast<uint16_t>(Port));
    if (bind(Selfsockfd, (sockaddr *) &Address, sizeof(Address)) < 0)
        throw errors::Binding;
    listen(Selfsockfd, 10);
    socklen_t Clilen = sizeof(Cli_addr);
    while (1)
    {
        std::cout << "\n waiting... \n";
        Sockfd = accept(Selfsockfd, (struct sockaddr *) &Cli_addr, &Clilen);
        std::vector<byte> Request = RecieveRawBytes();
        std::vector<byte> Respond;
        for (auto i : Request)
            std::cout << i;
        std::cout << std::endl;
        const byte reqLetter = Request[0];
        std::fstream DataBaseUsers("DataBaseUsers");
        DataBaseUsers.seekg(0, std::ios::end);
        int64_t DataBaseUsersSize = DataBaseUsers.tellg();
        std::string UsersInformation;
        if (DataBaseUsersSize <= 0)
        {
            std::ofstream FileStream("DataBaseUsers" ,std::ios::trunc);
            FileStream.close();
            UsersInformation = "";
        }
        else
        {
            DataBaseUsers.seekg(0, std::ios::beg);
            std::string Temp(static_cast<uint64_t>(DataBaseUsersSize), '\0');
            UsersInformation = Temp;
            DataBaseUsers.read(&UsersInformation.at(0), DataBaseUsersSize);
        }
        DataBaseUsers.close();
        switch (static_cast<Requests>(reqLetter))
        {
            case Requests::REGISTRATION :
                //Request example: R<username>&<psswrd>
                if (DoesUserExist(Request, UsersInformation))
                    Respond += std::to_string((uint8_t) errors::AlreadyRegistered);
                else
                {
                    try {
                        Registrate(Request, DataBaseUsers);
                        Respond += std::to_string((uint8_t) errors::NoErrors);
                    } catch (errors e){
                        Respond += std::to_string((uint8_t) e);
                    }
                }
                break;
            case Requests::LOGIN:
                //Request example: L<username>&<password>
                if (DoesAccExist(Request, UsersInformation))
                    Respond += std::to_string((uint8_t) errors::NoErrors);
                else
                    Respond += std::to_string((uint8_t) errors::BadLogin);
                //Responce is obv 1 byte
                break;
            case Requests::FILE_LISTING:
                //Request example: F<username>&<password>&<folder1/folder2/>
                Respond += SendFileListing(Request, UsersInformation);
                //Responce is obv 1 byte
                break;
            case Requests::UPLOAD:
                //Request example: U<username>&<password>&<folder1/folder2/file>&<bytes>
                WriteFile(Request , UsersInformation);
                Respond += std::to_string((uint8_t) errors::NoErrors);
                break;
            case Requests::DOWNLOAD:
                //Request example: D<username>&<password>&<folder1/folder2/file>
                Respond = GetFile(Request , UsersInformation);
                break;
        }
        SendRawBytes(Respond);
    }
}

std::vector<byte> Server::GetFile(std::vector<byte> Request, std::string &UsersInformation) const {
    std::string Path = Parse(Request , UsersInformation);
    std::ifstream FileStream(STORAGE_FOLDER_NAME + Path , std::ios::binary | std::ios::ate);
    std::ifstream::pos_type FileSize = FileStream.tellg();
    if (FileSize <= 0)
        throw errors::HackingAttempt;
    std::vector<byte> Res(static_cast<uint64_t>(FileSize));
    FileStream.seekg(0 , std::ios::beg);
    FileStream.read(reinterpret_cast<char*>(&Res.at(0)) , FileSize);
    FileStream.close();
    return Res;
}

void Server::WriteFile(std::vector<byte> Request, std::string &UsersInformation) const {
    std::string PathAndBytes =   Parse(Request , UsersInformation);
    std::string Path;
    while (PathAndBytes.front() != '&' && PathAndBytes.length() > 0)
    {
        Path += PathAndBytes.front();
        PathAndBytes = PathAndBytes.substr(1 , PathAndBytes.length());
    }
    std::ofstream FileStream(STORAGE_FOLDER_NAME + Path , std::ios::binary | std::ios::trunc);
    FileStream.write(reinterpret_cast<const char*>(&Request[PathAndBytes.length() + 1]) , Request.size() - PathAndBytes.length() - 1);
    FileStream.close();
}

std::string Server::Exec(const char *cmd) const {
    constexpr int16_t BUFFER_LENGTH = 0xFF;
    std::array<char, BUFFER_LENGTH> buffer;
    std::string Res;
    std::shared_ptr<FILE> CmdReader(popen(cmd, "r"), pclose);
    while (!feof(CmdReader.get()))
    {
        if (fgets(buffer.data(), BUFFER_LENGTH, CmdReader.get()) != NULL)
            Res += buffer.data();
    }
    return Res;
}

std::string Server::Parse(std::vector<byte> Request, std::string &UsersInformation) const {
    std::vector<byte> UserData;
    std::string RawData;
    std::vector<std::string>::size_type i = 0;
    while (Request[i] != '&')
    {
        UserData.push_back(Request[i]);
        i++;
    }
    UserData.push_back('&');
    i++;
    while (Request[i] != '&')
    {
        UserData.push_back(Request[i]);
        i++;
    }
    i++;
    while (i < Request.size())
    {
        RawData += Request[i];
        i++;
    }
    if (!DoesAccExist(UserData, UsersInformation))
        throw std::to_string((uint8_t) errors::BadLogin);
    return RawData;
}

std::string Server::SendFileListing(std::vector<byte> &Request, std::string &UsersInformation) const {
    std::string Path    = Parse(Request , UsersInformation);
    std::string Folders = Exec(std::string("cd " + STORAGE_FOLDER_NAME + Path + "; ls -d */").c_str());
    std::string Files   = Exec(std::string("cd " + STORAGE_FOLDER_NAME + Path + "; ls -p | grep -v /").c_str());
    if (Folders == "ls: cannot access '*/': No such file or directory")
        return Files;
    else
        return Folders + Files;
}

void Server::Registrate(const std::vector<byte> &Request, std::fstream &DataBaseUsers) const {
    if (Request.size() < 12)
        throw errors::HackingAttempt;
    DataBaseUsers.open("DataBaseUsers", std::ios::app);
    DataBaseUsers.write(reinterpret_cast<const char*>(&Request[1]), Request.size() - 1);
    DataBaseUsers << std::endl;
    DataBaseUsers.close();
}

bool Server::DoesAccExist(const std::vector<byte> &Request, const std::string &FileInfo) const {
    std::string Line;
    std::string::size_type i = 0;
    while (i < FileInfo.length())
    {
        switch (FileInfo[i])
        {
            case '\n':
                if (Line == VecToString(Request).substr(1, Request.size()))
                {
                    std::cout << "account does exist" << std::endl;
                    return true;
                }
                Line = "";
                break;
            default:
                Line += FileInfo[i];
                break;
        }
        i++;
    }
    std::cout << "account doesnt exist" << std::endl;
    return false;
}

bool Server::DoesUserExist(const std::vector<byte> &Request, const std::string &FileInfo) const {
    std::string Name;
    for (auto i = Request.begin() + 1;
         i != Request.end(); i++)
    {
        switch (*i)
        {
            case '>':
            case '~':
                break;
            case '&':
                i = Request.end() - 1;
                break;
            default:
                Name += *i;
                break;
        }
    }
    std::string CurrUsername;
    std::string::size_type i = 0;
    while (i < FileInfo.length())
    {
        switch (FileInfo[i])
        {
            case '&':
                if (CurrUsername == Name)
                {
                    std::cout << " user does exist" << std::endl;
                    return true;
                }
                while (FileInfo[++i] != '\n')
                    if (i < FileInfo.length())
                        break;
                break;
            case '\n':
                CurrUsername = "";
                break;
            default:
                CurrUsername += FileInfo[i];
                break;
        }
        i++;
    }
    std::cout << "user does not exist" << std::endl;
    return false;
}
