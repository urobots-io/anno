#pragma once
#include <fstream>
#include <list>
#include <memory>
#include <vector>
#include <memory.h>

class BlobPacker {
public:
    int AddBlob(const char *data, int size, bool copy_data) {
        Blob blob; 
        blob.size = size;
        if (copy_data) {
            blob.copy.reset(new std::vector<char>(data, data + size));
        }
        else {
            blob.data = data;            
        }
        blobs_.push_back(blob);
        return int(blobs_.size()) - 1;
    }   

    int GetSize() const {
        int size = 0;
        for (auto & blob : blobs_) {
            size += blob.size + sizeof(blob.size);
        }
        return size;
    }

    void WriteTo(char *buffer) const {
        for (auto & blob : blobs_) {
            memcpy(buffer, &blob.size, sizeof(blob.size));
            buffer += sizeof(blob.size);

            if (blob.size) {
                auto data = blob.data ? blob.data : &blob.copy->at(0);
                memcpy(buffer, data, blob.size);
                buffer += blob.size;
            }
        }
    }

    void ReadFrom(const char *buffer, int buffer_size, bool copy_data) {
        auto end = buffer + buffer_size;
        while (buffer < end) {
            Blob blob;

            memcpy(&blob.size, buffer, sizeof(blob.size));
            buffer += sizeof(blob.size);

            if (blob.size) {
                if (copy_data) {
                    blob.copy.reset(new std::vector<char>(blob.size));
                    memcpy(&blob.copy->at(0), buffer, blob.size);
                }
                else {
                    blob.data = buffer;
                }
                buffer += blob.size;
            }

            blobs_.push_back(blob);
        }
    }

    const std::pair<const char*, int> GetBlob(int index) const {
        auto& blob = blobs_[index];
        return { blob.data ? blob.data : &blob.copy->at(0), blob.size };
    }

private:
    struct Blob {
        // external data
        const char* data = nullptr;
        int size = 0;
        // copied data
        std::shared_ptr<std::vector<char>> copy;
    };

    std::vector<Blob> blobs_;    
};
