#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>

bool space_check(char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }

uint32_t search_entry(std::istream& str, std::string search)
{
    str.clear();
    str.seekg(0);

    std::string line;
    while (std::getline(str, line, '\n'))
    {
        std::string neddle = "        " + search;
        if (line.substr(0, neddle.size()) == neddle)
            break;
    }

    line.erase(std::unique(line.begin(), line.end(), space_check), line.end());

    std::istringstream parser(line);
    std::string name, section, start;

    parser.get();
    std::getline(parser, name, ' ');
    std::getline(parser, section, ' ');
    std::getline(parser, start, ' ');

    if (name != search)
        return 0;

    return std::stoul(start, nullptr, 16);
}

std::vector<std::pair<std::string, uint32_t>> get_broken_entries(std::istream &nk_map)
{
    std::vector<std::pair<std::string, uint32_t>> out;

    std::string line;
    while (std::getline(nk_map, line, '\n'))
    {
        if (line.find("COREDLL.dll") == -1 || line.substr(0, 6) != " 0001:")
            continue;

        line.erase(std::unique(line.begin(), line.end(), space_check), line.end());

        std::string addr, name;

        std::istringstream parser(line.substr(6));
        std::getline(parser, addr, ' ');
        std::getline(parser, name, ' ');

        out.push_back(std::make_pair(name, std::stoul(addr, nullptr, 16)));
    }

    return out;
}

std::unordered_map<std::string, uint32_t> parse_map(std::istream& map)
{
    std::unordered_map<std::string, uint32_t> out;

    std::string line;
    while (std::getline(map, line, '\n'))
    {
        if (line.substr(0, 6) != " 0001:")
            continue;

        line.erase(std::unique(line.begin(), line.end(), space_check), line.end());

        std::string addr, name;

        std::istringstream parser(line.substr(6));
        std::getline(parser, addr, ' ');
        std::getline(parser, name, ' ');

        out.insert(std::make_pair(name, std::stoul(addr, nullptr, 16)));
    }

    return out;
}

void patch_broken_entries(std::string map_file, std::unordered_map<std::string, uint32_t> coredll_entries, uint32_t base, uint32_t coredll_base, uint32_t image_start, uint8_t *mem)
{
    if (base == 0 || coredll_base == 0)
        return;

    std::ifstream map_stream(map_file, std::ios_base::in | std::ios_base::binary);
    if (!map_stream.good())
        return;

    auto broken_entries = get_broken_entries(map_stream);

    for (const auto& entry : broken_entries) {
        auto it = coredll_entries.find(entry.first);
        if (it == coredll_entries.end())
            continue;

        printf("patching function: %s\n", entry.first.c_str());

        *((uint32_t*)&mem[base + entry.second - image_start + 4]) = 0xE1A00000;
        *((uint32_t*)&mem[base + entry.second - image_start + 12]) = coredll_base + it->second;
    }
}

int main()
{
    std::ifstream input("NK.bin", std::ios_base::in | std::ios_base::binary);

    uint8_t magic[] = { 0x42, 0x30, 0x30, 0x30, 0x46, 0x46, 0x0A };
    char buf[7];

    input.read(buf, 7);
    if (memcmp(magic, buf, 7) != 0)
        return -1;

    uint32_t image_start;
    uint32_t image_size;

    input.read((char*)&image_start, 4);
    input.read((char*)&image_size, 4);

    printf("image_start: %#x\n", image_start);
    printf("image_size: %u\n\n", image_size);

    uint8_t* mem = new uint8_t[image_size];

    uint32_t exec_addr = 0;

    while (1)
    {
        uint32_t record_start;
        uint32_t record_size;
        uint32_t record_checksum;

        input.read((char*)&record_start, 4);
        input.read((char*)&record_size, 4);
        input.read((char*)&record_checksum, 4);

        if (record_start == 0) {
            exec_addr = record_size;
            break;
        }

        if (mem + (record_start - image_start) + record_size > &mem[image_size])
            return -1;

        input.read((char*)mem + (record_start - image_start), record_size);
    }

    std::ifstream makeimg_diag("makeimg_diag.out", std::ios_base::in | std::ios_base::binary);
    if (makeimg_diag.good()) {
        uint32_t nk_base = search_entry(makeimg_diag, "nk.exe");
        uint32_t kitl_base = search_entry(makeimg_diag, "kitl.dll");
        uint32_t coredll_base = search_entry(makeimg_diag, "coredll.dll");

        printf("nk.exe: %#x\n", nk_base);
        printf("kitl.dll: %#x\n", kitl_base);
        printf("coredll.dll: %#x\n\n", coredll_base);

        std::ifstream coredll_map("coredll.map", std::ios_base::in | std::ios_base::binary);
        auto coredll_entries = parse_map(coredll_map);

        patch_broken_entries("nk.map", coredll_entries, nk_base, coredll_base, image_start, mem);
        patch_broken_entries("kitl.map", coredll_entries, kitl_base, coredll_base, image_start, mem);

        printf("\n");
    }

    std::ofstream output("NK.bin.raw", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    output.write((char*)mem, image_size);

    delete[] mem;

    printf("-device loader,addr=%#x,file=NK.bin.raw -device loader,addr=%#x,cpu-num=0\n", image_start, exec_addr);

    return 0;
}

