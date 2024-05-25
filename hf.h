#pragma once
#include <unordered_map>

/**
* Оценить распределение вероятностей встречаемости байт.
* @param bytes последовательность байт, для которой выполняется оценка.
* @return распределение вероятностей (ассоциативный массив):
* ключ - значение символа, значение - вероятность встретить байт в последовательности.
*/
std::unordered_map<char, float> estimate_proba(const std::vector<char>& bytes);
// Прототипы функций
std::unordered_map<char, float> estimate_proba(const std::vector<char>& bytes);
std::unordered_map<char, std::vector<bool>> build_code(const std::unordered_map<char, float>& proba);
std::vector<bool> encode(const std::vector<char>& bytes, const std::unordered_map<char, std::vector<bool>>& table);
std::vector<char> decode(const std::vector<bool>& bits, const std::unordered_map<char, std::vector<bool>>& table);
float estimate_compression(const std::unordered_map<char, std::vector<bool>>& table, const std::unordered_map<char, float>& proba);
float estimate_compression(size_t bytes_before, size_t bytes_after);
bool is_optimal(const std::unordered_map<char, std::vector<bool>>& table, const std::unordered_map<char, float>& proba);
bool is_prefix(const std::unordered_map<char, std::vector<bool>>& table);