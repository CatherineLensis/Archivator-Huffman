#include "hf.h"
#include <functional>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <queue>
#include <cmath>
using namespace std;
unordered_map<char, float> estimate_proba(const std::vector<char>& bytes) //��������� ����������� ��������� ������� �������.
{
	unordered_map<char, int> count_map; // ������� ��� ������� �����

	// ������� ���������� ������� �����
	for (char byte : bytes) {
		count_map[byte]++;
	}

	std::unordered_map<char, float> proba_map; // ����� ��� �������� ������������
	int total_bytes = bytes.size(); // ����� ���������� ����

	// ������ ������������
	for (const auto& pair : count_map) {
		char byte = pair.first;
		int count = pair.second;
		proba_map[byte] = static_cast<float>(count) / total_bytes;
	}

	return proba_map;
}

unordered_map<char, vector<bool>> build_code(const unordered_map<char, float>& proba)//������ ������� ������� �� ������ ������������.
{
	// ���������� ������� ������� � �������������� ��������� ��������
	   // ����������� ��������� ����
	struct Node
	{
		char ch;
		float freq;
		Node* left;
		Node* right;
		Node(char c, float f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
		Node(Node* l, Node* r) : ch(0), freq(l->freq + r->freq), left(l), right(r) {}
	};
	// ���������� ��� ������� � �����������
	auto comp = [](Node* l, Node* r) { return l->freq > r->freq; };
	priority_queue<Node*, vector<Node*>, decltype(comp)> pq(comp);
	// ���������� ������� ������, ����������� ������� � �� �����������
	for (const auto& p : proba) {
		pq.push(new Node(p.first, p.second));
	}
	// ���������� ������ ��������
	while (pq.size() > 1)// ���������, ��� � ������� � ����������� (priority queue) ���� ������ ������ ��������.
		//��� ����������, ������ ��� �������� �������� ������ ������, ��������� ����, ���� �� ��������� ������ ���� ����, �������������� ������ ������.
	{
		Node* left = pq.top(); pq.pop();//��������� ���� � ���������� �������� �� ������� � ��������� ��� � ��������� left. ����� ����� ���� ��������� �� �������.
		Node* right = pq.top(); pq.pop();// ��������� ��������� ���� � ���������� �������� � ��������� ��� � ��������� right. ���� ����� ��������� �� �������.
		pq.push(new Node(left, right));//������� ����� ����, ��������� ��� ����������� ����.
		//����������� Node(Node* l, Node* r) ������� ����� ����, ��� left � right ���������� ��������� ������ ������ ����. ������� ������ ���� �������� ������ ������ ��� �������� �����.
	}

	Node* root = pq.top();

	unordered_map<char, vector<bool>> table;
	vector<bool> code;
	// ����� ������ �������� ��� ���������� ������� ����
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
// ������� ��� ����������� ������������������ ������ � �������������� ������� ����� ��������
vector<bool> encode(const vector<char>& bytes, const unordered_map<char, vector<bool>>& table)
{
	vector<bool> bits;
	for (char ch : bytes) {
		const vector<bool>& code = table.at(ch);
		bits.insert(bits.end(), code.begin(), code.end());
	}
	return bits;
}
// ������� ��� ������������� ������������������ ����� ������� � ������������������ ������
vector<char> decode(const std::vector<bool>& bits, const unordered_map<char, vector<bool>>& table) {
	// ����� �� ������������, ��� ���� ������� ��� ���������� �������� ������� �������������
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


// ������� ������������� ����������� ������
float estimate_compression(const unordered_map<char, vector<bool>>& table, const unordered_map<char, float>& proba) {
	float average_length = 0;
	for (const auto& p : proba) {
		average_length += p.second * table.at(p.first).size();
	}
	return 8 / average_length;
}
// ������������� ������� ��� ������ ������������ ������������ ������
float estimate_compression(size_t bytes_before, size_t bytes_after) {
	return static_cast<float>(bytes_before) / bytes_after;
}

// ������ �������� ������������������ ������
float calculate_entropy(const unordered_map<char, float>& proba) {
	float entropy = 0;
	for (const auto& p : proba) {
		entropy -= p.second * log2(p.second);
	}
	return entropy;
}

// ������ ������� ����� �������� �����
float calculate_average_code_length(const unordered_map<char, vector<bool>>& table, const unordered_map<char, float>& proba) {
	float average_length = 0;
	for (const auto& p : proba) {
		average_length += p.second * table.at(p.first).size();
	}
	return average_length;
}
// ������� ��� ��������, �������� �� ��� �������� �����������
bool is_optimal(const unordered_map<char, vector<bool>>& table, const unordered_map<char, float>& proba) {
	float entropy = calculate_entropy(proba);
	float average_code_length = calculate_average_code_length(table, proba);
	// ��� ��������� �����������, ���� ������� ����� �������� ����� ������ ��� ����� ��������
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

// ������������� ����� ���� � ����� ���.
// @param bytes ����� ����.
// @return ����� ���.
inline vector<bool> bytes2bits(const vector<char>& bytes)
{
	vector<bool> bits; // ������� ������ ��� �������� �����
	for (char byte : bytes) { // ���������� ������ ���� �������� �������
		for (int i = 7; i >= 0; --i) { // �������� �� ������� ���� � �����, ������� � ��������
			// ��������� ��� � ������.
			// ���������� ����� ������ �� i ������� � ��������� � � 1 ��� ��������� �������� ����.
			bits.push_back((byte >> i) & 1);
		}
	}
	return bits; // ���������� ������ �����
}

