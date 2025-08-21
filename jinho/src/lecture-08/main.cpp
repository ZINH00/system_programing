#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

// Packet IDs from the protocol design document
#define REQ_RESUME 1001
#define CMD_RESUME_ACK 1002
#define REQ_INQUIRY 1003
#define CMD_INQUIRY 1004
#define REQ_ANSWER 1005
#define CMD_ANSWER_ACK 1006

// Server connection details from the practice material
const char* SERVER_IP = "<IP 주소>";
//const char* SERVER_IP = "127.0.0.1";
const WORD SERVER_PORT = 42345;

std::string from_utf8(const std::string& str) {
    if (str.empty()) return std::string();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

    size_needed = WideCharToMultiByte(CP_ACP, 0, &wstrTo[0], (int)wstrTo.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, &wstrTo[0], (int)wstrTo.size(), &strTo[0], size_needed, NULL, NULL);

    return strTo;
}

// Helper function to ensure all bytes are sent.
bool send_all(SOCKET sock, const char* buffer, size_t length) {
    size_t total_sent = 0;
    while (total_sent < length) {
        int bytes_sent = send(sock, buffer + total_sent, length - total_sent, 0);
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "send_all failed with error: " << WSAGetLastError() << std::endl;
            return false;
        }
        total_sent += bytes_sent;
    }
    return true;
}

// Helper function to ensure all bytes are received.
bool recv_all(SOCKET sock, char* buffer, size_t length) {
    size_t total_received = 0;
    while (total_received < length) {
        int bytes_received = recv(sock, buffer + total_received, length - total_received, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Connection closed by server." << std::endl;
            }
            else {
                std::cerr << "recv_all failed with error: " << WSAGetLastError() << std::endl;
            }
            return false;
        }
        total_received += bytes_received;
    }
    return true;
}

// Function to convert a local codepage string to UTF-8
std::string to_utf8(const std::string& str) {
    if (str.empty()) return std::string();

    int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

    size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstrTo[0], (int)wstrTo.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstrTo[0], (int)wstrTo.size(), &strTo[0], size_needed, NULL, NULL);

    return strTo;
}

// Function to send a packet with a given ID and context.
bool send_packet(SOCKET sock, unsigned int packet_id, const std::string& context) {
    unsigned int context_length = static_cast<unsigned int>(context.size());

    std::string header_data;
    header_data.resize(8);
    memcpy(&header_data[0], &packet_id, sizeof(unsigned int));
    memcpy(&header_data[4], &context_length, sizeof(unsigned int));

    std::string full_packet = header_data + context;

    return send_all(sock, full_packet.c_str(), full_packet.length());
}

// Function to receive a packet and return its ID and context.
bool recv_packet(SOCKET sock, unsigned int& packet_id, std::string& context) {
    char header_buffer[8];
    if (!recv_all(sock, header_buffer, 8)) {
        return false;
    }

    unsigned int context_length = 0;
    memcpy(&packet_id, &header_buffer[0], sizeof(unsigned int));
    memcpy(&context_length, &header_buffer[4], sizeof(unsigned int));

    if (context_length > 0) {
        context.resize(context_length);
        if (!recv_all(sock, &context[0], context_length)) {
            return false;
        }
    }
    else {
        context.clear();
    }

    return true;
}

// A very basic JSON-like string parser for the CMD_INQUIRY response
size_t count_questions(const std::string& json_str) {
    size_t count = 0;
    size_t pos = json_str.find("\"Questions\":[");
    if (pos == std::string::npos) return 0;

    pos = json_str.find("{", pos);
    while (pos != std::string::npos) {
        count++;
        pos = json_str.find("{", pos + 1);
    }
    return count;
}


