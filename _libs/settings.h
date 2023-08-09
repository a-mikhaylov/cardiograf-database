#pragma once
#include <fstream>
#include <iostream>
#include <chrono>

namespace settings {
    const std::string BIG_FILE   = "/../_data/big_8x60e6";  // большой файл записи
    const std::string SMALL_FILE = "/../_data/small_8x1e6"; // маленький файл записи

    const std::string BIG_PATH = "../_data/big_8x60e6";
    const std::string SMALL_PATH = "../_data/small_8x1e6";

    const std::string MARKUP_DIR = "../_data/_markup/"; // директория для складывания результатов разметки
    const std::string MARKUP_DB_DIR = "../$MarkupDB/";  // директория для складывания баз данных разметки
    // const std::string MARKUP_CSV_PATH = "../_data/_markup/qrs.csv";
    // const std::string MARKUP_BIN_PATH = "../_data/_markup/qrs.bin";
    // const std::string MARKUP_TOML_PATH = "../_data/_markup/qrs.toml";

    void PrintVector(std::string label, std::vector<int> x) {
        std::cerr << label << std::endl << "\t{ ";
        for (int i = 0; i < x.size(); ++i) std::cerr << x[i] << ",\t";
        std::cerr << "\b\b }" << std::endl;
    }

    void PrintVector(std::string label, std::vector<std::vector<int>> x) {
        std::cerr << label << std::endl;
        for (int i = 0; i < x.size(); ++i) {
            std::cerr << "\t{  ";
            for (int j = 0; j < x[i].size(); ++j)
                std::cerr << x[i][j] << ",\t";
            std::cerr << "\b\b }" << std::endl;
        }
    }
    // обновление переменной var разницей (в мс) между stop и start
    void UpdateTime(float& var, std::chrono::high_resolution_clock::time_point& start, std::chrono::high_resolution_clock::time_point& stop) {
        var = var + ((float)std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() / 1000000.0f);
    }
    // сброс значений переменных
    void ResetTime(float& var1, float& var2) { var1 = 0.0f; var2 = 0.0f; }
    // сброс значений вектора
    void ResetTime(std::vector<float>& part_time) {
        for (int i = 0; i < part_time.size(); ++i )
            part_time[i] = 0;
    }
    // удаление пробелов из строки
    void CutSpaces(std::string& str) { str.erase(remove_if(str.begin(), str.end(), isspace), str.end()); }
    // преобразование строки в bool-значение
    bool to_bool(std::string s) { return (s == "1" || s == "true"); }
}