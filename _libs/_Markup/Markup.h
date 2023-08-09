#pragma once

#include "../_DuckDB/duckdb.hpp"
#include "../settings.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#define DEFAULT_VALUE "__DEFAULT__"
#define DEFAULT_INT -1
#define DEFAULT_CHAR "undef"
#define DEFAULT_BOOL false
#define DEFAULT_FLOAT -1.0f

// Класс, трансформирующий разметку из csv/bin+toml/json форматов
// в базу данных duckdb, а также позволяющий работать с ней
class Markup {

    enum Type : int {
        NONE, 
        INTEGER,
        VARCHAR,
        BOOLEAN,
        FLOAT
    };

    duckdb::DuckDB* db;
    duckdb::Connection* con;
    duckdb::Appender* appender;
    
    std::string csv_fname;  // файл-источник
    std::string bin_fname;  // файл-источник
    std::string toml_fname; // файл-источник

    std::string Table_name = "AllFileInfo"; //таблица с данными по всему файлу
    std::string FB_name = "FlatBorders";    //таблица с данными по комплексу (сведённые границы)
    std::string Channel_name = "ChannelInfo";//таблица с данными по одному каналу

    std::string file_name;  // имя файла с базой данных
    
    int col_count;
    std::vector<std::string> col_names;     //имена колонок
    std::vector<std::string> col_types_str; //типы колонок в строковом значении
    std::vector<Type> col_types;    //вектор типов каждого столбца (enum)

    int num = 0;    //порядковый номер записи

    //границы колонок в csv-файле
    inline bool isBorder(char c) {
        return (c == ' ' || c == ';' || c == ',');
    }
    //заполнение вектора col_types, зная названия этих типов (string -> enum)
    inline void toType(std::vector<std::string> str_types) {
        if (str_types.size() != col_names.size()) {
            std::cerr << "[ERROR]: Incorrect count of types" << std::endl;
            return;
        }
        col_types.clear();
        
        for (int i = 0; i < str_types.size(); ++i) {
            if (str_types[i] == "INTEGER")
                col_types.push_back(Type::INTEGER);
            else if (str_types[i] == "VARCHAR")
                col_types.push_back(Type::VARCHAR);
            else if (str_types[i] == "BOOLEAN")
                col_types.push_back(Type::BOOLEAN);
            else if (str_types[i] == "FLOAT")
                col_types.push_back(Type::FLOAT);
            else {
                col_types.push_back(Type::NONE);
                std::cerr << "[ERROR]: Incorrect type: \"" 
                          << str_types[i] << "\"" << std::endl;
            }
        }
    }
    //сплит строки по значениям границ (isBorder(c) == true)
    inline std::vector<std::string> Split(std::string str) {
        std::vector<std::string> res;
        std::string s;

        for (int i = 0; i < str.size(); ++i) {
            char c = str[i];
            
            if (isBorder(c)) {
                res.push_back(s);
                s.clear();
                continue;
            }
            s.push_back(c);
        }
        res.push_back(s);
        return res;
    }
    //ветвление по типам данных для аппендера. вынесено для удобства
    inline void AppendSwitch(Type type, std::string value) {
        switch (type)
        {
        case Type::INTEGER:
            if (value != DEFAULT_VALUE)
                appender->Append<int32_t>(atoi(value.c_str()));
            else 
                appender->Append<int32_t>(DEFAULT_INT);
            break;
        case Type::FLOAT:
            if (value != DEFAULT_VALUE)
                appender->Append<float>(atof(value.c_str()));
            else 
                appender->Append<float>(DEFAULT_FLOAT);
            break;
        case Type::BOOLEAN:
            if (value != DEFAULT_VALUE)
                appender->Append<bool>(settings::to_bool(value));
            else 
                appender->Append<bool>(DEFAULT_BOOL);
            break;
        case Type::VARCHAR:
            if (value != DEFAULT_VALUE)
                appender->Append<duckdb::string_t>(value);
            else 
                appender->Append<duckdb::string_t>(DEFAULT_CHAR);
            break;
        default:
            break;
        }
    }

    inline void Init() {
        col_count = col_names.size();

        std::string init_str = "CREATE TABLE " + Table_name + 
                               "(NUM INTEGER, ";
        for (int i = 0; i < col_names.size(); ++i) {
            init_str = init_str + col_names[i] + " " 
                                + col_types_str[i] + ", ";
        }
        init_str.erase(init_str.end() - 2, init_str.end()); 
        init_str.push_back(')');
        // std::cerr << "{" << init_str << "}" << std::endl;
        con->Query(init_str);
        appender = new duckdb::Appender(*con, Table_name);
    }

    inline void InitAllFileInfo() {

    }

public:
    Markup(std::string _table_name) {
        // Table_name = _table_name;
        file_name = settings::MARKUP_DB_DIR + 
                    Table_name + 
                    ".duckdb";
        
        db = new duckdb::DuckDB(file_name);
        con = new duckdb::Connection(*db);  
        appender = nullptr;
    }

