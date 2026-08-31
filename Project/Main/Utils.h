#pragma once

#define GLEW_STATIC
//#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httplib.h"


#ifdef WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <cstdlib>
#include <vector>
#include <time.h>
#include <chrono>
#include <functional>
#include <algorithm>
#include <filesystem>
#include <deque>
#include <mutex>
#include <regex>

#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>


#ifdef _WIN32

#pragma comment(lib, "opengl32.lib")

#endif

using namespace std;

namespace Utils {
    string StrToLower(string Str) {
        string NewStr = Str;
        std::transform(NewStr.begin(), NewStr.end(), NewStr.begin(),
            [](unsigned char c) { return std::tolower(c); });

        return NewStr;
    }

    string StrToUpper(string Str) {
        string NewStr = Str;
        std::transform(NewStr.begin(), NewStr.end(), NewStr.begin(),
            [](unsigned char c) { return std::toupper(c); });

        return NewStr;
    }

    const char* ReadFile(const char* FileName) {
        FILE* file = fopen(FileName, "rb");

        if (file == NULL) {
            cout << "File is NULL." << endl;

            return "";
        }

        fseek(file, 0, SEEK_END);

        size_t size = ftell(file); // ABSOLUTE CINNAMON
        rewind(file);

        char* text = (char*)malloc(size + 1);

        if (!text)
            return NULL;

        memset(text, 0, size + 1);

        fread(text, 1, size, file);

        fclose(file);

        return text;
    }

    void WriteFile(const char* FileName, const char* Text) {
        FILE* file = fopen(FileName, "wb");

        if (file == NULL) {
            cout << "Error opening file for writing." << endl;

            return;
        }

        fwrite(Text, 1, strlen(Text), file);

        fclose(file);
    }

    int RoundIncrement(int value, int increment) {
        return ((value + increment / 2) / increment) * increment;
    }

    

    auto GetMilliseconds() {
        auto duration = chrono::system_clock::now().time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        return millis;
    }

    auto ProcessStartMilli = GetMilliseconds();

    float GetSecondsSinceStart() {
        auto MillisecondsSinceStart = GetMilliseconds() - ProcessStartMilli;

        return (MillisecondsSinceStart) / 1000.0;
    }

    namespace FrameRate {
        template<std::intmax_t FPS>
        class frame_rater {
        public:
            frame_rater() :                 // initialize the object keeping the pace
                time_between_frames{ 1 },     // std::ratio<1, FPS> seconds
                tp{ std::chrono::steady_clock::now() }
            {
            }

            void sleep() {
                // add to time point
                tp += time_between_frames;

                // and sleep until that time point
                std::this_thread::sleep_until(tp);
            }

        private:
            // a duration with a length of 1/FPS seconds
            std::chrono::duration<double, std::ratio<1, FPS>> time_between_frames;

            // the time point we'll add to in every loop
            std::chrono::time_point<std::chrono::steady_clock, decltype(time_between_frames)> tp;
        };

        frame_rater<60> fr;

        int Count = 0;

        void Cap(bool enabled = true) {
            if (enabled) {
                fr.sleep();
            }

            Count++;
        }
    }

    namespace Files {
        // For brevity in the example
        namespace fs = std::filesystem;

        // Function to get all descendant paths
        std::vector<fs::path> get_descendants(const fs::path& directory_path) {
            std::vector<fs::path> paths;

            try {
                if (fs::exists(directory_path) && fs::is_directory(directory_path)) {
                    for (const auto& entry : fs::recursive_directory_iterator(directory_path)) {
                        paths.push_back(entry.path());
                    }
                }
                else {
                    std::cerr << "Path does not exist or is not a directory: " << directory_path << std::endl;
                }
            }
            catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << std::endl;
            }
            return paths;
        }

    }
}