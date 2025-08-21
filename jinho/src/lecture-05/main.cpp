#include "pch.h"

namespace fs = std::filesystem;

static std::string wildcardToRegex(const std::string& pat)
{
    std::string rx;
    rx.reserve(pat.size() * 2);
    rx += '^';
    for (char c : pat) {
        switch (c) {
        case '*': rx += ".*"; break;
        case '?': rx += '.';  break;
        case '.': rx += "\\."; break;
        case '\\': rx += "\\\\"; break;
        default:  rx += c;   break;
        }
    }
    rx += '$';
    return rx;
}

static void grepFiles(const fs::path& root,
    const std::string& pattern,
    std::vector<fs::path>& out)
{
    const std::regex rx(wildcardToRegex(pattern),
        std::regex_constants::icase);
    for (auto&& entry : fs::recursive_directory_iterator(root,
        fs::directory_options::skip_permission_denied)) {
        if (!entry.is_regular_file()) continue;
        const std::string name = entry.path().filename().string();
        if (std::regex_match(name, rx))
            out.push_back(entry.path());
    }
}

static void printHeader(const fs::path& p)
{
    std::ifstream f(p, std::ios::binary);
    if (!f) return;

    std::array<unsigned char, 4> buf{};
    if (!f.read(reinterpret_cast<char*>(buf.data()), buf.size())) return;

    std::cout << p.string() << ": ";

    for (size_t i = 0; i < buf.size(); ++i) {
        if (i) std::cout << ' ';
        std::cout << std::uppercase
            << std::setw(2) << std::setfill('0')
            << std::hex << static_cast<int>(buf[i]);
    }
    std::cout << std::dec << '\n';
}


int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: headerDump <dir> <pattern>\n";
        return 1;
    }

    const fs::path root = argv[1];
    const std::string pattern = argv[2];

    std::vector<fs::path> files;
    grepFiles(root, pattern, files);

    for (const auto& p : files)
        printHeader(p);
}
