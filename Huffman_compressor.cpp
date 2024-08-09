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
	char ch;//символ
	int parent;//индекс родителя
	int zero;//индекс левого потомка
	int one;//индекс правого потомка
	bool branch;//принадлежность ветки
};

int Compressor()
{
	int weight[0x100] = { 0 };
	ifstream f("Alices Adventures in Wonderland.txt", ios::binary);
	if (!f) {
		cerr << "Ошибка открытия файла для чтения" << endl;
		return -1;
	}
	//Каждый символ считывается и добавляется в вектор bytes. Также обновляется массив weight для подсчета частоты каждого символа.
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
	//Массив weight заполняется частотами появления каждого символа в файле.
	for (auto& i : weight)
		i = 0;
	{
		ifstream f("Alices Adventures in Wonderland.txt", ios::binary);
		while (!f.eof())
		{
			unsigned char ch;
			f.read((char*)&ch, sizeof(ch));
			++weight[ch];
		}
	}
	//Далее создается multimap для сортировки символов по их частотам.
	multimap <int/*вес*/, int/*индекс*/> sortedWeight;
	for (auto i : sortedWeight)
	{
		if (i.first > 0)
		{
			cout << i.first << " " << i.second<< endl;
		}
	}
	vector<Node> tree;
	map<char, int> charMap;
	//Каждый символ с ненулевой частотой добавляется в вектор tree как узел дерева.
	//Вектор tree содержит структуру Node, представляющую узел дерева Хаффмана.
	//Узлы дерева сортируются по частотам в multimap sortedWeight.
	//Узлы с наименьшими частотами объединяются для создания новых узлов до тех пор, пока не останется один узел, представляющий корень дерева.
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
		//Проход по файлу снова, чтобы преобразовать каждый символ в последовательность бит на основе построенного дерева.
		//Биты для каждого символа добавляются в вектор data.
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
	//Оценка вероятностей символов в файле с помощью функции estimate_proba.
	// Построение кодовой таблицы с помощью функции build_code.
	// Кодирование данных с помощью функции encode.
	// Расчет и вывод практического коэффициента сжатия.
	// Проверка оптимальности кода и его префиксности.
	cout << "Архивация файла выполнена" << endl;
	// Оценка вероятностей
	unordered_map<char, float> proba = estimate_proba(bytes);

	// Построение кодовой таблицы Хаффмана
	unordered_map<char, vector<bool>> codes_table = build_code(proba);

	// Кодирование данных
	vector<bool> encoded_bytes = encode(bytes, codes_table);

	float theoretical_compression = estimate_compression(codes_table, proba);
	cout << "\nТеоретический коэффициент сжатия: " << theoretical_compression << endl;

	cout << "\nПрактический коэффициент сжатия: ";

	size_t encoded_size_in_bytes = (encoded_bytes.size() + 7) / 8; // округление вверх до ближайшего байта
	size_t total_compressed_size = encoded_size_in_bytes + codes_table.size();

	cout << estimate_compression(bytes.size(), total_compressed_size) << endl;


	// for (int i = 0; i < 0x100; i++)
	 //{
	  //   if (weight[i] > 0)
	  //       cout << weight[i] << " " << (char)i<<endl; //вывели распределение

	//cout << "\nпрактический коэффициент сжатия: " << estimate_compression(bytes.size(), encoded_bytes.size() + codes_table.size()) << endl;

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
	// Кодирование данных
	//vector<bool> encoded_bytes = encode(bytes, codes_table);
	// Запись закодированных данных в бинарный файл
	ofstream code_file("code.bin", ios::binary);
	size_t bit_count = encoded_bytes.size();
	code_file.write(reinterpret_cast<char*>(&bit_count), sizeof(bit_count)); // Запись количества значимых битов
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
	//Считываются сжатые данные и преобразуются обратно в последовательность бит.
	// Проход по этим битам для восстановления исходных символов с помощью дерева Хаффмана.
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
	// Чтение кодовой таблицы из файла table.bin
	unordered_map<char, vector<bool>> codes_table;
	ifstream table_file("table.bin", ios::binary);
	if (!table_file) {
		cerr << "Ошибка открытия файла table.bin" << endl;
		return -1;
	}

	while (true) {
		char ch;
		table_file.get(ch);
		if (table_file.eof()) break;
		size_t size;
		table_file.read(reinterpret_cast<char*>(&size), sizeof(size));
		vector<bool> code(size);
		for (size_t i = 0; i < size; ++i) {
			char bit;
			table_file.get(bit);
			code[i] = bit;
		}
		codes_table[ch] = code;
	}
	table_file.close();

	// Чтение закодированных данных из файла code.bin
	vector<bool> encoded_bits;
	ifstream code_file("code.bin", ios::binary);
	if (!code_file) {
		cerr << "Ошибка открытия файла code.bin" << endl;
		return -1;
	}

	size_t bit_count;
	code_file.read(reinterpret_cast<char*>(&bit_count), sizeof(bit_count)); // Чтение количества значимых битов

	while (true) {
		char byte;
		code_file.get(byte);
		if (code_file.eof()) break;
		for (int i = 7; i >= 0; --i) {
			encoded_bits.push_back((byte >> i) & 1);
		}
	}
	code_file.close();

	// Удаление лишних битов
	if (encoded_bits.size() > bit_count) {
		encoded_bits.resize(bit_count);
	}

	// Декодирование данных
	vector<char> decoded_data = decode(encoded_bits, codes_table);
	// Запись декодированных данных в файл output.txt
	ofstream output_file("output.txt", ios::binary);
	if (!output_file) {
		cerr << "Ошибка создания выходного файла" << endl;
		return -1;
	}

	for (char ch : decoded_data) {
		output_file.put(ch);
	}
	outputFile.close();
	if (!outputFile)
	{
		cerr << "Ошибка при закрытии выходного файла output.txt" << endl;
		return -1;
	}

	// Оценка теоретического коэффициента сжатия
	/*ifstream originalFile("Alices Adventures in Wonderland.txt", ios::binary);
	if (!originalFile) {
		cerr << "Ошибка открытия исходного файла для оценки сжатия" << endl;
		return -1;
	}
	vector<char> originalBytes((istreambuf_iterator<char>(originalFile)), istreambuf_iterator<char>());
	originalFile.close();

	unordered_map<char, float> proba = estimate_proba(originalBytes);
	unordered_map<char, vector<bool>> table = build_code(proba);
	*/
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