    ~Markup() {
        if (db != nullptr)
            delete db;
        if (con != nullptr)
            delete con;
        if (appender != nullptr)
            delete appender;
    }

    inline void Reset() {
        if (db != nullptr)
            delete db; 
        db = nullptr;

        if (con != nullptr)
            delete con; 
        con = nullptr;

        if (appender != nullptr)
            delete appender; 
        appender = nullptr;

        csv_fname.clear();
        bin_fname.clear();
        toml_fname.clear();
        col_names.clear();
    }

    //чтение заголовочного файла для создания корректной таблички
    inline void InitCSV(std::string _file_name) {
        // Reset();
        col_names.clear();

        // csv_fname = _file_name;
        std::ifstream csv_in(_file_name);
        if(!csv_in.is_open()) {
            std::cerr << "::ParseCSV() - CSV file didn't found!" << std::endl;
            return;
        }

        std::string names, types;
        std::getline(csv_in, names);
        std::getline(csv_in, types);

        //записываем имена столбцов в вектор
        col_names = Split(names);
        //записываем типы данных для создания корректной таблички
        col_types_str = Split(types);
        toType(col_types_str);

        if (col_names.size() != col_types_str.size()) {
            std::cerr << "[ERROR]: Different count of columns in header rows"
                      << std::endl;
            return;
        }

        std::unique_ptr<duckdb::MaterializedQueryResult> result = 
            con->Query("SELECT * FROM " + Table_name);
        num = result->RowCount();

        Init();
    }
    //копирование информации из csv в базу данных
    inline void ParseCSV(std::string _file_name) {
        csv_fname = _file_name;

        std::ifstream csv_in(csv_fname);
        if (!csv_in.is_open()) {
            std::cerr << "[ERROR]: CSV file didn't found" << std::endl;
            return;
        }

        std::string cur_str;
        std::vector<std::string> splited;

        getline(csv_in, cur_str);
        splited = Split(cur_str);

        if (splited != col_names) {
            std::cerr << "[ERROR]: Different columns" << std::endl;
            return;
        }

        while (true) {
            getline(csv_in, cur_str);
            if (csv_in.eof())
                break;
            splited = Split(cur_str);

            if (splited.size() != col_count) {
                std::cerr << "[ERROR]: Incorrect table: " 
                          << col_count << " - " << splited.size()
                          << std::endl
                          << "{" << splited[0] << "}" 
                          << std::endl
                          << cur_str
                          << std::endl;
                return;
            }

            appender->BeginRow();
            appender->Append<int32_t>(num++);

            for (int i = 0; i < col_count; ++i) {
                AppendSwitch(col_types[i], splited[i]);
            }

            appender->EndRow();
        }
        appender->Flush();
    } //ParseCSV

    //пока без строковых значений
    template<typename T> 
    inline void EditCell(int num, std::string column, T new_value) {
        con->Query(
            "UPDATE " + Table_name + " SET " +
            column + "=" + std::to_string(new_value) + 
            " WHERE NUM=" + std::to_string(num)
            );
    }

    //добавляет одну строчку, используя объект json 
    inline void AddRow(nlohmann::json j) {
        appender->BeginRow();
        appender->Append(num++);
        for (int i = 0; i < col_names.size(); ++i) {
            if (j.contains(col_names[i])) 
                AppendSwitch(col_types[i], j[col_names[i]].dump());
            else 
                AppendSwitch(col_types[i], DEFAULT_VALUE);
        }
        appender->EndRow();
        appender->Flush();
    }
    //добавление всех строк из json-файла
    inline void AddJson(std::string fname) {
        nlohmann::json j;
        std::ifstream inp(fname);
        inp >> j;

        if (j.contains("global_intervals"))
            j = j["global_intervals"];

        for (int i = 0; i < j.size(); ++i)
            AddRow(j[i]);
    }

    inline void DeleteRow(int delete_num) {
        if (delete_num < 0 || delete_num > num - 1)
            return;

        con->Query("DELETE FROM " + Table_name 
                  + " WHERE NUM=" + std::to_string(delete_num));
        
        for (int i = delete_num + 1; i < num; ++i) {
            EditCell(i, "NUM", i - 1);
        }
        --num;
    }

    inline void DeleteRow(int delete_start, int delete_end) {
        if (delete_end < delete_start)
            return;

        for (int i = delete_start; i < delete_end; ++i) 
            DeleteRow(i);
    }
    //вставка строки на нужную позицию
    inline void InsertRow(int pos, nlohmann::json j) {
        if (pos < 0)
            pos = 0;

        if (pos > num) {
            AddRow(j);
            return;
        }

        // TODO
    }

    inline void PrintCurrentDB() {
        std::unique_ptr<duckdb::MaterializedQueryResult> result = 
            con->Query("SELECT * FROM " + Table_name);
        std::cerr << result->ToString() << std::endl
                  << std::endl << std::endl;
    }
}; //Markup
