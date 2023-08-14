#include "linear.h"

void PreloadSearchLinear(std::string path, const std::string& search,
    std::vector<std::string>& results,
    std::vector<std::string>& folders) {
    if (path.back() == '\\') path.pop_back();

    HPathReadDirectory(path, [&](const WIN32_FIND_DATAA& findData) {
        const std::string fileName = findData.cFileName;
        std::string subDirectory = path + "\\" + fileName;
        bool valid_dir = IsValidDirectory(findData);

        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || valid_dir) {
            if (fileName.find(search) != std::string::npos) {
                results.emplace_back(subDirectory);
            }
            if (valid_dir) {
                folders.emplace_back(subDirectory);
            }
        }

        return true;
        });
}

void SearchLinear(std::string path, const std::string& search,
    std::vector<std::string>& results) {
    if (path.back() == '\\') path.pop_back();

    HPathReadDirectory(path, [&](const WIN32_FIND_DATAA& findData) {
        const std::string fileName = findData.cFileName;
        std::string subDirectory = path + "\\" + fileName;
        bool valid_dir = IsValidDirectory(findData);

        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || valid_dir) {
            if (fileName.find(search) != std::string::npos) {
                m_mutex.lock();
                results.emplace_back(subDirectory);
                m_mutex.unlock();
            }

            if (valid_dir) {
                SearchLinear(subDirectory, search, results);
            }
        }

        return true;
        });
}

void SearchParallelLinear(std::string target, const std::string& search,
    std::vector<std::string>& results) {
    unsigned int numThreads = std::thread::hardware_concurrency();

    std::vector<std::string> folders;
    PreloadSearchLinear(target, search, results, folders);

    std::vector<std::thread> threads;

    if (numThreads >= folders.size()) {
        for (size_t i = 0; i < folders.size(); i++) {
            threads.emplace_back(&SearchLinear, folders[i], search,
                std::ref(results));
        }
    }
    else {
        std::vector<std::vector<std::string>> foldersPerThread(numThreads);

        for (size_t i = 0; i < folders.size(); i++) {
            foldersPerThread[i % numThreads].emplace_back(folders[i]);
        }

        for (std::vector<std::string>& threadFolders : foldersPerThread) {
            threads.emplace_back([search, &results, threadFolders]() {
                for (std::string folder : threadFolders) {
                    SearchLinear(folder, search, results);
                }
                });
        }
    }

    for (std::thread& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}