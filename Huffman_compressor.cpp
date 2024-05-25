// Huffman.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <fstream>
#include <iostream>
#include <string>
#include<map>
#include <vector>
#include <locale>
#include "hf.h"
using namespace std;

struct Node
{
	char ch;
	int parent;
	int zero;
	int one;
	bool branch;// к какой ветке относится нод
};

int Compressor()//в компрессоре включить вызов с консоли 1-заархивировать 2- разархивировать
{
	int weight[0x100] = { 0 };
	ifstream f("Alices Adventures in Wonderland.txt", ios::binary);
	if (!f) {
		cerr << "Ошибка открытия файла для чтения" << endl;
		return -1;
	}
	std::vector<char> bytes;
	while (f) {
		unsigned char ch;
		f.read(reinterpret_cast<char*>(&ch), sizeof(ch));
		bytes.push_back(ch);
		if (f.gcount() > 0) {
			++weight[ch];
		}
	}
	f.close();
	for (auto& i : weight)
		i = 0;
	{
		ifstream f("Alices Adventures in Wonderland.txt");
		while (!f.eof())
		{
			unsigned char ch;
			f.read((char*)&ch, sizeof(ch));
			++weight[ch];
		}
	}
	multimap <int/*вес*/, int/*индекс*/> sortedWeight;
	// нужно отсортировать веса, можем положить в map, в первый положим вес, во второй символ
	// распечатаем данные из карты
	for (auto i : sortedWeight)
	{
		if (i.first > 0)
		{
			cout << i.first << " " << i.second<< endl;
		}
	}
	vector<Node> tree;
	map<char, int> charMap;
	for (size_t i = 0; i < 0x100; i++)
	{
		if (weight[i] > 0)
		{
			tree.push_back(Node{ (char)i,-1,-1,-1,false });
			charMap[i] = tree.size() - 1;
			//выбираем два нода с минимальным весом, вытаскиваем, и они уже не будут учавствовать,
			//  но нужно будет вернуть назад безсимвольный нод,
			//  снова вытаскиваем и возвращаем нод
			sortedWeight.insert(make_pair(weight[i], tree.size() - 1));
		}
	}
	
	//будем вытаскивать ноды из сортид, отсортированные веса
		//  и пока будет больше,
		//  чем 1 элемент будем идти по циклу
	while (sortedWeight.size() > 1)
	{
		//вытащим самый первый элемент из отсортированных весов
		int w0 = begin(sortedWeight)->first;
		int n0 = begin(sortedWeight)->second;
		sortedWeight.erase(begin(sortedWeight));
		int w1 = begin(sortedWeight)->first;
		int n1 = begin(sortedWeight)->second;
		sortedWeight.erase(begin(sortedWeight));
		tree.push_back(Node{ '\0',-1,n0,n1,false});
		tree[n0].parent = tree.size() - 1;
		tree[n0].branch = false;
		tree[n1].parent = tree.size() - 1;
		tree[n1].branch = true;
		//суммы весов детей, потом индекс
		sortedWeight.insert(make_pair(w0 + w1, tree.size() - 1));
	}
	// Проверка содержимого дерева
	/*for (const auto& node : tree) {
		cout << "Node: " << node.ch << " Parent: " << node.parent << endl;
	}
	*/
	// Проверка содержимого sortedWeight
	if (sortedWeight.empty()) {
		cout << "sortedWeight пуст" << endl;
	}
	else {
		cout << "sortedWeight содержит элементы" << endl;
	}

	vector<bool> data;
	f.open("Alices Adventures in Wonderland.txt", ios::binary);
	if (!f) {
		cerr << "Ошибка открытия файла для чтения" << endl;
		return -1;
	}
	while (f)
	{
		unsigned char ch;
		f.read(reinterpret_cast<char*>(&ch), sizeof(ch));
		if (f.gcount() > 0) {
			auto n = tree[charMap[ch]];
			vector<bool> compressedChar;
			while (n.parent != -1)//у всех есть родитель кроме верхнего
			{
				compressedChar.push_back(n.branch);
				n = tree[n.parent];
			}
			data.insert(end(data), compressedChar.rbegin(), compressedChar.rend());
		}
	}
	f.close();
	ofstream outFile("Huffmantext.huff", ios::binary);
	if (!outFile) {
		cerr << "Ошибка создания выходного файла" << endl;
		return -1;
	}
	int treeSize = tree.size();
	outFile.write(reinterpret_cast<char*>(&treeSize), sizeof(treeSize));
	for (auto& node : tree) //(auto i : tree)
	{
		outFile.write(reinterpret_cast<char*>(&node), sizeof(node));
	}
	for (size_t i = 0; i < data.size(); i += 8) {
		unsigned char ch = 0;
		for (int j = 0; j < 8 && (i + j) < data.size(); j++) {
			if (data[i + j]) {
				ch |= (1 << j);
			}
		}
		outFile.write(reinterpret_cast<char*>(&ch), sizeof(ch));
	}
	outFile.close();
	
	cout << "Архивация файла выполнена" << endl;
	// Оценка вероятностей
	unordered_map<char, float> proba = estimate_proba(bytes);

	// Построение кодовой таблицы Хаффмана
	unordered_map<char, vector<bool>> codes_table = build_code(proba);

	// Кодирование данных
	vector<bool> encoded_bytes = encode(bytes, codes_table);

	// for (int i = 0; i < 0x100; i++)
	 //{
	  //   if (weight[i] > 0)
	  //       cout << weight[i] << " " << (char)i<<endl; //вывели распределение

	cout << "\nпрактический коэффициент сжатия: " << estimate_compression(bytes.size(), encoded_bytes.size() + codes_table.size()) << endl;

	//проверка оптимальности кода
	if (is_optimal(codes_table, proba)) cout << "\nКод является оптимальным." << endl;
	else cout << "\nКод НЕ является оптимальным." << endl;
	//проверка префиксности кода
	if (is_prefix(codes_table)) cout << "\nКод является префиксным." << endl;
	else cout << "\nКод НЕ является префиксным." << endl;
	// Запись кодовой таблицы в бинарный файл
	ofstream table_file("table.bin", ios::binary);
	for (const auto& pair : codes_table) {
		table_file.put(pair.first);
		size_t size = pair.second.size();
		table_file.write(reinterpret_cast<const char*>(&size), sizeof(size));
		for (bool bit : pair.second) {
			table_file.put(bit);
		}
	}
	table_file.close();

	// Запись закодированных данных в бинарный файл
	ofstream code_file("code.bin", ios::binary);
	for (size_t i = 0; i < encoded_bytes.size(); i += 8) {
		char byte = 0;
		for (size_t j = 0; j < 8 && i + j < encoded_bytes.size(); ++j) {
			byte |= (encoded_bytes[i + j] << (7 - j));
		}
		code_file.put(byte);
	}
	code_file.close();

	// Сообщение о записи файлов
	cout << "\nКодовая таблица и закодированные данные записаны в бинарные файлы table.bin и code.bin." << endl;

	return 1;

}
int Decompressor()
{
	//общение с пользователем
	vector<Node> tree;
	ifstream f("Huffmantext.huff", ios::binary);
	if (!f) {
		cerr << "Ошибка открытия файла" << endl;
		return -1;
	}
	int treeSize;
	//f.read((char*)&treeSize, sizeof(treeSize));
	f.read(reinterpret_cast<char*>(&treeSize), sizeof(treeSize));
	if (!f) {
		cerr << "Ошибка чтения размера дерева" << endl;
		return -1;
	}
	for (int i = 0; i < treeSize; i++)
	{
		Node n;
		//f.read((char*)&n, sizeof(n));
		f.read(reinterpret_cast<char*>(&n), sizeof(n));
		if (!f) {
			cerr << "Ошибка чтения узла дерева" << endl;
			return -1;
		}
		tree.push_back(n);
	}
	vector<bool> data;
	while (true) // !f.eof() глюк, если вектор не могут кратен 8 битам,
		//то попытается раскодировать и в конце могут добавиться символы,
	{
		unsigned char ch;
		f.read(reinterpret_cast<char*>(&ch), sizeof(ch));
		if (f.eof()) break;
		if (!f)
		{
			cerr << "Ошибка чтения данных" << endl;
			return -1;
		}

		//f.read((char*)&ch, sizeof(ch));
		//пройти по всем битам символа
		for (int i = 0; i < 8; i++)
		{
			data.push_back((ch & (1 << i)) != 0);
		}
	}
	ofstream outputFile("output.txt", ios::binary);
	if (!outputFile)
	{
		cerr << "Ошибка создания выходного файла" << endl;
		return -1;
	}
	//будем двигаться по ноду, который в самом конуе дерева
	size_t n = tree.size() - 1;
	//auto n = tree.size() - 1;
	for (auto i : data)
	{
		if (i)
		{
			n = tree[n].one;
		}
		else
		{
			n = tree[n].zero;
		}
		if (tree[n].one == -1)
		{
			outputFile.put(tree[n].ch);
			//cout << tree[n].ch;
			n = tree.size() - 1;
		}
	}
	outputFile.close();
	if (!outputFile)
	{
		cerr << "Ошибка при закрытии выходного файла output.txt" << endl;
		return -1;
	}

	// Оценка теоретического коэффициента сжатия
	ifstream originalFile("Alices Adventures in Wonderland.txt", ios::binary);
	if (!originalFile) {
		cerr << "Ошибка открытия исходного файла для оценки сжатия" << endl;
		return -1;
	}
	vector<char> originalBytes((istreambuf_iterator<char>(originalFile)), istreambuf_iterator<char>());
	originalFile.close();

	unordered_map<char, float> proba = estimate_proba(originalBytes);
	unordered_map<char, vector<bool>> table = build_code(proba);
	float theoretical_compression = estimate_compression(table, proba);
	cout << "\nТеоретический коэффициент сжатия: " << theoretical_compression << endl;
	cout << "\nРазархивация файла выполнена" << endl;
	return 1;
}
int main()
{
	setlocale(LC_ALL, "Russian");

	int choice;
	cout << "Чтобы архивировать файл выберите - 1 " << endl;
	cout << "Чтобы разархивировать файл выберите - 2 " << endl;
	cin >> choice;

	if (choice == 1)
	{
		Compressor();
	}
	else if (choice == 2)
	{
		Decompressor();
	}
	else {
		cout << "Неверный выбор" << endl;
	}

	return 0;

}