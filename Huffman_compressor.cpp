// Huffman.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <fstream>
#include <iostream>
#include <string>
#include<map>
#include <vector>
#include <locale>
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

	while (f) {
		unsigned char ch;
		f.read(reinterpret_cast<char*>(&ch), sizeof(ch));
		if (f.gcount() > 0) {
			++weight[ch];
		}
	}
	f.close();
	/*for (auto& i : weight)
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
	*/
	// нужно отсортировать веса, можем положить в map, в первый положим вес, во второй символ

	/*for (int i = 0; i < 0x100; i++)
	{
	}
	// распечатаем данные из карты
	for (auto i : sortedWeight)
	{
		if (i.first > 0)
		{
			cout << i.first << " " << i.second<< endl;
		}
	}
	*/
	multimap <int/*вес*/, int/*индекс*/> sortedWeight;
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
		tree.push_back(Node{ '\0',-1,n0,n1,false });
		tree[n0].parent = tree.size() - 1;
		tree[n0].branch = false;
		tree[n1].parent = tree.size() - 1;
		tree[n1].branch = true;
		//суммы весов детей, потом индекс
		sortedWeight.insert(make_pair(w0 + w1, tree.size() - 1));
	}
	vector<bool> data;
	f.open("Alices Adventures in Wonderland.txt", ios::binary);
	if (!f) {
		cerr << "Ошибка открытия файла для чтения" << endl;
		return -1;
	}
	//ifstream f("Alices Adventures in Wonderland.txt");
	while (f)
	{
		unsigned char ch;
		//f.read((char*)&ch, sizeof(ch));
		f.read(reinterpret_cast<char*>(&ch), sizeof(ch));
		if (f.gcount() > 0) {
			auto n = tree[charMap[ch]];
			vector<bool> compressedChar;
			//auto n = tree[charMap[ch]];
			//vector<bool> compressedChar;
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
	//ofstream f("Huffmantext.huff");
	int treeSize = tree.size();
	//f.write((char*)&treeSize, sizeof(treeSize));
	outFile.write(reinterpret_cast<char*>(&treeSize), sizeof(treeSize));
	for (auto& node : tree) //(auto i : tree)
	{
		//f.write((char*)&i, sizeof(i));
		outFile.write(reinterpret_cast<char*>(&node), sizeof(node));
	}
	for (size_t i = 0; i <= data.size() / 8; i++)
	{
		unsigned char ch = 0;
		for (int j = 0; j < 8; j++)
		{
			if (data[i * 8 + j])
			{
				ch|= (1 << j);
			}
		}
		//f.write((char*)&ch, sizeof(ch));
		outFile.write(reinterpret_cast<char*>(&ch), sizeof(ch));
	}
	outFile.close();
	cout << "Архивация файла выполнена" << endl;
	return 1;
	// for (int i = 0; i < 0x100; i++)
	 //{
	  //   if (weight[i] > 0)
	  //       cout << weight[i] << " " << (char)i<<endl; //вывели распределение


	 // }
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