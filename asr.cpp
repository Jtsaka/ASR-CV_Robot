#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <array>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <cmath>

using namespace std;

struct Command {
    float directionDeg;   // 0–359 degrees
    float distanceMeters; // always in meters
};

// Execute a shell command and capture its output
string execCommand(const string &cmd) {
    array<char, 256> buffer{};
    string result;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        result += buffer.data();
    pclose(pipe);
    return result;
}

// Unit conversion
float toMeters(float value, const string &unit) {
    string u = unit;
    for (auto &c : u) c = tolower(c);

    if (u == "m" || u == "meter" || u == "meters") return value;
    if (u == "ft" || u == "foot" || u == "feet")   return value * 0.3048f;
    if (u == "in" || u == "inch" || u == "inches") return value * 0.0254f;
    if (u == "cm" || u == "centimeter" || u == "centimeters") return value * 0.01f;
    return value; 
}

// Parse transcript
Command parseTranscript(const string &text) {
    Command cmd{0.0f, 0.0f};

    // Direction — numeric degrees OR cardinal
    regex degreeRegex("(\\d+(?:\\.\\d+)?)\\s*(?:degrees?|deg|heading)?", regex::icase);
    regex dirRegex("\\b(forward|backward|left|right)\\b", regex::icase);

    smatch degMatch, dirMatch;

    if (regex_search(text, degMatch, degreeRegex)) {
        cmd.directionDeg = stof(degMatch.str(1));
    } else if (regex_search(text, dirMatch, dirRegex)) {
        string dir = dirMatch.str(1);
        for (auto &c : dir) c = tolower(c);
        if (dir == "forward")   cmd.directionDeg = 0.0f;
        else if (dir == "right")    cmd.directionDeg = 90.0f;
        else if (dir == "backward") cmd.directionDeg = 180.0f;
        else if (dir == "left")     cmd.directionDeg = 270.0f;
    }

    // Normalize bearing to 0–359
    if (cmd.directionDeg >= 360.0f || cmd.directionDeg < 0.0f)
        cmd.directionDeg = fmod(fmod(cmd.directionDeg, 360.0f) + 360.0f, 360.0f);

    // Distance — numeric + unit
    regex distRegex("(\\d+(?:\\.\\d+)?)\\s*(feet|foot|ft|meters?|m|inches?|inch|cm|centimeters?)",
                    regex::icase);
    smatch distMatch;
    if (regex_search(text, distMatch, distRegex)) {
        float val = stof(distMatch.str(1));
        string unit = distMatch.str(2);
        cmd.distanceMeters = toMeters(val, unit);
    }

    return cmd;
}

int main() {
    namespace fs = std::filesystem;
    fs::create_directory("recordings");

    const string audioFile = "recordings/command.wav";
    const string transcriptFile = audioFile + ".txt";

    const string whisperPath =
        "/home/amartinezhall/whisper-project/whisper.cpp/build/bin/whisper-cli";
    const string modelPath =
        "/home/amartinezhall/whisper-project/whisper.cpp/models/ggml-base.en.bin";

    // Get card # for sndrpigooglevoi (adafruit mic) dynamically
    string deviceName = execCommand(
        "arecord -l | grep -o 'card [0-9]*: sndrpigooglevoi' | head -1 | awk '{print $2}' | tr -d ':'"
    );
    deviceName.erase(remove(deviceName.begin(), deviceName.end(), '\n'), deviceName.end());
    if (deviceName.empty()) deviceName = "0"; // fallback

    // Record audio (15s)
    string recordCmd = "arecord -D plughw:" + deviceName + ",0 -f S16_LE -r 16000 -c 1 -d 15 " + audioFile;
    if (system(recordCmd.c_str()) != 0) return 1;

    // Transcribe audio
    string whisperCmd = whisperPath + " -m " + modelPath + " -f " + audioFile + " -otxt";
    if (system(whisperCmd.c_str()) != 0) {
        fs::remove(audioFile);
        return 1;
    }

    // Read transcript
    ifstream file(transcriptFile);
    if (!file.is_open()) {
        fs::remove(audioFile);
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string transcript = buffer.str();
    file.close();

    // Parse text
    Command cmd = parseTranscript(transcript);

    // Output "<direction_deg>,<distance_m>"
    cout << cmd.directionDeg << "," << cmd.distanceMeters << endl;

    // Cleanup recording files
    fs::remove(audioFile);
    fs::remove(transcriptFile);

    return 0;
}
