//
// Created by symbx on 05.08.17.
//

#include <utils/Utils.h>
#include <cstring>
#include <SFML/Network/Http.hpp>
#include <SFML/System/FileInputStream.hpp>
#include "core/AssetsManager.h"
#include "utils/Logger.h"
#include "Constants.h"

AssetsManager::AssetsManager(const std::string &appDir) :
    _dir(appDir),
    _packages(),
    _entries() {
    L_INFO("Initialize AssetsManager");
#ifdef _DEBUG
    _dir.append("/res/");
#else
    _dir.append("/res_");
#endif
    _state = EMPTY;
}

AssetsManager::~AssetsManager() {
    clean();
}

void AssetsManager::scan() {
    L_INFO("Scan " + _dir + " for packages");
    _state = SCAN;
    for (int i = 0; i < PACK_MAX; ++i) {
        std::string file(_dir + std::to_string(i) + ".package");
        if(!Utils::isFileExists(file))
            break;
        L_INFO("Found "+file);
        _packages.insert(_packages.begin(), file);
    }
    _state = _packages.empty()?NO_PACKAGES:IDLE;
}

void AssetsManager::parse() {
    if(_packages.empty())
        return;
    L_INFO("Parsing packages");
    clean();
    _progress = 0;
    float perFile = 100.0f / _packages.size();
    while(!_packages.empty()) {
        read(_packages.back());
        _packages.pop_back();
        _progress += perFile;
    }
}

void AssetsManager::clean() {
    for(auto i = _entries.begin(); i != _entries.end(); ++i) {
        _entries.erase(i->first);
    }
}

void AssetsManager::read(const std::string &file) {
    L_INFO("Read "+file);
    std::ifstream stream(file, std::ifstream::binary);
    char head[4];
    stream.read(&head[0], 4);
    if(head[0]!='G' || head[1]!='R' || head[2]!='P' || head[3]!='F') {
        L_WARN("Incorrect header, skip file");
        return;
    }

    sf::Uint32 count;
    stream.read((char*)&count, sizeof(sf::Uint32));

    unsigned char nameLen;
    char* buffer = new char[256];
    std::string name;
    for (int i = 0; i < count; ++i) {
        struct Entry entry;
        stream.read((char*)&nameLen, 1);
        memset(buffer, 0, 256);
        stream.read(buffer, nameLen);
        name.assign(buffer, nameLen);
        entry.package = file;
        stream.read((char*)&entry.offset, sizeof(sf::Uint64));
        stream.read((char*)&entry.size, sizeof(sf::Uint64));
        _entries[name] = entry;
    }
    L_INFO("Loaded " + std::to_string(count) + " entries");
    delete[] buffer;
}

void AssetsManager::run() {
#ifdef _DEBUG
    scan();
    if(_state == NO_PACKAGES)
        download();
    else {
        parse();
        std::string version(GAME_VERS);
        if(hasEntry("/version")) {
            sf::InputStream* stream = getStream("/version");
            char buffer[8];
            sf::Int64 len = stream->read(&buffer[0], 8);
            version.assign(buffer, static_cast<unsigned long>(len));
        }
        checkUpdate(version);
    }
#endif
}

AssetsManager::State AssetsManager::getState() {
    return _state;
}

float AssetsManager::getProgress() {
    if(_progress > 100.0f)
        return 100.0f;
    return _progress;
}

void AssetsManager::download() {

}

void AssetsManager::checkUpdate(const std::string& version) {
    sf::Http http(GAME_SITE_HOST, GAME_SITE_PORT);
    sf::Http::Request req(GAME_SITE_UPD);
    req.setField("Game-Version", version);
    sf::Http::Response resp = http.sendRequest(req, sf::seconds(30.0f));
    if(resp.getStatus() != sf::Http::Response::Ok) {
        _state = NO_INET;
        return;
    }
}

bool AssetsManager::hasEntry(const std::string &name) {
    return _entries.count(name) > 0;
}

sf::InputStream *AssetsManager::getStream(const std::string &name) {
#ifdef _DEBUG
    std::string file(_dir + name);
    if(!Utils::isFileExists(file))
        return NULL;
    sf::FileInputStream* stream = new sf::FileInputStream();
    stream->open(file);
    return stream;
#else
    struct Entry entry = _entries[name];
    return new PackageStream(entry.package, entry.offset, entry.size);
#endif
}

