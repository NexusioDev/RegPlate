#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include <codecvt>
#include <algorithm>
#include <io.h>
#include <fcntl.h>

using json = nlohmann::json; 
using namespace std; 

enum Language { Deutsch, English };

// Hilfsfunktion: UTF-16 zu UTF-8 (für JSON-Lookup)
string wstring_to_utf8(const wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}

// Hilfsfunktion: UTF-8 zu UTF-16 (für die Ausgabe via wcout)
wstring utf8_to_wstring(const string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    wstring result(size - 1, '\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

wstring to_uppercase(const wstring& str) {
    wstring result;
    for (wchar_t c : str) {
        switch (c) {
            case 0x00E4: result += (wchar_t)0x00C4; break; // ä -> Ä
            case 0x00F6: result += (wchar_t)0x00D6; break; // ö -> Ö
            case 0x00FC: result += (wchar_t)0x00DC; break; // ü -> Ü
            case 0x00DF: result += L"SS"; break;          // ß -> SS (Besonderheit im Deutschen)
            default: 
                result += towupper(c); 
                break;
        }
    }
    return result;
}

string getBundesland(int index, Language lang) {
    if (index == 1 && lang == Language::Deutsch) return "Nordrhein-Westfalen";
    else if (index == 1 && lang == Language::English) return "North Rhine-Westphalia";
    if (index == 2 && lang == Language::Deutsch) return "Bayern";
    else if (index == 2 && lang == Language::English) return "Bavaria";
    if (index == 3 && lang == Language::Deutsch) return "Berlin";
    else if (index == 3 && lang == Language::English) return "Berlin";
    if (index == 4 && lang == Language::Deutsch) return "Sachsen";
    else if (index == 4 && lang == Language::English) return "Saxony";

    return "Deutschland";
}

int main() {
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    ifstream file("kfz.json");
    if (!file.is_open()) {
        wcerr << L"Fehler: kfz.json nicht gefunden!" << endl;
        return 1;
    }
    json data;
    file >> data;
    file.close();

    wstring input;
    Language lang = Language::English;

    wcout << L"Enter license plate abbreviation (e.g., B, M, HH):\nType '#' to switch language." << endl;

    while (true) {
        wcout << L"> ";
        if (!getline(wcin, input)) break;

        // Trimmen
        input.erase(0, input.find_first_not_of(L" \t\n\r"));
        input.erase(input.find_last_not_of(L" \t\n\r") + 1);

        if (input.empty()) continue;
        if (input == L"#") {
            lang = (lang == Language::Deutsch) ? Language::English : Language::Deutsch;
            wcout << (lang == Language::Deutsch ? L"Sprache: Deutsch" : L"Language: English") << endl;
            continue;
        }

        // Nur das erste Wort nehmen
        size_t space_pos = input.find(L' ');
        if (space_pos != wstring::npos) input = input.substr(0, space_pos);

        wstring input_up = to_uppercase(input);
        string lookup = wstring_to_utf8(input_up);

        bool found = false;
        if (data.contains(lookup)) {
            auto& entry = data[lookup];
            if (entry.is_object()) {
                string city_utf8 = entry["name"];
                wcout << input_up << L": " << utf8_to_wstring(city_utf8) << endl;
                if (entry.contains("funfact")) {
                    string funfact_utf8 = entry["funfact"];
                    wcout << utf8_to_wstring(funfact_utf8) << endl;
                }
                if (entry.contains("bundesland")) {
                    int bundesland_utf8 = entry["bundesland"];
                    wcout << L"Bundesland: " << utf8_to_wstring(getBundesland(entry["bundesland"], lang)) << endl;
                }
            } else {
                string city_utf8 = data[lookup];
                wcout << input_up << L": " << utf8_to_wstring(city_utf8) << endl;
            }
            found = true;
        } else {
            // Fallback für Ä -> A, Ö -> O, Ü -> U
            string fallback = lookup;
            // UTF-8 Ersatz für Ä, Ö, Ü
            auto replace_utf8 = [&](string search, string replace) {
                size_t pos = 0;
                while ((pos = fallback.find(search, pos)) != string::npos) {
                    fallback.replace(pos, search.length(), replace);
                    pos += replace.length();
                }
            };
            replace_utf8("\xC3\x84", "A"); // Ä
            replace_utf8("\xC3\x96", "O"); // Ö
            replace_utf8("\xC3\x9C", "U"); // Ü

            if (fallback != lookup && data.contains(fallback)) {
                string city_utf8 = data[fallback];
                wcout << input_up << L" (" << utf8_to_wstring(fallback) << L"): " << utf8_to_wstring(city_utf8) << endl;
                found = true;
            }
        }

        if (!found) {
            if (lang == Language::Deutsch)
                wcout << L"Kennzeichen " << input_up << L" wurde nicht gefunden." << endl;
            else
                wcout << L"License plate " << input_up << L" not found." << endl;
        }
    }
    return 0;
}