int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
        return 1;
    }

    SOCKET client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock == INVALID_SOCKET) {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    std::cout << "Connecting to server at " << SERVER_IP << ":" << SERVER_PORT << "..." << std::endl;
    iResult = connect(client_sock, (sockaddr*)&server_addr, sizeof(server_addr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl;
        closesocket(client_sock);
        WSACleanup();
        return 1;
    }
    std::cout << "Connected to server." << std::endl;

    // --- Phase 1: Send REQ_RESUME and receive CMD_RESUME_ACK ---
    std::cout << "\n[1] Sending REQ_RESUME packet (ID: " << REQ_RESUME << ")\n";
    std::string name_utf8 = to_utf8("정진호");
    std::string greeting_utf8 = to_utf8("안녕하세요.");
    std::string introduction_utf8 = to_utf8("BoB 14기");
    std::string resume_data = "{\"Resume\":{\"Name\":\"" + name_utf8 + "\",\"Greeting\":\"" + greeting_utf8 + "\",\"Introduction\":\"" + introduction_utf8 + "\"}}";

    if (send_packet(client_sock, REQ_RESUME, resume_data)) {
        std::cout << "패킷이 성공적으로 전송되었습니다." << std::endl;
    }
    else {
        std::cerr << "패킷 전송에 실패했습니다." << std::endl;
        closesocket(client_sock);
        WSACleanup();
        return 1;
    }

    unsigned int received_packet_id;
    std::string received_context;

    std::cout << "\n서버 응답을 기다립니다...\n";
    if (recv_packet(client_sock, received_packet_id, received_context)) {
        std::cout << "서버로부터 패킷을 수신했습니다.\n";
        std::cout << "패킷 ID: " << received_packet_id << std::endl;
        std::cout << "컨텍스트: " << received_context << std::endl;

        if (received_packet_id != CMD_RESUME_ACK) {
            std::cerr << "예상치 못한 응답 패킷 ID: " << received_packet_id << std::endl;
            closesocket(client_sock);
            WSACleanup();
            return 1;
        }
    }
    else {
        std::cerr << "패킷 수신에 실패했습니다." << std::endl;
        closesocket(client_sock);
        WSACleanup();
        return 1;
    }

    // --- Phase 2: Send REQ_INQUIRY and receive CMD_INQUIRY ---
    std::cout << "\n[2] Sending REQ_INQUIRY packet (ID: " << REQ_INQUIRY << ")\n";
    std::string inquiry_data = "{\"QuestionCode\":\"BOB13th\"}";

    if (send_packet(client_sock, REQ_INQUIRY, inquiry_data)) {
        std::cout << "REQ_INQUIRY 패킷이 성공적으로 전송되었습니다." << std::endl;
    }
    else {
        std::cerr << "REQ_INQUIRY 패킷 전송에 실패했습니다." << std::endl;
        closesocket(client_sock);
        WSACleanup();
        return 1;
    }

    size_t question_count = 0;
    std::vector<std::vector<std::string>> all_question_choices;

    std::cout << "\n서버로부터 CMD_INQUIRY 패킷을 기다립니다...\n";
    if (recv_packet(client_sock, received_packet_id, received_context)) {
        std::cout << "서버로부터 패킷을 수신했습니다.\n";
        std::cout << "패킷 ID: " << received_packet_id << std::endl;
        std::cout << "컨텍스트: " << received_context << std::endl;

        if (received_packet_id == CMD_INQUIRY) {
            question_count = count_questions(received_context);
            std::cout << "질문 목록을 성공적으로 받았습니다. 총 " << question_count << "개의 질문이 있습니다." << std::endl;

            std::string temp_context = received_context;
            size_t search_pos = 0;
            for (size_t i = 0; i < question_count; ++i) {
                search_pos = temp_context.find("\"Selection\":[", search_pos);
                if (search_pos == std::string::npos) break;

                size_t choices_end_pos = temp_context.find("]", search_pos);
                if (choices_end_pos == std::string::npos) break;

                std::string choices_str = temp_context.substr(search_pos + 13, choices_end_pos - (search_pos + 13));

                std::vector<std::string> current_choices;
                size_t current_pos = 0;
                while (current_pos < choices_str.length()) {
                    size_t quote_start = choices_str.find('"', current_pos);
                    if (quote_start == std::string::npos) break;

                    std::string choice;
                    bool escaped = false;
                    size_t j = quote_start + 1;
                    for (; j < choices_str.length(); ++j) {
                        if (escaped) {
                            choice += choices_str[j];
                            escaped = false;
                        }
                        else {
                            if (choices_str[j] == '\\') {
                                escaped = true;
                            }
                            else if (choices_str[j] == '"') {
                                break; // End of choice string
                            }
                            else {
                                choice += choices_str[j];
                            }
                        }
                    }
                    current_choices.push_back(choice);
                    current_pos = j + 1;
                }
                all_question_choices.push_back(current_choices);

                search_pos = choices_end_pos;
            }
        }
        else {
            std::cerr << "예상치 못한 응답 패킷 ID: " << received_packet_id << std::endl;
            closesocket(client_sock);
            WSACleanup();
            return 1;
        }
    }
    else {
        std::cerr << "패킷 수신에 실패했습니다." << std::endl;
        closesocket(client_sock);
        WSACleanup();
        return 1;
    }

    std::cout << "\n[3] 질문에 답변을 전송합니다...\n";
    for (size_t i = 0; i < question_count; i++) {
        int answer_num;
        std::cout << "\n질문 " << i + 1 << "에 대한 답을 입력하세요 (예: 1, 2, 3, 4): ";
        std::cin >> answer_num;

        if (i >= all_question_choices.size() || answer_num <= 0 || (size_t)answer_num > all_question_choices[i].size()) {
            std::cerr << "잘못된 선택이거나 파싱 오류입니다. 1부터 " << all_question_choices[i].size() << " 사이의 숫자를 입력하세요." << std::endl;
            i--;
            continue;
        }

        std::string question_id = std::to_string(i + 1);
        std::string actual_answer_text = all_question_choices[i][(size_t)answer_num - 1];

        std::string answer_data_escaped;
        for (char c : actual_answer_text) {
            if (c == '\\' || c == '"') {
                answer_data_escaped += '\\';
            }
            answer_data_escaped += c;
        }

        std::string answer_data = "{\"QuestionIdx\":" + question_id + ",\"Answer\":\"" + answer_data_escaped + "\"}";

        std::cout << "REQ_ANSWER 패킷 전송 (ID: " << REQ_ANSWER << ")\n";
        std::cout << "컨텍스트: " << answer_data << std::endl;

        if (!send_packet(client_sock, REQ_ANSWER, answer_data)) {
            std::cerr << "REQ_ANSWER 패킷 전송 실패 (질문 " << i + 1 << ")" << std::endl;
            closesocket(client_sock);
            WSACleanup();
            return 1;
        }

        unsigned int received_packet_id_ack;
        std::string received_context_ack;
        std::cout << "CMD_ANSWER_ACK 패킷 수신 대기...\n";

        if (recv_packet(client_sock, received_packet_id_ack, received_context_ack)) {
            std::cout << "서버로부터 패킷 수신: ID " << received_packet_id_ack << ", 컨텍스트 " << received_context_ack << std::endl;

            if (received_packet_id_ack != CMD_ANSWER_ACK) {
                std::cerr << "예상치 못한 응답 패킷 ID: " << received_packet_id_ack << std::endl;
                closesocket(client_sock);
                WSACleanup();
                return 1;
            }
        }
        else {
            std::cerr << "CMD_ANSWER_ACK 패킷 수신 실패 (질문 " << i + 1 << ")" << std::endl;
            closesocket(client_sock);
            WSACleanup();
            return 1;
        }
    }

    closesocket(client_sock);
    WSACleanup();
    std::cout << "\n모든 질문에 대한 답변 전송 및 확인 완료. 연결 종료.\n";

    return 0;
}