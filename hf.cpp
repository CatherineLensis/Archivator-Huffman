#include "hf.h"
#include <functional>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <queue>
#include <cmath>
using namespace std;
unordered_map<char, float> estimate_proba(const std::vector<char>& bytes) //Оценивает вероятности появления каждого символа.
{
	unordered_map<char, int> count_map; // Счетчик для каждого байта

	// Подсчет количества каждого байта
	for (char byte : bytes) {
		count_map[byte]++;
	}

	std::unordered_map<char, float> proba_map; // Карта для хранения вероятностей
	int total_bytes = bytes.size(); // Общее количество байт

	// Расчет вероятностей
	for (const auto& pair : count_map) {
		char byte = pair.first;
		int count = pair.second;
		proba_map[byte] = static_cast<float>(count) / total_bytes;
	}

	return proba_map;
}

unordered_map<char, vector<bool>> build_code(const unordered_map<char, float>& proba)//Строит кодовую таблицу на основе вероятностей.
{
	// Построение кодовой таблицы с использованием алгоритма Хаффмана
	   // Определение структуры узла
	struct Node
	{
		char ch;
		float freq;
		Node* left;
		Node* right;
		Node(char c, float f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
		Node(Node* l, Node* r) : ch(0), freq(l->freq + r->freq), left(l), right(r) {}
	};
	// Компаратор для очереди с приоритетом
	auto comp = [](Node* l, Node* r) { return l->freq > r->freq; };
	priority_queue<Node*, vector<Node*>, decltype(comp)> pq(comp);
	// Заполнение очереди узлами, содержащими символы и их вероятности
	for (const auto& p : proba) {
		pq.push(new Node(p.first, p.second));
	}
	// Построение дерева Хаффмана
	while (pq.size() > 1)// проверяет, что в очереди с приоритетом (priority queue) есть больше одного элемента.
		//Это необходимо, потому что алгоритм Хаффмана строит дерево, объединяя узлы, пока не останется только один узел, представляющий корень дерева.
	{
		Node* left = pq.top(); pq.pop();//извлекает узел с наименьшей частотой из очереди и сохраняет его в указателе left. После этого узел удаляется из очереди.
		Node* right = pq.top(); pq.pop();// извлекает следующий узел с наименьшей частотой и сохраняет его в указателе right. Узел также удаляется из очереди.
		pq.push(new Node(left, right));//создает новый узел, объединяя два извлеченных узла.
		//Конструктор Node(Node* l, Node* r) создает новый узел, где left и right становятся дочерними узлами нового узла. Частота нового узла является суммой частот его дочерних узлов.
	}

	Node* root = pq.top();

	unordered_map<char, vector<bool>> table;
	vector<bool> code;
	// Обход дерева Хаффмана для построения кодовых слов
	function<void(Node*)> dfs = [&](Node* node)
		{
			if (!node->left && !node->right) {
				table[node->ch] = code;
				return;
			}
			if (node->left) {
				code.push_back(0);
				dfs(node->left);
				code.pop_back();
			}
			if (node->right) {
				code.push_back(1);
				dfs(node->right);
				code.pop_back();
			}
		};

	dfs(root);

	return table;
}
// Функция для кодирования последовательности байтов с использованием таблицы кодов Хаффмана
vector<bool> encode(const vector<char>& bytes, const unordered_map<char, vector<bool>>& table)
{
	vector<bool> bits;
	for (char ch : bytes) {
		const vector<bool>& code = table.at(ch);
		bits.insert(bits.end(), code.begin(), code.end());
	}
	return bits;
}
// Функция для декодирования последовательности битов обратно в последовательность байтов
vector<char> decode(const std::vector<bool>& bits, const unordered_map<char, vector<bool>>& table) {
	// Здесь мы предполагаем, что есть функция для построения обратной таблицы декодирования
	unordered_map<vector<bool>, char> reverse_table;
	for (const auto& p : table) {
		reverse_table[p.second] = p.first;
	}

	vector<char> bytes;
	vector<bool> current_bits;
	for (bool bit : bits) {
		current_bits.push_back(bit);
		if (reverse_table.count(current_bits)) {
			bytes.push_back(reverse_table[current_bits]);
			current_bits.clear();
		}
	}
	return bytes;
}


// Оценить теоретический коэффициент сжатия
float estimate_compression(const unordered_map<char, vector<bool>>& table, const unordered_map<char, float>& proba) {
	float average_length = 0;
	for (const auto& p : proba) {
		average_length += p.second * table.at(p.first).size();
	}
	return 8 / average_length;
}
// Перегруженная функция для оценки фактического коэффициента сжатия
float estimate_compression(size_t bytes_before, size_t bytes_after) {
	return static_cast<float>(bytes_before) / bytes_after;
}

// Оценка энтропии последовательности байтов
float calculate_entropy(const unordered_map<char, float>& proba) {
	float entropy = 0;
	for (const auto& p : proba) {
		entropy -= p.second * log2(p.second);
	}
	return entropy;
}

// Оценка средней длины кодового слова
float calculate_average_code_length(const unordered_map<char, vector<bool>>& table, const unordered_map<char, float>& proba) {
	float average_length = 0;
	for (const auto& p : proba) {
		average_length += p.second * table.at(p.first).size();
	}
	return average_length;
}
// Функция для проверки, является ли код Хаффмана оптимальным
bool is_optimal(const unordered_map<char, vector<bool>>& table, const unordered_map<char, float>& proba) {
	float entropy = calculate_entropy(proba);
	float average_code_length = calculate_average_code_length(table, proba);
	// Код считается оптимальным, если средняя длина кодового слова больше или равна энтропии
	return average_code_length >= entropy;
}


bool is_prefix(const unordered_map<char, vector<bool>>& table) {
	for (const auto& p1 : table) {
		for (const auto& p2 : table) {
			if (p1.first != p2.first) {
				if (std::mismatch(p1.second.begin(), p1.second.end(), p2.second.begin()).first == p1.second.end()) {
					return false; // p1.second is a prefix of p2.second
				}
			}
		}
	}
	return true;
}

// Преобразовать набор байт в набор бит.
// @param bytes набор байт.
// @return набор бит.
inline vector<bool> bytes2bits(const vector<char>& bytes)
{
	vector<bool> bits; // Создаем вектор для хранения битов
	for (char byte : bytes) { // Перебираем каждый байт входного вектора
		for (int i = 7; i >= 0; --i) { // Проходим по каждому биту в байте, начиная с младшего
			// Добавляем бит в вектор.
			// Используем сдвиг вправо на i позиций и побитовое И с 1 для получения значения бита.
			bits.push_back((byte >> i) & 1);
		}
	}
	return bits; // Возвращаем вектор битов
}

